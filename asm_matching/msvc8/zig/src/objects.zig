//! Functions the tree files apart from where the original has them (objects.py): a function
//! the original passes register arguments to beyond its declared convention was compiled in
//! one object with all its callers; where the tree puts a caller in another TU, the files part
//! where Blizzard's did not. Writes evidence/objects.tsv.
const std = @import("std");
const c = @import("c");
const audit = @import("audit.zig");
const classify = @import("classify.zig");
const dec = @import("decode.zig");
const ours = @import("ours.zig");
const refimage = @import("refimage.zig");
const util = @import("util.zig");

const gpa = util.gpa;

const R = struct {
    const ecx = classify.ECX;
    const edx = classify.EDX;
};

/// Registers a convention passes arguments in, from the mangled name; null when unknown.
fn conventionArgs(sym: []const u8) ?u8 {
    if (sym.len == 0 or sym[0] != '?') return null;
    const at = std.mem.indexOf(u8, sym, "@@") orelse return null;
    const rest = sym[at + 2 ..];
    const letter: ?u8 = if (rest.len >= 1 and (rest[0] == 'Y' or rest[0] == 'S' or rest[0] == 'K' or rest[0] == 'C'))
        (if (rest.len >= 2) rest[1] else null)
    else if (rest.len >= 3) rest[2] else null;
    return switch (letter orelse return null) {
        'A', 'G' => 0,
        'I' => R.ecx | R.edx,
        'E' => R.ecx,
        else => null,
    };
}

/// '+'.join(sorted(names)) of a register set.
fn regNames(bits: u8) ![]const u8 {
    var names: std.ArrayList([]const u8) = .empty;
    for (classify.GPR_NAMES) |g| if (bits & g[1] != 0) try names.append(gpa, g[0]);
    std.mem.sort([]const u8, names.items, {}, lessStr);
    var out: std.ArrayList(u8) = .empty;
    for (names.items, 0..) |n, i| {
        if (i > 0) try out.append(gpa, '+');
        try out.appendSlice(gpa, n);
    }
    return out.items;
}

/// The @sourcefile annotation above a function's definition, by TU and line.
const SourceFiles = struct {
    repo: []const u8,
    text: std.StringHashMapUnmanaged([][]const u8) = .empty,

    fn sourcefile(line: []const u8) ?[]const u8 {
        // ^//\s*@sourcefile\S*\s+(\S+)
        if (!std.mem.startsWith(u8, line, "//")) return null;
        var k: usize = 2;
        while (k < line.len and std.ascii.isWhitespace(line[k])) k += 1;
        if (!std.mem.startsWith(u8, line[k..], "@sourcefile")) return null;
        k += "@sourcefile".len;
        while (k < line.len and !std.ascii.isWhitespace(line[k])) k += 1;
        const ws = k;
        while (k < line.len and std.ascii.isWhitespace(line[k])) k += 1;
        if (k == ws or k >= line.len) return null;
        const st = k;
        while (k < line.len and !std.ascii.isWhitespace(line[k])) k += 1;
        return line[st..k];
    }

    fn at(sf: *SourceFiles, tu: []const u8, line0: ?u32) ![]const u8 {
        const line = line0 orelse return "";
        const g = try sf.text.getOrPut(gpa, tu);
        if (!g.found_existing) {
            var lines: std.ArrayList([]const u8) = .empty;
            if (try util.readFileOpt(try std.fmt.allocPrint(gpa, "{s}/{s}.cpp", .{ sf.repo, tu }))) |raw| {
                // Universal newlines, as Python's text mode reads it.
                const t = try std.mem.replaceOwned(u8, gpa, raw, "\r\n", "\n");
                std.mem.replaceScalar(u8, t, '\r', '\n');
                var it = std.mem.splitScalar(u8, t, '\n');
                while (it.next()) |l| try lines.append(gpa, l);
            }
            g.value_ptr.* = lines.items;
        }
        const lines = g.value_ptr.*;
        var k: i64 = @as(i64, @min(line, lines.len)) - 1;
        while (k > 0 and !std.mem.startsWith(u8, lines[@intCast(k)], "//") and @as(i64, line) - k < 4) k -= 1;
        while (k >= 0 and std.mem.startsWith(u8, lines[@intCast(k)], "//")) : (k -= 1) {
            if (sourcefile(lines[@intCast(k)])) |s| return s;
        }
        return "";
    }
};

fn lessStr(_: void, a: []const u8, b: []const u8) bool {
    return std.mem.order(u8, a, b) == .lt;
}

pub fn main(args: []const []const u8) !void {
    var root: ?[]const u8 = null;
    var out_path: ?[]const u8 = null;
    var funcmap_path: ?[]const u8 = null;
    var k: usize = 0;
    while (k < args.len) : (k += 1) {
        if (std.mem.eql(u8, args[k], "--root") and k + 1 < args.len) {
            root = args[k + 1];
            k += 1;
        } else if (std.mem.eql(u8, args[k], "--out") and k + 1 < args.len) {
            out_path = args[k + 1];
            k += 1;
        } else if (std.mem.eql(u8, args[k], "--funcmap") and k + 1 < args.len) {
            funcmap_path = args[k + 1];
            k += 1;
        } else {
            std.debug.print("usage: oracle-zig objects --root <asm_matching/msvc8> [--funcmap F] [--out F]\n", .{});
            std.process.exit(2);
        }
    }
    const cfg = try audit.Config.init(root orelse {
        std.debug.print("oracle-zig objects: --root is required\n", .{});
        std.process.exit(2);
    });
    const out_file = out_path orelse try cfg.evidence("objects.tsv");
    const rows = try audit.loadFuncmap(funcmap_path orelse try cfg.evidence("funcmap.tsv"));
    const ref = try refimage.Reference.open(try util.join(cfg.oracle, "reference/Game.exe"));
    try ref.load(try util.join(cfg.refcache, "functions.tsv"));
    var model = try ref.model();
    var d = try dec.Decoder.init(gpa);
    var fns: std.StringHashMapUnmanaged(*const ours.OurFunc) = .empty;
    for (try ours.loadAll(&d, try util.join(cfg.build, "obj"))) |*f| try fns.put(gpa, try f.key(), f);
    var by_start: std.AutoHashMapUnmanaged(u32, audit.MapRow) = .empty;
    for (rows) |r| try by_start.put(gpa, r.s, r);
    var callers: std.AutoHashMapUnmanaged(u32, std.ArrayList(u32)) = .empty;
    for (ref.starts) |s| for (model.getPtr(s).?.refs) |r| if (r.kind == .call) {
        const g = try callers.getOrPut(gpa, r.int);
        if (!g.found_existing) g.value_ptr.* = .empty;
        try g.value_ptr.append(gpa, s);
    };
    var files = SourceFiles{ .repo = cfg.repo };
    var cl = classify.Classifier{ .cap = try classify.Cap.init() };
    const Label = struct {
        fn of(fs: *SourceFiles, bs: *const std.AutoHashMapUnmanaged(u32, audit.MapRow), fm: *const std.StringHashMapUnmanaged(*const ours.OurFunc), start: u32) ![]const u8 {
            const r = bs.get(start) orelse return "";
            const f = fm.get(try std.fmt.allocPrint(gpa, "{s}|{s}", .{ r.tu, r.sym })) orelse return "";
            return fs.at(r.tu, try ours.sourceLine(f));
        }
    };
    var o: std.ArrayList(u8) = .empty;
    try o.appendSlice(gpa, "start\tname\ttu\tkind\tlive_in\tforeign_callers\tsourcefile\tcaller_sourcefiles\n");
    var nconv: usize = 0;
    var nobj: usize = 0;
    for (rows) |r| {
        const args_in = conventionArgs(r.sym) orelse continue;
        const e = model.getPtr(r.s) orelse return error.NotInModel;
        const extra = (try classify.liveIn(&cl, &e.f, ref)) & ~args_in;
        if (extra == 0) continue;
        const convention = extra & ~(R.ecx | R.edx) == 0;
        var foreign: std.StringHashMapUnmanaged(usize) = .empty;
        var theirs: std.StringHashMapUnmanaged(void) = .empty;
        if (callers.get(r.s)) |clist| for (clist.items) |cs| {
            const cr = by_start.get(cs) orelse continue;
            if (std.mem.eql(u8, cr.tu, r.tu)) continue;
            const g = try foreign.getOrPut(gpa, cr.tu);
            if (!g.found_existing) g.value_ptr.* = 0;
            g.value_ptr.* += 1;
        };
        if (!convention and foreign.count() == 0) continue; // all callers in its TU: static is enough
        if (callers.get(r.s)) |clist| for (clist.items) |cs| {
            const cr = by_start.get(cs) orelse continue;
            if (std.mem.eql(u8, cr.tu, r.tu)) continue;
            const lab = try Label.of(&files, &by_start, &fns, cs);
            if (lab.len > 0) try theirs.put(gpa, lab, {});
        };
        const ft = try gpa.alloc([]const u8, foreign.count());
        var fit = foreign.keyIterator();
        var q: usize = 0;
        while (fit.next()) |x| : (q += 1) ft[q] = x.*;
        std.mem.sort([]const u8, ft, {}, lessStr);
        const th = try gpa.alloc([]const u8, theirs.count());
        var tit = theirs.keyIterator();
        q = 0;
        while (tit.next()) |x| : (q += 1) th[q] = x.*;
        std.mem.sort([]const u8, th, {}, lessStr);
        try o.print(gpa, "{x:0>8}\t{s}\t{s}\t{s}\t{s}\t", .{ r.s, r.name, r.tu, if (convention) "convention" else "object", try regNames(extra) });
        for (ft, 0..) |t, i| try o.print(gpa, "{s}{s}:{d}", .{ if (i > 0) ", " else "", t, foreign.get(t).? });
        try o.print(gpa, "\t{s}\t", .{try Label.of(&files, &by_start, &fns, r.s)});
        for (th, 0..) |t, i| try o.print(gpa, "{s}{s}", .{ if (i > 0) ", " else "", t });
        try o.append(gpa, '\n');
        if (convention) nconv += 1 else nobj += 1;
    }
    try util.writeFile(out_file, o.items);
    var msg: std.ArrayList(u8) = .empty;
    try msg.print(gpa, "{s}: ", .{out_file});
    if (nconv > 0) try msg.print(gpa, "convention {d}", .{nconv});
    if (nobj > 0) try msg.print(gpa, "{s}object {d}", .{ if (nconv > 0) ", " else "", nobj });
    try msg.append(gpa, '\n');
    util.say(msg.items);
}
