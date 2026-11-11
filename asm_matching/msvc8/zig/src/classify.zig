//! classify (classify.py): why does each mapped function differ? Tags, counted.
//!
//!   oracle-zig classify --root <asm_matching/msvc8> [--out build/classes.tsv] [--tag TAG] [--single]
//!
//! For every mapped function that is not a MATCH, compares the two instruction streams for
//! the differences that have one cause in the source, and tags it:
//!
//!   cleanup      `ret N` differs: calling convention or parameter count
//!   regargs      a different set of registers is live on entry (liveIn)
//!   frame        the stack frame (`sub esp, N`) differs: locals differ
//!   saved        other callee-saved registers are saved and restored
//!   calls        a different number of calls, or different callees
//!   inverted     a conditional jump is the inverse of the original's at the same place
//!   signedness   a signed jump where the original has an unsigned one, or back
//!   regalloc     same instructions, registers differ only
//!   displacement same instructions, memory displacements differ only
//!   immediate    same instructions, constants differ only
//!   shape        none of the above explains it
//!
//! A function can carry several tags. The summary counts each tag, and how many functions
//! carry it alone. --tag lists the functions with a tag, most similar first.
const std = @import("std");
const c = @import("c");
const audit = @import("audit.zig");
const compare = @import("compare.zig");
const dec = @import("decode.zig");
const difflib = @import("difflib.zig");
const ours = @import("ours.zig");
const refimage = @import("refimage.zig");
const util = @import("util.zig");

const gpa = util.gpa;

// The general registers liveness follows, as bits: ESP is never one.
pub const EAX: u8 = 1;
pub const ECX: u8 = 2;
pub const EDX: u8 = 4;
pub const EBX: u8 = 8;
pub const ESI: u8 = 16;
pub const EDI: u8 = 32;
pub const EBP: u8 = 64;
const CALLER_SAVED: u8 = EAX | ECX | EDX;

pub const GPR_NAMES = [_]struct { []const u8, u8 }{
    .{ "eax", EAX }, .{ "ecx", ECX }, .{ "edx", EDX }, .{ "ebx", EBX }, .{ "esi", ESI }, .{ "edi", EDI }, .{ "ebp", EBP },
};
const SUB = [_]struct { []const u8, []const u8 }{
    .{ "al", "eax" }, .{ "ah", "eax" }, .{ "ax", "eax" }, .{ "cl", "ecx" }, .{ "ch", "ecx" }, .{ "cx", "ecx" },
    .{ "dl", "edx" }, .{ "dh", "edx" }, .{ "dx", "edx" }, .{ "bl", "ebx" }, .{ "bh", "ebx" }, .{ "bx", "ebx" },
    .{ "si", "esi" }, .{ "di", "edi" }, .{ "bp", "ebp" },
};

fn eql(a: []const u8, b: []const u8) bool {
    return std.mem.eql(u8, a, b);
}

/// The GPR a register name is (or is part of), as a bit; 0 for any other register.
fn gprBit(name0: []const u8) u8 {
    var name = name0;
    for (SUB) |s| if (eql(name, s[0])) {
        name = s[1];
        break;
    };
    for (GPR_NAMES) |g| if (eql(name, g[0])) return g[1];
    return 0;
}

fn isGpr(name: []const u8) bool {
    for (GPR_NAMES) |g| if (eql(name, g[0])) return true;
    return false;
}

const INVERSE = [_][2][]const u8{
    .{ "je", "jne" }, .{ "jne", "je" }, .{ "jl", "jge" }, .{ "jge", "jl" }, .{ "jle", "jg" }, .{ "jg", "jle" },
    .{ "jb", "jae" }, .{ "jae", "jb" }, .{ "jbe", "ja" }, .{ "ja", "jbe" }, .{ "js", "jns" }, .{ "jns", "js" },
    .{ "jp", "jnp" }, .{ "jnp", "jp" }, .{ "jo", "jno" }, .{ "jno", "jo" },
};
const SIGNED = [_][2][]const u8{
    .{ "jl", "jb" },   .{ "jge", "jae" }, .{ "jle", "jbe" }, .{ "jg", "ja" },   .{ "jb", "jl" },   .{ "jae", "jge" },
    .{ "jbe", "jle" }, .{ "ja", "jg" },   .{ "jl", "ja" },   .{ "jg", "jb" },   .{ "jle", "jae" }, .{ "jge", "jbe" },
    .{ "jb", "jg" },   .{ "ja", "jl" },   .{ "jae", "jle" }, .{ "jbe", "jge" },
};

fn inverseOf(m: []const u8) ?[]const u8 {
    for (INVERSE) |p| if (eql(p[0], m)) return p[1];
    return null;
}

fn isSigned(a: []const u8, b: []const u8) bool {
    for (SIGNED) |p| if (eql(p[0], a) and eql(p[1], b)) return true;
    return false;
}

/// A capstone handle with detail, for one instruction at a time.
pub const Cap = struct {
    h: c.csh,
    insn: [*c]c.cs_insn,

    pub fn init() !Cap {
        var h: c.csh = 0;
        if (c.cs_open(c.CS_ARCH_X86, c.CS_MODE_32, &h) != c.CS_ERR_OK) return error.Capstone;
        _ = c.cs_option(h, c.CS_OPT_DETAIL, c.CS_OPT_ON);
        return .{ .h = h, .insn = c.cs_malloc(h) };
    }

    fn one(cp: *Cap, bytes: []const u8, addr: u64) bool {
        var p: [*c]const u8 = bytes.ptr;
        var n: usize = bytes.len;
        var a: u64 = addr;
        return c.cs_disasm_iter(cp.h, &p, &n, &a, cp.insn);
    }

    fn mnemonic(cp: *const Cap) []const u8 {
        return std.mem.sliceTo(&cp.insn.*.mnemonic, 0);
    }

    fn opStr(cp: *const Cap) []const u8 {
        return std.mem.sliceTo(&cp.insn.*.op_str, 0);
    }

    fn regName(cp: *const Cap, r: c_uint) []const u8 {
        const p = c.cs_reg_name(cp.h, r) orelse return "";
        return std.mem.sliceTo(p, 0);
    }

    /// ins.regs_access() as GPR bits; null when capstone refuses (CsError).
    fn access(cp: *const Cap) ?struct { u8, u8 } {
        var rr: c.cs_regs = undefined;
        var rw: c.cs_regs = undefined;
        var nr: u8 = 0;
        var nw: u8 = 0;
        if (c.cs_regs_access(cp.h, cp.insn, &rr, &nr, &rw, &nw) != c.CS_ERR_OK) return null;
        var reads: u8 = 0;
        var writes: u8 = 0;
        for (rr[0..nr]) |r| reads |= gprBit(cp.regName(r));
        for (rw[0..nw]) |r| writes |= gprBit(cp.regName(r));
        return .{ reads, writes };
    }
};

/// op_str.split(', ')
fn ops(op: []const u8) ![][]const u8 {
    var out: std.ArrayList([]const u8) = .empty;
    var it = std.mem.splitSequence(u8, op, ", ");
    while (it.next()) |p| try out.append(gpa, p);
    return out.items;
}

const UD = struct { use: u8, def: u8 };
const BytesKey = struct { b: [16]u8, n: u8 };

pub const Classifier = struct {
    cap: Cap,
    ud_cache: std.AutoHashMapUnmanaged(BytesKey, UD) = .empty,

    /// _use_def: (registers read, registers written) by one instruction, for liveness: a call
    /// reads nothing and clobbers EAX/ECX/EDX; saving a callee-saved register is no read;
    /// `xor r, r`, `or r, -1`, `and r, 0` and kin read nothing; `cpuid` reads EAX only;
    /// padding no-ops read and write nothing. Decoded at address 0, as the Python does.
    fn useDef(cl: *Classifier, i: *const dec.Insn) !UD {
        var key = BytesKey{ .b = @splat(0), .n = i.size };
        @memcpy(key.b[0..i.size], i.code());
        if (cl.ud_cache.get(key)) |v| return v;
        const out = try cl.useDefUncached(i.code());
        try cl.ud_cache.put(gpa, key, out);
        return out;
    }

    fn useDefUncached(cl: *Classifier, bytes: []const u8) !UD {
        const cp = &cl.cap;
        if (!cp.one(bytes, 0)) return .{ .use = 0, .def = 0 };
        const m = cp.mnemonic();
        if (eql(m, "call")) return .{ .use = 0, .def = CALLER_SAVED };
        if (std.mem.startsWith(u8, m, "ret")) return .{ .use = 0, .def = 0 };
        if (eql(m, "cpuid")) return .{ .use = EAX, .def = EAX | EBX | ECX | EDX };
        const x = cp.insn.*.detail.*.unnamed_0.x86;
        if (eql(m, "push") and x.op_count > 0 and x.operands[0].type == c.X86_OP_REG) {
            const r = cp.regName(@intCast(x.operands[0].unnamed_0.reg));
            if (eql(r, "ebx") or eql(r, "esi") or eql(r, "edi") or eql(r, "ebp")) return .{ .use = 0, .def = 0 };
        }
        var reads: u8 = 0;
        var writes: u8 = 0;
        if (cp.access()) |a| {
            reads = a[0];
            writes = a[1];
        }
        const op = cp.opStr();
        const o = try ops(op);
        if ((eql(m, "xor") or eql(m, "sub") or eql(m, "sbb")) and o.len == 2 and eql(o[0], o[1])) {
            reads = 0;
        } else if (o.len == 2 and isGpr(o[0]) and ((eql(m, "or") and eql(o[1], "0xffffffff")) or (eql(m, "and") and eql(o[1], "0")))) {
            // `or ecx, -1` (the inline strlen's count) and `and r, 0` set the register
            // whatever it held.
            reads &= ~gprBit(o[0]);
        } else if ((eql(m, "mov") and o.len == 2 and eql(o[0], o[1])) or
            (eql(m, "lea") and o.len == 2 and (try leaNoop(o[0], o[1]))))
        {
            reads = 0; // a no-op of padding: `lea ebx, [ebx]`, `mov edi, edi`
            writes = 0;
        }
        return .{ .use = reads, .def = writes };
    }
};

fn leaNoop(r: []const u8, src: []const u8) !bool {
    const a = try std.fmt.allocPrint(gpa, "[{s}]", .{r});
    const b = try std.fmt.allocPrint(gpa, "[{s} + 0]", .{r});
    return eql(src, a) or eql(src, b);
}

fn text(i: *const dec.Insn) ![]const u8 {
    return std.fmt.allocPrint(gpa, "{s} {s}", .{ i.mn, i.op });
}

/// _slot_pushes: indices of the `push ecx` that reserve stack slots right after the frame is
/// set up (VC8's 4-byte local), unless the slot is read before it is written.
fn slotPushes(insns: []const dec.Insn, out: *std.AutoHashMapUnmanaged(usize, void)) !void {
    if (insns.len < 3 or !eql(try text(&insns[0]), "push ebp") or !eql(try text(&insns[1]), "mov ebp, esp")) return;
    var k: usize = 2;
    while (k < insns.len and eql(try text(&insns[k]), "push ecx")) {
        const slot = try std.fmt.allocPrint(gpa, "[ebp - {d}]", .{4 * (k - 1)});
        for (insns[k + 1 ..]) |*i| {
            if (std.mem.indexOf(u8, i.op, slot) != null) {
                const o = try ops(i.op);
                if (!eql(i.mn, "lea") and !(eql(i.mn, "mov") and std.mem.endsWith(u8, o[0], slot))) return; // read first
                break;
            }
        }
        try out.put(gpa, k, {});
        k += 1;
    }
}

/// live_in: registers live at the function's entry, by a backward data-flow over the
/// instructions' control flow (branches, switch tables, fall-through). `tableEntries` gives
/// a jump table's targets.
pub fn liveIn(cl: *Classifier, f: *const dec.Function, ctx: anytype) !u8 {
    const insns = f.insns.items;
    if (insns.len == 0) return 0;
    var slots: std.AutoHashMapUnmanaged(usize, void) = .empty;
    try slotPushes(insns, &slots);
    var tables: std.AutoHashMapUnmanaged(u32, []u32) = .empty;
    for (f.tables.items) |t| {
        const e = try gpa.alloc(u32, t.n);
        for (0..t.n) |k| e[k] = ctx.relocAt(t.addr + 4 * @as(u32, @intCast(k))) orelse return error.TableEntryLost;
        try tables.put(gpa, t.addr, e);
    }
    var index: std.AutoHashMapUnmanaged(u32, usize) = .empty;
    for (insns, 0..) |i, k| try index.put(gpa, i.addr, k);
    const succ = try gpa.alloc([]usize, insns.len);
    for (insns, 0..) |*i, k| {
        var out: std.ArrayList(usize) = .empty;
        const m = i.mn;
        const nxt = index.get(i.addr + i.size);
        if (std.mem.startsWith(u8, m, "ret") or eql(m, "int3") or eql(m, "hlt") or eql(m, "ud2")) {
            // none
        } else if (eql(m, "jmp")) {
            if (i.target) |t| {
                if (index.get(t)) |j| try out.append(gpa, j);
            } else if (i.table) |tb| {
                if (tables.get(tb)) |es| for (es) |t| if (index.get(t)) |j| try out.append(gpa, j);
            }
        } else if (dec.isCond(m)) {
            if (i.target) |t| if (index.get(t)) |j| try out.append(gpa, j);
            if (nxt) |j| try out.append(gpa, j);
        } else if (nxt) |j| try out.append(gpa, j);
        succ[k] = out.items;
    }
    // _argument_pushes: a push of ebx/esi/edi the function pops nowhere passes the value on.
    var popped: u8 = 0;
    for (insns) |*i| if (eql(i.mn, "pop") and (eql(i.op, "ebx") or eql(i.op, "esi") or eql(i.op, "edi"))) {
        popped |= gprBit(i.op);
    };
    const ud = try gpa.alloc(UD, insns.len);
    for (insns, 0..) |*i, k| {
        if (slots.contains(k)) {
            ud[k] = .{ .use = 0, .def = 0 };
        } else if (eql(i.mn, "push") and (eql(i.op, "ebx") or eql(i.op, "esi") or eql(i.op, "edi")) and popped & gprBit(i.op) == 0) {
            ud[k] = .{ .use = gprBit(i.op), .def = 0 };
        } else ud[k] = try cl.useDef(i);
    }
    const live = try gpa.alloc(u8, insns.len);
    @memset(live, 0);
    var changed = true;
    while (changed) {
        changed = false;
        var k = insns.len;
        while (k > 0) {
            k -= 1;
            var after: u8 = 0;
            for (succ[k]) |j| after |= live[j];
            const new = ud[k].use | (after & ~ud[k].def);
            if (new != live[k]) {
                live[k] = new;
                changed = true;
            }
        }
    }
    return live[0] & ~EBP;
}

/// int(s, 16) for the forms capstone prints: '0x1c' or '8'.
fn pyHex(s: []const u8) !u64 {
    const t = if (std.mem.startsWith(u8, s, "0x")) s[2..] else s;
    return std.fmt.parseInt(u64, t, 16);
}

/// ret_imm: the set of `ret N` immediates, as a sorted list.
fn retImm(insns: []const dec.Insn) ![]u64 {
    var vals: std.ArrayList(u64) = .empty;
    for (insns) |*i| if (std.mem.startsWith(u8, i.mn, "ret")) {
        const v = if (i.op.len > 0) try pyHex(i.op) else 0;
        if (std.mem.indexOfScalar(u64, vals.items, v) == null) try vals.append(gpa, v);
    };
    std.mem.sort(u64, vals.items, {}, std.sort.asc(u64));
    return vals.items;
}

fn frameSize(insns: []const dec.Insn) !u64 {
    for (insns[0..@min(8, insns.len)]) |*i| {
        if (eql(i.mn, "sub") and std.mem.startsWith(u8, i.op, "esp, ")) return pyHex(i.op[5..]);
    }
    return 0;
}

/// saved: the callee-saved registers the function restores before returning.
fn savedRegs(insns: []const dec.Insn) u8 {
    var out: u8 = 0;
    for (insns) |*i| if (eql(i.mn, "pop") and (eql(i.op, "ebx") or eql(i.op, "esi") or eql(i.op, "edi"))) {
        out |= gprBit(i.op);
    };
    return out;
}

// OPERAND = re.compile(r'0x[0-9a-f]+|\b\d+\b')
fn isW(ch: u8) bool {
    return std.ascii.isAlphanumeric(ch) or ch == '_' or ch >= 0x80;
}

fn isLowerHex(ch: u8) bool {
    return std.ascii.isDigit(ch) or (ch >= 'a' and ch <= 'f');
}

fn operandAt(s: []const u8, p: usize) ?usize {
    if (s[p] == '0' and p + 2 < s.len and s[p + 1] == 'x' and isLowerHex(s[p + 2])) {
        var q = p + 2;
        while (q < s.len and isLowerHex(s[q])) q += 1;
        return q;
    }
    if (std.ascii.isDigit(s[p]) and (p == 0 or !isW(s[p - 1]))) {
        var q = p;
        while (q < s.len and std.ascii.isDigit(s[q])) q += 1;
        if (q == s.len or !isW(s[q])) return q;
    }
    return null;
}

/// OPERAND.sub('N', s)
fn operandSub(s: []const u8) ![]const u8 {
    var out: std.ArrayList(u8) = .empty;
    var p: usize = 0;
    while (p < s.len) {
        if (operandAt(s, p)) |q| {
            try out.append(gpa, 'N');
            p = q;
        } else {
            try out.append(gpa, s[p]);
            p += 1;
        }
    }
    return out.items;
}

/// OPERAND.findall(s)
fn operandAll(s: []const u8) ![][]const u8 {
    var out: std.ArrayList([]const u8) = .empty;
    var p: usize = 0;
    while (p < s.len) {
        if (operandAt(s, p)) |q| {
            try out.append(gpa, s[p..q]);
            p = q;
        } else p += 1;
    }
    return out.items;
}

/// s[s.find('['):] (find's -1 slices off all but the last character).
fn fromBracket(s: []const u8) []const u8 {
    if (std.mem.indexOfScalar(u8, s, '[')) |i| return s[i..];
    return if (s.len > 0) s[s.len - 1 ..] else s;
}

fn sameStrs(a: []const []const u8, b: []const []const u8) bool {
    if (a.len != b.len) return false;
    for (a, b) |x, y| if (!eql(x, y)) return false;
    return true;
}

pub const Tag = enum {
    cleanup,
    regargs,
    frame,
    saved,
    calls,
    inverted,
    signedness,
    displacement,
    immediate,
    regalloc,
    shape,
};

const Tags = struct {
    list: std.ArrayList(Tag) = .empty,
    fn has(t: *const Tags, x: Tag) bool {
        return std.mem.indexOfScalar(Tag, t.list.items, x) != null;
    }
    fn add(t: *Tags, x: Tag) !void {
        try t.list.append(gpa, x);
    }
};

fn isCallTok(t: *const compare.Token, ours_side: bool) bool {
    return t.kind == .fn_ or t.kind == .import or (ours_side and t.kind == .fnq);
}

fn classifyOne(cl: *Classifier, viewer: *compare.Viewer, of: *const ours.OurFunc, s: u32, model: *const compare.Model, ref: *refimage.Reference, syms: anytype) !struct { []Tag, i64 } {
    const me = model.getPtr(s).?;
    const ov = try viewer.ourView(of, syms);
    const rv = try viewer.refViewSpan(s, me.f.end, model);
    const ro = of.f.decoded.insns.items;
    const rr = me.f.insns.items;
    var tags = Tags{};
    if (!std.mem.eql(u64, try retImm(ro), try retImm(rr))) try tags.add(.cleanup);
    if (try liveIn(cl, &of.f.decoded, &of.f) != try liveIn(cl, &me.f, ref)) try tags.add(.regargs);
    if (try frameSize(ro) != try frameSize(rr)) try tags.add(.frame);
    if (savedRegs(ro) != savedRegs(rr)) try tags.add(.saved);

    var cal_o: std.ArrayList(*const compare.Token) = .empty;
    var cal_r: std.ArrayList(*const compare.Token) = .empty;
    for (ov) |*i| for (i.tokens) |*t| if (isCallTok(t, true)) try cal_o.append(gpa, t);
    for (rv) |*i| for (i.tokens) |*t| if (isCallTok(t, false)) try cal_r.append(gpa, t);
    var calls = cal_o.items.len != cal_r.items.len;
    if (!calls) for (cal_o.items, cal_r.items) |a, b| {
        if (a.kind != .fnq and !a.eql(b)) {
            calls = true;
            break;
        }
    };
    if (calls) try tags.add(.calls);

    const mo = try gpa.alloc(u32, ov.len);
    for (ov, 0..) |x, k| mo[k] = x.mnemonic;
    const mr = try gpa.alloc(u32, rv.len);
    for (rv, 0..) |x, k| mr[k] = x.mnemonic;
    var sm = try difflib.SequenceMatcher.init(gpa, mo, mr);
    defer sm.deinit();
    for (try sm.opcodes()) |op| {
        if (op.tag == .replace and op.ci2 - op.ci == op.cj2 - op.cj) {
            for (ov[op.ci..op.ci2], rv[op.cj..op.cj2]) |*x, *y| {
                if (inverseOf(x.mn_text)) |inv| if (eql(inv, y.mn_text) and !tags.has(.inverted)) try tags.add(.inverted);
                if (isSigned(x.mn_text, y.mn_text) and !tags.has(.signedness)) try tags.add(.signedness);
            }
        }
    }
    var same_mn = ov.len == rv.len;
    if (same_mn) for (ov, rv) |*x, *y| if (x.mnemonic != y.mnemonic) {
        same_mn = false;
        break;
    };
    if (same_mn) {
        var displacement = false;
        var immediate = false;
        var regalloc = false;
        for (ov, rv) |*x, *y| {
            if (x.key == y.key) continue;
            const xk = viewer.intern.str(x.key);
            const yk = viewer.intern.str(y.key);
            if (!eql(try operandSub(xk), try operandSub(yk))) {
                regalloc = true;
            } else if (std.mem.indexOfScalar(u8, xk, '[') != null and !sameStrs(try operandAll(fromBracket(xk)), try operandAll(fromBracket(yk)))) {
                displacement = true;
            } else immediate = true;
        }
        // sorted(kinds)
        if (displacement) try tags.add(.displacement);
        if (immediate) try tags.add(.immediate);
        if (regalloc) try tags.add(.regalloc);
    }
    if (tags.list.items.len == 0) try tags.add(.shape);
    return .{ tags.list.items, @as(i64, @intCast(ov.len)) - @as(i64, @intCast(rv.len)) };
}

const Row = struct { s: u32, name: []const u8, tu: []const u8, st: []const u8, sim: f64, sim_text: []const u8, delta: i64, tags: []Tag };

fn joinTags(out: *std.ArrayList(u8), tags: []const Tag) !void {
    for (tags, 0..) |t, k| {
        if (k > 0) try out.append(gpa, ',');
        try out.appendSlice(gpa, @tagName(t));
    }
}

/// Python's '%-Ns' % s[:n]: by code points, not bytes.
fn padCodepoints(out: *std.ArrayList(u8), s: []const u8, width: usize, cut: ?usize) !void {
    var n: usize = 0;
    var end: usize = 0;
    while (end < s.len) {
        if (cut) |lim| if (n == lim) break;
        const l = std.unicode.utf8ByteSequenceLength(s[end]) catch 1;
        end = @min(s.len, end + l);
        n += 1;
    }
    try out.appendSlice(gpa, s[0..end]);
    while (n < width) : (n += 1) try out.append(gpa, ' ');
}

pub fn main(args: []const []const u8) !void {
    var root: ?[]const u8 = null;
    var out_path: ?[]const u8 = null;
    var tag_arg: ?[]const u8 = null;
    var single = false;
    var k: usize = 0;
    while (k < args.len) : (k += 1) {
        const a = args[k];
        if (eql(a, "--single")) {
            single = true;
            continue;
        }
        const Opt = struct { name: []const u8, dst: *?[]const u8 };
        const opts = [_]Opt{ .{ .name = "--root", .dst = &root }, .{ .name = "--out", .dst = &out_path }, .{ .name = "--tag", .dst = &tag_arg } };
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
            std.debug.print("usage: oracle-zig classify --root <asm_matching/msvc8> [--out O] [--tag T] [--single]\n", .{});
            std.process.exit(2);
        }
    }
    const cfg = try audit.Config.init(root orelse {
        std.debug.print("oracle-zig classify: --root is required\n", .{});
        std.process.exit(2);
    });
    const out_file = out_path orelse try util.join(cfg.build, "classes.tsv");

    const rows = try audit.loadFuncmap(try cfg.evidence("funcmap.tsv"));
    const Status = struct { st: []const u8, sim: f64, text: []const u8 };
    var status: std.AutoHashMapUnmanaged(u32, Status) = .empty;
    {
        var it = util.Lines{ .data = try util.readFile(try cfg.evidence("audit.tsv")) };
        while (it.next()) |line| {
            const p = try util.split(line, '\t');
            if (eql(p[0], "start")) continue;
            const sim = try std.fmt.parseFloat(f64, util.strip(p[8]));
            var t: std.ArrayList(u8) = .empty;
            try util.fixed(&t, "%.3f", sim);
            try status.put(gpa, try util.hex(p[0]), .{ .st = p[9], .sim = sim, .text = t.items });
        }
    }
    const ref = try refimage.Reference.open(try util.join(cfg.oracle, "reference/Game.exe"));
    try ref.load(try util.join(cfg.refcache, "functions.tsv"));
    var model = try ref.model();
    var d = try dec.Decoder.init(gpa);
    var fns: std.StringHashMapUnmanaged(*const ours.OurFunc) = .empty;
    for (try ours.loadAll(&d, try util.join(cfg.build, "obj"))) |*f| try fns.put(gpa, try f.key(), f);
    const libmap_file = try cfg.evidence("libmap.tsv");
    const librows: []audit.MapRow = if (util.exists(libmap_file)) try audit.loadFuncmap(libmap_file) else &.{};
    var syms = try audit.symMap(rows, librows, try audit.loadCrtmap(try cfg.evidence("crtmap.tsv")), &ref.img, &model);

    var cl = Classifier{ .cap = try Cap.init() };
    var viewer = compare.Viewer{ .d = &d };
    var out: std.ArrayList(Row) = .empty;
    for (rows) |r| {
        const st: Status = status.get(r.s) orelse .{ .st = "?", .sim = 0.0, .text = "0.000" };
        if (eql(st.st, "MATCH") or eql(st.st, "QUARANTINE") or eql(st.st, "MISSING") or eql(st.st, "?")) continue;
        const key = try std.fmt.allocPrint(gpa, "{s}|{s}", .{ r.tu, r.sym });
        const f = fns.get(key) orelse continue;
        const tags, const delta = try classifyOne(&cl, &viewer, f, r.s, &model, ref, &syms);
        try out.append(gpa, .{ .s = r.s, .name = r.name, .tu = r.tu, .st = st.st, .sim = st.sim, .sim_text = st.text, .delta = delta, .tags = tags });
    }
    {
        var o: std.ArrayList(u8) = .empty;
        try o.appendSlice(gpa, "start\tname\ttu\tstatus\tsimilarity\tdelta\ttags\n");
        for (out.items) |r| {
            try o.print(gpa, "{x:0>8}\t{s}\t{s}\t{s}\t{s}\t{d}\t", .{ r.s, r.name, r.tu, r.st, r.sim_text, r.delta });
            try joinTags(&o, r.tags);
            try o.append(gpa, '\n');
        }
        try util.writeFile(out_file, o.items);
    }
    var o: std.ArrayList(u8) = .empty;
    if (tag_arg) |want| {
        var sel: std.ArrayList(Row) = .empty;
        for (out.items) |r| {
            var has = false;
            for (r.tags) |t| if (eql(@tagName(t), want)) {
                has = true;
            };
            if (has and (!single or (r.tags.len == 1 and eql(@tagName(r.tags[0]), want)))) try sel.append(gpa, r);
        }
        std.mem.sort(Row, sel.items, {}, struct {
            fn lt(_: void, a: Row, b: Row) bool {
                return -a.sim < -b.sim;
            }
        }.lt);
        for (sel.items) |r| {
            try o.print(gpa, "{x:0>8} {s} ", .{ r.s, r.sim_text });
            try padCodepoints(&o, r.st, 16, null);
            try o.append(gpa, ' ');
            try padCodepoints(&o, r.name, 60, 60);
            try o.append(gpa, ' ');
            try joinTags(&o, r.tags);
            try o.append(gpa, '\n');
        }
        util.say(o.items);
        return;
    }
    // Counter.most_common: by count, ties in first-seen order.
    var allc: std.AutoArrayHashMapUnmanaged(Tag, usize) = .empty;
    var alone: std.AutoHashMapUnmanaged(Tag, usize) = .empty;
    for (out.items) |r| {
        for (r.tags) |t| {
            const g = try allc.getOrPut(gpa, t);
            if (!g.found_existing) g.value_ptr.* = 0;
            g.value_ptr.* += 1;
        }
        if (r.tags.len == 1) {
            const g = try alone.getOrPut(gpa, r.tags[0]);
            if (!g.found_existing) g.value_ptr.* = 0;
            g.value_ptr.* += 1;
        }
    }
    const Item = struct { t: Tag, n: usize, at: usize };
    var items: std.ArrayList(Item) = .empty;
    for (allc.keys(), allc.values(), 0..) |t, n, at| try items.append(gpa, .{ .t = t, .n = n, .at = at });
    std.mem.sort(Item, items.items, {}, struct {
        fn lt(_: void, a: Item, b: Item) bool {
            if (a.n != b.n) return a.n > b.n;
            return a.at < b.at;
        }
    }.lt);
    try o.print(gpa, "{d} mapped functions that do not match\n", .{out.items.len});
    try o.print(gpa, "{s:<14} {s:>7} {s:>7}\n", .{ "tag", "with it", "alone" });
    for (items.items) |it| try o.print(gpa, "{s:<14} {d:>7} {d:>7}\n", .{ @tagName(it.t), it.n, alone.get(it.t) orelse 0 });
    util.say(o.items);
}

test "operand regex" {
    try std.testing.expectEqualStrings("mov eax, dword ptr [ebp - N]", try operandSub("mov eax, dword ptr [ebp - 0x1c]"));
    try std.testing.expectEqualStrings("add esp, N", try operandSub("add esp, 8"));
    try std.testing.expectEqualStrings("mov al, byte ptr [ecx + edx*N]", try operandSub("mov al, byte ptr [ecx + edx*2]"));
    try std.testing.expectEqualStrings("call FN", try operandSub("call FN"));
    const all = try operandAll("[esi + 0x10], 3");
    try std.testing.expectEqual(@as(usize, 2), all.len);
    try std.testing.expectEqualStrings("0x10", all[0]);
    try std.testing.expectEqualStrings("3", all[1]);
    try std.testing.expectEqualStrings("]", fromBracket("ab]"));
}
