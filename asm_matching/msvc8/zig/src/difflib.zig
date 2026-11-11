//! difflib.SequenceMatcher(None, a, b, autojunk=False), element for element: the same
//! longest-match search, the same tie-breaking, so the same matching blocks and opcodes.
//! Elements are interned ids (equal strings, equal ids).
const std = @import("std");

pub const Match = struct { a: usize, b: usize, size: usize };
pub const Tag = enum { equal, replace, delete, insert };
pub const Opcode = struct { tag: Tag, ci: usize, ci2: usize, cj: usize, cj2: usize };

pub const SequenceMatcher = struct {
    gpa: std.mem.Allocator,
    a: []const u32,
    b: []const u32,
    b2j: std.AutoHashMapUnmanaged(u32, std.ArrayList(usize)) = .empty,
    blocks: ?[]Match = null,

    pub fn init(gpa: std.mem.Allocator, a: []const u32, b: []const u32) !SequenceMatcher {
        var sm = SequenceMatcher{ .gpa = gpa, .a = a, .b = b };
        for (b, 0..) |elt, i| {
            const g = try sm.b2j.getOrPut(gpa, elt);
            if (!g.found_existing) g.value_ptr.* = .empty;
            try g.value_ptr.append(gpa, i);
        }
        return sm;
    }

    pub fn deinit(sm: *SequenceMatcher) void {
        var it = sm.b2j.valueIterator();
        while (it.next()) |v| v.deinit(sm.gpa);
        sm.b2j.deinit(sm.gpa);
        if (sm.blocks) |bl| sm.gpa.free(bl);
    }

    pub fn findLongestMatch(sm: *SequenceMatcher, alo: usize, ahi: usize, blo: usize, bhi: usize) !Match {
        const a = sm.a;
        const b = sm.b;
        var besti = alo;
        var bestj = blo;
        var bestsize: usize = 0;
        var j2len: std.AutoHashMapUnmanaged(usize, usize) = .empty;
        defer j2len.deinit(sm.gpa);
        var newj2len: std.AutoHashMapUnmanaged(usize, usize) = .empty;
        defer newj2len.deinit(sm.gpa);
        var i = alo;
        while (i < ahi) : (i += 1) {
            newj2len.clearRetainingCapacity();
            if (sm.b2j.get(a[i])) |js| {
                for (js.items) |j| {
                    if (j < blo) continue;
                    if (j >= bhi) break;
                    const k = (if (j > 0) j2len.get(j - 1) orelse 0 else 0) + 1;
                    try newj2len.put(sm.gpa, j, k);
                    if (k > bestsize) {
                        besti = i + 1 - k;
                        bestj = j + 1 - k;
                        bestsize = k;
                    }
                }
            }
            std.mem.swap(std.AutoHashMapUnmanaged(usize, usize), &j2len, &newj2len);
        }
        // Without junk the extensions below never fire; kept as difflib has them.
        while (besti > alo and bestj > blo and a[besti - 1] == b[bestj - 1]) {
            besti -= 1;
            bestj -= 1;
            bestsize += 1;
        }
        while (besti + bestsize < ahi and bestj + bestsize < bhi and a[besti + bestsize] == b[bestj + bestsize]) bestsize += 1;
        return .{ .a = besti, .b = bestj, .size = bestsize };
    }

    pub fn matchingBlocks(sm: *SequenceMatcher) ![]Match {
        if (sm.blocks) |bl| return bl;
        const la = sm.a.len;
        const lb = sm.b.len;
        const Q = struct { alo: usize, ahi: usize, blo: usize, bhi: usize };
        var queue: std.ArrayList(Q) = .empty;
        defer queue.deinit(sm.gpa);
        try queue.append(sm.gpa, .{ .alo = 0, .ahi = la, .blo = 0, .bhi = lb });
        var mb: std.ArrayList(Match) = .empty;
        defer mb.deinit(sm.gpa);
        while (queue.pop()) |q| {
            const x = try sm.findLongestMatch(q.alo, q.ahi, q.blo, q.bhi);
            if (x.size > 0) {
                try mb.append(sm.gpa, x);
                if (q.alo < x.a and q.blo < x.b) try queue.append(sm.gpa, .{ .alo = q.alo, .ahi = x.a, .blo = q.blo, .bhi = x.b });
                if (x.a + x.size < q.ahi and x.b + x.size < q.bhi) try queue.append(sm.gpa, .{ .alo = x.a + x.size, .ahi = q.ahi, .blo = x.b + x.size, .bhi = q.bhi });
            }
        }
        std.mem.sort(Match, mb.items, {}, struct {
            fn lt(_: void, p: Match, q: Match) bool {
                if (p.a != q.a) return p.a < q.a;
                if (p.b != q.b) return p.b < q.b;
                return p.size < q.size;
            }
        }.lt);
        // Collapse adjacent blocks.
        var out: std.ArrayList(Match) = .empty;
        var ci: usize = 0;
        var cj: usize = 0;
        var ck: usize = 0;
        for (mb.items) |m| {
            if (ci + ck == m.a and cj + ck == m.b) {
                ck += m.size;
            } else {
                if (ck > 0) try out.append(sm.gpa, .{ .a = ci, .b = cj, .size = ck });
                ci = m.a;
                cj = m.b;
                ck = m.size;
            }
        }
        if (ck > 0) try out.append(sm.gpa, .{ .a = ci, .b = cj, .size = ck });
        try out.append(sm.gpa, .{ .a = la, .b = lb, .size = 0 });
        sm.blocks = try out.toOwnedSlice(sm.gpa);
        return sm.blocks.?;
    }

    pub fn opcodes(sm: *SequenceMatcher) ![]Opcode {
        var i: usize = 0;
        var j: usize = 0;
        var out: std.ArrayList(Opcode) = .empty;
        for (try sm.matchingBlocks()) |m| {
            const tag: ?Tag = if (i < m.a and j < m.b) .replace else if (i < m.a) .delete else if (j < m.b) .insert else null;
            if (tag) |t| try out.append(sm.gpa, .{ .tag = t, .ci = i, .ci2 = m.a, .cj = j, .cj2 = m.b });
            i = m.a + m.size;
            j = m.b + m.size;
            if (m.size > 0) try out.append(sm.gpa, .{ .tag = .equal, .ci = m.a, .ci2 = i, .cj = m.b, .cj2 = j });
        }
        return out.toOwnedSlice(sm.gpa);
    }

    pub fn ratio(sm: *SequenceMatcher) !f64 {
        var matches: usize = 0;
        for (try sm.matchingBlocks()) |m| matches += m.size;
        const length = sm.a.len + sm.b.len;
        if (length == 0) return 1.0;
        return 2.0 * @as(f64, @floatFromInt(matches)) / @as(f64, @floatFromInt(length));
    }
};
