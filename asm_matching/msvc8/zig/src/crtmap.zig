//! The statically linked VC8 runtime in the original, by bytes (crtmap.py): every function of
//! libcmt.lib and libcpmt.lib, RTM and SP1, reduced to the audit's masked view and looked up
//! among the original's functions. Writes evidence/crtmap.tsv and its .summary.
const std = @import("std");
const coff = @import("coff.zig");
const compare = @import("compare.zig");
const dec = @import("decode.zig");
const objfuncs = @import("objfuncs.zig");
const ours = @import("ours.zig");
const refimage = @import("refimage.zig");
const util = @import("util.zig");

const gpa = util.gpa;
const LIBS = [_][]const u8{ "libcmt.lib", "libcpmt.lib" };

/// No function is mapped yet: every call names our symbol.
pub const NoSyms = struct {
    pub fn get(_: NoSyms, _: []const u8) !?u32 {
        return null;
    }
};

/// key_of(view): (len(view), the masked bytes joined), as one byte string.
pub fn keyOf(v: []const compare.NInsn) ![]const u8 {
    var out: std.ArrayList(u8) = .empty;
    try out.print(gpa, "{d}:", .{v.len});
    for (v) |i| try out.appendSlice(gpa, i.masked);
    return out.items;
}

fn lessStr(_: void, a: []const u8, b: []const u8) bool {
    return std.mem.order(u8, a, b) == .lt;
}

/// The wine prefix as libmap.lib_dir finds it: VC8_WINEPREFIX, else VC8_TOOLCHAIN's.
fn prefix() ![]const u8 {
    if (util.getenv("VC8_WINEPREFIX")) |p| if (p.len > 0) return p;
    const tc = util.getenv("VC8_TOOLCHAIN") orelse try util.join(util.getenv("HOME") orelse "", "q2-dev/asm-matching-toolchain");
    return util.join(tc, "wine-vc8");
}

pub fn main(args: []const []const u8) !void {
    var root: ?[]const u8 = null;
    var out_path: ?[]const u8 = null;
    var k: usize = 0;
    while (k < args.len) : (k += 1) {
        if (std.mem.eql(u8, args[k], "--root") and k + 1 < args.len) {
            root = args[k + 1];
            k += 1;
        } else if (std.mem.eql(u8, args[k], "--out") and k + 1 < args.len) {
            out_path = args[k + 1];
            k += 1;
        } else {
            std.debug.print("usage: oracle-zig crt --root <asm_matching/msvc8> [--out F]\n", .{});
            std.process.exit(2);
        }
    }
    const oracle = root orelse {
        std.debug.print("oracle-zig crt: --root is required\n", .{});
        std.process.exit(2);
    };
    const out_file = out_path orelse try util.join(oracle, "evidence/crtmap.tsv");
    const ref = try refimage.Reference.open(try util.join(oracle, "reference/Game.exe"));
    try ref.load(try util.join(oracle, "build/ref/functions.tsv"));
    var model = try ref.model();
    var d = try dec.Decoder.init(gpa);
    var viewer = compare.Viewer{ .d = &d };

    var index: std.StringHashMapUnmanaged(std.ArrayList(u32)) = .empty;
    for (ref.starts) |s| {
        const v = try viewer.refViewSpan(s, model.getPtr(s).?.f.end, &model);
        if (v.len < 3) continue;
        const g = try index.getOrPut(gpa, try keyOf(v));
        if (!g.found_existing) g.value_ptr.* = .empty;
        try g.value_ptr.append(gpa, s);
    }

    const pfx = try prefix();
    const versions = [_][]const u8{ "rtm", "sp1" };
    var hits: [2]std.AutoHashMapUnmanaged(u32, std.StringArrayHashMapUnmanaged(void)) = .{ .empty, .empty };
    var totals: [2]usize = .{ 0, 0 };
    for (versions, 0..) |version, vi| {
        for (LIBS) |lib| {
            const path = try std.fmt.allocPrint(gpa, "{s}/drive_c/vc8/{s}/lib/{s}", .{ pfx, version, lib });
            const data = try util.readFile(path);
            for (try coff.readArchive(data)) |m| {
                if (m.body.len >= 4 and std.mem.eql(u8, m.body[0..4], "\x00\x00\xff\xff")) continue;
                if (m.body.len < 20) continue;
                const fl = objfuncs.functions(&d, m.name, m.body, false) catch continue;
                const short = if (std.mem.lastIndexOfScalar(u8, m.name, '\\')) |i| m.name[i + 1 ..] else m.name;
                for (fl, 0..) |f, order| {
                    totals[vi] += 1;
                    const of = ours.OurFunc{ .tu = m.name, .order = order, .f = f };
                    const v = try viewer.ourView(&of, NoSyms{});
                    if (v.len < 3) continue;
                    const starts = index.get(try keyOf(v)) orelse continue;
                    for (starts.items) |s| {
                        const g = try hits[vi].getOrPut(gpa, s);
                        if (!g.found_existing) g.value_ptr.* = .empty;
                        try g.value_ptr.put(gpa, try std.fmt.allocPrint(gpa, "{s}({s})", .{ f.name, short }), {});
                    }
                }
            }
        }
    }
    var all: std.AutoArrayHashMapUnmanaged(u32, void) = .empty;
    for (hits) |h| {
        var it = h.keyIterator();
        while (it.next()) |s| try all.put(gpa, s.*, {});
    }
    const starts = try gpa.dupe(u32, all.keys());
    std.mem.sort(u32, starts, {}, std.sort.asc(u32));
    var only = [3]usize{ 0, 0, 0 }; // rtm+sp1, rtm, sp1
    var o: std.ArrayList(u8) = .empty;
    try o.appendSlice(gpa, "start\tend\tname\tmatches\n");
    for (starts) |s| {
        const r = hits[0].get(s);
        const p = hits[1].get(s);
        const which: []const u8 = if (r != null and p != null) "rtm+sp1" else if (r != null) "rtm" else "sp1";
        only[if (r != null and p != null) 0 else if (r != null) 1 else 2] += 1;
        var short: std.StringArrayHashMapUnmanaged(void) = .empty;
        for ([_]?std.StringArrayHashMapUnmanaged(void){ r, p }) |set| if (set) |st| for (st.keys()) |n| {
            try short.put(gpa, n[0 .. std.mem.indexOfScalar(u8, n, '(') orelse n.len], {});
        };
        const names = try gpa.dupe([]const u8, short.keys());
        std.mem.sort([]const u8, names, {}, lessStr);
        try o.print(gpa, "{x:0>8}\t{x:0>8}\t", .{ s, model.getPtr(s).?.f.end });
        if (names.len == 1) try o.appendSlice(gpa, names[0]) else {
            try o.append(gpa, '?');
            for (names, 0..) |n, i| {
                if (i > 0) try o.append(gpa, '|');
                try o.appendSlice(gpa, n);
            }
        }
        try o.print(gpa, "\t{s}\n", .{which});
    }
    try util.writeFile(out_file, o.items);
    const summary = try std.fmt.allocPrint(gpa,
        \\libcmt.lib + libcpmt.lib functions: rtm {d}, sp1 {d}
        \\original functions identical to a library function: {d}
        \\  matched by rtm+sp1  {d: >5}
        \\  matched by rtm      {d: >5}
        \\  matched by sp1      {d: >5}
        \\
    , .{ totals[0], totals[1], starts.len, only[0], only[1], only[2] });
    try util.writeFile(try std.fmt.allocPrint(gpa, "{s}.summary", .{out_file}), summary);
    util.say(summary);
}
