//! Map every compiled function to its address in the original (funcmap.py): the tiers,
//! strongest first - sourcemap, name, export, votes, exact, located, string, import, then
//! calls/bracket/refs/align to a fixpoint - each pairing one-to-one. Writes
//! evidence/funcmap.tsv and <out>.unmapped, row for row what funcmap.py writes.
const std = @import("std");
const c = @import("c");
const audit = @import("audit.zig");
const compare = @import("compare.zig");
const crtmap = @import("crtmap.zig");
const dec = @import("decode.zig");
const difflib = @import("difflib.zig");
const objfuncs = @import("objfuncs.zig");
const ours = @import("ours.zig");
const refimage = @import("refimage.zig");
const stage = @import("stage.zig");
const util = @import("util.zig");

const gpa = util.gpa;
const Model = compare.Model;
const NInsn = compare.NInsn;

extern fn log(x: f64) f64; // libm's, as Python's math.log

const MAX_STRING_FANOUT = 4;
const ALIGN_THRESHOLD = 0.80;
const EXACT_MIN = 6;
const OVERWHELM_VOTES = 20;
const OVERWHELM_SHARE = 0.95;
const COOKIE_CHECK: u32 = 0x00681a48; // ('fn', __security_check_cookie)
const BRACKET_MARGIN = 0.5;
const FOREIGN_GAP = 8;
const OWN_TREE = "asm_matching";

const Pair = struct { sc: f64, k: u32, s: u32, d: []const u8 };
const Entry = struct { s: u32, method: []const u8, detail: []const u8 };

fn lessStr(_: void, a: []const u8, b: []const u8) bool {
    return std.mem.order(u8, a, b) == .lt;
}

fn fmt(comptime f: []const u8, args: anytype) ![]const u8 {
    return std.fmt.allocPrint(gpa, f, args);
}

/// Python's sum() of floats (3.12+): the first item, then Neumaier's compensated sum.
pub const FSum = struct {
    f: f64 = 0,
    comp: f64 = 0,
    n: usize = 0,
    pub fn add(s: *FSum, x: f64) void {
        if (s.n == 0) {
            s.f = x;
        } else {
            const t = s.f + x;
            if (@abs(s.f) >= @abs(x)) s.comp += (s.f - t) + x else s.comp += (x - t) + s.f;
            s.f = t;
        }
        s.n += 1;
    }
    pub fn result(s: *const FSum) f64 {
        if (s.n == 0) return 0;
        if (s.comp != 0 and std.math.isFinite(s.comp)) return s.f + s.comp;
        return s.f;
    }
};

/// Matcher.bare_name: the last component of a qualified name, without a parameter list.
pub fn bareName(name0: []const u8) []const u8 {
    const name = name0[0 .. std.mem.indexOfScalar(u8, name0, '(') orelse name0.len];
    var depth: i64 = 0;
    var cut: usize = 0;
    for (name, 0..) |ch, i| {
        if (ch == '<') {
            depth += 1;
        } else if (ch == '>') {
            depth -= 1;
        } else if (ch == ':' and depth == 0 and std.mem.startsWith(u8, name[i..], "::")) cut = i + 2;
    }
    return name[cut..];
}

/// re.search(r'_(00[4-6][0-9a-f]{5}|[4-6][0-9a-f]{5})$', name)
fn nameAddress(name: []const u8) ?u32 {
    const H = struct {
        fn hx(ch: u8) bool {
            return (ch >= '0' and ch <= '9') or (ch >= 'a' and ch <= 'f');
        }
        fn ok(s: []const u8) bool {
            if (s[0] < '4' or s[0] > '6') return false;
            for (s[1..]) |ch| if (!hx(ch)) return false;
            return true;
        }
    };
    if (name.len >= 9 and name[name.len - 9] == '_' and name[name.len - 8] == '0' and name[name.len - 7] == '0' and H.ok(name[name.len - 6 ..]))
        return std.fmt.parseInt(u32, name[name.len - 8 ..], 16) catch null;
    if (name.len >= 7 and name[name.len - 7] == '_' and H.ok(name[name.len - 6 ..]))
        return std.fmt.parseInt(u32, name[name.len - 6 ..], 16) catch null;
    return null;
}

/// int(str(v), 16) for a sourcemap's address: a string or an integer; null for ValueError.
fn pyHex(v: std.json.Value) ?u32 {
    var buf: [32]u8 = undefined;
    const s0: []const u8 = switch (v) {
        .string => |s| s,
        .integer => |i| std.fmt.bufPrint(&buf, "{d}", .{i}) catch return null,
        .number_string => |s| s,
        else => return null,
    };
    var s = util.strip(s0);
    var neg = false;
    if (s.len > 0 and (s[0] == '+' or s[0] == '-')) {
        neg = s[0] == '-';
        s = s[1..];
    }
    if (s.len >= 2 and s[0] == '0' and (s[1] == 'x' or s[1] == 'X')) {
        s = s[2..];
        if (s.len > 0 and s[0] == '_') s = s[1..];
    }
    if (s.len == 0) return null;
    var v64: u64 = 0;
    var prev_us = true;
    for (s) |ch| {
        if (ch == '_') {
            if (prev_us) return null;
            prev_us = true;
            continue;
        }
        prev_us = false;
        const d = std.fmt.charToDigit(ch, 16) catch return null;
        v64 = v64 *% 16 +% d;
        if (v64 > 0xffffffffff) return null;
    }
    if (prev_us) return null;
    if (neg or v64 > 0xffffffff) return null; // never a model start
    return @intCast(v64);
}

const SeqItem = union(enum) { imp: []const u8, fn_: u32, our_fn: u32, our_sym: []const u8 };

/// Tokens for the refs tier: ('fn', start), ('imp', name), ('str', bytes) as one byte string.
fn tok(kind: u8, s: []const u8) ![]const u8 {
    var out = try gpa.alloc(u8, s.len + 1);
    out[0] = kind;
    @memcpy(out[1..], s);
    return out;
}

fn tokFn(t: u32) ![]const u8 {
    var b: [4]u8 = undefined;
    std.mem.writeInt(u32, &b, t, .little);
    return tok('f', &b);
}

const SymTable = struct {
    base: *const std.StringHashMapUnmanaged(u32),
    over_sym: ?[]const u8 = null,
    over_val: u32 = 0,
    pub fn get(t: SymTable, sym: []const u8) !?u32 {
        if (t.over_sym) |o| if (std.mem.eql(u8, o, sym)) return t.over_val;
        return t.base.get(sym);
    }
};

const Located = struct { lines: usize, files: usize };
const Group = struct { ours: std.ArrayList(u32) = .empty, theirs: std.ArrayList(u32) = .empty };
const ExactGroup = struct { ours: []u32, theirs: []u32 };
const Site = struct { sym: []const u8, t: u32 };

pub const Matcher = struct {
    ref: *refimage.Reference,
    model: *Model,
    starts: []const u32,
    fns: []ours.OurFunc,
    viewer: compare.Viewer,
    crt: std.StringHashMapUnmanaged(u32),
    crt_starts: std.AutoHashMapUnmanaged(u32, void) = .empty,
    keys: [][]const u8 = &.{},
    kid: []u32 = &.{}, // fn index -> the index by_key names for its key (the last one)
    layout: []ours.LayoutKey = &.{},
    by_symbol: std.StringArrayHashMapUnmanaged(std.ArrayList(u32)) = .empty,
    sourcemap_addr: std.AutoHashMapUnmanaged(u32, u32) = .empty,
    map: std.AutoArrayHashMapUnmanaged(u32, Entry) = .empty,
    rmap: std.AutoHashMapUnmanaged(u32, u32) = .empty,
    sorted_refs: std.AutoHashMapUnmanaged(u32, []refimage.Ref) = .empty,
    r_strings: std.AutoHashMapUnmanaged(u32, [][]const u8) = .empty,
    r_imports: std.AutoArrayHashMapUnmanaged(u32, [][]const u8) = .empty,
    r_seq: std.AutoHashMapUnmanaged(u32, []SeqItem) = .empty,
    str_index: std.StringHashMapUnmanaged(std.AutoArrayHashMapUnmanaged(u32, void)) = .empty,
    qual: std.StringHashMapUnmanaged([]const u8) = .empty,
    root: []const u8,
    // caches
    oview: []?[]NInsn = &.{},
    rview: std.AutoHashMapUnmanaged(u32, []NInsn) = .empty,
    okey: []?[]const u8 = &.{},
    rkey: std.AutoHashMapUnmanaged(u32, ?[]const u8) = .empty,
    ofree: []?[]const u8 = &.{},
    rfree: std.AutoHashMapUnmanaged(u32, ?[]const u8) = .empty,
    keys_done: bool = false,
    exact_keys: ?[]ExactGroup = null,
    located: ?LocatedCache = null,
    by_code: ?std.StringHashMapUnmanaged(Group) = null,
    sim: std.AutoHashMapUnmanaged(u64, f64) = .empty,
    mnem: compare.Interner = .{},
    sourcemaps: ?[]SourceMap = null,
    external: ?std.StringHashMapUnmanaged(std.ArrayList(u32)) = null,
    per_tu: ?[][]u32 = null,
    exact_votes: std.StringHashMapUnmanaged(std.AutoArrayHashMapUnmanaged(u32, usize)) = .empty,
    pin_votes: std.AutoHashMapUnmanaged(u32, [2]usize) = .empty,

    const LocatedCache = struct { groups: std.StringArrayHashMapUnmanaged(Group), otext: std.AutoHashMapUnmanaged(u32, []Text), rtext: std.AutoHashMapUnmanaged(u32, []Text) };
    const SmEntry = struct { q: []const u8, n: []const u8, a: u32 };
    const SourceMap = struct { ours: []u32, path: []const u8, entries: []SmEntry };
    /// What an instruction's text says for the located tier: a push of a plain number, a
    /// jump to a plain address, or anything else.
    const Text = struct { push: ?u64 = null, jump: ?u64 = null, mn: []const u8 = "" };

    pub fn init(m: *Matcher) !void {
        var cit = m.crt.valueIterator();
        while (cit.next()) |v| try m.crt_starts.put(gpa, v.*, {});
        const n = m.fns.len;
        m.keys = try gpa.alloc([]const u8, n);
        m.kid = try gpa.alloc(u32, n);
        m.layout = try gpa.alloc(ours.LayoutKey, n);
        var last: std.StringHashMapUnmanaged(u32) = .empty;
        for (m.fns, 0..) |*f, i| {
            m.keys[i] = try f.key();
            try last.put(gpa, m.keys[i], @intCast(i));
            m.layout[i] = try ours.layoutKey(f);
            const g = try m.by_symbol.getOrPut(gpa, f.f.symbol);
            if (!g.found_existing) g.value_ptr.* = .empty;
            try g.value_ptr.append(gpa, @intCast(i));
        }
        if (last.count() != n) std.debug.print("funcmap: {d} functions share a key with another\n", .{n - last.count()});
        for (m.keys, 0..) |k, i| m.kid[i] = last.get(k).?;
        m.oview = try gpa.alloc(?[]NInsn, n);
        @memset(m.oview, null);
        for (m.starts) |s| {
            const e = m.model.getPtr(s).?;
            const refs = try gpa.dupe(refimage.Ref, e.refs);
            std.mem.sort(refimage.Ref, refs, {}, refLess);
            try m.sorted_refs.put(gpa, s, refs);
            var strs: std.ArrayList([]const u8) = .empty;
            var imps: std.ArrayList([]const u8) = .empty;
            var seq: std.ArrayList(SeqItem) = .empty;
            for (refs) |r| switch (r.kind) {
                .string => try strs.append(gpa, r.bytes),
                .import => {
                    const nm = afterBang(r.bytes);
                    try imps.append(gpa, nm);
                    try seq.append(gpa, .{ .imp = nm });
                },
                .call => try seq.append(gpa, .{ .fn_ = r.int }),
                else => {},
            };
            try m.r_strings.put(gpa, s, strs.items);
            try m.r_imports.put(gpa, s, imps.items);
            try m.r_seq.put(gpa, s, seq.items);
            for (strs.items) |st| {
                const g = try m.str_index.getOrPut(gpa, st);
                if (!g.found_existing) g.value_ptr.* = .empty;
                try g.value_ptr.put(gpa, s, {});
            }
        }
    }

    fn afterBang(s: []const u8) []const u8 {
        const i = std.mem.indexOfScalar(u8, s, '!') orelse return s;
        return s[i + 1 ..];
    }

    fn kindName(k: refimage.RefKind) []const u8 {
        return switch (k) {
            .call => "call",
            .data => "data",
            .fptr => "fptr",
            .import => "import",
            .self => "self",
            .string => "string",
        };
    }

    fn refLess(_: void, a: refimage.Ref, b: refimage.Ref) bool {
        if (a.a != b.a) return a.a < b.a;
        if (a.off != b.off) return a.off < b.off;
        if (a.size != b.size) return a.size < b.size;
        const o = std.mem.order(u8, kindName(a.kind), kindName(b.kind));
        if (o != .eq) return o == .lt;
        return switch (a.kind) {
            .string, .import => std.mem.order(u8, a.bytes, b.bytes) == .lt,
            else => a.int < b.int,
        };
    }

    fn qualified(m: *Matcher, sym: []const u8) ![]const u8 {
        const g = try m.qual.getOrPut(gpa, sym);
        if (!g.found_existing) g.value_ptr.* = try objfuncs.qualified(sym);
        return g.value_ptr.*;
    }

    fn mapped(m: *const Matcher, i: usize) bool {
        return m.map.contains(m.kid[i]);
    }

    fn insns(m: *const Matcher, i: usize) usize {
        return m.fns[i].f.decoded.insns.items.len;
    }

    fn end(m: *const Matcher, s: u32) u32 {
        return m.model.getPtr(s).?.f.end;
    }

    // -- bookkeeping ---------------------------------------------------------

    /// One-to-one, highest score first; a key or start proposed twice at equal best score
    /// is dropped.
    fn assign(m: *Matcher, pairs: []const Pair, method: []const u8) !usize {
        const BK = struct { sc: f64, s: u32, d: []const u8, ties: usize };
        const BS = struct { sc: f64, k: u32, n: usize };
        var best_k: std.AutoArrayHashMapUnmanaged(u32, BK) = .empty;
        var best_s: std.AutoHashMapUnmanaged(u32, BS) = .empty;
        const is_sm = std.mem.eql(u8, method, "sourcemap");
        for (pairs) |p| {
            if (m.map.contains(p.k) or m.rmap.contains(p.s)) continue;
            if (!is_sm and !m.sourcemapAllows(p.k, p.s)) continue;
            if (best_k.getPtr(p.k)) |b| {
                if (p.sc > b.sc) {
                    b.* = .{ .sc = p.sc, .s = p.s, .d = p.d, .ties = 1 };
                } else if (p.sc == b.sc and p.s != b.s) b.ties += 1;
            } else try best_k.put(gpa, p.k, .{ .sc = p.sc, .s = p.s, .d = p.d, .ties = 1 });
            if (best_s.getPtr(p.s)) |b| {
                if (p.sc > b.sc) {
                    b.* = .{ .sc = p.sc, .k = p.k, .n = 1 };
                } else if (p.sc == b.sc and p.k != b.k) b.n += 1;
            } else try best_s.put(gpa, p.s, .{ .sc = p.sc, .k = p.k, .n = 1 });
        }
        const Item = struct { k: u32, v: BK };
        const items = try gpa.alloc(Item, best_k.count());
        for (best_k.keys(), best_k.values(), 0..) |k, v, i| items[i] = .{ .k = k, .v = v };
        std.mem.sort(Item, items, {}, struct {
            fn lt(_: void, a: Item, b: Item) bool {
                return a.v.sc > b.v.sc;
            }
        }.lt);
        var n: usize = 0;
        for (items) |it| {
            if (it.v.ties > 1) continue;
            const bs = best_s.get(it.v.s) orelse continue;
            if (bs.k != it.k or bs.n > 1) continue;
            if (m.map.contains(it.k) or m.rmap.contains(it.v.s)) continue;
            try m.map.put(gpa, it.k, .{ .s = it.v.s, .method = method, .detail = it.v.d });
            try m.rmap.put(gpa, it.v.s, it.k);
            n += 1;
        }
        return n;
    }

    fn sourcemapAllows(m: *const Matcher, k: u32, s: u32) bool {
        const a = m.sourcemap_addr.get(k) orelse return true;
        return s <= a and a < m.end(s);
    }

    fn resolveCallee(m: *const Matcher, caller: usize, sym: []const u8) ?u32 {
        const cands = m.by_symbol.get(sym) orelse return null;
        if (cands.items.len == 1) return cands.items[0];
        var found: ?u32 = null;
        var count: usize = 0;
        for (cands.items) |g| if (std.mem.eql(u8, m.fns[g].tu, m.fns[caller].tu)) {
            count += 1;
            if (found == null) found = g;
        };
        return if (count == 1) found else null;
    }

    /// The original start our callee `sym` stands for: mapped, or a known CRT function.
    fn calleeStart(m: *Matcher, caller: usize, sym: []const u8) !?u32 {
        const g = m.resolveCallee(caller, sym);
        if (g) |gi| {
            if (m.map.get(m.kid[gi])) |e| return e.s;
            return null;
        }
        return m.crt.get(try m.qualified(sym));
    }

    // -- views and code keys -------------------------------------------------

    fn ourView(m: *Matcher, i: usize) ![]NInsn {
        if (m.oview[i]) |v| return v;
        const v = try m.viewer.ourView(&m.fns[i], crtmap.NoSyms{});
        m.oview[i] = v;
        return v;
    }

    fn refView(m: *Matcher, s: u32) ![]NInsn {
        if (m.rview.get(s)) |v| return v;
        const v = try m.viewer.refViewSpan(s, m.end(s), m.model);
        try m.rview.put(gpa, s, v);
        return v;
    }

    /// code_key: per instruction the masked bytes and the references known on both sides;
    /// null for code with a straddling relocation.
    fn codeKey(view: []const NInsn) !?[]const u8 {
        var out: std.ArrayList(u8) = .empty;
        for (view) |i| {
            try out.print(gpa, "{d}:", .{i.masked.len});
            try out.appendSlice(gpa, i.masked);
            try out.print(gpa, "{d}", .{i.tokens.len});
            for (i.tokens) |t| switch (t.kind) {
                .straddle => return null,
                .str => try out.print(gpa, "s{d}:{s}", .{ t.s.len, t.s }),
                .import => try out.print(gpa, "i{d}:{s}", .{ t.s.len, t.s }),
                .self => try out.print(gpa, "e{d};", .{t.int}),
                .fn_, .fnq => try out.append(gpa, 'f'),
                .data_sym, .data_addr => try out.append(gpa, 'd'),
            };
            try out.append(gpa, '|');
        }
        return out.items;
    }

    /// callfree: a code key that identifies by itself (no call or function pointer, at least
    /// three instructions), else null.
    fn callFree(view: []const NInsn, key: ?[]const u8) ?[]const u8 {
        const k = key orelse return null;
        if (view.len < 3) return null;
        for (view) |i| for (i.tokens) |t| if (t.kind == .fn_ or t.kind == .fnq) return null;
        return k;
    }

    fn codeKeys(m: *Matcher) !void {
        if (m.keys_done) return;
        m.okey = try gpa.alloc(?[]const u8, m.fns.len);
        m.ofree = try gpa.alloc(?[]const u8, m.fns.len);
        for (m.fns, 0..) |_, i| {
            const v = try m.ourView(i);
            m.okey[i] = try codeKey(v);
            m.ofree[i] = callFree(v, m.okey[i]);
        }
        for (m.starts) |s| {
            const v = try m.refView(s);
            const k = try codeKey(v);
            try m.rkey.put(gpa, s, k);
            try m.rfree.put(gpa, s, callFree(v, k));
        }
        m.keys_done = true;
    }

    /// okey/ofree of the function by_key names for fn i's key.
    fn okeyOf(m: *const Matcher, i: usize) ?[]const u8 {
        return m.okey[m.kid[i]];
    }

    fn ofreeOf(m: *const Matcher, i: usize) ?[]const u8 {
        return m.ofree[m.kid[i]];
    }

    // -- tiers ---------------------------------------------------------------

    fn sourcemapFiles(m: *Matcher) !std.StringHashMapUnmanaged([]const u8) {
        var out: std.StringHashMapUnmanaged([]const u8) = .empty;
        try walkMaps(m.root, "", &out);
        return out;
    }

    fn walkMaps(root: []const u8, rel: []const u8, out: *std.StringHashMapUnmanaged([]const u8)) !void {
        const dir_path = if (rel.len == 0) root else try util.join(root, rel);
        const dz = try util.z(dir_path);
        const dp = c.opendir(dz.ptr) orelse return;
        var dirs: std.ArrayList([]const u8) = .empty;
        while (c.readdir(dp)) |ent| {
            const name = std.mem.sliceTo(@as([*:0]const u8, @ptrCast(&ent.*.d_name)), 0);
            if (std.mem.eql(u8, name, ".") or std.mem.eql(u8, name, "..")) continue;
            const nm = try gpa.dupe(u8, name);
            const full = try util.join(dir_path, nm);
            var st: c.struct_stat = undefined;
            const fz = try util.z(full);
            if (c.lstat(fz.ptr, &st) != 0) continue;
            const is_dir = (st.st_mode & c.S_IFMT) == c.S_IFDIR;
            const relp = if (rel.len == 0) nm else try util.join(rel, nm);
            if (is_dir) {
                if (nm[0] != '.' and !std.mem.eql(u8, nm, "asm_matching")) try dirs.append(gpa, relp);
            } else {
                // os.walk lists a symlink to a file among the files, as stat() sees it.
                var is_file = true;
                if ((st.st_mode & c.S_IFMT) == c.S_IFLNK) {
                    var st2: c.struct_stat = undefined;
                    is_file = c.stat(fz.ptr, &st2) == 0 and (st2.st_mode & c.S_IFMT) != c.S_IFDIR;
                }
                if (is_file and std.mem.endsWith(u8, nm, ".cpp.map")) {
                    const k = try gpa.dupe(u8, relp[0 .. relp.len - 8]);
                    for (k) |*ch| ch.* = std.ascii.toLower(ch.*);
                    try out.put(gpa, k, full);
                }
            }
        }
        _ = c.closedir(dp);
        for (dirs.items) |d| try walkMaps(root, d, out);
    }

    fn sourcemapEntries(m: *Matcher) ![]SourceMap {
        if (m.sourcemaps) |s| return s;
        var by_tu: std.StringArrayHashMapUnmanaged(std.ArrayList(u32)) = .empty;
        for (m.fns, 0..) |f, i| {
            const g = try by_tu.getOrPut(gpa, f.tu);
            if (!g.found_existing) g.value_ptr.* = .empty;
            try g.value_ptr.append(gpa, @intCast(i));
        }
        var maps = try m.sourcemapFiles();
        var out: std.ArrayList(SourceMap) = .empty;
        var it = by_tu.iterator();
        while (it.next()) |e| {
            const low = try gpa.dupe(u8, e.key_ptr.*);
            for (low) |*ch| ch.* = std.ascii.toLower(ch.*);
            const path = maps.get(low) orelse continue;
            const text = try util.readFile(path);
            const parsed = std.json.parseFromSliceLeaky(std.json.Value, gpa, text, .{}) catch continue;
            var entries: std.ArrayList(SmEntry) = .empty;
            if (parsed == .object) if (parsed.object.get("functions")) |fl| if (fl == .array) for (fl.array.items) |fv| {
                if (fv != .object) continue;
                const a = pyHex(fv.object.get("address") orelse std.json.Value{ .string = "" }) orelse continue;
                const name = if (fv.object.get("name")) |v| (if (v == .string) v.string else "") else "";
                const ns = if (fv.object.get("namespace")) |v| (if (v == .string) v.string else "") else "";
                const q = if (ns.len > 0) try fmt("{s}::{s}", .{ ns, name }) else name;
                try entries.append(gpa, .{ .q = q, .n = name, .a = a });
            };
            try out.append(gpa, .{ .ours = e.value_ptr.items, .path = path, .entries = entries.items });
        }
        m.sourcemaps = out.items;
        return out.items;
    }

    fn bySourcemaps(m: *Matcher) !usize {
        var pairs: std.ArrayList(Pair) = .empty;
        for (try m.sourcemapEntries()) |sm| {
            var names: std.StringHashMapUnmanaged(u32) = .empty;
            var ours_bare: std.StringHashMapUnmanaged(usize) = .empty;
            var bare: std.StringHashMapUnmanaged(u32) = .empty;
            for (sm.ours) |i| {
                try names.put(gpa, m.fns[i].f.name, i);
                const b = bareName(m.fns[i].f.name);
                const g = try ours_bare.getOrPut(gpa, b);
                if (!g.found_existing) g.value_ptr.* = 0;
                g.value_ptr.* += 1;
                try bare.put(gpa, b, i);
            }
            var map_bare: std.StringHashMapUnmanaged(usize) = .empty;
            for (sm.entries) |e| {
                const g = try map_bare.getOrPut(gpa, bareName(e.n));
                if (!g.found_existing) g.value_ptr.* = 0;
                g.value_ptr.* += 1;
            }
            const base = util.basename(sm.path);
            for (sm.entries) |e| {
                var f: ?u32 = names.get(e.q) orelse names.get(e.n);
                var detail = try fmt("sourcemap {s}", .{base});
                if (f == null) {
                    const b = bareName(e.n);
                    if (b.len > 0 and (ours_bare.get(b) orelse 0) == 1 and (map_bare.get(b) orelse 0) == 1) {
                        f = bare.get(b).?;
                        detail = try fmt("{s} (bare name)", .{detail});
                    }
                }
                if (f == null) {
                    f = try m.keptElsewhere(e.q, e.n);
                    if (f) |fi| detail = try fmt("{s} (the copy the link keeps, {s})", .{ detail, m.fns[fi].tu });
                }
                if (f) |fi| {
                    try m.sourcemap_addr.put(gpa, m.kid[fi], e.a);
                    if (m.model.contains(e.a)) try pairs.append(gpa, .{ .sc = 10.0, .k = m.kid[fi], .s = e.a, .d = detail });
                }
            }
        }
        return m.assign(pairs.items, "sourcemap");
    }

    fn keptElsewhere(m: *Matcher, q: []const u8, n: []const u8) !?u32 {
        if (m.external == null) {
            m.external = .empty;
            for (m.fns, 0..) |f, i| if (!f.f.static) {
                const g = try m.external.?.getOrPut(gpa, bareName(f.f.name));
                if (!g.found_existing) g.value_ptr.* = .empty;
                try g.value_ptr.append(gpa, @intCast(i));
            };
        }
        const b = bareName(n);
        const cands: []const u32 = if (b.len > 0) (if (m.external.?.get(b)) |l| l.items else &.{}) else &.{};
        var exact: ?u32 = null;
        var ne: usize = 0;
        for (cands) |i| if (std.mem.eql(u8, m.fns[i].f.name, q)) {
            ne += 1;
            exact = i;
        };
        if (ne == 1) return exact;
        const ns = if (std.mem.endsWith(u8, q, n)) q[0 .. q.len - n.len] else q;
        const suffix = try fmt("::{s}", .{b});
        var near: ?u32 = null;
        var nn: usize = 0;
        for (cands) |i| {
            const full = m.fns[i].f.name;
            const name = full[0 .. std.mem.indexOfScalar(u8, full, '(') orelse full.len];
            if (!std.mem.endsWith(u8, name, suffix)) continue;
            const qual = name[0 .. name.len - b.len];
            if (qual.len > 0 and std.mem.startsWith(u8, ns, qual)) {
                nn += 1;
                if (near == null) near = i;
            }
        }
        return if (nn == 1) near else null;
    }

    fn bySourcemapRest(m: *Matcher) !usize {
        var pairs: std.ArrayList(Pair) = .empty;
        for (try m.sourcemapEntries()) |sm| {
            var by_bare: std.StringHashMapUnmanaged(std.ArrayList(u32)) = .empty;
            for (sm.ours) |i| {
                const g = try by_bare.getOrPut(gpa, bareName(m.fns[i].f.name));
                if (!g.found_existing) g.value_ptr.* = .empty;
                try g.value_ptr.append(gpa, i);
            }
            var map_by_bare: std.StringArrayHashMapUnmanaged(std.ArrayList(u32)) = .empty;
            for (sm.entries) |e| {
                const g = try map_by_bare.getOrPut(gpa, bareName(e.n));
                if (!g.found_existing) g.value_ptr.* = .empty;
                try g.value_ptr.append(gpa, e.a);
            }
            var it = map_by_bare.iterator();
            while (it.next()) |e| {
                const b = e.key_ptr.*;
                const addrs = e.value_ptr.items;
                const fs: []const u32 = if (by_bare.get(b)) |l| l.items else &.{};
                if (b.len == 0 or addrs.len < 2) continue;
                var uniq: std.AutoHashMapUnmanaged(u32, void) = .empty;
                for (addrs) |a| try uniq.put(gpa, a, {});
                if (uniq.count() != addrs.len) continue;
                var free_a: ?u32 = null;
                var nfa: usize = 0;
                for (addrs) |a| if (!m.rmap.contains(a)) {
                    nfa += 1;
                    free_a = a;
                };
                var free_f: ?u32 = null;
                var nff: usize = 0;
                for (fs) |i| if (!m.mapped(i)) {
                    nff += 1;
                    free_f = i;
                };
                if (nfa != 1 or nff != 1 or !m.model.contains(free_a.?)) continue;
                var keys: std.AutoHashMapUnmanaged(u32, void) = .empty;
                for (fs) |i| try keys.put(gpa, m.kid[i], {});
                var bad = false;
                for (addrs) |a| if (a != free_a.? and !keys.contains(m.rmap.get(a).?)) {
                    bad = true;
                };
                if (bad) continue;
                try pairs.append(gpa, .{ .sc = 10.0, .k = m.kid[free_f.?], .s = free_a.?, .d = try fmt("sourcemap {s} (bare name, the last of {d})", .{ util.basename(sm.path), addrs.len }) });
            }
        }
        return m.assign(pairs.items, "sourcemap");
    }

    fn byNames(m: *Matcher) !usize {
        var pairs: std.ArrayList(Pair) = .empty;
        for (m.fns, 0..) |f, i| {
            const a = nameAddress(f.f.name) orelse continue;
            if (!m.model.contains(a)) continue;
            const last = if (std.mem.lastIndexOf(u8, f.f.name, "::")) |p| f.f.name[p + 2 ..] else f.f.name;
            try pairs.append(gpa, .{ .sc = 5.0, .k = m.kid[i], .s = a, .d = try fmt("name {s}", .{last}) });
        }
        return m.assign(pairs.items, "name");
    }

    fn byExports(m: *Matcher) !usize {
        var exports: std.StringArrayHashMapUnmanaged(u32) = .empty;
        const img = &m.ref.img;
        const rva = img.dirs[0][0];
        if (rva != 0) {
            var hdr: [40]u8 = undefined;
            if (img.read(img.base + rva, &hdr)) {
                const nnames = std.mem.readInt(u32, hdr[24..28], .little);
                const afun = std.mem.readInt(u32, hdr[28..32], .little);
                const anames = std.mem.readInt(u32, hdr[32..36], .little);
                const aords = std.mem.readInt(u32, hdr[36..40], .little);
                for (0..nnames) |i| {
                    const np = img.u32At(img.base + anames + 4 * @as(u32, @intCast(i))) orelse continue;
                    const nm = (try img.cstring(gpa, img.base + np, 4096)) orelse continue;
                    var ob: [2]u8 = undefined;
                    if (!img.read(img.base + aords + 2 * @as(u32, @intCast(i)), &ob)) continue;
                    const o = std.mem.readInt(u16, &ob, .little);
                    const f = img.u32At(img.base + afun + 4 * @as(u32, o)) orelse continue;
                    try exports.put(gpa, try objfuncs.qualified(nm), img.base + f);
                }
            }
        }
        var pairs: std.ArrayList(Pair) = .empty;
        for (m.fns, 0..) |f, i| {
            for (exports.keys(), exports.values()) |q, v| {
                if (!m.model.contains(v)) continue;
                const hit = std.mem.eql(u8, f.f.name, q) or (f.f.name.len >= q.len + 2 and std.mem.endsWith(u8, f.f.name, q) and std.mem.eql(u8, f.f.name[f.f.name.len - q.len - 2 .. f.f.name.len - q.len], "::"));
                if (hit) try pairs.append(gpa, .{ .sc = 3.0, .k = m.kid[i], .s = v, .d = try fmt("export {s}", .{q}) });
            }
        }
        return m.assign(pairs.items, "export");
    }

    fn byExact(m: *Matcher) !usize {
        if (m.exact_keys == null) {
            try m.codeKeys();
            var ours_k: std.StringHashMapUnmanaged(std.ArrayList(u32)) = .empty;
            for (m.fns, 0..) |_, i| {
                if (m.insns(i) < EXACT_MIN) continue;
                const k = m.okeyOf(i) orelse continue;
                const g = try ours_k.getOrPut(gpa, k);
                if (!g.found_existing) g.value_ptr.* = .empty;
                try g.value_ptr.append(gpa, m.kid[i]);
            }
            var theirs: std.StringArrayHashMapUnmanaged(std.ArrayList(u32)) = .empty;
            for (m.starts) |s| {
                const k = m.rkey.get(s).? orelse continue;
                if (!ours_k.contains(k) or m.model.getPtr(s).?.f.insns.items.len < EXACT_MIN) continue;
                const g = try theirs.getOrPut(gpa, k);
                if (!g.found_existing) g.value_ptr.* = .empty;
                try g.value_ptr.append(gpa, s);
            }
            var list: std.ArrayList(ExactGroup) = .empty;
            for (theirs.keys(), theirs.values()) |k, v| try list.append(gpa, .{ .ours = ours_k.get(k).?.items, .theirs = v.items });
            m.exact_keys = list.items;
        }
        var pairs: std.ArrayList(Pair) = .empty;
        for (m.exact_keys.?) |e| {
            var k1: ?u32 = null;
            var nk: usize = 0;
            for (e.ours) |k| if (!m.map.contains(k)) {
                nk += 1;
                k1 = k;
            };
            var s1: ?u32 = null;
            var ns: usize = 0;
            for (e.theirs) |s| if (!m.rmap.contains(s)) {
                ns += 1;
                s1 = s;
            };
            if (nk == 1 and ns == 1) try pairs.append(gpa, .{ .sc = 6.0, .k = k1.?, .s = s1.?, .d = try fmt("identical code, {d} insns", .{m.insns(k1.?)}) });
        }
        return m.assign(pairs.items, "exact");
    }

    // The located tier reads the rendered text of a few kinds of instruction.

    fn parsePush(text: []const u8) ?u64 {
        if (!std.mem.startsWith(u8, text, "push ")) return null;
        return parseNum(text[5..], true);
    }

    /// 0x[0-9a-f]+, or with `dec` also \d+.
    fn parseNum(s: []const u8, dec_ok: bool) ?u64 {
        if (s.len > 2 and s[0] == '0' and s[1] == 'x') {
            for (s[2..]) |ch| if (!((ch >= '0' and ch <= '9') or (ch >= 'a' and ch <= 'f'))) return null;
            return std.fmt.parseInt(u64, s[2..], 16) catch null;
        }
        if (!dec_ok or s.len == 0) return null;
        for (s) |ch| if (ch < '0' or ch > '9') return null;
        return std.fmt.parseInt(u64, s, 10) catch null;
    }

    fn parseJump(text: []const u8) ?u64 {
        const sp = std.mem.indexOfScalar(u8, text, ' ') orelse return null;
        const mn = text[0..sp];
        var ok = false;
        if (mn.len >= 2 and mn[0] == 'j') {
            ok = true;
            for (mn[1..]) |ch| if (ch < 'a' or ch > 'z') {
                ok = false;
            };
        } else if (std.mem.startsWith(u8, mn, "loop")) {
            ok = true;
            for (mn[4..]) |ch| if (ch < 'a' or ch > 'z') {
                ok = false;
            };
        }
        if (!ok) return null;
        return parseNum(text[sp + 1 ..], false);
    }

    const RELATIVE = [_][]const u8{ "call", "jmp", "xbegin", "jo", "jno", "jb", "jae", "je", "jne", "jbe", "ja", "js", "jns", "jp", "jnp", "jl", "jge", "jle", "jg", "jecxz", "jcxz", "loop", "loope", "loopne" };

    fn isRelative(mn: []const u8) bool {
        for (RELATIVE) |r| if (std.mem.eql(u8, mn, r)) return true;
        return false;
    }

    fn textOf(m: *Matcher, i: *const dec.Insn, addr: u32, nfields: usize, labelled: bool) !Text {
        if (labelled) return .{ .mn = i.mn };
        var buf: [256]u8 = undefined;
        var s: []const u8 = undefined;
        if (nfields == 0 and (addr == i.addr or !isRelative(i.mn))) {
            const t = try std.fmt.bufPrint(&buf, "{s} {s}", .{ i.mn, i.op });
            s = util.strip(t);
        } else {
            s = m.viewer.d.render(i.code(), addr, &buf) orelse "(bad)";
        }
        return .{ .push = parsePush(s), .jump = parseJump(s), .mn = i.mn };
    }

    fn ourTexts(m: *Matcher, i: usize) ![]Text {
        const v = try m.ourView(i);
        const f = &m.fns[i];
        const out = try gpa.alloc(Text, v.len);
        for (f.f.decoded.insns.items, 0..) |*ins, n| out[n] = try m.textOf(ins, ins.addr, v[n].tokens.len, v[n].tokens.len > 0);
        return out;
    }

    fn refTexts(m: *Matcher, s: u32) ![]Text {
        const e = m.model.getPtr(s).?;
        var fields: std.AutoHashMapUnmanaged(u32, struct { n: usize, labelled: bool }) = .empty;
        for (e.refs) |r| {
            const g = try fields.getOrPut(gpa, r.a);
            if (!g.found_existing) g.value_ptr.* = .{ .n = 0, .labelled = false };
            g.value_ptr.n += 1;
        }
        const out = try gpa.alloc(Text, e.f.insns.items.len);
        for (e.f.insns.items, 0..) |*ins, n| {
            var nf: usize = 0;
            var lab = false;
            if (fields.get(ins.addr)) |fl| {
                nf = fl.n;
                for (e.refs) |r| if (r.a == ins.addr and r.size == 4 and @as(u32, r.off) + 4 <= ins.size) {
                    lab = true;
                };
            }
            out[n] = try m.textOf(ins, ins.addr - s, nf, lab);
        }
        return out;
    }

    /// Per instruction: the coarse key element, joined.
    fn coarseKey(view: []const NInsn, texts: []const Text) ![]const u8 {
        var out: std.ArrayList(u8) = .empty;
        for (view, texts) |i, t| {
            if (t.push != null) {
                try out.appendSlice(gpa, "P|");
            } else if (t.jump != null) {
                try out.print(gpa, "J{s}|", .{t.mn});
            } else {
                try out.print(gpa, "T{d}:", .{i.masked.len});
                try out.appendSlice(gpa, i.masked);
                try out.print(gpa, "{d}", .{i.tokens.len});
                for (i.tokens) |tk| switch (tk.kind) {
                    .straddle => unreachable,
                    .str => try out.append(gpa, 's'),
                    .self => try out.append(gpa, 'e'),
                    .import => try out.print(gpa, "i{d}:{s}", .{ tk.s.len, tk.s }),
                    .fn_, .fnq => try out.append(gpa, 'f'),
                    .data_sym, .data_addr => try out.append(gpa, 'd'),
                };
                try out.append(gpa, '|');
            }
        }
        return out.items;
    }

    /// The code-key form of one token: (kind, payload) with fn and data reduced to kinds.
    fn tokEq(a: compare.Token, b: compare.Token) bool {
        const ka = keyKind(a.kind);
        if (ka != keyKind(b.kind)) return false;
        return switch (ka) {
            's', 'i' => std.mem.eql(u8, a.s, b.s),
            'e' => a.int == b.int,
            else => true,
        };
    }

    fn keyKind(k: compare.TokKind) u8 {
        return switch (k) {
            .str => 's',
            .import => 'i',
            .self => 'e',
            .fn_, .fnq => 'f',
            .data_sym, .data_addr => 'd',
            .straddle => 'x',
        };
    }

    fn locatedEqual(m: *Matcher, k: u32, s: u32, ot: []const Text, rt: []const Text) !?Located {
        const ov = try m.ourView(k);
        const rv = try m.refView(s);
        if (ov.len != rv.len) return null;
        var oi: std.AutoHashMapUnmanaged(u64, usize) = .empty;
        var ri: std.AutoHashMapUnmanaged(u64, usize) = .empty;
        for (ov, 0..) |x, n| try oi.put(gpa, x.addr, n);
        for (rv, 0..) |y, n| try ri.put(gpa, y.addr, n);
        var lines: usize = 0;
        var files: usize = 0;
        const of = &m.fns[k];
        for (ov, rv, ot, rt) |x, y, tx, ty| {
            if (tx.jump != null or ty.jump != null) {
                if (!(tx.jump != null and ty.jump != null and std.mem.eql(u8, tx.mn, ty.mn))) return null;
                const a = oi.get(tx.jump.?) orelse return null;
                const b = ri.get(ty.jump.?) orelse return null;
                if (a != b) return null;
                continue;
            }
            const same_m = std.mem.eql(u8, x.masked, y.masked);
            var same_t = x.tokens.len == y.tokens.len;
            if (same_t) for (x.tokens, y.tokens) |p, q| if (!tokEq(p, q)) {
                same_t = false;
            };
            if (same_m and same_t) continue;
            if (tx.push) |v| {
                const line: u32 = if (of.lineAt(x.addr)) |l| l.line else 0;
                if (line != 0 and v == line and ty.push != null) {
                    lines += 1;
                    continue;
                }
            }
            if (same_m and x.tokens.len == y.tokens.len) {
                var ok = true;
                for (x.tokens, y.tokens) |p, q| {
                    if (tokEq(p, q)) continue;
                    if (p.kind == .str and std.mem.indexOf(u8, p.s, OWN_TREE) != null and q.kind == .str) continue;
                    ok = false;
                }
                if (ok) {
                    files += 1;
                    continue;
                }
            }
            return null;
        }
        return if (lines > 0 and files > 0) .{ .lines = lines, .files = files } else null;
    }

    fn byLocated(m: *Matcher) !usize {
        try m.codeKeys();
        if (m.located == null) {
            m.located = .{ .groups = .empty, .otext = .empty, .rtext = .empty };
            const L = &m.located.?;
            var lengths: std.AutoHashMapUnmanaged(usize, void) = .empty;
            for (m.fns, 0..) |f, i| {
                if (m.insns(i) < EXACT_MIN or m.okeyOf(i) == null) continue;
                var own = false;
                for (f.f.refs) |r| if (r.kind == .string and std.mem.indexOf(u8, r.s, OWN_TREE) != null) {
                    own = true;
                };
                if (!own) continue;
                const t = try m.ourTexts(i);
                try L.otext.put(gpa, m.kid[i], t);
                const ck = try coarseKey(try m.ourView(i), t);
                const g = try L.groups.getOrPut(gpa, ck);
                if (!g.found_existing) g.value_ptr.* = .{};
                try g.value_ptr.ours.append(gpa, m.kid[i]);
                try lengths.put(gpa, m.insns(i), {});
            }
            for (m.starts) |s| {
                if (m.rkey.get(s).? == null) continue;
                const n = (try m.refView(s)).len;
                if (!lengths.contains(n) or n < EXACT_MIN) continue;
                const t = try m.refTexts(s);
                const ck = try coarseKey(try m.refView(s), t);
                if (L.groups.getPtr(ck)) |g| {
                    try g.theirs.append(gpa, s);
                    try L.rtext.put(gpa, s, t);
                }
            }
        }
        const L = &m.located.?;
        var pairs: std.ArrayList(Pair) = .empty;
        const Found = struct { k: u32, s: u32, l: Located };
        for (L.groups.values()) |g| {
            var found: std.ArrayList(Found) = .empty;
            for (g.ours.items) |k| {
                if (m.map.contains(k)) continue;
                for (g.theirs.items) |s| {
                    if (m.rmap.contains(s)) continue;
                    if (try m.locatedEqual(k, s, L.otext.get(k).?, L.rtext.get(s).?)) |l| try found.append(gpa, .{ .k = k, .s = s, .l = l });
                }
            }
            for (found.items) |x| {
                var nk: usize = 0;
                var ns: usize = 0;
                for (found.items) |y| {
                    if (y.k == x.k) nk += 1;
                    if (y.s == x.s) ns += 1;
                }
                if (nk == 1 and ns == 1) try pairs.append(gpa, .{ .sc = 5.0, .k = x.k, .s = x.s, .d = try fmt("identical code but for {d} source lines and {d} file names, {d} insns", .{ x.l.lines, x.l.files, m.insns(x.k) }) });
            }
        }
        return m.assign(pairs.items, "located");
    }

    fn byStrings(m: *Matcher) !usize {
        var pairs: std.ArrayList(Pair) = .empty;
        for (m.fns, 0..) |f, i| {
            var rare: std.StringArrayHashMapUnmanaged(void) = .empty;
            for (f.f.refs) |r| if (r.kind == .string) {
                const idx = m.str_index.get(r.s);
                const n = if (idx) |x| x.count() else 0;
                if (r.s.len >= 4 and n > 0 and n <= MAX_STRING_FANOUT) try rare.put(gpa, r.s, {});
            };
            if (rare.count() == 0) continue;
            var votes: std.AutoArrayHashMapUnmanaged(u32, usize) = .empty;
            for (rare.keys()) |s| for (m.str_index.get(s).?.keys()) |a| {
                const g = try votes.getOrPut(gpa, a);
                if (!g.found_existing) g.value_ptr.* = 0;
                g.value_ptr.* += 1;
            };
            var best: u32 = 0;
            var n: usize = 0;
            var tie = false;
            for (votes.keys(), votes.values()) |a, v| {
                if (v > n) {
                    best = a;
                    n = v;
                    tie = false;
                } else if (v == n) tie = true;
            }
            if (n * 2 < rare.count() or tie) continue;
            try pairs.append(gpa, .{ .sc = 1.0 + @as(f64, @floatFromInt(n)) / 100.0, .k = m.kid[i], .s = best, .d = try fmt("{d}/{d} strings", .{ n, rare.count() }) });
        }
        return m.assign(pairs.items, "string");
    }

    fn joinNames(names: []const []const u8) ![]const u8 {
        var out: std.ArrayList(u8) = .empty;
        for (names) |n| try out.print(gpa, "{d}:{s}", .{ n.len, n });
        return out.items;
    }

    fn byImports(m: *Matcher) !usize {
        var index: std.StringHashMapUnmanaged(std.ArrayList(u32)) = .empty;
        for (m.r_imports.keys(), m.r_imports.values()) |s, imps| {
            if (imps.len < 2) continue;
            const g = try index.getOrPut(gpa, try joinNames(imps));
            if (!g.found_existing) g.value_ptr.* = .empty;
            try g.value_ptr.append(gpa, s);
        }
        var pairs: std.ArrayList(Pair) = .empty;
        for (m.fns, 0..) |f, i| {
            var imps: std.ArrayList([]const u8) = .empty;
            for (f.f.refs) |r| if (r.kind == .import) try imps.append(gpa, r.s);
            if (imps.items.len < 2) continue;
            const cands = index.get(try joinNames(imps.items)) orelse continue;
            if (cands.items.len == 1) try pairs.append(gpa, .{ .sc = 1.0, .k = m.kid[i], .s = cands.items[0], .d = try fmt("{d} imports", .{imps.items.len}) });
        }
        return m.assign(pairs.items, "import");
    }

    fn refTokens(m: *Matcher, s: u32) !std.StringArrayHashMapUnmanaged(usize) {
        var out: std.StringArrayHashMapUnmanaged(usize) = .empty;
        for (m.model.getPtr(s).?.refs) |r| {
            const t: []const u8 = switch (r.kind) {
                .call => if (m.model.contains(r.int)) try tokFn(r.int) else continue,
                .import => try tok('i', afterBang(r.bytes)),
                .string => try tok('s', r.bytes),
                else => continue,
            };
            const g = try out.getOrPut(gpa, t);
            if (!g.found_existing) g.value_ptr.* = 0;
            g.value_ptr.* += 1;
        }
        return out;
    }

    fn ourTokens(m: *Matcher, i: usize) !std.StringArrayHashMapUnmanaged(usize) {
        var out: std.StringArrayHashMapUnmanaged(usize) = .empty;
        for (m.fns[i].f.refs) |r| {
            const t: []const u8 = switch (r.kind) {
                .import => try tok('i', r.s),
                .string => try tok('s', r.s),
                .call => blk: {
                    const g = m.resolveCallee(i, r.s);
                    if (g) |gi| {
                        if (m.map.get(m.kid[gi])) |e| break :blk try tokFn(e.s);
                        continue;
                    }
                    if (m.crt.get(try m.qualified(r.s))) |cs| break :blk try tokFn(cs);
                    continue;
                },
                else => continue,
            };
            const g = try out.getOrPut(gpa, t);
            if (!g.found_existing) g.value_ptr.* = 0;
            g.value_ptr.* += 1;
        }
        return out;
    }

    fn byRefs(m: *Matcher) !usize {
        var index: std.StringHashMapUnmanaged(std.ArrayList(u32)) = .empty;
        var rtok: std.AutoHashMapUnmanaged(u32, std.StringArrayHashMapUnmanaged(usize)) = .empty;
        for (m.starts) |s| {
            if (m.rmap.contains(s)) continue;
            const t = try m.refTokens(s);
            try rtok.put(gpa, s, t);
            for (t.keys()) |k| {
                const g = try index.getOrPut(gpa, k);
                if (!g.found_existing) g.value_ptr.* = .empty;
                try g.value_ptr.append(gpa, s);
            }
        }
        var df: std.StringHashMapUnmanaged(usize) = .empty;
        for (m.starts) |s| {
            const t = rtok.get(s) orelse try m.refTokens(s);
            for (t.keys()) |k| {
                const g = try df.getOrPut(gpa, k);
                if (!g.found_existing) g.value_ptr.* = 0;
                g.value_ptr.* += 1;
            }
        }
        const n_all: f64 = @floatFromInt(m.starts.len);
        var pairs: std.ArrayList(Pair) = .empty;
        for (m.fns, 0..) |_, i| {
            if (m.mapped(i)) continue;
            const ot = try m.ourTokens(i);
            var total_n: usize = 0;
            for (ot.values()) |v| total_n += v;
            if (total_n < 2) continue;
            var scores: std.AutoArrayHashMapUnmanaged(u32, f64) = .empty;
            for (ot.keys(), ot.values()) |t, cnt| {
                const d = df.get(t) orelse 0;
                if (d > 50) continue;
                const w = log(n_all / @as(f64, @floatFromInt(1 + d)));
                if (index.get(t)) |l| for (l.items) |s| {
                    const g = try scores.getOrPut(gpa, s);
                    if (!g.found_existing) g.value_ptr.* = 0;
                    g.value_ptr.* += w * @as(f64, @floatFromInt(@min(cnt, rtok.get(s).?.get(t) orelse 0)));
                };
            }
            if (scores.count() == 0) continue;
            // most_common(2): the first highest, then the next highest after it in a stable sort.
            var bi: usize = 0;
            for (scores.values(), 0..) |v, k| if (v > scores.values()[bi]) {
                bi = k;
            };
            var second: f64 = 0.0;
            var have2 = false;
            for (scores.values(), 0..) |v, k| if (k != bi and (!have2 or v > second)) {
                second = v;
                have2 = true;
            };
            const best = scores.keys()[bi];
            const sc = scores.values()[bi];
            var total = FSum{};
            for (ot.keys(), ot.values()) |t, cnt| {
                const d = df.get(t) orelse 0;
                if (d <= 50) total.add(log(n_all / @as(f64, @floatFromInt(1 + d))) * @as(f64, @floatFromInt(cnt)));
            }
            if (sc >= 2 * @max(second, 1e-9) and sc >= 0.6 * total.result() and sc >= 8.0) {
                var d: std.ArrayList(u8) = .empty;
                try d.appendSlice(gpa, "refs ");
                try util.fixed(&d, "%.1f", sc);
                try d.appendSlice(gpa, " (next ");
                try util.fixed(&d, "%.1f", second);
                try d.append(gpa, ')');
                try pairs.append(gpa, .{ .sc = sc / 100.0, .k = m.kid[i], .s = best, .d = d.items });
            }
        }
        return m.assign(pairs.items, "refs");
    }

    /// our_seq: f's calls in code order.
    fn ourSeq(m: *Matcher, i: usize) ![]SeqItem {
        var out: std.ArrayList(SeqItem) = .empty;
        for (m.fns[i].f.refs) |r| switch (r.kind) {
            .import => try out.append(gpa, .{ .imp = r.s }),
            .call => {
                const g = m.resolveCallee(i, r.s);
                if (g) |gi| {
                    if (m.map.get(m.kid[gi])) |e| try out.append(gpa, .{ .fn_ = e.s }) else try out.append(gpa, .{ .our_fn = gi });
                } else if (m.crt.get(try m.qualified(r.s))) |cs| {
                    try out.append(gpa, .{ .fn_ = cs });
                } else try out.append(gpa, .{ .our_sym = r.s });
            },
            else => {},
        };
        return out.items;
    }

    fn byCalls(m: *Matcher) !usize {
        var pairs: std.ArrayList(Pair) = .empty;
        var ids: std.StringHashMapUnmanaged(u32) = .empty;
        var fresh: u32 = 0x80000000;
        const snapshot_k = try gpa.dupe(u32, m.map.keys());
        const snapshot_v = try gpa.dupe(Entry, m.map.values());
        for (snapshot_k, snapshot_v) |k, e| {
            const s = e.s;
            const a = try m.ourSeq(k);
            const b = m.r_seq.get(s) orelse &.{};
            if (a.len == 0 or b.len == 0) continue;
            const ka = try gpa.alloc(u32, a.len);
            for (a, 0..) |x, n| ka[n] = switch (x) {
                .imp => |nm| try internId(&ids, 'i', nm),
                .fn_ => |t| try internFn(&ids, t),
                else => blk: {
                    fresh += 1;
                    break :blk fresh;
                },
            };
            const kb = try gpa.alloc(u32, b.len);
            for (b, 0..) |y, n| kb[n] = switch (y) {
                .imp => |nm| try internId(&ids, 'i', nm),
                .fn_ => |t| if (!m.rmap.contains(t) and !m.crt_starts.contains(t)) blk: {
                    fresh += 1;
                    break :blk fresh;
                } else try internFn(&ids, t),
                else => unreachable,
            };
            var sm = try difflib.SequenceMatcher.init(gpa, ka, kb);
            for (try sm.opcodes()) |op| {
                if (op.tag != .replace) continue;
                var xa: ?SeqItem = null;
                var nx: usize = 0;
                for (a[op.ci..op.ci2]) |x| if (!(x == .fn_ and x.fn_ == COOKIE_CHECK)) {
                    nx += 1;
                    xa = x;
                };
                var yb: ?SeqItem = null;
                var ny: usize = 0;
                for (b[op.cj..op.cj2]) |y| if (!(y == .fn_ and y.fn_ == COOKIE_CHECK)) {
                    ny += 1;
                    yb = y;
                };
                if (nx != 1 or ny != 1) continue;
                if (xa.? != .our_fn or yb.? != .fn_) continue;
                const t = yb.?.fn_;
                if (m.rmap.contains(t) or m.crt_starts.contains(t) or !m.model.contains(t)) continue;
                try pairs.append(gpa, .{ .sc = 1.0, .k = m.kid[xa.?.our_fn], .s = t, .d = try fmt("called from {s}", .{m.fns[k].f.name}) });
            }
        }
        // A callee proposed at two different addresses by two callers: drop it.
        var seen: std.AutoHashMapUnmanaged(u32, std.AutoHashMapUnmanaged(u32, void)) = .empty;
        for (pairs.items) |p| {
            const g = try seen.getOrPut(gpa, p.k);
            if (!g.found_existing) g.value_ptr.* = .empty;
            try g.value_ptr.put(gpa, p.s, {});
        }
        var kept: std.ArrayList(Pair) = .empty;
        for (pairs.items) |p| if (seen.get(p.k).?.count() == 1) try kept.append(gpa, p);
        return m.assign(kept.items, "calls");
    }

    fn internId(ids: *std.StringHashMapUnmanaged(u32), kind: u8, s: []const u8) !u32 {
        const key = try tok(kind, s);
        const g = try ids.getOrPut(gpa, key);
        if (!g.found_existing) g.value_ptr.* = ids.count() - 1;
        return g.value_ptr.*;
    }

    fn internFn(ids: *std.StringHashMapUnmanaged(u32), t: u32) !u32 {
        var b: [4]u8 = undefined;
        std.mem.writeInt(u32, &b, t, .little);
        return internId(ids, 'f', &b);
    }

    fn perTu(m: *Matcher) ![][]u32 {
        if (m.per_tu) |p| return p;
        var by: std.StringArrayHashMapUnmanaged(std.ArrayList(u32)) = .empty;
        for (m.fns, 0..) |f, i| {
            const g = try by.getOrPut(gpa, f.tu);
            if (!g.found_existing) g.value_ptr.* = .empty;
            try g.value_ptr.append(gpa, @intCast(i));
        }
        const out = try gpa.alloc([]u32, by.count());
        for (by.values(), 0..) |l, n| {
            std.mem.sort(u32, l.items, m, struct {
                fn lt(mm: *Matcher, a: u32, b: u32) bool {
                    return ours.LayoutKey.lessThan(mm.layout[a], mm.layout[b]);
                }
            }.lt);
            out[n] = l.items;
        }
        m.per_tu = out;
        return out;
    }

    fn ourCallees(m: *Matcher, i: usize) !?[]u32 {
        var out: std.ArrayList(u32) = .empty;
        for (m.fns[i].f.refs) |r| {
            if (r.kind != .call and r.kind != .fptr) continue;
            const s = (try m.calleeStart(i, r.s)) orelse return null;
            try out.append(gpa, s);
        }
        return out.items;
    }

    fn refCallees(m: *Matcher, s: u32) ![]u32 {
        var out: std.ArrayList(u32) = .empty;
        for (m.sorted_refs.get(s).?) |r| if (r.kind == .call or r.kind == .fptr) try out.append(gpa, r.int);
        return out.items;
    }

    fn calleesKey(base: []const u8, callees: ?[]const u32) ![]const u8 {
        var out: std.ArrayList(u8) = .empty;
        try out.appendSlice(gpa, base);
        if (callees) |cl| {
            try out.print(gpa, "#T{d}", .{cl.len});
            for (cl) |x| try out.print(gpa, ",{x}", .{x});
        } else try out.appendSlice(gpa, "#N");
        return out.items;
    }

    const GapPair = struct { f: u32, r: u32 };

    /// Pairs whose keys are equal and unique within the gap on both sides, in gap order.
    fn identical(ok: []const ?[]const u8, rk: []const ?[]const u8, ours_gap: []const u32, ref_gap: []const u32) ![]GapPair {
        var ko: std.StringHashMapUnmanaged(usize) = .empty;
        for (ok) |k| if (k) |kk| {
            const g = try ko.getOrPut(gpa, kk);
            if (!g.found_existing) g.value_ptr.* = 0;
            g.value_ptr.* += 1;
        };
        var kr: std.StringHashMapUnmanaged(usize) = .empty;
        for (rk) |k| if (k) |kk| {
            const g = try kr.getOrPut(gpa, kk);
            if (!g.found_existing) g.value_ptr.* = 0;
            g.value_ptr.* += 1;
        };
        var unique_r: std.StringHashMapUnmanaged(u32) = .empty;
        for (rk, ref_gap) |k, r| if (k) |kk| if (kr.get(kk).? == 1) try unique_r.put(gpa, kk, r);
        var out: std.ArrayList(GapPair) = .empty;
        for (ok, ours_gap) |k, f| if (k) |kk| if (ko.get(kk).? == 1) if (unique_r.get(kk)) |r| try out.append(gpa, .{ .f = f, .r = r });
        return out.items;
    }

    fn byBrackets(m: *Matcher, alignment: bool, exact_only: bool) !usize {
        try m.codeKeys();
        var pairs: std.ArrayList(Pair) = .empty;
        var exact: std.ArrayList(Pair) = .empty;
        for (try m.perTu()) |lst| {
            const Anchor = struct { i: usize, s: u32 };
            var anchored: std.ArrayList(Anchor) = .empty;
            for (lst, 0..) |f, i| if (m.map.get(m.kid[f])) |e| try anchored.append(gpa, .{ .i = i, .s = e.s });
            const chain = try longestIncreasing(Anchor, anchored.items);
            if (chain.len < 2) continue;
            for (chain[0 .. chain.len - 1], chain[1..]) |p1, p2| {
                const ia = p1.i;
                const s1 = p1.s;
                const ib = p2.i;
                const s2 = p2.s;
                var ours_gap: std.ArrayList(u32) = .empty;
                for (lst[ia + 1 .. ib]) |f| if (!m.mapped(f)) try ours_gap.append(gpa, f);
                if (ours_gap.items.len != ib - ia - 1) continue;
                const lo = std.sort.upperBound(u32, m.starts, s1, orderU32);
                const hi = std.sort.lowerBound(u32, m.starts, s2, orderU32);
                var ref_gap: std.ArrayList(u32) = .empty;
                var foreign = false;
                if (lo < hi) for (m.starts[lo..hi]) |r| {
                    if (m.rmap.contains(r)) foreign = true else try ref_gap.append(gpa, r);
                };
                if (ours_gap.items.len == 0 or ref_gap.items.len == 0) continue;
                const og = ours_gap.items;
                const rg = ref_gap.items;
                var ok = try gpa.alloc(?[]const u8, og.len);
                var rk = try gpa.alloc(?[]const u8, rg.len);
                for (og, 0..) |f, n| ok[n] = m.ofreeOf(f);
                for (rg, 0..) |r, n| rk[n] = m.rfree.get(r).?;
                var same = try identical(ok, rk, og, rg);
                const with_callees = same.len == 0;
                if (same.len == 0) {
                    ok = try gpa.alloc(?[]const u8, og.len);
                    rk = try gpa.alloc(?[]const u8, rg.len);
                    for (og, 0..) |f, n| ok[n] = if (m.okeyOf(f)) |k| try calleesKey(k, try m.ourCallees(f)) else null;
                    for (rg, 0..) |r, n| rk[n] = if (m.rkey.get(r).?) |k| try calleesKey(k, try m.refCallees(r)) else null;
                    same = try identical(ok, rk, og, rg);
                }
                if (foreign and rg.len > FOREIGN_GAP) {
                    var kept: std.ArrayList(GapPair) = .empty;
                    for (same) |p| if (try m.uniqueOverall(p.f, p.r, with_callees)) try kept.append(gpa, p);
                    same = kept.items;
                }
                if (same.len > 0) {
                    for (same) |p| try exact.append(gpa, .{ .sc = 2.0, .k = m.kid[p.f], .s = p.r, .d = try fmt("identical code between {x:0>8} and {x:0>8}", .{ s1, s2 }) });
                    continue;
                }
                if (exact_only or foreign) continue;
                if (og.len == rg.len and !try m.shifted(og, rg)) {
                    for (og, rg) |f, r| try pairs.append(gpa, .{ .sc = 1.0, .k = m.kid[f], .s = r, .d = try fmt("between {x:0>8} and {x:0>8}", .{ s1, s2 }) });
                } else if (alignment) {
                    for (try m.alignGap(og, rg, ALIGN_THRESHOLD)) |a| {
                        var d: std.ArrayList(u8) = .empty;
                        try d.appendSlice(gpa, "aligned ");
                        try util.fixed(&d, "%.2f", a.sc);
                        try d.print(gpa, " between {x:0>8} and {x:0>8}", .{ s1, s2 });
                        try pairs.append(gpa, .{ .sc = a.sc, .k = m.kid[a.f], .s = a.r, .d = d.items });
                    }
                }
            }
        }
        const a = try m.assign(exact.items, "exact");
        return a + try m.assign(pairs.items, if (alignment) "align" else "bracket");
    }

    fn orderU32(key: u32, x: u32) std.math.Order {
        return std.math.order(key, x);
    }

    fn uniqueOverall(m: *Matcher, f: u32, r: u32, with_callees: bool) !bool {
        try m.codeKeys();
        if (m.by_code == null) {
            m.by_code = .empty;
            const B = &m.by_code.?;
            for (m.fns, 0..) |_, i| if (m.okeyOf(i)) |k| {
                const g = try B.getOrPut(gpa, k);
                if (!g.found_existing) g.value_ptr.* = .{};
                try g.value_ptr.ours.append(gpa, m.kid[i]);
            };
            for (m.starts) |s| if (m.rkey.get(s).?) |k| if (B.getPtr(k)) |g| try g.theirs.append(gpa, s);
        }
        const k = m.okeyOf(f) orelse return false;
        const g = m.by_code.?.get(k) orelse return false;
        var os: std.ArrayList(u32) = .empty;
        for (g.ours.items) |x| if (!m.map.contains(x)) try os.append(gpa, x);
        var ts: std.ArrayList(u32) = .empty;
        for (g.theirs.items) |s| if (!m.rmap.contains(s)) try ts.append(gpa, s);
        if (with_callees) {
            const callees = try m.ourCallees(f);
            var os2: std.ArrayList(u32) = .empty;
            for (os.items) |x| if (eqlOpt(try m.ourCallees(x), callees)) try os2.append(gpa, x);
            var ts2: std.ArrayList(u32) = .empty;
            for (ts.items) |s| if (eqlOpt(try m.refCallees(s), callees)) try ts2.append(gpa, s);
            os = os2;
            ts = ts2;
        }
        return os.items.len == 1 and os.items[0] == m.kid[f] and ts.items.len == 1 and ts.items[0] == r;
    }

    fn eqlOpt(a: ?[]const u32, b: ?[]const u32) bool {
        if (a == null or b == null) return a == null and b == null;
        return std.mem.eql(u32, a.?, b.?);
    }

    fn shifted(m: *Matcher, og: []const u32, rg: []const u32) !bool {
        if (og.len < 2) return false;
        var diagonal = FSum{};
        for (og, rg) |f, r| diagonal.add(try m.similarity(f, r));
        var best = FSum{};
        for (try m.alignGap(og, rg, 0.0)) |a| best.add(a.sc);
        return best.result() > diagonal.result() + BRACKET_MARGIN;
    }

    fn mnemonics(m: *Matcher, insns_: []const dec.Insn) ![]u32 {
        const n = @min(insns_.len, 600);
        const out = try gpa.alloc(u32, n);
        for (insns_[0..n], 0..) |i, k| out[k] = try m.mnem.id(i.mn);
        return out;
    }

    fn similarity(m: *Matcher, f: u32, s: u32) !f64 {
        const key = (@as(u64, m.kid[f]) << 32) | s;
        if (m.sim.get(key)) |v| return v;
        const a = try m.mnemonics(m.fns[f].f.decoded.insns.items);
        const b = try m.mnemonics(m.model.getPtr(s).?.f.insns.items);
        var v: f64 = 0.0;
        if (!(a.len == 0 or b.len == 0 or @min(a.len, b.len) * 2 < @max(a.len, b.len))) {
            var sm = try difflib.SequenceMatcher.init(gpa, a, b);
            v = try sm.ratio();
        }
        try m.sim.put(gpa, key, v);
        return v;
    }

    const Aligned = struct { f: u32, r: u32, sc: f64 };

    fn alignGap(m: *Matcher, og: []const u32, rg: []const u32, threshold: f64) ![]Aligned {
        const n = og.len;
        const mm = rg.len;
        if (n * mm > 4000) return &.{};
        const sim = try gpa.alloc(f64, n * mm);
        for (og, 0..) |f, i| for (rg, 0..) |r, j| {
            sim[i * mm + j] = try m.similarity(f, r);
        };
        const w = mm + 1;
        const best = try gpa.alloc(f64, (n + 1) * w);
        @memset(best, 0.0);
        var i: usize = n;
        while (i > 0) {
            i -= 1;
            var j: usize = mm;
            while (j > 0) {
                j -= 1;
                best[i * w + j] = @max(@max(best[(i + 1) * w + j], best[i * w + j + 1]), sim[i * mm + j] + best[(i + 1) * w + j + 1]);
            }
        }
        var out: std.ArrayList(Aligned) = .empty;
        i = 0;
        var j: usize = 0;
        while (i < n and j < mm) {
            const sv = sim[i * mm + j];
            if (sv + best[(i + 1) * w + j + 1] >= best[i * w + j] - 1e-9 and sv >= threshold) {
                try out.append(gpa, .{ .f = og[i], .r = rg[j], .sc = sv });
                i += 1;
                j += 1;
            } else if (best[(i + 1) * w + j] >= best[i * w + j + 1]) {
                i += 1;
            } else j += 1;
        }
        return out.items;
    }

    /// (our callee symbol, original target) of every call that lines up between f and s.
    fn alignedCalls(m: *Matcher, f: u32, s: u32) ![]Site {
        const ov = try m.ourView(f);
        const rv = try m.refView(s);
        const P = struct { x: *const NInsn, y: *const NInsn };
        var pairs: std.ArrayList(P) = .empty;
        var same = ov.len == rv.len;
        if (same) for (ov, rv) |x, y| if (x.mnemonic != y.mnemonic) {
            same = false;
        };
        if (same) {
            for (ov, rv) |*x, *y| try pairs.append(gpa, .{ .x = x, .y = y });
        } else {
            const a = try gpa.alloc(u32, ov.len);
            for (ov, 0..) |x, k| a[k] = x.mnemonic;
            const b = try gpa.alloc(u32, rv.len);
            for (rv, 0..) |y, k| b[k] = y.mnemonic;
            var sm = try difflib.SequenceMatcher.init(gpa, a, b);
            for (try sm.opcodes()) |op| if (op.tag == .equal) for (op.ci..op.ci2, op.cj..op.cj2) |p, q| try pairs.append(gpa, .{ .x = &ov[p], .y = &rv[q] });
        }
        var out: std.ArrayList(Site) = .empty;
        for (pairs.items) |p| {
            const n = @min(p.x.tokens.len, p.y.tokens.len);
            for (p.x.tokens[0..n], p.y.tokens[0..n]) |ta, tb| if (ta.kind == .fnq and tb.kind == .fn_) try out.append(gpa, .{ .sym = ta.s, .t = @intCast(tb.int) });
        }
        return out.items;
    }

    fn callVotes(m: *Matcher) !std.StringArrayHashMapUnmanaged(std.AutoArrayHashMapUnmanaged(u32, usize)) {
        var votes: std.StringArrayHashMapUnmanaged(std.AutoArrayHashMapUnmanaged(u32, usize)) = .empty;
        m.exact_votes = .empty;
        for (m.map.keys(), m.map.values()) |k, e| {
            for (try m.alignedCalls(k, e.s)) |site| {
                const g = try votes.getOrPut(gpa, site.sym);
                if (!g.found_existing) g.value_ptr.* = .empty;
                const c2 = try g.value_ptr.getOrPut(gpa, site.t);
                if (!c2.found_existing) c2.value_ptr.* = 0;
                c2.value_ptr.* += 1;
                if (std.mem.eql(u8, e.method, "exact")) {
                    const g2 = try m.exact_votes.getOrPut(gpa, site.sym);
                    if (!g2.found_existing) g2.value_ptr.* = .empty;
                    const c3 = try g2.value_ptr.getOrPut(gpa, site.t);
                    if (!c3.found_existing) c3.value_ptr.* = 0;
                    c3.value_ptr.* += 1;
                }
            }
        }
        return votes;
    }

    const Pin = struct { k: u32, t: u32, d: []const u8 };

    fn pinsFrom(m: *Matcher, votes: *const std.StringArrayHashMapUnmanaged(std.AutoArrayHashMapUnmanaged(u32, usize))) ![]Pin {
        var pins: std.ArrayList(Pin) = .empty;
        m.pin_votes = .empty;
        for (votes.keys(), votes.values()) |sym, cnt| {
            var t: u32 = 0;
            var n: usize = 0;
            var total: usize = 0;
            for (cnt.keys(), cnt.values()) |tt, v| {
                total += v;
                if (v > n) {
                    n = v;
                    t = tt;
                }
            }
            const ex = if (m.exact_votes.get(sym)) |e| e.get(t) orelse 0 else 0;
            const enough = n >= 2 or (ex >= 1 and n == total);
            if (enough and @as(f64, @floatFromInt(n)) >= 0.75 * @as(f64, @floatFromInt(total)) and m.model.contains(t)) {
                const g = m.by_symbol.get(sym) orelse continue;
                if (g.items.len == 1) {
                    const k = m.kid[g.items[0]];
                    try pins.append(gpa, .{ .k = k, .t = t, .d = try fmt("{d}/{d} call sites", .{ n, total }) });
                    try m.pin_votes.put(gpa, k, .{ n, total });
                }
            }
        }
        return pins.items;
    }

    fn overwhelming(m: *const Matcher, k: u32) bool {
        const v = m.pin_votes.get(k) orelse return false;
        return v[0] >= OVERWHELM_VOTES and @as(f64, @floatFromInt(v[0])) >= OVERWHELM_SHARE * @as(f64, @floatFromInt(v[1]));
    }

    fn byPins(m: *Matcher, pins: []const Pin) !usize {
        const pairs = try gpa.alloc(Pair, pins.len);
        for (pins, 0..) |p, i| pairs[i] = .{ .sc = 4.0, .k = p.k, .s = p.t, .d = p.d };
        return m.assign(pairs, "votes");
    }

    fn matches(m: *Matcher, f: u32, s: u32, table: SymTable) !bool {
        const ov = try m.viewer.ourView(&m.fns[f], table);
        const res = try compare.compare(ov, try m.refView(s));
        return std.mem.eql(u8, res.status, "MATCH");
    }

    fn keepMatches(m: *Matcher, pins: []const Pin) ![]Pin {
        var syms: std.StringHashMapUnmanaged(u32) = .empty;
        for (m.map.keys(), m.map.values()) |k, e| try syms.put(gpa, m.fns[k].f.symbol, e.s);
        const base = SymTable{ .base = &syms };
        const KS = struct { k: u32, s: u32 };
        var callers: std.StringHashMapUnmanaged(std.ArrayList(KS)) = .empty;
        for (m.map.keys(), m.map.values()) |k, e| for (m.fns[k].f.refs) |r| if (r.kind == .call or r.kind == .fptr) {
            const g = try callers.getOrPut(gpa, r.s);
            if (!g.found_existing) g.value_ptr.* = .empty;
            try g.value_ptr.append(gpa, .{ .k = k, .s = e.s });
        };
        var memo: std.AutoHashMapUnmanaged(u64, bool) = .empty;
        var out: std.ArrayList(Pin) = .empty;
        for (pins) |p| {
            const s0 = if (m.map.get(p.k)) |e| e.s else null;
            if (s0 != null and s0.? != p.t and !m.overwhelming(p.k)) {
                const sym = m.fns[p.k].f.symbol;
                if (try m.matchesMemo(&memo, p.k, s0.?, base) and !try m.matchesMemo(&memo, p.k, p.t, base)) continue;
                const moved = SymTable{ .base = &syms, .over_sym = sym, .over_val = p.t };
                var broke = false;
                if (callers.get(sym)) |cl| for (cl.items) |g| {
                    if (try m.matchesMemo(&memo, g.k, g.s, base) and !try m.matches(g.k, g.s, moved)) {
                        broke = true;
                        break;
                    }
                };
                if (broke) continue;
            }
            try out.append(gpa, p);
        }
        return out.items;
    }

    fn matchesMemo(m: *Matcher, memo: *std.AutoHashMapUnmanaged(u64, bool), f: u32, s: u32, table: SymTable) !bool {
        const key = (@as(u64, f) << 32) | s;
        if (memo.get(key)) |v| return v;
        const v = try m.matches(f, s, table);
        try memo.put(gpa, key, v);
        return v;
    }

    fn dropContradicted(m: *Matcher) !usize {
        try m.codeKeys();
        var bad: std.ArrayList(u32) = .empty;
        for (m.map.keys(), m.map.values()) |k, e| {
            const meth = e.method;
            if (!(std.mem.eql(u8, meth, "bracket") or std.mem.eql(u8, meth, "align") or std.mem.eql(u8, meth, "exact") or std.mem.eql(u8, meth, "located"))) continue;
            if (std.mem.eql(u8, meth, "exact") and m.ofree[k] != null) continue;
            const o = (try m.ourCallees(k)) orelse continue;
            const t = try m.refCallees(e.s);
            if (o.len != t.len) continue;
            const a = try gpa.dupe(u32, o);
            const b = try gpa.dupe(u32, t);
            std.mem.sort(u32, a, {}, std.sort.asc(u32));
            std.mem.sort(u32, b, {}, std.sort.asc(u32));
            if (!std.mem.eql(u32, a, b)) try bad.append(gpa, k);
        }
        for (bad.items) |k| {
            const e = m.map.get(k).?;
            _ = m.map.orderedRemove(k);
            _ = m.rmap.remove(e.s);
        }
        return bad.items.len;
    }

    const Log = struct { tier: []const u8, n: i64 };

    fn derive(m: *Matcher, pins: []const Pin, lg: *std.ArrayList(Log)) !i64 {
        var total: i64 = 0;
        const L = struct {
            fn add(l: *std.ArrayList(Log), t: *i64, tier: []const u8, n: usize) !void {
                try l.append(gpa, .{ .tier = tier, .n = @intCast(n) });
                t.* += @intCast(n);
            }
        };
        try L.add(lg, &total, "sourcemap", try m.bySourcemaps());
        try L.add(lg, &total, "name", try m.byNames());
        try L.add(lg, &total, "export", try m.byExports());
        var strong: std.ArrayList(Pin) = .empty;
        for (pins) |p| if (m.overwhelming(p.k)) try strong.append(gpa, p);
        if (strong.items.len > 0) try L.add(lg, &total, "votes", try m.byPins(strong.items));
        try L.add(lg, &total, "exact", try m.byExact());
        try L.add(lg, &total, "located", try m.byLocated());
        if (pins.len > 0) try L.add(lg, &total, "votes", try m.byPins(pins));
        try L.add(lg, &total, "string", try m.byStrings());
        try L.add(lg, &total, "import", try m.byImports());
        while (true) {
            const e = try m.byBrackets(false, true);
            const a = try m.byCalls();
            const b = try m.byBrackets(false, false);
            try L.add(lg, &total, "calls+bracket", e + a + b);
            if (e + a + b == 0) break;
        }
        while (true) {
            const r = try m.byRefs();
            const e = try m.byBrackets(false, true);
            const a = try m.byCalls();
            const b = try m.byBrackets(false, false);
            try L.add(lg, &total, "refs+calls+bracket", r + e + a + b);
            if (r + e + a + b == 0) break;
        }
        while (true) {
            const e = try m.byBrackets(false, true);
            const cc = try m.byBrackets(true, false);
            const r = try m.byRefs();
            const a = try m.byCalls();
            const b = try m.byBrackets(false, false);
            try L.add(lg, &total, "align+refs+calls+bracket", e + a + b + cc + r);
            if (e + a + b + cc + r == 0) break;
        }
        return total;
    }

    pub fn run(m: *Matcher) ![]Log {
        var first: std.ArrayList(Log) = .empty;
        const n_first = try m.derive(&.{}, &first);
        var votes = try m.callVotes();
        const pins = try m.keepMatches(try m.pinsFrom(&votes));
        var moved: usize = 0;
        for (pins) |p| if (m.map.get(p.k)) |e| {
            if (e.s != p.t) moved += 1;
        } else {
            moved += 1;
        };
        m.map = .empty;
        m.rmap = .empty;
        var lg: std.ArrayList(Log) = .empty;
        try lg.append(gpa, .{ .tier = "first pass", .n = n_first });
        try lg.append(gpa, .{ .tier = "pinned by votes", .n = @intCast(pins.len) });
        try lg.append(gpa, .{ .tier = "of which moved", .n = @intCast(moved) });
        _ = try m.derive(pins, &lg);
        try lg.append(gpa, .{ .tier = "dropped: callees disagree", .n = -@as(i64, @intCast(try m.dropContradicted())) });
        try lg.append(gpa, .{ .tier = "sourcemap, the rest", .n = @intCast(try m.bySourcemapRest()) });
        return lg.items;
    }
};

/// Longest subsequence of (index, start) pairs whose starts increase (funcmap.longest_increasing).
pub fn longestIncreasing(comptime T: type, pairs: []const T) ![]T {
    if (pairs.len == 0) return &.{};
    const prev = try gpa.alloc(?usize, pairs.len);
    var idx: std.ArrayList(usize) = .empty;
    for (pairs, 0..) |p, k| {
        // bisect_left over the starts at idx
        var lo: usize = 0;
        var hi: usize = idx.items.len;
        while (lo < hi) {
            const mid = (lo + hi) / 2;
            if (pairs[idx.items[mid]].s < p.s) lo = mid + 1 else hi = mid;
        }
        const pos = lo;
        if (pos == idx.items.len) try idx.append(gpa, k) else idx.items[pos] = k;
        prev[k] = if (pos > 0) idx.items[pos - 1] else null;
    }
    var out: std.ArrayList(T) = .empty;
    var k: ?usize = idx.items[idx.items.len - 1];
    while (k) |kk| {
        try out.append(gpa, pairs[kk]);
        k = prev[kk];
    }
    std.mem.reverse(T, out.items);
    return out.items;
}

/// Unambiguous CRT names from crtmap.tsv -> original start (funcmap.load_crtmap).
fn loadCrtmap(path: []const u8) !std.StringHashMapUnmanaged(u32) {
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
            std.debug.print("usage: oracle-zig map --root <asm_matching/msvc8> [--out F]\n", .{});
            std.process.exit(2);
        }
    }
    const cfg = try audit.Config.init(root orelse {
        std.debug.print("oracle-zig map: --root is required\n", .{});
        std.process.exit(2);
    });
    const out_file = out_path orelse try cfg.evidence("funcmap.tsv");
    if (cfg.experiment and std.mem.eql(u8, try realpath(util.dirname(out_file)), try realpath(try util.join(cfg.oracle, "evidence")))) {
        std.debug.print("funcmap: not in an experiment - an A/B audit uses the baseline map (evidence/funcmap.tsv)\n", .{});
        std.process.exit(1);
    }
    // Everything the map is derived from: the original and its carve, our objects, the CRT
    // map, the generator's source maps, the module link order (and this executable).
    const objdir = try util.join(cfg.build, "obj");
    var key = try stage.Key.init("funcmap");
    try key.file(try util.join(cfg.oracle, "reference/Game.exe"));
    try key.file(try util.join(cfg.refcache, "functions.tsv"));
    for (try ours.completedObjects(objdir)) |rel| try key.file(try util.join(objdir, rel));
    try key.file(try cfg.evidence("crtmap.tsv"));
    {
        var maps: std.StringHashMapUnmanaged([]const u8) = .empty;
        try Matcher.walkMaps(cfg.repo, "", &maps);
        const paths = try gpa.alloc([]const u8, maps.count());
        var mi = maps.valueIterator();
        var n: usize = 0;
        while (mi.next()) |v| : (n += 1) paths[n] = v.*;
        std.mem.sort([]const u8, paths, {}, lessStr);
        for (paths) |p| try key.file(p);
    }
    try key.file(try util.join(cfg.repo, "Source/modules.cmake"));
    const key_hex = key.hex();
    const outputs = [_][]const u8{ out_file, try fmt("{s}.unmapped", .{out_file}) };
    if (try stage.reuse(cfg.build, "funcmap", &key_hex, &outputs)) {
        util.say(try fmt("funcmap: inputs unchanged, {s} kept\n", .{out_file}));
        return;
    }
    const ref = try refimage.Reference.open(try util.join(cfg.oracle, "reference/Game.exe"));
    try ref.load(try util.join(cfg.refcache, "functions.tsv"));
    var model = try ref.model();
    var d = try dec.Decoder.init(gpa);
    const all = try ours.loadAll(&d, objdir);
    const fns = try ours.linkKept(all, cfg.repo);
    var m = Matcher{ .ref = ref, .model = &model, .starts = ref.starts, .fns = fns, .viewer = .{ .d = &d }, .crt = try loadCrtmap(try cfg.evidence("crtmap.tsv")), .root = cfg.repo };
    try m.init();
    var say: std.ArrayList(u8) = .empty;
    for (try m.run()) |l| {
        try say.print(gpa, "  {s}", .{l.tier});
        if (l.tier.len < 22) try say.appendNTimes(gpa, ' ', 22 - l.tier.len);
        try say.print(gpa, " +{d}\n", .{l.n});
    }
    util.say(say.items);
    util.makeDirs(util.dirname(out_file));
    const Row = struct { s: u32, i: u32, e: Entry };
    var rows: std.ArrayList(Row) = .empty;
    for (m.map.keys(), m.map.values()) |kk, e| try rows.append(gpa, .{ .s = e.s, .i = kk, .e = e });
    std.mem.sort(Row, rows.items, {}, struct {
        fn lt(_: void, a: Row, b: Row) bool {
            return a.s < b.s;
        }
    }.lt);
    var o: std.ArrayList(u8) = .empty;
    try o.appendSlice(gpa, "start\tend\tname\ttu\tsymbol\tmethod\tdetail\n");
    for (rows.items) |r| {
        const f = &fns[r.i];
        try o.print(gpa, "{x:0>8}\t{x:0>8}\t{s}\t{s}\t{s}\t{s}\t{s}\n", .{ r.s, model.getPtr(r.s).?.f.end, f.f.name, f.tu, f.f.symbol, r.e.method, r.e.detail });
    }
    try util.writeFile(out_file, o.items);
    const order = try gpa.alloc(u32, fns.len);
    for (order, 0..) |*x, i| x.* = @intCast(i);
    std.mem.sort(u32, order, fns, struct {
        fn lt(fs: []ours.OurFunc, a: u32, b: u32) bool {
            const t = std.mem.order(u8, fs[a].tu, fs[b].tu);
            if (t != .eq) return t == .lt;
            return fs[a].order < fs[b].order;
        }
    }.lt);
    var u: std.ArrayList(u8) = .empty;
    for (order) |i| if (!m.map.contains(m.kid[i])) try u.print(gpa, "{s}\t{s}\t{d} insns\n", .{ fns[i].tu, fns[i].f.name, fns[i].f.decoded.insns.items.len });
    try util.writeFile(outputs[1], u.items);
    try stage.done(cfg.build, "funcmap", &key_hex, &outputs);
    // mapped N of M ...: methods by count, ties in first-seen order
    var by: std.StringArrayHashMapUnmanaged(usize) = .empty;
    for (rows.items) |r| {
        const g = try by.getOrPut(gpa, r.e.method);
        if (!g.found_existing) g.value_ptr.* = 0;
        g.value_ptr.* += 1;
    }
    const Item = struct { k: []const u8, n: usize };
    const items = try gpa.alloc(Item, by.count());
    for (by.keys(), by.values(), 0..) |kk, n, i| items[i] = .{ .k = kk, .n = n };
    std.mem.sort(Item, items, {}, struct {
        fn lt(_: void, a: Item, b: Item) bool {
            return a.n > b.n;
        }
    }.lt);
    var sm: std.ArrayList(u8) = .empty;
    try sm.print(gpa, "mapped {d} of {d} compiled functions ({d} original functions): ", .{ rows.items.len, fns.len, model.count() });
    for (items, 0..) |it, i| try sm.print(gpa, "{s}{s} {d}", .{ if (i > 0) ", " else "", it.k, it.n });
    try sm.append(gpa, '\n');
    util.say(sm.items);
}

fn realpath(p: []const u8) ![]const u8 {
    var buf: [4096]u8 = undefined;
    const pz = try util.z(p);
    const r = c.realpath(pz.ptr, &buf) orelse return p;
    return gpa.dupe(u8, std.mem.sliceTo(r, 0));
}
