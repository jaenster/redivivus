//! The statically linked runtime's side (the parts of libmap.py the audit reads): every
//! function of the VC8 runtime libraries, and the import-thunk test.
const std = @import("std");
const builtin = @import("builtin");
const coff = @import("coff.zig");
const objfuncs = @import("objfuncs.zig");
const ours = @import("ours.zig");
const dec = @import("decode.zig");
const refimage = @import("refimage.zig");
const util = @import("util.zig");
const proc = @import("proc.zig");

const gpa = util.gpa;

pub const LIBS = [_][]const u8{ "libcmt.lib", "libcpmt.lib" };
pub const LIBVC = "sp1"; // the runtime the original links: evidence/toolchain.md

/// The runtime libraries' directory, from the environment as libmap.lib_dir has it.
pub fn libDir() ![]const u8 {
    if (builtin.os.tag == .windows) {
        const root = util.getenv("VC8_NATIVE_ROOT") orelse {
            std.debug.print("Windows libmap requires VC8_NATIVE_ROOT (the tree containing sp1/lib and rtm/lib)\n", .{});
            return error.NativeRuntimeRootRequired;
        };
        if (root.len == 0) {
            std.debug.print("VC8_NATIVE_ROOT must not be empty\n", .{});
            return error.NativeRuntimeRootRequired;
        }
        return std.fmt.allocPrint(gpa, "{s}/{s}/lib", .{ root, LIBVC });
    }
    const prefix = util.getenv("VC8_WINEPREFIX") orelse blk: {
        const tc = util.getenv("VC8_TOOLCHAIN") orelse try util.join(util.getenv("HOME") orelse "", "q2-dev/asm-matching-toolchain");
        break :blk try util.join(tc, "wine-vc8");
    };
    return std.fmt.allocPrint(gpa, "{s}/drive_c/vc8/{s}/lib", .{ prefix, LIBVC });
}

fn lower(s: []const u8) ![]u8 {
    const o = try gpa.dupe(u8, s);
    for (o) |*ch| ch.* = std.ascii.toLower(ch.*);
    return o;
}

fn endsWithObjCi(s: []const u8) bool {
    return s.len >= 4 and std.ascii.eqlIgnoreCase(s[s.len - 4 ..], ".obj");
}

/// Archive member path -> short id: the base name without .obj, or the last two path
/// components where two members share a base name.
fn memberIds(names: []const []const u8) ![][]const u8 {
    var count: std.StringHashMapUnmanaged(u32) = .empty;
    const norm = try gpa.alloc([]u8, names.len);
    for (names, 0..) |n, i| {
        norm[i] = try gpa.dupe(u8, n);
        std.mem.replaceScalar(u8, norm[i], '\\', '/');
        const b = try lower(util.basename(norm[i]));
        const g = try count.getOrPut(gpa, b);
        if (!g.found_existing) g.value_ptr.* = 0;
        g.value_ptr.* += 1;
    }
    const out = try gpa.alloc([]const u8, names.len);
    for (norm, 0..) |n, i| {
        const b = util.basename(n);
        var short: []const u8 = b;
        if (count.get(try lower(b)).? > 1) {
            const head = util.dirname(n);
            short = if (std.mem.indexOfScalar(u8, n, '/') == null) b else try util.join(util.basename(head), b);
        }
        out[i] = if (endsWithObjCi(short)) short[0 .. short.len - 4] else short;
    }
    return out;
}

/// Every function of the libraries, as OurFuncs with tu '<lib>/<member id>'.
pub fn loadLibFunctions(d: *dec.Decoder) ![]ours.OurFunc {
    const dir = try libDir();
    var fns: std.ArrayList(ours.OurFunc) = .empty;
    for (LIBS) |lib| {
        const data = try util.readFile(try util.join(dir, lib));
        var members: std.ArrayList(coff.Member) = .empty;
        for (try coff.readArchive(data)) |m| {
            if (m.body.len >= 4 and std.mem.eql(u8, m.body[0..4], "\x00\x00\xff\xff")) continue;
            if (m.body.len < 20) continue;
            try members.append(gpa, m);
        }
        const names = try gpa.alloc([]const u8, members.items.len);
        for (members.items, 0..) |m, i| names[i] = m.name;
        const ids = try memberIds(names);
        for (members.items, 0..) |m, i| {
            // Not an i386 COFF object (an /GL or anonymous member): skipped.
            const fl = objfuncs.functions(d, m.name, m.body, true) catch continue;
            const tu = try std.fmt.allocPrint(gpa, "{s}/{s}", .{ lib, ids[i] });
            for (fl, 0..) |f, k| try fns.append(gpa, .{ .tu = tu, .order = k, .f = f });
        }
    }
    return fns.items;
}

pub fn isImportThunk(e: *const refimage.ModelEntry) bool {
    const insns = e.f.insns.items;
    if (insns.len != 1 or insns[0].size < 2 or insns[0].bytes[0] != 0xff or insns[0].bytes[1] != 0x25) return false;
    for (e.refs) |r| if (r.kind == .import) return true;
    return false;
}

// -- the pairing (libmap.py main): evidence/libmap.tsv and its .summary ---------------------

const compare = @import("compare.zig");
const difflib = @import("difflib.zig");
const audit = @import("audit.zig");
const crtmap = @import("crtmap.zig");
const stage = @import("stage.zig");
const c = @import("c");

const EXACT_MIN_INSNS = 3;
const LIB_ORDER_THRESHOLD = 0.60;
const LIB_ORDER_MARGIN = 0.10;
const MAX_PIECES = 4;
const LIB_SIMILAR_THRESHOLD = 0.80;
const LIB_SIMILAR_MARGIN = 0.05;
const VOTES_MIN = 2;
const VOTES_SHARE = 0.75;
const NEIGHBOURS = 3;
const PAD = 16;

fn isLowerHex(s: []const u8) bool {
    for (s) |ch| if (!((ch >= '0' and ch <= '9') or (ch >= 'a' and ch <= 'f'))) return false;
    return true;
}

/// (member base name lowercased, symbol) of every library symbol the linker map lists
/// (libmap.linked_symbols), each as "member\x00symbol"; empty when there is no map.
pub fn linkedSymbols(path: []const u8) !std.StringHashMapUnmanaged(void) {
    var out: std.StringHashMapUnmanaged(void) = .empty;
    const data = (try util.readFileOpt(path)) orelse return out;
    var it = util.Lines{ .data = data };
    while (it.next()) |raw| {
        const line = std.mem.trimEnd(u8, raw, " \t\n\r\x0b\x0c");
        var toks: std.ArrayList([]const u8) = .empty;
        var ti = std.mem.tokenizeAny(u8, line, " \t\n\r\x0b\x0c");
        while (ti.next()) |t| try toks.append(gpa, t);
        const t = toks.items;
        // ^\s*[0-9a-f]{4}:[0-9a-f]{8}\s+(\S+)\s+([0-9a-f]{8})\s+(?:f\s+)?(?:i\s+)?(\S+):(\S+)$
        if (t.len < 4 or t[0].len != 13 or t[0][4] != ':' or !isLowerHex(t[0][0..4]) or !isLowerHex(t[0][5..])) continue;
        if (t[2].len != 8 or !isLowerHex(t[2])) continue;
        var k: usize = 3;
        if (k + 1 < t.len and std.mem.eql(u8, t[k], "f")) k += 1;
        if (k + 1 < t.len and std.mem.eql(u8, t[k], "i")) k += 1;
        if (k + 1 != t.len) continue;
        const last = t[k];
        // (\S+):(\S+)$: the last colon that leaves something after it
        var colon: ?usize = null;
        var j: usize = last.len;
        while (j > 1) {
            j -= 1;
            if (last[j] == ':' and j + 1 < last.len) {
                colon = j;
                break;
            }
        }
        const cpos = colon orelse continue;
        const member = try lower(last[cpos + 1 ..]);
        try out.put(gpa, try std.fmt.allocPrint(gpa, "{s}\x00{s}", .{ member, t[1] }), {});
    }
    return out;
}

pub fn isLinked(f: *const ours.OurFunc, linked: *const std.StringHashMapUnmanaged(void)) !bool {
    const rest = f.tu[(std.mem.indexOfScalar(u8, f.tu, '/') orelse return false) + 1 ..];
    const base = util.basename(rest);
    return linked.contains(try std.fmt.allocPrint(gpa, "{s}.obj\x00{s}", .{ try lower(base), f.f.symbol }));
}

/// SequenceMatcher.quick_ratio and real_quick_ratio, as difflib computes them.
fn quickRatio(a: []const u32, b: []const u32) !f64 {
    var full: std.AutoHashMapUnmanaged(u32, i64) = .empty;
    for (b) |x| {
        const g = try full.getOrPut(gpa, x);
        if (!g.found_existing) g.value_ptr.* = 0;
        g.value_ptr.* += 1;
    }
    var avail: std.AutoHashMapUnmanaged(u32, i64) = .empty;
    var matches: usize = 0;
    for (a) |x| {
        const numb = avail.get(x) orelse (full.get(x) orelse 0);
        try avail.put(gpa, x, numb - 1);
        if (numb > 0) matches += 1;
    }
    return ratioOf(matches, a.len + b.len);
}

fn realQuickRatio(a: []const u32, b: []const u32) f64 {
    return ratioOf(@min(a.len, b.len), a.len + b.len);
}

fn ratioOf(matches: usize, length: usize) f64 {
    if (length == 0) return 1.0;
    return 2.0 * @as(f64, @floatFromInt(matches)) / @as(f64, @floatFromInt(length));
}

/// The neutral listing of a view, as interned ids.
fn listing(v: []const compare.NInsn) ![]u32 {
    const out = try gpa.alloc(u32, v.len);
    for (v, 0..) |ins, k| out[k] = ins.key;
    return out;
}

const MapE = struct { s: u32, e: u32, method: []const u8, detail: []const u8 };
const Span = struct { s: u32, e: u32 };
const Scored = struct { sim: f64, g: u32, s: u32, e: u32 };
const Site = struct { sym: []const u8, t: u32 };
const Voter = struct { s: u32, e: u32, f: *const ours.OurFunc };

const LibMatcher = struct {
    model: *compare.Model,
    starts: []const u32,
    index: std.AutoHashMapUnmanaged(u32, usize) = .empty,
    taken: std.AutoHashMapUnmanaged(u32, void) = .empty,
    fns: []ours.OurFunc,
    kid: []u32 = &.{},
    names: [][]const u8 = &.{},
    linked: *const std.StringHashMapUnmanaged(void),
    viewer: *compare.Viewer,
    views: std.AutoHashMapUnmanaged(usize, []compare.NInsn) = .empty,
    map: std.AutoArrayHashMapUnmanaged(u32, MapE) = .empty,
    by_member: std.StringArrayHashMapUnmanaged(std.ArrayList(u32)) = .empty,
    pos: std.AutoHashMapUnmanaged(u32, usize) = .empty,

    fn init(m: *LibMatcher) !void {
        for (m.starts, 0..) |s, k| try m.index.put(gpa, s, k);
        m.names = try gpa.alloc([]const u8, m.fns.len);
        m.kid = try gpa.alloc(u32, m.fns.len);
        var last: std.StringHashMapUnmanaged(u32) = .empty;
        for (m.fns, 0..) |*f, i| {
            m.names[i] = try f.key();
            try last.put(gpa, m.names[i], @intCast(i));
            const g = try m.by_member.getOrPut(gpa, f.tu);
            if (!g.found_existing) g.value_ptr.* = .empty;
            try g.value_ptr.append(gpa, @intCast(i));
        }
        if (last.count() != m.fns.len) std.debug.print("libmap: {d} library functions share a key with another\n", .{m.fns.len - last.count()});
        for (m.names, 0..) |k, i| m.kid[i] = last.get(k).?;
        for (m.by_member.values()) |*v| {
            std.mem.sort(u32, v.items, m.fns, struct {
                fn lt(fs: []ours.OurFunc, a: u32, b: u32) bool {
                    return fs[a].order < fs[b].order;
                }
            }.lt);
            for (v.items, 0..) |f, k| try m.pos.put(gpa, m.kid[f], k);
        }
    }

    fn our(m: *LibMatcher, f: *const ours.OurFunc) ![]compare.NInsn {
        const key = @intFromPtr(f);
        if (m.views.get(key)) |v| return v;
        const v = try m.viewer.ourView(f, crtmap.NoSyms{});
        try m.views.put(gpa, key, v);
        return v;
    }

    fn mapped(m: *const LibMatcher, i: u32) bool {
        return m.map.contains(m.kid[i]);
    }

    fn end(m: *const LibMatcher, s: u32) u32 {
        return m.model.getPtr(s).?.f.end;
    }

    /// Ends of the runs of 1..MAX_PIECES unpaired carved functions starting at s.
    fn spans(m: *const LibMatcher, s: u32, limit: ?u32) ![]u32 {
        var out: std.ArrayList(u32) = .empty;
        if (m.taken.contains(s)) return out.items;
        var e = s;
        for (0..MAX_PIECES) |n| {
            if (n > 0 and (!m.model.contains(e) or m.taken.contains(e))) break;
            e = m.end(e);
            if (limit) |l| if (e - s > l) break;
            try out.append(gpa, e);
        }
        return out.items;
    }

    fn take(m: *LibMatcher, i: u32, s: u32, e: u32, method: []const u8, detail0: []const u8) !bool {
        const pieces = (try compare.spanPieces(m.model, s, e)).?;
        for (pieces) |p| if (m.taken.contains(p)) return false;
        if (m.mapped(i)) return false;
        for (pieces) |p| try m.taken.put(gpa, p, {});
        var detail = detail0;
        if (pieces.len > 1) detail = try std.fmt.allocPrint(gpa, "{s}{s}{d} pieces", .{ detail, if (detail.len > 0) "; " else "", pieces.len });
        if (try isLinked(&m.fns[i], m.linked)) detail = try std.fmt.allocPrint(gpa, "{s}{s}linked", .{ detail, if (detail.len > 0) "; " else "" });
        try m.map.put(gpa, m.kid[i], .{ .s = s, .e = e, .method = method, .detail = detail });
        return true;
    }

    fn byExact(m: *LibMatcher, extent: ?[2]u32) !usize {
        var lib: std.StringArrayHashMapUnmanaged(std.ArrayList(u32)) = .empty;
        var shapes: std.AutoHashMapUnmanaged([2]usize, void) = .empty;
        var limit: u32 = 0;
        for (m.fns, 0..) |*f, i| {
            limit = @max(limit, @as(u32, @intCast(f.f.code.len)));
            const v = try m.our(f);
            if (v.len < EXACT_MIN_INSNS) continue;
            const k = try crtmap.keyOf(v);
            const g = try lib.getOrPut(gpa, k);
            if (!g.found_existing) g.value_ptr.* = .empty;
            try g.value_ptr.append(gpa, @intCast(i));
            var nb: usize = 0;
            for (v) |ins| nb += ins.masked.len;
            try shapes.put(gpa, .{ v.len, nb }, {});
        }
        var ref: std.StringArrayHashMapUnmanaged(std.ArrayList(Span)) = .empty;
        for (m.starts) |s| {
            for (try m.spans(s, limit)) |e| {
                const pieces = (try compare.spanPieces(m.model, s, e)).?;
                var ni: usize = 0;
                var nb: usize = 0;
                for (pieces) |p| for (m.model.getPtr(p).?.f.insns.items) |ins| {
                    ni += 1;
                    nb += ins.size;
                };
                if (!shapes.contains(.{ ni, nb })) continue;
                const g = try ref.getOrPut(gpa, try crtmap.keyOf(try m.viewer.refViewSpan(s, e, m.model)));
                if (!g.found_existing) g.value_ptr.* = .empty;
                try g.value_ptr.append(gpa, .{ .s = s, .e = e });
            }
        }
        // Longest code first (Python orders equal lengths by set order; here first seen).
        var both: std.ArrayList([]const u8) = .empty;
        for (ref.keys()) |k| if (lib.contains(k)) try both.append(gpa, k);
        std.mem.sort([]const u8, both.items, {}, struct {
            fn lt(_: void, a: []const u8, b: []const u8) bool {
                return byteLen(a) > byteLen(b);
            }
        }.lt);
        var n: usize = 0;
        for (both.items) |k| {
            const r = ref.get(k).?.items;
            if (r.len != 1) continue;
            const s = r[0].s;
            const e = r[0].e;
            const l = lib.get(k).?.items;
            if (l.len == 1) {
                n += @intFromBool(try m.take(l[0], s, e, "lib-exact", ""));
            } else if (extent) |x| if (x[0] <= s and s < x[1]) {
                var one: ?u32 = null;
                var cnt: usize = 0;
                for (l) |f| if (try isLinked(&m.fns[f], m.linked) and !m.mapped(f)) {
                    cnt += 1;
                    one = f;
                };
                if (cnt == 1) n += @intFromBool(try m.take(one.?, s, e, "lib-exact", try std.fmt.allocPrint(gpa, "one of {d} identical", .{l.len})));
            };
        }
        return n;
    }

    /// The masked byte length of a keyOf key ("<n>:<bytes>").
    fn byteLen(k: []const u8) usize {
        return k.len - (std.mem.indexOfScalar(u8, k, ':').? + 1);
    }

    fn runExtent(m: *const LibMatcher) [2]u32 {
        var lo: ?Span = null;
        var hi: ?Span = null;
        for (m.map.values()) |v| if (std.mem.eql(u8, v.method, "lib-exact")) {
            const sp = Span{ .s = v.s, .e = v.e };
            if (lo == null or sp.s < lo.?.s or (sp.s == lo.?.s and sp.e < lo.?.e)) lo = sp;
            if (hi == null or sp.s > hi.?.s or (sp.s == hi.?.s and sp.e > hi.?.e)) hi = sp;
        };
        return if (lo) |l| .{ l.s, hi.?.e } else .{ 0, 0 };
    }

    fn similarity(m: *LibMatcher, f: u32, s: u32, e: u32) !f64 {
        const a = try listing(try m.our(&m.fns[f]));
        const b = try listing(try m.viewer.refViewSpan(s, e, m.model));
        var sm = try difflib.SequenceMatcher.init(gpa, a, b);
        defer sm.deinit();
        return sm.ratio();
    }

    fn bySimilar(m: *LibMatcher, lo: u32, hi: u32) !usize {
        var bylen: std.AutoHashMapUnmanaged(usize, std.ArrayList(u32)) = .empty;
        for (m.fns, 0..) |*f, i| if (!m.mapped(@intCast(i))) {
            const g = try bylen.getOrPut(gpa, (try m.our(f)).len);
            if (!g.found_existing) g.value_ptr.* = .empty;
            try g.value_ptr.append(gpa, @intCast(i));
        };
        var n: usize = 0;
        for (m.starts) |s| {
            if (!(lo <= s and s < hi) or m.taken.contains(s)) continue;
            var scored: std.ArrayList(Scored) = .empty;
            for (try m.spans(s, null)) |e| {
                const b = try listing(try m.viewer.refViewSpan(s, e, m.model));
                if (b.len < EXACT_MIN_INSNS) continue;
                const fl: f64 = @floatFromInt(b.len);
                const mlo: usize = @intFromFloat(@trunc(fl * 0.8));
                const mhi: usize = @intFromFloat(@trunc(fl * 1.25));
                var ml = mlo;
                while (ml <= mhi) : (ml += 1) {
                    const gs = bylen.get(ml) orelse continue;
                    for (gs.items) |g| {
                        const a = try listing(try m.our(&m.fns[g]));
                        const floor = if (scored.items.len > 1) scored.items[1].sim else LIB_SIMILAR_THRESHOLD - LIB_SIMILAR_MARGIN;
                        if (realQuickRatio(a, b) < floor or try quickRatio(a, b) < floor) continue;
                        var sm = try difflib.SequenceMatcher.init(gpa, a, b);
                        defer sm.deinit();
                        try scored.append(gpa, .{ .sim = try sm.ratio(), .g = g, .s = s, .e = e });
                        std.mem.sort(Scored, scored.items, {}, struct {
                            fn lt(_: void, x: Scored, y: Scored) bool {
                                return x.sim > y.sim;
                            }
                        }.lt);
                        // Keep the best per function, and the best of any other.
                        var kept: std.ArrayList(Scored) = .empty;
                        var seen: std.AutoHashMapUnmanaged(u32, void) = .empty;
                        for (scored.items) |t| {
                            if (seen.contains(m.kid[t.g])) continue;
                            try seen.put(gpa, m.kid[t.g], {});
                            try kept.append(gpa, t);
                        }
                        kept.items.len = @min(kept.items.len, 2);
                        scored = kept;
                    }
                }
            }
            if (scored.items.len == 0) continue;
            const top = scored.items[0];
            const rival = if (scored.items.len > 1) scored.items[1].sim else 0.0;
            if (top.sim >= LIB_SIMILAR_THRESHOLD and top.sim - rival >= LIB_SIMILAR_MARGIN) {
                var d: std.ArrayList(u8) = .empty;
                try util.fixed(&d, "%.2f", top.sim);
                try d.appendSlice(gpa, ", next ");
                try util.fixed(&d, "%.2f", rival);
                n += @intFromBool(try m.take(top.g, s, top.e, "lib-similar", d.items));
            }
        }
        return n;
    }

    /// The unpaired functions of f's member next to it, up to the next paired one.
    fn neighbours(m: *const LibMatcher, f: u32, step: i64) ![]u32 {
        const member = m.by_member.get(m.fns[f].tu).?.items;
        var k: i64 = @as(i64, @intCast(m.pos.get(m.kid[f]).?)) + step;
        var out: std.ArrayList(u32) = .empty;
        while (k >= 0 and k < member.len and out.items.len < NEIGHBOURS) {
            const g = member[@intCast(k)];
            if (m.mapped(g)) break;
            try out.append(gpa, g);
            k += step;
        }
        return out.items;
    }

    fn best(m: *LibMatcher, cands: []const u32, sps: []const Span) !?Scored {
        var scored: std.ArrayList(Scored) = .empty;
        for (cands) |g| for (sps) |sp| try scored.append(gpa, .{ .sim = try m.similarity(g, sp.s, sp.e), .g = g, .s = sp.s, .e = sp.e });
        if (scored.items.len == 0) return null;
        std.mem.sort(Scored, scored.items, {}, struct {
            fn lt(_: void, x: Scored, y: Scored) bool {
                return x.sim > y.sim;
            }
        }.lt);
        const top = scored.items[0];
        var rival: f64 = 0.0;
        for (scored.items[1..]) |t| if (m.kid[t.g] != m.kid[top.g]) {
            rival = t.sim;
            break;
        };
        if (top.sim >= LIB_ORDER_THRESHOLD and top.sim - rival >= LIB_ORDER_MARGIN) return top;
        return null;
    }

    fn orderDetail(where: []const u8, name: []const u8, sim: f64) ![]const u8 {
        var d: std.ArrayList(u8) = .empty;
        try d.print(gpa, "{s} {s} ", .{ where, name });
        try util.fixed(&d, "%.2f", sim);
        return d.items;
    }

    fn byOrder(m: *LibMatcher) !usize {
        var n: usize = 0;
        var changed = true;
        while (changed) {
            changed = false;
            const K = struct { k: u32, s: u32, e: u32 };
            var snap: std.ArrayList(K) = .empty;
            for (m.map.keys(), m.map.values()) |k, v| try snap.append(gpa, .{ .k = k, .s = v.s, .e = v.e });
            std.mem.sort(K, snap.items, {}, struct {
                fn lt(_: void, a: K, b: K) bool {
                    return a.s < b.s;
                }
            }.lt);
            for (snap.items) |it| {
                const f = it.k;
                const s = it.s;
                const e = it.e;
                var k: ?usize = m.index.get(e);
                if (k == null) {
                    var j = m.index.get(s).? + 1;
                    while (j < m.starts.len) : (j += 1) if (m.starts[j] >= e) {
                        k = j;
                        break;
                    };
                }
                if (k) |kk| if (kk < m.starts.len and m.starts[kk] - e < PAD) {
                    const t = m.starts[kk];
                    var sps: std.ArrayList(Span) = .empty;
                    for (try m.spans(t, null)) |x| try sps.append(gpa, .{ .s = t, .e = x });
                    if (try m.best(try m.neighbours(f, 1), sps.items)) |hit| {
                        if (try m.take(hit.g, hit.s, hit.e, "lib-order", try orderDetail("after", m.fns[f].f.name, hit.sim))) {
                            n += 1;
                            changed = true;
                        }
                    }
                };
                const j = m.index.get(s).?;
                var runs: std.ArrayList(Span) = .empty;
                for (1..MAX_PIECES + 1) |back| {
                    if (j < back) break;
                    const t = m.starts[j - back];
                    for (try m.spans(t, null)) |x| {
                        const d = @as(i64, s) - @as(i64, x);
                        if (0 <= d and d < PAD) try runs.append(gpa, .{ .s = t, .e = x });
                    }
                }
                if (try m.best(try m.neighbours(f, -1), runs.items)) |hit| {
                    if (try m.take(hit.g, hit.s, hit.e, "lib-order", try orderDetail("before", m.fns[f].f.name, hit.sim))) {
                        n += 1;
                        changed = true;
                    }
                }
            }
        }
        return n;
    }

    fn alignedCalls(m: *LibMatcher, f: *const ours.OurFunc, s: u32, e: u32) ![]Site {
        const ov = try m.our(f);
        const rv = try m.viewer.refViewSpan(s, e, m.model);
        var sm = try difflib.SequenceMatcher.init(gpa, try listing(ov), try listing(rv));
        defer sm.deinit();
        var out: std.ArrayList(Site) = .empty;
        for (try sm.matchingBlocks()) |b| {
            for (ov[b.a .. b.a + b.size], rv[b.b .. b.b + b.size]) |x, y| {
                if (!std.mem.eql(u8, x.mn_text, "call")) continue;
                const nt = @min(x.tokens.len, y.tokens.len);
                for (x.tokens[0..nt], y.tokens[0..nt]) |ta, tb| if (ta.kind == .fnq and tb.kind == .fn_) try out.append(gpa, .{ .sym = ta.s, .t = @intCast(tb.int) });
            }
        }
        return out.items;
    }

    fn byVotes(m: *LibMatcher, pairs: []const Voter) !usize {
        var cands: std.StringArrayHashMapUnmanaged(std.ArrayList(u32)) = .empty;
        var own: std.StringHashMapUnmanaged(u32) = .empty;
        var defined: std.StringHashMapUnmanaged(void) = .empty;
        for (m.fns, 0..) |*f, i| {
            try defined.put(gpa, try std.fmt.allocPrint(gpa, "{s}\x00{s}", .{ f.tu, f.f.symbol }), {});
            if (!m.mapped(@intCast(i)) and try isLinked(f, m.linked)) {
                const g = try cands.getOrPut(gpa, f.f.symbol);
                if (!g.found_existing) g.value_ptr.* = .empty;
                try g.value_ptr.append(gpa, @intCast(i));
                try own.put(gpa, try std.fmt.allocPrint(gpa, "{s}\x00{s}", .{ f.tu, f.f.symbol }), @intCast(i));
            }
        }
        var by_sym: std.StringHashMapUnmanaged(u32) = .empty;
        for (cands.keys(), cands.values()) |k, v| if (v.items.len == 1) try by_sym.put(gpa, k, v.items[0]);
        var votes: std.AutoArrayHashMapUnmanaged(u32, std.AutoArrayHashMapUnmanaged(u32, usize)) = .empty;
        var target_fn: std.AutoHashMapUnmanaged(u32, u32) = .empty;
        for (pairs) |p| {
            var any = false;
            for (p.f.f.refs) |r| if ((r.kind == .call or r.kind == .fptr) and cands.contains(r.s)) {
                any = true;
            };
            if (!any) continue; // no call that could vote: nothing to align
            for (try m.alignedCalls(p.f, p.s, p.e)) |site| {
                const dk = try std.fmt.allocPrint(gpa, "{s}\x00{s}", .{ p.f.tu, site.sym });
                const g: ?u32 = if (defined.contains(dk)) own.get(dk) else by_sym.get(site.sym);
                const gi = g orelse continue;
                try target_fn.put(gpa, m.kid[gi], gi);
                const vg = try votes.getOrPut(gpa, m.kid[gi]);
                if (!vg.found_existing) vg.value_ptr.* = .empty;
                const cg = try vg.value_ptr.getOrPut(gpa, site.t);
                if (!cg.found_existing) cg.value_ptr.* = 0;
                cg.value_ptr.* += 1;
            }
        }
        // sorted(votes.items()): by our key
        const vk = try gpa.dupe(u32, votes.keys());
        std.mem.sort(u32, vk, m, struct {
            fn lt(mm: *LibMatcher, a: u32, b: u32) bool {
                return std.mem.order(u8, mm.names[a], mm.names[b]) == .lt;
            }
        }.lt);
        var n: usize = 0;
        for (vk) |key| {
            const cnt = votes.get(key).?;
            var t: u32 = 0;
            var k: usize = 0;
            var total: usize = 0;
            for (cnt.keys(), cnt.values()) |tt, v| {
                total += v;
                if (v > k) {
                    k = v;
                    t = tt;
                }
            }
            if (k < VOTES_MIN or @as(f64, @floatFromInt(k)) < VOTES_SHARE * @as(f64, @floatFromInt(total)) or m.taken.contains(t) or !m.model.contains(t)) continue;
            const f = target_fn.get(key).?;
            const ends = try m.spans(t, @intCast(m.fns[f].f.code.len));
            if (ends.len == 0) continue;
            var bsim: f64 = -1;
            var be: u32 = 0;
            for (ends) |e| {
                const sim = try m.similarity(f, t, e);
                if (sim > bsim or (sim == bsim and e > be)) {
                    bsim = sim;
                    be = e;
                }
            }
            if (bsim < LIB_ORDER_THRESHOLD) continue;
            var d: std.ArrayList(u8) = .empty;
            try d.print(gpa, "{d} of {d} call sites, ", .{ k, total });
            try util.fixed(&d, "%.2f", bsim);
            n += @intFromBool(try m.take(f, t, be, "lib-votes", d.items));
        }
        return n;
    }
};

fn realpathOf(p: []const u8) ![]const u8 {
    if (builtin.os.tag == .windows) return proc.realpath(p);
    var buf: [4096]u8 = undefined;
    const pz = try util.z(p);
    const r = c.realpath(pz.ptr, &buf) orelse return p;
    return gpa.dupe(u8, std.mem.sliceTo(r, 0));
}

/// `oracle-zig libmap --root <dir> [--exe-map F] [--funcmap F] [--out F]` (libmap.py main).
pub fn main(args: []const []const u8) !void {
    var root: ?[]const u8 = null;
    var exe_map: ?[]const u8 = null;
    var funcmap_path: ?[]const u8 = null;
    var out_path: ?[]const u8 = null;
    var k: usize = 0;
    while (k < args.len) : (k += 1) {
        const Opt = struct { name: []const u8, dst: *?[]const u8 };
        const opts = [_]Opt{ .{ .name = "--root", .dst = &root }, .{ .name = "--exe-map", .dst = &exe_map }, .{ .name = "--funcmap", .dst = &funcmap_path }, .{ .name = "--out", .dst = &out_path } };
        var hit = false;
        for (opts) |o| if (std.mem.eql(u8, args[k], o.name) and k + 1 < args.len) {
            o.dst.* = args[k + 1];
            k += 1;
            hit = true;
            break;
        };
        if (!hit) {
            std.debug.print("usage: oracle-zig libmap --root <asm_matching/msvc8> [--exe-map F] [--funcmap F] [--out F]\n", .{});
            std.process.exit(2);
        }
    }
    const cfg = try audit.Config.init(root orelse {
        std.debug.print("oracle-zig libmap: --root is required\n", .{});
        std.process.exit(2);
    });
    const exe_map_file = exe_map orelse try util.join(cfg.oracle, "build-exe/d2recon-vc8.map");
    const funcmap_file = funcmap_path orelse try cfg.evidence("funcmap.tsv");
    const out_file = out_path orelse try cfg.evidence("libmap.tsv");
    if (cfg.experiment and std.mem.eql(u8, try realpathOf(util.dirname(out_file)), try realpathOf(try util.join(cfg.oracle, "evidence")))) {
        std.debug.print("libmap: not in an experiment - an A/B audit uses the baseline map (evidence/libmap.tsv)\n", .{});
        std.process.exit(1);
    }
    var linked = try linkedSymbols(exe_map_file);
    // Everything the pairing reads: the funcmap, which library functions our image links,
    // the libraries, our objects, the original and its carve (and this executable).
    const objdir = try util.join(cfg.build, "obj");
    var key = try stage.Key.init("libmap");
    try key.file(funcmap_file);
    try key.file(exe_map_file);
    const ldir = try libDir();
    for (LIBS) |lib| try key.file(try util.join(ldir, lib));
    for (try ours.completedObjects(objdir)) |rel| try key.file(try util.join(objdir, rel));
    try key.file(try util.join(cfg.oracle, "reference/Game.exe"));
    try key.file(try util.join(cfg.refcache, "functions.tsv"));
    const key_hex = key.hex();
    const summary_file = try std.fmt.allocPrint(gpa, "{s}.summary", .{out_file});
    const outputs = [_][]const u8{ out_file, summary_file };
    if (try stage.reuse(cfg.build, "libmap", &key_hex, &outputs)) {
        util.say(try util.readFile(summary_file));
        return;
    }
    const ref = try refimage.Reference.open(try util.join(cfg.oracle, "reference/Game.exe"));
    try ref.load(try util.join(cfg.refcache, "functions.tsv"));
    var model = try ref.model();
    const rows = try audit.loadFuncmap(funcmap_file);
    var paired: std.AutoHashMapUnmanaged(u32, void) = .empty;
    for (rows) |r| try paired.put(gpa, r.s, {});
    var d = try dec.Decoder.init(gpa);
    const fns = try loadLibFunctions(&d);
    var viewer = compare.Viewer{ .d = &d };
    var m = LibMatcher{ .model = &model, .starts = ref.starts, .fns = fns, .linked = &linked, .viewer = &viewer };
    try m.init();
    var pit = paired.keyIterator();
    while (pit.next()) |s| try m.taken.put(gpa, s.*, {});
    _ = try m.byExact(null);
    _ = try m.byOrder();
    const ext = m.runExtent();
    const lo = ext[0];
    const hi = ext[1];
    _ = try m.byExact(ext);
    _ = try m.bySimilar(lo, hi);
    _ = try m.byOrder();
    var objs: std.StringHashMapUnmanaged(*const ours.OurFunc) = .empty;
    for (try ours.loadAll(&d, objdir)) |*f| try objs.put(gpa, try f.key(), f);
    var game: std.ArrayList(Voter) = .empty;
    for (rows) |r| {
        const f = objs.get(try std.fmt.allocPrint(gpa, "{s}|{s}", .{ r.tu, r.sym })) orelse continue;
        if (model.contains(r.s)) try game.append(gpa, .{ .s = r.s, .e = r.e, .f = f });
    }
    for (0..3) |_| {
        var all: std.ArrayList(Voter) = .empty;
        try all.appendSlice(gpa, game.items);
        for (m.map.keys(), m.map.values()) |kk, v| try all.append(gpa, .{ .s = v.s, .e = v.e, .f = &fns[kk] });
        var got = try m.byVotes(all.items);
        got += try m.byOrder();
        if (got == 0) break;
    }
    const Row = struct { s: u32, i: u32, v: MapE };
    var out: std.ArrayList(Row) = .empty;
    for (fns, 0..) |_, i| if (m.map.get(m.kid[i])) |v| try out.append(gpa, .{ .s = v.s, .i = @intCast(i), .v = v });
    std.mem.sort(Row, out.items, {}, struct {
        fn lt(_: void, a: Row, b: Row) bool {
            return a.s < b.s;
        }
    }.lt);
    var o: std.ArrayList(u8) = .empty;
    try o.appendSlice(gpa, "start\tend\tname\ttu\tsymbol\tmethod\tdetail\n");
    for (out.items) |r| {
        const f = &fns[r.i];
        try o.print(gpa, "{x:0>8}\t{x:0>8}\t{s}\t{s}\t{s}\t{s}\t{s}\n", .{ r.s, r.v.e, f.f.name, f.tu, f.f.symbol, r.v.method, r.v.detail });
    }
    util.makeDirs(util.dirname(out_file));
    try util.writeFile(out_file, o.items);

    var nlinked: usize = 0;
    for (fns) |*f| nlinked += @intFromBool(try isLinked(f, &linked));
    var lines: std.ArrayList(u8) = .empty;
    try lines.print(gpa, "libcmt.lib + libcpmt.lib functions ({s}): {d}, of which our image links {d}", .{ LIBVC, fns.len, nlinked });
    if (linked.count() == 0) try lines.print(gpa, " (no linker map: {s})", .{exe_map_file});
    var nunp: usize = 0;
    var bunp: u64 = 0;
    var mit = model.iterator();
    while (mit.next()) |e| if (!paired.contains(e.key_ptr.*)) {
        nunp += 1;
        bunp += e.value_ptr.f.end - e.key_ptr.*;
    };
    try lines.print(gpa, "\noriginal functions the funcmap leaves unpaired: {d} ({d} bytes)", .{ nunp, bunp });
    var pbytes: u64 = 0;
    var plinked: usize = 0;
    for (out.items) |r| {
        pbytes += r.v.e - r.s;
        if (std.mem.indexOf(u8, r.v.detail, "linked") != null) plinked += 1;
    }
    try lines.print(gpa, "\npaired with a library function: {d} ({d} bytes), of which our image links {d}", .{ out.items.len, pbytes, plinked });
    for ([_][]const u8{ "lib-exact", "lib-order", "lib-similar", "lib-votes" }) |t| {
        var cnt: usize = 0;
        for (out.items) |r| cnt += @intFromBool(std.mem.eql(u8, r.v.method, t));
        try lines.print(gpa, "\n  {s}", .{t});
        try lines.appendNTimes(gpa, ' ', 11 - t.len);
        try lines.print(gpa, " {d: >5}", .{cnt});
    }
    const Tot = struct { n: usize = 0, b: u64 = 0 };
    var all_left = Tot{};
    var inside = Tot{};
    var thunks = Tot{};
    var outside = Tot{};
    for (ref.starts) |s| {
        if (m.taken.contains(s)) continue;
        const e = model.getPtr(s).?;
        const sz = e.f.end - s;
        all_left.n += 1;
        all_left.b += sz;
        const t: *Tot = if (isImportThunk(e)) &thunks else if (lo <= s and s < hi) &inside else &outside;
        t.n += 1;
        t.b += sz;
    }
    try lines.print(gpa, "\nstill unpaired: {d} ({d} bytes)", .{ all_left.n, all_left.b });
    try lines.print(gpa, "\n  among the library code ({x:0>8}..{x:0>8}): {d} ({d} bytes)", .{ lo, hi, inside.n, inside.b });
    try lines.print(gpa, "\n  import thunks (jmp [__imp_X], made by the linker): {d} ({d} bytes)", .{ thunks.n, thunks.b });
    try lines.print(gpa, "\n  elsewhere: {d} ({d} bytes)\n", .{ outside.n, outside.b });
    try util.writeFile(summary_file, lines.items);
    util.say(lines.items);
    try stage.done(cfg.build, "libmap", &key_hex, &outputs);
}
