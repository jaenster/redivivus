//! What the compile driver needs of the system beyond util.zig: child processes (posix_spawn,
//! both pipes captured, a timeout), a mutex, file locks, directory walks in os.walk's
//! manner, os.path's normpath/relpath/realpath, and the text rules of a Python str decoded
//! from latin1 (splitlines, strip, isspace) that the compiler's output is judged by.
const std = @import("std");
const c = @import("c");
const util = @import("util.zig");
const gpa = util.gpa;

extern var environ: [*:null]?[*:0]u8;

// ---- locking

pub const Mutex = struct {
    m: c.pthread_mutex_t = undefined,
    pub fn init(self: *Mutex) void {
        _ = c.pthread_mutex_init(&self.m, null);
    }
    pub fn lock(self: *Mutex) void {
        _ = c.pthread_mutex_lock(&self.m);
    }
    pub fn unlock(self: *Mutex) void {
        _ = c.pthread_mutex_unlock(&self.m);
    }
};

/// flock(LOCK_EX) on `path` (created), held until the returned descriptor is closed.
pub fn flockPath(path: []const u8) !c_int {
    const pz = try util.z(path);
    const fd = c.open(pz.ptr, c.O_WRONLY | c.O_CREAT | c.O_TRUNC | c.O_CLOEXEC, @as(c_uint, 0o666));
    if (fd < 0) return error.Open;
    while (c.flock(fd, c.LOCK_EX) != 0) {
        if (errno() != c.EINTR) return error.Lock;
    }
    return fd;
}

pub fn closeFd(fd: c_int) void {
    _ = c.close(fd);
}

// ---- time

pub fn now() f64 {
    var tv: c.struct_timeval = undefined;
    _ = c.gettimeofday(&tv, null);
    return @as(f64, @floatFromInt(tv.tv_sec)) + @as(f64, @floatFromInt(tv.tv_usec)) / 1e6;
}

// ---- files

pub fn remove(path: []const u8) void {
    const pz = util.z(path) catch return;
    _ = c.unlink(pz.ptr);
}

/// os.replace.
pub fn rename(from: []const u8, to: []const u8) !void {
    const a = try util.z(from);
    const b = try util.z(to);
    if (c.rename(a.ptr, b.ptr) != 0) return error.Rename;
}

pub fn isDir(path: []const u8) bool {
    const pz = util.z(path) catch return false;
    var st: c.struct_stat = undefined;
    return c.stat(pz.ptr, &st) == 0 and (st.st_mode & c.S_IFMT) == c.S_IFDIR;
}

pub fn isFile(path: []const u8) bool {
    const pz = util.z(path) catch return false;
    var st: c.struct_stat = undefined;
    return c.stat(pz.ptr, &st) == 0 and (st.st_mode & c.S_IFMT) == c.S_IFREG;
}

/// os.path.getsize (0 when it does not exist).
pub fn fileSize(path: []const u8) u64 {
    const pz = util.z(path) catch return 0;
    var st: c.struct_stat = undefined;
    if (c.stat(pz.ptr, &st) != 0) return 0;
    return @intCast(st.st_size);
}

pub const Stat = struct { ino: u64, size: i64, mtime_ns: i128 };

/// A stat's modification and access times: Darwin names the fields st_mtimespec and
/// st_atimespec, glibc and musl st_mtim and st_atim.
pub fn statMtime(st: *const c.struct_stat) c.struct_timespec {
    return if (@hasField(c.struct_stat, "st_mtimespec")) st.st_mtimespec else st.st_mtim;
}
pub fn statAtime(st: *const c.struct_stat) c.struct_timespec {
    return if (@hasField(c.struct_stat, "st_atimespec")) st.st_atimespec else st.st_atim;
}

/// Only the descriptors a spawn's file actions set reach the child, so another thread's
/// pipes never leak into it: Darwin has a spawn flag for that; elsewhere the pipes are
/// opened close-on-exec, and the dup2 onto 0, 1 and 2 clears the flag on the child's copy.
const spawn_flags = if (@hasDecl(c, "POSIX_SPAWN_CLOEXEC_DEFAULT")) c.POSIX_SPAWN_CLOEXEC_DEFAULT else 0;
extern "c" fn pipe2(fds: *[2]c_int, flags: c_int) c_int;
fn pipeCloexec(fds: *[2]c_int) c_int {
    if (@hasDecl(c, "POSIX_SPAWN_CLOEXEC_DEFAULT")) return c.pipe(fds);
    return pipe2(fds, c.O_CLOEXEC);
}

/// The calling thread's errno, which Darwin reaches through __error and glibc through
/// __errno_location.
fn errno() c_int {
    return std.c._errno().*;
}

pub fn stat(path: []const u8) ?Stat {
    const pz = util.z(path) catch return null;
    var st: c.struct_stat = undefined;
    if (c.stat(pz.ptr, &st) != 0) return null;
    return .{ .ino = @intCast(st.st_ino), .size = @intCast(st.st_size), .mtime_ns = @as(i128, statMtime(&st).tv_sec) * 1_000_000_000 + statMtime(&st).tv_nsec };
}

/// Write via a temporary file and rename (build.py's write_text, objcache.install).
pub fn writeAtomic(path: []const u8, data: []const u8) !void {
    const tmp = try std.fmt.allocPrint(gpa, "{s}.{d}.{d}.tmp", .{ path, c.getpid(), threadTag() });
    try util.writeFile(tmp, data);
    try rename(tmp, path);
}

/// Distinguishes the threads of this process in temporary names; Python's names carry
/// only the pid, its threads never write the same file at once (the GIL and the locks).
fn threadTag() u64 {
    // pthread_t is a pointer on Darwin and an unsigned long on glibc.
    const t = c.pthread_self();
    return if (@typeInfo(@TypeOf(t)) == .pointer or @typeInfo(@TypeOf(t)) == .optional) @intFromPtr(t) else @intCast(t);
}

/// shutil.copyfile into a temporary beside `dst`, then os.replace (objcache.install).
pub fn copyAtomic(src: []const u8, dst: []const u8) !void {
    try writeAtomic(dst, try util.readFile(src));
}

/// The names in a directory (os.listdir: no '.' or '..'), in no particular order.
pub fn listDir(path: []const u8) ![]const []const u8 {
    var out: std.ArrayList([]const u8) = .empty;
    const pz = try util.z(path);
    const d = c.opendir(pz.ptr) orelse return out.items;
    defer _ = c.closedir(d);
    while (c.readdir(d)) |e| {
        const name = std.mem.sliceTo(@as([*:0]const u8, @ptrCast(&e.*.d_name)), 0);
        if (std.mem.eql(u8, name, ".") or std.mem.eql(u8, name, "..")) continue;
        try out.append(gpa, try gpa.dupe(u8, name));
    }
    return out.items;
}

pub const WalkDir = struct { path: []const u8, dirs: [][]const u8, files: [][]const u8 };

/// One level of os.walk: subdirectories (a symbolic link to a directory counts as one,
/// and is not descended into) and the rest.
pub fn scanDir(path: []const u8) !WalkDir {
    var dirs: std.ArrayList([]const u8) = .empty;
    var files: std.ArrayList([]const u8) = .empty;
    for (try listDir(path)) |name| {
        const full = try util.join(path, name);
        if (isDir(full)) try dirs.append(gpa, name) else try files.append(gpa, name);
    }
    return .{ .path = path, .dirs = dirs.items, .files = files.items };
}

pub fn isLink(path: []const u8) bool {
    const pz = util.z(path) catch return false;
    var st: c.struct_stat = undefined;
    return c.lstat(pz.ptr, &st) == 0 and (st.st_mode & c.S_IFMT) == c.S_IFLNK;
}

/// os.walk(top) top-down; `prune(dirpath, dirs)` may drop subdirectories (dirnames[:] = ...).
pub fn walk(top: []const u8, ctx: anytype, comptime visit: fn (@TypeOf(ctx), *WalkDir) anyerror!void) !void {
    if (!isDir(top)) return;
    var w = try scanDir(top);
    try visit(ctx, &w);
    for (w.dirs) |d| {
        const sub = try util.join(top, d);
        if (isLink(sub)) continue;
        try walk(sub, ctx, visit);
    }
}

/// Every file under `top` (os.walk, followlinks=False), full paths.
pub fn allFiles(top: []const u8) ![]const []const u8 {
    var out: std.ArrayList([]const u8) = .empty;
    const V = struct {
        fn visit(o: *std.ArrayList([]const u8), w: *WalkDir) !void {
            for (w.files) |f| try o.append(gpa, try util.join(w.path, f));
        }
    };
    try walk(top, &out, V.visit);
    return out.items;
}

pub fn lessThan(_: void, a: []const u8, b: []const u8) bool {
    return std.mem.order(u8, a, b) == .lt;
}

pub fn sortStrings(items: [][]const u8) void {
    std.mem.sort([]const u8, items, {}, lessThan);
}

// ---- os.path

/// os.path.normpath (posix).
pub fn normpath(p: []const u8) ![]const u8 {
    if (p.len == 0) return ".";
    const abs = p[0] == '/';
    const lead2 = abs and p.len > 1 and p[1] == '/' and (p.len == 2 or p[2] != '/');
    var comps: std.ArrayList([]const u8) = .empty;
    var it = std.mem.splitScalar(u8, p, '/');
    while (it.next()) |comp| {
        if (comp.len == 0 or std.mem.eql(u8, comp, ".")) continue;
        if (std.mem.eql(u8, comp, "..")) {
            if (comps.items.len > 0 and !std.mem.eql(u8, comps.items[comps.items.len - 1], "..")) {
                _ = comps.pop();
                continue;
            }
            if (abs) continue;
        }
        try comps.append(gpa, comp);
    }
    const body = try std.mem.join(gpa, "/", comps.items);
    const prefix: []const u8 = if (lead2) "//" else if (abs) "/" else "";
    const out = try std.mem.concat(gpa, u8, &.{ prefix, body });
    return if (out.len == 0) "." else out;
}

/// os.path.join(a, b) for a relative b.
pub fn joinNorm(a: []const u8, b: []const u8) ![]const u8 {
    if (b.len > 0 and b[0] == '/') return normpath(b);
    return normpath(try util.join(a, b));
}

/// os.path.relpath(path, start) for absolute paths.
pub fn relpath(path: []const u8, start: []const u8) ![]const u8 {
    const p = try normpath(path);
    const s = try normpath(start);
    var pc: std.ArrayList([]const u8) = .empty;
    var sc: std.ArrayList([]const u8) = .empty;
    var it = std.mem.tokenizeScalar(u8, p, '/');
    while (it.next()) |x| try pc.append(gpa, x);
    it = std.mem.tokenizeScalar(u8, s, '/');
    while (it.next()) |x| try sc.append(gpa, x);
    var i: usize = 0;
    while (i < pc.items.len and i < sc.items.len and std.mem.eql(u8, pc.items[i], sc.items[i])) i += 1;
    var parts: std.ArrayList([]const u8) = .empty;
    for (i..sc.items.len) |_| try parts.append(gpa, "..");
    try parts.appendSlice(gpa, pc.items[i..]);
    if (parts.items.len == 0) return ".";
    return std.mem.join(gpa, "/", parts.items);
}

/// os.path.realpath (strict=False): symbolic links resolved as far as the path exists.
pub fn realpath(p: []const u8) ![]const u8 {
    var buf: [4096]u8 = undefined;
    const pz = try util.z(p);
    if (c.realpath(pz.ptr, &buf)) |r| return gpa.dupe(u8, std.mem.sliceTo(r, 0));
    const np = try normpath(p);
    if (std.mem.eql(u8, np, "/") or std.mem.indexOfScalar(u8, np, '/') == null) return np;
    const parent = try realpath(util.dirname(np));
    return util.join(if (std.mem.eql(u8, parent, "/")) "" else parent, util.basename(np));
}

// ---- Python str rules over latin1-decoded text

/// str.isspace for one latin1 character.
pub fn isSpace(ch: u8) bool {
    return switch (ch) {
        ' ', '\t', '\n', '\r', 0x0b, 0x0c, 0x1c, 0x1d, 0x1e, 0x1f, 0x85, 0xa0 => true,
        else => false,
    };
}

pub fn strip(s: []const u8) []const u8 {
    var a: usize = 0;
    var b: usize = s.len;
    while (a < b and isSpace(s[a])) a += 1;
    while (b > a and isSpace(s[b - 1])) b -= 1;
    return s[a..b];
}

/// \w for one latin1 character (str.isalnum or '_').
pub fn isWord(ch: u8) bool {
    if (std.ascii.isAlphanumeric(ch) or ch == '_') return true;
    return switch (ch) {
        0xaa, 0xb2, 0xb3, 0xb5, 0xb9, 0xba, 0xbc, 0xbd, 0xbe => true,
        0xc0...0xff => ch != 0xd7 and ch != 0xf7,
        else => false,
    };
}

/// str.lower for latin1.
pub fn lower(s: []const u8) ![]u8 {
    const out = try gpa.alloc(u8, s.len);
    for (s, 0..) |ch, i| out[i] = switch (ch) {
        'A'...'Z' => ch + 32,
        0xc0...0xde => if (ch == 0xd7) ch else ch + 32,
        else => ch,
    };
    return out;
}

/// What a text-mode read (universal newlines) makes of bytes: \r\n and \r become \n.
pub fn universalNewlines(s: []const u8) ![]const u8 {
    if (std.mem.indexOfScalar(u8, s, '\r') == null) return s;
    var out: std.ArrayList(u8) = .empty;
    var i: usize = 0;
    while (i < s.len) : (i += 1) {
        if (s[i] == '\r') {
            try out.append(gpa, '\n');
            if (i + 1 < s.len and s[i + 1] == '\n') i += 1;
        } else try out.append(gpa, s[i]);
    }
    return out.items;
}

/// str.splitlines() of a latin1-decoded string.
pub fn splitlines(s: []const u8) ![]const []const u8 {
    var out: std.ArrayList([]const u8) = .empty;
    var start: usize = 0;
    var i: usize = 0;
    while (i < s.len) {
        const ch = s[i];
        const brk = switch (ch) {
            '\n', '\r', 0x0b, 0x0c, 0x1c, 0x1d, 0x1e, 0x85 => true,
            else => false,
        };
        if (!brk) {
            i += 1;
            continue;
        }
        try out.append(gpa, s[start..i]);
        i += if (ch == '\r' and i + 1 < s.len and s[i + 1] == '\n') 2 else 1;
        start = i;
    }
    if (start < s.len) try out.append(gpa, s[start..]);
    return out.items;
}

/// latin1 text as UTF-8 (a Python str written through open(..., 'w')).
pub fn latin1ToUtf8(s: []const u8) ![]const u8 {
    var hi = false;
    for (s) |ch| if (ch >= 0x80) {
        hi = true;
        break;
    };
    if (!hi) return s;
    var out: std.ArrayList(u8) = .empty;
    for (s) |ch| {
        if (ch < 0x80) try out.append(gpa, ch) else {
            try out.append(gpa, 0xc0 | (ch >> 6));
            try out.append(gpa, 0x80 | (ch & 0x3f));
        }
    }
    return out.items;
}

// ---- child processes

pub const Output = struct { out: []const u8, err: []const u8, status: i32, timed_out: bool = false };

pub const RunOptions = struct {
    /// Capture stdout and stderr (else all three are /dev/null).
    capture: bool = true,
    timeout_s: ?f64 = null,
    /// Variables to set in the child's environment, NAME=value.
    env_set: []const []const u8 = &.{},
    /// stderr into the stdout pipe (stderr=subprocess.STDOUT); `err` stays empty.
    merge_stderr: bool = false,
    /// The child's stdin (else /dev/null).
    stdin_path: ?[]const u8 = null,
};

fn buildEnv(set: []const []const u8) ![*:null]?[*:0]u8 {
    var list: std.ArrayList(?[*:0]u8) = .empty;
    var i: usize = 0;
    while (environ[i]) |e| : (i += 1) {
        const s = std.mem.sliceTo(e, 0);
        var replaced = false;
        for (set) |kv| {
            const eq = std.mem.indexOfScalar(u8, kv, '=').?;
            if (s.len > eq and std.mem.eql(u8, s[0 .. eq + 1], kv[0 .. eq + 1])) replaced = true;
        }
        if (!replaced) try list.append(gpa, e);
    }
    for (set) |kv| try list.append(gpa, (try util.z(kv)).ptr);
    try list.append(gpa, null);
    return @ptrCast(list.items.ptr);
}

/// subprocess.run(argv, capture_output=..., timeout=...): the exit status as Python reports it
/// (-N for a signal). On a timeout the child is killed and `timed_out` set.
pub fn run(argv: []const []const u8, opts: RunOptions) !Output {
    var cargv: std.ArrayList(?[*:0]const u8) = .empty;
    for (argv) |a| try cargv.append(gpa, (try util.z(a)).ptr);
    try cargv.append(gpa, null);

    var fa: c.posix_spawn_file_actions_t = undefined;
    var attr: c.posix_spawnattr_t = undefined;
    _ = c.posix_spawn_file_actions_init(&fa);
    defer _ = c.posix_spawn_file_actions_destroy(&fa);
    _ = c.posix_spawnattr_init(&attr);
    defer _ = c.posix_spawnattr_destroy(&attr);
    // Only the descriptors set here reach the child: another thread's pipes never leak into it.
    _ = c.posix_spawnattr_setflags(&attr, spawn_flags);
    const stdin_z = try util.z(opts.stdin_path orelse "/dev/null");
    _ = c.posix_spawn_file_actions_addopen(&fa, 0, stdin_z.ptr, c.O_RDONLY, 0);
    var po: [2]c_int = .{ -1, -1 };
    var pe: [2]c_int = .{ -1, -1 };
    if (opts.capture) {
        if (pipeCloexec(&po) != 0 or pipeCloexec(&pe) != 0) return error.Pipe;
        _ = c.posix_spawn_file_actions_adddup2(&fa, po[1], 1);
        _ = c.posix_spawn_file_actions_adddup2(&fa, if (opts.merge_stderr) po[1] else pe[1], 2);
    } else {
        _ = c.posix_spawn_file_actions_addopen(&fa, 1, "/dev/null", c.O_WRONLY, 0);
        _ = c.posix_spawn_file_actions_addopen(&fa, 2, "/dev/null", c.O_WRONLY, 0);
    }
    const envp = if (opts.env_set.len > 0) try buildEnv(opts.env_set) else environ;
    var pid: c.pid_t = 0;
    const rc = c.posix_spawnp(&pid, cargv.items[0].?, &fa, &attr, @ptrCast(cargv.items.ptr), @ptrCast(envp));
    if (opts.capture) {
        _ = c.close(po[1]);
        _ = c.close(pe[1]);
    }
    if (rc != 0) {
        if (opts.capture) {
            _ = c.close(po[0]);
            _ = c.close(pe[0]);
        }
        std.debug.print("cannot run {s}: error {d}\n", .{ argv[0], rc });
        return error.Spawn;
    }
    var out: std.ArrayList(u8) = .empty;
    var err: std.ArrayList(u8) = .empty;
    var timed_out = false;
    if (opts.capture) {
        const deadline: ?f64 = if (opts.timeout_s) |t| now() + t else null;
        var fds = [2]c.struct_pollfd{ .{ .fd = po[0], .events = c.POLLIN, .revents = 0 }, .{ .fd = pe[0], .events = c.POLLIN, .revents = 0 } };
        var open: usize = 2;
        var buf: [65536]u8 = undefined;
        while (open > 0) {
            var wait_ms: c_int = -1;
            if (deadline) |d| {
                const left = d - now();
                if (left <= 0) {
                    timed_out = true;
                    break;
                }
                wait_ms = @intFromFloat(@min(left * 1000 + 1, 60_000));
            }
            const n = c.poll(&fds, 2, wait_ms);
            if (n < 0) {
                if (errno() == c.EINTR) continue;
                return error.Poll;
            }
            for (&fds, 0..) |*f, k| {
                if (f.fd < 0 or f.revents == 0) continue;
                const got = c.read(f.fd, &buf, buf.len);
                if (got <= 0) {
                    if (got < 0 and errno() == c.EINTR) continue;
                    _ = c.close(f.fd);
                    f.fd = -1;
                    open -= 1;
                } else try (if (k == 0) &out else &err).appendSlice(gpa, buf[0..@intCast(got)]);
            }
        }
        if (timed_out) {
            _ = c.kill(pid, c.SIGKILL);
            for (fds) |f| if (f.fd >= 0) {
                _ = c.close(f.fd);
            };
        }
    }
    var status: c_int = 0;
    while (c.waitpid(pid, &status, 0) < 0) {
        if (errno() != c.EINTR) break;
    }
    const st: u32 = @bitCast(status);
    const code: i32 = if (st & 0x7f == 0) @intCast((st >> 8) & 0xff) else -@as(i32, @intCast(st & 0x7f));
    return .{ .out = out.items, .err = err.items, .status = code, .timed_out = timed_out };
}

/// A long-lived child talking lines over its stdin/stdout (stderr inherited).
pub const Pipe = struct {
    pid: c.pid_t,
    in: c_int,
    out: c_int,
    buf: std.ArrayList(u8) = .empty,
    pos: usize = 0,

    pub fn spawn(argv: []const []const u8) !Pipe {
        var cargv: std.ArrayList(?[*:0]const u8) = .empty;
        for (argv) |a| try cargv.append(gpa, (try util.z(a)).ptr);
        try cargv.append(gpa, null);
        var fa: c.posix_spawn_file_actions_t = undefined;
        var attr: c.posix_spawnattr_t = undefined;
        _ = c.posix_spawn_file_actions_init(&fa);
        defer _ = c.posix_spawn_file_actions_destroy(&fa);
        _ = c.posix_spawnattr_init(&attr);
        defer _ = c.posix_spawnattr_destroy(&attr);
        _ = c.posix_spawnattr_setflags(&attr, spawn_flags);
        var pi: [2]c_int = undefined;
        var po: [2]c_int = undefined;
        if (pipeCloexec(&pi) != 0 or pipeCloexec(&po) != 0) return error.Pipe;
        _ = c.posix_spawn_file_actions_adddup2(&fa, pi[0], 0);
        _ = c.posix_spawn_file_actions_adddup2(&fa, po[1], 1);
        _ = c.posix_spawn_file_actions_adddup2(&fa, 2, 2);
        var pid: c.pid_t = 0;
        const rc = c.posix_spawnp(&pid, cargv.items[0].?, &fa, &attr, @ptrCast(cargv.items.ptr), @ptrCast(environ));
        _ = c.close(pi[0]);
        _ = c.close(po[1]);
        if (rc != 0) {
            std.debug.print("cannot run {s}: error {d}\n", .{ argv[0], rc });
            return error.Spawn;
        }
        _ = c.fcntl(pi[1], c.F_SETFD, c.FD_CLOEXEC);
        _ = c.fcntl(po[0], c.F_SETFD, c.FD_CLOEXEC);
        return .{ .pid = pid, .in = pi[1], .out = po[0] };
    }

    pub fn write(p: *Pipe, data: []const u8) !void {
        var rest = data;
        while (rest.len > 0) {
            const n = c.write(p.in, rest.ptr, rest.len);
            if (n < 0) {
                if (errno() == c.EINTR) continue;
                return error.Write;
            }
            rest = rest[@intCast(n)..];
        }
    }

    /// The next line with its '\n', or "" at end of file (file.readline()).
    pub fn readLine(p: *Pipe) ![]const u8 {
        while (true) {
            if (std.mem.indexOfScalarPos(u8, p.buf.items, p.pos, '\n')) |e| {
                const line = try gpa.dupe(u8, p.buf.items[p.pos .. e + 1]);
                p.pos = e + 1;
                if (p.pos > 1 << 20) {
                    const rest = p.buf.items[p.pos..];
                    std.mem.copyForwards(u8, p.buf.items[0..rest.len], rest);
                    p.buf.shrinkRetainingCapacity(rest.len);
                    p.pos = 0;
                }
                return line;
            }
            var buf: [65536]u8 = undefined;
            const n = c.read(p.out, &buf, buf.len);
            if (n < 0) {
                if (errno() == c.EINTR) continue;
                return error.Read;
            }
            if (n == 0) {
                const line = try gpa.dupe(u8, p.buf.items[p.pos..]);
                p.pos = p.buf.items.len;
                return line;
            }
            try p.buf.appendSlice(gpa, buf[0..@intCast(n)]);
        }
    }

    pub fn closeIn(p: *Pipe) void {
        if (p.in >= 0) _ = c.close(p.in);
        p.in = -1;
    }

    pub fn wait(p: *Pipe) i32 {
        var status: c_int = 0;
        while (c.waitpid(p.pid, &status, 0) < 0) {
            if (errno() != c.EINTR) return -1;
        }
        const st: u32 = @bitCast(status);
        return if (st & 0x7f == 0) @intCast((st >> 8) & 0xff) else -@as(i32, @intCast(st & 0x7f));
    }
};

pub fn cpuCount() usize {
    const n = c.sysconf(c._SC_NPROCESSORS_ONLN);
    return if (n > 0) @intCast(n) else 4;
}
