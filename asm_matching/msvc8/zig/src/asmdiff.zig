//! asmdiff (asmdiff.py): one function, ours beside the original.
//!
//!   oracle-zig diff [--root R] [--funcmap FILE] <name | original address>
//!
//! Resolves through evidence/funcmap.tsv, then evidence/libmap.tsv (the statically linked
//! runtime; a name may be given unqualified when it is unique). Both columns are the
//! normalised view the audit judges by; markers as compare.sideBySide. Then the source lines
//! the differences come from, as the tree has them. Reads the objects on disk.
const std = @import("std");
const audit = @import("audit.zig");
const cli = @import("cli.zig");
const compare = @import("compare.zig");
const context = @import("context.zig");
const libmap = @import("libmap.zig");
const ours = @import("ours.zig");
const proc = @import("proc.zig");
const util = @import("util.zig");

const gpa = util.gpa;

const USAGE =
    \\asmdiff - one function, ours beside the original.
    \\
    \\    oracle diff [--funcmap FILE] <name | original address>
    \\        e.g.  oracle diff DRLGENV_UpdateLightIntensity
    \\              oracle diff 0045a2b0
    \\
    \\Markers between the columns: blank = identical; ! = identical bytes, but a
    \\reference names something else; ~ = same instruction, different
    \\bytes; | = different instruction; < only ours; > only the original's.
    \\Rebuild the object first (oracle build <tu>) - this reads what is on disk.
    \\
;

/// want.lower().lstrip('0x').lstrip('0') == '%x' % s, name == want, or name ends in ::want.
fn wanted(want: []const u8, r: *const audit.MapRow) !bool {
    if (std.mem.eql(u8, r.name, want)) return true;
    if (std.mem.endsWith(u8, r.name, want) and r.name.len >= want.len + 2 and std.mem.eql(u8, r.name[r.name.len - want.len - 2 .. r.name.len - want.len], "::")) return true;
    const lw = try proc.lower(want);
    var i: usize = 0;
    while (i < lw.len and (lw[i] == '0' or lw[i] == 'x')) i += 1;
    var buf: [16]u8 = undefined;
    return std.mem.eql(u8, lw[i..], try std.fmt.bufPrint(&buf, "{x}", .{r.s}));
}

pub fn main(args: []const []const u8) !void {
    var root_arg: ?[]const u8 = null;
    var rest: std.ArrayList([]const u8) = .empty;
    var k: usize = 0;
    while (k < args.len) : (k += 1) {
        if (std.mem.eql(u8, args[k], "--root") and k + 1 < args.len) {
            root_arg = args[k + 1];
            k += 1;
        } else try rest.append(gpa, args[k]);
    }
    const root = try cli.root(root_arg);
    var argv = rest.items;
    var funcmap_path: []const u8 = try util.join(root, "evidence/funcmap.tsv");
    if (argv.len == 3 and std.mem.eql(u8, argv[0], "--funcmap")) {
        funcmap_path = argv[1];
        argv = argv[2..];
    }
    if (argv.len != 1) {
        cli.sayErr(USAGE);
        std.process.exit(1);
    }
    const want = argv[0];
    const x = try context.Context.open(root);
    const rows = try audit.loadFuncmap(funcmap_path);
    const librows = try x.libRows();
    var hits: std.ArrayList(struct { r: *const audit.MapRow, lib: bool }) = .empty;
    for (rows) |*r| if (try wanted(want, r)) try hits.append(gpa, .{ .r = r, .lib = false });
    for (librows) |*r| if (try wanted(want, r)) try hits.append(gpa, .{ .r = r, .lib = true });
    if (hits.items.len == 0) cli.exitMsg("{s}: not in the funcmap (unmapped functions are listed in evidence/funcmap.tsv.unmapped)", .{want});
    if (hits.items.len > 1) {
        var m: std.ArrayList(u8) = .empty;
        try m.print(gpa, "{s} is ambiguous:", .{want});
        for (hits.items) |h| try m.print(gpa, "\n  {x:0>8} {s} ({s})", .{ h.r.s, h.r.name, h.r.tu });
        cli.exitMsg("{s}", .{m.items});
    }
    const r = hits.items[0].r;
    // A row that is in both lists is the library's (Python: `hits[0] in librows`).
    var lib = hits.items[0].lib;
    if (!lib) for (librows) |*l| if (l.s == r.s and l.e == r.e and std.mem.eql(u8, l.name, r.name) and std.mem.eql(u8, l.tu, r.tu) and
        std.mem.eql(u8, l.sym, r.sym) and std.mem.eql(u8, l.method, r.method) and std.mem.eql(u8, l.detail, r.detail))
    {
        lib = true;
    };
    const fns: []ours.OurFunc = if (lib) try libmap.loadLibFunctions(&x.d) else try x.loadObject(r.tu);
    const want_key = try std.fmt.allocPrint(gpa, "{s}|{s}", .{ r.tu, r.sym });
    var f: ?*const ours.OurFunc = null;
    for (fns) |*g| if (std.mem.eql(u8, try g.key(), want_key)) {
        f = g; // the last of a key wins, as the dict comprehension's
    };
    if (f == null) cli.exitMsg("{s} not found in {s}.obj - rebuild it", .{ r.sym, r.tu });
    var syms = try x.symMap(rows, librows);
    var viewer = compare.Viewer{ .d = &x.d, .with_text = true };
    const ov = try viewer.ourView(f.?, &syms);
    const rv = viewer.refViewSpan(r.s, r.e, &x.model) catch |e| {
        if (e == error.NotARun) cli.exitMsg("ValueError: {x:0>8}..{x:0>8} is not a run of carved functions", .{ r.s, r.e });
        return e;
    };
    const res = try compare.compare(ov, rv);
    var out = cli.Out{};
    try out.print("{s}  [{s}]  original {x:0>8}..{x:0>8}  paired by {s} ({s})\n", .{ r.name, r.tu, r.s, r.e, r.method, r.detail });
    try out.print("status {s}  insns ours {d} / original {d}  byte diffs {d}  similarity ", .{ res.status, res.our_insns, res.ref_insns, res.byte_diffs });
    try util.fixed(&out.buf, "%.3f", res.similarity);
    try out.print("  refs differ {d}, unverified {d}\n", .{ res.ref_diffs, res.unverified });
    try out.print("{s:<66}   {s}\n", .{ "ours (offset, source line, instruction)", "original" });
    for (try compare.sideBySide(&viewer, ov, rv, 66)) |l| try out.latLine(l);
    // The source lines the differences come from, as the tree has them (the translation
    // keeps line numbers, so these are the real file's lines).
    const src = try std.fmt.allocPrint(gpa, "{s}/{s}.cpp", .{ x.cfg.repo, r.tu });
    const lines = try compare.differingLines(ov, rv);
    if (lines.len > 0 and util.exists(src)) {
        const text = try sourceLines(try util.readFile(src));
        try out.print("\nsource lines with differing code ({s}.cpp):\n", .{r.tu});
        std.mem.sort(u32, lines, {}, std.sort.asc(u32));
        for (lines) |ln| if (0 < ln and ln <= text.len) {
            try out.print("{d:>6}  ", .{ln});
            try out.raw(try clip(text[ln - 1], 150));
            try out.raw("\n");
        };
    }
    try out.flush();
}

/// open(src, encoding='utf-8', errors='replace').read().split('\n'): universal newlines, then
/// lines; each still UTF-8 (an invalid byte as U+FFFD).
pub fn sourceLines(data: []const u8) ![][]const u8 {
    const text = try proc.universalNewlines(try utf8Replace(data));
    return util.split(text, '\n');
}

/// Bytes that are not UTF-8 replaced by U+FFFD, one per byte that starts no valid sequence.
pub fn utf8Replace(s: []const u8) ![]const u8 {
    if (std.unicode.utf8ValidateSlice(s)) return s;
    var out: std.ArrayList(u8) = .empty;
    var i: usize = 0;
    while (i < s.len) {
        const n = std.unicode.utf8ByteSequenceLength(s[i]) catch 0;
        if (n > 0 and i + n <= s.len) {
            if (std.unicode.utf8Decode(s[i .. i + n])) |_| {
                try out.appendSlice(gpa, s[i .. i + n]);
                i += n;
                continue;
            } else |_| {}
        }
        try out.appendSlice(gpa, "\u{FFFD}");
        i += 1;
    }
    return out.items;
}

fn isPySpace(cp: u21) bool {
    return switch (cp) {
        ' ', '\t', '\n', '\r', 0x0b, 0x0c, 0x1c...0x1f, 0x85, 0xa0, 0x1680, 0x2000...0x200a, 0x2028, 0x2029, 0x202f, 0x205f, 0x3000 => true,
        else => false,
    };
}

/// line.rstrip()[:n], counted in characters.
pub fn clip(line: []const u8, n: usize) ![]const u8 {
    var cps: std.ArrayList(struct { at: usize, len: usize, cp: u21 }) = .empty;
    var it = (try std.unicode.Utf8View.init(line)).iterator();
    var at: usize = 0;
    while (it.nextCodepointSlice()) |sl| {
        try cps.append(gpa, .{ .at = at, .len = sl.len, .cp = try std.unicode.utf8Decode(sl) });
        at += sl.len;
    }
    var end = cps.items.len;
    while (end > 0 and isPySpace(cps.items[end - 1].cp)) end -= 1;
    const keep = @min(end, n);
    if (keep == 0) return "";
    const last = cps.items[keep - 1];
    return line[0 .. last.at + last.len];
}
