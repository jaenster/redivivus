//! permodule (permodule.py): the byte-match figures per module, and how far each module's
//! layout is from the original's.
//!
//!   oracle-zig permodule --root <asm_matching/msvc8> --map <build>/d2recon-vc8.map
//!                        [--audit evidence/audit.tsv] [--funcmap F] [--libmap L]
//!                        [--data <workspace>/docs/data] [--out evidence/permodule.tsv]
//!
//! Writes <out> (one row per module, a TOTAL row last) and <out without .tsv>.md. README.md,
//! "Per-module figures", defines every column.
const std = @import("std");
const audit = @import("audit.zig");
const modules = @import("modules.zig");
const refimage = @import("refimage.zig");
const util = @import("util.zig");

const gpa = util.gpa;
const eql = struct {
    fn f(a: []const u8, b: []const u8) bool {
        return std.mem.eql(u8, a, b);
    }
}.f;

const NEW_MODULES = [_][]const u8{ "crashy", "Game", "D2Hell" }; // in the module table, not in attribution-v2

fn isNew(m: []const u8) bool {
    for (NEW_MODULES) |n| if (eql(n, m)) return true;
    return false;
}

const MapFn = struct { addr: u32, sym: []const u8, lib: []const u8, obj: []const u8 };

fn isLowerHexN(s: []const u8, n: usize) bool {
    if (s.len != n) return false;
    for (s) |ch| if (!(std.ascii.isDigit(ch) or (ch >= 'a' and ch <= 'f'))) return false;
    return true;
}

/// load_map: every function of a VC8 .map ('f' entries of 'Publics by Value' and 'Static
/// symbols'): r'^\s*[0-9a-f]{4}:[0-9a-f]{8}\s+(\S+)\s+([0-9a-f]{8})\s+(f\s+)?(?:i\s+)?(\S+)\s*$'.
fn loadMap(path: []const u8) ![]MapFn {
    var out: std.ArrayList(MapFn) = .empty;
    var it = util.Lines{ .data = try util.readFile(path) };
    while (it.next()) |line| {
        var toks: [8][]const u8 = undefined;
        var n: usize = 0;
        var t = std.mem.tokenizeAny(u8, line, " \t\r\x0b\x0c");
        while (t.next()) |x| {
            if (n == toks.len) {
                n += 1;
                break;
            }
            toks[n] = x;
            n += 1;
        }
        if (n < 4 or n > 6) continue;
        const sec = toks[0];
        if (sec.len != 13 or sec[4] != ':' or !isLowerHexN(sec[0..4], 4) or !isLowerHexN(sec[5..], 8)) continue;
        if (!isLowerHexN(toks[2], 8)) continue;
        const rest = toks[3..n];
        var has_f = false;
        var where: []const u8 = undefined;
        switch (rest.len) {
            1 => where = rest[0],
            2 => if (eql(rest[0], "f")) {
                has_f = true;
                where = rest[1];
            } else if (eql(rest[0], "i")) {
                where = rest[1];
            } else continue,
            3 => if (eql(rest[0], "f") and eql(rest[1], "i")) {
                has_f = true;
                where = rest[2];
            } else continue,
            else => continue,
        }
        if (!has_f) continue;
        const colon = std.mem.lastIndexOfScalar(u8, where, ':');
        try out.append(gpa, .{
            .addr = try util.hex(toks[2]),
            .sym = toks[1],
            .lib = if (colon) |c| where[0..c] else "",
            .obj = if (colon) |c| where[c + 1 ..] else where,
        });
    }
    return out.items;
}

/// The length of a longest strictly increasing subsequence (bisect_left patience).
fn lisLength(seq: []const u32) !usize {
    var tails: std.ArrayList(u32) = .empty;
    for (seq) |x| {
        const i = std.sort.lowerBound(u32, tails.items, x, struct {
            fn cmp(key: u32, v: u32) std.math.Order {
                return std.math.order(key, v);
            }
        }.cmp);
        if (i == tails.items.len) try tails.append(gpa, x) else tails.items[i] = x;
    }
    return tails.items.len;
}

fn lower(s: []const u8) ![]const u8 {
    return std.ascii.allocLowerString(gpa, s);
}

const Band = struct { lo: u32, hi: u32 };
const AllBand = struct { lo: u32, hi: u32, m: []const u8, status: []const u8 };
const AuditRow = struct { tu: []const u8, status: []const u8, ident: u64 };
const Pair = struct { tu: []const u8, sym: []const u8 };

fn sortBands(b: []Band) void {
    std.mem.sort(Band, b, {}, struct {
        fn lt(_: void, x: Band, y: Band) bool {
            if (x.lo != y.lo) return x.lo < y.lo;
            return x.hi < y.hi;
        }
    }.lt);
}

/// os.path.splitext(p)[0]
fn splitextRoot(p: []const u8) []const u8 {
    const base_at = if (std.mem.lastIndexOfScalar(u8, p, '/')) |i| i + 1 else 0;
    const dot = std.mem.lastIndexOfScalar(u8, p[base_at..], '.') orelse return p;
    // A leading run of dots in the basename is not an extension.
    var k: usize = 0;
    while (k < dot and p[base_at + k] == '.') k += 1;
    if (k == dot) return p;
    return p[0 .. base_at + dot];
}

fn pct(out: *std.ArrayList(u8), a: u64, b: u64) !void {
    const v: f64 = if (b != 0) 100.0 * @as(f64, @floatFromInt(a)) / @as(f64, @floatFromInt(b)) else 0.0;
    try util.fixed(out, "%.1f", v);
}

const OutRow = struct {
    m: []const u8,
    n: usize,
    match: usize,
    nbytes: u64,
    mbytes: u64,
    ibytes: u64,
    bandtxt: []const u8,
    here: usize,
    elsewhere: usize,
    unpaired: usize,
    missing: usize,
    lib: usize,
    foreign: usize,
    lcs: usize,
    same: usize,
    best: usize,
    best_at: ?u32,
    whole: bool,
};

pub fn main(args: []const []const u8) !void {
    var root: ?[]const u8 = null;
    var map_path: ?[]const u8 = null;
    var audit_path: ?[]const u8 = null;
    var funcmap_path: ?[]const u8 = null;
    var libmap_path: ?[]const u8 = null;
    var data_dir: ?[]const u8 = null;
    var out_path: ?[]const u8 = null;
    var k: usize = 0;
    while (k < args.len) : (k += 1) {
        const a = args[k];
        const Opt = struct { name: []const u8, dst: *?[]const u8 };
        const opts = [_]Opt{
            .{ .name = "--root", .dst = &root },          .{ .name = "--map", .dst = &map_path },
            .{ .name = "--audit", .dst = &audit_path },   .{ .name = "--funcmap", .dst = &funcmap_path },
            .{ .name = "--libmap", .dst = &libmap_path }, .{ .name = "--data", .dst = &data_dir },
            .{ .name = "--out", .dst = &out_path },
        };
        var hit = false;
        for (opts) |o| {
            if (eql(a, o.name) and k + 1 < args.len) {
                o.dst.* = args[k + 1];
                k += 1;
                hit = true;
            } else if (std.mem.startsWith(u8, a, o.name) and a.len > o.name.len and a[o.name.len] == '=') {
                o.dst.* = a[o.name.len + 1 ..];
                hit = true;
            }
        }
        if (!hit) {
            std.debug.print("usage: oracle-zig permodule --root <asm_matching/msvc8> --map M [--audit A] [--funcmap F] [--libmap L] [--data D] [--out O]\n", .{});
            std.process.exit(2);
        }
    }
    const cfg = try audit.Config.init(root orelse {
        std.debug.print("oracle-zig permodule: --root is required\n", .{});
        std.process.exit(2);
    });
    const mapf = map_path orelse {
        std.debug.print("oracle-zig permodule: --map is required\n", .{});
        std.process.exit(2);
    };
    const repo = cfg.repo;
    const auditf = audit_path orelse try cfg.evidence("audit.tsv");
    const funcmapf = funcmap_path orelse try cfg.evidence("funcmap.tsv");
    const libmapf = libmap_path orelse try cfg.evidence("libmap.tsv");
    const datad = data_dir orelse blk: {
        if (util.getenv("D2_DATA")) |e| if (e.len > 0) break :blk e;
        break :blk try std.fmt.allocPrint(gpa, "{s}/../../docs/data", .{repo});
    };
    const outf = out_path orelse try cfg.evidence("permodule.tsv");

    const ref = try refimage.Reference.open(try util.join(cfg.oracle, "reference/Game.exe"));
    try ref.load(try util.join(cfg.refcache, "functions.tsv"));
    var model = try ref.model();
    const librows: []audit.MapRow = if (util.exists(libmapf)) try audit.loadFuncmap(libmapf) else &.{};
    var runtime: std.AutoHashMapUnmanaged(u32, void) = .empty;
    _ = try audit.runtimeStarts(&model, librows, try cfg.evidence("runtime.tsv"), &runtime);
    try audit.loadStarts(try cfg.evidence("prebuilt.tsv"), &model, &runtime);
    var eh = try audit.ehStarts(&ref.img, &model);
    var game_l: std.ArrayList(u32) = .empty;
    {
        var it = model.keyIterator();
        while (it.next()) |s| if (!runtime.contains(s.*) and !eh.contains(s.*)) try game_l.append(gpa, s.*);
    }
    const game = game_l.items;
    std.mem.sort(u32, game, {}, std.sort.asc(u32));
    var size: std.AutoHashMapUnmanaged(u32, u32) = .empty;
    for (game) |s| try size.put(gpa, s, model.getPtr(s).?.f.end - s);

    const linked = try modules.linked(repo);
    var names: std.StringHashMapUnmanaged([]const u8) = .empty;
    for (linked) |n| try names.put(gpa, try lower(n), n);
    const Canon = struct {
        names: *const std.StringHashMapUnmanaged([]const u8),
        fn of(cn: @This(), n: []const u8) ![]const u8 {
            return cn.names.get(try lower(n)) orelse n;
        }
    };
    const canon = Canon{ .names = &names };

    var gt: std.AutoHashMapUnmanaged(u32, []const u8) = .empty;
    {
        var it = util.Lines{ .data = try util.readFile(try util.join(datad, "attribution-v2.tsv")) };
        while (it.next()) |line| {
            const p = try util.split(line, '\t');
            if (!eql(p[0], "addr") and p.len > 2 and p[2].len > 0) try gt.put(gpa, try util.hex(p[0]), try canon.of(p[2]));
        }
    }
    var bands: std.StringArrayHashMapUnmanaged(std.ArrayList(Band)) = .empty;
    var allbands: std.ArrayList(AllBand) = .empty;
    {
        var it = util.Lines{ .data = try util.readFile(try util.join(datad, "runs-v2.tsv")) };
        while (it.next()) |line| {
            const p = try util.split(line, '\t');
            if (eql(p[0], "module")) continue;
            const m = try canon.of(p[0]);
            const lo = try util.hex(p[1]);
            const hi = try util.hex(p[2]);
            const status = p[8];
            try allbands.append(gpa, .{ .lo = lo, .hi = hi, .m = m, .status = status });
            if (!eql(status, "shared")) {
                const g = try bands.getOrPut(gpa, m);
                if (!g.found_existing) g.value_ptr.* = .empty;
                try g.value_ptr.append(gpa, .{ .lo = lo, .hi = hi });
            }
        }
    }
    var rows: std.AutoHashMapUnmanaged(u32, AuditRow) = .empty;
    {
        var it = util.Lines{ .data = try util.readFile(auditf) };
        while (it.next()) |line| {
            const p = try util.split(line, '\t');
            if (eql(p[0], "start")) continue;
            const ident: u64 = if (p[11].len > 0) try std.fmt.parseInt(u64, util.strip(p[11]), 10) else 0;
            try rows.put(gpa, try util.hex(p[0]), .{ .tu = p[3], .status = p[9], .ident = ident });
        }
    }
    var pair: std.AutoHashMapUnmanaged(u32, Pair) = .empty;
    for (try audit.loadFuncmap(funcmapf)) |r| try pair.put(gpa, r.s, .{ .tu = r.tu, .sym = r.sym });

    var of: std.AutoHashMapUnmanaged(u32, []const u8) = .empty;
    for (game) |s| {
        const tu: []const u8 = if (rows.get(s)) |r| r.tu else if (pair.get(s)) |p| p.tu else "";
        const m: []const u8 = blk: {
            if (tu.len > 0) {
                const o = try modules.moduleOf(tu, repo);
                if (isNew(o)) break :blk o;
            }
            if (gt.get(s)) |g| break :blk g;
            for (allbands.items) |b| if (b.lo <= s and s < b.hi and !eql(b.status, "shared")) break :blk b.m;
            break :blk "unattributed";
        };
        try of.put(gpa, s, m);
    }
    // The three modules attribution-v2 lacks: their bands are the runs of at least two of
    // their functions, consecutive in the game's order.
    for (NEW_MODULES) |m| {
        var nb: std.ArrayList(Band) = .empty;
        var cur: std.ArrayList(u32) = .empty;
        for (game) |s| {
            if (eql(of.get(s).?, m)) {
                try cur.append(gpa, s);
            } else if (cur.items.len > 0) {
                if (cur.items.len >= 2) {
                    const last = cur.items[cur.items.len - 1];
                    try nb.append(gpa, .{ .lo = cur.items[0], .hi = last + size.get(last).? });
                }
                cur = .empty;
            }
        }
        if (cur.items.len >= 2) {
            const last = cur.items[cur.items.len - 1];
            try nb.append(gpa, .{ .lo = cur.items[0], .hi = last + size.get(last).? });
        }
        try bands.put(gpa, m, nb);
    }

    // Ours: every function of the map, by module library.
    var libname: std.StringHashMapUnmanaged([]const u8) = .empty;
    for (linked) |n| try libname.put(gpa, try lower(if (eql(n, modules.ROOT_LIB)) "d2_root" else n), n);
    const Entry = struct { addr: u32, sym: []const u8 };
    var ours_by_lib: std.StringHashMapUnmanaged(std.ArrayList(Entry)) = .empty;
    var where3: std.StringHashMapUnmanaged(u32) = .empty; // module \x00 symbol \x00 obj
    var where2: std.StringHashMapUnmanaged(u32) = .empty; // module \x00 symbol
    for (try loadMap(mapf)) |e| {
        const m = libname.get(try lower(e.lib)) orelse continue;
        const g = try ours_by_lib.getOrPut(gpa, m);
        if (!g.found_existing) g.value_ptr.* = .empty;
        try g.value_ptr.append(gpa, .{ .addr = e.addr, .sym = e.sym });
        const k3 = try std.fmt.allocPrint(gpa, "{s}\x00{s}\x00{s}", .{ m, e.sym, try lower(e.obj) });
        const g3 = try where3.getOrPut(gpa, k3);
        if (!g3.found_existing) g3.value_ptr.* = e.addr;
        const k2 = try std.fmt.allocPrint(gpa, "{s}\x00{s}", .{ m, e.sym });
        const g2 = try where2.getOrPut(gpa, k2);
        if (!g2.found_existing) g2.value_ptr.* = e.addr;
    }
    if (ours_by_lib.count() == 0) {
        std.debug.print("{s}: no function from a module library - is it the per-module link's map?\n", .{mapf});
        std.process.exit(1);
    }

    var present: std.ArrayList([]const u8) = .empty;
    {
        var seen: std.StringHashMapUnmanaged(void) = .empty;
        for (game) |s| {
            const m = of.get(s).?;
            if (!seen.contains(m)) {
                try seen.put(gpa, m, {});
                try present.append(gpa, m);
            }
        }
        const Ctx = struct {
            order: []const []const u8,
            fn idx(cx: @This(), m: []const u8) usize {
                for (cx.order, 0..) |o, i| if (eql(o, m)) return i;
                return cx.order.len;
            }
            fn lt(cx: @This(), a: []const u8, b: []const u8) bool {
                const ia = cx.idx(a);
                const ib = cx.idx(b);
                if (ia != ib) return ia < ib;
                return std.mem.order(u8, a, b) == .lt;
            }
        };
        std.mem.sort([]const u8, present.items, Ctx{ .order = linked }, Ctx.lt);
    }

    var out: std.ArrayList(OutRow) = .empty;
    var tot = OutRow{ .m = "TOTAL", .n = 0, .match = 0, .nbytes = 0, .mbytes = 0, .ibytes = 0, .bandtxt = "", .here = 0, .elsewhere = 0, .unpaired = 0, .missing = 0, .lib = 0, .foreign = 0, .lcs = 0, .same = 0, .best = 0, .best_at = null, .whole = false };
    for (present.items) |m| {
        var fs: std.ArrayList(u32) = .empty;
        for (game) |s| if (eql(of.get(s).?, m)) try fs.append(gpa, s);
        const n = fs.items.len;
        var nbytes: u64 = 0;
        var nmatch: usize = 0;
        var mbytes: u64 = 0;
        var ibytes: u64 = 0;
        for (fs.items) |s| {
            const sz = size.get(s).?;
            nbytes += sz;
            if (rows.get(s)) |r| {
                if (eql(r.status, "MATCH")) {
                    nmatch += 1;
                    mbytes += sz;
                }
                ibytes += @min(r.ident, sz);
            }
        }
        // the longest run of consecutive MATCH functions in the original's order
        var best: usize = 0;
        var cur: usize = 0;
        var best_at: ?u32 = null;
        var cur_at: ?u32 = null;
        for (fs.items) |s| {
            if (rows.get(s)) |r| if (eql(r.status, "MATCH")) {
                if (cur == 0) cur_at = s;
                cur += 1;
                if (cur > best) {
                    best = cur;
                    best_at = cur_at;
                }
                continue;
            };
            cur = 0;
        }
        // layout
        var lib: []Entry = &.{};
        if (ours_by_lib.get(m)) |l| {
            lib = try gpa.dupe(Entry, l.items);
            std.mem.sort(Entry, lib, {}, struct {
                fn lt(_: void, a: Entry, b: Entry) bool {
                    if (a.addr != b.addr) return a.addr < b.addr;
                    return std.mem.order(u8, a.sym, b.sym) == .lt;
                }
            }.lt);
        }
        const lib_start: ?u32 = if (lib.len > 0) lib[0].addr else null;
        const Here = struct { s: u32, a: u32 };
        var here: std.ArrayList(Here) = .empty;
        var elsewhere: usize = 0;
        var missing: usize = 0;
        var unpaired: usize = 0;
        for (fs.items) |s| {
            const p = pair.get(s) orelse {
                unpaired += 1;
                continue;
            };
            const lm = try modules.moduleOf(p.tu, repo);
            const obj = try std.fmt.allocPrint(gpa, "{s}.obj", .{try lower(util.basename(p.tu))});
            const a = where3.get(try std.fmt.allocPrint(gpa, "{s}\x00{s}\x00{s}", .{ lm, p.sym, obj })) orelse
                where2.get(try std.fmt.allocPrint(gpa, "{s}\x00{s}", .{ lm, p.sym }));
            if (a == null) {
                missing += 1;
            } else if (!eql(lm, m)) {
                elsewhere += 1;
            } else try here.append(gpa, .{ .s = s, .a = a.? });
        }
        const addrs = try gpa.alloc(u32, here.items.len);
        for (here.items, 0..) |h, i| addrs[i] = h.a;
        const lcs = try lisLength(addrs);
        // The original's offset of each function: inside its band, M's bands one after another.
        var off: std.AutoHashMapUnmanaged(u32, u64) = .empty;
        var bl: []Band = &.{};
        if (bands.get(m)) |b| {
            bl = try gpa.dupe(Band, b.items);
            sortBands(bl);
        }
        {
            var base: u64 = 0;
            for (bl) |b| {
                var inb: std.ArrayList(u32) = .empty;
                for (fs.items) |s| if (b.lo <= s and s < b.hi) try inb.append(gpa, s);
                if (inb.items.len == 0) continue;
                const first = inb.items[0];
                for (inb.items) |s| try off.put(gpa, s, base + (s - first));
                const last = inb.items[inb.items.len - 1];
                base += (@as(u64, last) + size.get(last).? - first + 15) & ~@as(u64, 15);
            }
        }
        var same: usize = 0;
        for (here.items) |h| if (off.get(h.s)) |o| {
            if (@as(i64, h.a) - @as(i64, lib_start.?) == @as(i64, @intCast(o))) same += 1;
        };
        var claimed: std.AutoHashMapUnmanaged(u32, void) = .empty;
        for (here.items) |h| try claimed.put(gpa, h.a, {});
        var foreign: usize = 0;
        for (lib) |e| if (!claimed.contains(e.addr)) {
            foreign += 1;
        };
        const whole = n > 0 and nmatch == n and same == n and foreign == 0;
        var bandtxt: std.ArrayList(u8) = .empty;
        for (bl, 0..) |b, i| {
            if (i > 0) try bandtxt.append(gpa, ' ');
            try bandtxt.print(gpa, "{x:0>8}-{x:0>8}", .{ b.lo, b.hi });
        }
        const r = OutRow{ .m = m, .n = n, .match = nmatch, .nbytes = nbytes, .mbytes = mbytes, .ibytes = ibytes, .bandtxt = bandtxt.items, .here = here.items.len, .elsewhere = elsewhere, .unpaired = unpaired, .missing = missing, .lib = lib.len, .foreign = foreign, .lcs = lcs, .same = same, .best = best, .best_at = best_at, .whole = whole };
        try out.append(gpa, r);
        tot.n += r.n;
        tot.match += r.match;
        tot.nbytes += r.nbytes;
        tot.mbytes += r.mbytes;
        tot.ibytes += r.ibytes;
        tot.here += r.here;
        tot.elsewhere += r.elsewhere;
        tot.unpaired += r.unpaired;
        tot.missing += r.missing;
        tot.lib += r.lib;
        tot.foreign += r.foreign;
        tot.lcs += r.lcs;
        tot.same += r.same;
    }

    const tree = try audit.gitDescribe(repo);
    {
        var o: std.ArrayList(u8) = .empty;
        try o.print(gpa, "# tree {s}; {s}; map {s}\n", .{ tree, util.basename(auditf), util.basename(mapf) });
        try o.appendSlice(gpa, "module\tfunctions\tmatch\tbytes\tmatch_bytes\tident_bytes\tbands\tin_lib\tin_other_lib\tunpaired\tnot_linked\tlib_functions\tlib_foreign\torder_lcs\tsame_offset\tlongest_match_run\trun_start\tidentical\n");
        for (out.items) |r| {
            try o.print(gpa, "{s}\t{d}\t{d}\t{d}\t{d}\t{d}\t{s}\t{d}\t{d}\t{d}\t{d}\t{d}\t{d}\t{d}\t{d}\t{d}\t", .{ r.m, r.n, r.match, r.nbytes, r.mbytes, r.ibytes, r.bandtxt, r.here, r.elsewhere, r.unpaired, r.missing, r.lib, r.foreign, r.lcs, r.same, r.best });
            if (r.best_at) |b| try o.print(gpa, "{x:0>8}", .{b}) else try o.append(gpa, '-');
            try o.print(gpa, "\t{s}\n", .{if (r.whole) "yes" else "no"});
        }
        try o.print(gpa, "TOTAL\t{d}\t{d}\t{d}\t{d}\t{d}\t\t{d}\t{d}\t{d}\t{d}\t{d}\t{d}\t{d}\t{d}\t\t\t\n", .{ tot.n, tot.match, tot.nbytes, tot.mbytes, tot.ibytes, tot.here, tot.elsewhere, tot.unpaired, tot.missing, tot.lib, tot.foreign, tot.lcs, tot.same });
        try util.writeFile(outf, o.items);
    }
    const md = try std.fmt.allocPrint(gpa, "{s}.md", .{splitextRoot(outf)});
    {
        var o: std.ArrayList(u8) = .empty;
        try o.print(gpa, "Per-module figures, tree {s} (definitions: README.md, \"Per-module figures\").\n\n", .{tree});
        try o.appendSlice(gpa, "|Module|Functions MATCH|Bytes MATCH|Bytes identical|Order (LCS)|Same offset|Longest MATCH run|Band(s)|\n|-|-|-|-|-|-|-|-|\n");
        for (out.items) |r| {
            try o.print(gpa, "|{s}{s}|{d}/{d} (", .{ r.m, if (r.whole) " (identical)" else "", r.match, r.n });
            try pct(&o, r.match, r.n);
            try o.appendSlice(gpa, "%)|");
            try pct(&o, r.mbytes, r.nbytes);
            try o.appendSlice(gpa, "%|");
            try pct(&o, r.ibytes, r.nbytes);
            try o.appendSlice(gpa, "%|");
            try pct(&o, r.lcs, r.n);
            try o.print(gpa, "%|{d}|{d}|", .{ r.same, r.best });
            if (r.bandtxt.len > 0) {
                const bt = try std.mem.replaceOwned(u8, gpa, r.bandtxt, " ", ", ");
                try o.appendSlice(gpa, bt);
            } else try o.append(gpa, '-');
            try o.appendSlice(gpa, "|\n");
        }
        try o.print(gpa, "|Total|{d}/{d} (", .{ tot.match, tot.n });
        try pct(&o, tot.match, tot.n);
        try o.appendSlice(gpa, "%)|");
        try pct(&o, tot.mbytes, tot.nbytes);
        try o.appendSlice(gpa, "%|");
        try pct(&o, tot.ibytes, tot.nbytes);
        try o.appendSlice(gpa, "%|");
        try pct(&o, tot.lcs, tot.n);
        try o.print(gpa, "%|{d}|||\n", .{tot.same});
        try util.writeFile(md, o.items);
    }
    var o: std.ArrayList(u8) = .empty;
    try o.print(gpa, "{d} modules; {d} of {d} functions MATCH, {d} of {d} bytes; order {d}, same offset {d} -> {s}, {s}\n", .{ out.items.len, tot.match, tot.n, tot.mbytes, tot.nbytes, tot.lcs, tot.same, outf, md });
    util.say(o.items);
}

test "lis and splitext" {
    try std.testing.expectEqual(@as(usize, 3), try lisLength(&.{ 5, 1, 2, 2, 9, 3 }));
    try std.testing.expectEqualStrings("a/permodule", splitextRoot("a/permodule.tsv"));
    try std.testing.expectEqualStrings("a/.hidden", splitextRoot("a/.hidden"));
    try std.testing.expectEqualStrings("a.b/c", splitextRoot("a.b/c"));
}
