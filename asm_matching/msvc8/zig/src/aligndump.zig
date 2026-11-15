//! aligndump: every function that does not match, ours aligned with the original, as JSON lines.
//!
//!   oracle-zig aligndump [--root R] [--audit A] [--funcmap F] [--out FILE] [ADDR ...]
//!
//! The alignment is the audit's own (compare.compare: difflib over the neutral keys), so a
//! consumer sees exactly the stretches the audit judged. One line per function:
//!
//!   {"a":"0045a2b0","e":"0045a2f0","name":..,"tu":..,"status":..,"sim":0.9,
//!    "o":[[off,line,"text","key"|null,"hdr"|null,"masked hex",[tok|null,..]],..],
//!                                    ours; line 0 = none, hdr = the header file of an instruction
//!                                    inlined from one; tok = a comparable reference's label,
//!                                    null for one the audit cannot compare (data, unmapped)
//!    "r":[[off,"text","key"|null,"masked hex",[tok|null,..]],..],   the original's
//!    "ops":[["e",i,i2,j,j2,"marks"],["r",..],["d",..],["i",..]]}
//!
//! marks (equal ops only), one per pair: ' ' identical, '!' same bytes but a comparable
//! reference names something else, '~' same instruction, other bytes. key is null when it
//! equals text. Then one line per translation unit with the definition line of each of its
//! functions (ours.sourceLine), so a consumer can tell which function a source line belongs
//! to - code VC8 inlined from a static callee carries the callee's lines:
//!
//!   {"tu":..,"fns":[[line|0,"symbol","original address"|null,static],..]}
//!
//! Default: every funcmap row whose audit status is not MATCH, QUARANTINE or MISSING. The
//! sole-blocker index (oracle-zig soleblock) is built from this.
const std = @import("std");
const audit = @import("audit.zig");
const cli = @import("cli.zig");
const compare = @import("compare.zig");
const context = @import("context.zig");
const ours = @import("ours.zig");
const util = @import("util.zig");

const gpa = util.gpa;

const USAGE =
    \\usage: oracle aligndump [--root R] [--audit A] [--funcmap F] [--out FILE] [ADDR ...]
    \\
    \\Every function that does not match, ours aligned with the original, as JSON lines.
    \\
;

/// A JSON string of Latin-1 bytes (the views hold Latin-1): every byte >= 0x80 as \u00XX.
fn jstr(out: *std.ArrayList(u8), s: []const u8) !void {
    try out.append(gpa, '"');
    for (s) |ch| switch (ch) {
        '"' => try out.appendSlice(gpa, "\\\""),
        '\\' => try out.appendSlice(gpa, "\\\\"),
        0...0x1f, 0x7f...0xff => try out.print(gpa, "\\u{x:0>4}", .{ch}),
        else => try out.append(gpa, ch),
    };
    try out.append(gpa, '"');
}

fn comparable(t: *const compare.Token) bool {
    return switch (t.kind) {
        .data_sym, .data_addr, .fnq, .straddle => false,
        else => true,
    };
}

/// The audit's marker for an aligned pair of equal keys.
fn mark(x: *const compare.NInsn, y: *const compare.NInsn) u8 {
    if (!std.mem.eql(u8, x.masked, y.masked)) return '~';
    if (x.tokens.len != y.tokens.len) return '!';
    for (x.tokens, y.tokens) |*p, *q| if (comparable(p) and comparable(q) and !p.eql(q)) return '!';
    return ' ';
}

/// ,"masked hex",[label|null,..]: what the audit's identity test reads.
fn bytesAndTokens(out: *std.ArrayList(u8), i: *const compare.NInsn) !void {
    try out.appendSlice(gpa, ",\"");
    for (i.masked) |b| try out.print(gpa, "{x:0>2}", .{b});
    try out.appendSlice(gpa, "\",[");
    for (i.tokens, 0..) |*t, k| {
        if (k > 0) try out.append(gpa, ',');
        if (comparable(t)) {
            var lab: std.ArrayList(u8) = .empty;
            try compare.label(&lab, t);
            try jstr(out, lab.items);
        } else try out.appendSlice(gpa, "null");
    }
    try out.append(gpa, ']');
}

fn ownFile(file: []const u8, tu: []const u8) !bool {
    const own = try std.fmt.allocPrint(gpa, "{s}.cpp", .{tu});
    for (own) |*ch| ch.* = std.ascii.toLower(ch.*);
    const f = try gpa.dupe(u8, file);
    for (f) |*ch| ch.* = if (ch.* == '\\') '/' else std.ascii.toLower(ch.*);
    return std.mem.endsWith(u8, f, own);
}

pub fn main(args: []const []const u8) !void {
    const p = cli.parse(args, &.{ .{ .long = "--root" }, .{ .long = "--audit" }, .{ .long = "--funcmap" }, .{ .long = "--out" } }, USAGE);
    const x = try context.Context.open(try cli.root(p.get("--root")));
    const rows = try audit.loadFuncmap(p.get("--funcmap") orelse try x.cfg.evidence("funcmap.tsv"));
    var syms = try x.symMap(rows, try x.libRows());
    var status: std.AutoHashMapUnmanaged(u32, []const u8) = .empty;
    var sims: std.AutoHashMapUnmanaged(u32, []const u8) = .empty;
    {
        var it = util.Lines{ .data = try util.readFile(p.get("--audit") orelse try x.cfg.evidence("audit.tsv")) };
        while (it.next()) |line| {
            const f = try util.split(line, '\t');
            if (std.mem.eql(u8, f[0], "start") or f.len <= 9) continue;
            try status.put(gpa, try util.hex(f[0]), f[9]);
            try sims.put(gpa, try util.hex(f[0]), f[8]);
        }
    }
    var want: std.AutoHashMapUnmanaged(u32, void) = .empty;
    for (p.pos) |a| try want.put(gpa, try util.hex(a), {});
    var orig_of: std.StringHashMapUnmanaged(u32) = .empty; // tu|sym -> original start
    for (rows) |r| try orig_of.put(gpa, try std.fmt.allocPrint(gpa, "{s}|{s}", .{ r.tu, r.sym }), r.s);

    var objs: std.StringHashMapUnmanaged(std.StringHashMapUnmanaged(*const ours.OurFunc)) = .empty;
    var tus: std.ArrayList([]const u8) = .empty;
    var viewer = compare.Viewer{ .d = &x.d, .with_text = true };
    var out: std.ArrayList(u8) = .empty;
    var n: usize = 0;
    for (rows) |r| {
        const st = status.get(r.s) orelse continue;
        if (want.count() > 0) {
            if (!want.contains(r.s)) continue;
        } else if (std.mem.eql(u8, st, "MATCH") or std.mem.eql(u8, st, "QUARANTINE") or std.mem.eql(u8, st, "MISSING")) continue;
        const g = try objs.getOrPut(gpa, r.tu);
        if (!g.found_existing) {
            g.value_ptr.* = .empty;
            if (util.exists(try std.fmt.allocPrint(gpa, "{s}/obj/{s}.obj", .{ x.cfg.build, r.tu }))) {
                for (try x.loadObject(r.tu)) |*f| try g.value_ptr.put(gpa, f.f.symbol, f);
                try tus.append(gpa, r.tu);
            }
        }
        const f = g.value_ptr.get(r.sym) orelse continue;
        const ov = try viewer.ourView(f, &syms);
        const rv = viewer.refViewSpan(r.s, r.e, &x.model) catch continue;
        const sm_ids = struct {
            fn of(xs: []const compare.NInsn) ![]u32 {
                const o = try gpa.alloc(u32, xs.len);
                for (xs, 0..) |i, k| o[k] = i.key;
                return o;
            }
        };
        var sm = try @import("difflib.zig").SequenceMatcher.init(gpa, try sm_ids.of(ov), try sm_ids.of(rv));
        defer sm.deinit();
        try out.print(gpa, "{{\"a\":\"{x:0>8}\",\"e\":\"{x:0>8}\",\"name\":", .{ r.s, r.e });
        try jstr(&out, r.name);
        try out.appendSlice(gpa, ",\"tu\":");
        try jstr(&out, r.tu);
        try out.appendSlice(gpa, ",\"status\":");
        try jstr(&out, st);
        try out.print(gpa, ",\"sim\":{s},\"o\":[", .{sims.get(r.s) orelse "0"});
        for (ov, 0..) |*i, k| {
            if (k > 0) try out.append(gpa, ',');
            var line: u32 = 0;
            var hdr: ?[]const u8 = null;
            if (f.lineAt(i.addr)) |l| {
                if (try ownFile(l.file, r.tu)) line = l.line else hdr = try std.fmt.allocPrint(gpa, "{s}:{d}", .{ util.basename(l.file), l.line });
            }
            try out.print(gpa, "[{d},{d},", .{ i.addr, line });
            try jstr(&out, viewer.intern.str(i.text));
            try out.append(gpa, ',');
            if (i.key == i.text) try out.appendSlice(gpa, "null") else try jstr(&out, viewer.intern.str(i.key));
            try out.append(gpa, ',');
            if (hdr) |h| try jstr(&out, h) else try out.appendSlice(gpa, "null");
            try bytesAndTokens(&out, i);
            try out.append(gpa, ']');
        }
        try out.appendSlice(gpa, "],\"r\":[");
        for (rv, 0..) |*i, k| {
            if (k > 0) try out.append(gpa, ',');
            try out.print(gpa, "[{d},", .{i.addr});
            try jstr(&out, viewer.intern.str(i.text));
            try out.append(gpa, ',');
            if (i.key == i.text) try out.appendSlice(gpa, "null") else try jstr(&out, viewer.intern.str(i.key));
            try bytesAndTokens(&out, i);
            try out.append(gpa, ']');
        }
        try out.appendSlice(gpa, "],\"ops\":[");
        for (try sm.opcodes(), 0..) |op, k| {
            if (k > 0) try out.append(gpa, ',');
            const t: []const u8 = switch (op.tag) {
                .equal => "e",
                .replace => "r",
                .delete => "d",
                .insert => "i",
            };
            try out.print(gpa, "[\"{s}\",{d},{d},{d},{d}", .{ t, op.ci, op.ci2, op.cj, op.cj2 });
            if (op.tag == .equal) {
                try out.appendSlice(gpa, ",\"");
                for (ov[op.ci..op.ci2], rv[op.cj..op.cj2]) |*a, *b| try out.append(gpa, mark(a, b));
                try out.append(gpa, '"');
            }
            try out.append(gpa, ']');
        }
        try out.appendSlice(gpa, "]}\n");
        n += 1;
    }
    for (tus.items) |tu| {
        try out.appendSlice(gpa, "{\"tu\":");
        try jstr(&out, tu);
        try out.appendSlice(gpa, ",\"fns\":[");
        var first = true;
        var it = objs.get(tu).?.valueIterator();
        while (it.next()) |fp| {
            const f = fp.*;
            const line = (try ours.sourceLine(f)) orelse 0;
            if (!first) try out.append(gpa, ',');
            first = false;
            try out.print(gpa, "[{d},", .{line});
            try jstr(&out, f.f.symbol);
            if (orig_of.get(try f.key())) |a| try out.print(gpa, ",\"{x:0>8}\"", .{a}) else try out.appendSlice(gpa, ",null");
            try out.print(gpa, ",{s}]", .{if (f.f.static) "true" else "false"});
        }
        try out.appendSlice(gpa, "]}\n");
    }
    if (p.get("--out")) |o| {
        try util.writeFile(o, out.items);
        std.debug.print("aligndump: {d} functions, {d} units -> {s}\n", .{ n, tus.items.len, o });
    } else util.say(out.items);
}
