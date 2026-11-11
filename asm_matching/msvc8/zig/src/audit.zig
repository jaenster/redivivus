//! audit (audit.py): compare every mapped function with the original.
//!
//!   oracle-zig audit --root <asm_matching/msvc8> [--funcmap F] [--libmap L] [--out O]
//!
//! One row per mapped function, sorted by original address, into evidence/audit.tsv (an
//! experiment: its build directory); a summary into <out>.summary, and the globals each of
//! ours implies into datamap.tsv beside it. The same rows, bytes and summary as audit.py.
const std = @import("std");
const c = @import("c");
const compare = @import("compare.zig");
const dec = @import("decode.zig");
const libmap = @import("libmap.zig");
const objfuncs = @import("objfuncs.zig");
const ours = @import("ours.zig");
const refimage = @import("refimage.zig");
const util = @import("util.zig");

const gpa = util.gpa;
const Model = compare.Model;

const DEFAULT_CFLAGS = "/O2 /Oy- /Ob1 /EHsc /GS /MT /W1 /D WIN32 /D NDEBUG /D _WINDOWS";
const EH_HANDLER_PREFIX = "__ehhandler$";
const EH_MAGIC = [_]u32{ 0x19930520, 0x19930521, 0x19930522 };

fn isEhMagic(v: u32) bool {
    return std.mem.indexOfScalar(u32, &EH_MAGIC, v) != null;
}

/// oracle_config.py: where a build lives, decided from its configuration.
pub const Config = struct {
    oracle: []const u8,
    repo: []const u8,
    build: []const u8,
    refcache: []const u8,
    vc: []const u8,
    cflags: []const u8,
    experiment: bool,

    pub fn init(root: []const u8) !Config {
        const vc = util.getenv("ORACLE_VC") orelse "sp1";
        const env_cflags = util.getenv("ORACLE_CFLAGS");
        const cflags = if (env_cflags != null and env_cflags.?.len > 0) env_cflags.? else DEFAULT_CFLAGS;
        const experiment = !std.mem.eql(u8, vc, "sp1") or !std.mem.eql(u8, cflags, DEFAULT_CFLAGS);
        const baseline = try util.join(root, "build");
        const explicit = util.getenv("ORACLE_BUILD");
        var build: []const u8 = baseline;
        if (experiment) {
            var md5: [16]u8 = undefined;
            std.crypto.hash.Md5.hash(env_cflags orelse "", &md5, .{});
            const tag = std.fmt.bytesToHex(md5, .lower);
            const default = try std.fmt.allocPrint(gpa, "{s}/experiment-{s}-{s}", .{ baseline, vc, tag[0..8] });
            if (explicit) |e| if (e.len > 0) {
                if (std.mem.eql(u8, try realpath(e), try realpath(baseline))) {
                    std.debug.print("refusing to build an experiment (ORACLE_VC={s}, ORACLE_CFLAGS={s}) into the baseline directory {s}\n", .{ vc, cflags, baseline });
                    std.process.exit(1);
                }
            };
            build = if (explicit != null and explicit.?.len > 0) explicit.? else default;
        } else if (explicit != null and explicit.?.len > 0) build = explicit.?;
        return .{ .oracle = root, .repo = util.dirname(util.dirname(root)), .build = build, .refcache = try util.join(baseline, "ref"), .vc = vc, .cflags = cflags, .experiment = experiment };
    }

    pub fn evidence(cfg: *const Config, name: []const u8) ![]const u8 {
        return std.fmt.allocPrint(gpa, "{s}/evidence/{s}", .{ cfg.oracle, name });
    }
};

/// os.path.realpath, for an existing path; else the path as given.
fn realpath(p: []const u8) ![]const u8 {
    var buf: [4096]u8 = undefined;
    const pz = try util.z(p);
    const r = c.realpath(pz.ptr, &buf) orelse return p;
    return gpa.dupe(u8, std.mem.sliceTo(r, 0));
}

pub const MapRow = struct { s: u32, e: u32, name: []const u8, tu: []const u8, sym: []const u8, method: []const u8, detail: []const u8 };

pub fn loadFuncmap(path: []const u8) ![]MapRow {
    var rows: std.ArrayList(MapRow) = .empty;
    var it = util.Lines{ .data = try util.readFile(path) };
    while (it.next()) |line| {
        if (std.mem.startsWith(u8, line, "start\t")) continue;
        const p = try util.split(line, '\t');
        if (p.len != 7) return error.BadFuncmapRow;
        try rows.append(gpa, .{ .s = try util.hex(p[0]), .e = try util.hex(p[1]), .name = p[2], .tu = p[3], .sym = p[4], .method = p[5], .detail = p[6] });
    }
    return rows.items;
}

/// Unambiguous CRT names from crtmap.tsv -> original start (funcmap.load_crtmap).
pub fn loadCrtmap(path: []const u8) !std.StringHashMapUnmanaged(u32) {
    var out: std.StringHashMapUnmanaged(u32) = .empty;
    const data = (try util.readFileOpt(path)) orelse return out;
    var it = util.Lines{ .data = data };
    while (it.next()) |line| {
        if (std.mem.startsWith(u8, line, "start\t")) continue;
        const p = try util.split(line, '\t');
        if (p.len != 4) return error.BadCrtmapRow;
        if (!std.mem.startsWith(u8, p[2], "?")) try out.put(gpa, p[2], try util.hex(p[0]));
    }
    return out;
}

/// The address a Ghidra-derived name ends in (FOO_00512340, FOO_512340), if any:
/// re.search(r'_(00[4-9a-f][0-9a-f]{5}|[4-9a-f][0-9a-f]{5})(?:@|$)').
pub fn embeddedAddress(name: []const u8) ?u32 {
    const H = struct {
        fn hx(ch: u8) bool {
            return (ch >= '0' and ch <= '9') or (ch >= 'a' and ch <= 'f');
        }
        fn hi(ch: u8) bool {
            return (ch >= '4' and ch <= '9') or (ch >= 'a' and ch <= 'f');
        }
        fn tail(s: []const u8, at: usize) bool {
            return at == s.len or s[at] == '@';
        }
        fn run(s: []const u8, from: usize, n: usize) bool {
            if (from + n > s.len) return false;
            for (s[from .. from + n]) |ch| if (!hx(ch)) return false;
            return true;
        }
    };
    var i: usize = 0;
    while (i < name.len) : (i += 1) {
        if (name[i] != '_') continue;
        const p = i + 1;
        if (p + 8 <= name.len and name[p] == '0' and name[p + 1] == '0' and H.hi(name[p + 2]) and H.run(name, p + 3, 5) and H.tail(name, p + 8))
            return std.fmt.parseInt(u32, name[p .. p + 8], 16) catch null;
        if (p + 6 <= name.len and H.hi(name[p]) and H.run(name, p + 1, 5) and H.tail(name, p + 6))
            return std.fmt.parseInt(u32, name[p .. p + 6], 16) catch null;
    }
    return null;
}

/// (tu, function name) -> the compile error that quarantined it.
pub fn loadQuarantine(path: []const u8) !std.StringHashMapUnmanaged([]const u8) {
    var out: std.StringHashMapUnmanaged([]const u8) = .empty;
    const data = (try util.readFileOpt(path)) orelse return out;
    var it = util.Lines{ .data = data };
    while (it.next()) |line| {
        const p = try util.split(line, '\t');
        if (p.len >= 5 and !std.mem.eql(u8, p[0], "file")) {
            const tu = if (std.mem.endsWith(u8, p[0], ".cpp")) p[0][0 .. p[0].len - 4] else p[0];
            try out.put(gpa, try qkey(tu, lastPart(p[3])), p[4]);
        }
    }
    return out;
}

pub fn qkey(tu: []const u8, name: []const u8) ![]const u8 {
    return std.fmt.allocPrint(gpa, "{s}\x00{s}", .{ tu, name });
}

/// name.rsplit('::', 1)[-1]
pub fn lastPart(name: []const u8) []const u8 {
    const i = std.mem.lastIndexOf(u8, name, "::") orelse return name;
    return name[i + 2 ..];
}

fn imm32(i: *const dec.Insn, at: usize) u32 {
    return std.mem.readInt(u32, i.bytes[at..][0..4], .little);
}

/// Original function start -> the __ehhandler stub its C++ EH frame links (eh_handler_of).
fn ehHandlerOf(img: *const @import("pe.zig").Image, model: *const Model) !std.AutoHashMapUnmanaged(u32, u32) {
    var stubs: std.AutoHashMapUnmanaged(u32, void) = .empty;
    var it = model.iterator();
    while (it.next()) |e| {
        const ins = e.value_ptr.f.insns.items;
        var k: usize = 0;
        while (k + 1 < ins.len) : (k += 1) {
            const i = &ins[k];
            if (i.bytes[0] == 0xb8 and i.size == 5 and ins[k + 1].kind == .jmp) {
                const magic = img.u32At(imm32(i, 1)) orelse break;
                if (isEhMagic(magic)) try stubs.put(gpa, e.key_ptr.*, {});
                break;
            }
        }
    }
    var out: std.AutoHashMapUnmanaged(u32, u32) = .empty;
    it = model.iterator();
    while (it.next()) |e| {
        const all = e.value_ptr.f.insns.items;
        const ins = all[0..@min(6, all.len)];
        var k: usize = 0;
        while (k + 1 < ins.len) : (k += 1) {
            const i = &ins[k];
            const j = &ins[k + 1];
            if (i.size == 2 and i.bytes[0] == 0x6a and i.bytes[1] == 0xff and j.bytes[0] == 0x68 and j.size == 5) {
                const t = imm32(j, 1);
                if (stubs.contains(t)) try out.put(gpa, e.key_ptr.*, t);
                break;
            }
        }
    }
    return out;
}

/// Our symbol -> original start: the funcmap's unambiguous symbols, the libmap's, then the
/// CRT function crtmap.tsv names, and a compiler-written `__ehhandler$<f>` as the stub the
/// original of f links into its frame.
pub const SymMap = struct {
    base: std.StringHashMapUnmanaged(u32) = .empty,
    crt: std.StringHashMapUnmanaged(u32) = .empty,
    eh: ?std.AutoHashMapUnmanaged(u32, u32) = null,
    img: *const @import("pe.zig").Image,
    model: *const Model,

    pub fn get(m: *SymMap, sym: []const u8) !?u32 {
        if (m.base.get(sym)) |v| return v;
        if (std.mem.startsWith(u8, sym, EH_HANDLER_PREFIX)) {
            const parent = m.base.get(sym[EH_HANDLER_PREFIX.len..]) orelse return null;
            if (m.eh == null) m.eh = try ehHandlerOf(m.img, m.model);
            return m.eh.?.get(parent);
        }
        return m.crt.get(try objfuncs.qualified(sym));
    }
};

pub fn symMap(rows: []const MapRow, librows: []const MapRow, crt: std.StringHashMapUnmanaged(u32), img: *const @import("pe.zig").Image, model: *const Model) !SymMap {
    var seen: std.StringArrayHashMapUnmanaged(std.AutoArrayHashMapUnmanaged(u32, void)) = .empty;
    for (rows) |r| {
        const g = try seen.getOrPut(gpa, r.sym);
        if (!g.found_existing) g.value_ptr.* = .empty;
        try g.value_ptr.put(gpa, r.s, {});
    }
    var m = SymMap{ .crt = crt, .img = img, .model = model };
    var it = seen.iterator();
    while (it.next()) |e| if (e.value_ptr.count() == 1) try m.base.put(gpa, e.key_ptr.*, e.value_ptr.keys()[0]);
    var lib: std.StringArrayHashMapUnmanaged(std.AutoArrayHashMapUnmanaged(u32, void)) = .empty;
    for (librows) |r| {
        const g = try lib.getOrPut(gpa, r.sym);
        if (!g.found_existing) g.value_ptr.* = .empty;
        try g.value_ptr.put(gpa, r.s, {});
    }
    it = lib.iterator();
    while (it.next()) |e| if (!seen.contains(e.key_ptr.*) and e.value_ptr.count() == 1) try m.base.put(gpa, e.key_ptr.*, e.value_ptr.keys()[0]);
    return m;
}

pub fn loadStarts(path: []const u8, model: *const Model, out: *std.AutoHashMapUnmanaged(u32, void)) !void {
    const data = (try util.readFileOpt(path)) orelse return;
    var it = util.Lines{ .data = data };
    while (it.next()) |line| {
        if (std.mem.startsWith(u8, line, "start\t") or util.strip(line).len == 0) continue;
        const f = line[0 .. std.mem.indexOfScalar(u8, line, '\t') orelse line.len];
        const s = try util.hex(util.strip(f));
        if (model.contains(s)) try out.put(gpa, s, {});
    }
}

/// The original functions of the statically linked C runtime (runtime_starts), and the
/// library run libmap's pairs span.
pub fn runtimeStarts(model: *const Model, librows: []const MapRow, path: []const u8, out: *std.AutoHashMapUnmanaged(u32, void)) !struct { u32, u32 } {
    var lo: ?u32 = null;
    var hi: ?u32 = null;
    for (librows) |r| {
        try out.put(gpa, r.s, {});
        lo = if (lo) |l| @min(l, r.s) else r.s;
        hi = if (hi) |h| @max(h, r.e) else r.e;
    }
    var it = model.iterator();
    while (it.next()) |e| {
        const s = e.key_ptr.*;
        if (lo != null and lo.? <= s and s < hi.?) try out.put(gpa, s, {});
        if (libmap.isImportThunk(e.value_ptr)) try out.put(gpa, s, {});
    }
    try loadStarts(path, model, out);
    return .{ lo orelse 0, hi orelse 0 };
}

/// The compiler's C++ EH code: each __ehhandler stub and each unwind funclet its FuncInfo
/// names (eh_starts).
pub fn ehStarts(img: *const @import("pe.zig").Image, model: *const Model) !std.AutoHashMapUnmanaged(u32, void) {
    const Head = [5]u32;
    var stubs: std.AutoArrayHashMapUnmanaged(u32, Head) = .empty;
    var it = model.iterator();
    while (it.next()) |e| {
        const ins = e.value_ptr.f.insns.items;
        var k: usize = 0;
        while (k + 1 < ins.len) : (k += 1) {
            const i = &ins[k];
            if (i.bytes[0] == 0xb8 and i.size == 5 and ins[k + 1].kind == .jmp) {
                const fi = imm32(i, 1);
                var h: Head = undefined;
                if (dwords(img, fi, &h) and isEhMagic(h[0])) {
                    try stubs.put(gpa, e.key_ptr.*, h);
                    break;
                }
            }
        }
    }
    var out: std.AutoHashMapUnmanaged(u32, void) = .empty;
    for (stubs.keys()) |s| try out.put(gpa, s, {});
    for (stubs.values()) |h| {
        const states = h[1];
        const unwind = h[2];
        for (0..@min(states, 4096)) |k| {
            var ent: [2]u32 = undefined;
            const va = @as(u64, unwind) + 8 * @as(u64, k);
            if (va > 0xffffffff) continue;
            if (dwords(img, @intCast(va), &ent) and model.contains(ent[1])) try out.put(gpa, ent[1], {});
        }
    }
    return out;
}

fn dwords(img: *const @import("pe.zig").Image, va: u32, out: []u32) bool {
    var buf: [32]u8 = undefined;
    const n = 4 * out.len;
    if (@as(u64, va) + n > 0x100000000) return false;
    if (!img.read(va, buf[0..n])) return false;
    for (out, 0..) |*o, k| o.* = std.mem.readInt(u32, buf[4 * k ..][0..4], .little);
    return true;
}

const OutRow = struct {
    s: u32,
    e: u32,
    name: []const u8,
    tu: []const u8,
    method: []const u8,
    ref_insns: usize,
    our_insns: usize,
    byte_diffs: usize,
    similarity: f64,
    status: []const u8,
    notes: []const u8,
    ident: usize,
    operand: usize,

    fn lessThan(_: void, a: OutRow, b: OutRow) bool {
        if (a.s != b.s) return a.s < b.s;
        if (a.e != b.e) return a.e < b.e;
        inline for (.{ "name", "tu", "method" }) |fld| {
            const o = std.mem.order(u8, @field(a, fld), @field(b, fld));
            if (o != .eq) return o == .lt;
        }
        inline for (.{ "ref_insns", "our_insns", "byte_diffs", "similarity" }) |fld| {
            if (@field(a, fld) != @field(b, fld)) return @field(a, fld) < @field(b, fld);
        }
        inline for (.{ "status", "notes" }) |fld| {
            const o = std.mem.order(u8, @field(a, fld), @field(b, fld));
            if (o != .eq) return o == .lt;
        }
        if (a.ident != b.ident) return a.ident < b.ident;
        return a.operand < b.operand;
    }
};

/// A Counter: counts in first-seen order, most_common as Python orders ties.
fn Counter(comptime K: type) type {
    return struct {
        const Self = @This();
        const Map = if (K == []const u8) std.StringArrayHashMapUnmanaged(usize) else std.AutoArrayHashMapUnmanaged(K, usize);
        map: Map = .empty,

        fn add(self: *Self, k: K, n: usize) !void {
            const g = try self.map.getOrPut(gpa, k);
            if (!g.found_existing) g.value_ptr.* = 0;
            g.value_ptr.* += n;
        }
        fn get(self: *const Self, k: K) usize {
            return self.map.get(k) orelse 0;
        }
        const Item = struct { k: K, n: usize };
        /// Highest count first; equal counts in first-seen order (a stable sort).
        fn mostCommon(self: *const Self) ![]Item {
            const items = try gpa.alloc(Item, self.map.count());
            for (self.map.keys(), self.map.values(), 0..) |k, n, i| items[i] = .{ .k = k, .n = n };
            std.mem.sort(Item, items, {}, struct {
                fn lt(_: void, a: Item, b: Item) bool {
                    return a.n > b.n;
                }
            }.lt);
            return items;
        }
    };
}

const BYTE_KINDS = [_][2][]const u8{
    .{ "whole", "functions that compile to the original bytes (MATCH)" },
    .{ "ident", "already right, but the function still differs elsewhere" },
    .{ "operand", "same instruction, other registers or offsets" },
    .{ "differ", "instructions with no matching partner" },
    .{ "noncode", "switch tables and padding in functions that still differ" },
    .{ "uncompared", "original functions with no counterpart compared" },
};

fn fnBytes(model: *const Model, s: u32) u64 {
    return model.getPtr(s).?.f.end - s;
}

/// Summary lines splitting the game's bytes into BYTE_KINDS (byte_kinds).
fn byteKinds(lines: *std.ArrayList([]const u8), model: *const Model, game: []const u32, rows: []const OutRow) !void {
    var total: u64 = 0;
    for (game) |s| total += fnBytes(model, s);
    var n: [6]u64 = @splat(0);
    var compared: u64 = 0;
    for (rows) |r| {
        const size: u64 = r.e - r.s;
        if (std.mem.eql(u8, r.status, "MISSING") or std.mem.eql(u8, r.status, "QUARANTINE")) continue;
        compared += size;
        if (std.mem.eql(u8, r.status, "MATCH")) {
            n[0] += size;
            continue;
        }
        var code: u64 = 0;
        for ((try compare.spanPieces(model, r.s, r.e)) orelse return error.NotARun) |p| for (model.getPtr(p).?.f.insns.items) |i| {
            code += i.size;
        };
        if (!(r.ident + r.operand <= code and code <= size)) return error.ByteKindsAssert;
        n[1] += r.ident;
        n[2] += r.operand;
        n[3] += code - r.ident - r.operand;
        n[4] += size - code;
    }
    if (compared > total) return error.ByteKindsAssert;
    n[5] = total - compared;
    try lines.append(gpa, try std.fmt.allocPrint(gpa, "byte kinds of the headline total: {d} bytes", .{total}));
    for (BYTE_KINDS, 0..) |bk, k| {
        var l: std.ArrayList(u8) = .empty;
        try l.print(gpa, "  kind {s:<10} {d:>8} ", .{ bk[0], n[k] });
        try util.fixed(&l, "%6.2f", 100.0 * @as(f64, @floatFromInt(n[k])) / @as(f64, @floatFromInt(@max(1, total))));
        try l.print(gpa, "% {s}", .{bk[1]});
        try lines.append(gpa, l.items);
    }
}

fn setBytes(model: *const Model, set: *std.AutoHashMapUnmanaged(u32, void)) u64 {
    var n: u64 = 0;
    var it = set.keyIterator();
    while (it.next()) |s| n += fnBytes(model, s.*);
    return n;
}

fn pct(l: *std.ArrayList(u8), a: u64, b: u64) !void {
    try util.fixed(l, "%.2f", 100.0 * @as(f64, @floatFromInt(a)) / @as(f64, @floatFromInt(b)));
}

pub fn gitDescribe(repo: []const u8) ![]const u8 {
    // git -C <repo> describe --always --dirty --tags; stdout stripped, else 'unknown'.
    var cmd: std.ArrayList(u8) = .empty;
    try cmd.appendSlice(gpa, "git -C '");
    for (repo) |ch| if (ch == '\'') try cmd.appendSlice(gpa, "'\\''") else try cmd.append(gpa, ch);
    try cmd.appendSlice(gpa, "' describe --always --dirty --tags 2>/dev/null");
    const p = c.popen((try util.z(cmd.items)).ptr, "r") orelse return "unknown";
    var out: std.ArrayList(u8) = .empty;
    var buf: [256]u8 = undefined;
    while (true) {
        const n = c.fread(&buf, 1, buf.len, p);
        if (n == 0) break;
        try out.appendSlice(gpa, buf[0..n]);
    }
    _ = c.pclose(p);
    const s = util.strip(out.items);
    return if (s.len == 0) "unknown" else s;
}

pub fn main(args: []const []const u8) !void {
    var root: ?[]const u8 = null;
    var funcmap_path: ?[]const u8 = null;
    var libmap_path: ?[]const u8 = null;
    var out_path: ?[]const u8 = null;
    var k: usize = 0;
    while (k < args.len) : (k += 1) {
        const a = args[k];
        const Opt = struct { name: []const u8, dst: *?[]const u8 };
        const opts = [_]Opt{ .{ .name = "--root", .dst = &root }, .{ .name = "--funcmap", .dst = &funcmap_path }, .{ .name = "--libmap", .dst = &libmap_path }, .{ .name = "--out", .dst = &out_path } };
        var hit = false;
        for (opts) |o| {
            if (std.mem.eql(u8, a, o.name) and k + 1 < args.len) {
                o.dst.* = args[k + 1];
                k += 1;
                hit = true;
            } else if (std.mem.startsWith(u8, a, o.name) and a.len > o.name.len and a[o.name.len] == '=') {
                o.dst.* = a[o.name.len + 1 ..];
                hit = true;
            }
        }
        if (!hit) {
            std.debug.print("usage: oracle-zig audit --root <asm_matching/msvc8> [--funcmap F] [--libmap L] [--out O]\n", .{});
            std.process.exit(2);
        }
    }
    const cfg = try Config.init(root orelse {
        std.debug.print("oracle-zig audit: --root is required\n", .{});
        std.process.exit(2);
    });
    const funcmap_file = funcmap_path orelse try cfg.evidence("funcmap.tsv");
    const libmap_file = libmap_path orelse try cfg.evidence("libmap.tsv");
    const out_file = if (out_path) |o| blk: {
        if (cfg.experiment and std.mem.eql(u8, try realpath(util.dirname(o)), try realpath(try util.join(cfg.oracle, "evidence")))) {
            std.debug.print("refusing to write an experiment audit into evidence/\n", .{});
            std.process.exit(1);
        }
        break :blk o;
    } else if (cfg.experiment) try util.join(cfg.build, "audit.tsv") else try cfg.evidence("audit.tsv");

    // The original, carved and decoded.
    const ref = try refimage.Reference.open(try util.join(cfg.oracle, "reference/Game.exe"));
    try ref.load(try util.join(cfg.refcache, "functions.tsv"));
    var model = try ref.model();

    const rows = try loadFuncmap(funcmap_file);
    var d = try dec.Decoder.init(gpa);
    var fns: std.StringHashMapUnmanaged(*const ours.OurFunc) = .empty;
    for (try ours.loadAll(&d, try util.join(cfg.build, "obj"))) |*f| try fns.put(gpa, try f.key(), f);
    const librows: []MapRow = if (util.exists(libmap_file)) try loadFuncmap(libmap_file) else &.{};
    if (librows.len > 0) for (try libmap.loadLibFunctions(&d)) |*f| try fns.put(gpa, try f.key(), f);
    var syms = try symMap(rows, librows, try loadCrtmap(try cfg.evidence("crtmap.tsv")), &ref.img, &model);
    var quarantined = try loadQuarantine(try util.join(cfg.build, "quarantine.tsv"));

    var viewer = compare.Viewer{ .d = &d };
    var out: std.ArrayList(OutRow) = .empty;
    var statuses: Counter([]const u8) = .{};
    // our global -> implied original address -> uses, in first-seen order
    var wiring: std.StringArrayHashMapUnmanaged(Counter(u32)) = .empty;
    const all_rows = try std.mem.concat(gpa, MapRow, &.{ rows, librows });
    for (all_rows) |r| {
        const key = try std.fmt.allocPrint(gpa, "{s}|{s}", .{ r.tu, r.sym });
        const f = fns.get(key);
        if (f == null or !model.contains(r.s)) {
            try out.append(gpa, .{ .s = r.s, .e = r.e, .name = r.name, .tu = r.tu, .method = r.method, .ref_insns = 0, .our_insns = 0, .byte_diffs = 0, .similarity = 0.0, .status = "MISSING", .notes = "object not compiled", .ident = 0, .operand = 0 });
            try statuses.add("MISSING", 1);
            continue;
        }
        if (quarantined.get(try qkey(r.tu, lastPart(r.name)))) |q| {
            // Its body was blanked so that the rest of its TU compiles: nothing to judge.
            try out.append(gpa, .{ .s = r.s, .e = r.e, .name = r.name, .tu = r.tu, .method = r.method, .ref_insns = model.getPtr(r.s).?.f.insns.items.len, .our_insns = 0, .byte_diffs = 0, .similarity = 0.0, .status = "QUARANTINE", .notes = q, .ident = 0, .operand = 0 });
            try statuses.add("QUARANTINE", 1);
            continue;
        }
        const res = try compare.compare(try viewer.ourView(f.?, &syms), try viewer.refViewSpan(r.s, r.e, &model));
        for (res.wiring) |w| {
            const g = try wiring.getOrPut(gpa, w.sym);
            if (!g.found_existing) g.value_ptr.* = .{};
            try g.value_ptr.add(w.addr, 1);
        }
        var notes: std.ArrayList(u8) = .empty;
        if (res.ref_diffs > 0) try notes.print(gpa, "{d} refs differ", .{res.ref_diffs});
        if (res.unverified > 0) try notes.print(gpa, "{s}{d} refs unverified", .{ if (notes.items.len > 0) "; " else "", res.unverified });
        try out.append(gpa, .{ .s = r.s, .e = r.e, .name = r.name, .tu = r.tu, .method = r.method, .ref_insns = res.ref_insns, .our_insns = res.our_insns, .byte_diffs = res.byte_diffs, .similarity = res.similarity, .status = res.status, .notes = notes.items, .ident = res.ident_bytes, .operand = res.operand_bytes });
        try statuses.add(if (std.mem.startsWith(u8, res.status, "OUR")) "OUR+-N" else res.status, 1);
    }
    std.mem.sort(OutRow, out.items, {}, OutRow.lessThan);
    util.makeDirs(util.dirname(out_file));
    {
        var o: std.ArrayList(u8) = .empty;
        try o.appendSlice(gpa, "start\tend\tname\ttu\tmethod\tref_insns\tour_insns\tbyte_diffs\tsimilarity\tstatus\tnotes\tident_bytes\toperand_bytes\n");
        for (out.items) |r| {
            try o.print(gpa, "{x:0>8}\t{x:0>8}\t{s}\t{s}\t{s}\t{d}\t{d}\t{d}\t", .{ r.s, r.e, r.name, r.tu, r.method, r.ref_insns, r.our_insns, r.byte_diffs });
            try util.fixed(&o, "%.3f", r.similarity);
            try o.print(gpa, "\t{s}\t{s}\t{d}\t{d}\n", .{ r.status, r.notes, r.ident, r.operand });
        }
        try util.writeFile(out_file, o.items);
    }

    const total_ref = model.count();
    var ref_bytes: u64 = 0;
    {
        var it = model.iterator();
        while (it.next()) |e| ref_bytes += e.value_ptr.f.end - e.key_ptr.*;
    }
    var match_bytes: u64 = 0;
    for (out.items) |r| if (std.mem.eql(u8, r.status, "MATCH")) {
        match_bytes += r.e - r.s;
    };

    // The headline: the game without the statically linked runtime, prebuilt libraries and
    // the compiler's EH code; the whole binary follows.
    var runtime: std.AutoHashMapUnmanaged(u32, void) = .empty;
    const rlo, const rhi = try runtimeStarts(&model, librows, try cfg.evidence("runtime.tsv"), &runtime);
    var prebuilt_all: std.AutoHashMapUnmanaged(u32, void) = .empty;
    try loadStarts(try cfg.evidence("prebuilt.tsv"), &model, &prebuilt_all);
    var prebuilt: std.AutoHashMapUnmanaged(u32, void) = .empty;
    {
        var it = prebuilt_all.keyIterator();
        while (it.next()) |s| if (!runtime.contains(s.*)) try prebuilt.put(gpa, s.*, {});
    }
    var runtime_only_n: usize = 0;
    var runtime_only_b: u64 = 0;
    {
        var it = runtime.keyIterator();
        while (it.next()) |s| {
            runtime_only_n += 1;
            runtime_only_b += fnBytes(&model, s.*);
        }
        var pit = prebuilt.keyIterator();
        while (pit.next()) |s| try runtime.put(gpa, s.*, {});
    }
    var eh: std.AutoHashMapUnmanaged(u32, void) = .empty;
    {
        var all_eh = try ehStarts(&ref.img, &model);
        var it = all_eh.keyIterator();
        while (it.next()) |s| if (!runtime.contains(s.*)) try eh.put(gpa, s.*, {});
    }
    var game: std.ArrayList(u32) = .empty;
    var game_bytes: u64 = 0;
    {
        var it = model.keyIterator();
        while (it.next()) |s| if (!runtime.contains(s.*) and !eh.contains(s.*)) {
            try game.append(gpa, s.*);
            game_bytes += fnBytes(&model, s.*);
        };
    }
    var game_out: std.ArrayList(OutRow) = .empty;
    var game_match: usize = 0;
    var game_match_bytes: u64 = 0;
    for (out.items) |r| if (!runtime.contains(r.s) and !eh.contains(r.s)) {
        try game_out.append(gpa, r);
        if (std.mem.eql(u8, r.status, "MATCH")) {
            game_match += 1;
            game_match_bytes += r.e - r.s;
        }
    };

    var lines: std.ArrayList([]const u8) = .empty;
    {
        var l: std.ArrayList(u8) = .empty;
        try l.print(gpa, "tree {s}; compiler {s}; flags {s}", .{ try gitDescribe(cfg.repo), cfg.vc, cfg.cflags });
        if (try util.readFileOpt(try util.join(cfg.oracle, "flags.tsv"))) |ft| {
            // Python keeps each line's '\n' and takes split('\t')[:2].
            var pos: usize = 0;
            while (pos < ft.len) {
                const nl = std.mem.indexOfScalarPos(u8, ft, pos, '\n');
                const line = ft[pos..if (nl) |x| x + 1 else ft.len];
                pos = if (nl) |x| x + 1 else ft.len;
                if (util.strip(line).len == 0 or line[0] == '#') continue;
                var it = std.mem.splitScalar(u8, line, '\t');
                const p0 = it.next().?;
                const p1 = it.next() orelse return error.BadFlagsRow;
                try l.print(gpa, "; {s} {s}", .{ p0, p1 });
            }
        }
        try lines.append(gpa, l.items);
    }
    {
        var l: std.ArrayList(u8) = .empty;
        try l.print(gpa, "Without the C runtime: {d} of {d} functions MATCH (", .{ game_match, game.items.len });
        try pct(&l, game_match, @max(1, game.items.len));
        try l.print(gpa, "%), {d} of {d} bytes (", .{ game_match_bytes, game_bytes });
        try pct(&l, game_match_bytes, @max(1, game_bytes));
        try l.appendSlice(gpa, "%)");
        try lines.append(gpa, l.items);
    }
    try lines.append(gpa, try std.fmt.allocPrint(gpa, "  runtime left out: {d} functions ({d} bytes): libmap pairs, the library run {x:0>8}..{x:0>8}, import thunks, runtime.tsv", .{ runtime_only_n, runtime_only_b, rlo, rhi }));
    try lines.append(gpa, try std.fmt.allocPrint(gpa, "  prebuilt library left out: {d} functions ({d} bytes): byte-identical to the prebuilt library objects in prebuilt.tsv", .{ prebuilt.count(), setBytes(&model, &prebuilt) }));
    try lines.append(gpa, try std.fmt.allocPrint(gpa, "  compiler EH code left out: {d} functions ({d} bytes): __ehhandler stubs and the unwind funclets their FuncInfo names", .{ eh.count(), setBytes(&model, &eh) }));
    try byteKinds(&lines, &model, game.items, game_out.items);
    try lines.append(gpa, "whole binary:");
    try lines.append(gpa, try std.fmt.allocPrint(gpa, "original functions: {d} ({d} bytes of code)", .{ total_ref, ref_bytes }));
    try lines.append(gpa, try std.fmt.allocPrint(gpa, "mapped and compared: {d}", .{out.items.len}));
    for ([_][]const u8{ "MATCH", "REF_DIFF", "INSN_MATCH", "INSN_COUNT_MATCH", "OUR+-N", "QUARANTINE", "MISSING" }) |st|
        try lines.append(gpa, try std.fmt.allocPrint(gpa, "  {s:<17} {d:>6}", .{ st, statuses.get(st) }));
    {
        var l: std.ArrayList(u8) = .empty;
        try l.print(gpa, "matched code: {d} bytes (", .{match_bytes});
        try pct(&l, match_bytes, ref_bytes);
        try l.appendSlice(gpa, "% of the original)");
        try lines.append(gpa, l.items);
    }
    {
        var libtus: std.StringHashMapUnmanaged(void) = .empty;
        for (librows) |r| try libtus.put(gpa, r.tu, {});
        var n: usize = 0;
        var b: u64 = 0;
        var nm: usize = 0;
        var bm: u64 = 0;
        for (out.items) |r| if (libtus.contains(r.tu)) {
            n += 1;
            b += r.e - r.s;
            if (std.mem.eql(u8, r.status, "MATCH")) {
                nm += 1;
                bm += r.e - r.s;
            }
        };
        try lines.append(gpa, try std.fmt.allocPrint(gpa, "  of which the statically linked runtime ({s}): {d} compared ({d} bytes), {d} MATCH ({d} bytes)", .{ util.basename(libmap_file), n, b, nm, bm }));
    }
    {
        var by_method: Counter([]const u8) = .{};
        for (out.items) |r| try by_method.add(r.method, 1);
        var l: std.ArrayList(u8) = .empty;
        try l.appendSlice(gpa, "pairings by funcmap tier: ");
        for (try by_method.mostCommon(), 0..) |it, i| try l.print(gpa, "{s}{s} {d}", .{ if (i == 0) "" else ", ", it.k, it.n });
        try lines.append(gpa, l.items);
    }
    // Globals: each of ours must imply one original address wherever it is used.
    const dm = try util.join(util.dirname(out_file), "datamap.tsv");
    var consistent: usize = 0;
    {
        const names = try gpa.dupe([]const u8, wiring.keys());
        std.mem.sort([]const u8, names, {}, struct {
            fn lt(_: void, a: []const u8, b: []const u8) bool {
                return std.mem.order(u8, a, b) == .lt;
            }
        }.lt);
        var o: std.ArrayList(u8) = .empty;
        try o.appendSlice(gpa, "symbol\taddress\tuses\tother_addresses\n");
        for (names) |sym| {
            const mc = try wiring.getPtr(sym).?.mostCommon();
            try o.print(gpa, "{s}\t{x:0>8}\t{d}\t", .{ sym, mc[0].k, mc[0].n });
            var first = true;
            for (mc) |e| if (e.k != mc[0].k) {
                try o.print(gpa, "{s}{x:0>8}:{d}", .{ if (first) "" else ",", e.k, e.n });
                first = false;
            };
            if (first) consistent += 1;
            try o.append(gpa, '\n');
        }
        try util.writeFile(dm, o.items);
    }
    try lines.append(gpa, try std.fmt.allocPrint(gpa, "globals seen at aligned instructions: {d}, of which consistent {d} ({s})", .{ wiring.count(), consistent, util.basename(dm) }));
    // Self-check: a generated name that carries its Ghidra address must land there.
    {
        var agree: usize = 0;
        var disagree: std.ArrayList([]const u8) = .empty;
        for (out.items) |r| if (embeddedAddress(r.name)) |a| if (0x401000 <= a and a < 0x6cc000) {
            if (a == r.s) agree += 1 else try disagree.append(gpa, try std.fmt.allocPrint(gpa, "{s} at {x:0>8}", .{ r.name, r.s }));
        };
        for (wiring.keys(), wiring.values()) |sym, *cnt| if (embeddedAddress(sym)) |a| if (a >= 0x6cc000) {
            const top = (try cnt.mostCommon())[0].k;
            if (top == a) agree += 1 else try disagree.append(gpa, try std.fmt.allocPrint(gpa, "{s} at {x:0>8}", .{ sym, top }));
        };
        var l: std.ArrayList(u8) = .empty;
        try l.print(gpa, "self-check, names carrying their own address: {d} agree, {d} elsewhere", .{ agree, disagree.items.len });
        for (disagree.items[0..@min(20, disagree.items.len)]) |x| try l.print(gpa, "\n  look at: {s}", .{x});
        try lines.append(gpa, l.items);
    }
    var summary: std.ArrayList(u8) = .empty;
    for (lines.items) |l| {
        try summary.appendSlice(gpa, l);
        try summary.append(gpa, '\n');
    }
    try util.writeFile(try std.fmt.allocPrint(gpa, "{s}.summary", .{out_file}), summary.items);
    util.say(summary.items);
}
