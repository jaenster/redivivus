//! build.py: compile the reconstruction with Visual C++ 2005, one object per TU.
//!
//!   oracle-zig build --root <asm_matching/msvc8> [-j N] [--batch N] [--force] [--with-data]
//!                    [--extra DIR [--support "A.cpp B.cpp"]] [PATTERN ...]
//!
//! The tree is mirrored into build/src by ghidra-reconstruct's VC8 translator
//! (ts_translate.zig), then every translation unit (or those whose path contains a PATTERN)
//! is compiled to build/obj/<path>.obj under wine, N cl.exe at a time, each cl.exe taking a
//! batch of TUs of one object directory. Incremental by content: every compile is stored in
//! the shared cache (objcache.zig) under the hash of what produced it, and a TU is compiled
//! only when no stored compile matches; an object already in place for the same inputs (its
//! <path>.key) is left untouched. A function VC8 cannot compile is quarantined round by round
//! (build/quarantine.tsv) and its body blanked in the translation. Outputs, keys and cache
//! entries are build.py's, file for file; build.py stays the fall-back when there is no
//! ghidra-reconstruct checkout to translate with. build.py's decoder pool (--decode-jobs) is
//! not here: it only warms the Python readers' pickle cache.
const std = @import("std");
const builtin = @import("builtin");
const c = @import("c");
const util = @import("util.zig");
const proc = @import("proc.zig");
const objcache = @import("objcache.zig");
const ts_translate = @import("ts_translate.zig");
const coff = @import("coff.zig");
const gpa = util.gpa;

const DEFAULT_CFLAGS = "/O2 /Oy- /Ob1 /EHsc /GS /MT /W1 /D WIN32 /D NDEBUG /D _WINDOWS";
const TOOL_CFLAGS = "/Z7";
const SUPPORT = "Redivivus";
pub const PCH_HEADER = "oracle_pch.h";
const PLATFORM_H = "Source/D2Headers/d2_platform.h";
const CACHE_VERSION = "build-1";
const MAX_QUARANTINE_ROUNDS = 40;
pub const COMPILE_TIMEOUT = 300.0;
const BATCH = 16;
const BACKEND = "Generating Code...";
const QHEADER = "file\tline\tfunction_line\tfunction\terror\n";
const EXTS = [_][]const u8{ ".h", ".hpp", ".inl", ".cpp", ".c" };
const OBJ_EXTS = [_][]const u8{ ".obj", ".d", ".log", ".flags", ".key", ".fn.pkl", ".fn.sum" };
const SKIP_DIRS = [_][]const u8{ ".git", ".github", ".idea", "asm_matching", "build", "cmake", "tools", "metrics" };
const VENDORED = [_][]const u8{ "3rdparty/libbacktrace", "3rdparty/implibs" };
const HEADERS_ONLY_BUT = "Redivivus/ghidra_version.cpp";

// ---- configuration (oracle_config.py, build.py's module globals)

var ORACLE: []const u8 = undefined;
pub var REPO: []const u8 = undefined;
pub var BUILD: []const u8 = undefined;
var SRC: []const u8 = undefined;
var OBJ: []const u8 = undefined;
pub var HERE: []const u8 = undefined;
var PCHDIR: []const u8 = undefined;
pub var PCH_FILE: []const u8 = undefined;
var QUARANTINE_TSV: []const u8 = undefined;
var DRIVE: []const u8 = undefined;
var VC: []const u8 = undefined;
var CFLAGS: []const u8 = undefined;
var EXPERIMENT = false;
pub var USE_PCH = true;
var REPO_REAL: []const u8 = undefined;

const FlagRow = struct { prefix: []const u8, flags: []const u8 };
var FLAGS_TABLE: []FlagRow = &.{};

var TS: ts_translate.Translator = undefined;
var qlock: proc.Mutex = .{};
var out_lock: proc.Mutex = .{};
/// rel -> its quarantined lines, sorted (vc8_translate.QUARANTINE). Under qlock.
var QUARANTINE: std.StringHashMapUnmanaged([]u32) = .empty;

pub fn configure(root_arg: []const u8) !void {
    ORACLE = if (isAbsolute(root_arg)) try proc.normpath(root_arg) else blk: {
        var buf: [4096]u8 = undefined;
        const cwd = std.mem.sliceTo(c.getcwd(&buf, buf.len) orelse return error.Cwd, 0);
        break :blk try proc.joinNorm(cwd, root_arg);
    };
    REPO = util.dirname(util.dirname(ORACLE));
    REPO_REAL = try proc.realpath(REPO);
    HERE = try util.join(ORACLE, "scripts");
    VC = util.getenv("ORACLE_VC") orelse "sp1";
    const env_cflags = util.getenv("ORACLE_CFLAGS");
    CFLAGS = if (env_cflags != null and env_cflags.?.len > 0) env_cflags.? else DEFAULT_CFLAGS;
    EXPERIMENT = !std.mem.eql(u8, VC, "sp1") or !std.mem.eql(u8, CFLAGS, DEFAULT_CFLAGS);
    const baseline = try util.join(ORACLE, "build");
    const explicit = util.getenv("ORACLE_BUILD");
    const has_explicit = explicit != null and explicit.?.len > 0;
    BUILD = baseline;
    if (EXPERIMENT) {
        var md5: [16]u8 = undefined;
        std.crypto.hash.Md5.hash(env_cflags orelse "", &md5, .{});
        const tag = std.fmt.bytesToHex(md5, .lower);
        if (has_explicit and std.mem.eql(u8, try proc.realpath(explicit.?), try proc.realpath(baseline))) {
            std.debug.print("refusing to build an experiment (ORACLE_VC={s}, ORACLE_CFLAGS='{s}') into the baseline directory {s}\n", .{ VC, CFLAGS, baseline });
            std.process.exit(1);
        }
        BUILD = if (has_explicit) explicit.? else try std.fmt.allocPrint(gpa, "{s}/experiment-{s}-{s}", .{ baseline, VC, tag[0..8] });
    } else if (has_explicit) BUILD = explicit.?;
    SRC = try util.join(BUILD, "src");
    OBJ = try util.join(BUILD, "obj");
    USE_PCH = std.mem.eql(u8, util.getenv("ORACLE_PCH") orelse "1", "1");
    PCHDIR = try util.join(BUILD, "pch");
    PCH_FILE = try util.join(PCHDIR, "oracle.pch");
    QUARANTINE_TSV = try util.join(BUILD, "quarantine.tsv");
    const d = util.getenv("ORACLE_DRIVE") orelse "r";
    DRIVE = try std.fmt.allocPrint(gpa, "{s}:", .{try std.ascii.allocUpperString(gpa, d)});
    FLAGS_TABLE = try loadFlagsTable(try util.join(ORACLE, "flags.tsv"));
}

fn isAbsolute(path: []const u8) bool {
    if (path.len > 0 and path[0] == '/') return true;
    return builtin.os.tag == .windows and
        ((path.len >= 3 and std.ascii.isAlphabetic(path[0]) and path[1] == ':' and (path[2] == '\\' or path[2] == '/')) or
        (path.len >= 2 and path[0] == '\\' and path[1] == '\\'));
}

fn loadFlagsTable(path: []const u8) ![]FlagRow {
    var rows: std.ArrayList(FlagRow) = .empty;
    const data = (try util.readFileOpt(path)) orelse return rows.items;
    var it = util.Lines{ .data = data };
    while (it.next()) |line| {
        if (std.mem.startsWith(u8, line, "#") or util.strip(line).len == 0) continue;
        const parts = try util.split(line, '\t');
        try rows.append(gpa, .{ .prefix = parts[0], .flags = if (parts.len > 1) parts[1] else "" });
    }
    std.mem.sort(FlagRow, rows.items, {}, struct {
        fn lt(_: void, a: FlagRow, b: FlagRow) bool {
            return a.prefix.len > b.prefix.len;
        }
    }.lt);
    return rows.items;
}

/// The flags.tsv row with the longest prefix of `tu`, or "".
pub fn extraFlags(tu: []const u8) []const u8 {
    for (FLAGS_TABLE) |r| if (std.mem.startsWith(u8, tu, r.prefix)) return r.flags;
    return "";
}

// ---- paths

const TU = struct {
    rel: []const u8,
    src: []const u8,
    objdir: []const u8,
    base: []const u8,
    statusdir: []const u8,

    fn init(rel: []const u8, src: []const u8, objdir: []const u8, statusdir: []const u8) !*TU {
        const t = try gpa.create(TU);
        t.* = .{ .rel = rel, .src = src, .objdir = objdir, .base = try util.join(objdir, rel[0 .. rel.len - 4]), .statusdir = statusdir };
        return t;
    }
    fn path(t: *const TU, ext: []const u8) ![]const u8 {
        return std.mem.concat(gpa, u8, &.{ t.base, ext });
    }
};

/// The path as the compiler sees it: drive:\<path in the repository>.
pub fn win(path: []const u8) ![]const u8 {
    const rel = try proc.relpath(try proc.realpath(path), REPO_REAL);
    if (std.mem.startsWith(u8, rel, "..")) {
        std.debug.print("{s} is outside the repository\n", .{path});
        return error.OutsideRepo;
    }
    const out = if (builtin.os.tag == .windows)
        try gpa.dupe(u8, try proc.realpath(path))
    else
        try std.mem.concat(gpa, u8, &.{ DRIVE, "\\", rel });
    std.mem.replaceScalar(u8, out, '/', '\\');
    return out;
}

/// A path from /showIncludes or a message back to the host; only repository paths matter.
fn linux(winpath: []const u8) !?[]const u8 {
    if (builtin.os.tag == .windows) {
        const norm = try gpa.dupe(u8, winpath);
        std.mem.replaceScalar(u8, norm, '\\', '/');
        const repo = try gpa.dupe(u8, REPO_REAL);
        std.mem.replaceScalar(u8, repo, '\\', '/');
        if (norm.len <= repo.len or norm[repo.len] != '/' or
            !std.ascii.eqlIgnoreCase(norm[0..repo.len], repo)) return null;
        return try proc.joinNorm(REPO, norm[repo.len + 1 ..]);
    }
    const p = try gpa.dupe(u8, winpath);
    std.mem.replaceScalar(u8, p, '\\', '/');
    if (p.len < 2) return null;
    const head = try proc.lower(p[0..2]);
    const dl = try proc.lower(DRIVE);
    if (!std.mem.eql(u8, head, dl)) return null;
    return try proc.joinNorm(REPO, if (p.len > 3) p[3..] else "");
}

fn repoRel(path: []const u8) ![]const u8 {
    return proc.relpath(path, REPO);
}

fn hashDep(_: void, rel: []const u8) anyerror![]const u8 {
    return objcache.shaFile(try util.join(REPO, rel));
}

fn hasSuffix(name: []const u8, exts: []const []const u8) bool {
    for (exts) |e| if (std.mem.endsWith(u8, name, e)) return true;
    return false;
}

fn writeText(path: []const u8, text: []const u8) !void {
    try proc.writeAtomic(path, text);
}

fn writeIfChanged(path: []const u8, text: []const u8) !void {
    if (try util.readFileOpt(path)) |old| if (std.mem.eql(u8, old, text)) return;
    util.makeDirs(util.dirname(path));
    try util.writeFile(path, text);
}

fn readKey(path: []const u8) !?[]const u8 {
    const t = (try util.readFileOpt(path)) orelse return null;
    return util.strip(t);
}

// ---- the tree (vc8_translate.sources)

const Source = struct { rel: []const u8, name: []const u8 };

const SourcesCtx = struct { root: []const u8, out: *std.ArrayList(Source) };

fn sourcesVisit(ctx: SourcesCtx, w: *proc.WalkDir) !void {
    const rel = try proc.relpath(w.path, ctx.root);
    var keep: std.ArrayList([]const u8) = .empty;
    for (w.dirs) |d| {
        if (std.mem.eql(u8, rel, ".")) {
            var skip = std.mem.startsWith(u8, d, "build");
            for (SKIP_DIRS) |s| skip = skip or std.mem.eql(u8, d, s);
            if (skip) continue;
        } else {
            const full = try util.join(rel, d);
            var vend = false;
            for (VENDORED) |v| vend = vend or std.mem.eql(u8, full, v);
            if (vend) continue;
        }
        try keep.append(gpa, d);
    }
    w.dirs = keep.items;
    const top = if (std.mem.indexOfScalar(u8, rel, '/')) |i| rel[0..i] else rel;
    const names = try gpa.dupe([]const u8, w.files);
    proc.sortStrings(names);
    for (names) |name| {
        if (!hasSuffix(name, &EXTS)) continue;
        const relname = try util.join(rel, name);
        if (std.mem.eql(u8, top, SUPPORT) and !std.mem.endsWith(u8, name, ".h") and !std.mem.eql(u8, relname, HEADERS_ONLY_BUT)) continue;
        try ctx.out.append(gpa, .{ .rel = rel, .name = name });
    }
}

fn sources(root: []const u8) ![]Source {
    var out: std.ArrayList(Source) = .empty;
    try proc.walk(root, SourcesCtx{ .root = root, .out = &out }, sourcesVisit);
    return out.items;
}

// ---- quarantine (under qlock)

fn loadQuarantine() !void {
    QUARANTINE.clearRetainingCapacity();
    const data = (try util.readFileOpt(QUARANTINE_TSV)) orelse return;
    var tmp: std.StringHashMapUnmanaged(std.ArrayList(u32)) = .empty;
    var it = util.Lines{ .data = data };
    while (it.next()) |line| {
        const parts = try util.split(line, '\t');
        if (parts.len < 2 or std.mem.eql(u8, parts[0], "file")) continue;
        const n = try std.fmt.parseInt(u32, util.strip(parts[1]), 10);
        const g = try tmp.getOrPut(gpa, parts[0]);
        if (!g.found_existing) g.value_ptr.* = .empty;
        if (std.mem.indexOfScalar(u32, g.value_ptr.items, n) == null) try g.value_ptr.append(gpa, n);
    }
    var ti = tmp.iterator();
    while (ti.next()) |e| {
        std.mem.sort(u32, e.value_ptr.items, {}, std.sort.asc(u32));
        try QUARANTINE.put(gpa, e.key_ptr.*, e.value_ptr.items);
    }
}

fn qlines(rel: []const u8) []const u32 {
    return QUARANTINE.get(rel) orelse &.{};
}

/// The lines of quarantine.tsv with their '\n'.
fn quarantineLines() ![]const []const u8 {
    var out: std.ArrayList([]const u8) = .empty;
    const data = (try util.readFileOpt(QUARANTINE_TSV)) orelse return out.items;
    var i: usize = 0;
    while (i < data.len) {
        const e = if (std.mem.indexOfScalarPos(u8, data, i, '\n')) |n| n + 1 else data.len;
        try out.append(gpa, data[i..e]);
        i = e;
    }
    return out.items;
}

fn firstField(line: []const u8) []const u8 {
    return line[0 .. std.mem.indexOfScalar(u8, line, '\t') orelse line.len];
}

fn rstripNl(s: []const u8) []const u8 {
    return std.mem.trimEnd(u8, s, "\n");
}

/// quarantine.tsv as `lines`, or no file when no row is left.
fn writeQuarantine(lines: []const []const u8) !void {
    var any = false;
    for (lines) |l| any = any or !std.mem.eql(u8, firstField(l), "file");
    if (any) try util.writeFile(QUARANTINE_TSV, try std.mem.concat(gpa, u8, lines)) else if (util.exists(QUARANTINE_TSV)) proc.remove(QUARANTINE_TSV);
}

fn quarantineRows(rel: []const u8) ![]const []const u8 {
    var out: std.ArrayList([]const u8) = .empty;
    for (try quarantineLines()) |l| if (std.mem.eql(u8, firstField(l), rel)) try out.append(gpa, rstripNl(l));
    return out.items;
}

/// Make the TU's rows in quarantine.tsv exactly `rows`, and its translation follow them.
fn setQuarantine(t: *TU, rows: []const []const u8) !void {
    qlock.lock();
    defer qlock.unlock();
    const lines = try quarantineLines();
    var cur: std.ArrayList([]const u8) = .empty;
    var keep: std.ArrayList([]const u8) = .empty;
    for (lines) |l| {
        if (std.mem.eql(u8, firstField(l), t.rel)) try cur.append(gpa, rstripNl(l)) else try keep.append(gpa, l);
    }
    if (cur.items.len == rows.len) {
        var same = true;
        for (cur.items, rows) |a, b| same = same and std.mem.eql(u8, a, b);
        if (same) return;
    }
    if (keep.items.len == 0 and rows.len > 0) try keep.append(gpa, QHEADER);
    for (rows) |r| try keep.append(gpa, try std.mem.concat(gpa, u8, &.{ r, "\n" }));
    var changed = keep.items.len != lines.len;
    if (!changed) for (keep.items, lines) |a, b| {
        if (!std.mem.eql(u8, a, b)) changed = true;
    };
    if (changed) try writeQuarantine(keep.items);
    try loadQuarantine();
    const out = try TS.translate(t.rel, qlines(t.rel));
    const old = try util.readFileOpt(t.src);
    if (old == null or !std.mem.eql(u8, old.?, out)) try util.writeFile(t.src, out);
}

/// Forget the TU's quarantined functions and re-translate it without them.
fn unquarantine(t: *TU) !void {
    qlock.lock();
    defer qlock.unlock();
    if (!util.exists(QUARANTINE_TSV)) return;
    const lines = try quarantineLines();
    var keep: std.ArrayList([]const u8) = .empty;
    for (lines) |l| if (!std.mem.eql(u8, firstField(l), t.rel)) try keep.append(gpa, l);
    if (keep.items.len == lines.len) return;
    try writeQuarantine(keep.items);
    try loadQuarantine();
    try util.writeFile(t.src, try TS.translate(t.rel, qlines(t.rel)));
}

const ErrorAt = struct { file: []const u8, line: u32, code: []const u8, msg: []const u8 };

/// ^(?P<file>.+?)\((?P<line>\d+)\) : (?:fatal )?error (?P<code>C\d{4}): (?P<msg>.*)$
fn errorAt(s: []const u8) ?ErrorAt {
    var i: usize = 1;
    while (i < s.len) : (i += 1) {
        if (s[i] != '(') continue;
        var j = i + 1;
        while (j < s.len and std.ascii.isDigit(s[j])) j += 1;
        if (j == i + 1) continue;
        var r = s[j..];
        if (!std.mem.startsWith(u8, r, ") : ")) continue;
        r = r[4..];
        if (std.mem.startsWith(u8, r, "fatal ")) r = r[6..];
        if (!std.mem.startsWith(u8, r, "error C") or r.len < 13) continue;
        const code = r[6..11];
        if (!(std.ascii.isDigit(code[1]) and std.ascii.isDigit(code[2]) and std.ascii.isDigit(code[3]) and std.ascii.isDigit(code[4]))) continue;
        if (!std.mem.startsWith(u8, r[11..], ": ")) continue;
        const ln = std.fmt.parseInt(u32, s[i + 1 .. j], 10) catch continue;
        return .{ .file = s[0..i], .line = ln, .code = code, .msg = r[13..] };
    }
    return null;
}

/// Record the functions holding the TU's own compile errors and re-translate it with their
/// bodies blanked. Returns the number of functions newly quarantined (0: none can be).
fn quarantine(t: *TU, log: []const []const u8) !usize {
    const Own = struct { line: u32, msg: []const u8 };
    var own: std.ArrayList(Own) = .empty;
    const src_l = try proc.lower(t.src);
    for (log) |line| {
        const m = errorAt(proc.strip(line)) orelse continue;
        const lp = (try linux(m.file)) orelse continue;
        if (!std.mem.eql(u8, try proc.lower(lp), src_l)) continue;
        try own.append(gpa, .{ .line = m.line, .msg = try std.mem.concat(gpa, u8, &.{ m.code, " ", m.msg }) });
    }
    if (own.items.len == 0) return 0;
    const at = try gpa.alloc(u32, own.items.len);
    for (own.items, 0..) |o, k| at[k] = o.line;
    qlock.lock();
    const cur = try gpa.dupe(u32, qlines(t.rel));
    qlock.unlock();
    const found = try TS.functions(t.rel, cur, at);
    var rows: std.ArrayList([]const u8) = .empty;
    for (own.items, found) |o, f| {
        const fun = f orelse return 0; // an error outside any function: cannot quarantine
        try rows.append(gpa, try std.fmt.allocPrint(gpa, "{s}\t{d}\t{d}\t{s}\t{s}", .{ t.rel, o.line, fun.line, fun.name, try proc.latin1ToUtf8(o.msg) }));
    }
    qlock.lock();
    defer qlock.unlock();
    var known: std.StringHashMapUnmanaged(void) = .empty;
    for (try quarantineLines()) |l| {
        var it = std.mem.splitScalar(u8, l, '\t');
        const a = it.next() orelse "";
        const b = it.next() orelse "";
        try known.put(gpa, try std.mem.concat(gpa, u8, &.{ a, "\t", b }), {});
    }
    var add: std.ArrayList(u8) = .empty;
    if (known.count() == 0) try add.appendSlice(gpa, QHEADER);
    var n: usize = 0;
    for (rows.items, own.items) |r, o| {
        const k = try std.fmt.allocPrint(gpa, "{s}\t{d}", .{ t.rel, o.line });
        if (known.contains(k)) continue;
        try add.appendSlice(gpa, r);
        try add.append(gpa, '\n');
        n += 1;
    }
    const old = (try util.readFileOpt(QUARANTINE_TSV)) orelse "";
    try util.writeFile(QUARANTINE_TSV, try std.mem.concat(gpa, u8, &.{ old, add.items }));
    try loadQuarantine();
    try util.writeFile(t.src, try TS.translate(t.rel, qlines(t.rel)));
    return n;
}

// ---- keys and the cache

var compiler_id_v: ?[]const u8 = null;
var compat_id_v: ?[]const u8 = null;

/// Hash of the compiler's own binaries: a service pack or a patched c2.dll is another compiler.
fn compilerId() ![]const u8 {
    if (compiler_id_v) |v| return v;
    if (builtin.os.tag == .windows) {
        const cl = try nativeCl();
        const bindir = util.dirname(cl);
        var names: std.ArrayList([]const u8) = .empty;
        for (try proc.listDir(bindir)) |n| {
            const l = try std.ascii.allocLowerString(gpa, n);
            if (std.mem.endsWith(u8, l, ".exe") or std.mem.endsWith(u8, l, ".dll")) try names.append(gpa, n);
        }
        proc.sortStrings(names.items);
        var parts: std.ArrayList([]const u8) = .empty;
        try parts.appendSlice(gpa, &.{ "native-vc8", VC, cl, util.getenv("INCLUDE") orelse "", util.getenv("LIB") orelse "" });
        for (names.items) |n| try parts.append(gpa, try std.mem.concat(gpa, u8, &.{ n, try objcache.shaFile(try util.join(bindir, n)) }));
        compiler_id_v = try objcache.shaBytes(parts.items);
        return compiler_id_v.?;
    }
    const wp = util.getenv("WINEPREFIX");
    const prefix = if (wp != null and wp.?.len > 0) wp.? else (util.getenv("VC8_WINEPREFIX") orelse "");
    const bindir = try std.fmt.allocPrint(gpa, "{s}/drive_c/vc8/{s}/bin", .{ prefix, VC });
    var names: std.ArrayList([]const u8) = .empty;
    for (try proc.listDir(bindir)) |n| {
        const l = try std.ascii.allocLowerString(gpa, n);
        if (std.mem.endsWith(u8, l, ".exe") or std.mem.endsWith(u8, l, ".dll")) try names.append(gpa, n);
    }
    proc.sortStrings(names.items);
    var parts: std.ArrayList([]const u8) = .empty;
    try parts.append(gpa, VC);
    for (names.items) |n| try parts.append(gpa, try std.mem.concat(gpa, u8, &.{ n, try objcache.shaFile(try util.join(bindir, n)) }));
    compiler_id_v = try objcache.shaBytes(parts.items);
    return compiler_id_v.?;
}

pub fn nativeCl() ![]const u8 {
    const path = util.getenv("VC8_NATIVE_CL") orelse {
        std.debug.print("Windows oracle build requires VC8_NATIVE_CL (absolute path to VC8 cl.exe)\n", .{});
        return error.NativeCompilerPathRequired;
    };
    if (!isAbsolute(path) or !proc.isFile(path)) return error.NativeCompilerPathInvalid;
    return proc.realpath(path);
}

fn compatId() ![]const u8 {
    if (compat_id_v) |v| return v;
    var compat: std.ArrayList([]const u8) = .empty;
    try compat.append(gpa, try util.join(ORACLE, "vc8_compat.h"));
    const shim = try util.join(ORACLE, "shim_include");
    for (try proc.listDir(shim)) |f| try compat.append(gpa, try util.join(shim, f));
    var files: std.ArrayList([]const u8) = .empty;
    for (compat.items) |p| {
        if (proc.isDir(p)) try files.appendSlice(gpa, try proc.allFiles(p)) else try files.append(gpa, p);
    }
    proc.sortStrings(files.items);
    var parts: std.ArrayList([]const u8) = .empty;
    for (files.items) |p| try parts.append(gpa, try std.mem.concat(gpa, u8, &.{ try repoRel(p), try objcache.shaFile(p) }));
    compat_id_v = try objcache.shaBytes(parts.items);
    return compat_id_v.?;
}

fn depsOf(dfile: []const u8) ![]const []const u8 {
    var out: std.ArrayList([]const u8) = .empty;
    const data = (try util.readFileOpt(dfile)) orelse return out.items;
    var it = util.Lines{ .data = data };
    while (it.next()) |l| if (util.strip(l).len > 0) try out.append(gpa, l);
    return out.items;
}

/// Dependencies as a manifest records them: repository-relative, sorted, with their hashes.
fn storeDeps(deps: []const []const u8) ![]objcache.Dep {
    var rels: std.ArrayList([]const u8) = .empty;
    for (deps) |d| try rels.append(gpa, try repoRel(d));
    proc.sortStrings(rels.items);
    var out: std.ArrayList(objcache.Dep) = .empty;
    for (rels.items, 0..) |r, i| {
        if (i > 0 and std.mem.eql(u8, r, rels.items[i - 1])) continue;
        try out.append(gpa, .{ .rel = r, .hash = try hashDep({}, r) });
    }
    return out.items;
}

fn depsKey(base: []const u8, deps: []const objcache.Dep) ![]const u8 {
    var j: std.ArrayList(u8) = .empty;
    try objcache.depsJson(&j, deps);
    return objcache.shaBytes(&.{ base, j.items });
}

fn depRels(deps: []const objcache.Dep) ![]const []const u8 {
    const out = try gpa.alloc([]const u8, deps.len);
    for (deps, 0..) |d, i| out[i] = d.rel;
    return out;
}

/// The .d text a cache entry's dependency list restores.
fn depText(info: std.json.Value) ![]const u8 {
    var out: std.ArrayList(u8) = .empty;
    for (info.object.get("deps").?.array.items) |r| {
        try out.appendSlice(gpa, REPO);
        try out.append(gpa, '/');
        try out.appendSlice(gpa, r.string);
        try out.append(gpa, '\n');
    }
    return out.items;
}

/// The TU's translation with none of its functions blanked: what a compile starts from.
fn unquarantinedText(t: *TU) ![]const u8 {
    if (QUARANTINE.contains(t.rel)) return TS.translate(t.rel, &.{});
    return proc.universalNewlines(try util.readFile(t.src));
}

fn tuBaseKey(t: *TU, rsp_text: []const u8, pch_id: []const u8) ![]const u8 {
    return objcache.shaBytes(&.{ CACHE_VERSION, try compilerId(), rsp_text, extraFlags(t.rel), pch_id, try win(t.src), try win(try t.path(".obj")), try compatId(), try unquarantinedText(t) });
}

/// Put the cached compile `key` in place for TU t. Returns true when it did.
fn restore(t: *TU, key: []const u8) !bool {
    const d = (try objcache.get("obj", key)) orelse return false;
    const info = try objcache.meta(d);
    var rows: std.ArrayList([]const u8) = .empty;
    if (info.object.get("quarantine")) |q| for (q.array.items) |r| try rows.append(gpa, r.string);
    try setQuarantine(t, rows.items);
    util.makeDirs(util.dirname(t.base));
    try objcache.install(try util.join(d, "obj"), try t.path(".obj"));
    try objcache.install(try util.join(d, "log"), try t.path(".log"));
    try writeText(try t.path(".d"), try depText(info));
    try writeText(try t.path(".flags"), extraFlags(t.rel));
    try writeText(try t.path(".key"), key);
    return true;
}

/// Record a successful compile of TU t in the cache, and its key beside the object.
fn store(t: *TU, base: []const u8) !void {
    const deps = try storeDeps(try depsOf(try t.path(".d")));
    const key = try depsKey(base, deps);
    qlock.lock();
    const qrows = quarantineRows(t.rel);
    qlock.unlock();
    var info: std.ArrayList(u8) = .empty;
    try info.appendSlice(gpa, "{\"tu\": ");
    try objcache.jsonString(&info, t.rel);
    try info.appendSlice(gpa, ", \"deps\": ");
    try objcache.stringList(&info, try depRels(deps));
    try info.appendSlice(gpa, ", \"quarantine\": ");
    try objcache.stringList(&info, try qrows);
    try info.append(gpa, '}');
    try objcache.put("obj", key, &.{ .{ .name = "obj", .path = try t.path(".obj") }, .{ .name = "log", .path = try t.path(".log") } }, info.items);
    try objcache.record(base, deps, key);
    try writeText(try t.path(".key"), key);
}

// ---- the compiler's output

/// ^Note: including file:\s*(.*?)\s*$
fn including(line: []const u8) ?[]const u8 {
    const pfx = "Note: including file:";
    if (!std.mem.startsWith(u8, line, pfx)) return null;
    return proc.strip(line[pfx.len..]);
}

/// \b(error|fatal error) C\d{4}\b, anywhere in the line.
pub fn isError(line: []const u8) bool {
    var i: usize = 0;
    while (std.mem.indexOfPos(u8, line, i, "error C")) |p| : (i = p + 1) {
        if (p > 0 and proc.isWord(line[p - 1])) continue;
        const d = p + 7;
        if (d + 4 > line.len) continue;
        if (!(std.ascii.isDigit(line[d]) and std.ascii.isDigit(line[d + 1]) and std.ascii.isDigit(line[d + 2]) and std.ascii.isDigit(line[d + 3]))) continue;
        if (d + 4 < line.len and proc.isWord(line[d + 4])) continue;
        return true;
    }
    return false;
}

/// ^(\S.*?)(?:\(\d+\))? : -> group 1.
fn messageFile(line: []const u8) ?[]const u8 {
    if (line.len == 0 or proc.isSpace(line[0])) return null;
    var j: usize = 1;
    while (j <= line.len) : (j += 1) {
        const r = line[j..];
        if (r.len > 0 and r[0] == '(') {
            var k: usize = 1;
            while (k < r.len and std.ascii.isDigit(r[k])) k += 1;
            if (k > 1 and std.mem.startsWith(u8, r[k..], ") : ")) return line[0..j];
        }
        if (std.mem.startsWith(u8, r, " : ")) return line[0..j];
    }
    return null;
}

/// True if `path` is a complete i386 COFF object with a compiler stamp.
pub fn validObject(path: []const u8) bool {
    const data = (util.readFileOpt(path) catch return false) orelse return false;
    const o = coff.Object.parse(data, path) catch return false;
    return o.compId() != null;
}

const Result = struct { t: *TU, ok: bool, errors: []const []const u8, secs: f64, log: []const []const u8 = &.{} };

fn clearOutputs(t: *TU) !void {
    util.makeDirs(util.dirname(t.base));
    for ([_][]const u8{ ".obj", ".d", ".key" }) |e| {
        const p = try t.path(e);
        if (util.exists(p)) proc.remove(p);
    }
}

fn joinLines(lines: []const []const u8) ![]const u8 {
    var out: std.ArrayList(u8) = .empty;
    for (lines, 0..) |l, i| {
        if (i > 0) try out.append(gpa, '\n');
        try out.appendSlice(gpa, l);
    }
    try out.append(gpa, '\n');
    return out.items;
}

/// Judge one TU's compile from the compiler's output for it: write its log, and on success
/// its dependencies.
fn finish(t: *TU, out: []const u8, returncode: i32, secs: f64) !Result {
    var deps: std.ArrayList([]const u8) = .empty;
    var log: std.ArrayList([]const u8) = .empty;
    for (try proc.splitlines(out)) |line| {
        if (including(line)) |p| {
            if (try linux(p)) |hp| try deps.append(gpa, hp);
        } else try log.append(gpa, line);
    }
    try util.writeFile(try t.path(".log"), try proc.latin1ToUtf8(try joinLines(log.items)));
    var errors: std.ArrayList([]const u8) = .empty;
    for (log.items) |l| if (isError(l)) try errors.append(gpa, l);
    var ok = errors.items.len == 0 and validObject(try t.path(".obj"));
    if (returncode != 0 and returncode != 137 and returncode != -9 and ok) {
        ok = false;
        errors.clearRetainingCapacity();
        try errors.append(gpa, try std.fmt.allocPrint(gpa, "cl.exe exit status {d}", .{returncode}));
    }
    if (ok) {
        proc.sortStrings(deps.items);
        var uniq: std.ArrayList([]const u8) = .empty;
        for (deps.items, 0..) |d, i| if (i == 0 or !std.mem.eql(u8, d, deps.items[i - 1])) try uniq.append(gpa, try proc.latin1ToUtf8(d));
        try util.writeFile(try t.path(".d"), try joinLines(uniq.items));
        try util.writeFile(try t.path(".flags"), extraFlags(t.rel));
    }
    return .{ .t = t, .ok = ok, .errors = errors.items, .secs = secs, .log = log.items };
}

pub fn clCommand(rsp: []const u8, tu: []const u8, out: []const u8, srcs: []const []const u8) ![]const []const u8 {
    var cmd: std.ArrayList([]const u8) = .empty;
    if (builtin.os.tag == .windows) {
        try cmd.append(gpa, try nativeCl());
    } else {
        try cmd.appendSlice(gpa, &.{ try util.join(HERE, "vc8.sh"), "cl" });
    }
    try cmd.appendSlice(gpa, &.{ try std.mem.concat(gpa, u8, &.{ "@", try win(rsp) }), "/showIncludes" });
    var it = std.mem.tokenizeAny(u8, extraFlags(tu), " \t\n\r\x0b\x0c");
    while (it.next()) |f| try cmd.append(gpa, f);
    if (USE_PCH) try cmd.append(gpa, "/Yu" ++ PCH_HEADER);
    try cmd.append(gpa, try std.mem.concat(gpa, u8, &.{ "/Fo", out }));
    for (srcs) |s| try cmd.append(gpa, try win(s));
    return cmd.items;
}

pub fn concatOutput(o: proc.Output) ![]const u8 {
    return std.mem.concat(gpa, u8, &.{ try proc.universalNewlines(o.out), try proc.universalNewlines(o.err) });
}

fn compileOnce(t: *TU, rsp: []const u8) !Result {
    try clearOutputs(t);
    const cmd = try clCommand(rsp, t.rel, try win(try t.path(".obj")), &.{t.src});
    const t0 = proc.now();
    var attempt: u32 = 1;
    while (true) : (attempt += 1) {
        const p = try proc.run(cmd, .{ .timeout_s = COMPILE_TIMEOUT });
        if (p.timed_out) {
            // A compile under wine can block indefinitely; one retry, then give up.
            if (attempt == 2) {
                const e = try gpa.dupe([]const u8, &.{try std.fmt.allocPrint(gpa, "timed out twice after {d}s", .{@as(u32, @intFromFloat(COMPILE_TIMEOUT))})});
                return .{ .t = t, .ok = false, .errors = e, .secs = proc.now() - t0 };
            }
            continue;
        }
        return finish(t, try concatOutput(p), p.status, proc.now() - t0);
    }
}

/// Compile TU t, quarantining what VC8 cannot compile, round by round. `first`: the result
/// of a compile already made (a batch); the rounds go on from there.
fn compileOne(t: *TU, rsp: []const u8, base_key: []const u8, first_in: ?Result) !Result {
    var first = first_in;
    if (first == null) try unquarantine(t);
    var r: Result = undefined;
    var round: u32 = 0;
    while (round < MAX_QUARANTINE_ROUNDS + 1) : (round += 1) {
        if (first) |f| {
            r = f;
            first = null;
        } else r = try compileOnce(t, rsp);
        if (r.ok or try quarantine(t, r.log) == 0) break;
    }
    if (r.ok) try store(t, base_key);
    return r;
}

const Item = struct { t: *TU, base: []const u8, size: u64 };

/// The compiler's output for several sources, cut into what a compile of each alone prints,
/// or null where it cannot be.
fn splitOutput(stdout: []const u8, stderr: []const u8, srcs: []const []const u8) !?[]const []const u8 {
    const n = srcs.len;
    const names = try gpa.alloc([]const u8, n);
    var paths: std.StringHashMapUnmanaged(usize) = .empty;
    for (srcs, 0..) |s, k| {
        names[k] = util.basename(s);
        try paths.put(gpa, try proc.lower(try win(s)), k);
    }
    const parts = try gpa.alloc(std.ArrayList([]const u8), n);
    for (parts) |*p| p.* = .empty;
    var k: ?isize = -1;
    var backend = false;
    for (try proc.splitlines(stdout)) |line| {
        if (backend) {
            if (messageFile(line)) |f| {
                const got = paths.get(try proc.lower(f)) orelse return null;
                k = @intCast(got);
            } else if (proc.strip(line).len > 0 and (k == null or !(line.len > 0 and proc.isSpace(line[0])))) return null;
            if (proc.strip(line).len > 0) try parts[@intCast(k.?)].append(gpa, line);
            continue;
        }
        const kk = k.?;
        if (kk == @as(isize, @intCast(n)) - 1 and std.mem.eql(u8, line, BACKEND)) {
            backend = true;
            k = null;
            continue;
        }
        if (kk + 1 < @as(isize, @intCast(n)) and std.mem.eql(u8, proc.strip(line), names[@intCast(kk + 1)])) k = kk + 1;
        if (k.? < 0) {
            if (proc.strip(line).len > 0) return null;
            continue;
        }
        try parts[@intCast(k.?)].append(gpa, line);
    }
    if (!backend and k.? != @as(isize, @intCast(n)) - 1) return null;
    const errl = try proc.splitlines(stderr);
    const out = try gpa.alloc([]const u8, n);
    for (parts, 0..) |*p, i| {
        try p.appendSlice(gpa, errl);
        out[i] = try joinLines(p.items);
    }
    return out;
}

/// Compile a batch in one cl.exe; each TU is judged on its own output as compileOnce judges
/// it, and one that failed goes on through compileOne's rounds alone.
fn compileBatch(batch: []const Item, rsp: []const u8) ![]Result {
    const res = try gpa.alloc(Result, batch.len);
    if (batch.len == 1) {
        res[0] = try compileOne(batch[0].t, rsp, batch[0].base, null);
        return res;
    }
    for (batch) |it| {
        try unquarantine(it.t);
        try clearOutputs(it.t);
    }
    const first = batch[0].t;
    const srcs = try gpa.alloc([]const u8, batch.len);
    for (batch, 0..) |it, i| srcs[i] = it.t.src;
    const cmd = try clCommand(rsp, first.rel, try std.mem.concat(gpa, u8, &.{ try win(util.dirname(first.base)), "\\" }), srcs);
    const t0 = proc.now();
    const p = try proc.run(cmd, .{ .timeout_s = COMPILE_TIMEOUT * @as(f64, @floatFromInt(batch.len)) });
    const secs = proc.now() - t0;
    var parts: ?[]const []const u8 = null;
    if (!p.timed_out) parts = try splitOutput(try proc.universalNewlines(p.out), try proc.universalNewlines(p.err), srcs);
    const results = try gpa.alloc(Result, batch.len);
    if (parts) |ps| {
        var all_ok = true;
        for (batch, ps, 0..) |it, part, i| {
            results[i] = try finish(it.t, part, 0, secs / @as(f64, @floatFromInt(batch.len)));
            all_ok = all_ok and results[i].ok;
        }
        if (p.status != 0 and p.status != 137 and p.status != -9 and all_ok) parts = null; // it failed, and no source says why
    }
    if (parts == null) {
        for (batch, 0..) |it, i| res[i] = try compileOne(it.t, rsp, it.base, null);
        return res;
    }
    // A TU with no object and no error of its own may have been cut short by another's.
    for (batch, results, 0..) |it, r, i| res[i] = try compileOne(it.t, rsp, it.base, if (r.ok or r.errors.len > 0) r else null);
    return res;
}

/// Split the TUs to compile into lists one cl.exe compiles together: one object directory and
/// one set of flags, near-equal batches of at most `size`, the largest first.
fn batches(todo: []const Item, size: usize) ![][]Item {
    const Group = struct { dir: []const u8, flags: []const u8, items: std.ArrayList(Item) };
    var groups: std.ArrayList(Group) = .empty;
    for (todo) |it| {
        const d = util.dirname(it.t.base);
        const f = extraFlags(it.t.rel);
        var found: ?*Group = null;
        for (groups.items) |*g| if (std.mem.eql(u8, g.dir, d) and std.mem.eql(u8, g.flags, f)) {
            found = g;
            break;
        };
        if (found == null) {
            try groups.append(gpa, .{ .dir = d, .flags = f, .items = .empty });
            found = &groups.items[groups.items.len - 1];
        }
        try found.?.items.append(gpa, it);
    }
    std.mem.sort(Group, groups.items, {}, struct {
        fn lt(_: void, a: Group, b: Group) bool {
            const o = std.mem.order(u8, a.dir, b.dir);
            if (o != .eq) return o == .lt;
            return std.mem.order(u8, a.flags, b.flags) == .lt;
        }
    }.lt);
    var out: std.ArrayList([]Item) = .empty;
    for (groups.items) |*g| {
        const items = g.items.items;
        std.mem.sort(Item, items, {}, struct {
            fn lt(_: void, a: Item, b: Item) bool {
                if (a.size != b.size) return a.size > b.size;
                return std.mem.order(u8, a.t.rel, b.t.rel) == .lt;
            }
        }.lt);
        const m = @max(@as(usize, 1), size);
        const n = (items.len + m - 1) / m;
        for (0..n) |k| {
            var part: std.ArrayList(Item) = .empty;
            var i = k;
            while (i < items.len) : (i += n) try part.append(gpa, items[i]);
            // Two sources one case apart would write the same object on a case-insensitive
            // drive: each gets a batch of its own.
            var clash = false;
            for (part.items, 0..) |a, x| for (part.items[x + 1 ..]) |b| {
                if (std.ascii.eqlIgnoreCase(util.basename(a.t.src), util.basename(b.t.src))) clash = true;
            };
            if (!clash) try out.append(gpa, part.items) else for (part.items) |it| try out.append(gpa, try gpa.dupe(Item, &.{it}));
        }
    }
    std.mem.sort([]Item, out.items, {}, struct {
        fn total(b: []Item) u64 {
            var s: u64 = 0;
            for (b) |it| s += it.size;
            return s;
        }
        fn lt(_: void, a: []Item, b: []Item) bool {
            const ta = total(a);
            const tb = total(b);
            if (ta != tb) return ta > tb;
            return std.mem.order(u8, a[0].t.rel, b[0].t.rel) == .lt;
        }
    }.lt);
    return out.items;
}

// ---- the response file and the precompiled header

const RspCtx = struct { dirs: *std.ArrayList([]const u8) };

fn rspVisit(ctx: RspCtx, w: *proc.WalkDir) !void {
    for (w.files) |f| if (std.mem.endsWith(u8, f, ".h")) {
        try ctx.dirs.append(gpa, w.path);
        return;
    };
}

fn rspPath(flag: []const u8, path: []const u8) ![]const u8 {
    const p = try win(path);
    if (builtin.os.tag == .windows and
        (std.mem.indexOfScalar(u8, p, ' ') != null or std.mem.indexOfScalar(u8, p, '\t') != null))
        return std.fmt.allocPrint(gpa, "{s}\"{s}\"", .{ flag, p });
    return std.mem.concat(gpa, u8, &.{ flag, p });
}

fn writeRsp() ![]const u8 {
    var dirs: std.ArrayList([]const u8) = .empty;
    try proc.walk(SRC, RspCtx{ .dirs = &dirs }, rspVisit);
    proc.sortStrings(dirs.items);
    var lines: std.ArrayList([]const u8) = .empty;
    try lines.append(gpa, "/nologo /c " ++ "");
    lines.items[0] = try std.fmt.allocPrint(gpa, "/nologo /c {s} {s}", .{ CFLAGS, TOOL_CFLAGS });
    try lines.append(gpa, try rspPath("/FI", try util.join(ORACLE, "vc8_compat.h")));
    if (USE_PCH) {
        try writeIfChanged(try util.join(PCHDIR, PCH_HEADER), "// The prefix every TU parses, precompiled once (build.py, ORACLE_PCH=1).\n#include \"" ++ PLATFORM_H ++ "\"\n#include \"Source/globals/globals.h\"\n");
        try writeIfChanged(try util.join(PCHDIR, "oracle_pch.cpp"), "// precompiled-header creation unit\n");
        try lines.append(gpa, "/FI" ++ PCH_HEADER);
        try lines.append(gpa, try rspPath("/I", PCHDIR));
        try lines.append(gpa, try rspPath("/Fp", PCH_FILE));
    } else try lines.append(gpa, try rspPath("/FI", try util.join(SRC, PLATFORM_H)));
    try lines.append(gpa, try rspPath("/I", try util.join(ORACLE, "shim_include")));
    try lines.append(gpa, try rspPath("/I", SRC));
    var prev: ?[]const u8 = null;
    for (dirs.items) |d| {
        if (std.mem.eql(u8, d, SRC)) continue;
        if (prev != null and std.mem.eql(u8, prev.?, d)) continue;
        prev = d;
        try lines.append(gpa, try rspPath("/I", d));
    }
    const rsp = try util.join(BUILD, "cl.rsp");
    try writeIfChanged(rsp, try joinLines(lines.items));
    return rsp;
}

var wine_up = false;

/// Start the wine session before the parallel phase, with no pipes attached: the first wine
/// process spawns wine's services, which inherit its stdout and stderr, and a compile captured
/// through pipes would then wait for an end of file that comes only when they exit.
fn startWine() !void {
    if (builtin.os.tag == .windows) {
        _ = try nativeCl();
        return;
    }
    if (wine_up) return;
    const vp = util.getenv("VC8_WINEPREFIX");
    const prefix = if (vp) |v| v else try std.fmt.allocPrint(gpa, "{s}/q2-dev/asm-matching-toolchain/wine-vc8", .{util.getenv("HOME") orelse ""});
    _ = try proc.run(&.{ "wineserver", "-p" }, .{ .capture = false, .env_set = &.{try std.mem.concat(gpa, u8, &.{ "WINEPREFIX=", prefix })} });
    _ = try proc.run(&.{ try util.join(HERE, "vc8.sh"), "cl", "/nologo" }, .{ .capture = false });
    wine_up = true;
}

const PchResult = struct { err: ?[]const u8, how: []const u8 };

fn buildPch(rsp: []const u8) !PchResult {
    const dep = try util.join(PCHDIR, "oracle.d");
    const pobj = try util.join(PCHDIR, "oracle_pch.obj");
    const sidecar = try util.join(PCHDIR, "oracle.key");
    const base = try objcache.shaBytes(&.{ CACHE_VERSION, "pch", try compilerId(), try util.readFile(rsp), try compatId(), try util.readFile(try util.join(PCHDIR, PCH_HEADER)), try util.readFile(try util.join(PCHDIR, "oracle_pch.cpp")), try win(PCH_FILE), try win(pobj) });
    if (try objcache.lookup(base, {}, hashDep)) |key| {
        const sk = try readKey(sidecar);
        if (sk != null and std.mem.eql(u8, sk.?, key) and util.exists(PCH_FILE) and util.exists(pobj)) return .{ .err = null, .how = "current" };
        if (try objcache.get("pch", key)) |d| {
            try objcache.install(try util.join(d, "pch"), PCH_FILE);
            try objcache.install(try util.join(d, "obj"), pobj);
            try writeText(dep, try depText(try objcache.meta(d)));
            try writeText(sidecar, key);
            return .{ .err = null, .how = "from cache" };
        }
    }
    for ([_][]const u8{ PCH_FILE, dep, sidecar }) |f| if (util.exists(f)) proc.remove(f);
    try startWine();
    var cmd: std.ArrayList([]const u8) = .empty;
    if (builtin.os.tag == .windows) {
        try cmd.append(gpa, try nativeCl());
    } else {
        try cmd.appendSlice(gpa, &.{ try util.join(HERE, "vc8.sh"), "cl" });
    }
    try cmd.appendSlice(gpa, &.{ try std.mem.concat(gpa, u8, &.{ "@", try win(rsp) }), "/showIncludes", "/Yc" ++ PCH_HEADER, try std.mem.concat(gpa, u8, &.{ "/Fo", try win(pobj) }), try win(try util.join(PCHDIR, "oracle_pch.cpp")) });
    const p = try proc.run(cmd.items, .{});
    const lines = try proc.splitlines(try concatOutput(p));
    var deps: std.ArrayList([]const u8) = .empty;
    var first_err: ?[]const u8 = null;
    for (lines) |l| {
        if (including(l)) |inc| if (try linux(inc)) |hp| try deps.append(gpa, try proc.latin1ToUtf8(hp));
        if (first_err == null and isError(l)) first_err = l;
    }
    if (first_err != null or !util.exists(PCH_FILE)) return .{ .err = first_err orelse "no precompiled header produced", .how = "compiled" };
    proc.sortStrings(deps.items);
    var uniq: std.ArrayList([]const u8) = .empty;
    for (deps.items, 0..) |d, i| if (i == 0 or !std.mem.eql(u8, d, deps.items[i - 1])) try uniq.append(gpa, d);
    // '\n'.join(sorted(set(...))) + '\n'
    try util.writeFile(dep, try joinLines(uniq.items));
    const sdeps = try storeDeps(try depsOf(dep));
    const key = try depsKey(base, sdeps);
    var info: std.ArrayList(u8) = .empty;
    try info.appendSlice(gpa, "{\"deps\": ");
    try objcache.stringList(&info, try depRels(sdeps));
    try info.append(gpa, '}');
    try objcache.put("pch", key, &.{ .{ .name = "pch", .path = PCH_FILE }, .{ .name = "obj", .path = pobj } }, info.items);
    try objcache.record(base, sdeps, key);
    try writeText(sidecar, key);
    return .{ .err = null, .how = "compiled" };
}

// ---- support TUs, pruning, status

fn translateSupport(extra_src: []const u8, names: []const []const u8) !usize {
    const out = try util.join(extra_src, SUPPORT);
    util.makeDirs(out);
    var n: usize = 0;
    for (names) |name| {
        const rel = try util.join(SUPPORT, name);
        qlock.lock();
        const lines = try gpa.dupe(u32, qlines(rel));
        qlock.unlock();
        const data = try TS.translate(rel, lines);
        const dst = try util.join(out, name);
        const old = try util.readFileOpt(dst);
        if (old == null or !std.mem.eql(u8, old.?, data)) {
            try util.writeFile(dst, data);
            n += 1;
        }
    }
    for (try proc.listDir(out)) |f| {
        if (!std.mem.endsWith(u8, f, ".cpp")) continue;
        var named = false;
        for (names) |x| named = named or std.mem.eql(u8, x, f);
        if (!named) proc.remove(try util.join(out, f));
    }
    return n;
}

const Keep = struct {
    objdir: []const u8,
    /// Source/globals/ objects belong here.
    globals: bool,
    /// The game's own objects belong here (build/obj).
    game: bool,
    support: []const []const u8 = &.{},

    fn keeps(k: *const Keep, r: []const u8) bool {
        const data = "Source/globals/";
        const exists = util.exists(std.mem.concat(gpa, u8, &.{ SRC, "/", r, ".cpp" }) catch return true);
        if (std.mem.startsWith(u8, r, data)) return k.globals and exists;
        if (k.game and exists) return true;
        for (k.support) |s| if (std.mem.eql(u8, s, r)) return true;
        return false;
    }
};

const PruneSrcCtx = struct { expected: *std.StringHashMapUnmanaged(void), removed: *usize };

fn pruneSrcVisit(ctx: PruneSrcCtx, w: *proc.WalkDir) !void {
    for (w.files) |f| {
        if (!hasSuffix(f, &EXTS)) continue;
        const p = try util.join(w.path, f);
        if (ctx.expected.contains(p)) continue;
        proc.remove(p);
        ctx.removed.* += 1;
    }
}

const PruneObjCtx = struct { keep: *const Keep, removed: *usize };

fn pruneObjVisit(ctx: PruneObjCtx, w: *proc.WalkDir) !void {
    for (w.files) |f| {
        var ext: ?[]const u8 = null;
        for (OBJ_EXTS) |e| if (std.mem.endsWith(u8, f, e)) {
            ext = e;
            break;
        };
        const e = ext orelse continue;
        const stem = try util.join(w.path, f[0 .. f.len - e.len]);
        if (ctx.keep.keeps(try proc.relpath(stem, ctx.keep.objdir))) continue;
        proc.remove(try util.join(w.path, f));
        ctx.removed.* += 1;
    }
}

/// Remove translations of files the tree no longer has, and objects of TUs no longer
/// compiled into a directory: a clean build would not have them.
fn prune(expected: *std.StringHashMapUnmanaged(void), keeps: []const Keep) !usize {
    var removed: usize = 0;
    try proc.walk(SRC, PruneSrcCtx{ .expected = expected, .removed = &removed }, pruneSrcVisit);
    for (keeps) |*k| try proc.walk(k.objdir, PruneObjCtx{ .keep = k, .removed = &removed }, pruneObjVisit);
    return removed;
}

/// box64's orphaned dynarec cache files (<module>.box64.tmp.<pid>.N of a process that is gone).
fn sweepBox64Cache() void {
    const folder = util.getenv("BOX64_DYNACACHE_FOLDER") orelse (std.fmt.allocPrint(gpa, "{s}/.cache/box64", .{util.getenv("HOME") orelse ""}) catch return);
    const names = proc.listDir(folder) catch return;
    for (names) |f| {
        const i = std.mem.indexOf(u8, f, ".box64.tmp.") orelse continue;
        const rest = f[i + ".box64.tmp.".len ..];
        const dot = std.mem.indexOfScalar(u8, rest, '.') orelse continue;
        const pid = rest[0..dot];
        const nn = rest[dot + 1 ..];
        if (pid.len == 0 or nn.len == 0) continue;
        var digits = true;
        for (pid) |ch| digits = digits and std.ascii.isDigit(ch);
        for (nn) |ch| digits = digits and std.ascii.isDigit(ch);
        if (!digits) continue;
        if (util.exists(std.mem.concat(gpa, u8, &.{ "/proc/", pid }) catch continue)) continue;
        proc.remove(util.join(folder, f) catch continue);
    }
}

var status: std.StringHashMapUnmanaged([]const []const u8) = .empty;

fn loadStatus(dir: []const u8) !void {
    const data = (try util.readFileOpt(try util.join(dir, "compile_status.tsv"))) orelse return;
    var it = util.Lines{ .data = data };
    while (it.next()) |line| {
        const parts = try util.split(line, '\t');
        if (parts.len >= 2 and !std.mem.eql(u8, parts[0], "tu")) try status.put(gpa, parts[0], parts);
    }
}

fn writeStatus(dir: []const u8, tus: []const *TU) !void {
    const sorted = try gpa.dupe(*TU, tus);
    std.mem.sort(*TU, sorted, {}, struct {
        fn lt(_: void, a: *TU, b: *TU) bool {
            return std.mem.order(u8, a.rel, b.rel) == .lt;
        }
    }.lt);
    var out: std.ArrayList(u8) = .empty;
    try out.appendSlice(gpa, "tu\tstatus\terrors\tfirst_error\n");
    for (sorted) |t| if (status.get(t.rel)) |row| {
        try out.appendSlice(gpa, try std.mem.join(gpa, "\t", row));
        try out.append(gpa, '\n');
    };
    try util.writeFile(try util.join(dir, "compile_status.tsv"), out.items);
}

// ---- the parallel compile

const Pool = struct {
    work: []const []Item,
    rsp: []const u8,
    next: std.atomic.Value(usize) = .init(0),
    done: usize = 0,
    failed: usize = 0,
    total: usize,
    err: ?anyerror = null,
};

fn fixed1(v: f64) ![]const u8 {
    var o: std.ArrayList(u8) = .empty;
    try util.fixed(&o, "%5.1f", v);
    return o.items;
}

fn worker(p: *Pool) void {
    while (true) {
        const i = p.next.fetchAdd(1, .monotonic);
        if (i >= p.work.len) return;
        const results = compileBatch(p.work[i], p.rsp) catch |e| {
            out_lock.lock();
            p.err = e;
            out_lock.unlock();
            return;
        };
        out_lock.lock();
        defer out_lock.unlock();
        for (results) |r| {
            p.done += 1;
            if (!r.ok) p.failed += 1;
            const first = if (r.errors.len > 0) proc.strip(r.errors[0]) else "";
            const row = gpa.dupe([]const u8, &.{ r.t.rel, if (r.ok) "ok" else "FAIL", std.fmt.allocPrint(gpa, "{d}", .{r.errors.len}) catch "", proc.latin1ToUtf8(first[0..@min(first.len, 300)]) catch "" }) catch continue;
            status.put(gpa, r.t.rel, row) catch {};
            const line = std.fmt.allocPrint(gpa, "[{d}/{d}] {s: <4} {s}s {s}{s}{s}\n", .{ p.done, p.total, if (r.ok) "ok" else "FAIL", fixed1(r.secs) catch "", r.t.rel, if (r.ok) "" else "  ", if (r.ok) "" else proc.latin1ToUtf8(first[0..@min(first.len, 160)]) catch "" }) catch continue;
            util.say(line);
        }
    }
}

fn say(comptime fmt: []const u8, args: anytype) void {
    util.say(std.fmt.allocPrint(gpa, fmt, args) catch return);
}

fn usage() noreturn {
    std.debug.print(
        \\usage: oracle-zig build --root <asm_matching/msvc8> [-j N] [--batch N] [--decode-jobs N] [--force]
        \\                        [--with-data] [--extra DIR] [--support "A.cpp B.cpp"] [PATTERN ...]
        \\
    , .{});
    std.process.exit(2);
}

pub fn main(args: []const []const u8) !void {
    var root: ?[]const u8 = null;
    var jobs: usize = @min(proc.cpuCount(), 6);
    var batch: usize = BATCH;
    if (util.getenv("ORACLE_BATCH")) |b| batch = try std.fmt.parseInt(usize, b, 10);
    var force = false;
    var with_data = false;
    var probe = false;
    var extra: ?[]const u8 = null;
    var support_arg: []const u8 = "";
    var patterns: std.ArrayList([]const u8) = .empty;
    var i: usize = 0;
    while (i < args.len) : (i += 1) {
        const a = args[i];
        const val = struct {
            fn get(as: []const []const u8, k: *usize, arg: []const u8, long: []const u8, short: ?[]const u8) ?[]const u8 {
                if (std.mem.eql(u8, arg, long) or (short != null and std.mem.eql(u8, arg, short.?))) {
                    k.* += 1;
                    if (k.* >= as.len) usage();
                    return as[k.*];
                }
                if (std.mem.startsWith(u8, arg, long) and arg.len > long.len and arg[long.len] == '=') return arg[long.len + 1 ..];
                if (short != null and std.mem.startsWith(u8, arg, short.?) and arg.len > short.?.len) return arg[short.?.len..];
                return null;
            }
        }.get;
        if (val(args, &i, a, "--root", null)) |v| root = v else if (val(args, &i, a, "--jobs", "-j")) |v| jobs = try std.fmt.parseInt(usize, v, 10) else if (val(args, &i, a, "--batch", null)) |v| batch = try std.fmt.parseInt(usize, v, 10) else if (val(args, &i, a, "--decode-jobs", null)) |_| {} else if (val(args, &i, a, "--extra", null)) |v| extra = v else if (val(args, &i, a, "--support", null)) |v| support_arg = v else if (std.mem.eql(u8, a, "--force")) force = true else if (std.mem.eql(u8, a, "--with-data")) with_data = true else if (std.mem.eql(u8, a, "--probe")) probe = true else if (std.mem.eql(u8, a, "-h") or std.mem.eql(u8, a, "--help")) usage() else if (std.mem.startsWith(u8, a, "-")) usage() else try patterns.append(gpa, a);
    }
    // --probe: exit 0 when a ghidra-reconstruct checkout can translate (else build.py must).
    if (probe) std.process.exit(if (try ts_translate.checkout()) |_| 0 else 1);
    try configure(root orelse usage());
    if (support_arg.len > 0 and extra == null) {
        std.debug.print("--support needs --extra\n", .{});
        std.process.exit(2);
    }
    try objcache.init();
    objcache.initLocks();
    if (force) objcache.read_enabled = false;
    qlock.init();
    out_lock.init();
    const gr = (try ts_translate.checkout()) orelse {
        std.debug.print("oracle-zig build: no ghidra-reconstruct checkout (GR, ~/code/ts/gr-daemon); build.py translates without one\n", .{});
        std.process.exit(3);
    };

    util.makeDirs(BUILD);
    _ = try proc.flockPath(try util.join(BUILD, ".build.lock")); // one build of this directory at a time
    if (EXPERIMENT) {
        std.debug.print("*** EXPERIMENT: ORACLE_VC={s} ORACLE_CFLAGS='{s}' -> {s}\n", .{ VC, CFLAGS, BUILD });
        std.debug.print("*** evidence from this build must not be committed\n", .{});
    }
    const t_start = proc.now();
    // build.py's record of a vc8_translate.py mirror no longer describes build/src.
    for ([_][]const u8{ "mirror.key", "mirror.state.pkl" }) |f| {
        const p = try util.join(BUILD, f);
        if (util.exists(p)) proc.remove(p);
    }
    try loadQuarantine();
    TS = .{ .gr = gr, .repo = REPO };
    TS.init();
    defer TS.close();
    const wk = try TS.mirror(SRC, QUARANTINE_TSV);
    say("translate (ghidra-reconstruct {s}): {d} written, {d} unchanged ({s}s)\n", .{ gr, wk[0], wk[1], try fixedN(proc.now() - t_start) });

    var extra_src: ?[]const u8 = null;
    var extra_obj: ?[]const u8 = null;
    var support_names: std.ArrayList([]const u8) = .empty;
    var sit = std.mem.tokenizeAny(u8, support_arg, " \t\n\r\x0b\x0c");
    while (sit.next()) |s| try support_names.append(gpa, s);
    if (extra) |e| {
        var buf: [4096]u8 = undefined;
        const cwd = std.mem.sliceTo(c.getcwd(&buf, buf.len) orelse return error.Cwd, 0);
        const ex = try proc.joinNorm(cwd, e);
        extra_src = try util.join(ex, "src");
        extra_obj = try util.join(ex, "obj");
        const n = try translateSupport(extra_src.?, support_names.items);
        say("support: {d} TUs, {d} written\n", .{ support_names.items.len, n });
    }
    const rsp = try writeRsp();
    const rsp_text = try util.readFile(rsp);

    var pch_id: []const u8 = "";
    if (USE_PCH) {
        const t_pch = proc.now();
        const r = try buildPch(rsp);
        if (r.err) |e| {
            std.debug.print("precompiled header failed: {s}\n", .{try proc.latin1ToUtf8(e)});
            std.process.exit(1);
        }
        say("precompiled header: {s} ({s}s)\n", .{ r.how, try fixedN(proc.now() - t_pch) });
        pch_id = try std.mem.concat(gpa, u8, &.{ try objcache.shaFile(PCH_FILE), try objcache.shaFile(try util.join(PCHDIR, "oracle_pch.obj")) });
    }

    var expected: std.StringHashMapUnmanaged(void) = .empty;
    var tus: std.ArrayList(*TU) = .empty;
    const statusdir_extra = extra orelse BUILD;
    for (try sources(REPO)) |s| {
        const relpath = try proc.normpath(try util.join(s.rel, s.name));
        try expected.put(gpa, try util.join(SRC, relpath), {});
        if (!std.mem.endsWith(u8, s.name, ".cpp")) continue;
        if (std.mem.startsWith(u8, relpath, "Source/globals/")) {
            if (with_data) try tus.append(gpa, try TU.init(relpath, try util.join(SRC, relpath), extra_obj orelse OBJ, statusdir_extra));
        } else try tus.append(gpa, try TU.init(relpath, try util.join(SRC, relpath), OBJ, BUILD));
    }
    for (support_names.items) |name| {
        const relpath = try util.join(SUPPORT, name);
        try tus.append(gpa, try TU.init(relpath, try util.join(extra_src.?, relpath), extra_obj.?, extra.?));
    }
    std.mem.sort(*TU, tus.items, {}, struct {
        fn lt(_: void, a: *TU, b: *TU) bool {
            return std.mem.order(u8, a.rel, b.rel) == .lt;
        }
    }.lt);
    const globals_in_obj = with_data and extra_obj == null;
    var keeps: std.ArrayList(Keep) = .empty;
    try keeps.append(gpa, .{ .objdir = OBJ, .globals = globals_in_obj, .game = true });
    if (extra_obj) |eo| {
        var sup: std.ArrayList([]const u8) = .empty;
        for (support_names.items) |n| try sup.append(gpa, try util.join(SUPPORT, n[0 .. n.len - 4]));
        try keeps.append(gpa, .{ .objdir = eo, .globals = true, .game = false, .support = sup.items });
    }
    const pruned = try prune(&expected, keeps.items);
    if (patterns.items.len > 0) {
        var kept: std.ArrayList(*TU) = .empty;
        for (tus.items) |t| {
            for (patterns.items) |p| if (std.mem.indexOf(u8, t.rel, p) != null) {
                try kept.append(gpa, t);
                break;
            };
        }
        tus = kept;
    }

    // Which TUs need a compile: none whose inputs a stored compile matches.
    const t0 = proc.now();
    var todo: std.ArrayList(Item) = .empty;
    var ready: std.ArrayList(*TU) = .empty;
    var restored: usize = 0;
    var current: usize = 0;
    for (tus.items) |t| {
        const base = try tuBaseKey(t, rsp_text, pch_id);
        const key = try objcache.lookup(base, {}, hashDep);
        if (key) |k| {
            const sk = try readKey(try t.path(".key"));
            if (sk != null and std.mem.eql(u8, sk.?, k) and util.exists(try t.path(".obj")) and util.exists(try t.path(".d"))) {
                current += 1;
                try ready.append(gpa, t);
                continue;
            }
            if (try restore(t, k)) {
                restored += 1;
                try ready.append(gpa, t);
                continue;
            }
        }
        try todo.append(gpa, .{ .t = t, .base = base, .size = proc.fileSize(t.src) });
    }
    const work = try batches(todo.items, batch);
    say("{d} TUs: {d} unchanged, {d} from cache, {d} to compile in {d} cl.exe, {d} jobs{s} ({s}s)\n", .{ tus.items.len, current, restored, todo.items.len, work.len, jobs, if (pruned > 0) try std.fmt.allocPrint(gpa, ", {d} stale files removed", .{pruned}) else "", try fixedN(proc.now() - t0) });

    var statusdirs: std.ArrayList([]const u8) = .empty;
    for (tus.items) |t| {
        var seen = false;
        for (statusdirs.items) |d| seen = seen or std.mem.eql(u8, d, t.statusdir);
        if (!seen) try statusdirs.append(gpa, t.statusdir);
    }
    for (statusdirs.items) |d| try loadStatus(d);
    // Only a compile that succeeded is stored: a row a failed compile left is stale.
    for (ready.items) |t| try status.put(gpa, t.rel, try gpa.dupe([]const u8, &.{ t.rel, "ok", "0", "" }));
    if (todo.items.len > 0) try startWine();
    var pool = Pool{ .work = work, .rsp = rsp, .total = todo.items.len };
    var threads: std.ArrayList(std.Thread) = .empty;
    for (0..@max(@as(usize, 1), @min(jobs, work.len))) |_| try threads.append(gpa, try std.Thread.spawn(.{}, worker, .{&pool}));
    for (threads.items) |th| th.join();
    if (pool.err) |e| return e;

    for (tus.items) |t| if (!status.contains(t.rel) and util.exists(try t.path(".obj"))) try status.put(gpa, t.rel, try gpa.dupe([]const u8, &.{ t.rel, "ok", "0", "" }));
    for (statusdirs.items) |d| {
        var group: std.ArrayList(*TU) = .empty;
        for (tus.items) |t| if (std.mem.eql(u8, t.statusdir, d)) try group.append(gpa, t);
        if (patterns.items.len > 0) {
            // Keep the rows of the TUs this run did not look at.
            var seen: std.StringHashMapUnmanaged(void) = .empty;
            for (group.items) |t| try seen.put(gpa, t.rel, {});
            if (try util.readFileOpt(try util.join(d, "compile_status.tsv"))) |data| {
                var it = util.Lines{ .data = data };
                while (it.next()) |line| {
                    const parts = try util.split(line, '\t');
                    if (!std.mem.eql(u8, parts[0], "tu") and !seen.contains(parts[0]) and util.exists(try util.join(SRC, parts[0])))
                        try group.append(gpa, try TU.init(parts[0], try util.join(SRC, parts[0]), OBJ, d));
                }
            }
        }
        try writeStatus(d, group.items);
    }
    var nok: usize = 0;
    for (tus.items) |t| if (status.get(t.rel)) |row| {
        if (row.len > 1 and std.mem.eql(u8, row[1], "ok")) nok += 1;
    };
    sweepBox64Cache();
    say("compiled {d}/{d} TUs OK ({d} failed this run) in {d}s\n", .{ nok, tus.items.len, pool.failed, @as(u64, @intFromFloat(@round(proc.now() - t0))) });
    TS.close();
    std.process.exit(if (nok == tus.items.len) 0 else 1);
}

fn fixedN(v: f64) ![]const u8 {
    var o: std.ArrayList(u8) = .empty;
    try util.fixed(&o, "%.1f", v);
    return o.items;
}
