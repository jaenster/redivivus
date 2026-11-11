//! Content-hash cache for per-translation-unit gcc compiles (recon/runs/tucache.py).
//!
//! Used by recon/runs/measure.sh (mode `syntax`: the strict error count of a -fsyntax-only
//! compile) and link.sh (mode `obj`: the object file of a -c compile). A hit is served only
//! when every input the compile could have read is provably the same as when the result was
//! stored: ccache's direct mode, extended to -fsyntax-only. The keys, the cache layout and the
//! manifests (Python's json.dumps text) are tucache.py's, so the two share one cache.
//!
//!   oracle-zig tucache lookup --mode M --tree T --list cpplist --flags "..." --rsp R \
//!                             --hits H --misses X [--objdir B]
//!   oracle-zig tucache store  --mode M --tree T --list misses --flags "..." --rsp R \
//!                             --depdir D [--candidates tsv --results tsv] [--objdir B] [--failed list]
//!   oracle-zig tucache prune  [--days N] [--force]
//!   oracle-zig tucache probe  exits 0: the subcommand exists (link.sh / measure.sh ask)
//!
//! Cache root: $D2_TU_CACHE, default ~/Library/Caches/d2-tucache; D2_TU_CACHE=off disables
//! every lookup and store. The compiler is $D2_CXX (default i686-w64-mingw32-g++).
const std = @import("std");
const c = @import("c");
const util = @import("util.zig");
const gpa = util.gpa;

const FORMAT = "tucache-2";
const HEADER_EXT = [_][]const u8{ ".h", ".hh", ".hpp", ".hxx", ".inc", ".inl", ".ipp", ".tcc", ".def", "" };
const ENV_KEYS = [_][:0]const u8{ "LANG", "LC_ALL", "LC_MESSAGES", "LC_CTYPE", "CPATH", "CPLUS_INCLUDE_PATH", "C_INCLUDE_PATH", "GCC_EXEC_PREFIX", "COMPILER_PATH", "LIBRARY_PATH", "SOURCE_DATE_EPOCH", "GCC_COMPARE_DEBUG", "DEPENDENCIES_OUTPUT", "SUNPRO_DEPENDENCIES" };
const MAX_ENTRIES = 4;

const Args = struct {
    cmd: []const u8 = "",
    mode: ?[]const u8 = null,
    tree: []const u8 = ".",
    list: ?[]const u8 = null,
    flags: []const u8 = "",
    rsp: ?[]const u8 = null,
    hits: ?[]const u8 = null,
    misses: ?[]const u8 = null,
    objdir: ?[]const u8 = null,
    depdir: ?[]const u8 = null,
    candidates: ?[]const u8 = null,
    results: ?[]const u8 = null,
    failed: ?[]const u8 = null,
    days: i64 = 14,
    force: bool = false,
};

fn usage() noreturn {
    std.debug.print("usage: oracle-zig tucache lookup|store|prune|probe [--mode syntax|obj] [--tree T] [--list L] [--flags F] [--rsp R] [--hits H] [--misses M] [--objdir D] [--depdir D] [--candidates C] [--results R] [--failed F] [--days N] [--force]\n", .{});
    std.process.exit(2);
}

fn parseArgs(argv: []const []const u8) !Args {
    var a = Args{};
    var i: usize = 0;
    while (i < argv.len) : (i += 1) {
        var name = argv[i];
        var value: ?[]const u8 = null;
        if (std.mem.startsWith(u8, name, "--")) {
            if (std.mem.indexOfScalar(u8, name, '=')) |eq| {
                value = name[eq + 1 ..];
                name = name[0..eq];
            }
            if (std.mem.eql(u8, name, "--force")) {
                a.force = true;
                continue;
            }
            if (value == null) {
                i += 1;
                if (i >= argv.len) usage();
                value = argv[i];
            }
            const v = value.?;
            if (std.mem.eql(u8, name, "--mode")) {
                if (!std.mem.eql(u8, v, "syntax") and !std.mem.eql(u8, v, "obj")) usage();
                a.mode = v;
            } else if (std.mem.eql(u8, name, "--tree")) a.tree = v else if (std.mem.eql(u8, name, "--list")) a.list = v else if (std.mem.eql(u8, name, "--flags")) a.flags = v else if (std.mem.eql(u8, name, "--rsp")) a.rsp = v else if (std.mem.eql(u8, name, "--hits")) a.hits = v else if (std.mem.eql(u8, name, "--misses")) a.misses = v else if (std.mem.eql(u8, name, "--objdir")) a.objdir = v else if (std.mem.eql(u8, name, "--depdir")) a.depdir = v else if (std.mem.eql(u8, name, "--candidates")) a.candidates = v else if (std.mem.eql(u8, name, "--results")) a.results = v else if (std.mem.eql(u8, name, "--failed")) a.failed = v else if (std.mem.eql(u8, name, "--days")) {
                a.days = std.fmt.parseInt(i64, util.strip(v), 10) catch usage();
            } else usage();
        } else {
            if (a.cmd.len != 0) usage();
            a.cmd = name;
        }
    }
    return a;
}

pub fn main(argv: []const []const u8) !void {
    const a = try parseArgs(argv);
    if (std.mem.eql(u8, a.cmd, "probe")) return;
    if (std.mem.eql(u8, a.cmd, "lookup")) return cmdLookup(a);
    if (std.mem.eql(u8, a.cmd, "store")) return cmdStore(a);
    if (std.mem.eql(u8, a.cmd, "prune")) return cmdPrune(a);
    usage();
}

fn req(v: ?[]const u8) []const u8 {
    return v orelse {
        std.debug.print("tucache: missing a required option\n", .{});
        std.process.exit(2);
    };
}

// ---------------------------------------------------------------- paths

fn cacheRoot() !?[]const u8 {
    if (util.getenv("D2_TU_CACHE")) |r| {
        if (std.mem.eql(u8, r, "off")) return null;
        return r;
    }
    return try util.join(util.getenv("HOME") orelse "", "Library/Caches/d2-tucache");
}

fn shaHex(data: []const u8) ![]u8 {
    var d: [32]u8 = undefined;
    std.crypto.hash.sha2.Sha256.hash(data, &d, .{});
    const h = std.fmt.bytesToHex(d, .lower);
    return gpa.dupe(u8, &h);
}

fn isAbs(p: []const u8) bool {
    return p.len > 0 and p[0] == '/';
}

fn cwd() ![]const u8 {
    var buf: [4096]u8 = undefined;
    const r = c.getcwd(&buf, buf.len) orelse return error.Cwd;
    return gpa.dupe(u8, std.mem.sliceTo(r, 0));
}

/// os.path.normpath (posix).
fn normpath(p: []const u8) ![]const u8 {
    if (p.len == 0) return ".";
    var slashes: usize = 0;
    if (p[0] == '/') {
        slashes = 1;
        if (p.len > 1 and p[1] == '/' and !(p.len > 2 and p[2] == '/')) slashes = 2;
    }
    var comps: std.ArrayList([]const u8) = .empty;
    var it = std.mem.splitScalar(u8, p, '/');
    while (it.next()) |comp| {
        if (comp.len == 0 or std.mem.eql(u8, comp, ".")) continue;
        if (std.mem.eql(u8, comp, "..") and ((slashes == 0 and comps.items.len == 0) or
            (comps.items.len > 0 and std.mem.eql(u8, comps.items[comps.items.len - 1], ".."))))
        {
            try comps.append(gpa, comp);
        } else if (std.mem.eql(u8, comp, "..")) {
            if (comps.items.len > 0) _ = comps.pop();
        } else try comps.append(gpa, comp);
    }
    var out: std.ArrayList(u8) = .empty;
    try out.appendNTimes(gpa, '/', slashes);
    for (comps.items, 0..) |comp, k| {
        if (k > 0) try out.append(gpa, '/');
        try out.appendSlice(gpa, comp);
    }
    if (out.items.len == 0) return ".";
    return out.items;
}

fn abspath(p: []const u8) ![]const u8 {
    if (isAbs(p)) return normpath(p);
    return normpath(try util.join(try cwd(), p));
}

/// os.path.realpath (non-strict): symlinks resolved as far as the path exists.
fn realpath(p: []const u8) ![]const u8 {
    const ap = try abspath(p);
    var buf: [4096]u8 = undefined;
    const pz = try util.z(ap);
    if (c.realpath(pz.ptr, &buf)) |r| return gpa.dupe(u8, std.mem.sliceTo(r, 0));
    if (std.mem.eql(u8, ap, "/")) return ap;
    const parent = try realpath(util.dirname(ap));
    return normpath(try util.join(parent, util.basename(ap)));
}

/// os.path.relpath(path, start).
fn relpath(path: []const u8, start: []const u8) ![]const u8 {
    const p = try abspath(path);
    const s = try abspath(start);
    var pc: std.ArrayList([]const u8) = .empty;
    var sc: std.ArrayList([]const u8) = .empty;
    var it = std.mem.splitScalar(u8, p, '/');
    while (it.next()) |x| if (x.len > 0) try pc.append(gpa, x);
    it = std.mem.splitScalar(u8, s, '/');
    while (it.next()) |x| if (x.len > 0) try sc.append(gpa, x);
    var i: usize = 0;
    while (i < pc.items.len and i < sc.items.len and std.mem.eql(u8, pc.items[i], sc.items[i])) i += 1;
    var out: std.ArrayList(u8) = .empty;
    for (i..sc.items.len) |_| {
        if (out.items.len > 0) try out.append(gpa, '/');
        try out.appendSlice(gpa, "..");
    }
    for (pc.items[i..]) |x| {
        if (out.items.len > 0) try out.append(gpa, '/');
        try out.appendSlice(gpa, x);
    }
    if (out.items.len == 0) return ".";
    return out.items;
}

/// os.path.splitext(name)[1].lower(): a leading run of dots does not start an extension.
fn extLower(path: []const u8) ![]const u8 {
    const name = util.basename(path);
    const dot = std.mem.lastIndexOfScalar(u8, name, '.') orelse return "";
    var k: usize = 0;
    while (k < dot and name[k] == '.') k += 1;
    if (k == dot) return "";
    return std.ascii.allocLowerString(gpa, name[dot..]);
}

fn isHeaderExt(path: []const u8) !bool {
    const e = try extLower(path);
    for (HEADER_EXT) |h| if (std.mem.eql(u8, e, h)) return true;
    return false;
}

fn objName(tu: []const u8) ![]const u8 {
    const n = try gpa.dupe(u8, tu);
    std.mem.replaceScalar(u8, n, '/', '_');
    if (std.mem.endsWith(u8, tu, ".cpp")) return std.fmt.allocPrint(gpa, "{s}.o", .{n[0 .. n.len - 4]});
    return std.fmt.allocPrint(gpa, "{s}.o", .{n});
}

fn depName(tu: []const u8) ![]const u8 {
    const n = try gpa.dupe(u8, tu);
    std.mem.replaceScalar(u8, n, '/', '_');
    return std.fmt.allocPrint(gpa, "{s}.d", .{n});
}

// ---------------------------------------------------------------- files

const Stat = struct { size: i64, mtime_s: i64, mtime: f64, is_dir: bool, is_link: bool };

fn statPath(p: []const u8, follow: bool) ?Stat {
    const pz = util.z(p) catch return null;
    var st: c.struct_stat = undefined;
    const rc = if (follow) c.stat(pz.ptr, &st) else c.lstat(pz.ptr, &st);
    if (rc != 0) return null;
    const ts = st.st_mtimespec;
    return .{
        .size = @intCast(st.st_size),
        .mtime_s = @intCast(ts.tv_sec),
        .mtime = @as(f64, @floatFromInt(ts.tv_sec)) + @as(f64, @floatFromInt(ts.tv_nsec)) / 1e9,
        .is_dir = (st.st_mode & c.S_IFMT) == c.S_IFDIR,
        .is_link = (st.st_mode & c.S_IFMT) == c.S_IFLNK,
    };
}

fn readList(path: []const u8) ![][]const u8 {
    var out: std.ArrayList([]const u8) = .empty;
    var it = util.Lines{ .data = try util.readFile(path) };
    while (it.next()) |line| if (util.strip(line).len > 0) try out.append(gpa, line);
    return out.items;
}

fn writeLines(path: []const u8, lines: []const []const u8) !void {
    var buf: std.ArrayList(u8) = .empty;
    for (lines) |l| {
        try buf.appendSlice(gpa, l);
        try buf.append(gpa, '\n');
    }
    try util.writeFile(path, buf.items);
}

/// tempfile.mkstemp in the target's directory, then os.replace.
fn writeAtomic(path: []const u8, data: []const u8) !void {
    util.makeDirs(util.dirname(path));
    const tmpl = try std.fmt.allocPrintSentinel(gpa, "{s}/tmpXXXXXXXX", .{util.dirname(path)}, 0);
    const fd = c.mkstemp(tmpl.ptr);
    if (fd < 0) return error.Mkstemp;
    var p = data;
    while (p.len > 0) {
        const n = c.write(fd, p.ptr, p.len);
        if (n <= 0) {
            _ = c.close(fd);
            return error.Write;
        }
        p = p[@intCast(n)..];
    }
    _ = c.close(fd);
    if (c.rename(tmpl.ptr, (try util.z(path)).ptr) != 0) return error.Rename;
}

fn copyFile(src: []const u8, dst: []const u8) !void {
    const data = (try util.readFileOpt(src)) orelse return error.NoSource;
    try util.writeFile(dst, data);
}

/// Every file under `root` (os.walk, followlinks=False), skipping directories named `skip`.
fn walkFiles(root: []const u8, skip: ?[]const u8, out: *std.ArrayList([]const u8)) !void {
    const rz = try util.z(root);
    const d = c.opendir(rz.ptr) orelse return;
    defer _ = c.closedir(d);
    var subdirs: std.ArrayList([]const u8) = .empty;
    while (c.readdir(d)) |ent| {
        const name = std.mem.sliceTo(@as([*:0]const u8, @ptrCast(&ent.*.d_name)), 0);
        if (std.mem.eql(u8, name, ".") or std.mem.eql(u8, name, "..")) continue;
        const full = try util.join(root, name);
        const st = statPath(full, true);
        if (st != null and st.?.is_dir) {
            if (skip != null and std.mem.eql(u8, name, skip.?)) continue;
            const lst = statPath(full, false);
            if (lst != null and lst.?.is_link) continue;
            try subdirs.append(gpa, full);
        } else try out.append(gpa, full);
    }
    for (subdirs.items) |sd| try walkFiles(sd, skip, out);
}

// ---------------------------------------------------------------- keys

const Hasher = struct {
    tree: []const u8,
    memo: std.StringHashMapUnmanaged([]const u8) = .empty,

    fn file(h: *Hasher, path: []const u8) ![]const u8 {
        const full = if (isAbs(path)) path else try util.join(h.tree, path);
        if (h.memo.get(full)) |v| return v;
        const v: []const u8 = if (try util.readFileOpt(full)) |data| try shaHex(data) else "missing";
        try h.memo.put(gpa, full, v);
        return v;
    }
};

/// The stdout of a command run through the shell, stripped (subprocess.run(...).stdout.strip()).
fn runOut(cmd: []const u8) ![]const u8 {
    const cz = try util.z(cmd);
    const f = c.popen(cz.ptr, "r") orelse return "";
    var out: std.ArrayList(u8) = .empty;
    var buf: [4096]u8 = undefined;
    while (true) {
        const n = c.fread(&buf, 1, buf.len, f);
        if (n == 0) break;
        try out.appendSlice(gpa, buf[0..n]);
    }
    _ = c.pclose(f);
    return util.strip(out.items);
}

fn shellQuote(s: []const u8) ![]const u8 {
    var out: std.ArrayList(u8) = .empty;
    try out.append(gpa, '\'');
    for (s) |ch| {
        if (ch == '\'') try out.appendSlice(gpa, "'\\''") else try out.append(gpa, ch);
    }
    try out.append(gpa, '\'');
    return out.items;
}

fn isExecFile(p: []const u8) bool {
    const pz = util.z(p) catch return false;
    if (c.access(pz.ptr, c.X_OK) != 0) return false;
    const st = statPath(p, true) orelse return false;
    return !st.is_dir;
}

/// shutil.which(prog), or null.
fn which(prog: []const u8) !?[]const u8 {
    if (std.mem.indexOfScalar(u8, prog, '/') != null) return if (isExecFile(prog)) prog else null;
    const path = util.getenv("PATH") orelse "/bin:/usr/bin";
    var it = std.mem.splitScalar(u8, path, ':');
    var seen = std.StringHashMapUnmanaged(void).empty;
    while (it.next()) |dir| {
        if (seen.contains(dir)) continue;
        try seen.put(gpa, dir, {});
        const cand = try util.join(dir, prog);
        if (isExecFile(cand)) return cand;
    }
    return null;
}

fn compilerIdentity() ![]const u8 {
    const compiler = util.getenv("D2_CXX") orelse "i686-w64-mingw32-g++";
    const q = try shellQuote(compiler);
    var parts: std.ArrayList([]const u8) = .empty;
    const progs = [_][]const u8{ compiler, try runOut(try std.fmt.allocPrint(gpa, "{s} -print-prog-name=cc1plus 2>/dev/null", .{q})) };
    for (progs) |prog| {
        const path = (try which(prog)) orelse prog;
        const real = try realpath(path);
        const st = statPath(real, true) orelse {
            std.debug.print("tucache: cannot stat {s}\n", .{real});
            return error.Stat;
        };
        try parts.append(gpa, try std.fmt.allocPrint(gpa, "{s}:{d}:{d}", .{ real, st.size, st.mtime_s }));
    }
    for ([_][]const u8{ "-dumpfullversion", "-dumpmachine" }) |flag|
        try parts.append(gpa, try runOut(try std.fmt.allocPrint(gpa, "{s} {s} 2>/dev/null", .{ q, flag })));
    return std.mem.join(gpa, "|", parts.items);
}

fn lessThan(_: void, a: []const u8, b: []const u8) bool {
    return std.mem.order(u8, a, b) == .lt;
}

fn headerSet(tree: []const u8) ![]const u8 {
    var files: std.ArrayList([]const u8) = .empty;
    try walkFiles(tree, ".git", &files);
    var paths: std.ArrayList([]const u8) = .empty;
    for (files.items) |full| {
        if (try isHeaderExt(full)) {
            const rel = full[tree.len + 1 ..];
            try paths.append(gpa, try normpath(rel));
        }
    }
    std.mem.sort([]const u8, paths.items, {}, lessThan);
    return shaHex(try std.mem.join(gpa, "\n", paths.items));
}

fn globalKey(a: Args) ![]const u8 {
    var env: std.ArrayList([]const u8) = .empty;
    for (ENV_KEYS) |k| try env.append(gpa, try std.fmt.allocPrint(gpa, "{s}={s}", .{ k, util.getenv(k) orelse "" }));
    const rsp = try shaHex(try util.readFile(req(a.rsp)));
    const tree = if (a.tree.len > 0 and a.tree[a.tree.len - 1] == '/' and a.tree.len > 1) a.tree[0 .. a.tree.len - 1] else a.tree;
    const parts = [_][]const u8{ FORMAT, a.mode orelse "None", a.flags, try realpath(a.tree), rsp, try compilerIdentity(), try std.mem.join(gpa, "|", env.items), try headerSet(tree) };
    return shaHex(try std.mem.join(gpa, "\n", &parts));
}

fn manifestPath(root: []const u8, gkey: []const u8, tu: []const u8, h: *Hasher) ![]const u8 {
    const key = try shaHex(try std.fmt.allocPrint(gpa, "{s}\n{s}\n{s}", .{ gkey, tu, try h.file(tu) }));
    return std.fmt.allocPrint(gpa, "{s}/manifests/{s}/{s}.json", .{ root, key[0..2], key });
}

// ---------------------------------------------------------------- JSON as Python writes it

fn loadManifest(path: []const u8) []std.json.Value {
    const data = (util.readFileOpt(path) catch return &.{}) orelse return &.{};
    const v = std.json.parseFromSliceLeaky(std.json.Value, gpa, data, .{}) catch return &.{};
    return switch (v) {
        .array => |arr| arr.items,
        else => &.{},
    };
}

/// json.dumps's string: ensure_ascii, lowercase \u escapes, surrogate pairs above the BMP.
fn dumpString(out: *std.ArrayList(u8), s: []const u8) !void {
    try out.append(gpa, '"');
    var i: usize = 0;
    while (i < s.len) {
        const ch = s[i];
        if (ch < 0x80) {
            switch (ch) {
                '"' => try out.appendSlice(gpa, "\\\""),
                '\\' => try out.appendSlice(gpa, "\\\\"),
                '\n' => try out.appendSlice(gpa, "\\n"),
                '\r' => try out.appendSlice(gpa, "\\r"),
                '\t' => try out.appendSlice(gpa, "\\t"),
                0x08 => try out.appendSlice(gpa, "\\b"),
                0x0c => try out.appendSlice(gpa, "\\f"),
                else => if (ch < 0x20) try out.print(gpa, "\\u{x:0>4}", .{ch}) else try out.append(gpa, ch),
            }
            i += 1;
            continue;
        }
        const n = std.unicode.utf8ByteSequenceLength(ch) catch 1;
        const cp: u21 = if (n > 1 and i + n <= s.len) std.unicode.utf8Decode(s[i .. i + n]) catch ch else ch;
        i += if (n > 1 and i + n <= s.len) n else 1;
        if (cp >= 0x10000) {
            const v = cp - 0x10000;
            try out.print(gpa, "\\u{x:0>4}\\u{x:0>4}", .{ 0xd800 | (v >> 10), 0xdc00 | (v & 0x3ff) });
        } else try out.print(gpa, "\\u{x:0>4}", .{cp});
    }
    try out.append(gpa, '"');
}

fn dumpValue(out: *std.ArrayList(u8), v: std.json.Value) !void {
    switch (v) {
        .null => try out.appendSlice(gpa, "null"),
        .bool => |b| try out.appendSlice(gpa, if (b) "true" else "false"),
        .integer => |n| try out.print(gpa, "{d}", .{n}),
        .float => |f| try out.print(gpa, "{d}", .{f}),
        .number_string => |s| try out.appendSlice(gpa, s),
        .string => |s| try dumpString(out, s),
        .array => |arr| {
            try out.append(gpa, '[');
            for (arr.items, 0..) |x, k| {
                if (k > 0) try out.appendSlice(gpa, ", ");
                try dumpValue(out, x);
            }
            try out.append(gpa, ']');
        },
        .object => |obj| {
            try out.append(gpa, '{');
            var it = obj.iterator();
            var k: usize = 0;
            while (it.next()) |e| : (k += 1) {
                if (k > 0) try out.appendSlice(gpa, ", ");
                try dumpString(out, e.key_ptr.*);
                try out.appendSlice(gpa, ": ");
                try dumpValue(out, e.value_ptr.*);
            }
            try out.append(gpa, '}');
        },
    }
}

const Dep = struct { path: []const u8, hash: []const u8 };

fn field(v: std.json.Value, name: []const u8) ?std.json.Value {
    return switch (v) {
        .object => |o| o.get(name),
        else => null,
    };
}

/// The entry's dependencies, or null when they are not a list of [path, hash] pairs.
fn entryDeps(e: std.json.Value) ?[]Dep {
    const d = field(e, "deps") orelse return null;
    if (d != .array) return null;
    const out = gpa.alloc(Dep, d.array.items.len) catch return null;
    for (d.array.items, 0..) |pair, k| {
        if (pair != .array or pair.array.items.len != 2) return null;
        const p = pair.array.items[0];
        const h = pair.array.items[1];
        if (p != .string or h != .string) return null;
        out[k] = .{ .path = p.string, .hash = h.string };
    }
    return out;
}

fn depsEqual(a: []const Dep, b: []const Dep) bool {
    if (a.len != b.len) return false;
    for (a, b) |x, y| if (!std.mem.eql(u8, x.path, y.path) or !std.mem.eql(u8, x.hash, y.hash)) return false;
    return true;
}

// ---------------------------------------------------------------- commands

fn cmdLookup(a: Args) !void {
    const root = try cacheRoot();
    const tus = try readList(req(a.list));
    var hits: std.ArrayList([]const u8) = .empty;
    var misses: std.ArrayList([]const u8) = .empty;
    if (root == null) {
        try misses.appendSlice(gpa, tus);
    } else {
        const gkey = try globalKey(a);
        var h = Hasher{ .tree = a.tree };
        const obj_mode = std.mem.eql(u8, a.mode orelse "", "obj");
        for (tus) |tu| {
            var entry: ?std.json.Value = null;
            for (loadManifest(try manifestPath(root.?, gkey, tu, &h))) |cand| {
                const deps = entryDeps(cand) orelse continue;
                var all = true;
                for (deps) |d| if (!std.mem.eql(u8, try h.file(d.path), d.hash)) {
                    all = false;
                    break;
                };
                if (all) {
                    entry = cand;
                    break;
                }
            }
            const res = if (entry) |e| field(e, "result") else null;
            if (res == null) {
                try misses.append(gpa, tu);
                continue;
            }
            if (obj_mode) {
                const o = field(res.?, "obj") orelse {
                    try misses.append(gpa, tu);
                    continue;
                };
                if (o != .string or o.string.len < 2) {
                    try misses.append(gpa, tu);
                    continue;
                }
                const src = try std.fmt.allocPrint(gpa, "{s}/objects/{s}/{s}.o", .{ root.?, o.string[0..2], o.string });
                copyFile(src, try util.join(req(a.objdir), try objName(tu))) catch {
                    try misses.append(gpa, tu);
                    continue;
                };
                try hits.append(gpa, tu);
            } else {
                const s = field(res.?, "strict") orelse {
                    try misses.append(gpa, tu);
                    continue;
                };
                const n: i64 = switch (s) {
                    .integer => |n| n,
                    else => {
                        try misses.append(gpa, tu);
                        continue;
                    },
                };
                try hits.append(gpa, try std.fmt.allocPrint(gpa, "{d}\t{s}", .{ n, tu }));
            }
        }
    }
    try writeLines(req(a.hits), hits.items);
    try writeLines(req(a.misses), misses.items);
    std.debug.print("tucache: {d} hit(s), {d} miss(es){s}\n", .{ hits.items.len, misses.items.len, if (root != null) "" else " (cache off)" });
}

/// Make-syntax dependency list: `target: dep dep \` with `\ ` escaping spaces.
fn parseDepfile(path: []const u8) !?[][]const u8 {
    const raw = try util.readFile(path);
    const text = try std.mem.replaceOwned(u8, gpa, raw, "\\\n", " ");
    const colon = std.mem.indexOf(u8, text, ": ") orelse return null;
    const body = text[colon + 2 ..];
    var deps: std.ArrayList([]const u8) = .empty;
    var cur: std.ArrayList(u8) = .empty;
    var i: usize = 0;
    while (i < body.len) {
        const ch = body[i];
        if (ch == '\\' and i + 1 < body.len and (body[i + 1] == ' ' or body[i + 1] == '#')) {
            try cur.append(gpa, body[i + 1]);
            i += 2;
            continue;
        }
        if (ch == '$' and i + 1 < body.len and body[i + 1] == '$') {
            try cur.append(gpa, '$');
            i += 2;
            continue;
        }
        if (ch == ' ' or ch == '\t' or ch == '\n') {
            if (cur.items.len > 0) {
                try deps.append(gpa, cur.items);
                cur = .empty;
            }
            // -MD ends the first rule at a blank line; -MP phony targets follow it.
            if (ch == '\n' and i + 1 < body.len and body[i + 1] == '\n') break;
            i += 1;
            continue;
        }
        try cur.append(gpa, ch);
        i += 1;
    }
    if (cur.items.len > 0) try deps.append(gpa, cur.items);
    return deps.items;
}

const Candidate = struct { strict: i64, rc: i64 };

fn cmdStore(a: Args) !void {
    const root = (try cacheRoot()) orelse return;
    const gkey = try globalKey(a);
    var h = Hasher{ .tree = a.tree };
    const syntax = std.mem.eql(u8, a.mode orelse "", "syntax");
    var results = std.StringHashMapUnmanaged(Candidate).empty;
    var confirmed = std.StringHashMapUnmanaged([][]const u8).empty;
    if (syntax) {
        var it = util.Lines{ .data = try util.readFile(req(a.candidates)) };
        while (it.next()) |line| {
            const p = try util.split(line, '\t');
            if (p.len != 3) return error.BadCandidates;
            try results.put(gpa, p[1], .{ .strict = try std.fmt.parseInt(i64, util.strip(p[0]), 10), .rc = try std.fmt.parseInt(i64, util.strip(p[2]), 10) });
        }
        var cit = util.Lines{ .data = try util.readFile(req(a.results)) };
        while (cit.next()) |line| {
            const p = try util.split(line, '\t');
            if (p.len < 2) return error.BadResults;
            try confirmed.put(gpa, p[1], p);
        }
    }
    var failed = std.StringHashMapUnmanaged(void).empty;
    if (a.failed) |f| if (f.len > 0 and util.exists(f)) {
        for (try readList(f)) |l| try failed.put(gpa, l, {});
    };
    var stored: usize = 0;
    var skipped: usize = 0;
    const tree_real = try realpath(a.tree);
    const tree_prefix = try std.fmt.allocPrint(gpa, "{s}/", .{tree_real});
    const now: i64 = c.time(null);
    for (try readList(req(a.list))) |tu| {
        const depfile = try util.join(req(a.depdir), try depName(tu));
        const deps = (if (util.exists(depfile)) try parseDepfile(depfile) else null) orelse {
            skipped += 1;
            continue;
        };
        if (deps.len == 0) {
            skipped += 1;
            continue;
        }
        var ok = true;
        var rel: std.ArrayList([]const u8) = .empty;
        for (deps) |d0| {
            var d = d0;
            if (isAbs(d)) {
                const r = try realpath(d);
                if (std.mem.startsWith(u8, r, tree_prefix)) d = try relpath(r, tree_real);
            }
            d = try normpath(d);
            if (!isAbs(d) and !std.mem.eql(u8, d, tu) and !try isHeaderExt(d)) {
                ok = false;
                break;
            }
            try rel.append(gpa, d);
        }
        if (!ok) {
            skipped += 1;
            continue;
        }
        var result: []const u8 = undefined;
        if (syntax) {
            const r = results.get(tu) orelse {
                skipped += 1;
                continue;
            };
            if (confirmed.get(tu)) |row| {
                // Every candidate was recompiled serially; that row is the number.
                if (row.len != 3) return error.BadResults;
                if (!std.mem.eql(u8, row[2], "clean")) {
                    skipped += 1;
                    continue;
                }
                result = try std.fmt.allocPrint(gpa, "{{\"strict\": {d}}}", .{try std.fmt.parseInt(i64, util.strip(row[0]), 10)});
            } else if (r.strict == 0 and r.rc == 0) {
                result = "{\"strict\": 0}";
            } else {
                skipped += 1;
                continue;
            }
        } else {
            if (failed.contains(tu)) {
                skipped += 1;
                continue;
            }
            const data = (try util.readFileOpt(try util.join(req(a.objdir), try objName(tu)))) orelse {
                skipped += 1;
                continue;
            };
            const okey = try shaHex(data);
            const dst = try std.fmt.allocPrint(gpa, "{s}/objects/{s}/{s}.o", .{ root, okey[0..2], okey });
            if (!util.exists(dst)) {
                util.makeDirs(util.dirname(dst));
                const tmp = try std.fmt.allocPrint(gpa, "{s}.tmp{d}", .{ dst, c.getpid() });
                try util.writeFile(tmp, data);
                if (c.rename((try util.z(tmp)).ptr, (try util.z(dst)).ptr) != 0) return error.Rename;
            }
            result = try std.fmt.allocPrint(gpa, "{{\"obj\": \"{s}\"}}", .{okey});
        }
        const mine = try gpa.alloc(Dep, rel.items.len);
        for (rel.items, 0..) |d, k| mine[k] = .{ .path = d, .hash = try h.file(d) };
        var text: std.ArrayList(u8) = .empty;
        try text.appendSlice(gpa, "[{\"deps\": [");
        for (mine, 0..) |d, k| {
            if (k > 0) try text.appendSlice(gpa, ", ");
            try text.append(gpa, '[');
            try dumpString(&text, d.path);
            try text.appendSlice(gpa, ", ");
            try dumpString(&text, d.hash);
            try text.append(gpa, ']');
        }
        try text.print(gpa, "], \"result\": {s}, \"t\": {d}}}", .{ result, now });
        const mpath = try manifestPath(root, gkey, tu, &h);
        var kept: usize = 1;
        for (loadManifest(mpath)) |e| {
            if (kept >= MAX_ENTRIES) break;
            if (entryDeps(e)) |ed| if (depsEqual(ed, mine)) continue;
            try text.appendSlice(gpa, ", ");
            try dumpValue(&text, e);
            kept += 1;
        }
        try text.append(gpa, ']');
        try writeAtomic(mpath, text.items);
        stored += 1;
    }
    std.debug.print("tucache: stored {d}, not cacheable {d}\n", .{ stored, skipped });
}

fn nowF() f64 {
    var tv: c.struct_timeval = undefined;
    _ = c.gettimeofday(&tv, null);
    return @as(f64, @floatFromInt(tv.tv_sec)) + @as(f64, @floatFromInt(tv.tv_usec)) / 1e6;
}

fn cmdPrune(a: Args) !void {
    const root = (try cacheRoot()) orelse return;
    const rst = statPath(root, true) orelse return;
    if (!rst.is_dir) return;
    // Reading every manifest costs ~40 s on a 10 GB cache, and link.sh prunes after every
    // build: once per D2_TU_CACHE_PRUNE_HOURS (default 24) is enough for a 14-day expiry.
    // --force prunes now.
    const stamp = try util.join(root, ".last-prune");
    const hours = std.fmt.parseFloat(f64, util.strip(util.getenv("D2_TU_CACHE_PRUNE_HOURS") orelse "24")) catch 24;
    const now = nowF();
    if (statPath(stamp, true)) |st| {
        const age = now - st.mtime;
        if (!a.force and age < hours * 3600) {
            var buf: std.ArrayList(u8) = .empty;
            try util.fixed(&buf, "%.1f", age / 3600);
            std.debug.print("tucache: pruned {s} h ago; not again before {d} h\n", .{ buf.items, hours });
            return;
        }
    }
    try util.writeFile(stamp, try std.fmt.allocPrint(gpa, "{d}\n", .{@as(i64, @intFromFloat(now))}));
    const cutoff = now - @as(f64, @floatFromInt(a.days)) * 86400;
    var live = std.StringHashMapUnmanaged(void).empty;
    var manifests: std.ArrayList([]const u8) = .empty;
    try walkFiles(try util.join(root, "manifests"), null, &manifests);
    for (manifests.items) |p| {
        const st = statPath(p, true) orelse continue;
        if (st.mtime < cutoff) {
            _ = c.unlink((try util.z(p)).ptr);
            continue;
        }
        for (loadManifest(p)) |e| {
            const res = field(e, "result") orelse continue;
            if (field(res, "obj")) |o| if (o == .string) try live.put(gpa, o.string, {});
        }
    }
    var removed: usize = 0;
    var objects: std.ArrayList([]const u8) = .empty;
    try walkFiles(try util.join(root, "objects"), null, &objects);
    for (objects.items) |p| {
        const f = util.basename(p);
        if (std.mem.endsWith(u8, f, ".o") and !live.contains(f[0 .. f.len - 2])) {
            _ = c.unlink((try util.z(p)).ptr);
            removed += 1;
        }
    }
    std.debug.print("tucache: pruned {d} unreferenced object(s)\n", .{removed});
}
