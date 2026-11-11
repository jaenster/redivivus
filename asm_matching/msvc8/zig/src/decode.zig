//! One function by recursive descent (funcdecode.py), the same way on both sides of a
//! comparison. The caller says what the bytes mean through a context:
//!
//!   ctx.relocAt(addr) ?u32    the address an absolute field at addr holds (a base relocation in
//!                             the image, a same-section DIR32 in an object), else null
//!   ctx.externalAt(addr) bool the rel32 field at addr is relocated against another symbol
//!                             (in an object a jmp to another function holds 0)
//!
//! Following every branch, rather than sweeping linearly, keeps MSVC's in-.text switch tables
//! and the int3 padding out of the instruction stream.
const std = @import("std");
pub const c = @import("c");

pub const Kind = enum { stop, call, jmp, jcc, other };

pub const Insn = struct {
    addr: u32,
    size: u8,
    kind: Kind,
    bytes: [16]u8,
    /// Direct branch/call destination.
    target: ?u32 = null,
    /// Jump table address, for jmp [table + reg*4].
    table: ?u32 = null,
    /// Capstone's text; empty unless the decoder keeps text.
    mn: []const u8 = "",
    op: []const u8 = "",

    pub fn code(i: *const Insn) []const u8 {
        return i.bytes[0..i.size];
    }

    /// The instruction ends the function on its own (ret, jmp, int3, ud2, hlt).
    pub fn terminates(i: Insn) bool {
        return i.kind == .stop or i.kind == .jmp;
    }
};

pub const Table = struct { addr: u32, n: u32 };
pub const IndexTable = struct { addr: u32, len: u32 };
pub const Call = struct { at: u32, t: u32 };
/// jmp/jcc leaving [start, limit): target null for a relocated (external) rel32.
pub const Exit = struct { at: u32, t: ?u32 };

pub const Function = struct {
    start: u32,
    insns: std.ArrayList(Insn) = .empty, // in address order
    tables: std.ArrayList(Table) = .empty, // dword jump tables
    index_tables: std.ArrayList(IndexTable) = .empty, // byte index tables
    calls: std.ArrayList(Call) = .empty,
    exits: std.ArrayList(Exit) = .empty,
    end: u32, // one past the last instruction or table byte
    overrun: bool = false, // descent wanted to run past `limit`

    pub fn deinit(f: *Function, gpa: std.mem.Allocator) void {
        f.insns.deinit(gpa);
        f.tables.deinit(gpa);
        f.index_tables.deinit(gpa);
        f.calls.deinit(gpa);
        f.exits.deinit(gpa);
    }

    /// Merge another descent of the same bytes (an extra root: handler, funclet): its
    /// instructions not yet here, its tables, and its end. Call `sortInsns` after the last.
    pub fn mergeRoot(f: *Function, gpa: std.mem.Allocator, g: *const Function, with_index_and_calls: bool) !void {
        var have: std.AutoHashMapUnmanaged(u32, void) = .empty;
        defer have.deinit(gpa);
        for (f.insns.items) |i| try have.put(gpa, i.addr, {});
        for (g.insns.items) |i| if (!have.contains(i.addr)) try f.insns.append(gpa, i);
        try f.tables.appendSlice(gpa, g.tables.items);
        if (with_index_and_calls) {
            try f.index_tables.appendSlice(gpa, g.index_tables.items);
            try f.calls.appendSlice(gpa, g.calls.items);
        }
        f.end = @max(f.end, g.end);
    }

    pub fn sortInsns(f: *Function) void {
        std.mem.sort(Insn, f.insns.items, {}, struct {
            fn lt(_: void, x: Insn, y: Insn) bool {
                return x.addr < y.addr;
            }
        }.lt);
    }
};

const COND = [_][]const u8{ "jo", "jno", "jb", "jae", "je", "jne", "jbe", "ja", "js", "jns", "jp", "jnp", "jl", "jge", "jle", "jg", "jecxz", "jcxz", "loop", "loope", "loopne" };

pub fn isCond(m: []const u8) bool {
    for (COND) |cc| if (std.mem.eql(u8, m, cc)) return true;
    return false;
}

fn eql(m: []const u8, s: []const u8) bool {
    return std.mem.eql(u8, m, s);
}

pub const Decoder = struct {
    h: c.csh,
    insn: [*c]c.cs_insn,
    /// Kept text (mnemonic, operands) goes here; null keeps none.
    text: ?std.mem.Allocator = null,

    pub fn init(text: ?std.mem.Allocator) !Decoder {
        var h: c.csh = 0;
        if (c.cs_open(c.CS_ARCH_X86, c.CS_MODE_32, &h) != c.CS_ERR_OK) return error.Capstone;
        _ = c.cs_option(h, c.CS_OPT_DETAIL, c.CS_OPT_ON);
        return .{ .h = h, .insn = c.cs_malloc(h), .text = text };
    }

    /// One instruction of `bytes` at `addr` as capstone prints it ("mnemonic op_str", stripped),
    /// or null when it does not decode. Into buf.
    pub fn render(d: *Decoder, bytes: []const u8, addr: u64, buf: []u8) ?[]const u8 {
        var p: [*c]const u8 = bytes.ptr;
        var n: usize = bytes.len;
        var a: u64 = addr;
        if (!c.cs_disasm_iter(d.h, &p, &n, &a, d.insn)) return null;
        const m = std.mem.sliceTo(&d.insn.*.mnemonic, 0);
        const o = std.mem.sliceTo(&d.insn.*.op_str, 0);
        const s = std.fmt.bufPrint(buf, "{s} {s}", .{ m, o }) catch return null;
        return std.mem.trim(u8, s, " \t\n\r\x0b\x0c");
    }

    /// decode_function(code, base, start, limit, reloc_at, noreturn, external_at).
    pub fn decode(d: *Decoder, gpa: std.mem.Allocator, code: []const u8, base: u32, start: u32, limit: u32, ctx: anytype, no_return: ?*const std.AutoHashMapUnmanaged(u32, void)) !Function {
        var f = Function{ .start = start, .end = start };
        var seen: std.AutoArrayHashMapUnmanaged(u32, Insn) = .empty;
        defer seen.deinit(gpa);
        var work: std.ArrayList(u32) = .empty;
        defer work.deinit(gpa);
        var idx: std.ArrayList(u32) = .empty;
        defer idx.deinit(gpa);
        try work.append(gpa, start);
        while (work.pop()) |a0| {
            var a = a0;
            while (start <= a and a < limit and !seen.contains(a)) {
                const off = a - base;
                var p: [*c]const u8 = code.ptr + off;
                var n: usize = @min(16, code.len - off);
                var addr: u64 = a;
                if (n == 0 or !c.cs_disasm_iter(d.h, &p, &n, &addr, d.insn)) {
                    f.overrun = true;
                    break;
                }
                const ins = d.insn.*;
                const m = std.mem.sliceTo(&ins.mnemonic, 0);
                var it = Insn{ .addr = a, .size = @intCast(ins.size), .kind = .other, .bytes = undefined };
                @memcpy(it.bytes[0..ins.size], ins.bytes[0..ins.size]);
                if (d.text) |ta| {
                    it.mn = try ta.dupe(u8, m);
                    it.op = try ta.dupe(u8, std.mem.sliceTo(&ins.op_str, 0));
                }
                const nxt = a + ins.size;
                if (nxt > limit) f.overrun = true;
                const x = ins.detail.*.unnamed_0.x86;
                if (std.mem.startsWith(u8, m, "ret") or eql(m, "int3") or eql(m, "ud2") or eql(m, "hlt")) {
                    it.kind = .stop;
                    try seen.put(gpa, a, it);
                    break;
                }
                if (eql(m, "call")) {
                    it.kind = .call;
                    var stop = false;
                    if (x.op_count > 0 and x.operands[0].type == c.X86_OP_IMM) {
                        const t: u32 = @truncate(@as(u64, @bitCast(x.operands[0].unnamed_0.imm)));
                        it.target = t;
                        try f.calls.append(gpa, .{ .at = a, .t = t });
                        if (no_return) |nr| stop = nr.contains(t);
                    }
                    try seen.put(gpa, a, it);
                    if (stop) break;
                    a = nxt;
                    continue;
                }
                const is_cond = isCond(m);
                if (eql(m, "jmp") or is_cond) {
                    it.kind = if (is_cond) .jcc else .jmp;
                    // Recorded before its table is read: the table scan stops at a decoded address.
                    try seen.put(gpa, a, it);
                    if (x.op_count > 0 and x.operands[0].type == c.X86_OP_IMM) {
                        const t: u32 = @truncate(@as(u64, @bitCast(x.operands[0].unnamed_0.imm)));
                        seen.getPtr(a).?.target = t;
                        if (ins.size >= 5 and ctx.externalAt(nxt - 4)) {
                            try f.exits.append(gpa, .{ .at = a, .t = null });
                        } else if (start <= t and t < limit) {
                            try work.append(gpa, t);
                        } else try f.exits.append(gpa, .{ .at = a, .t = t });
                    } else if (!is_cond and x.op_count > 0 and x.operands[0].type == c.X86_OP_MEM) {
                        const mem = x.operands[0].unnamed_0.mem;
                        // The table's address through the relocation on the displacement: in an
                        // object the field holds only the addend against a $LN label.
                        const tbl = ctx.relocAt(a + x.encoding.disp_offset) orelse @as(u32, @truncate(@as(u64, @bitCast(mem.disp))));
                        if (mem.index != 0 and mem.scale == 4 and start <= tbl and tbl < limit) {
                            seen.getPtr(a).?.table = tbl;
                            var q = tbl;
                            var cnt: u32 = 0;
                            while (q + 4 <= limit and !seen.contains(q)) : (q += 4) {
                                const t = ctx.relocAt(q) orelse break;
                                if (!(start <= t and t < limit)) break;
                                try work.append(gpa, t);
                                cnt += 1;
                            }
                            try f.tables.append(gpa, .{ .addr = tbl, .n = cnt });
                        }
                    }
                    if (!is_cond) break;
                    a = nxt;
                    continue;
                }
                // movzx reg, byte ptr [reg + idx_table]: the byte index of a sparse switch
                if (eql(m, "movzx") and x.op_count == 2 and x.operands[1].type == c.X86_OP_MEM and x.operands[1].size == 1) {
                    if (ctx.relocAt(a + x.encoding.disp_offset)) |disp| {
                        if (start <= disp and disp < limit and x.operands[1].unnamed_0.mem.base != 0) try idx.append(gpa, disp);
                    }
                }
                try seen.put(gpa, a, it);
                a = nxt;
            }
        }
        try f.insns.appendSlice(gpa, seen.values());
        f.sortInsns();
        for (f.insns.items) |i| f.end = @max(f.end, i.addr + i.size);
        for (f.tables.items) |t| f.end = @max(f.end, t.addr + 4 * t.n);
        // A byte index table runs to the next instruction, table, or the limit, up to padding.
        for (idx.items) |disp| {
            var stop = limit;
            for (f.insns.items) |i| if (i.addr > disp) {
                stop = @min(stop, i.addr);
                break;
            };
            for (f.tables.items) |t| if (t.addr > disp) {
                stop = @min(stop, t.addr);
            };
            var n: u32 = 0;
            while (disp + n < stop and code[disp + n - base] != 0xCC) n += 1;
            try f.index_tables.append(gpa, .{ .addr = disp, .len = n });
            f.end = @max(f.end, disp + n);
        }
        return f;
    }
};
