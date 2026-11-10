//! One function by recursive descent, as funcdecode.py decodes it.
const std = @import("std");
pub const c = @import("c");

pub const Kind = enum { stop, call, jmp, jcc, movzx, other };

pub const Insn = struct {
    addr: u32,
    size: u8,
    kind: Kind,
    /// First byte and the last four bytes' field offset, for the immediates the carve reads.
    bytes: [16]u8,
};

pub const Table = struct { addr: u32, n: u32 };

pub const Function = struct {
    insns: std.ArrayList(Insn) = .empty,
    tables: std.ArrayList(Table) = .empty,
    calls: std.ArrayList(u32) = .empty, // direct call targets
    /// jmp/jcc leaving [start, limit): (insn addr, target); target null for an external rel32.
    exits: std.ArrayList(struct { at: u32, t: ?u32 }) = .empty,
    end: u32,

    pub fn deinit(f: *Function, gpa: std.mem.Allocator) void {
        f.insns.deinit(gpa);
        f.tables.deinit(gpa);
        f.calls.deinit(gpa);
        f.exits.deinit(gpa);
    }

    /// The last instruction ends the function on its own (ret, jmp, int3, ud2, hlt).
    pub fn terminates(i: Insn) bool {
        return i.kind == .stop or i.kind == .jmp;
    }
};

const COND = [_][]const u8{ "jo", "jno", "jb", "jae", "je", "jne", "jbe", "ja", "js", "jns", "jp", "jnp", "jl", "jge", "jle", "jg", "jecxz", "jcxz", "loop", "loope", "loopne" };

pub const Decoder = struct {
    h: c.csh,
    insn: [*c]c.cs_insn,
    code: []const u8,
    base: u32,
    relocs: *const std.AutoHashMapUnmanaged(u32, void),
    /// Reads a u32 outside `code` (pe.u32), for relocated fields in data.
    outer: *const fn (ctx: *const anyopaque, va: u32) ?u32,
    outer_ctx: *const anyopaque,

    pub fn init(code: []const u8, base: u32, relocs: *const std.AutoHashMapUnmanaged(u32, void), outer: *const fn (*const anyopaque, u32) ?u32, ctx: *const anyopaque) !Decoder {
        var h: c.csh = 0;
        if (c.cs_open(c.CS_ARCH_X86, c.CS_MODE_32, &h) != c.CS_ERR_OK) return error.Capstone;
        _ = c.cs_option(h, c.CS_OPT_DETAIL, c.CS_OPT_ON);
        return .{ .h = h, .insn = c.cs_malloc(h), .code = code, .base = base, .relocs = relocs, .outer = outer, .outer_ctx = ctx };
    }

    pub fn inCode(d: *const Decoder, a: u32) bool {
        return a >= d.base and a - d.base < d.code.len;
    }

    pub fn relocAt(d: *const Decoder, a: u32) ?u32 {
        if (!d.relocs.contains(a)) return null;
        if (d.inCode(a) and a - d.base + 4 <= d.code.len) return std.mem.readInt(u32, d.code[a - d.base ..][0..4], .little);
        return d.outer(d.outer_ctx, a);
    }

    fn eql(m: []const u8, s: []const u8) bool {
        return std.mem.eql(u8, m, s);
    }

    pub fn decode(d: *Decoder, gpa: std.mem.Allocator, start: u32, limit: u32) !Function {
        var f = Function{ .end = start };
        var seen: std.AutoArrayHashMapUnmanaged(u32, Insn) = .empty;
        defer seen.deinit(gpa);
        var work: std.ArrayList(u32) = .empty;
        defer work.deinit(gpa);
        var idx_tables: std.ArrayList(u32) = .empty;
        defer idx_tables.deinit(gpa);
        try work.append(gpa, start);
        while (work.pop()) |a0| {
            var a = a0;
            while (start <= a and a < limit and !seen.contains(a)) {
                const off = a - d.base;
                var p: [*c]const u8 = d.code.ptr + off;
                var n: usize = @min(16, d.code.len - off);
                var addr: u64 = a;
                if (!c.cs_disasm_iter(d.h, &p, &n, &addr, d.insn)) break;
                const ins = d.insn.*;
                const m = std.mem.sliceTo(&ins.mnemonic, 0);
                var it = Insn{ .addr = a, .size = @intCast(ins.size), .kind = .other, .bytes = undefined };
                @memcpy(it.bytes[0..ins.size], ins.bytes[0..ins.size]);
                const nxt = a + ins.size;
                const x = ins.detail.*.unnamed_0.x86;
                if (std.mem.startsWith(u8, m, "ret") or eql(m, "int3") or eql(m, "ud2") or eql(m, "hlt")) {
                    it.kind = .stop;
                    try seen.put(gpa, a, it);
                    break;
                }
                if (eql(m, "call")) {
                    it.kind = .call;
                    try seen.put(gpa, a, it);
                    if (x.op_count > 0 and x.operands[0].type == c.X86_OP_IMM) try f.calls.append(gpa, @truncate(@as(u64, @bitCast(x.operands[0].unnamed_0.imm))));
                    a = nxt;
                    continue;
                }
                var is_cond = false;
                for (COND) |cc| if (eql(m, cc)) {
                    is_cond = true;
                };
                if (eql(m, "jmp") or is_cond) {
                    it.kind = if (is_cond) .jcc else .jmp;
                    try seen.put(gpa, a, it);
                    if (x.op_count > 0 and x.operands[0].type == c.X86_OP_IMM) {
                        const t: u32 = @truncate(@as(u64, @bitCast(x.operands[0].unnamed_0.imm)));
                        if (start <= t and t < limit) try work.append(gpa, t) else try f.exits.append(gpa, .{ .at = a, .t = t });
                    } else if (!is_cond and x.op_count > 0 and x.operands[0].type == c.X86_OP_MEM) {
                        const mem = x.operands[0].unnamed_0.mem;
                        const tbl = d.relocAt(a + x.encoding.disp_offset) orelse @as(u32, @truncate(@as(u64, @bitCast(mem.disp))));
                        if (mem.index != 0 and mem.scale == 4 and start <= tbl and tbl < limit) {
                            var q = tbl;
                            var cnt: u32 = 0;
                            while (q + 4 <= limit and !seen.contains(q)) : (q += 4) {
                                const t = d.relocAt(q) orelse break;
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
                if (eql(m, "movzx") and x.op_count == 2 and x.operands[1].type == c.X86_OP_MEM and x.operands[1].size == 1) {
                    if (d.relocAt(a + x.encoding.disp_offset)) |disp| {
                        if (start <= disp and disp < limit and x.operands[1].unnamed_0.mem.base != 0) try idx_tables.append(gpa, disp);
                    }
                }
                try seen.put(gpa, a, it);
                a = nxt;
            }
        }
        try f.insns.appendSlice(gpa, seen.values());
        std.mem.sort(Insn, f.insns.items, {}, struct {
            fn lt(_: void, x: Insn, y: Insn) bool {
                return x.addr < y.addr;
            }
        }.lt);
        for (f.insns.items) |i| f.end = @max(f.end, i.addr + i.size);
        for (f.tables.items) |t| f.end = @max(f.end, t.addr + 4 * t.n);
        // A byte index table runs to the next instruction, table, or the limit, up to padding.
        for (idx_tables.items) |disp| {
            var stop = limit;
            for (f.insns.items) |i| if (i.addr > disp) {
                stop = @min(stop, i.addr);
                break;
            };
            for (f.tables.items) |t| if (t.addr > disp) {
                stop = @min(stop, t.addr);
            };
            var n: u32 = 0;
            while (disp + n < stop and d.code[disp + n - d.base] != 0xCC) n += 1;
            f.end = @max(f.end, disp + n);
        }
        return f;
    }
};
