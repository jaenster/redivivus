//! Compare one compiled function with its original, instruction by instruction (compare.py).
//!
//! Both sides are reduced to the same view: per instruction, the bytes with every reference
//! field zeroed, and per reference a token naming what it refers to:
//!   fn 0x00401090   a function, by ORIGINAL address (ours through the funcmap; unmapped: fn? name)
//!   import Name     str "..." (by content)     self 0x1c (into the function itself)
//!   data            a global: not comparable one by one (counted; audit checks consistency)
//! A function MATCHes when the streams have the same length, the masked bytes are identical
//! and every comparable reference agrees. Status classes: MATCH, REF_DIFF, INSN_MATCH,
//! INSN_COUNT_MATCH, OUR+N / OUR-N.
const std = @import("std");
const dec = @import("decode.zig");
const difflib = @import("difflib.zig");
const objfuncs = @import("objfuncs.zig");
const ours = @import("ours.zig");
const refimage = @import("refimage.zig");
const util = @import("util.zig");

const gpa = util.gpa;
const SENTINEL: u32 = 0x7EA00000;

/// Token kinds, ordered as Python orders their names ('data' < 'fn' < 'fn?' < 'import' <
/// 'self' < 'str' < 'straddle'); data_sym and data_addr are both 'data'.
pub const TokKind = enum { data_sym, data_addr, fn_, fnq, import, self, str, straddle };

pub const Token = struct {
    kind: TokKind,
    int: u64 = 0, // fn / self / data_addr
    s: []const u8 = "", // fn? name / import name / str bytes / data symbol
    add: i32 = 0, // data_sym addend
    inner: ?*const Token = null, // straddle

    fn kindRank(k: TokKind) u8 {
        return switch (k) {
            .data_sym, .data_addr => 0,
            .fn_ => 1,
            .fnq => 2,
            .import => 3,
            .self => 4,
            .str => 5,
            .straddle => 6,
        };
    }

    /// Python's tuple ordering, for the kinds a sort can meet together.
    pub fn order(a: *const Token, b: *const Token) std.math.Order {
        const ka = kindRank(a.kind);
        const kb = kindRank(b.kind);
        if (ka != kb) return std.math.order(ka, kb);
        return switch (a.kind) {
            .fn_, .self, .data_addr => std.math.order(a.int, b.int),
            .data_sym => blk: {
                const o = std.mem.order(u8, a.s, b.s);
                break :blk if (o != .eq) o else std.math.order(a.add, b.add);
            },
            .straddle => a.inner.?.order(b.inner.?),
            else => std.mem.order(u8, a.s, b.s),
        };
    }

    pub fn eql(a: *const Token, b: *const Token) bool {
        if (a.kind != b.kind) return false;
        return switch (a.kind) {
            .fn_, .self, .data_addr => a.int == b.int,
            .data_sym => std.mem.eql(u8, a.s, b.s) and a.add == b.add,
            .straddle => a.inner.?.eql(b.inner.?),
            else => std.mem.eql(u8, a.s, b.s),
        };
    }

    fn isData(t: *const Token) bool {
        return t.kind == .data_sym or t.kind == .data_addr;
    }
};

pub fn label(out: *std.ArrayList(u8), t: *const Token) !void {
    switch (t.kind) {
        .fn_ => try out.print(gpa, "fn_{x:0>8}", .{t.int}),
        .fnq => try out.print(gpa, "<{s}>", .{t.s}),
        .import => try out.print(gpa, "__imp_{s}", .{t.s}),
        .str => {
            const s = if (t.s.len > 0 and t.s[t.s.len - 1] == 0) t.s[0 .. t.s.len - 1] else t.s;
            if (s.len <= 28) try out.print(gpa, "\"{s}\"", .{s}) else try out.print(gpa, "\"{s}...\"", .{s[0..25]});
        },
        .self => try out.print(gpa, "$+0x{x}", .{t.int}),
        .straddle => {
            try out.appendSlice(gpa, "<straddle ");
            try label(out, t.inner.?);
            try out.append(gpa, '>');
        },
        .data_sym => {
            try out.appendSlice(gpa, t.s);
            if (t.add > 0) try out.print(gpa, "+0x{x}", .{t.add}) else if (t.add < 0) try out.print(gpa, "-0x{x}", .{-@as(i64, t.add)});
        },
        .data_addr => try out.print(gpa, "g_{x:0>8}", .{t.int}),
    }
}

fn neutral(out: *std.ArrayList(u8), t: *const Token) !void {
    switch (t.kind) {
        .fn_, .fnq => try out.appendSlice(gpa, "FN"),
        .data_sym, .data_addr => try out.appendSlice(gpa, "DATA"),
        else => try label(out, t),
    }
}

/// Interned strings: listing keys and mnemonics compare as ids.
pub const Interner = struct {
    map: std.StringHashMapUnmanaged(u32) = .empty,
    /// id -> the string.
    strs: std.ArrayList([]const u8) = .empty,
    pub fn id(in: *Interner, s: []const u8) !u32 {
        const g = try in.map.getOrPut(gpa, s);
        if (!g.found_existing) {
            g.key_ptr.* = try gpa.dupe(u8, s);
            g.value_ptr.* = in.map.count() - 1;
            try in.strs.append(gpa, g.key_ptr.*);
        }
        return g.value_ptr.*;
    }

    pub fn str(in: *const Interner, i: u32) []const u8 {
        return in.strs.items[i];
    }
};

pub const NInsn = struct {
    addr: u32,
    masked: []const u8,
    mnemonic: u32, // interned
    mn_text: []const u8,
    key: u32, // interned: the text with function and global references neutralised
    tokens: []const Token,
    /// Interned display text (references by name) and our source line (/Z7, 0 for none):
    /// only with Viewer.with_text, for the listings (side_by_side, our_view's text).
    text: u32 = 0,
    line: u32 = 0,
};

/// (field offset, size, token, is_branch)
pub const Field = struct {
    off: u32,
    size: u32,
    tok: Token,
    branch: bool,

    fn lessThan(_: void, a: Field, b: Field) bool {
        if (a.off != b.off) return a.off < b.off;
        if (a.size != b.size) return a.size < b.size;
        const o = a.tok.order(&b.tok);
        if (o != .eq) return o == .lt;
        return @intFromBool(a.branch) < @intFromBool(b.branch);
    }
};

// Mnemonics whose operand capstone prints relative to the instruction's own address.
const RELATIVE = [_][]const u8{ "call", "jmp", "xbegin", "jo", "jno", "jb", "jae", "je", "jne", "jbe", "ja", "js", "jns", "jp", "jnp", "jl", "jge", "jle", "jg", "jecxz", "jcxz", "loop", "loope", "loopne" };

fn isRelative(m: []const u8) bool {
    for (RELATIVE) |r| if (std.mem.eql(u8, m, r)) return true;
    return false;
}

pub const Viewer = struct {
    d: *dec.Decoder,
    intern: Interner = .{},
    /// Also the display text and our source lines (compare.py's render=True for a listing);
    /// the audit compares by the key alone.
    with_text: bool = false,

    /// _render: disassemble with each reference field replaced by a unique sentinel, then
    /// swap the printed sentinel for the reference's label (text) or neutral name (key).
    fn render(v: *Viewer, raw: []const u8, addr: u32, fields: []const Field) !struct { u32, u32 } {
        var b: [16]u8 = undefined;
        @memcpy(b[0..raw.len], raw);
        const Sub = struct { val: u32, tok: *const Token };
        var subs: std.ArrayList(Sub) = .empty;
        for (fields, 0..) |*fl, k| {
            if (fl.size != 4 or fl.off + 4 > raw.len) continue;
            const val = SENTINEL + 16 * @as(u32, @intCast(k));
            const w = if (fl.branch) val -% (addr +% @as(u32, @intCast(raw.len))) else val;
            std.mem.writeInt(u32, b[fl.off..][0..4], w, .little);
            try subs.append(gpa, .{ .val = val, .tok = &fl.tok });
        }
        var buf: [256]u8 = undefined;
        var key: []const u8 = v.d.render(b[0..raw.len], addr, &buf) orelse "(bad)";
        var txt = key;
        for (subs.items) |s| {
            var needle: [16]u8 = undefined;
            const nd = try std.fmt.bufPrint(&needle, "0x{x}", .{s.val});
            if (v.with_text) {
                var lab: std.ArrayList(u8) = .empty;
                try label(&lab, s.tok);
                txt = try std.mem.replaceOwned(u8, gpa, txt, nd, lab.items);
            }
            var rep: std.ArrayList(u8) = .empty;
            try neutral(&rep, s.tok);
            key = try std.mem.replaceOwned(u8, gpa, key, nd, rep.items);
        }
        const kid = try v.intern.id(key);
        return .{ if (v.with_text) try v.intern.id(txt) else kid, kid };
    }

    /// _text: an instruction with no reference field keys as it decoded, unless it is
    /// rendered at another address and its operand is relative to that address.
    /// (text, key).
    fn text(v: *Viewer, i: *const dec.Insn, addr: u32, fields: []const Field) !struct { u32, u32 } {
        if (fields.len == 0 and (addr == i.addr or !isRelative(i.mn))) {
            var buf: [256]u8 = undefined;
            const s = try std.fmt.bufPrint(&buf, "{s} {s}", .{ i.mn, i.op });
            const id = try v.intern.id(util.strip(s));
            return .{ id, id };
        }
        return v.render(i.code(), addr, fields);
    }

    fn finish(v: *Viewer, i: *const dec.Insn, addr: u32, fields: []Field) !NInsn {
        const masked = try gpa.dupe(u8, i.code());
        for (fields) |fl| @memset(masked[fl.off .. fl.off + fl.size], 0);
        const toks = try gpa.alloc(Token, fields.len);
        for (fields, 0..) |fl, k| toks[k] = fl.tok;
        const txt, const kid = try v.text(i, addr, fields);
        return .{ .addr = addr, .masked = masked, .mnemonic = try v.intern.id(i.mn), .mn_text = i.mn, .key = kid, .tokens = toks, .text = txt };
    }

    /// our_view: f is ours, syms our symbol -> original start (the funcmap).
    pub fn ourView(v: *Viewer, of: *const ours.OurFunc, syms: anytype) ![]NInsn {
        const f = &of.f;
        const ByOff = struct { kind: objfuncs.Kind, tok: Token };
        var by_off: std.AutoHashMapUnmanaged(u32, ByOff) = .empty;
        for (f.refs) |r| {
            const tok: Token = switch (r.kind) {
                .call, .fptr => if (try syms.get(r.s)) |s| .{ .kind = .fn_, .int = s } else .{ .kind = .fnq, .s = r.s },
                .string => blk: {
                    // The original names a string by the bytes at the address, so a reference
                    // into the middle of a literal ("Amazon"+4) is "on".
                    const add = try addend(f, r.off);
                    break :blk .{ .kind = .str, .s = if (0 < add and add < r.s.len) r.s[@intCast(add)..] else r.s };
                },
                .import => .{ .kind = .import, .s = r.s },
                .self => .{ .kind = .self, .int = r.int },
                else => .{ .kind = .data_sym, .s = r.s, .add = try addend(f, r.off) },
            };
            try by_off.put(gpa, r.off, .{ .kind = r.kind, .tok = tok });
        }
        const out = try gpa.alloc(NInsn, f.decoded.insns.items.len);
        for (f.decoded.insns.items, 0..) |*i, n| {
            var fields: std.ArrayList(Field) = .empty;
            var off: i64 = @as(i64, i.addr) - 3;
            while (off < @as(i64, i.addr) + i.size) : (off += 1) {
                if (off < 0) continue;
                const o: u32 = @intCast(off);
                const e = by_off.get(o) orelse continue;
                var tok = e.tok;
                if (o < i.addr or o + 4 > i.addr + i.size) {
                    // A relocation straddling an instruction boundary: these bytes are not
                    // code. Mask what overlaps, and say so.
                    const inner = try gpa.create(Token);
                    inner.* = e.tok;
                    tok = .{ .kind = .straddle, .inner = inner };
                }
                const lo = @max(o, i.addr);
                try fields.append(gpa, .{ .off = lo - i.addr, .size = @min(4, i.addr + i.size - lo), .tok = tok, .branch = e.kind == .call });
            }
            out[n] = try v.finish(i, i.addr, fields.items);
            if (v.with_text) if (of.lineAt(i.addr)) |l| {
                out[n].line = l.line;
            };
        }
        return out;
    }

    /// ref_view of the original's code from `start` to `end`, where the carve may have split
    /// one function into several (ref_view_span); with end == the carve's end, ref_view.
    pub fn refViewSpan(v: *Viewer, start: u32, end: u32, model: *const Model) ![]NInsn {
        const pieces = (try spanPieces(model, start, end)) orelse return error.NotARun;
        var out: std.ArrayList(NInsn) = .empty;
        const single = pieces.len == 1;
        for (pieces) |p| {
            const e = model.getPtr(p).?;
            var calls: std.AutoHashMapUnmanaged(u32, void) = .empty;
            if (!single) for (e.f.insns.items) |i| if (i.bytes[0] == 0xE8) try calls.put(gpa, i.addr, {});
            var per_insn: std.AutoArrayHashMapUnmanaged(u32, std.ArrayList(Field)) = .empty;
            for (e.refs) |r| {
                var tok: Token = undefined;
                if (single) {
                    tok = switch (r.kind) {
                        .call, .fptr => .{ .kind = .fn_, .int = r.int },
                        .string => .{ .kind = .str, .s = r.bytes },
                        .import => .{ .kind = .import, .s = try importName(r.bytes) },
                        .self => .{ .kind = .self, .int = r.int },
                        .data => .{ .kind = .data_addr, .int = r.int },
                    };
                } else {
                    const inside = start <= r.int and r.int < end;
                    if (r.kind == .call and inside and !(r.int == start and calls.contains(r.a))) continue; // a branch inside the span: raw bytes
                    tok = switch (r.kind) {
                        .self => .{ .kind = .self, .int = @as(u64, r.int) + p - start },
                        .call => .{ .kind = .fn_, .int = r.int },
                        .fptr => if (inside) .{ .kind = .self, .int = r.int - start } else .{ .kind = .fn_, .int = r.int },
                        .string => .{ .kind = .str, .s = r.bytes },
                        .import => .{ .kind = .import, .s = try importName(r.bytes) },
                        .data => if (inside) .{ .kind = .self, .int = r.int - start } else .{ .kind = .data_addr, .int = r.int },
                    };
                }
                const g = try per_insn.getOrPut(gpa, r.a);
                if (!g.found_existing) g.value_ptr.* = .empty;
                try g.value_ptr.append(gpa, .{ .off = r.off, .size = r.size, .tok = tok, .branch = r.kind == .call });
            }
            for (e.f.insns.items) |*i| {
                const fields: []Field = if (per_insn.getPtr(i.addr)) |l| l.items else &.{};
                std.mem.sort(Field, fields, {}, Field.lessThan);
                // Rendered at its offset in the function, as our side is, so an internal jump
                // reads the same on both sides.
                try out.append(gpa, try v.finish(i, i.addr - start, fields));
            }
        }
        return out.items;
    }
};

pub const Model = std.AutoHashMapUnmanaged(u32, refimage.ModelEntry);

/// The carved functions that tile [start, end) exactly, in order, or null.
pub fn spanPieces(model: *const Model, start: u32, end: u32) !?[]u32 {
    var out: std.ArrayList(u32) = .empty;
    var p = start;
    while (p < end) {
        const e = model.getPtr(p) orelse return null;
        try out.append(gpa, p);
        p = e.f.end;
    }
    return if (p == end) out.items else null;
}

/// The value a DIR32 field holds in our object: the offset from its symbol.
fn addend(f: *const objfuncs.Function, off: u32) !i32 {
    for (f.decoded.insns.items) |*i| {
        if (i.addr <= off and off < i.addr + i.size) {
            const k = off - i.addr;
            if (k + 4 > i.size) return error.AddendPastInstruction; // struct.error in Python
            return std.mem.readInt(i32, i.bytes[k..][0..4], .little);
        }
    }
    return 0;
}

// ws2_32 exports its classic Winsock API by ordinal, and Game.exe imports it so.
const WS2_32 = [_]struct { u32, []const u8 }{
    .{ 1, "accept" },            .{ 2, "bind" },              .{ 3, "closesocket" },    .{ 4, "connect" },        .{ 5, "getpeername" },
    .{ 6, "getsockname" },       .{ 7, "getsockopt" },        .{ 8, "htonl" },          .{ 9, "htons" },          .{ 10, "ioctlsocket" },
    .{ 11, "inet_addr" },        .{ 12, "inet_ntoa" },        .{ 13, "listen" },        .{ 14, "ntohl" },         .{ 15, "ntohs" },
    .{ 16, "recv" },             .{ 17, "recvfrom" },         .{ 18, "select" },        .{ 19, "send" },          .{ 20, "sendto" },
    .{ 21, "setsockopt" },       .{ 22, "shutdown" },         .{ 23, "socket" },        .{ 51, "gethostbyaddr" }, .{ 52, "gethostbyname" },
    .{ 53, "getprotobyname" },   .{ 54, "getprotobynumber" }, .{ 55, "getservbyname" }, .{ 56, "getservbyport" }, .{ 57, "gethostname" },
    .{ 111, "WSAGetLastError" }, .{ 112, "WSASetLastError" }, .{ 115, "WSAStartup" },   .{ 116, "WSACleanup" },   .{ 151, "__WSAFDIsSet" },
};

fn isWord(ch: u8) bool {
    return std.ascii.isAlphanumeric(ch) or ch == '_';
}

/// 'dll!name' as our objects name the same import: a C name without its leading underscore
/// and stdcall decoration, and a ws2_32 ordinal as the function it is.
pub fn importName(tgt: []const u8) ![]const u8 {
    const bang = std.mem.indexOfScalar(u8, tgt, '!') orelse return error.BadImport;
    const dll = tgt[0..bang];
    const name = tgt[bang + 1 ..];
    if (name.len > 0 and name[0] == '#' and std.ascii.eqlIgnoreCase(dll, "ws2_32.dll")) {
        const n = std.fmt.parseInt(u32, name[1..], 10) catch return error.BadOrdinal;
        for (WS2_32) |e| if (e[0] == n) return e[1];
        return name;
    }
    // ^_([A-Za-z_]\w*)@\d+$
    if (name.len >= 4 and name[0] == '_' and (std.ascii.isAlphabetic(name[1]) or name[1] == '_')) {
        if (std.mem.lastIndexOfScalar(u8, name, '@')) |at| if (at >= 2) {
            var ok = at + 1 < name.len;
            for (name[at + 1 ..]) |ch| if (!std.ascii.isDigit(ch)) {
                ok = false;
            };
            for (name[2..at]) |ch| if (!isWord(ch)) {
                ok = false;
            };
            if (ok) return name[1..at];
        };
    }
    return name;
}

pub const Wire = struct { sym: []const u8, addr: u32 };

pub const Result = struct {
    status: []const u8 = "",
    ref_insns: usize = 0,
    our_insns: usize = 0,
    byte_diffs: usize = 0,
    similarity: f64 = 0,
    ref_diffs: usize = 0,
    unverified: usize = 0,
    ident_bytes: usize = 0, // original's bytes in instructions identical to an aligned one of ours
    operand_bytes: usize = 0, // ... aligned with one of the same mnemonic, other operands
    wiring: []Wire = &.{}, // (our data symbol, original address it implies) at aligned positions
};

/// True/False when comparable, null when not (data, an unmapped callee, a straddle).
fn tokensAgree(a: *const Token, b: *const Token) ?bool {
    if (a.isData() or a.kind == .fnq or a.kind == .straddle or b.isData() or b.kind == .fnq or b.kind == .straddle) return null;
    return a.eql(b);
}

fn identical(x: *const NInsn, y: *const NInsn) bool {
    if (!std.mem.eql(u8, x.masked, y.masked) or x.tokens.len != y.tokens.len) return false;
    for (x.tokens, y.tokens) |*p, *q| if (tokensAgree(p, q) == false) return false;
    return true;
}

fn ids(xs: []const NInsn, comptime field: []const u8) ![]u32 {
    const out = try gpa.alloc(u32, xs.len);
    for (xs, 0..) |x, k| out[k] = @field(x, field);
    return out;
}

/// (ident, operand) - compare.aligned_bytes.
fn alignedBytes(ours_v: []const NInsn, ref: []const NInsn, sm: *difflib.SequenceMatcher) !struct { usize, usize } {
    var ident: usize = 0;
    var operand: usize = 0;
    for (try sm.opcodes()) |op| {
        switch (op.tag) {
            .equal => for (ours_v[op.ci..op.ci2], ref[op.cj..op.cj2]) |*x, *y| {
                if (identical(x, y)) ident += y.masked.len else operand += y.masked.len;
            },
            .replace => {
                const xs = ours_v[op.ci..op.ci2];
                const ys = ref[op.cj..op.cj2];
                var inner = try difflib.SequenceMatcher.init(gpa, try ids(xs, "mnemonic"), try ids(ys, "mnemonic"));
                defer inner.deinit();
                for (try inner.matchingBlocks()) |m| {
                    for (xs[m.a .. m.a + m.size], ys[m.b .. m.b + m.size]) |*x, *y| {
                        if (identical(x, y)) ident += y.masked.len else operand += y.masked.len;
                    }
                }
            },
            else => {},
        }
    }
    return .{ ident, operand };
}

pub fn compare(ours_v: []const NInsn, ref: []const NInsn) !Result {
    var r = Result{ .ref_insns = ref.len, .our_insns = ours_v.len };
    var sm = try difflib.SequenceMatcher.init(gpa, try ids(ours_v, "key"), try ids(ref, "key"));
    defer sm.deinit();
    r.similarity = if (ours_v.len > 0 or ref.len > 0) try sm.ratio() else 1.0;
    r.ident_bytes, r.operand_bytes = try alignedBytes(ours_v, ref, &sm);
    if (ours_v.len != ref.len) {
        const d = @as(i64, @intCast(ours_v.len)) - @as(i64, @intCast(ref.len));
        r.status = if (d > 0) try std.fmt.allocPrint(gpa, "OUR+{d}", .{d}) else try std.fmt.allocPrint(gpa, "OUR-{d}", .{-d});
        return r;
    }
    var same_bytes = true;
    var same_mn = true;
    var wiring: std.ArrayList(Wire) = .empty;
    for (ours_v, ref) |*x, *y| {
        if (x.mnemonic != y.mnemonic) same_mn = false;
        if (!std.mem.eql(u8, x.masked, y.masked)) {
            same_bytes = false;
            if (x.masked.len == y.masked.len) {
                for (x.masked, y.masked) |p, q| {
                    if (p != q) r.byte_diffs += 1;
                }
            } else r.byte_diffs += @max(x.masked.len, y.masked.len);
        }
        const n = @min(x.tokens.len, y.tokens.len);
        for (x.tokens[0..n], y.tokens[0..n]) |*ta, *tb| {
            if (ta.kind == .data_sym and tb.isData() and x.mnemonic == y.mnemonic) {
                if (tb.kind != .data_addr) return error.WiringOnOurSide;
                try wiring.append(gpa, .{ .sym = ta.s, .addr = @truncate(tb.int -% @as(u64, @bitCast(@as(i64, ta.add)))) });
            }
            if (tokensAgree(ta, tb)) |ok| {
                if (!ok) r.ref_diffs += 1;
            } else r.unverified += 1;
        }
        if (x.tokens.len != y.tokens.len) r.ref_diffs += @max(x.tokens.len, y.tokens.len) - n;
    }
    r.wiring = wiring.items;
    if (same_bytes) {
        r.status = if (r.ref_diffs == 0) "MATCH" else "REF_DIFF";
    } else if (same_mn) {
        r.status = "INSN_MATCH";
    } else r.status = "INSN_COUNT_MATCH";
    return r;
}

/// compare.side_by_side: lines of a diff of the two listings, aligned on the neutral keys
/// (Viewer.with_text). Marker: ' ' same bytes and every comparable reference agrees; '!' same
/// bytes, a reference names something else; '~' same instruction, other bytes; '|' different
/// instructions; '<' only ours; '>' only the original's. Latin-1 bytes, as the views hold them.
pub fn sideBySide(v: *const Viewer, ours_v: []const NInsn, ref: []const NInsn, width: usize) ![][]const u8 {
    var sm = try difflib.SequenceMatcher.init(gpa, try ids(ours_v, "key"), try ids(ref, "key"));
    defer sm.deinit();
    var out: std.ArrayList([]const u8) = .empty;
    for (try sm.opcodes()) |op| {
        const n = @max(op.ci2 - op.ci, op.cj2 - op.cj);
        for (0..n) |k| {
            const x: ?*const NInsn = if (op.ci + k < op.ci2) &ours_v[op.ci + k] else null;
            const y: ?*const NInsn = if (op.cj + k < op.cj2) &ref[op.cj + k] else null;
            var mark: u8 = undefined;
            if (x == null) {
                mark = '>';
            } else if (y == null) {
                mark = '<';
            } else if (std.mem.eql(u8, x.?.masked, y.?.masked)) {
                var disagree = x.?.tokens.len != y.?.tokens.len;
                const m = @min(x.?.tokens.len, y.?.tokens.len);
                for (x.?.tokens[0..m], y.?.tokens[0..m]) |*p, *q| if (tokensAgree(p, q) == false) {
                    disagree = true;
                };
                mark = if (disagree) '!' else ' ';
            } else if (op.tag == .equal) {
                mark = '~';
            } else mark = '|';
            var left: std.ArrayList(u8) = .empty;
            if (x) |xi| {
                try left.print(gpa, "{x:>5} ", .{xi.addr});
                if (xi.line != 0) try left.print(gpa, "{d:>5}", .{xi.line}) else try left.appendSlice(gpa, "     ");
                try left.print(gpa, " {s}", .{v.intern.str(xi.text)});
            }
            var line: std.ArrayList(u8) = .empty;
            const l = left.items[0..@min(width, left.items.len)];
            try line.appendSlice(gpa, l);
            for (l.len..width) |_| try line.append(gpa, ' ');
            try line.print(gpa, " {c} ", .{mark});
            if (y) |yi| try line.print(gpa, "{x:>5}  {s}", .{ yi.addr, v.intern.str(yi.text) });
            try out.append(gpa, line.items);
        }
    }
    return out.items;
}

/// compare.differing_lines: our source lines of the instructions that differ, in order.
pub fn differingLines(ours_v: []const NInsn, ref: []const NInsn) ![]u32 {
    var sm = try difflib.SequenceMatcher.init(gpa, try ids(ours_v, "key"), try ids(ref, "key"));
    defer sm.deinit();
    var lines: std.ArrayList(u32) = .empty;
    for (try sm.opcodes()) |op| {
        for (op.ci..op.ci2) |k| {
            const x = &ours_v[k];
            const differs = op.tag != .equal or !std.mem.eql(u8, x.masked, ref[op.cj + (k - op.ci)].masked);
            if (differs and x.line != 0 and std.mem.indexOfScalar(u32, lines.items, x.line) == null) try lines.append(gpa, x.line);
        }
        if (op.tag == .insert and op.ci > 0) {
            const ln = ours_v[op.ci - 1].line;
            if (ln != 0 and std.mem.indexOfScalar(u32, lines.items, ln) == null) try lines.append(gpa, ln);
        }
    }
    return lines.items;
}
