//! check (check.py): re-judge some TUs' functions against the committed audit.
//!
//!   oracle-zig check [--root R] [--update] PATTERN ...   (PATTERN: a path substring)
//!   oracle-zig check [--root R]                          (the carve check alone)
//!
//! The inner loop and the regression gate. For every mapped function whose TU path contains
//! a PATTERN, compares the object on disk with the original and with its row in
//! evidence/audit.tsv, and prints new MATCHes, regressions and similarity moves. Exit status
//! 1 if anything regressed. --update writes the new rows into evidence/audit.tsv. Every run
//! first checks the carve against CARVE, extents the carving has got wrong before; exit
//! status 1 if one differs.
const std = @import("std");
const audit = @import("audit.zig");
const cli = @import("cli.zig");
const compare = @import("compare.zig");
const context = @import("context.zig");
const ours = @import("ours.zig");
const util = @import("util.zig");

const gpa = util.gpa;

const USAGE =
    \\usage: oracle check [-h] [--update] [patterns ...]
    \\
;

const Extent = struct { s: u32, e: u32, roots: []const u32, name: []const u8 };

/// start, end (exclusive), roots, name: what the carve must produce.
const CARVE = [_]Extent{
    // A 16-aligned 0xCC that is an operand byte of the last instruction (call rel32 at
    // 0066a15d) is not a padded start.
    .{ .s = 0x00669130, .e = 0x0066a19f, .roots = &.{}, .name = "CUBE_LoadCubeMainTable" },
    .{ .s = 0x00401f10, .e = 0x004020c2, .roots = &.{}, .name = "GetSymbolNameForAddress" },
    .{ .s = 0x00409dd0, .e = 0x00409fea, .roots = &.{}, .name = "InitializePoolSystem" },
    // A branch past a padded start into its body: the switch table lies beyond.
    .{ .s = 0x00626200, .e = 0x006266bc, .roots = &.{}, .name = "STATLIST_CalcCombinedStatValue" },
    // An SEH prolog starts a function nothing calls (_tsopen_helper); its __finally funclet
    // is its root, not _tsopen_nolock's.
    .{ .s = 0x0069b08a, .e = 0x0069b7e3, .roots = &.{}, .name = "_tsopen_nolock" },
    .{ .s = 0x0069b7e3, .e = 0x0069b85f, .roots = &.{0x0069b85d}, .name = "_tsopen_helper" },
    .{ .s = 0x006a8b91, .e = 0x006a92eb, .roots = &.{}, .name = "_tsopen_nolock (wopen)" },
    .{ .s = 0x006a92eb, .e = 0x006a9367, .roots = &.{0x006a9365}, .name = "_tsopen_helper (wopen)" },
    // Reached only by a tail jump (the unwind thunk at 00604460), right after its
    // predecessor's ret with no padding.
    .{ .s = 0x006043a0, .e = 0x006043b0, .roots = &.{}, .name = "TSGrowableArray<CELLIST> constructor" },
    .{ .s = 0x006043b0, .e = 0x0060443a, .roots = &.{}, .name = "TSGrowableArray<CELLIST>::New" },
    // A callback held in data, right after its predecessor's ret with no padding.
    .{ .s = 0x005ee040, .e = 0x005ee260, .roots = &.{}, .name = "AITHINK_Fn1_CatapultSpotter" },
    .{ .s = 0x005ee260, .e = 0x005ee3b6, .roots = &.{}, .name = "AITHINK_Fn1_DeathMauler" },
    .{ .s = 0x0046a840, .e = 0x0046a940, .roots = &.{}, .name = "WindowsKeyPressedEvent" },
    .{ .s = 0x0046a940, .e = 0x0046aa1b, .roots = &.{}, .name = "INPUT_DispatchKeyCommand" },
    // Funclets the exception tables name stay roots, even on the 16-byte grid.
    .{ .s = 0x0069ac98, .e = 0x0069ad85, .roots = &.{0x0069ad80}, .name = "SEH function, __finally on the grid" },
    .{ .s = 0x006b42f0, .e = 0x006b4c86, .roots = &.{ 0x006b4c42, 0x006b4c73 }, .name = "Direct3DCallBack catch funclets" },
};

fn rootList(out: *std.ArrayList(u8), roots: []const u32) !void {
    if (roots.len == 0) return out.append(gpa, '-');
    for (roots, 0..) |r, i| try out.print(gpa, "{s}{x:0>8}", .{ if (i == 0) "" else ",", r });
}

/// Lines describing every CARVE entry the carve gets wrong.
fn checkCarve(ref: anytype) ![][]const u8 {
    var bad: std.ArrayList([]const u8) = .empty;
    for (CARVE) |x| {
        const got = ref.functions.get(x.s) orelse {
            try bad.append(gpa, try std.fmt.allocPrint(gpa, "carve: {x:0>8} {s} is not a function start", .{ x.s, x.name }));
            continue;
        };
        if (got.end != x.e or !std.mem.eql(u32, got.roots, x.roots)) {
            var l: std.ArrayList(u8) = .empty;
            try l.print(gpa, "carve: {x:0>8} {s} is {x:0>8}..{x:0>8} roots ", .{ x.s, x.name, x.s, got.end });
            try rootList(&l, got.roots);
            try l.print(gpa, ", expected ..{x:0>8} roots ", .{x.e});
            try rootList(&l, x.roots);
            try bad.append(gpa, l.items);
        }
    }
    return bad.items;
}

pub const Judged = struct {
    status: []const u8,
    sim: f64,
    ri: usize,
    oi: usize,
    bd: usize,
    notes: []const u8,
    e: u32,
    name: []const u8,
    tu: []const u8,
    method: []const u8,
    ident: usize,
    operand: usize,
};

const Name = struct { tu: []const u8, name: []const u8 };

/// start -> the judgement of every mapped function of the TUs a pattern selects.
fn judge(x: *context.Context, rows: []const audit.MapRow, pats: []const []const u8, syms: *audit.SymMap, quarantined: *std.StringHashMapUnmanaged([]const u8)) !std.AutoArrayHashMapUnmanaged(u32, Judged) {
    var tus: std.StringArrayHashMapUnmanaged(void) = .empty;
    for (rows) |r| for (pats) |p| if (std.mem.indexOf(u8, r.tu, p) != null) {
        try tus.put(gpa, r.tu, {});
        break;
    };
    const sorted = try gpa.dupe([]const u8, tus.keys());
    std.mem.sort([]const u8, sorted, {}, struct {
        fn lt(_: void, a: []const u8, b: []const u8) bool {
            return std.mem.order(u8, a, b) == .lt;
        }
    }.lt);
    var fns: std.StringHashMapUnmanaged(*const ours.OurFunc) = .empty;
    var by_name: std.StringHashMapUnmanaged(std.ArrayList(*const ours.OurFunc)) = .empty;
    for (sorted) |tu| {
        const base = try std.fmt.allocPrint(gpa, "{s}/obj/{s}", .{ x.cfg.build, tu });
        if (!util.exists(try std.fmt.allocPrint(gpa, "{s}.obj", .{base})) or !util.exists(try std.fmt.allocPrint(gpa, "{s}.d", .{base}))) continue;
        for (try x.loadObject(tu)) |*f| {
            try fns.put(gpa, try f.key(), f);
            const g = try by_name.getOrPut(gpa, try audit.qkey(tu, f.f.name));
            if (!g.found_existing) g.value_ptr.* = .empty;
            try g.value_ptr.append(gpa, f);
        }
    }
    var viewer = compare.Viewer{ .d = &x.d };
    var out: std.AutoArrayHashMapUnmanaged(u32, Judged) = .empty;
    for (rows) |r| {
        if (!tus.contains(r.tu)) continue;
        var f = fns.get(try std.fmt.allocPrint(gpa, "{s}|{s}", .{ r.tu, r.sym }));
        if (f == null) if (by_name.get(try audit.qkey(r.tu, r.name))) |l| if (l.items.len == 1) {
            f = l.items[0]; // its signature changed since the map: same function, other mangling
        };
        const q = quarantined.get(try audit.qkey(r.tu, audit.lastPart(r.name)));
        var j = Judged{ .status = "", .sim = 0.0, .ri = 0, .oi = 0, .bd = 0, .notes = "", .e = r.e, .name = r.name, .tu = r.tu, .method = r.method, .ident = 0, .operand = 0 };
        if (f == null) {
            j.status = "MISSING";
            j.notes = "object not compiled";
        } else if (q) |msg| {
            const m = x.model.getPtr(r.s) orelse return error.KeyError;
            j.status = "QUARANTINE";
            j.ri = m.f.insns.items.len;
            j.notes = msg;
        } else {
            const m = x.model.getPtr(r.s) orelse return error.KeyError;
            const res = try compare.compare(try viewer.ourView(f.?, syms), try viewer.refViewSpan(r.s, m.f.end, &x.model));
            var notes: std.ArrayList(u8) = .empty;
            if (res.ref_diffs > 0) try notes.print(gpa, "{d} refs differ", .{res.ref_diffs});
            if (res.unverified > 0) try notes.print(gpa, "{s}{d} refs unverified", .{ if (notes.items.len > 0) "; " else "", res.unverified });
            j = .{ .status = res.status, .sim = res.similarity, .ri = res.ref_insns, .oi = res.our_insns, .bd = res.byte_diffs, .notes = notes.items, .e = r.e, .name = r.name, .tu = r.tu, .method = r.method, .ident = res.ident_bytes, .operand = res.operand_bytes };
        }
        try out.put(gpa, r.s, j);
    }
    return out;
}

pub fn main(args: []const []const u8) !void {
    const p = cli.parse(args, &.{ .{ .long = "--root" }, .{ .long = "--update", .value = false } }, USAGE);
    const root = try cli.root(p.get("--root"));
    const x = try context.Context.open(root);
    var out = cli.Out{};
    const bad = try checkCarve(x.ref);
    for (bad) |l| try out.line(l);
    try out.print("carve: {d} of {d} checked extents as expected\n", .{ CARVE.len - bad.len, CARVE.len });
    if (p.pos.len == 0) {
        try out.flush();
        std.process.exit(if (bad.len > 0) 1 else 0);
    }
    const audit_path = try x.cfg.evidence("audit.tsv");
    const rows = try audit.loadFuncmap(try x.cfg.evidence("funcmap.tsv"));
    var syms = try x.symMap(rows, try x.libRows());
    var quarantined = try audit.loadQuarantine(try util.join(x.cfg.build, "quarantine.tsv"));
    var old: std.AutoHashMapUnmanaged(u32, struct { st: []const u8, sim: f64 }) = .empty;
    const text = try util.readFile(audit_path);
    {
        var lines = std.mem.splitScalar(u8, text, '\n');
        _ = lines.next();
        while (lines.next()) |line| {
            const f = try util.split(line, '\t');
            if (f.len > 9) try old.put(gpa, try util.hex(f[0]), .{ .st = f[9], .sim = try std.fmt.parseFloat(f64, f[8]) });
        }
    }
    var new = try judge(x, rows, p.pos, &syms, &quarantined);
    const keys = try gpa.dupe(u32, new.keys());
    std.mem.sort(u32, keys, {}, std.sort.asc(u32));
    var gained: usize = 0;
    var lost: usize = 0;
    var up: f64 = 0.0;
    var down: f64 = 0.0;
    var match: usize = 0;
    for (keys) |s| {
        const j = new.get(s).?;
        const o = old.get(s) orelse @TypeOf(old.get(s).?){ .st = "?", .sim = 0.0 };
        const is_match = std.mem.eql(u8, j.status, "MATCH");
        const was_match = std.mem.eql(u8, o.st, "MATCH");
        if (is_match and !was_match) {
            gained += 1;
            try out.print("  + MATCH  {x:0>8} {s}\n", .{ s, j.name });
        } else if (was_match and !is_match) {
            lost += 1;
            try out.print("  - LOST   {x:0>8} {s} ({s})\n", .{ s, j.name, j.status });
        } else if (@abs(j.sim - o.sim) >= 0.001) {
            up += @max(0.0, j.sim - o.sim);
            down += @max(0.0, o.sim - j.sim);
        }
    }
    for (new.values()) |j| if (std.mem.eql(u8, j.status, "MATCH")) {
        match += 1;
    };
    try out.print("{d} functions: {d} MATCH (+{d}, -{d}), similarity +", .{ new.count(), match, gained, lost });
    try util.fixed(&out.buf, "%.2f", up);
    try out.buf.appendSlice(gpa, " / -");
    try util.fixed(&out.buf, "%.2f", down);
    try out.buf.append(gpa, '\n');
    try out.flush();
    if (p.flag("--update")) try updateRows(audit_path, text, &new);
    std.process.exit(if (lost > 0 or bad.len > 0) 1 else 0);
}

/// Write the judged functions' rows into evidence/audit.tsv.
fn updateRows(path: []const u8, text: []const u8, state: *std.AutoArrayHashMapUnmanaged(u32, Judged)) !void {
    var o: std.ArrayList(u8) = .empty;
    var lines = std.mem.splitScalar(u8, text, '\n');
    try o.appendSlice(gpa, lines.next().?);
    while (lines.next()) |line| {
        try o.append(gpa, '\n');
        const f = try util.split(line, '\t');
        if (f.len > 9) if (state.get(try util.hex(f[0]))) |j| {
            const s = try util.hex(f[0]);
            try o.print(gpa, "{x:0>8}\t{x:0>8}\t{s}\t{s}\t{s}\t{d}\t{d}\t{d}\t", .{ s, j.e, j.name, j.tu, j.method, j.ri, j.oi, j.bd });
            try util.fixed(&o, "%.3f", j.sim);
            try o.print(gpa, "\t{s}\t{s}\t{d}\t{d}", .{ j.status, j.notes, j.ident, j.operand });
            continue;
        };
        try o.appendSlice(gpa, line);
    }
    try util.writeFile(path, o.items);
}
