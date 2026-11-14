//! Native Windows host backend for the oracle. Paths exposed to the rest of the
//! oracle use forward slashes, as the generated-tree helpers expect.
const std = @import("std");
const c = @import("c");
const util = @import("util.zig");
const common = @import("proc_posix.zig");
const gpa = util.gpa;

pub const isSpace = common.isSpace;
pub const strip = common.strip;
pub const isWord = common.isWord;
pub const lower = common.lower;
pub const universalNewlines = common.universalNewlines;
pub const splitlines = common.splitlines;
pub const latin1ToUtf8 = common.latin1ToUtf8;
pub const lessThan = common.lessThan;
pub const sortStrings = common.sortStrings;

fn wide(path: []const u8) ![:0]u16 {
    return std.unicode.wtf8ToWtf16LeAllocZ(gpa, path);
}

fn forward(path: []const u8) ![]u8 {
    const out = try gpa.dupe(u8, path);
    for (out) |*ch| if (ch.* == '\\') {
        ch.* = '/';
    };
    return out;
}

fn handleOk(handle: c.HANDLE) bool {
    return handle != null and handle != c.INVALID_HANDLE_VALUE;
}

pub const Mutex = struct {
    m: c.CRITICAL_SECTION = undefined,
    pub fn init(self: *Mutex) void { c.InitializeCriticalSection(&self.m); }
    pub fn lock(self: *Mutex) void { c.EnterCriticalSection(&self.m); }
    pub fn unlock(self: *Mutex) void { c.LeaveCriticalSection(&self.m); }
};

/// The CRT descriptor owns the underlying Win32 handle, including its lock.
pub fn flockPath(path: []const u8) !c_int {
    const p = try wide(path);
    const h = c.CreateFileW(p.ptr, c.GENERIC_READ | c.GENERIC_WRITE,
        c.FILE_SHARE_READ | c.FILE_SHARE_WRITE, null, c.OPEN_ALWAYS,
        c.FILE_ATTRIBUTE_NORMAL, null);
    if (!handleOk(h)) return error.Open;
    var overlap: c.OVERLAPPED = std.mem.zeroes(c.OVERLAPPED);
    if (c.LockFileEx(h, c.LOCKFILE_EXCLUSIVE_LOCK, 0, 1, 0, &overlap) == 0) {
        _ = c.CloseHandle(h);
        return error.Lock;
    }
    const fd = c._open_osfhandle(@intCast(@intFromPtr(h.?)), c._O_BINARY);
    if (fd < 0) {
        _ = c.CloseHandle(h);
        return error.Open;
    }
    return fd;
}

pub fn closeFd(fd: c_int) void { _ = c._close(fd); }

fn filetimeTicks(t: c.FILETIME) u64 {
    return (@as(u64, t.dwHighDateTime) << 32) | t.dwLowDateTime;
}

pub fn now() f64 {
    var ft: c.FILETIME = undefined;
    c.GetSystemTimeAsFileTime(&ft);
    return @as(f64, @floatFromInt(filetimeTicks(ft))) / 10_000_000 - 11_644_473_600;
}

pub fn remove(path: []const u8) void {
    const p = wide(path) catch return;
    _ = c.DeleteFileW(p.ptr);
}

pub fn rename(from: []const u8, to: []const u8) !void {
    const a = try wide(from);
    const b = try wide(to);
    if (c.MoveFileExW(a.ptr, b.ptr, c.MOVEFILE_REPLACE_EXISTING) == 0) return error.Rename;
}

fn attributes(path: []const u8) ?c.DWORD {
    const p = wide(path) catch return null;
    const attr = c.GetFileAttributesW(p.ptr);
    return if (attr == c.INVALID_FILE_ATTRIBUTES) null else attr;
}

pub fn isDir(path: []const u8) bool {
    return if (attributes(path)) |a| (a & c.FILE_ATTRIBUTE_DIRECTORY) != 0 else false;
}

pub fn isFile(path: []const u8) bool {
    return if (attributes(path)) |a| (a & c.FILE_ATTRIBUTE_DIRECTORY) == 0 else false;
}

pub const Stat = struct { ino: u64, size: i64, mtime_ns: i128 };

pub fn stat(path: []const u8) ?Stat {
    const p = wide(path) catch return null;
    const h = c.CreateFileW(p.ptr, 0, c.FILE_SHARE_READ | c.FILE_SHARE_WRITE | c.FILE_SHARE_DELETE,
        null, c.OPEN_EXISTING, c.FILE_FLAG_BACKUP_SEMANTICS, null);
    if (!handleOk(h)) return null;
    defer _ = c.CloseHandle(h);
    var info: c.BY_HANDLE_FILE_INFORMATION = undefined;
    if (c.GetFileInformationByHandle(h, &info) == 0) return null;
    const file_id = (@as(u64, info.nFileIndexHigh) << 32) | info.nFileIndexLow;
    const size = (@as(u64, info.nFileSizeHigh) << 32) | info.nFileSizeLow;
    const ticks = filetimeTicks(info.ftLastWriteTime);
    return .{
        .ino = file_id ^ (@as(u64, info.dwVolumeSerialNumber) << 32),
        .size = @intCast(size),
        .mtime_ns = (@as(i128, ticks) - 116_444_736_000_000_000) * 100,
    };
}

pub fn fileSize(path: []const u8) u64 {
    return if (stat(path)) |s| @intCast(s.size) else 0;
}

pub fn writeAtomic(path: []const u8, data: []const u8) !void {
    const tmp = try std.fmt.allocPrint(gpa, "{s}.{d}.{d}.tmp", .{
        path, c.GetCurrentProcessId(), c.GetCurrentThreadId(),
    });
    try util.writeFile(tmp, data);
    try rename(tmp, path);
}

pub fn copyAtomic(src: []const u8, dst: []const u8) !void {
    try writeAtomic(dst, try util.readFile(src));
}

pub fn listDir(path: []const u8) ![]const []const u8 {
    var out: std.ArrayList([]const u8) = .empty;
    const pattern = try wide(try util.join(path, "*"));
    var data: c.WIN32_FIND_DATAW = undefined;
    const h = c.FindFirstFileW(pattern.ptr, &data);
    if (!handleOk(h)) return out.items;
    defer _ = c.FindClose(h);
    while (true) {
        const name_w = std.mem.sliceTo(@as([*:0]const u16, @ptrCast(&data.cFileName)), 0);
        const name = try std.unicode.wtf16LeToWtf8Alloc(gpa, name_w);
        if (!std.mem.eql(u8, name, ".") and !std.mem.eql(u8, name, "..")) try out.append(gpa, name);
        if (c.FindNextFileW(h, &data) == 0) break;
    }
    return out.items;
}

pub const WalkDir = struct { path: []const u8, dirs: [][]const u8, files: [][]const u8 };

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
    return if (attributes(path)) |a| (a & c.FILE_ATTRIBUTE_REPARSE_POINT) != 0 else false;
}

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

fn drivePrefix(p: []const u8) usize {
    return if (p.len >= 2 and std.ascii.isAlphabetic(p[0]) and p[1] == ':') 2 else 0;
}

/// Windows normpath rendered with '/' separators for the oracle's path code.
pub fn normpath(path: []const u8) ![]const u8 {
    if (path.len == 0) return ".";
    const p = try forward(path);
    const drive = drivePrefix(p);
    const unc = drive == 0 and std.mem.startsWith(u8, p, "//");
    const abs = unc or (p.len > drive and p[drive] == '/');
    var comps: std.ArrayList([]const u8) = .empty;
    var it = std.mem.splitScalar(u8, p[drive..], '/');
    while (it.next()) |comp| {
        if (comp.len == 0 or std.mem.eql(u8, comp, ".")) continue;
        if (std.mem.eql(u8, comp, "..")) {
            const min: usize = if (unc) @min(comps.items.len, 2) else 0;
            if (comps.items.len > min and !std.mem.eql(u8, comps.items[comps.items.len - 1], "..")) {
                _ = comps.pop();
                continue;
            }
            if (abs) continue;
        }
        try comps.append(gpa, comp);
    }
    const body = try std.mem.join(gpa, "/", comps.items);
    const prefix = if (unc) "//" else if (abs) "/" else "";
    const result = try std.mem.concat(gpa, u8, &.{ p[0..drive], prefix, body });
    return if (result.len == 0) "." else result;
}

pub fn joinNorm(a: []const u8, b: []const u8) ![]const u8 {
    if (drivePrefix(b) != 0 or (b.len > 0 and (b[0] == '/' or b[0] == '\\'))) return normpath(b);
    return normpath(try util.join(a, b));
}

pub fn relpath(path: []const u8, start: []const u8) ![]const u8 {
    const p = try normpath(path);
    const s = try normpath(start);
    const pd = drivePrefix(p);
    const sd = drivePrefix(s);
    if (pd != sd or !std.ascii.eqlIgnoreCase(p[0..pd], s[0..sd])) return error.DifferentDrive;
    var pc: std.ArrayList([]const u8) = .empty;
    var sc: std.ArrayList([]const u8) = .empty;
    var it = std.mem.tokenizeScalar(u8, p[pd..], '/');
    while (it.next()) |x| try pc.append(gpa, x);
    it = std.mem.tokenizeScalar(u8, s[sd..], '/');
    while (it.next()) |x| try sc.append(gpa, x);
    var i: usize = 0;
    while (i < pc.items.len and i < sc.items.len and std.ascii.eqlIgnoreCase(pc.items[i], sc.items[i])) i += 1;
    var parts: std.ArrayList([]const u8) = .empty;
    for (i..sc.items.len) |_| try parts.append(gpa, "..");
    try parts.appendSlice(gpa, pc.items[i..]);
    return if (parts.items.len == 0) "." else std.mem.join(gpa, "/", parts.items);
}

pub fn realpath(path: []const u8) ![]const u8 {
    const w = try wide(path);
    var buf: [32768]u16 = undefined;
    const n = c.GetFullPathNameW(w.ptr, @intCast(buf.len), &buf, null);
    if (n == 0 or n >= buf.len) return error.RealPath;
    const h = c.CreateFileW(w.ptr, 0, c.FILE_SHARE_READ | c.FILE_SHARE_WRITE | c.FILE_SHARE_DELETE,
        null, c.OPEN_EXISTING, c.FILE_FLAG_BACKUP_SEMANTICS, null);
    if (handleOk(h)) {
        defer _ = c.CloseHandle(h);
        var final: [32768]u16 = undefined;
        const count = c.GetFinalPathNameByHandleW(h, &final, @intCast(final.len), 0);
        if (count > 0 and count < final.len) {
            const full = try std.unicode.wtf16LeToWtf8Alloc(gpa, final[0..count]);
            if (std.mem.startsWith(u8, full, "\\\\?\\UNC\\")) return forward(try std.mem.concat(gpa, u8, &.{ "//", full[8..] }));
            if (std.mem.startsWith(u8, full, "\\\\?\\")) return forward(full[4..]);
            return forward(full);
        }
    }
    return forward(try std.unicode.wtf16LeToWtf8Alloc(gpa, buf[0..n]));
}

pub const Output = struct { out: []const u8, err: []const u8, status: i32, timed_out: bool = false };
pub const RunOptions = struct {
    capture: bool = true,
    timeout_s: ?f64 = null,
    env_set: []const []const u8 = &.{},
    merge_stderr: bool = false,
    stdin_path: ?[]const u8 = null,
};

const WinPipe = struct {
    read: c.HANDLE = null,
    write: c.HANDLE = null,

    fn close(self: *WinPipe) void {
        if (handleOk(self.read)) _ = c.CloseHandle(self.read);
        if (handleOk(self.write)) _ = c.CloseHandle(self.write);
        self.* = .{};
    }
};

const Drain = enum { open, closed, read };

/// PeekNamedPipe works on CreatePipe's anonymous read handle. One thread owns
/// each read handle, and only the available bytes are requested from ReadFile.
fn drainPipe(handle: c.HANDLE, out: *std.ArrayList(u8)) !Drain {
    var available: c.DWORD = 0;
    if (c.PeekNamedPipe(handle, null, 0, null, &available, null) == 0) {
        const e = c.GetLastError();
        if (e == c.ERROR_BROKEN_PIPE or e == c.ERROR_PIPE_NOT_CONNECTED) return .closed;
        return error.Peek;
    }
    if (available == 0) return .open;
    var buf: [65536]u8 = undefined;
    var n: c.DWORD = 0;
    if (c.ReadFile(handle, &buf, @intCast(@min(available, buf.len)), &n, null) == 0) {
        const e = c.GetLastError();
        if (e == c.ERROR_BROKEN_PIPE or e == c.ERROR_PIPE_NOT_CONNECTED) return .closed;
        return error.Read;
    }
    if (n > 0) try out.appendSlice(gpa, buf[0..n]);
    return .read;
}

/// Anonymous pipes are synchronous handles, matching ReadFile/WriteFile below.
/// The parent end alone has inheritance disabled; Zig passes child ends through
/// its explicit process handle list.
fn makePipe(parent_reads: bool) !WinPipe {
    var p: WinPipe = .{};
    var security: c.SECURITY_ATTRIBUTES = .{
        .nLength = @sizeOf(c.SECURITY_ATTRIBUTES),
        .lpSecurityDescriptor = null,
        .bInheritHandle = 1,
    };
    if (c.CreatePipe(&p.read, &p.write, &security, 0) == 0) return error.Pipe;
    const parent = if (parent_reads) p.read else p.write;
    if (c.SetHandleInformation(parent, c.HANDLE_FLAG_INHERIT, 0) == 0) {
        p.close();
        return error.Pipe;
    }
    return p;
}

fn commandLine(argv: []const []const u8) ![:0]u16 {
    if (argv.len == 0 or argv[0].len == 0 or std.mem.indexOfScalar(u8, argv[0], '"') != null)
        return error.InvalidArgv;
    var text: std.ArrayList(u8) = .empty;
    for (argv, 0..) |arg, index| {
        if (index > 0) try text.append(gpa, ' ');
        var quote = arg.len == 0;
        for (arg) |ch| if (ch <= ' ' or ch == '"') {
            quote = true;
            break;
        };
        if (!quote) {
            try text.appendSlice(gpa, arg);
            continue;
        }
        try text.append(gpa, '"');
        if (index == 0) {
            // Windows argv[0] has different backslash rules and cannot contain
            // quotes (quotes are invalid in an executable path).
            try text.appendSlice(gpa, arg);
        } else {
            var slashes: usize = 0;
            for (arg) |ch| switch (ch) {
                '\\' => slashes += 1,
                '"' => {
                    try text.appendNTimes(gpa, '\\', slashes * 2 + 1);
                    try text.append(gpa, '"');
                    slashes = 0;
                },
                else => {
                    try text.appendNTimes(gpa, '\\', slashes);
                    try text.append(gpa, ch);
                    slashes = 0;
                },
            };
            try text.appendNTimes(gpa, '\\', slashes * 2);
        }
        try text.append(gpa, '"');
    }
    return std.unicode.wtf8ToWtf16LeAllocZ(gpa, text.items);
}

fn inheritedFile(path: []const u8, access: c.DWORD) !c.HANDLE {
    const w = try wide(path);
    var security: c.SECURITY_ATTRIBUTES = .{
        .nLength = @sizeOf(c.SECURITY_ATTRIBUTES),
        .lpSecurityDescriptor = null,
        .bInheritHandle = 1,
    };
    const h = c.CreateFileW(w.ptr, access, c.FILE_SHARE_READ | c.FILE_SHARE_WRITE | c.FILE_SHARE_DELETE,
        &security, c.OPEN_EXISTING, c.FILE_ATTRIBUTE_NORMAL, null);
    if (!handleOk(h)) return error.Open;
    return h;
}

fn inheritedStderr() !c.HANDLE {
    const source = c.GetStdHandle(c.STD_ERROR_HANDLE);
    if (!handleOk(source)) return inheritedFile("NUL", c.GENERIC_WRITE);
    const current = c.GetCurrentProcess();
    var dup: c.HANDLE = null;
    if (c.DuplicateHandle(current, source, current, &dup, 0, 1, c.DUPLICATE_SAME_ACCESS) == 0)
        return error.Duplicate;
    return dup;
}

/// Spawn with only the three specified stdio handles inheritable. The explicit
/// handle list prevents concurrent compiler jobs from keeping each other's
/// stdout writers open and delaying EOF.
fn spawnNative(argv: []const []const u8, stdin_h: c.HANDLE, stdout_h: c.HANDLE,
    stderr_h: c.HANDLE, env_set: []const []const u8) !c.HANDLE {
    const cmd = try commandLine(argv);
    var env_block: ?std.process.Environ.WindowsBlock = null;
    defer if (env_block) |block| block.deinit(std.heap.c_allocator);
    if (env_set.len > 0) {
        var map = try std.process.Environ.createMap(.{ .block = .global }, std.heap.c_allocator);
        defer map.deinit();
        for (env_set) |kv| {
            const eq = std.mem.indexOfScalar(u8, kv, '=') orelse return error.InvalidEnvironment;
            try map.put(kv[0..eq], kv[eq + 1 ..]);
        }
        env_block = try map.createWindowsBlock(std.heap.c_allocator, .{});
    }

    var handles: [3]c.HANDLE = undefined;
    var count: usize = 0;
    for ([_]c.HANDLE{ stdin_h, stdout_h, stderr_h }) |h| {
        if (!handleOk(h)) return error.InvalidHandle;
        var seen = false;
        for (handles[0..count]) |old| if (old == h) {
            seen = true;
        };
        if (!seen) {
            handles[count] = h;
            count += 1;
        }
    }
    var attr_bytes: c.SIZE_T = 0;
    _ = c.InitializeProcThreadAttributeList(null, 1, 0, &attr_bytes);
    if (attr_bytes == 0) return error.AttributeList;
    const attr_mem = c.malloc(attr_bytes) orelse return error.OutOfMemory;
    defer c.free(attr_mem);
    const attr_list: c.LPPROC_THREAD_ATTRIBUTE_LIST = @ptrCast(@alignCast(attr_mem));
    if (c.InitializeProcThreadAttributeList(attr_list, 1, 0, &attr_bytes) == 0)
        return error.AttributeList;
    defer c.DeleteProcThreadAttributeList(attr_list);
    if (c.UpdateProcThreadAttribute(attr_list, 0, c.PROC_THREAD_ATTRIBUTE_HANDLE_LIST,
        @ptrCast(&handles), count * @sizeOf(c.HANDLE), null, null) == 0)
        return error.AttributeList;

    var startup: c.STARTUPINFOEXW = std.mem.zeroes(c.STARTUPINFOEXW);
    startup.StartupInfo.cb = @sizeOf(c.STARTUPINFOEXW);
    startup.StartupInfo.dwFlags = c.STARTF_USESTDHANDLES;
    startup.StartupInfo.hStdInput = stdin_h;
    startup.StartupInfo.hStdOutput = stdout_h;
    startup.StartupInfo.hStdError = stderr_h;
    startup.lpAttributeList = attr_list;
    var process: c.PROCESS_INFORMATION = std.mem.zeroes(c.PROCESS_INFORMATION);
    const env_ptr: c.LPVOID = if (env_block) |block| @ptrCast(@constCast(block.slice.ptr)) else null;
    const flags: c.DWORD = @as(c.DWORD, c.EXTENDED_STARTUPINFO_PRESENT | c.CREATE_NO_WINDOW) |
        (if (env_block == null) @as(c.DWORD, 0) else @as(c.DWORD, c.CREATE_UNICODE_ENVIRONMENT));
    if (c.CreateProcessW(null, cmd.ptr, null, null, 1, flags, env_ptr, null,
        &startup.StartupInfo, &process) == 0) {
        std.debug.print("cannot run {s}: Win32 error {d}\n", .{ argv[0], c.GetLastError() });
        return error.Spawn;
    }
    _ = c.CloseHandle(process.hThread);
    return process.hProcess;
}

fn exitCode(process: c.HANDLE) i32 {
    var code: c.DWORD = 0;
    if (c.GetExitCodeProcess(process, &code) == 0) return -1;
    return @bitCast(code);
}

/// Error-path cleanup must not leave a compiler or translator running after
/// its parent drops the process handle. Keep the reap bounded even if the OS
/// rejects termination for a process that has already exited.
fn terminateAndReap(process: c.HANDLE) void {
    if (!handleOk(process)) return;
    if (c.WaitForSingleObject(process, 0) == c.WAIT_TIMEOUT) _ = c.TerminateProcess(process, 1);
    _ = c.WaitForSingleObject(process, 5000);
}

fn waitForChild(process: c.HANDLE, timeout_s: ?f64) !bool {
    const milliseconds: c.DWORD = if (timeout_s) |seconds|
        @intFromFloat(@min(@max(seconds * 1000, 0), 4_294_967_294))
    else c.INFINITE;
    const waited = c.WaitForSingleObject(process, milliseconds);
    if (waited == c.WAIT_OBJECT_0) return false;
    if (waited != c.WAIT_TIMEOUT) return error.Wait;
    if (c.TerminateProcess(process, 1) == 0) return error.Terminate;
    if (c.WaitForSingleObject(process, c.INFINITE) != c.WAIT_OBJECT_0) return error.Wait;
    return true;
}

pub fn run(argv: []const []const u8, opts: RunOptions) !Output {
    const stdin_h = try inheritedFile(opts.stdin_path orelse "NUL", c.GENERIC_READ);
    defer _ = c.CloseHandle(stdin_h);
    var out_pipe = if (opts.capture) try makePipe(true) else WinPipe{};
    defer out_pipe.close();
    var err_pipe = if (opts.capture and !opts.merge_stderr) try makePipe(true) else WinPipe{};
    defer err_pipe.close();
    const null_out = if (opts.capture) @as(c.HANDLE, null) else try inheritedFile("NUL", c.GENERIC_WRITE);
    defer {
        if (handleOk(null_out)) _ = c.CloseHandle(null_out);
    }
    const stdout_h = if (opts.capture) out_pipe.write else null_out;
    const stderr_h = if (opts.capture) (if (opts.merge_stderr) out_pipe.write else err_pipe.write) else null_out;
    const process = try spawnNative(argv, stdin_h, stdout_h, stderr_h, opts.env_set);
    defer _ = c.CloseHandle(process);
    errdefer terminateAndReap(process);
    if (handleOk(out_pipe.write)) _ = c.CloseHandle(out_pipe.write);
    out_pipe.write = null;
    if (handleOk(err_pipe.write)) _ = c.CloseHandle(err_pipe.write);
    err_pipe.write = null;
    if (!opts.capture) {
        const timed_out = try waitForChild(process, opts.timeout_s);
        return .{ .out = "", .err = "", .status = exitCode(process), .timed_out = timed_out };
    }

    var out: std.ArrayList(u8) = .empty;
    var err: std.ArrayList(u8) = .empty;
    var out_open = true;
    var err_open = !opts.merge_stderr;
    var timed_out = false;
    const start_ms = c.GetTickCount64();
    var exited_ms: ?u64 = null;
    const timeout_ms: ?u64 = if (opts.timeout_s) |seconds|
        @intFromFloat(@min(@max(seconds * 1000, 0), 4_294_967_294))
    else null;
    while (true) {
        var read_any = false;
        if (out_open) switch (try drainPipe(out_pipe.read, &out)) {
            .closed => out_open = false,
            .read => read_any = true,
            .open => {},
        };
        if (err_open) switch (try drainPipe(err_pipe.read, &err)) {
            .closed => err_open = false,
            .read => read_any = true,
            .open => {},
        };
        const state = c.WaitForSingleObject(process, 0);
        if (state == c.WAIT_OBJECT_0) {
            if (exited_ms == null) exited_ms = c.GetTickCount64();
        } else if (state != c.WAIT_TIMEOUT) return error.Wait;
        if (exited_ms != null and !out_open and !err_open) break;
        const elapsed = c.GetTickCount64() - start_ms;
        if (!timed_out and timeout_ms != null and elapsed >= timeout_ms.?) {
            timed_out = true;
            if (state != c.WAIT_OBJECT_0) {
                if (c.TerminateProcess(process, 1) == 0) return error.Terminate;
                if (c.WaitForSingleObject(process, c.INFINITE) != c.WAIT_OBJECT_0) return error.Wait;
            }
            exited_ms = c.GetTickCount64();
        }
        // A descendant can retain a writer after the direct child exits. Mark
        // the output incomplete rather than hanging or reporting a clean run.
        if (exited_ms) |t| if (c.GetTickCount64() - t >= 2000) {
            timed_out = true;
            break;
        };
        if (!read_any) c.Sleep(5);
    }
    out_pipe.close();
    err_pipe.close();
    return .{ .out = out.items, .err = err.items, .status = exitCode(process), .timed_out = timed_out };
}

pub const Pipe = struct {
    process: c.HANDLE,
    in: c.HANDLE,
    out: c.HANDLE,
    buf: std.ArrayList(u8) = .empty,
    pos: usize = 0,

    pub fn spawn(argv: []const []const u8) !Pipe {
        var stdin_pipe = try makePipe(false);
        errdefer stdin_pipe.close();
        var stdout_pipe = try makePipe(true);
        errdefer stdout_pipe.close();
        const stderr_h = try inheritedStderr();
        defer _ = c.CloseHandle(stderr_h);
        const process = try spawnNative(argv, stdin_pipe.read, stdout_pipe.write, stderr_h, &.{});
        errdefer {
            terminateAndReap(process);
            _ = c.CloseHandle(process);
        }
        _ = c.CloseHandle(stdin_pipe.read);
        stdin_pipe.read = null;
        _ = c.CloseHandle(stdout_pipe.write);
        stdout_pipe.write = null;
        return .{ .process = process, .in = stdin_pipe.write, .out = stdout_pipe.read };
    }

    pub fn write(self: *Pipe, data: []const u8) !void {
        var rest = data;
        if (!handleOk(self.in)) return error.Closed;
        while (rest.len > 0) {
            var n: c.DWORD = 0;
            const chunk: c.DWORD = @intCast(@min(rest.len, 65536));
            if (c.WriteFile(self.in, rest.ptr, chunk, &n, null) == 0 or n == 0) return error.Write;
            rest = rest[n..];
        }
    }

    pub fn readLine(self: *Pipe) ![]const u8 {
        while (true) {
            if (std.mem.indexOfScalarPos(u8, self.buf.items, self.pos, '\n')) |e| {
                const line = try gpa.dupe(u8, self.buf.items[self.pos .. e + 1]);
                self.pos = e + 1;
                if (self.pos > 1 << 20) {
                    const rest = self.buf.items[self.pos..];
                    std.mem.copyForwards(u8, self.buf.items[0..rest.len], rest);
                    self.buf.shrinkRetainingCapacity(rest.len);
                    self.pos = 0;
                }
                return line;
            }
            var buf: [65536]u8 = undefined;
            var n: c.DWORD = 0;
            if (!handleOk(self.out)) return error.Closed;
            if (c.ReadFile(self.out, &buf, buf.len, &n, null) == 0 or n == 0) {
                const line = try gpa.dupe(u8, self.buf.items[self.pos..]);
                self.pos = self.buf.items.len;
                return line;
            }
            try self.buf.appendSlice(gpa, buf[0..n]);
        }
    }

    pub fn closeIn(self: *Pipe) void {
        if (handleOk(self.in)) _ = c.CloseHandle(self.in);
        self.in = null;
    }

    pub fn wait(self: *Pipe) i32 {
        defer {
            self.closeIn();
            if (handleOk(self.out)) _ = c.CloseHandle(self.out);
            self.out = null;
            if (handleOk(self.process)) _ = c.CloseHandle(self.process);
            self.process = null;
        }
        _ = waitForChild(self.process, null) catch {
            terminateAndReap(self.process);
            return -1;
        };
        return exitCode(self.process);
    }
};

pub fn cpuCount() usize { return std.Thread.getCpuCount() catch 4; }

test "Windows paths and file metadata" {
    try std.testing.expectEqualStrings("C:/bar", try normpath("C:\\foo\\..\\bar"));
    try std.testing.expectEqualStrings("//server/share", try normpath("\\\\server\\share\\dir\\.."));
    try std.testing.expectEqualStrings("../bar", try relpath("C:/one/bar", "c:/one/two"));
    const here = try realpath(".");
    try std.testing.expect(std.mem.indexOf(u8, here, ":/") != null);
    try std.testing.expect(isDir(here));
    const build_file = try util.join(here, "build.zig");
    try std.testing.expect(isFile(build_file));
    try std.testing.expect(stat(build_file) != null);
    var saw_build = false;
    for (try listDir(here)) |name| if (std.mem.eql(u8, name, "build.zig")) {
        saw_build = true;
    };
    try std.testing.expect(saw_build);
}

test "Windows argv quoting" {
    const line = try commandLine(&.{ "C:\\Program Files\\tool.exe", "two words", "x\"y", "tail \\" });
    const text = try std.unicode.wtf16LeToWtf8Alloc(gpa, line);
    const expected = try std.mem.concat(gpa, u8, &.{ "\"C:\\Program Files\\tool.exe\" \"two words\" \"x\\\"y\" \"tail ", "\\\\", "\"" });
    try std.testing.expectEqualStrings(expected, text);
    try std.testing.expectError(error.InvalidArgv, commandLine(&.{"bad\"name.exe"}));
}

test "Windows captured child and merged stream" {
    const shell = util.getenv("ComSpec") orelse "C:/Windows/System32/cmd.exe";
    const captured = try run(&.{ shell, "/d", "/c", "echo proc-run-ok" }, .{});
    try std.testing.expectEqual(@as(i32, 0), captured.status);
    try std.testing.expect(std.mem.indexOf(u8, captured.out, "proc-run-ok") != null);
    const merged = try run(&.{ shell, "/d", "/c", "echo first & echo second 1>&2" }, .{ .merge_stderr = true });
    try std.testing.expectEqual(@as(i32, 0), merged.status);
    const first = std.mem.indexOf(u8, merged.out, "first") orelse return error.MissingStdout;
    const second = std.mem.indexOf(u8, merged.out, "second") orelse return error.MissingStderr;
    try std.testing.expect(first < second);
    try std.testing.expectEqual(@as(usize, 0), merged.err.len);

    const env = try run(&.{ shell, "/d", "/c", "echo %ORACLE_PORT_TEST%" },
        .{ .env_set = &.{"ORACLE_PORT_TEST=from-child"} });
    try std.testing.expectEqual(@as(i32, 0), env.status);
    try std.testing.expect(std.mem.indexOf(u8, env.out, "from-child") != null);

    const input = ".zig-cache/proc-windows-stdin.txt";
    try util.writeFile(input, "from-file\r\n");
    defer remove(input);
    const fed = try run(&.{ shell, "/d", "/c", "more" }, .{ .stdin_path = input });
    try std.testing.expectEqual(@as(i32, 0), fed.status);
    try std.testing.expect(std.mem.indexOf(u8, fed.out, "from-file") != null);
}

test "Windows child output larger than pipe buffer" {
    const shell = util.getenv("ComSpec") orelse "C:/Windows/System32/cmd.exe";
    const many = try run(&.{ shell, "/d", "/c", "for /L %i in (1,1,12000) do @echo 1234567890" },
        .{ .timeout_s = 20 });
    try std.testing.expectEqual(@as(i32, 0), many.status);
    try std.testing.expect(!many.timed_out);
    try std.testing.expect(many.out.len > 65536);
}

test "Windows descendant writer is bounded after direct child exits" {
    const shell = util.getenv("ComSpec") orelse "C:/Windows/System32/cmd.exe";
    const start_ms = c.GetTickCount64();
    const child = try run(&.{ shell, "/d", "/c", "start /b ping -n 4 127.0.0.1" },
        .{ .timeout_s = 6 });
    const elapsed = c.GetTickCount64() - start_ms;
    try std.testing.expect(child.timed_out);
    try std.testing.expect(elapsed >= 1900 and elapsed < 6000);
}

test "Windows long-lived child pipe and no-capture timeout" {
    const shell = util.getenv("ComSpec") orelse "C:/Windows/System32/cmd.exe";
    var pipe = try Pipe.spawn(&.{ shell, "/d", "/c", "more" });
    try pipe.write("pipe-hello\r\n");
    pipe.closeIn();
    const line = try pipe.readLine();
    try std.testing.expect(std.mem.indexOf(u8, line, "pipe-hello") != null);
    try std.testing.expectEqual(@as(i32, 0), pipe.wait());

    const root = util.getenv("SystemRoot") orelse "C:/Windows";
    const ping = try util.join(root, "System32/ping.exe");
    const slow = try run(&.{ ping, "-n", "4", "127.0.0.1" }, .{ .capture = false, .timeout_s = 0.1 });
    try std.testing.expect(slow.timed_out);
}

test "Windows error cleanup reaps a running child" {
    const root = util.getenv("SystemRoot") orelse "C:/Windows";
    const ping = try util.join(root, "System32/ping.exe");
    const stdin_h = try inheritedFile("NUL", c.GENERIC_READ);
    defer _ = c.CloseHandle(stdin_h);
    const stdout_h = try inheritedFile("NUL", c.GENERIC_WRITE);
    defer _ = c.CloseHandle(stdout_h);
    const process = try spawnNative(&.{ ping, "-n", "4", "127.0.0.1" }, stdin_h, stdout_h, stdout_h, &.{});
    defer _ = c.CloseHandle(process);
    defer terminateAndReap(process);
    try std.testing.expectEqual(@as(c.DWORD, c.WAIT_TIMEOUT), c.WaitForSingleObject(process, 0));
    terminateAndReap(process);
    try std.testing.expectEqual(@as(c.DWORD, c.WAIT_OBJECT_0), c.WaitForSingleObject(process, 0));
}
