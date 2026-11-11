//! snippet (snippet.py): compile a variant of some code and compare it with the original.
//!
//!   oracle-zig snippet [--root R] FILE [--func NAME [--ref ADDR]] [--cflags FLAGS] [--tu TU]
//!
//! The experiment bench for a source hypothesis ("was it `if (a && b)` or a ternary?").
//! FILE (inside the repository; build/scratch/ is the place) is compiled with the baseline
//! compiler, flags and context of every TU - the same response file and precompiled header,
//! so the tree's types and globals are there - into build/scratch/, never into build/obj.
//! FILE is passed as is: write it in what VC8 accepts (a copy of the translated TU,
//! build/src/<tu>.cpp, is a ready start).
//!
//! Without --func every function of the object is printed. With --func, that function is
//! printed beside the original - at ADDR, or where the function map puts a function of that
//! name - with the status the audit would give it (exit status 0 on MATCH, else 2). The
//! module's own flags (flags.tsv) are those of the TU the function map files the function
//! under, or of --tu. --cflags adds flags after those (an experiment: not evidence).
const std = @import("std");
const c = @import("c");
const audit = @import("audit.zig");
const cli = @import("cli.zig");
const compare = @import("compare.zig");
const compile = @import("compile.zig");
const context = @import("context.zig");
const objfuncs = @import("objfuncs.zig");
const ours = @import("ours.zig");
const proc = @import("proc.zig");
const util = @import("util.zig");

const gpa = util.gpa;

const USAGE =
    \\usage: oracle snippet [-h] [--func FUNC] [--ref REF] [--cflags CFLAGS] [--tu TU] file
    \\
    \\Compile a variant of some code (FILE, inside the repository; build/scratch/ is the
    \\place) with the baseline compiler, flags and context of every TU, and compare it
    \\with the original. Without --func every function of the object is printed; with
    \\--func, that function beside the original (at --ref ADDR, or where the function map
    \\puts a function of that name), exit status 0 on MATCH, else 2. --tu: the TU whose
    \\flags.tsv row applies (default: the function's, from the map). --cflags adds flags.
    \\
;

fn mtimeNs(path: []const u8) ?i128 {
    return if (proc.stat(path)) |s| s.mtime_ns else null;
}

/// shutil.copy2: the bytes, then the source's times.
fn copy2(src: []const u8, dst: []const u8) !void {
    try util.writeFile(dst, try util.readFile(src));
    const sz = try util.z(src);
    var st: c.struct_stat = undefined;
    if (c.stat(sz.ptr, &st) != 0) return error.Stat;
    var times = [2]c.struct_timespec{ st.st_atimespec, st.st_mtimespec };
    const dz = try util.z(dst);
    _ = c.utimensat(c.AT_FDCWD, dz.ptr, &times, 0);
}

/// A copy of the precompiled header: a build running alongside rewrites (and locks) the
/// original whenever a header it was made from changes.
fn pchCopy(scratch: []const u8) ![]const u8 {
    const copy = try util.join(scratch, util.basename(compile.PCH_FILE));
    const tmp = try std.fmt.allocPrint(gpa, "{s}.tmp", .{copy});
    for (0..60) |_| {
        if (mtimeNs(copy)) |cm| if (mtimeNs(compile.PCH_FILE)) |pm| if (cm >= pm) break;
        // Only a header that stayed the same through the copy is complete.
        const before = proc.stat(compile.PCH_FILE) orelse return error.NoPch;
        try copy2(compile.PCH_FILE, tmp);
        const after = proc.stat(compile.PCH_FILE) orelse return error.NoPch;
        if (before.mtime_ns == after.mtime_ns and before.size == after.size and @as(u64, @intCast(after.size)) == proc.fileSize(tmp)) {
            try proc.rename(tmp, copy);
            break;
        }
        _ = c.sleep(5);
    }
    return copy;
}

/// Compile `path` in the tree's context: (object path or null when the compile failed, log).
fn compileFile(path: []const u8, cflags: []const u8) !struct { ?[]const u8, []const []const u8 } {
    const scratch = try util.join(compile.BUILD, "scratch");
    util.makeDirs(scratch);
    const base = util.basename(path);
    const stem = if (std.mem.lastIndexOfScalar(u8, base, '.')) |d| (if (d > 0) base[0..d] else base) else base;
    const obj = try std.fmt.allocPrint(gpa, "{s}/{s}.obj", .{ scratch, stem });
    if (util.exists(obj)) proc.remove(obj);
    const rsp = try util.join(compile.BUILD, "cl.rsp");
    if (!util.exists(rsp)) cli.exitMsg("no {s} - run `oracle build` once first", .{rsp});
    var cmd: std.ArrayList([]const u8) = .empty;
    try cmd.appendSlice(gpa, &.{ try util.join(compile.HERE, "vc8.sh"), "cl", try std.mem.concat(gpa, u8, &.{ "@", try compile.win(rsp) }) });
    if (compile.USE_PCH) {
        const copy = try pchCopy(scratch);
        try cmd.appendSlice(gpa, &.{ "/Yu" ++ compile.PCH_HEADER, try std.mem.concat(gpa, u8, &.{ "/Fp", try compile.win(copy) }) });
    }
    var it = std.mem.tokenizeAny(u8, cflags, " \t\n\r\x0b\x0c");
    while (it.next()) |f| try cmd.append(gpa, f);
    try cmd.appendSlice(gpa, &.{ try std.mem.concat(gpa, u8, &.{ "/Fo", try compile.win(obj) }), try compile.win(path) });
    const p = try proc.run(cmd.items, .{ .timeout_s = compile.COMPILE_TIMEOUT });
    if (p.timed_out) cli.exitMsg("subprocess.TimeoutExpired: the compile took longer than {d} seconds", .{@as(u32, @intFromFloat(compile.COMPILE_TIMEOUT))});
    var log: std.ArrayList([]const u8) = .empty;
    for (try proc.splitlines(try compile.concatOutput(p))) |l| if (proc.strip(l).len > 0) try log.append(gpa, l);
    // As in the build: the exit status is not trusted under wine, the object is.
    var ok = compile.validObject(obj);
    for (log.items) |l| if (compile.isError(l)) {
        ok = false;
    };
    return .{ if (ok) obj else null, log.items };
}

fn matches(name: []const u8, func: []const u8) bool {
    return std.mem.eql(u8, name, func) or (std.mem.endsWith(u8, name, func) and name.len >= func.len + 2 and
        std.mem.eql(u8, name[name.len - func.len - 2 .. name.len - func.len], "::"));
}

/// int(s, 16)
fn parseHex(s: []const u8) ?u32 {
    var t = proc.strip(s);
    if (t.len > 2 and t[0] == '0' and (t[1] == 'x' or t[1] == 'X')) t = t[2..];
    return std.fmt.parseInt(u32, t, 16) catch null;
}

fn absPath(p: []const u8) ![]const u8 {
    if (p.len > 0 and p[0] == '/') return proc.normpath(p);
    var buf: [4096]u8 = undefined;
    const cwd = std.mem.sliceTo(c.getcwd(&buf, buf.len) orelse return error.Cwd, 0);
    return proc.joinNorm(cwd, p);
}

pub fn main(args: []const []const u8) !void {
    const p = cli.parse(args, &.{ .{ .long = "--root" }, .{ .long = "--func" }, .{ .long = "--ref" }, .{ .long = "--cflags" }, .{ .long = "--tu" } }, USAGE);
    if (p.pos.len != 1) {
        cli.sayErr(USAGE);
        std.process.exit(2);
    }
    const ref_addr: ?u32 = if (p.get("--ref")) |r| parseHex(r) orelse {
        cli.sayErr(USAGE);
        cli.exitMsg("error: argument --ref: invalid value: '{s}'", .{r});
    } else null;
    const func = p.get("--func");
    const root = try cli.root(p.get("--root"));
    try compile.configure(root);
    const x = try context.Context.open(root);
    const rows = try audit.loadFuncmap(try x.cfg.evidence("funcmap.tsv"));
    var tu_of: ?[]const u8 = p.get("--tu");
    if (tu_of == null and func != null) {
        var tus: std.StringArrayHashMapUnmanaged(void) = .empty;
        for (rows) |r| if ((ref_addr == null or r.s == ref_addr.?) and matches(r.name, func.?)) try tus.put(gpa, r.tu, {});
        if (tus.count() == 1) tu_of = tus.keys()[0];
    }
    const extra = if (tu_of) |t| compile.extraFlags(t) else "";
    const cflags = proc.strip(try std.fmt.allocPrint(gpa, "{s} {s}", .{ extra, p.get("--cflags") orelse "" }));
    const obj_opt, const log = try compileFile(try absPath(p.pos[0]), cflags);
    var out = cli.Out{};
    const obj = obj_opt orelse {
        for (log, 0..) |l, i| {
            if (i > 0) try out.raw("\n");
            try out.lat(l);
        }
        try out.raw("\n");
        try out.flush();
        std.process.exit(1);
    };
    var syms = try x.symMap(rows, try x.libRows());
    const rel = try proc.relpath(obj, compile.BUILD);
    const tu = rel[0 .. rel.len - 4];
    var fns: std.ArrayList(ours.OurFunc) = .empty;
    for (try objfuncs.functions(&x.d, obj, try util.readFile(obj), false), 0..) |f, k| try fns.append(gpa, .{ .tu = tu, .order = k, .f = f });
    var viewer = compare.Viewer{ .d = &x.d, .with_text = true };
    if (func == null) {
        for (fns.items) |*f| {
            try out.lat(f.f.name);
            try out.print("  ({d} insns)\n", .{f.f.decoded.insns.items.len});
            for (try viewer.ourView(f, &syms)) |i| {
                try out.print("    {x:>4}  ", .{i.addr});
                try out.latLine(viewer.intern.str(i.text));
            }
        }
        try out.flush();
        return;
    }
    var hits: std.ArrayList(*const ours.OurFunc) = .empty;
    for (fns.items) |*f| if (matches(f.f.name, func.?)) try hits.append(gpa, f);
    if (hits.items.len != 1) cli.exitMsg("{s}: {d} functions of that name in {s}", .{ func.?, hits.items.len, obj });
    const f = hits.items[0];
    const start = ref_addr orelse blk: {
        var cands: std.ArrayList(u32) = .empty;
        for (rows) |r| if (std.mem.eql(u8, r.name, f.f.name) or (matches(r.name, func.?) and !std.mem.eql(u8, r.name, func.?))) try cands.append(gpa, r.s);
        if (cands.items.len != 1) cli.exitMsg("{s}: give --ref, the function map has {d} candidates", .{ func.?, cands.items.len });
        break :blk cands.items[0];
    };
    const m = x.model.getPtr(start) orelse cli.exitMsg("KeyError: {d}", .{start});
    const ov = try viewer.ourView(f, &syms);
    const rv = try viewer.refViewSpan(start, m.f.end, &x.model);
    const r = try compare.compare(ov, rv);
    try out.lat(f.f.name);
    try out.print("  vs original {x:0>8}\n", .{start});
    try out.print("status {s}  insns ours {d} / original {d}  byte diffs {d}  similarity ", .{ r.status, r.our_insns, r.ref_insns, r.byte_diffs });
    try util.fixed(&out.buf, "%.3f", r.similarity);
    try out.print("  refs differ {d}, unverified {d}\n", .{ r.ref_diffs, r.unverified });
    for (try compare.sideBySide(&viewer, ov, rv, 66)) |l| try out.latLine(l);
    try out.flush();
    std.process.exit(if (std.mem.eql(u8, r.status, "MATCH")) 0 else 2);
}
