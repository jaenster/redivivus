//! The read-only analyses over the audit, one subcommand each:
//!
//!   oracle-zig objdiff <objdir A> <objdir B> [PATTERN ...]          (objdiff.py)
//!   oracle-zig diffgrams [--root R] [--top N] [--max-len K] [--examples]   (diffgrams.py)
//!   oracle-zig sizetrend [--root R] [--audit A] [--funcmap F] [--out O] [--label T]  (sizetrend.py)
//!   oracle-zig abcompare <baseline audit.tsv> <experiment audit.tsv> [--depth N] [--list]
//!                                                                    (abcompare.py)
//!   oracle-zig tulayout [--root R] [--tu PATTERN] [--intruders]     (tulayout.py)
//!
//! Each prints what its Python script printed, byte for byte.
const std = @import("std");
const audit = @import("audit.zig");
const cli = @import("cli.zig");
const compare = @import("compare.zig");
const context = @import("context.zig");
const dec = @import("decode.zig");
const difflib = @import("difflib.zig");
const funcmap = @import("funcmap.zig");
const objfuncs = @import("objfuncs.zig");
const ours = @import("ours.zig");
const proc = @import("proc.zig");
const util = @import("util.zig");

const gpa = util.gpa;

fn lessStr(_: void, a: []const u8, b: []const u8) bool {
    return std.mem.order(u8, a, b) == .lt;
}

/// The characters of UTF-8 text `s`, the first n of them (s[:n] of a str read from a file).
fn cpPrefix(s: []const u8, n: usize) []const u8 {
    var i: usize = 0;
    var k: usize = 0;
    while (i < s.len and k < n) : (k += 1) i += std.unicode.utf8ByteSequenceLength(s[i]) catch 1;
    return s[0..@min(i, s.len)];
}

fn cpLen(s: []const u8) usize {
    return std.unicode.utf8CountCodepoints(s) catch s.len;
}

/// '%-Ns' of UTF-8 text.
fn padRight(out: *cli.Out, s: []const u8, w: usize) !void {
    try out.raw(s);
    var k = cpLen(s);
    while (k < w) : (k += 1) try out.raw(" ");
}

// ---- objdiff

const OBJDIFF_USAGE =
    \\objdiff - do two builds contain the same code? Function by function.
    \\
    \\    oracle objdiff <objdir A> <objdir B> [PATTERN ...]
    \\
;

const Fn = struct { code: []const u8, refs: []objfuncs.OurRef };

fn objFunctions(d: *dec.Decoder, path: []const u8) !std.StringArrayHashMapUnmanaged(Fn) {
    var out: std.StringArrayHashMapUnmanaged(Fn) = .empty;
    for (try objfuncs.functions(d, path, try util.readFile(path), false)) |f| {
        var refs: std.ArrayList(objfuncs.OurRef) = .empty;
        var it = f.relocs.iterator();
        while (it.next()) |e| try refs.append(gpa, e.value_ptr.*);
        std.mem.sort(objfuncs.OurRef, refs.items, {}, struct {
            fn lt(_: void, a: objfuncs.OurRef, b: objfuncs.OurRef) bool {
                return a.off < b.off;
            }
        }.lt);
        try out.put(gpa, f.symbol, .{ .code = f.code, .refs = refs.items });
    }
    return out;
}

fn sameFn(a: Fn, b: Fn) bool {
    if (!std.mem.eql(u8, a.code, b.code) or a.refs.len != b.refs.len) return false;
    for (a.refs, b.refs) |x, y| {
        if (x.off != y.off or x.kind != y.kind) return false;
        if (x.kind == .self) {
            if (x.int != y.int) return false;
        } else if (!std.mem.eql(u8, x.s, y.s)) return false;
    }
    return true;
}

const ObjdiffCtx = struct { a_dir: []const u8, b_dir: []const u8, pats: []const []const u8, d: *dec.Decoder, out: *cli.Out, objs: usize = 0, same: usize = 0, differ: usize = 0 };

fn objdiffVisit(x: *ObjdiffCtx, w: *proc.WalkDir) !void {
    var have: std.StringHashMapUnmanaged(void) = .empty;
    for (w.files) |f| try have.put(gpa, f, {});
    const files = try gpa.dupe([]const u8, w.files);
    std.mem.sort([]const u8, files, {}, lessStr);
    for (files) |f| {
        if (!std.mem.endsWith(u8, f, ".obj") or !have.contains(try std.fmt.allocPrint(gpa, "{s}.d", .{f[0 .. f.len - 4]}))) continue;
        const rel = try proc.relpath(try util.join(w.path, f), x.a_dir);
        const other = try util.join(x.b_dir, rel);
        if (x.pats.len > 0) {
            var any = false;
            for (x.pats) |p| if (std.mem.indexOf(u8, rel, p) != null) {
                any = true;
            };
            if (!any) continue;
        }
        if (!util.exists(other)) continue;
        x.objs += 1;
        var fa = try objFunctions(x.d, try util.join(x.a_dir, rel));
        var fb = try objFunctions(x.d, other);
        var syms: std.ArrayList([]const u8) = .empty;
        for (fa.keys()) |k| try syms.append(gpa, k);
        for (fb.keys()) |k| if (!fa.contains(k)) try syms.append(gpa, k);
        std.mem.sort([]const u8, syms.items, {}, lessStr);
        for (syms.items) |sym| {
            const a = fa.get(sym);
            const b = fb.get(sym);
            if (a == null or b == null) {
                try x.out.print("{s}: ", .{rel});
                try x.out.lat(sym);
                try x.out.print(" only in {s}\n", .{if (a != null) x.a_dir else x.b_dir});
                x.differ += 1;
            } else if (!sameFn(a.?, b.?)) {
                try x.out.print("{s}: ", .{rel});
                try x.out.lat(sym);
                try x.out.raw(" differs\n");
                x.differ += 1;
            } else x.same += 1;
        }
    }
}

pub fn objdiffMain(args: []const []const u8) !void {
    if (args.len < 2) {
        cli.sayErr(OBJDIFF_USAGE);
        std.process.exit(1);
    }
    var d = try dec.Decoder.init(gpa);
    var out = cli.Out{};
    var x = ObjdiffCtx{ .a_dir = args[0], .b_dir = args[1], .pats = args[2..], .d = &d, .out = &out };
    try proc.walk(args[0], &x, objdiffVisit);
    try out.print("{d} objects, {d} functions identical, {d} differ\n", .{ x.objs, x.same, x.differ });
    try out.flush();
    std.process.exit(if (x.differ > 0) 1 else 0);
}

// ---- diffgrams

const DIFFGRAMS_USAGE =
    \\usage: oracle diffgrams [-h] [--top TOP] [--max-len MAX_LEN] [--examples]
    \\
    \\The most frequent local differences, over every pair: each differing stretch of
    \\at most K instructions on either side, registers written R and numbers N.
    \\
;

fn isDigit(ch: u8) bool {
    return ch >= '0' and ch <= '9';
}

fn wordAt(s: []const u8, i: usize) bool {
    return i < s.len and proc.isWord(s[i]);
}

/// \b at position i.
fn boundary(s: []const u8, i: usize) bool {
    const before = i > 0 and proc.isWord(s[i - 1]);
    return before != wordAt(s, i);
}

const REGS = [_][]const u8{ "e?x", "e?i", "ebp", "esp", "?!", "?x", "si", "di" };

/// The length of REG's alternative k at s[i..], or null: e[abcd]x|e[sd]i|ebp|esp|[abcd][lh]|[abcd]x|si|di.
fn regAlt(s: []const u8, i: usize, k: usize) ?usize {
    const abcd = "abcd";
    const r = s[i..];
    switch (k) {
        0 => if (r.len >= 3 and r[0] == 'e' and std.mem.indexOfScalar(u8, abcd, r[1]) != null and r[2] == 'x') return 3,
        1 => if (r.len >= 3 and r[0] == 'e' and (r[1] == 's' or r[1] == 'd') and r[2] == 'i') return 3,
        2 => if (std.mem.startsWith(u8, r, "ebp")) return 3,
        3 => if (std.mem.startsWith(u8, r, "esp")) return 3,
        4 => if (r.len >= 2 and std.mem.indexOfScalar(u8, abcd, r[0]) != null and (r[1] == 'l' or r[1] == 'h')) return 2,
        5 => if (r.len >= 2 and std.mem.indexOfScalar(u8, abcd, r[0]) != null and r[1] == 'x') return 2,
        6 => if (std.mem.startsWith(u8, r, "si")) return 2,
        7 => if (std.mem.startsWith(u8, r, "di")) return 2,
        else => {},
    }
    return null;
}

/// REG.sub('R', s): \b(alternatives)\b.
fn subRegs(s: []const u8) ![]const u8 {
    var out: std.ArrayList(u8) = .empty;
    var i: usize = 0;
    while (i < s.len) {
        if (boundary(s, i)) {
            var hit: ?usize = null;
            for (0..REGS.len) |k| if (regAlt(s, i, k)) |n| if (boundary(s, i + n)) {
                hit = n;
                break;
            };
            if (hit) |n| {
                try out.append(gpa, 'R');
                i += n;
                continue;
            }
        }
        try out.append(gpa, s[i]);
        i += 1;
    }
    return out.items;
}

/// NUM.sub('N', s): 0x[0-9a-f]+|\b\d+\b.
fn subNums(s: []const u8) ![]const u8 {
    var out: std.ArrayList(u8) = .empty;
    var i: usize = 0;
    while (i < s.len) {
        if (i + 2 < s.len and s[i] == '0' and s[i + 1] == 'x' and (isDigit(s[i + 2]) or (s[i + 2] >= 'a' and s[i + 2] <= 'f'))) {
            var j = i + 2;
            while (j < s.len and (isDigit(s[j]) or (s[j] >= 'a' and s[j] <= 'f'))) j += 1;
            try out.append(gpa, 'N');
            i = j;
            continue;
        }
        if (isDigit(s[i]) and boundary(s, i)) {
            var j = i;
            while (j < s.len and isDigit(s[j])) j += 1;
            if (boundary(s, j)) {
                try out.append(gpa, 'N');
                i = j;
                continue;
            }
        }
        try out.append(gpa, s[i]);
        i += 1;
    }
    return out.items;
}

fn shape(v: *const compare.Viewer, insn: *const compare.NInsn) ![]const u8 {
    return subNums(try subRegs(v.intern.str(insn.key)));
}

pub fn diffgramsMain(args: []const []const u8) !void {
    const p = cli.parse(args, &.{ .{ .long = "--root" }, .{ .long = "--top" }, .{ .long = "--max-len" }, .{ .long = "--examples", .value = false } }, DIFFGRAMS_USAGE);
    if (p.pos.len > 0) {
        cli.sayErr(DIFFGRAMS_USAGE);
        std.process.exit(2);
    }
    const top: usize = @intCast(p.int("--top", 60));
    const max_len: usize = @intCast(p.int("--max-len", 3));
    const x = try context.Context.open(try cli.root(p.get("--root")));
    const rows = try audit.loadFuncmap(try x.cfg.evidence("funcmap.tsv"));
    var syms = try x.symMap(rows, try x.libRows());
    var status: std.AutoHashMapUnmanaged(u32, []const u8) = .empty;
    {
        var it = util.Lines{ .data = try util.readFile(try x.cfg.evidence("audit.tsv")) };
        while (it.next()) |line| {
            const f = try util.split(line, '\t');
            if (!std.mem.eql(u8, f[0], "start") and f.len > 9) try status.put(gpa, try util.hex(f[0]), f[9]);
        }
    }
    var objs: std.StringHashMapUnmanaged(std.StringHashMapUnmanaged(*const ours.OurFunc)) = .empty;
    var viewer = compare.Viewer{ .d = &x.d };
    const Gram = struct { a: []const []const u8, b: []const []const u8 };
    var grams: std.StringArrayHashMapUnmanaged(struct { g: Gram, n: usize, ex: std.ArrayList([]const u8) }) = .empty;
    var n: usize = 0;
    for (rows) |r| {
        const st = status.get(r.s) orelse continue;
        if (std.mem.eql(u8, st, "MATCH") or std.mem.eql(u8, st, "QUARANTINE") or std.mem.eql(u8, st, "MISSING")) continue;
        const g = try objs.getOrPut(gpa, r.tu);
        if (!g.found_existing) {
            g.value_ptr.* = .empty;
            if (util.exists(try std.fmt.allocPrint(gpa, "{s}/obj/{s}.obj", .{ x.cfg.build, r.tu }))) {
                for (try x.loadObject(r.tu)) |*f| try g.value_ptr.put(gpa, f.f.symbol, f);
            }
        }
        const f = g.value_ptr.get(r.sym) orelse continue;
        const m = x.model.getPtr(r.s) orelse return error.KeyError;
        const ov = try viewer.ourView(f, &syms);
        const rv = try viewer.refViewSpan(r.s, m.f.end, &x.model);
        n += 1;
        const am = try gpa.alloc(u32, ov.len);
        for (ov, 0..) |i, k| am[k] = i.mnemonic;
        const bm = try gpa.alloc(u32, rv.len);
        for (rv, 0..) |i, k| bm[k] = i.mnemonic;
        var sm = try difflib.SequenceMatcher.init(gpa, am, bm);
        defer sm.deinit();
        var seen: std.StringHashMapUnmanaged(void) = .empty;
        for (try sm.opcodes()) |op| {
            if (op.tag == .equal or op.ci2 - op.ci > max_len or op.cj2 - op.cj > max_len) continue;
            var key: std.ArrayList(u8) = .empty;
            const a = try gpa.alloc([]const u8, op.ci2 - op.ci);
            for (ov[op.ci..op.ci2], 0..) |*i, k| a[k] = try shape(&viewer, i);
            const b = try gpa.alloc([]const u8, op.cj2 - op.cj);
            for (rv[op.cj..op.cj2], 0..) |*i, k| b[k] = try shape(&viewer, i);
            // (tuple of ours, tuple of the original's), each shape length-prefixed.
            try key.print(gpa, "{d}|", .{a.len});
            for (a) |s| try key.print(gpa, "{d}:{s}", .{ s.len, s });
            for (b) |s| try key.print(gpa, "{d}:{s}", .{ s.len, s });
            if (seen.contains(key.items)) continue;
            try seen.put(gpa, key.items, {});
            const e = try grams.getOrPut(gpa, key.items);
            if (!e.found_existing) e.value_ptr.* = .{ .g = .{ .a = a, .b = b }, .n = 0, .ex = .empty };
            e.value_ptr.n += 1;
            if (e.value_ptr.ex.items.len < 3) try e.value_ptr.ex.append(gpa, r.name);
        }
    }
    var out = cli.Out{};
    try out.print("{d} functions that do not match; differing stretches by shape (ours => original):\n", .{n});
    const order = try gpa.alloc(usize, grams.count());
    for (order, 0..) |*o, i| o.* = i;
    const vals = grams.values();
    std.mem.sort(usize, order, vals, struct {
        fn lt(v: @TypeOf(vals), a: usize, b: usize) bool {
            return v[a].n > v[b].n;
        }
    }.lt);
    for (order[0..@min(top, order.len)]) |i| {
        const e = vals[i];
        try out.print("{d:>6}  ", .{e.n});
        try joinShapes(&out, e.g.a);
        try out.raw("  =>  ");
        try joinShapes(&out, e.g.b);
        if (p.flag("--examples")) {
            try out.raw("\n          e.g. ");
            for (e.ex.items, 0..) |name, k| {
                if (k > 0) try out.raw(", ");
                try out.raw(name);
            }
        }
        try out.raw("\n");
    }
    try out.flush();
}

fn joinShapes(out: *cli.Out, xs: []const []const u8) !void {
    if (xs.len == 0) return out.raw("-");
    for (xs, 0..) |s, k| {
        if (k > 0) try out.raw(" ; ");
        try out.lat(s);
    }
}

// ---- sizetrend

const SIZETREND_USAGE =
    \\usage: oracle sizetrend [-h] [--audit AUDIT] [--funcmap FUNCMAP] [--out OUT] [--label LABEL]
    \\
    \\MATCH rate and closeness by function size, from evidence/audit.tsv.
    \\
;

const Bucket = struct { lo: usize, hi: usize, name: []const u8 };
const BUCKETS = [_]Bucket{
    .{ .lo = 1, .hi = 1, .name = "1" },           .{ .lo = 2, .hi = 3, .name = "2-3" },         .{ .lo = 4, .hi = 9, .name = "4-9" },
    .{ .lo = 10, .hi = 29, .name = "10-29" },     .{ .lo = 30, .hi = 99, .name = "30-99" },     .{ .lo = 100, .hi = 299, .name = "100-299" },
    .{ .lo = 300, .hi = 1_000_000_000, .name = "300+" },
};

/// Diff hunks: runs of adjacent differing lines of the side-by-side listing.
fn hunks(v: *const compare.Viewer, o: []const compare.NInsn, rf: []const compare.NInsn) !usize {
    var n: usize = 0;
    var prev = false;
    for (try compare.sideBySide(v, o, rf, 66)) |line| {
        const d = line.len > 67 and line[67] != ' ';
        if (d and !prev) n += 1;
        prev = d;
    }
    return n;
}

pub fn sizetrendMain(args: []const []const u8) !void {
    const p = cli.parse(args, &.{ .{ .long = "--root" }, .{ .long = "--audit" }, .{ .long = "--funcmap" }, .{ .long = "--out" }, .{ .long = "--label" } }, SIZETREND_USAGE);
    if (p.pos.len > 0) {
        cli.sayErr(SIZETREND_USAGE);
        std.process.exit(2);
    }
    const x = try context.Context.open(try cli.root(p.get("--root")));
    const audit_path = p.get("--audit") orelse try x.cfg.evidence("audit.tsv");
    const funcmap_path = p.get("--funcmap") orelse try x.cfg.evidence("funcmap.tsv");
    const label = p.get("--label") orelse "";

    const librows = try x.libRows();
    var runtime: std.AutoHashMapUnmanaged(u32, void) = .empty;
    _ = try audit.runtimeStarts(&x.model, librows, try x.cfg.evidence("runtime.tsv"), &runtime);
    try audit.loadStarts(try x.cfg.evidence("prebuilt.tsv"), &x.model, &runtime);
    var eh = try audit.ehStarts(&x.ref.img, &x.model);

    const rows = try audit.loadFuncmap(funcmap_path);
    var syms = try x.symMap(rows, librows);
    var sym_of: std.StringHashMapUnmanaged([]const u8) = .empty;
    for (rows) |r| try sym_of.put(gpa, try std.fmt.allocPrint(gpa, "{x}|{s}", .{ r.s, r.tu }), r.sym);
    var fns: std.StringHashMapUnmanaged(*const ours.OurFunc) = .empty;
    for (try ours.loadAll(&x.d, try util.join(x.cfg.build, "obj"))) |*f| try fns.put(gpa, try f.key(), f);

    const Stat = struct { n: usize = 0, match: usize = 0, bytes: u64 = 0, hunks: std.ArrayList(usize) = .empty, sim: std.ArrayList(f64) = .empty };
    var stats: [BUCKETS.len]Stat = @splat(.{});
    var total_bytes: u64 = 0;
    var viewer = compare.Viewer{ .d = &x.d, .with_text = true };
    var it = util.Lines{ .data = try util.readFile(audit_path) };
    while (it.next()) |line| {
        if (std.mem.startsWith(u8, line, "start\t") or util.strip(line).len == 0) continue;
        const c = try util.split(line, '\t');
        const s = try util.hex(c[0]);
        const e = try util.hex(c[1]);
        const tu = c[3];
        const status = c[9];
        if (runtime.contains(s) or eh.contains(s)) continue;
        const m = x.model.getPtr(s) orelse continue;
        const ni = m.f.insns.items.len;
        var bi: ?usize = null;
        for (BUCKETS, 0..) |b, k| if (b.lo <= ni and ni <= b.hi) {
            bi = k;
            break;
        };
        const st = &stats[bi orelse continue];
        st.n += 1;
        st.bytes += e - s;
        total_bytes += e - s;
        if (std.mem.eql(u8, status, "MATCH")) {
            st.match += 1;
            continue;
        }
        if (std.mem.eql(u8, status, "MISSING") or std.mem.eql(u8, status, "QUARANTINE")) continue;
        try st.sim.append(gpa, try std.fmt.parseFloat(f64, c[8]));
        const sym = sym_of.get(try std.fmt.allocPrint(gpa, "{x}|{s}", .{ s, tu })) orelse "";
        const f = fns.get(try std.fmt.allocPrint(gpa, "{s}|{s}", .{ tu, sym })) orelse continue;
        try st.hunks.append(gpa, try hunks(&viewer, try viewer.ourView(f, &syms), try viewer.refViewSpan(s, e, &x.model)));
    }

    var o: std.ArrayList(u8) = .empty;
    if (label.len > 0) try o.print(gpa, "# sizetrend {s}\n", .{label}) else try o.appendSlice(gpa, "# sizetrend\n");
    try o.appendSlice(gpa, "bucket\tfunctions\tmatch_pct\tbytes_pct\thunks_median\thunks_mean\tsim_mean\n");
    for (BUCKETS, &stats) |b, *st| {
        try o.print(gpa, "{s}\t{d}\t", .{ b.name, st.n });
        try util.fixed(&o, "%.2f", 100.0 * @as(f64, @floatFromInt(st.match)) / @as(f64, @floatFromInt(@max(1, st.n))));
        try o.append(gpa, '\t');
        try util.fixed(&o, "%.2f", 100.0 * @as(f64, @floatFromInt(st.bytes)) / @as(f64, @floatFromInt(@max(1, total_bytes))));
        try o.append(gpa, '\t');
        const h = st.hunks.items;
        if (h.len > 0) {
            std.mem.sort(usize, h, {}, std.sort.asc(usize));
            const mid = h.len / 2;
            const med: f64 = if (h.len % 2 == 1) @floatFromInt(h[mid]) else @as(f64, @floatFromInt(h[mid - 1] + h[mid])) / 2.0;
            try util.fixed(&o, "%g", med);
            try o.append(gpa, '\t');
            var sum: u64 = 0;
            for (h) |v| sum += v;
            try util.fixed(&o, "%.2f", @as(f64, @floatFromInt(sum)) / @as(f64, @floatFromInt(h.len)));
        } else try o.appendSlice(gpa, "-\t-");
        try o.append(gpa, '\t');
        if (st.sim.items.len > 0) try util.fixed(&o, "%.4f", exactMean(st.sim.items)) else try o.append(gpa, '-');
        try o.append(gpa, '\n');
    }
    if (p.get("--out")) |path| try util.writeFile(path, o.items);
    util.say(o.items);
}

/// statistics.mean of floats: the exact sum (every value here lies in [0, 1] at a 2**-62 grid
/// at the finest, and there are far fewer than 2**40 of them, so f128 adds them exactly),
/// divided, rounded once to f64.
fn exactMean(xs: []const f64) f64 {
    var sum: f128 = 0;
    for (xs) |v| sum += v;
    return @floatCast(sum / @as(f128, @floatFromInt(xs.len)));
}

// ---- abcompare

const ABCOMPARE_USAGE =
    \\usage: oracle abcompare [-h] [--depth DEPTH] [--list] baseline experiment
    \\
;

const AbRow = struct { name: []const u8, tu: []const u8, sim: f64, status: []const u8 };

fn abLoad(path: []const u8) !std.StringHashMapUnmanaged(AbRow) {
    var rows: std.StringHashMapUnmanaged(AbRow) = .empty;
    var it = util.Lines{ .data = try util.readFile(path) };
    while (it.next()) |line| {
        const p = try util.split(line, '\t');
        if (std.mem.eql(u8, p[0], "start")) continue;
        if (p.len < 10) return error.IndexError;
        try rows.put(gpa, p[0], .{ .name = p[2], .tu = p[3], .sim = try std.fmt.parseFloat(f64, p[8]), .status = p[9] });
    }
    return rows;
}

pub fn abcompareMain(args: []const []const u8) !void {
    const p = cli.parse(args, &.{ .{ .long = "--depth" }, .{ .long = "--list", .value = false } }, ABCOMPARE_USAGE);
    if (p.pos.len != 2) {
        cli.sayErr(ABCOMPARE_USAGE);
        std.process.exit(2);
    }
    const depth: usize = @intCast(p.int("--depth", 1));
    var a = try abLoad(p.pos[0]);
    var b = try abLoad(p.pos[1]);
    var keys: std.ArrayList([]const u8) = .empty;
    var ki = a.keyIterator();
    while (ki.next()) |k| if (b.contains(k.*)) try keys.append(gpa, k.*);
    std.mem.sort([]const u8, keys.items, {}, lessStr);
    const Mod = struct { n: usize = 0, a: usize = 0, b: usize = 0, only_a: usize = 0, only_b: usize = 0, sa: f64 = 0, sb: f64 = 0, ns: usize = 0 };
    var mods: std.StringHashMapUnmanaged(Mod) = .empty;
    const Changed = struct { mod: []const u8, k: []const u8, name: []const u8, xa: []const u8, xb: []const u8 };
    var changed: std.ArrayList(Changed) = .empty;
    for (keys.items) |k| {
        const ra = a.get(k).?;
        const rb = b.get(k).?;
        const skip = [_][]const u8{ "QUARANTINE", "MISSING" };
        var skipped = false;
        for (skip) |s| if (std.mem.eql(u8, ra.status, s) or std.mem.eql(u8, rb.status, s)) {
            skipped = true;
        };
        if (skipped) continue;
        var parts = try util.split(ra.tu, '/');
        if ((std.mem.eql(u8, parts[0], "Source") or std.mem.eql(u8, parts[0], "3rdparty")) and parts.len > 1) parts = parts[1..];
        var mod: std.ArrayList(u8) = .empty;
        for (parts[0..@min(depth, parts.len)], 0..) |s, i| {
            if (i > 0) try mod.append(gpa, '/');
            try mod.appendSlice(gpa, s);
        }
        const g = try mods.getOrPut(gpa, mod.items);
        if (!g.found_existing) g.value_ptr.* = .{};
        const c = g.value_ptr;
        const ma = std.mem.eql(u8, ra.status, "MATCH");
        const mb = std.mem.eql(u8, rb.status, "MATCH");
        c.n += 1;
        c.a += @intFromBool(ma);
        c.b += @intFromBool(mb);
        c.only_a += @intFromBool(ma and !mb);
        c.only_b += @intFromBool(mb and !ma);
        c.sa += ra.sim;
        c.sb += rb.sim;
        c.ns += 1;
        if (ma != mb) try changed.append(gpa, .{ .mod = g.key_ptr.*, .k = k, .name = ra.name, .xa = ra.status, .xb = rb.status });
    }
    var out = cli.Out{};
    try out.print("{s:<28} {s:>6} {s:>8} {s:>8} {s:>7} {s:>7} {s:>8} {s:>8}\n", .{ "module", "n", "MATCH a", "MATCH b", "only a", "only b", "sim a", "sim b" });
    var names: std.ArrayList([]const u8) = .empty;
    var mi = mods.keyIterator();
    while (mi.next()) |k| try names.append(gpa, k.*);
    std.mem.sort([]const u8, names.items, {}, lessStr);
    var tot = Mod{};
    for (names.items) |name| {
        const c = mods.get(name).?;
        tot.n += c.n;
        tot.a += c.a;
        tot.b += c.b;
        tot.only_a += c.only_a;
        tot.only_b += c.only_b;
        try padRight(&out, name, 28);
        try out.print(" {d:>6} {d:>8} {d:>8} {d:>7} {d:>7} ", .{ c.n, c.a, c.b, c.only_a, c.only_b });
        try util.fixed(&out.buf, "%8.3f", c.sa / @as(f64, @floatFromInt(c.ns)));
        try out.raw(" ");
        try util.fixed(&out.buf, "%8.3f", c.sb / @as(f64, @floatFromInt(c.ns)));
        try out.raw("\n");
    }
    try out.print("{s:<28} {d:>6} {d:>8} {d:>8} {d:>7} {d:>7}\n", .{ "total", tot.n, tot.a, tot.b, tot.only_a, tot.only_b });
    if (p.flag("--list")) for (changed.items) |ch| {
        try out.print("  {s} {s} ", .{ ch.mod, ch.k });
        try padRight(&out, cpPrefix(ch.name, 60), 60);
        try out.print(" {s} -> {s}\n", .{ ch.xa, ch.xb });
    };
    try out.flush();
}

// ---- tulayout

const TULAYOUT_USAGE =
    \\usage: oracle tulayout [-h] [--tu TU] [--intruders]
    \\
;

const Entry = struct { key: ours.LayoutKey, s: u32, name: []const u8 };
const Owner = struct { tu: []const u8, name: []const u8 };

pub fn tulayoutMain(args: []const []const u8) !void {
    const p = cli.parse(args, &.{ .{ .long = "--root" }, .{ .long = "--tu" }, .{ .long = "--intruders", .value = false } }, TULAYOUT_USAGE);
    if (p.pos.len > 0) {
        cli.sayErr(TULAYOUT_USAGE);
        std.process.exit(2);
    }
    const x = try context.Context.open(try cli.root(p.get("--root")));
    const rows = try audit.loadFuncmap(try x.cfg.evidence("funcmap.tsv"));
    var fns: std.StringHashMapUnmanaged(*const ours.OurFunc) = .empty;
    for (try ours.loadAll(&x.d, try util.join(x.cfg.build, "obj"))) |*f| try fns.put(gpa, try f.key(), f);
    var by_tu: std.StringArrayHashMapUnmanaged(std.ArrayList(Entry)) = .empty;
    var owner: std.AutoHashMapUnmanaged(u32, Owner) = .empty;
    for (rows) |r| {
        const f = fns.get(try std.fmt.allocPrint(gpa, "{s}|{s}", .{ r.tu, r.sym })) orelse continue;
        const g = try by_tu.getOrPut(gpa, r.tu);
        if (!g.found_existing) g.value_ptr.* = .empty;
        try g.value_ptr.append(gpa, .{ .key = try ours.layoutKey(f), .s = r.s, .name = r.name });
        try owner.put(gpa, r.s, .{ .tu = r.tu, .name = r.name });
    }
    var starts: std.ArrayList(u32) = .empty;
    var oi = owner.keyIterator();
    while (oi.next()) |s| try starts.append(gpa, s.*);
    std.mem.sort(u32, starts.items, {}, std.sort.asc(u32));
    const Report = struct { n: usize, tu: []const u8, cnt: usize, dis: usize, lo: u32, hi: u32, foreign: []const u32 };
    var report: std.ArrayList(Report) = .empty;
    for (by_tu.keys(), by_tu.values()) |tu, *lst| {
        if (p.get("--tu")) |pat| if (std.mem.indexOf(u8, tu, pat) == null) continue;
        std.mem.sort(Entry, lst.items, {}, struct {
            fn lt(_: void, a: Entry, b: Entry) bool {
                if (a.key.lessThan(b.key)) return true;
                if (b.key.lessThan(a.key)) return false;
                if (a.s != b.s) return a.s < b.s;
                return std.mem.order(u8, a.name, b.name) == .lt;
            }
        }.lt);
        const chain = try funcmap.longestIncreasing(Entry, lst.items);
        const lo = chain[0].s;
        const hi = chain[chain.len - 1].s;
        const a = std.sort.lowerBound(u32, starts.items, lo, struct {
            fn cmp(v: u32, e: u32) std.math.Order {
                return std.math.order(v, e);
            }
        }.cmp);
        const b = std.sort.upperBound(u32, starts.items, hi, struct {
            fn cmp(v: u32, e: u32) std.math.Order {
                return std.math.order(v, e);
            }
        }.cmp);
        var foreign: std.ArrayList(u32) = .empty;
        for (starts.items[a..b]) |s| if (!std.mem.eql(u8, owner.get(s).?.tu, tu)) try foreign.append(gpa, s);
        try report.append(gpa, .{ .n = foreign.items.len, .tu = tu, .cnt = lst.items.len, .dis = lst.items.len - chain.len, .lo = lo, .hi = hi, .foreign = foreign.items });
    }
    std.mem.sort(Report, report.items, {}, struct {
        fn lt(_: void, a: Report, b: Report) bool {
            if (a.n != b.n) return a.n > b.n;
            return std.mem.order(u8, a.tu, b.tu) == .lt;
        }
    }.lt);
    var out = cli.Out{};
    try out.print("{s:<44} {s:>5} {s:>8} {s:>9}  {s:<17}\n", .{ "tu", "fns", "disorder", "intruders", "original range" });
    var clean: usize = 0;
    for (report.items) |r| {
        if (r.n == 0) clean += 1;
        try padRight(&out, cpPrefix(r.tu, 44), 44);
        try out.print(" {d:>5} {d:>8} {d:>9}  {x:0>8}-{x:0>8}\n", .{ r.cnt, r.dis, r.n, r.lo, r.hi });
        if (p.flag("--intruders")) for (r.foreign[0..@min(40, r.foreign.len)]) |s| {
            const o = owner.get(s).?;
            try out.print("      {x:0>8}  {s}  ({s})\n", .{ s, o.name, o.tu });
        };
    }
    try out.print("{d} TUs, {d} with no intruder\n", .{ report.items.len, clean });
    try out.flush();
}
