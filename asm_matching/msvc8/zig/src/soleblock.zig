//! soleblock: rank VC8 diff causes by how many functions they are the ONLY blocker of.
//!
//!   oracle-zig aligndump --root <asm_matching/msvc8> --out align.jsonl
//!   oracle-zig soleblock [--funcmap F] [--libmap L] [--crtmap C] [--ratchet DIR --ratchet-report TSV]
//!                        align.jsonl OUTDIR
//!
//! Reads the audit's own alignment (aligndump: difflib over the neutral keys) and turns every
//! differing stretch into cause signatures. Only the asm side is parsed; no C is read.
//!
//! Items within HUNK_GAP instructions of each other form a hunk named by its most specific item
//! (PRIORITY). Ripple signatures (registers renamed, internal jump targets, callee-saved push
//! order, frame size, slots, order) are dropped when a primary one is present. A function left
//! with register differences only is named by where the first misplaced value comes from
//! (reg-choice: a field, global, parameter slot, constant or call result): a controlled phase
//! screen found 109 of 110 such functions identical at every symbol-count phase, so they have a
//! source cause. A caller whose causes are all register-family and that calls functions of its
//! own unit that do not match is `callee-blocked` by them (VC8's custom convention for static
//! callees).
//!
//! Outputs in OUTDIR: index.tsv (causes ranked by sole_fns, then sole_bytes; scope `one fix`
//! when the key names one callee/constant/offset, `class` when it is fixed function by
//! function), callees.tsv (callees ranked by the callers they alone hold back), functions.tsv
//! (per function: its causes), frozen.tsv (with --ratchet: ratchet members kept at a
//! non-matching body).
//!
//! Text is held as Latin-1, one byte per character, as aligndump writes it (every byte >= 0x7f
//! as \u00XX), and written as UTF-8. Python's str.strip/split/isalnum are applied with their
//! Latin-1 meaning, so the output is byte-for-byte what soleblock.py wrote, except that the
//! kinds column of callees.tsv lists tied kinds in the order a caller first named them (the
//! script iterated a set there, so its tie order changed with the hash seed).
const std = @import("std");
const cli = @import("cli.zig");
const proc = @import("proc.zig");
const util = @import("util.zig");

const gpa = util.gpa;

const USAGE =
    \\usage: oracle-zig soleblock [--funcmap F] [--libmap L] [--crtmap C]
    \\                            [--ratchet DIR --ratchet-report TSV] align.jsonl OUTDIR
    \\
    \\Rank VC8 diff causes by how many functions they are the only blocker of.
    \\
;

// Per-function temporaries; reset once a function's causes are settled.
var scratch_state = std.heap.ArenaAllocator.init(std.heap.c_allocator);
const sa = scratch_state.allocator();

fn eql(a: []const u8, b: []const u8) bool {
    return std.mem.eql(u8, a, b);
}

fn optEql(a: ?[]const u8, b: ?[]const u8) bool {
    if (a == null or b == null) return a == null and b == null;
    return eql(a.?, b.?);
}

fn truthy(s: ?[]const u8) bool {
    return s != null and s.?.len > 0;
}

fn inSet(s: []const u8, set: []const []const u8) bool {
    for (set) |x| if (eql(s, x)) return true;
    return false;
}

fn startsWith(s: []const u8, p: []const u8) bool {
    return std.mem.startsWith(u8, s, p);
}

fn contains(s: []const u8, p: []const u8) bool {
    return std.mem.indexOf(u8, s, p) != null;
}

const REGS32 = [_][]const u8{ "eax", "ebx", "ecx", "edx", "esi", "edi", "ebp", "esp" };
const REGS = REGS32 ++ [_][]const u8{ "ax", "bx", "cx", "dx", "si", "di", "bp", "sp", "al", "ah", "bl", "bh", "cl", "ch", "dl", "dh" };
const REG32_OF = [_][2][]const u8{
    .{ "al", "eax" }, .{ "ah", "eax" }, .{ "ax", "eax" }, .{ "bl", "ebx" }, .{ "bh", "ebx" }, .{ "bx", "ebx" },
    .{ "cl", "ecx" }, .{ "ch", "ecx" }, .{ "cx", "ecx" }, .{ "dl", "edx" }, .{ "dh", "edx" }, .{ "dx", "edx" },
    .{ "si", "esi" }, .{ "di", "edi" }, .{ "bp", "ebp" }, .{ "sp", "esp" },
};
const JCC = [_][]const u8{ "je", "jne", "jb", "jae", "jbe", "ja", "jl", "jge", "jle", "jg", "js", "jns", "jo", "jno", "jp", "jnp" };
const INVERSE = [_][2][]const u8{
    .{ "je", "jne" }, .{ "jne", "je" }, .{ "jb", "jae" }, .{ "jae", "jb" }, .{ "jbe", "ja" }, .{ "ja", "jbe" },
    .{ "jl", "jge" }, .{ "jge", "jl" }, .{ "jle", "jg" }, .{ "jg", "jle" }, .{ "js", "jns" }, .{ "jns", "js" },
    .{ "jo", "jno" }, .{ "jno", "jo" }, .{ "jp", "jnp" }, .{ "jnp", "jp" },
};
// A signed form and its unsigned one: either way round is a signedness difference.
const SIGNED = [_][2][]const u8{
    .{ "jl", "jb" },       .{ "jge", "jae" },   .{ "jle", "jbe" },  .{ "jg", "ja" },     .{ "sar", "shr" },
    .{ "movsx", "movzx" }, .{ "idiv", "div" },  .{ "imul", "mul" }, .{ "setl", "setb" }, .{ "setge", "setae" },
    .{ "setle", "setbe" }, .{ "setg", "seta" }, .{ "cdq", "xor" },
};
const SAVED = [_][]const u8{ "ebx", "esi", "edi" };
/// Signatures that follow from another difference rather than cause one.
const RIPPLE = [_][]const u8{ "reg", "jmp-target", "saved-reg", "frame-size", "frame-slot", "spill", "order" };

fn isReg(s: []const u8) bool {
    return inSet(s, &REGS);
}

fn isJcc(s: []const u8) bool {
    return inSet(s, &JCC);
}

fn reg32(s: []const u8) []const u8 {
    for (REG32_OF) |p| if (eql(p[0], s)) return p[1];
    return s;
}

fn inverseOf(m: []const u8) ?[]const u8 {
    for (INVERSE) |p| if (eql(p[0], m)) return p[1];
    return null;
}

fn signPair(a: []const u8, b: []const u8) bool {
    for (SIGNED) |p| if ((eql(p[0], a) and eql(p[1], b)) or (eql(p[1], a) and eql(p[0], b))) return true;
    return false;
}

// ---------------------------------------------------------------------------------------------
// Python's str semantics on Latin-1 text.

/// str.isspace() for a Latin-1 character.
fn isSpace(ch: u8) bool {
    return ch == ' ' or (ch >= 9 and ch <= 13) or (ch >= 0x1c and ch <= 0x1f) or ch == 0x85 or ch == 0xa0;
}

/// str.isalnum() for a Latin-1 character.
fn isAlnum(ch: u8) bool {
    if (std.ascii.isAlphanumeric(ch)) return true;
    return switch (ch) {
        0xaa, 0xb2, 0xb3, 0xb5, 0xb9, 0xba, 0xbc, 0xbd, 0xbe => true,
        0xc0...0xd6, 0xd8...0xf6, 0xf8...0xff => true,
        else => false,
    };
}

/// str.strip().
fn pyStrip(s: []const u8) []const u8 {
    var a: usize = 0;
    var b: usize = s.len;
    while (a < b and isSpace(s[a])) a += 1;
    while (b > a and isSpace(s[b - 1])) b -= 1;
    return s[a..b];
}

/// int(s, 0), or null where Python raises ValueError.
fn pyInt(s0: []const u8) ?i128 {
    var s = pyStrip(s0);
    var neg = false;
    if (s.len > 0 and (s[0] == '+' or s[0] == '-')) {
        neg = s[0] == '-';
        s = s[1..];
    }
    if (s.len == 0) return null;
    var base: u8 = 10;
    var prefixed = false;
    if (s.len >= 2 and s[0] == '0') {
        switch (s[1] | 0x20) {
            'x' => base = 16,
            'o' => base = 8,
            'b' => base = 2,
            else => {},
        }
        if (base != 10) {
            s = s[2..];
            prefixed = true;
        }
    }
    // One underscore between digits; after a prefix one may lead.
    if (prefixed and s.len > 0 and s[0] == '_') s = s[1..];
    if (s.len == 0 or s[0] == '_' or s[s.len - 1] == '_') return null;
    var v: i128 = 0;
    var all_zero = true;
    var prev_us = false;
    for (s) |ch| {
        if (ch == '_') {
            if (prev_us) return null;
            prev_us = true;
            continue;
        }
        prev_us = false;
        const d = std.fmt.charToDigit(ch, base) catch return null;
        if (d != 0) all_zero = false;
        v = std.math.mul(i128, v, base) catch return null;
        v = std.math.add(i128, v, d) catch return null;
    }
    // Decimal: no leading zeros, except for zero itself.
    if (base == 10 and s[0] == '0' and !all_zero) return null;
    return if (neg) -v else v;
}

/// Python's '%x' of an int: a negative prints as '-' and the magnitude.
fn pyHex(out: *std.ArrayList(u8), v: i128) !void {
    if (v < 0) try out.print(sa, "-{x}", .{@abs(v)}) else try out.print(sa, "{x}", .{v});
}

fn hexStr(v: i128) ![]const u8 {
    var o: std.ArrayList(u8) = .empty;
    try pyHex(&o, v);
    return o.items;
}

fn mask32(v: i128) i128 {
    return v & 0xffffffff;
}

/// What clean() makes of a label: printable ASCII kept, other controls a space, the rest '?'.
fn clean(out: *std.ArrayList(u8), s: []const u8, limit: usize) !void {
    for (s, 0..) |ch, k| {
        if (k >= limit) break;
        try out.append(gpa, if (ch >= ' ' and ch < 0x7f) ch else if (ch >= 0x7f) '?' else ' ');
    }
}

/// UTF-8 text from a file Python opened as text, as Latin-1 (every character here is < 0x100).
fn toLatin1(s: []const u8) ![]const u8 {
    var hi = false;
    for (s) |ch| if (ch >= 0x80) {
        hi = true;
        break;
    };
    if (!hi) return s;
    var out: std.ArrayList(u8) = .empty;
    var it = (std.unicode.Utf8View.init(s) catch return error.NotUtf8).iterator();
    while (it.nextCodepoint()) |cp| {
        if (cp > 0xff) return error.NotLatin1;
        try out.append(gpa, @intCast(cp));
    }
    return out.items;
}

/// Lines of a file read in text mode: universal newlines, without the terminator.
const TextLines = struct {
    data: []const u8,
    pos: usize = 0,
    fn next(it: *TextLines) ?[]const u8 {
        if (it.pos >= it.data.len) return null;
        const rest = it.data[it.pos..];
        var n: usize = 0;
        while (n < rest.len and rest[n] != '\n' and rest[n] != '\r') n += 1;
        var adv = n;
        if (n < rest.len) {
            adv += 1;
            if (rest[n] == '\r' and n + 1 < rest.len and rest[n + 1] == '\n') adv += 1;
        }
        it.pos += adv;
        return rest[0..n];
    }
};

// ---------------------------------------------------------------------------------------------
// Instructions as text.

const Tok = struct { m: []const u8, ops: []const []const u8 };

fn splitFirst(s: []const u8) [2][]const u8 {
    const i = std.mem.indexOfScalar(u8, s, ' ') orelse return .{ s, "" };
    return .{ s[0..i], s[i + 1 ..] };
}

/// 'mov eax, dword ptr [ecx + 0x10]' -> (mnemonic, [operand strings]).
fn tokenize(text0: []const u8) !Tok {
    const text = pyStrip(text0);
    if (startsWith(text, "rep ") or startsWith(text, "repne ") or startsWith(text, "lock ")) {
        const p = splitFirst(text);
        const q = splitFirst(p[1]);
        return .{ .m = try std.fmt.allocPrint(sa, "{s} {s}", .{ p[0], q[0] }), .ops = try splitOps(q[1]) };
    }
    const q = splitFirst(text);
    return .{ .m = q[0], .ops = try splitOps(q[1]) };
}

fn splitOps(s: []const u8) ![]const []const u8 {
    var out: std.ArrayList([]const u8) = .empty;
    var depth: i32 = 0;
    var start: usize = 0;
    for (s, 0..) |ch, k| {
        if (ch == '[') depth += 1 else if (ch == ']') depth -= 1;
        if (ch == ',' and depth == 0) {
            try out.append(sa, pyStrip(s[start..k]));
            start = k + 1;
        }
    }
    const last = pyStrip(s[start..]);
    if (last.len > 0) try out.append(sa, last);
    return out.items;
}

const Mem = struct {
    size: []const u8,
    base: ?[]const u8 = null,
    index: ?[]const u8 = null,
    scale: i128 = 1,
    disp: i128 = 0,
    sym: ?[]const u8 = null,
};

const Opnd = union(enum) {
    reg: []const u8,
    imm: i128,
    mem: Mem,
    sym: []const u8,

    fn kind(o: Opnd) []const u8 {
        return @tagName(o);
    }

    fn eq(a: Opnd, b: Opnd) bool {
        return switch (a) {
            .reg => |x| b == .reg and eql(x, b.reg),
            .sym => |x| b == .sym and eql(x, b.sym),
            .imm => |x| b == .imm and x == b.imm,
            .mem => |x| b == .mem and eql(x.size, b.mem.size) and optEql(x.base, b.mem.base) and
                optEql(x.index, b.mem.index) and x.scale == b.mem.scale and x.disp == b.mem.disp and optEql(x.sym, b.mem.sym),
        };
    }

    fn isFrameMem(o: Opnd) bool {
        return o == .mem and o.mem.base != null and (eql(o.mem.base.?, "ebp") or eql(o.mem.base.?, "esp"));
    }
};

fn parseOp(o: []const u8) !Opnd {
    if (isReg(o)) return .{ .reg = o };
    if (pyInt(o)) |n| return .{ .imm = n };
    if (std.mem.indexOfScalar(u8, o, '[') != null and std.mem.indexOfScalar(u8, o, ']') != null and !startsWith(o, "\"")) {
        const lb = std.mem.indexOfScalar(u8, o, '[').?;
        var size = pyStrip(try std.mem.replaceOwned(u8, sa, o[0..lb], " ptr", ""));
        if (std.mem.indexOfScalar(u8, size, ':')) |ci| size = size[ci + 1 ..];
        const rb = std.mem.lastIndexOfScalar(u8, o, ']').?;
        const inner = if (rb > lb) o[lb + 1 .. rb] else "";
        var m = Mem{ .size = size };
        var it = std.mem.splitSequence(u8, try std.mem.replaceOwned(u8, sa, inner, " - ", " + -"), " + ");
        while (it.next()) |t0| {
            var t = pyStrip(t0);
            const neg = startsWith(t, "-");
            if (neg) t = pyStrip(t[1..]);
            if (std.mem.indexOfScalar(u8, t, '*')) |si| {
                m.index = t[0..si];
                const sc = pyInt(t[si + 1 ..]) orelse 0;
                m.scale = if (sc == 0) 1 else sc;
            } else if (isReg(t)) {
                if (m.base == null) m.base = t else m.index = t;
            } else if (pyInt(t)) |n| {
                m.disp += if (neg) -n else n;
            } else m.sym = t;
        }
        return .{ .mem = m };
    }
    return .{ .sym = o };
}

fn regsIn(text: []const u8) ![]const []const u8 {
    const t = try sa.dupe(u8, text);
    for (t) |*ch| if (ch.* == '[' or ch.* == ']' or ch.* == ',' or ch.* == '*' or ch.* == '+') {
        ch.* = ' ';
    };
    var out: std.ArrayList([]const u8) = .empty;
    var k: usize = 0;
    while (k < t.len) {
        while (k < t.len and isSpace(t[k])) k += 1;
        const s = k;
        while (k < t.len and !isSpace(t[k])) k += 1;
        if (k > s and isReg(t[s..k])) try out.append(sa, t[s..k]);
    }
    return out.items;
}

/// Registers -> R, numbers -> N: what is left when allocation and offsets are ignored.
fn shape(text: []const u8) ![]const u8 {
    const t = try tokenize(text);
    var out: std.ArrayList(u8) = .empty;
    try out.appendSlice(sa, t.m);
    try out.append(sa, ' ');
    for (t.ops, 0..) |o, k| {
        if (k > 0) try out.append(sa, ',');
        switch (try parseOp(o)) {
            .reg => try out.append(sa, 'R'),
            .imm => try out.append(sa, 'N'),
            .sym => try out.append(sa, 'S'),
            .mem => |m| {
                try out.print(sa, "{s}[", .{m.size});
                if (truthy(m.base)) try out.append(sa, 'R');
                if (truthy(m.index)) try out.print(sa, "+R*{d}", .{m.scale});
                if (truthy(m.sym)) try out.appendSlice(sa, "+S");
                try out.append(sa, ']');
            },
        }
    }
    return out.items;
}

fn isCall(text: []const u8) bool {
    return startsWith(text, "call ");
}

fn callTarget(text: []const u8) []const u8 {
    const i = std.mem.indexOfScalar(u8, text, ' ') orelse return "?";
    return pyStrip(text[i + 1 ..]);
}

fn stripRegs(text: []const u8) ![]const u8 {
    var out: std.ArrayList(u8) = .empty;
    var w: usize = 0;
    var k: usize = 0;
    while (k <= text.len) : (k += 1) {
        const ch: u8 = if (k < text.len) text[k] else ' ';
        if (isAlnum(ch) or ch == '_') continue;
        if (k > w) {
            const word = text[w..k];
            try out.appendSlice(sa, if (isReg(word)) "R" else word);
        }
        try out.append(sa, ch);
        w = k + 1;
    }
    return out.items;
}

// ---------------------------------------------------------------------------------------------
// One function's alignment.

const OIns = struct { line: i64, text: []const u8, key: []const u8, hdr: ?[]const u8, masked: []const u8, toks: []const ?[]const u8 };
const RIns = struct { text: []const u8, key: []const u8, masked: []const u8, toks: []const ?[]const u8 };
const Op = struct { t: u8, i1: usize, i2: usize, j1: usize, j2: usize, marks: []const u8 = "" };
const TuFn = struct { line: i64, sym: []const u8, addr: ?[]const u8 };
const Item = struct { cat: []const u8, key: []const u8, det: []const u8, owner: ?[]const u8, pos: i64 };

const Sig = struct {
    cat: []const u8,
    key: []const u8,
    fn id(s: Sig) ![]const u8 {
        return std.fmt.allocPrint(gpa, "{s}\x00{s}", .{ s.cat, s.key });
    }
};

/// A function's causes in order, each with its first example difference.
const Causes = std.StringArrayHashMapUnmanaged(struct { sig: Sig, det: []const u8 });

const Side = enum { o, r };

const Fn = struct {
    a: []const u8,
    name: []const u8,
    tu: []const u8,
    status: []const u8,
    size: i64,
    O: []const OIns,
    R: []const RIns,
    ops: []const Op,
    items: std.ArrayList(Item) = .empty,
    tufns: []const TuFn,
    own_line: ?i64 = null,

    fn init(f: *Fn) void {
        for (f.tufns) |t| if (t.addr != null and eql(t.addr.?, f.a)) {
            f.own_line = t.line;
            break;
        };
    }

    /// Which function of the TU the ours instruction oi was written in, or null for own.
    fn owner(f: *const Fn, oi: i64) !?[]const u8 {
        if (oi < 0 or oi >= f.O.len) return null;
        const x = f.O[@intCast(oi)];
        if (truthy(x.hdr)) {
            const h = x.hdr.?;
            const c = std.mem.lastIndexOfScalar(u8, h, ':');
            return try std.fmt.allocPrint(gpa, "hdr:{s}", .{if (c) |i| h[0..i] else h});
        }
        if (x.line == 0 or f.tufns.len == 0) return null;
        var best: ?TuFn = null;
        for (f.tufns) |t| {
            if (t.line <= x.line) best = t else break;
        }
        const b = best orelse return null;
        if (b.addr != null and eql(b.addr.?, f.a)) return null;
        if (f.own_line != null and b.line == f.own_line.?) return null;
        return try std.fmt.allocPrint(gpa, "fn:{s}", .{if (truthy(b.addr)) b.addr.? else b.sym});
    }

    fn okey(f: *const Fn, i: usize) []const u8 {
        return f.O[i].key;
    }

    fn rkey(f: *const Fn, j: usize) []const u8 {
        return f.R[j].key;
    }

    fn keyOf(f: *const Fn, side: Side, i: usize) []const u8 {
        return if (side == .o) f.O[i].key else f.R[i].key;
    }

    fn textOf(f: *const Fn, side: Side, i: usize) []const u8 {
        return if (side == .o) f.O[i].text else f.R[i].text;
    }

    fn len(f: *const Fn, side: Side) usize {
        return if (side == .o) f.O.len else f.R.len;
    }

    fn add(f: *Fn, cat: []const u8, key: []const u8, det: []const u8, oi: i64) !void {
        try f.items.append(gpa, .{ .cat = cat, .key = try gpa.dupe(u8, key), .det = try gpa.dupe(u8, det), .owner = try f.owner(oi), .pos = oi });
    }

    /// The same instruction for moved-detection: masked bytes and called functions; strings
    /// and data compare as equal, since the original's carve may call a string data.
    fn loose(f: *const Fn, side: Side, i: usize) ![]const u8 {
        const masked = if (side == .o) f.O[i].masked else f.R[i].masked;
        const toks = if (side == .o) f.O[i].toks else f.R[i].toks;
        var out: std.ArrayList(u8) = .empty;
        try out.appendSlice(sa, masked);
        for (toks) |t| if (t != null and startsWith(t.?, "fn_")) {
            try out.append(sa, 0);
            try out.appendSlice(sa, t.?);
        };
        return out.items;
    }

    /// The audit's identity: same masked bytes, every comparable reference the same.
    fn identical(f: *const Fn, i: usize, j: usize) bool {
        const x = f.O[i];
        const y = f.R[j];
        if (!eql(x.masked, y.masked) or x.toks.len != y.toks.len) return false;
        for (x.toks, y.toks) |a, b| if (a != null and b != null and !eql(a.?, b.?)) return false;
        return true;
    }
};

fn nextCall(f: *const Fn, side: Side, start: usize, limit: usize) ?usize {
    const n = f.len(side);
    var k = start;
    while (k < @min(n, start + limit)) : (k += 1) {
        const t = f.keyOf(side, k);
        if (isCall(t)) return k;
        if (startsWith(t, "ret") or startsWith(t, "jmp") or isJcc(splitFirst(t)[0])) return null;
    }
    return null;
}

fn prevCall(f: *const Fn, side: Side, start: usize) ?usize {
    var n: usize = 1;
    while (n <= 2 and n <= start) : (n += 1) {
        if (isCall(f.keyOf(side, start - n))) return start - n;
    }
    return null;
}

/// What a memory operand names, as far as the asm can say: a global by our symbol, a stack
/// slot as `frame`, a field as [+offset].
fn memkey(text: []const u8) ![]const u8 {
    for ((try tokenize(text)).ops) |o| {
        const p = try parseOp(o);
        if (p != .mem) continue;
        const m = p.mem;
        if (truthy(m.sym)) {
            if (m.disp != 0) return std.fmt.allocPrint(sa, "{s}+0x{s}", .{ m.sym.?, try hexStr(m.disp) });
            return m.sym.?;
        }
        if (p.isFrameMem()) return "frame";
        return std.fmt.allocPrint(sa, "[+0x{s}]", .{try hexStr(mask32(m.disp))});
    }
    return "";
}

const Class = struct { cat: []const u8, key: []const u8, det: []const u8 };

fn kindsJoin(ps: []const Opnd) ![]const u8 {
    var out: std.ArrayList(u8) = .empty;
    for (ps, 0..) |p, k| {
        if (k > 0) try out.append(sa, '/');
        try out.appendSlice(sa, p.kind());
    }
    return out.items;
}

/// One aligned pair of differing instructions -> (cat, key, detail).
fn classifyPair(f: *const Fn, i: usize, j: usize) !Class {
    const xt = f.O[i].text;
    const yt = f.R[j].text;
    const xk = f.okey(i);
    const yk = f.rkey(j);
    const tx = try tokenize(xk);
    const ty = try tokenize(yk);
    const xm = tx.m;
    const ym = ty.m;
    const xo = tx.ops;
    const yo = ty.ops;
    const det = try std.fmt.allocPrint(sa, "{s} => {s}", .{ xt, yt });
    const P = struct {
        fn c(cat: []const u8, key: []const u8, d: []const u8) Class {
            return .{ .cat = cat, .key = key, .det = d };
        }
    };
    if (eql(xm, ym) and (isJcc(xm) or eql(xm, "jmp"))) return P.c("jmp-target", "jump target", det);
    if (isJcc(xm) and isJcc(ym)) {
        if (inverseOf(xm)) |inv| if (eql(inv, ym)) return P.c("branch", "inverted", det);
        if (signPair(xm, ym)) return P.c("sign", try std.fmt.allocPrint(sa, "{s}/{s}", .{ xm, ym }), det);
        return P.c("branch", try std.fmt.allocPrint(sa, "{s}->{s}", .{ xm, ym }), det);
    }
    if (signPair(xm, ym)) return P.c("sign", try std.fmt.allocPrint(sa, "{s}/{s}", .{ xm, ym }), det);
    const movs = [_][]const u8{ "mov", "movzx", "movsx" };
    if (!eql(xm, ym)) {
        if (inSet(xm, &movs) and inSet(ym, &movs)) {
            const mk = try memkey(xt);
            return P.c("width", try std.fmt.allocPrint(sa, "{s}->{s} {s}", .{ xm, ym, if (mk.len > 0) mk else try memkey(yt) }), det);
        }
        return P.c("insn", try std.fmt.allocPrint(sa, "{s}->{s}", .{ try shape(xk), try shape(yk) }), det);
    }
    if (xo.len != yo.len) return P.c("insn", try std.fmt.allocPrint(sa, "{s}->{s}", .{ try shape(xk), try shape(yk) }), det);
    const px = try sa.alloc(Opnd, xo.len);
    const py = try sa.alloc(Opnd, yo.len);
    for (xo, 0..) |o, k| px[k] = try parseOp(o);
    for (yo, 0..) |o, k| py[k] = try parseOp(o);
    const commutative = [_][]const u8{ "add", "imul", "and", "or", "xor", "test", "cmp" };
    if (inSet(xm, &commutative) and xo.len == 2 and eql(xo[0], yo[1]) and eql(xo[1], yo[0])) return P.c("commute", xm, det);
    for (px, py) |a, b| {
        if (a == .mem and b == .mem and truthy(a.mem.index) and truthy(b.mem.index) and
            optEql(a.mem.base, b.mem.index) and optEql(a.mem.index, b.mem.base) and a.mem.scale == 1 and b.mem.scale == 1)
            return P.c("commute", "address base/index", det);
    }
    const sx = try shape(xk);
    const sy = try shape(yk);
    if (eql(sx, sy)) {
        // same form: registers, offsets or immediates
        const Diff = struct { kind: enum { reg, imm, frame_slot, disp, other }, a: Opnd = .{ .imm = 0 }, b: Opnd = .{ .imm = 0 } };
        var diffs: std.ArrayList(Diff) = .empty;
        for (px, py) |a, b| {
            if (a.eq(b)) continue;
            if (a == .reg and b == .reg) {
                try diffs.append(sa, .{ .kind = .reg });
            } else if (a == .imm and b == .imm) {
                try diffs.append(sa, .{ .kind = .imm, .a = a, .b = b });
            } else if (a == .mem and b == .mem) {
                if (a.mem.disp != b.mem.disp) {
                    const fb = a.isFrameMem() or b.isFrameMem();
                    try diffs.append(sa, .{ .kind = if (fb) .frame_slot else .disp, .a = a, .b = b });
                }
                if (!optEql(a.mem.base, b.mem.base) or !optEql(a.mem.index, b.mem.index)) try diffs.append(sa, .{ .kind = .reg });
            } else try diffs.append(sa, .{ .kind = .other, .a = a, .b = b });
        }
        var has = std.EnumSet(@FieldType(Diff, "kind")).init(.{});
        for (diffs.items) |d| has.insert(d.kind);
        var only_reg = true;
        for (diffs.items) |d| if (d.kind != .reg) {
            only_reg = false;
        };
        if (only_reg) return P.c("reg", "renamed", det);
        if (has.contains(.disp)) {
            for (diffs.items) |d| if (d.kind == .disp) {
                return P.c("disp", try std.fmt.allocPrint(sa, "{s} [+0x{s}]->[+0x{s}]", .{ d.a.mem.size, try hexStr(d.a.mem.disp), try hexStr(d.b.mem.disp) }), det);
            };
        }
        if (has.contains(.imm)) {
            for (diffs.items) |d| if (d.kind == .imm) {
                if (eql(xm, "ret")) return P.c("ret-n", try std.fmt.allocPrint(sa, "ret {d}->{d}", .{ d.a.imm, d.b.imm }), det);
                if (eql(xm, "sub") and eql(xo[0], "esp")) return P.c("frame-size", "sub esp", det);
                if (eql(xm, "add") and eql(xo[0], "esp")) return P.c("cleanup", try std.fmt.allocPrint(sa, "add esp {d}->{d}", .{ d.a.imm, d.b.imm }), det);
                return P.c("imm", try std.fmt.allocPrint(sa, "{s} 0x{s}->0x{s}", .{ xm, try hexStr(mask32(d.a.imm)), try hexStr(mask32(d.b.imm)) }), det);
            };
        }
        if (has.contains(.frame_slot)) return P.c("frame-slot", "stack slot", det);
        return P.c("insn", try std.fmt.allocPrint(sa, "{s}->{s}", .{ sx, sy }), det);
    }
    // same mnemonic, other operand form: widths, reg vs mem, imm vs reg
    for (px, py) |a, b| {
        if (a == .mem and b == .mem and !eql(a.mem.size, b.mem.size))
            return P.c("width", try std.fmt.allocPrint(sa, "{s} {s}->{s} {s}", .{ xm, a.mem.size, b.mem.size, try memkey(xt) }), det);
    }
    const kx = try kindsJoin(px);
    const ky = try kindsJoin(py);
    if (!eql(kx, ky)) {
        var frame = false;
        for (px) |p| frame = frame or p.isFrameMem();
        for (py) |p| frame = frame or p.isFrameMem();
        return P.c(if (frame) "held-vs-reread" else "operand", try std.fmt.allocPrint(sa, "{s} {s}->{s}", .{ xm, kx, ky }), det);
    }
    return P.c("insn", try std.fmt.allocPrint(sa, "{s}->{s}", .{ sx, sy }), det);
}

const EAX_WRITERS = [_][]const u8{
    "mov eax", "xor eax",  "movzx eax", "movsx eax", "neg eax", "sbb eax", "and eax",  "sete al",  "setne al",
    "setl al", "setg al",  "setle al",  "setge al",  "setb al", "seta al", "setbe al", "setae al", "inc eax",
    "dec eax", "add eax",  "sub eax",   "or eax",    "lea eax", "mov al",  "xor al",   "or al",    "and al",
    "mov ax",  "movzx ax",
};

fn writesEax(t: []const u8) bool {
    for (EAX_WRITERS) |p| if (startsWith(t, p)) return true;
    return false;
}

/// Does any path of our listing write EAX just before its ret (past the epilogue)?
fn returnsValue(f: *const Fn) bool {
    for (0..f.O.len) |q| {
        if (!startsWith(f.okey(q), "ret")) continue;
        var k: isize = @as(isize, @intCast(q)) - 1;
        while (k >= 0) : (k -= 1) {
            const t = f.okey(@intCast(k));
            if (!(startsWith(t, "pop ") or eql(t, "leave") or eql(t, "mov esp, ebp"))) break;
        }
        if (k >= 0 and writesEax(f.okey(@intCast(k)))) return true;
        if (k >= 0 and startsWith(f.okey(@intCast(k)), "call ")) return true; // a tail value from a call: cannot tell
    }
    return false;
}

/// Only epilogue instructions (or more EAX writes) from start to a ret, within 5.
fn epilogueFollows(f: *const Fn, side: Side, start: usize) bool {
    const n = f.len(side);
    var q = start;
    while (q < @min(n, start + 5)) : (q += 1) {
        const t = f.keyOf(side, q);
        if (startsWith(t, "ret")) return true;
        if (startsWith(t, "pop ") or eql(t, "leave") or eql(t, "mov esp, ebp") or writesEax(t)) continue;
        return false;
    }
    return false;
}

fn isCookie(target: []const u8) !bool {
    const n = names.get(try std.mem.replaceOwned(u8, sa, target, "fn_", "")) orelse return false;
    return eql(n, "__security_check_cookie");
}

/// An instruction only one side has.
fn classifyOneSided(f: *const Fn, side: Side, idx: usize) !Class {
    const t = f.textOf(side, idx);
    const k = f.keyOf(side, idx);
    const seq_len = f.len(side);
    const tk = try tokenize(k);
    const m = tk.m;
    const ops = tk.ops;
    const who: []const u8 = if (side == .o) "ours" else "orig";
    const det = try std.fmt.allocPrint(sa, "{s}: {s}", .{ who, t });
    const P = struct {
        fn c(cat: []const u8, key: []const u8, d: []const u8) Class {
            return .{ .cat = cat, .key = key, .det = d };
        }
    };
    const pp = [_][]const u8{ "push", "pop" };
    if (side == .r and ops.len > 0 and eql(reg32(ops[0]), "eax") and !inSet(m, &[_][]const u8{ "push", "pop", "cmp", "test" }) and
        epilogueFollows(f, side, idx + 1) and !returnsValue(f))
        // the original leaves a value in EAX for its caller: our prototype returns void
        return P.c("ret-value", "orig returns a value", det);
    if (inSet(m, &pp) and ops.len > 0 and inSet(ops[0], &SAVED))
        // prologue / epilogue save of a callee-saved register
        return P.c("saved-reg", try std.fmt.allocPrint(sa, "{s} {s} {s}", .{ who, m, ops[0] }), det);
    if ((inSet(m, &pp) and ops.len > 0 and eql(ops[0], "ebp")) or (eql(m, "mov") and ops.len == 2 and eql(ops[0], "ebp") and eql(ops[1], "esp")))
        return P.c("frame-pointer", who, det);
    if ((eql(m, "sub") and ops.len > 0 and eql(ops[0], "esp")) or
        (inSet(m, &pp) and ops.len == 1 and eql(ops[0], "ecx") and (idx < 4 or @as(isize, @intCast(idx)) > @as(isize, @intCast(seq_len)) - 4)))
        return P.c("frame-size", "alloc", det);
    if (isCall(k) or (eql(m, "jmp") and contains(k, "fn_"))) {
        const tgt = callTarget(t);
        if (try isCookie(tgt)) return P.c("gs-cookie", try std.fmt.allocPrint(sa, "{s}-only cookie", .{who}), det);
        return P.c("call", try std.fmt.allocPrint(sa, "{s}-only {s}", .{ who, tgt }), det);
    }
    if (eql(m, "add") and ops.len > 0 and eql(ops[0], "esp")) {
        if (prevCall(f, side, idx)) |pc| return P.c("cleanup", callTarget(f.textOf(side, pc)), det);
        return P.c("frame-size", "add esp", det);
    }
    if (eql(m, "push")) {
        if (nextCall(f, side, idx + 1, 8)) |nc| return P.c("args", callTarget(f.textOf(side, nc)), det);
    }
    if (inSet(m, &[_][]const u8{ "mov", "lea", "xor" }) and ops.len > 0 and inSet(ops[0], &[_][]const u8{ "ecx", "edx", "eax", "ebx", "esi", "edi" })) {
        if (nextCall(f, side, idx + 1, 3)) |nc| {
            const ct = callTarget(f.textOf(side, nc));
            // a register loaded just before a direct call: an argument in a register
            if (startsWith(ct, "fn_")) return P.c("args", ct, det);
        }
    }
    if (eql(m, "mov") and ops.len == 2) {
        const a = try parseOp(ops[0]);
        const b = try parseOp(ops[1]);
        if (a.isFrameMem() and b == .reg) return P.c("spill", "store", det);
        if (b.isFrameMem() and a == .reg) {
            if ((eql(b.mem.base.?, "ebp") and b.mem.disp > 0) or eql(b.mem.base.?, "esp")) return P.c("held-vs-reread", "param/stack read", det);
            return P.c("spill", "reload", det);
        }
        if (a == .reg and b == .reg) return P.c("copy", try std.fmt.allocPrint(sa, "{s} mov reg,reg", .{who}), det);
    }
    if (eql(m, "xor") and ops.len == 2 and eql(ops[1], "ebp")) return P.c("gs-cookie", try std.fmt.allocPrint(sa, "{s}-only cookie", .{who}), det);
    if (eql(m, "jmp")) return P.c("branch-layout", try std.fmt.allocPrint(sa, "{s} jmp", .{who}), det);
    if (isJcc(m)) return P.c("branch-layout", try std.fmt.allocPrint(sa, "{s} jcc", .{who}), det);
    if (startsWith(m, "ret")) return P.c("branch-layout", try std.fmt.allocPrint(sa, "{s} ret", .{who}), det);
    return P.c(if (side == .o) "extra" else "missing", try shape(k), det);
}

fn sortedStrs(xs: []const []const u8) ![]const []const u8 {
    const out = try sa.dupe([]const u8, xs);
    std.mem.sort([]const u8, out, {}, struct {
        fn lt(_: void, a: []const u8, b: []const u8) bool {
            return std.mem.order(u8, a, b) == .lt;
        }
    }.lt);
    return out;
}

fn sameStrs(a: []const []const u8, b: []const []const u8) bool {
    if (a.len != b.len) return false;
    for (a, b) |x, y| if (!eql(x, y)) return false;
    return true;
}

/// The block's operands with registers ignored, as multisets.
fn operandMultiset(f: *const Fn, side: Side, idx: []const usize) ![]const []const u8 {
    var out: std.ArrayList([]const u8) = .empty;
    for (idx) |i| for ((try tokenize(try stripRegs(f.keyOf(side, i)))).ops) |o| try out.append(sa, o);
    return sortedStrs(out.items);
}

/// Pair a replace block's instructions by mnemonic, in order (an LCS).
fn mnemonicPairs(f: *const Fn, xs: []const usize, ys: []const usize) ![]const [2]usize {
    const n = xs.len;
    const m = ys.len;
    if (n * m > 40000) return &.{};
    const a = try sa.alloc([]const u8, n);
    const b = try sa.alloc([]const u8, m);
    for (xs, 0..) |i, k| a[k] = (try tokenize(f.okey(i))).m;
    for (ys, 0..) |j, k| b[k] = (try tokenize(f.rkey(j))).m;
    const w = m + 1;
    const L = try sa.alloc(u32, (n + 1) * w);
    @memset(L, 0);
    var p: usize = n;
    while (p > 0) {
        p -= 1;
        var q: usize = m;
        while (q > 0) {
            q -= 1;
            L[p * w + q] = if (eql(a[p], b[q])) L[(p + 1) * w + q + 1] + 1 else @max(L[(p + 1) * w + q], L[p * w + q + 1]);
        }
    }
    var out: std.ArrayList([2]usize) = .empty;
    p = 0;
    var q: usize = 0;
    while (p < n and q < m) {
        if (eql(a[p], b[q])) {
            try out.append(sa, .{ xs[p], ys[q] });
            p += 1;
            q += 1;
        } else if (L[(p + 1) * w + q] >= L[p * w + q + 1]) p += 1 else q += 1;
    }
    return out.items;
}

fn range(a: usize, b: usize) ![]usize {
    const out = try sa.alloc(usize, b - a);
    for (out, 0..) |*x, k| x.* = a + k;
    return out;
}

fn analyse(f: *Fn) !void {
    const One = struct { idx: usize, near: i64 };
    var one_o: std.ArrayList(One) = .empty;
    var one_r: std.ArrayList(One) = .empty;
    var last_o: i64 = -1;
    for (f.ops) |op| {
        const pi1 = op.i1;
        const pi2 = op.i2;
        const pj1 = op.j1;
        const pj2 = op.j2;
        if (op.t == 'e') {
            for (op.marks, 0..) |mk, k| {
                if (mk == '!') {
                    try f.add("ref", try std.fmt.allocPrint(sa, "{s} -> {s}", .{ f.O[pi1 + k].text, f.R[pj1 + k].text }), f.O[pi1 + k].text, @intCast(pi1 + k));
                } else if (mk == '~') {
                    try f.add("encoding", (try tokenize(f.okey(pi1 + k))).m, f.O[pi1 + k].text, @intCast(pi1 + k));
                }
            }
            last_o = @as(i64, @intCast(pi2)) - 1;
            continue;
        }
        const xs = try range(pi1, pi2);
        const ys = try range(pj1, pj2);
        if (op.t == 'r') {
            var ok: std.ArrayList([]const u8) = .empty;
            var rk: std.ArrayList([]const u8) = .empty;
            for (xs) |i| try ok.append(sa, f.okey(i));
            for (ys) |j| try rk.append(sa, f.rkey(j));
            if (sameStrs(try sortedStrs(ok.items), try sortedStrs(rk.items))) {
                var stores = false;
                for (xs) |i| {
                    const t = try tokenize(f.okey(i));
                    const first = if (t.ops.len > 0) t.ops[0] else "";
                    if (eql(t.m, "mov") and std.mem.indexOfScalar(u8, first, '[') != null) stores = true;
                }
                try f.add("order", if (stores) "store order" else "schedule", try std.fmt.allocPrint(sa, "{d} insns permuted", .{xs.len}), @intCast(xs[0]));
                last_o = @as(i64, @intCast(pi2)) - 1;
                continue;
            }
        }
        if (op.t == 'r' and xs.len == ys.len) {
            var same_shape = true;
            for (xs, ys) |i, j| if (!eql(try shape(f.okey(i)), try shape(f.rkey(j)))) {
                same_shape = false;
                break;
            };
            if (same_shape) {
                const a = try operandMultiset(f, .o, xs);
                const b = try operandMultiset(f, .r, ys);
                var differs = false;
                for (xs, ys) |i, j| if (!eql(try stripRegs(f.okey(i)), try stripRegs(f.rkey(j)))) {
                    differs = true;
                };
                if (sameStrs(a, b) and differs) {
                    try f.add("commute", "operand order", try std.fmt.allocPrint(sa, "{s} => {s}", .{ f.O[xs[0]].text, f.R[ys[0]].text }), @intCast(xs[0]));
                    last_o = @as(i64, @intCast(pi2)) - 1;
                    continue;
                }
            }
        }
        var pairs: []const [2]usize = &.{};
        if (op.t == 'r') {
            if (xs.len == ys.len) {
                const pz = try sa.alloc([2]usize, xs.len);
                for (xs, ys, 0..) |i, j, k| pz[k] = .{ i, j };
                pairs = pz;
            } else pairs = try mnemonicPairs(f, xs, ys);
        }
        var po: std.AutoHashMapUnmanaged(usize, void) = .empty;
        var pr: std.AutoHashMapUnmanaged(usize, void) = .empty;
        for (pairs) |p| {
            try po.put(sa, p[0], {});
            try pr.put(sa, p[1], {});
        }
        for (pairs) |p| {
            if (f.identical(p[0], p[1])) continue;
            const c = try classifyPair(f, p[0], p[1]);
            try f.add(c.cat, c.key, c.det, @intCast(p[0]));
        }
        for (xs) |i| if (!po.contains(i)) try one_o.append(sa, .{ .idx = i, .near = @intCast(i) });
        for (ys) |j| if (!pr.contains(j)) {
            const near: i64 = if (op.t != 'r') @max(@as(i64, @intCast(pi1)) - 1, last_o) else @intCast(pi1);
            try one_r.append(sa, .{ .idx = j, .near = near });
        };
        if (pi2 > pi1) last_o = @as(i64, @intCast(pi2)) - 1;
    }
    // an instruction one side has here and the other has elsewhere: moved, not missing
    var ko: std.StringHashMapUnmanaged(i64) = .empty;
    var kr: std.StringHashMapUnmanaged(i64) = .empty;
    for (one_o.items) |x| (try ko.getOrPutValue(sa, try f.loose(.o, x.idx), 0)).value_ptr.* += 1;
    for (one_r.items) |x| (try kr.getOrPutValue(sa, try f.loose(.r, x.idx), 0)).value_ptr.* += 1;
    var mo: std.StringHashMapUnmanaged(i64) = .empty;
    var mr: std.StringHashMapUnmanaged(i64) = .empty;
    var it = ko.iterator();
    while (it.next()) |e| {
        const n = @min(e.value_ptr.*, kr.get(e.key_ptr.*) orelse 0);
        if (n > 0) {
            try mo.put(sa, e.key_ptr.*, n);
            try mr.put(sa, e.key_ptr.*, n);
        }
    }
    for (one_o.items) |x| {
        if (mo.getPtr(try f.loose(.o, x.idx))) |c| if (c.* > 0) {
            c.* -= 1;
            try f.add("order", "moved", f.O[x.idx].text, @intCast(x.idx));
            continue;
        };
        const c = try classifyOneSided(f, .o, x.idx);
        try f.add(c.cat, c.key, c.det, @intCast(x.idx));
    }
    for (one_r.items) |x| {
        if (mr.getPtr(try f.loose(.r, x.idx))) |c| if (c.* > 0) {
            c.* -= 1;
            continue;
        };
        const c = try classifyOneSided(f, .r, x.idx);
        try f.add(c.cat, c.key, c.det, x.near);
    }
}

/// Where the value an ours instruction writes comes from, as far as the asm says.
fn valueSource(f: *const Fn, i: usize) ![]const u8 {
    const t = f.O[i].text;
    const tk = try tokenize(t);
    const m = tk.m;
    const ops = tk.ops;
    if (ops.len < 2) {
        if (startsWith(m, "pop") or startsWith(m, "push")) return "saved register order";
        return m;
    }
    const src = ops[ops.len - 1];
    const p = try parseOp(src);
    if (p == .reg and eql(reg32(src), "eax")) {
        // the result of the call just before it
        var n: usize = 1;
        while (n <= 2 and n <= i) : (n += 1) {
            if (isCall(f.O[i - n].text)) return std.fmt.allocPrint(sa, "result of {s}", .{callTarget(f.O[i - n].text)});
        }
    }
    switch (p) {
        .mem => |mm| {
            if (truthy(mm.sym)) return std.fmt.allocPrint(sa, "global {s}", .{mm.sym.?});
            if (mm.base != null and eql(mm.base.?, "ebp")) {
                if (mm.disp > 0) return std.fmt.allocPrint(sa, "param [ebp+0x{s}]", .{try hexStr(mm.disp)});
                return std.fmt.allocPrint(sa, "local [ebp-0x{s}]", .{try hexStr(mask32(-mm.disp))});
            }
            if (mm.base != null and eql(mm.base.?, "esp")) return "stack [esp]";
            return std.fmt.allocPrint(sa, "field [+0x{s}] {s}", .{ try hexStr(mask32(mm.disp)), mm.size });
        },
        .imm => return "constant",
        .sym => return std.fmt.allocPrint(sa, "address {s}", .{src}),
        .reg => {},
    }
    if (eql(m, "lea")) return "address arithmetic";
    if (inSet(m, &[_][]const u8{ "add", "sub", "imul", "and", "or", "xor", "shl", "shr", "sar" })) return std.fmt.allocPrint(sa, "{s} result", .{m});
    return "register copy";
}

/// A register-only difference has a source cause too (a controlled phase screen found 109 of
/// 110 such functions identical at every symbol-count phase): name the first value VC8 puts in
/// another register by where it comes from, so functions that share a field, global, parameter
/// slot or callee group together.
fn regChoice(f: *const Fn, how: []const u8) !Sig {
    for (f.ops) |op| {
        if (op.t == 'e') continue;
        var i = op.i1;
        var j = op.j1;
        while (i < op.i2 and j < op.j2) : ({
            i += 1;
            j += 1;
        }) {
            if (!eql(f.okey(i), f.rkey(j))) return .{ .cat = "reg-choice", .key = try std.fmt.allocPrint(gpa, "{s}: {s}", .{ how, try valueSource(f, i) }) };
        }
        if (op.i2 > op.i1) return .{ .cat = "reg-choice", .key = try std.fmt.allocPrint(gpa, "{s}: {s}", .{ how, try valueSource(f, op.i1) }) };
    }
    return .{ .cat = "reg-choice", .key = how };
}

/// True when one register bijection maps every differing ours pair onto the original's.
fn consistentPermutation(f: *const Fn) !bool {
    var fw: std.StringHashMapUnmanaged([]const u8) = .empty;
    var bw: std.StringHashMapUnmanaged([]const u8) = .empty;
    for (f.ops) |op| {
        if (op.t == 'e') continue;
        if (op.t != 'r' or op.i2 - op.i1 != op.j2 - op.j1) return false;
        for (op.i1..op.i2, op.j1..op.j2) |i, j| {
            if (!eql(try stripRegs(f.okey(i)), try stripRegs(f.rkey(j)))) return false;
            const ra = try regsIn(f.okey(i));
            const rb = try regsIn(f.rkey(j));
            if (ra.len != rb.len) return false;
            for (ra, rb) |x0, y0| {
                const x = reg32(x0);
                const y = reg32(y0);
                const g1 = try fw.getOrPutValue(sa, x, y);
                if (!eql(g1.value_ptr.*, y)) return false;
                const g2 = try bw.getOrPutValue(sa, y, x);
                if (!eql(g2.value_ptr.*, x)) return false;
            }
        }
    }
    return true;
}

/// Within one hunk the most specific signature names it; the rest is taken to follow from it.
const PRIORITY = [_][]const u8{
    "gs-cookie", "callee-body", "call",      "ret-value",   "commute",       "ret-n",         "sign",    "width",
    "disp",      "imm",         "ref",       "callee-conv", "branch",        "insn",          "operand", "held-vs-reread",
    "missing",   "extra",       "copy",      "encoding",    "branch-layout", "frame-pointer", "spill",   "frame-size",
    "order",     "frame-slot",  "saved-reg", "reg",         "jmp-target",
};
const HUNK_GAP = 3;

fn rank(cat: []const u8) usize {
    for (PRIORITY, 0..) |p, k| if (eql(p, cat)) return k;
    return PRIORITY.len;
}

fn signature(cat: []const u8, key: []const u8, owner: ?[]const u8) !Sig {
    if ((eql(cat, "args") or eql(cat, "cleanup")) and try isCookie(key)) return .{ .cat = "gs-cookie", .key = "GS cookie" };
    if (eql(cat, "gs-cookie")) return .{ .cat = "gs-cookie", .key = "GS cookie" };
    // code VC8 inlined from another function of the unit, or a header
    if (truthy(owner) and !inSet(cat, &RIPPLE)) return .{ .cat = "callee-body", .key = owner.? };
    if (eql(cat, "args") or eql(cat, "cleanup")) return .{ .cat = "callee-conv", .key = key };
    return .{ .cat = cat, .key = key };
}

fn sigEql(a: Sig, b: Sig) bool {
    return eql(a.cat, b.cat) and eql(a.key, b.key);
}

fn putCause(cs: *Causes, sig: Sig, det: ?[]const u8) !void {
    const g = try cs.getOrPut(gpa, try sig.id());
    if (!g.found_existing) g.value_ptr.* = .{ .sig = sig, .det = "" };
    if (det) |d| if (g.value_ptr.det.len == 0 and !g.found_existing) {
        g.value_ptr.det = d;
    };
}

/// The function's cause signatures, ripple dropped when anything primary is there.
fn causes(f: *Fn) !Causes {
    var cs: Causes = .empty;
    const nm = if (std.mem.lastIndexOf(u8, f.name, "::")) |k| f.name[k + 2 ..] else f.name;
    if (startsWith(nm, "__GSHandlerCheck") or startsWith(nm, "SehHandler_") or startsWith(nm, "Unwind_") or startsWith(nm, "__unwindfunclet")) {
        try putCause(&cs, .{ .cat = "compiler-stub", .key = "EH/GS handler funclet" }, null);
        return cs;
    }
    // Group the items into hunks (ours positions at most HUNK_GAP apart); one signature each.
    const its = f.items.items;
    std.sort.block(Item, its, {}, struct {
        fn lt(_: void, a: Item, b: Item) bool {
            return a.pos < b.pos;
        }
    }.lt);
    var sigs: Causes = .empty;
    var h0: usize = 0;
    while (h0 < its.len) {
        var h1 = h0 + 1;
        while (h1 < its.len and its[h1].pos - its[h1 - 1].pos <= HUNK_GAP) h1 += 1;
        const h = its[h0..h1];
        var best: ?Sig = null;
        var best_rank: usize = 0;
        for (h) |it| {
            const s = try signature(it.cat, it.key, it.owner);
            const r = rank(s.cat);
            if (best == null or r < best_rank) {
                best = s;
                best_rank = r;
            }
        }
        var det: ?[]const u8 = null;
        for (h) |it| if (sigEql(try signature(it.cat, it.key, it.owner), best.?)) {
            det = it.det;
            break;
        };
        // A sig seen again keeps the example of its first hunk.
        const g = try sigs.getOrPut(gpa, try best.?.id());
        if (!g.found_existing) g.value_ptr.* = .{ .sig = best.?, .det = det.? };
        h0 = h1;
    }
    for (sigs.values()) |v| if (!inSet(v.sig.cat, &RIPPLE)) try cs.put(gpa, try v.sig.id(), v);
    // several stretches of one inlined callee, or several immediates of one kind, are one fact
    if (cs.count() > 0) return cs;
    if (sigs.count() == 0) {
        try putCause(&cs, .{ .cat = "none", .key = "no differing stretch" }, null);
        return cs;
    }
    if (try consistentPermutation(f)) {
        try putCause(&cs, try regChoice(f, "permutation"), null);
        return cs;
    }
    var cats: std.ArrayList([]const u8) = .empty;
    for (sigs.values()) |v| if (!inSet(v.sig.cat, cats.items)) try cats.append(sa, v.sig.cat);
    const subset = struct {
        fn of(xs: []const []const u8, set: []const []const u8) bool {
            for (xs) |x| if (!inSet(x, set)) return false;
            return true;
        }
    }.of;
    if (subset(cats.items, &.{ "reg", "jmp-target", "saved-reg" })) {
        try putCause(&cs, try regChoice(f, "registers"), null);
    } else if (subset(cats.items, &.{ "reg", "jmp-target", "saved-reg", "frame-size", "frame-slot", "spill" })) {
        try putCause(&cs, .{ .cat = "frame", .key = "slots/spills only" }, null);
    } else if (subset(cats.items, &.{ "order", "reg", "jmp-target", "saved-reg" })) {
        try putCause(&cs, .{ .cat = "order", .key = "order only" }, null);
    } else {
        const sorted = try sortedStrs(cats.items);
        try putCause(&cs, .{ .cat = "ripple-mix", .key = try std.mem.join(gpa, "+", sorted) }, null);
    }
    return cs;
}

/// The key with every original address that names a function followed by its name.
fn describe(out: *std.ArrayList(u8), key: []const u8) !void {
    var it = std.mem.splitScalar(u8, key, ' ');
    var first = true;
    while (it.next()) |w| {
        if (!first) try out.append(sa, ' ');
        first = false;
        try out.appendSlice(sa, w);
        const a = if (startsWith(w, "fn_") or startsWith(w, "fn:")) w[3..] else w;
        if (a.len != 8) continue;
        var hex = true;
        for (a) |ch| hex = hex and (std.ascii.isDigit(ch) or (ch >= 'a' and ch <= 'f'));
        if (!hex) continue;
        const n = names.get(a) orelse continue;
        const short = if (std.mem.lastIndexOf(u8, n, "::")) |k| n[k + 2 ..] else n;
        try out.print(sa, "({s})", .{short});
    }
}

const CALLEE_CLASS = [_][]const u8{ "callee-body", "callee-conv", "call", "callee-blocked" };
/// What a callee's register convention moves in a caller: VC8 gives a static function a custom
/// convention from its body (which registers it takes, clobbers and keeps), so a caller of a
/// callee that does not match yet allocates around a different contract.
const REGISTER_FAMILY = [_][]const u8{
    "reg-choice", "regtie",    "regalloc", "frame",      "order",      "ripple-mix", "copy", "held-vs-reread", "operand",
    "spill",      "saved-reg", "reg",      "jmp-target", "frame-slot", "frame-size",
};
/// Not a blocker anyone can fix in source: kept in functions.tsv, left out of the ranking.
const EXCLUDED = [_][]const u8{"compiler-stub"};
/// Signatures that name one thing a single fix changes (a callee, a constant, a field offset);
/// the rest are classes, fixed function by function.
const ONE_FIX = [_][]const u8{ "callee-blocked", "callee-body", "callee-conv", "call", "imm", "disp", "ret-n", "ref" };

fn calleesOf(f: *const Fn) ![]const []const u8 {
    var out: std.ArrayList([]const u8) = .empty;
    const Add = struct {
        fn one(o: *std.ArrayList([]const u8), text: []const u8, self: []const u8) !void {
            if (!startsWith(text, "call fn_") and !startsWith(text, "jmp fn_")) return;
            const k = std.mem.indexOf(u8, text, "fn_").? + 3;
            const a = text[k..@min(text.len, k + 8)];
            if (eql(a, self) or inSet(a, o.items)) return;
            try o.append(sa, a);
        }
    };
    for (f.O) |x| try Add.one(&out, x.text, f.a);
    for (f.R) |y| try Add.one(&out, y.text, f.a);
    return out.items;
}

/// A caller whose every cause is register-family (or the call of such a callee), calling
/// functions of its own unit that do not match: blocked by those callees, not by itself.
fn calleeBlocked(f: *const Fn, cs: Causes, tu_addrs: ?*const std.StringHashMapUnmanaged(void), nonmatch: *const std.StringHashMapUnmanaged([]const u8)) !Causes {
    var sc: std.ArrayList([]const u8) = .empty;
    for (try calleesOf(f)) |a| if (tu_addrs != null and tu_addrs.?.contains(a) and nonmatch.contains(a)) try sc.append(sa, a);
    if (sc.items.len == 0) return cs;
    for (cs.values()) |v| {
        if (inSet(v.sig.cat, &REGISTER_FAMILY)) continue;
        if (eql(v.sig.cat, "callee-conv") or eql(v.sig.cat, "call")) {
            var named = false;
            for (sc.items) |a| named = named or contains(v.sig.key, a);
            if (named) continue;
        }
        return cs;
    }
    var out: Causes = .empty;
    for (try sortedStrs(sc.items)) |a| try putCause(&out, .{ .cat = "callee-blocked", .key = try gpa.dupe(u8, a) }, null);
    return out;
}

// ---------------------------------------------------------------------------------------------
// Inputs.

/// Names from --funcmap/--libmap/--crtmap: the first row naming an address wins.
var names: std.StringHashMapUnmanaged([]const u8) = .empty;

fn loadNames(path: []const u8) !void {
    var it = TextLines{ .data = try util.readFile(path) };
    while (it.next()) |line0| {
        const line = try toLatin1(line0);
        const c = try util.split(line, '\t');
        if (c.len >= 3 and !eql(c[0], "start")) {
            const g = try names.getOrPut(gpa, c[0]);
            if (!g.found_existing) g.value_ptr.* = c[2];
        }
    }
}

const JV = std.json.Value;

fn jstr(v: JV) ![]const u8 {
    return switch (v) {
        .string => |s| try gpa.dupe(u8, try toLatin1(s)),
        else => error.BadAlignDump,
    };
}

fn jopt(v: JV) !?[]const u8 {
    return if (v == .null) null else try jstr(v);
}

fn jint(v: JV) !i64 {
    return switch (v) {
        .integer => |n| n,
        else => error.BadAlignDump,
    };
}

fn jtoks(v: JV) ![]const ?[]const u8 {
    const out = try gpa.alloc(?[]const u8, v.array.items.len);
    for (v.array.items, 0..) |t, k| out[k] = try jopt(t);
    return out;
}

const Input = struct {
    fns: std.ArrayList(Fn) = .empty,
    tus: std.StringHashMapUnmanaged([]const TuFn) = .empty,
    tu_addrs: std.StringHashMapUnmanaged(std.StringHashMapUnmanaged(void)) = .empty,
};

fn loadAlign(path: []const u8) !Input {
    var in: Input = .{};
    var line_arena = std.heap.ArenaAllocator.init(std.heap.c_allocator);
    defer line_arena.deinit();
    var it = TextLines{ .data = try util.readFile(path) };
    while (it.next()) |line| {
        if (pyStrip(line).len == 0) continue;
        _ = line_arena.reset(.retain_capacity);
        const d = try std.json.parseFromSliceLeaky(JV, line_arena.allocator(), line, .{});
        const o = d.object;
        if (o.get("tu") != null and o.get("fns") != null and o.get("a") == null) {
            const tu = try jstr(o.get("tu").?);
            var rows: std.ArrayList(TuFn) = .empty;
            var addrs: std.StringHashMapUnmanaged(void) = .empty;
            for (o.get("fns").?.array.items) |e| {
                const r = e.array.items;
                const t = TuFn{ .line = try jint(r[0]), .sym = try jstr(r[1]), .addr = try jopt(r[2]) };
                if (t.line != 0) try rows.append(gpa, t);
                if (truthy(t.addr)) try addrs.put(gpa, t.addr.?, {});
            }
            std.sort.block(TuFn, rows.items, {}, struct {
                fn lt(_: void, a: TuFn, b: TuFn) bool {
                    if (a.line != b.line) return a.line < b.line;
                    switch (std.mem.order(u8, a.sym, b.sym)) {
                        .lt => return true,
                        .gt => return false,
                        .eq => {},
                    }
                    if (a.addr == null or b.addr == null) return a.addr == null and b.addr != null;
                    return std.mem.order(u8, a.addr.?, b.addr.?) == .lt;
                }
            }.lt);
            try in.tus.put(gpa, tu, rows.items);
            try in.tu_addrs.put(gpa, tu, addrs);
            continue;
        }
        const ov = o.get("o").?.array.items;
        const O = try gpa.alloc(OIns, ov.len);
        for (ov, 0..) |e, k| {
            const r = e.array.items;
            const text = try jstr(r[2]);
            const key = try jopt(r[3]);
            O[k] = .{ .line = try jint(r[1]), .text = text, .key = if (truthy(key)) key.? else text, .hdr = try jopt(r[4]), .masked = try jstr(r[5]), .toks = try jtoks(r[6]) };
        }
        const rv = o.get("r").?.array.items;
        const R = try gpa.alloc(RIns, rv.len);
        for (rv, 0..) |e, k| {
            const r = e.array.items;
            const text = try jstr(r[1]);
            const key = try jopt(r[2]);
            R[k] = .{ .text = text, .key = if (truthy(key)) key.? else text, .masked = try jstr(r[3]), .toks = try jtoks(r[4]) };
        }
        const opv = o.get("ops").?.array.items;
        const ops = try gpa.alloc(Op, opv.len);
        for (opv, 0..) |e, k| {
            const r = e.array.items;
            const t = try jstr(r[0]);
            ops[k] = .{ .t = t[0], .i1 = @intCast(try jint(r[1])), .i2 = @intCast(try jint(r[2])), .j1 = @intCast(try jint(r[3])), .j2 = @intCast(try jint(r[4])), .marks = if (r.len > 5) try jstr(r[5]) else "" };
        }
        const a = try jstr(o.get("a").?);
        const e = try jstr(o.get("e").?);
        try in.fns.append(gpa, .{
            .a = a,
            .name = try jstr(o.get("name").?),
            .tu = try jstr(o.get("tu").?),
            .status = try jstr(o.get("status").?),
            .size = @as(i64, try std.fmt.parseInt(i64, e, 16)) - try std.fmt.parseInt(i64, a, 16),
            .O = O,
            .R = R,
            .ops = ops,
            .tufns = &.{},
        });
    }
    for (in.fns.items) |*f| {
        f.tufns = in.tus.get(f.tu) orelse &.{};
        f.init();
    }
    return in;
}

// ---------------------------------------------------------------------------------------------
// Outputs.

/// An insertion-ordered list of addresses with a count, as Python's lists and Counters keep them.
const Agg = struct {
    sig: Sig,
    sole: std.ArrayList([]const u8) = .empty,
    also: std.ArrayList([]const u8) = .empty,
    sole_bytes: i64 = 0,
    also_bytes: i64 = 0,
    ex: std.StringArrayHashMapUnmanaged([]const u8) = .empty,
};

const Cal = struct {
    sole: std.ArrayList([]const u8) = .empty,
    also: std.ArrayList([]const u8) = .empty,
    sole_bytes: i64 = 0,
    kinds: std.StringArrayHashMapUnmanaged(i64) = .empty,
};

fn byRank(comptime T: type, items: []T, comptime key: fn (*const T) [3]i64) void {
    std.sort.block(T, items, {}, struct {
        fn lt(_: void, a: T, b: T) bool {
            const ka = key(&a);
            const kb = key(&b);
            for (ka, kb) |x, y| if (x != y) return x < y;
            return false;
        }
    }.lt);
}

fn writeLatin1(path: []const u8, data: []const u8) !void {
    try util.writeFile(path, try proc.latin1ToUtf8(data));
}

/// Ratchet members kept at a body that does not match. `kept` in the regen's ratchet report
/// means the fresh generator body differs from the stored one: compile it (and the record's
/// caller with it) to see whether releasing gains. `equal` means releasing changes nothing
/// today, but the record will veto the next fix of that member until it is released.
fn frozen(store: []const u8, report: ?[]const u8, status: *const std.StringHashMapUnmanaged([]const u8), nm: *const std.StringHashMapUnmanaged([]const u8), outdir: []const u8) !void {
    var state: std.StringHashMapUnmanaged([]const u8) = .empty;
    if (report) |rp| {
        var it = TextLines{ .data = try util.readFile(rp) };
        while (it.next()) |line0| {
            const c = try util.split(try toLatin1(line0), '\t');
            if (c.len >= 4 and !eql(c[0], "status")) try state.put(gpa, try std.fmt.allocPrint(gpa, "{s}\x00{s}", .{ c[1], c[3] }), c[0]);
        }
    }
    const Row = struct { order: u8, f: [8][]const u8 };
    var rows: std.ArrayList(Row) = .empty;
    const dir = try util.join(store, "fn");
    const files = try sortedStrs(try proc.listDir(dir));
    var arena = std.heap.ArenaAllocator.init(std.heap.c_allocator);
    defer arena.deinit();
    for (files) |fname| {
        if (!std.mem.endsWith(u8, fname, ".json")) continue;
        const text = try util.readFile(try util.join(dir, fname));
        const d = (try std.json.parseFromSliceLeaky(JV, arena.allocator(), text, .{})).object;
        const rec = try jstr(d.get("address").?);
        const kind = if (d.get("kind")) |k| try jstr(k) else "";
        const members = if (d.get("members")) |m| m.array.items else &.{};
        for (members, 0..) |m, k| {
            const a = try jstr(m.object.get("address").?);
            const st_fn = status.get(a) orelse continue;
            const st = state.get(try std.fmt.allocPrint(gpa, "{s}\x00{s}", .{ a, rec })) orelse "unknown";
            const own = nm.get(a) orelse if (m.object.get("name")) |n| try jstr(n) else "";
            try rows.append(gpa, .{ .order = if (eql(st, "kept")) 0 else 1, .f = .{ a, own, st_fn, rec, nm.get(rec) orelse "", kind, if (k == 0) "caller" else "member", st } });
        }
    }
    std.sort.block(Row, rows.items, {}, struct {
        fn lt(_: void, a: Row, b: Row) bool {
            if (a.order != b.order) return a.order < b.order;
            for (a.f, b.f) |x, y| switch (std.mem.order(u8, x, y)) {
                .lt => return true,
                .gt => return false,
                .eq => {},
            };
            return false;
        }
    }.lt);
    var o: std.ArrayList(u8) = .empty;
    try o.appendSlice(gpa, "member\tname\tstatus\trecord\trecord_name\tkind\trole\tratchet\n");
    for (rows.items) |r| {
        for (r.f, 0..) |x, k| {
            if (k > 0) try o.append(gpa, '\t');
            try o.appendSlice(gpa, x);
        }
        try o.append(gpa, '\n');
    }
    try writeLatin1(try util.join(outdir, "frozen.tsv"), o.items);
}

pub fn main(args: []const []const u8) !void {
    const p = cli.parse(args, &.{
        .{ .long = "--funcmap", .many = true },
        .{ .long = "--libmap", .many = true },
        .{ .long = "--crtmap", .many = true },
        .{ .long = "--ratchet" },
        .{ .long = "--ratchet-report" },
    }, USAGE);
    if (p.pos.len != 2) {
        std.debug.print("{s}", .{USAGE});
        std.process.exit(2);
    }
    for ([_][]const u8{ "--funcmap", "--libmap", "--crtmap" }) |flag| for (p.all(flag)) |path| try loadNames(path);
    const outdir = p.pos[1];
    util.makeDirs(outdir);
    var in = try loadAlign(p.pos[0]);
    const fns = in.fns.items;

    var local: std.StringHashMapUnmanaged([]const u8) = .empty; // address -> our name
    var status: std.StringHashMapUnmanaged([]const u8) = .empty;
    for (fns) |*f| {
        try local.put(gpa, f.a, f.name);
        try status.put(gpa, f.a, f.status);
    }
    var agg: std.StringArrayHashMapUnmanaged(Agg) = .empty;
    const per_fn = try gpa.alloc(Causes, fns.len);
    for (fns, 0..) |*f, k| {
        try analyse(f);
        var cs = try causes(f);
        cs = try calleeBlocked(f, cs, in.tu_addrs.getPtr(f.tu), &local);
        _ = scratch_state.reset(.retain_capacity);
        per_fn[k] = cs;
        const sole = cs.count() == 1;
        for (cs.values()) |v| {
            const g = try agg.getOrPut(gpa, try v.sig.id());
            if (!g.found_existing) g.value_ptr.* = .{ .sig = v.sig };
            const a = g.value_ptr;
            if (sole) {
                try a.sole.append(gpa, f.a);
                a.sole_bytes += f.size;
            } else {
                try a.also.append(gpa, f.a);
                a.also_bytes += f.size;
            }
            if (a.ex.count() < 3 and (sole or a.ex.count() < 1)) try a.ex.put(gpa, f.a, v.det);
        }
    }

    // index.tsv
    {
        var rows: std.ArrayList(*Agg) = .empty;
        for (agg.values()) |*a| if (!inSet(a.sig.cat, &EXCLUDED)) try rows.append(gpa, a);
        byRank(*Agg, rows.items, struct {
            fn k(a: *const *Agg) [3]i64 {
                return .{ -@as(i64, @intCast(a.*.sole.items.len)), -a.*.sole_bytes, -@as(i64, @intCast(a.*.also.items.len)) };
            }
        }.k);
        var o: std.ArrayList(u8) = .empty;
        try o.appendSlice(gpa, "cause\tcategory\tsole_fns\tsole_bytes\talso_fns\talso_bytes\tscope\texample_addrs\texample_diff\n");
        for (rows.items) |g| {
            var ex: []const []const u8 = undefined;
            if (g.sole.items.len > 0) {
                const s = try gpa.dupe([]const u8, g.sole.items);
                const Ctx = struct {
                    nm: *const std.StringHashMapUnmanaged([]const u8),
                    fn lt(c: @This(), a: []const u8, b: []const u8) bool {
                        return std.mem.order(u8, c.nm.get(a) orelse "", c.nm.get(b) orelse "") == .lt;
                    }
                };
                std.sort.block([]const u8, s, Ctx{ .nm = &local }, Ctx.lt);
                ex = s[0..@min(5, s.len)];
            } else ex = g.ex.keys()[0..@min(3, g.ex.count())];
            var det: []const u8 = "";
            for (g.ex.values()) |v| if (v.len > 0) {
                det = v;
                break;
            };
            var label: std.ArrayList(u8) = .empty;
            try describe(&label, g.sig.key);
            try clean(&o, label.items, std.math.maxInt(usize));
            try o.print(gpa, "\t{s}\t{d}\t{d}\t{d}\t{d}\t{s}\t", .{
                g.sig.cat,                                              g.sole.items.len, g.sole_bytes, g.also.items.len, g.also_bytes,
                if (inSet(g.sig.cat, &ONE_FIX)) "one fix" else "class",
            });
            for (ex, 0..) |a, k| {
                if (k > 0) try o.append(gpa, ',');
                try o.appendSlice(gpa, a);
            }
            try o.append(gpa, '\t');
            try clean(&o, det, 140);
            try o.append(gpa, '\n');
        }
        _ = scratch_state.reset(.retain_capacity);
        try writeLatin1(try util.join(outdir, "index.tsv"), o.items);
    }

    // callees.tsv: static callees, how many callers each alone holds back
    var merged: std.StringHashMapUnmanaged([]const u8) = .empty; // dict(NAMES, **names)
    {
        var it = names.iterator();
        while (it.next()) |e| try merged.put(gpa, e.key_ptr.*, e.value_ptr.*);
        var il = local.iterator();
        while (il.next()) |e| try merged.put(gpa, e.key_ptr.*, e.value_ptr.*);
    }
    {
        var cal: std.StringArrayHashMapUnmanaged(Cal) = .empty;
        for (fns, per_fn) |*f, cs| {
            var tgts: std.StringArrayHashMapUnmanaged(std.StringArrayHashMapUnmanaged(void)) = .empty;
            const Put = struct {
                fn one(t: *std.StringArrayHashMapUnmanaged(std.StringArrayHashMapUnmanaged(void)), target: []const u8, kind: []const u8) !void {
                    const g = try t.getOrPut(gpa, target);
                    if (!g.found_existing) g.value_ptr.* = .empty;
                    try g.value_ptr.put(gpa, kind, {});
                }
            };
            var n_callee: usize = 0;
            for (cs.values()) |v| {
                const cat = v.sig.cat;
                const key = v.sig.key;
                if (inSet(cat, &CALLEE_CLASS)) n_callee += 1;
                if (eql(cat, "callee-blocked")) {
                    try Put.one(&tgts, key, "register contract");
                } else if (eql(cat, "callee-body") and startsWith(key, "fn:")) {
                    try Put.one(&tgts, key[3..], "inlined body");
                } else if (eql(cat, "callee-conv")) {
                    try Put.one(&tgts, try std.mem.replaceOwned(u8, gpa, key, "fn_", ""), "call args/cleanup");
                } else if (eql(cat, "call")) {
                    const sp = splitFirst(key);
                    try Put.one(&tgts, try std.mem.replaceOwned(u8, gpa, sp[1], "fn_", ""), sp[0]);
                }
            }
            // an indirect call (through a register or memory) names no callee
            var kept: std.ArrayList(usize) = .empty;
            for (tgts.keys(), 0..) |t, k| if (!isReg(t) and !contains(t, "[") and !contains(t, "ptr")) try kept.append(gpa, k);
            const only = cs.count() == n_callee and kept.items.len == 1;
            for (kept.items) |k| {
                const g = try cal.getOrPut(gpa, tgts.keys()[k]);
                if (!g.found_existing) g.value_ptr.* = .{};
                const c = g.value_ptr;
                if (only) {
                    try c.sole.append(gpa, f.a);
                    c.sole_bytes += f.size;
                } else try c.also.append(gpa, f.a);
                for (tgts.values()[k].keys()) |kind| (try c.kinds.getOrPutValue(gpa, kind, 0)).value_ptr.* += 1;
            }
        }
        const Row = struct { t: []const u8, c: *Cal };
        var rows: std.ArrayList(Row) = .empty;
        for (cal.keys(), cal.values()) |t, *c| try rows.append(gpa, .{ .t = t, .c = c });
        byRank(Row, rows.items, struct {
            fn k(r: *const Row) [3]i64 {
                return .{ -@as(i64, @intCast(r.c.sole.items.len)), -r.c.sole_bytes, -@as(i64, @intCast(r.c.also.items.len)) };
            }
        }.k);
        var o: std.ArrayList(u8) = .empty;
        try o.appendSlice(gpa, "callee\tname\tstatus\tsole_callers\tsole_bytes\tother_callers\tkinds\texample_callers\n");
        for (rows.items) |r| {
            const c = r.c;
            try o.print(gpa, "{s}\t{s}\t{s}\t{d}\t{d}\t{d}\t", .{ r.t, merged.get(r.t) orelse "", status.get(r.t) orelse "MATCH?", c.sole.items.len, c.sole_bytes, c.also.items.len });
            // Counter.most_common: by count, ties in first-seen order
            const KR = struct { k: []const u8, n: i64 };
            var ks: std.ArrayList(KR) = .empty;
            for (c.kinds.keys(), c.kinds.values()) |kk, n| try ks.append(gpa, .{ .k = kk, .n = n });
            std.sort.block(KR, ks.items, {}, struct {
                fn lt(_: void, a: KR, b: KR) bool {
                    return a.n > b.n;
                }
            }.lt);
            for (ks.items, 0..) |kr, k| {
                if (k > 0) try o.append(gpa, ',');
                try o.print(gpa, "{s}:{d}", .{ kr.k, kr.n });
            }
            try o.append(gpa, '\t');
            const ex = if (c.sole.items.len > 0) c.sole.items[0..@min(6, c.sole.items.len)] else c.also.items[0..@min(3, c.also.items.len)];
            for (ex, 0..) |a, k| {
                if (k > 0) try o.append(gpa, ',');
                try o.appendSlice(gpa, a);
            }
            try o.append(gpa, '\n');
        }
        try writeLatin1(try util.join(outdir, "callees.tsv"), o.items);
    }

    // functions.tsv
    {
        var o: std.ArrayList(u8) = .empty;
        try o.appendSlice(gpa, "addr\tname\tstatus\tbytes\tn_causes\tcauses\n");
        for (fns, per_fn) |*f, cs| {
            try o.print(gpa, "{s}\t{s}\t{s}\t{d}\t{d}\t", .{ f.a, f.name, f.status, f.size, cs.count() });
            var s: std.ArrayList(u8) = .empty;
            for (cs.values(), 0..) |v, k| {
                if (k > 0) try s.appendSlice(gpa, " | ");
                try s.print(gpa, "{s}:{s}", .{ v.sig.cat, v.sig.key });
            }
            try clean(&o, s.items, std.math.maxInt(usize));
            try o.append(gpa, '\n');
        }
        try writeLatin1(try util.join(outdir, "functions.tsv"), o.items);
    }

    if (p.get("--ratchet")) |store| try frozen(store, p.get("--ratchet-report"), &status, &merged, outdir);
    var nsole: usize = 0;
    for (per_fn) |cs| {
        if (cs.count() == 1) nsole += 1;
    }
    util.say(try std.fmt.allocPrint(gpa, "{d} functions, {d} with one cause, {d} causes\n", .{ fns.len, nsole, agg.count() }));
}

// ---------------------------------------------------------------------------------------------

const testing = std.testing;

fn testFn(o: []const OIns, r: []const RIns, ops: []const Op) Fn {
    return .{ .a = "00401000", .name = "T::f", .tu = "t", .status = "OUR+0", .size = 16, .O = o, .R = r, .ops = ops, .tufns = &.{} };
}

fn tO(text: []const u8) OIns {
    return .{ .line = 0, .text = text, .key = text, .hdr = null, .masked = text, .toks = &.{} };
}

fn tR(text: []const u8) RIns {
    return .{ .text = text, .key = text, .masked = text, .toks = &.{} };
}

test "python int and hex" {
    try testing.expectEqual(@as(?i128, 16), pyInt("0x10"));
    try testing.expectEqual(@as(?i128, -16), pyInt("-0x10"));
    try testing.expectEqual(@as(?i128, 31), pyInt(" 0x_1f "));
    try testing.expectEqual(@as(?i128, 0), pyInt("0_0"));
    try testing.expectEqual(@as(?i128, 5), pyInt("0b101"));
    try testing.expectEqual(@as(?i128, null), pyInt("010"));
    try testing.expectEqual(@as(?i128, null), pyInt("1__0"));
    try testing.expectEqual(@as(?i128, null), pyInt("0x"));
    try testing.expectEqual(@as(?i128, null), pyInt("eax"));
    try testing.expectEqualStrings("-10", try hexStr(-16));
    try testing.expectEqualStrings("fffffff0", try hexStr(mask32(-16)));
}

test "tokenize and operands" {
    const t = try tokenize("  rep movsd dword ptr es:[edi], dword ptr [esi] ");
    try testing.expectEqualStrings("rep movsd", t.m);
    try testing.expectEqual(@as(usize, 2), t.ops.len);
    const m = (try parseOp("dword ptr [ecx + eax*4 - 0x10]")).mem;
    try testing.expectEqualStrings("dword", m.size);
    try testing.expectEqualStrings("ecx", m.base.?);
    try testing.expectEqualStrings("eax", m.index.?);
    try testing.expectEqual(@as(i128, 4), m.scale);
    try testing.expectEqual(@as(i128, -16), m.disp);
    try testing.expectEqualStrings("", (try parseOp("dword ptr es:[edi]")).mem.size);
    try testing.expectEqualStrings("mov R,dword[R+R*4]", try shape("mov eax, dword ptr [ecx + edx*4]"));
    try testing.expectEqualStrings("push S", try shape("push fn_00401000"));
    try testing.expectEqualStrings("mov R, dword ptr [R + 8] ", try stripRegs("mov eax, dword ptr [ecx + 8]"));
    try testing.expectEqualStrings("[+0x8]", try memkey("mov eax, dword ptr [ecx + 8]"));
    try testing.expectEqualStrings("frame", try memkey("mov eax, dword ptr [ebp - 4]"));
}

test "latin-1 str semantics" {
    try testing.expect(isAlnum(0xe8));
    try testing.expect(!isAlnum(0xa5));
    try testing.expectEqualStrings("x", pyStrip("\x1c\xa0x\x85"));
    var o: std.ArrayList(u8) = .empty;
    try clean(&o, "a\tb\xe8\x7f", 10);
    try testing.expectEqualStrings("a b??", o.items);
}

test "pair classes" {
    const o = [_]OIns{ tO("jl 0x10"), tO("mov eax, dword ptr [ecx + 8]"), tO("ret 4"), tO("mov eax, ecx"), tO("movzx eax, byte ptr [ecx + 4]") };
    const r = [_]RIns{ tR("jb 0x10"), tR("mov eax, dword ptr [ecx + 0xc]"), tR("ret 8"), tR("mov edx, ecx"), tR("mov eax, dword ptr [ecx + 4]") };
    const f = testFn(&o, &r, &.{});
    const want = [_][2][]const u8{
        .{ "sign", "jl/jb" },
        .{ "disp", "dword [+0x8]->[+0xc]" },
        .{ "ret-n", "ret 4->8" },
        .{ "reg", "renamed" },
        .{ "width", "movzx->mov [+0x4]" },
    };
    for (want, 0..) |w, k| {
        const c = try classifyPair(&f, k, k);
        try testing.expectEqualStrings(w[0], c.cat);
        try testing.expectEqualStrings(w[1], c.key);
    }
}

test "causes: a register-only function names its value source" {
    const o = [_]OIns{ tO("mov ecx, dword ptr [esi + 0x14]"), tO("ret") };
    const r = [_]RIns{ tR("mov edx, dword ptr [esi + 0x14]"), tR("ret") };
    const ops = [_]Op{ .{ .t = 'r', .i1 = 0, .i2 = 1, .j1 = 0, .j2 = 1 }, .{ .t = 'e', .i1 = 1, .i2 = 2, .j1 = 1, .j2 = 2, .marks = " " } };
    var f = testFn(&o, &r, &ops);
    try analyse(&f);
    const cs = try causes(&f);
    try testing.expectEqual(@as(usize, 1), cs.count());
    try testing.expectEqualStrings("reg-choice", cs.values()[0].sig.cat);
    try testing.expectEqualStrings("permutation: field [+0x14] dword", cs.values()[0].sig.key);
}

test "causes: a one-sided call and its argument form one hunk" {
    const o = [_]OIns{ tO("push 1"), tO("call fn_00402000"), tO("ret") };
    const r = [_]RIns{tR("ret")};
    const ops = [_]Op{ .{ .t = 'd', .i1 = 0, .i2 = 2, .j1 = 0, .j2 = 0 }, .{ .t = 'e', .i1 = 2, .i2 = 3, .j1 = 0, .j2 = 1, .marks = " " } };
    var f = testFn(&o, &r, &ops);
    try analyse(&f);
    const cs = try causes(&f);
    try testing.expectEqual(@as(usize, 1), cs.count());
    try testing.expectEqualStrings("call", cs.values()[0].sig.cat);
    try testing.expectEqualStrings("ours-only fn_00402000", cs.values()[0].sig.key);
}
