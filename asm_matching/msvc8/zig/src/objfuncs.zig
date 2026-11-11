//! Our side of the comparison: the functions in one VC8 object (objfuncs.py). Each function
//! is a COMDAT code section (/O2 implies /Gy), so its extent is exact. Relocations are
//! resolved to what they name, so a function compares by what it refers to, not by where.
const std = @import("std");
const coff = @import("coff.zig");
const dec = @import("decode.zig");
const util = @import("util.zig");

const gpa = util.gpa;

/// What a relocation names. Ordered as Python sorts the kind strings.
pub const Kind = enum {
    call,
    data,
    fptr,
    import,
    other,
    self,
    string,

    pub fn order(a: Kind, b: Kind) std.math.Order {
        return std.math.order(@backingInt(a), @backingInt(b));
    }
};

/// (offset, kind, target): `int` for self (the offset into the function), `s` otherwise
/// (a symbol, an import's name, a string literal's bytes).
pub const OurRef = struct {
    off: u32,
    kind: Kind,
    int: u32 = 0,
    s: []const u8 = "",

    pub fn lessThan(_: void, a: OurRef, b: OurRef) bool {
        if (a.off != b.off) return a.off < b.off;
        if (a.kind != b.kind) return a.kind.order(b.kind) == .lt;
        if (a.kind == .self) return a.int < b.int;
        return std.mem.order(u8, a.s, b.s) == .lt;
    }
};

pub const Line = struct { off: u32, file: []const u8, line: u32 };

pub const Function = struct {
    obj: []const u8,
    symbol: []const u8, // mangled
    name: []const u8, // qualified, readable
    static: bool,
    section: *const coff.Section,
    code: []const u8,
    /// offset -> what the relocation there names (the last one at an offset wins).
    relocs: std.AutoHashMapUnmanaged(u32, OurRef) = .empty,
    refs: []OurRef = &.{}, // in code order
    decoded: dec.Function = undefined,
    lines: []Line = &.{}, // from /Z7

    pub fn relocAt(f: *const Function, addr: u32) ?u32 {
        const k = f.relocs.get(addr) orelse return null;
        return if (k.kind == .self) k.int else null;
    }

    pub fn externalAt(f: *const Function, addr: u32) bool {
        const k = f.relocs.get(addr) orelse return false;
        return k.kind == .call or k.kind == .import;
    }
};

fn isPyDigit(ch: u8) bool {
    // str.isdigit() over latin-1: the ASCII digits and the superscripts two, three and one.
    return (ch >= '0' and ch <= '9') or ch == 0xB2 or ch == 0xB3 or ch == 0xB9;
}

fn isUpperOrDigit(ch: u8) bool {
    return (ch >= 'A' and ch <= 'Z') or (ch >= '0' and ch <= '9');
}

/// Python slicing s[a:b] with b possibly -1 (find's "not found").
fn pyslice(s: []const u8, a: usize, b: isize) []const u8 {
    const n: isize = @intCast(s.len);
    var e: isize = if (b < 0) n + b else b;
    if (e < 0) e = 0;
    if (e > n) e = n;
    const st = @min(a, s.len);
    return if (@as(usize, @intCast(e)) <= st) "" else s[st..@intCast(e)];
}

fn find(s: []const u8, ch: u8, from: usize) isize {
    if (from >= s.len) return -1;
    return if (std.mem.indexOfScalarPos(u8, s, from, ch)) |i| @intCast(i) else -1;
}

/// `?Name@Ns2@Ns1@@...` -> 'Ns1::Ns2::Name' (enough to match by name). C names lose their
/// leading underscore and stdcall/fastcall decoration.
pub fn qualified(mangled: []const u8) ![]const u8 {
    if (mangled.len == 0 or mangled[0] != '?') {
        const n = if (mangled.len == 0 or mangled[0] == '_' or mangled[0] == '@') mangled[@min(1, mangled.len)..] else mangled;
        // re.sub(r'@\d+$', '', n)
        if (std.mem.lastIndexOfScalar(u8, n, '@')) |i| {
            const tail = n[i + 1 ..];
            var digits = tail.len > 0;
            for (tail) |ch| if (ch < '0' or ch > '9') {
                digits = false;
            };
            if (digits) return n[0..i];
        }
        return n;
    }
    var body = mangled[1..];
    var special: []const u8 = "";
    if (body.len > 0 and body[0] == '?') {
        // ??0 ctor, ??1 dtor, ??_G scalar deleting dtor, ??4 operator=, ...
        if (body.len >= 3 and body[1] == '_' and isUpperOrDigit(body[2])) {
            special = body[0..3];
        } else if (body.len >= 2 and isUpperOrDigit(body[1])) {
            special = body[0..2];
        } else special = body[0..1];
        body = body[special.len..];
    }
    var parts: std.ArrayList([]const u8) = .empty;
    var i: usize = 0;
    while (i < body.len and body[i] != '@') {
        if (std.mem.startsWith(u8, body[i..], "?$")) { // template name: ?$Name@args@
            const j = find(body, '@', i + 2);
            try parts.append(gpa, try std.fmt.allocPrint(gpa, "{s}<>", .{pyslice(body, i + 2, j)}));
            var k: usize = @intCast(j + 1);
            while (k < body.len) : (k += 1) if (body[k] == '@') break;
            i = k + 1;
            continue;
        }
        const j = find(body, '@', i);
        if (j < 0) break;
        var seg = body[i..@intCast(j)];
        if (seg.len > 0 and isPyDigit(seg[0])) seg = try std.fmt.allocPrint(gpa, "#{s}", .{seg}); // back-reference
        try parts.append(gpa, seg);
        i = @as(usize, @intCast(j)) + 1;
    }
    std.mem.reverse([]const u8, parts.items);
    if (special.len > 0) {
        const cls = if (parts.items.len > 0) parts.items[parts.items.len - 1] else "?";
        const sp = special[1..];
        const name = if (std.mem.eql(u8, sp, "0"))
            cls
        else if (std.mem.eql(u8, sp, "1"))
            try std.fmt.allocPrint(gpa, "~{s}", .{cls})
        else if (std.mem.eql(u8, sp, "_G"))
            "`scalar deleting dtor`"
        else if (std.mem.eql(u8, sp, "_E"))
            "`vector deleting dtor`"
        else if (std.mem.eql(u8, sp, "4"))
            "operator="
        else
            try std.fmt.allocPrint(gpa, "operator{s}", .{special});
        try parts.append(gpa, name);
    }
    var out: std.ArrayList(u8) = .empty;
    for (parts.items) |p| if (p.len > 0) {
        if (out.items.len > 0) try out.appendSlice(gpa, "::");
        try out.appendSlice(gpa, p);
    };
    return out.items;
}

/// `__imp__Name@N` -> Name (objfuncs._IMP), else null.
pub fn importName(sym: []const u8) ?[]const u8 {
    if (!std.mem.startsWith(u8, sym, "__imp__")) return null;
    const rest = sym[7..];
    if (rest.len == 0) return null;
    if (std.mem.lastIndexOfScalar(u8, rest, '@')) |i| if (i >= 1) {
        const tail = rest[i + 1 ..];
        var digits = tail.len > 0;
        for (tail) |ch| if (ch < '0' or ch > '9') {
            digits = false;
        };
        if (digits) return rest[0..i];
    };
    return rest;
}

fn rd32(d: []const u8, o: usize) !u32 {
    if (o + 4 > d.len) return error.Truncated;
    return std.mem.readInt(u32, d[o..][0..4], .little);
}

/// symbol -> [(function-relative offset, source file, line)] from the CodeView C13 line
/// tables /Z7 writes into .debug$S.
pub fn lineTables(o: *const coff.Object) !std.StringArrayHashMapUnmanaged([]Line) {
    var strings: []const u8 = "";
    var checks: []const u8 = "";
    const Raw = struct { off: u32, fid: u32, ln: u32 };
    var raw: std.StringArrayHashMapUnmanaged(std.ArrayList(Raw)) = .empty;
    for (o.sections[1..]) |*sec| {
        if (!std.mem.startsWith(u8, sec.name, ".debug$S")) continue;
        const d = sec.data;
        if (d.len < 4 or try rd32(d, 0) != 4) continue;
        var relocs: std.AutoHashMapUnmanaged(u32, coff.Reloc) = .empty;
        for (sec.relocs) |r| try relocs.put(gpa, r.offset, r);
        var p: usize = 4;
        while (p + 8 <= d.len) {
            const t = try rd32(d, p);
            const n = try rd32(d, p + 4);
            const body = p + 8;
            if (t == 0xF3) {
                strings = d[@min(body, d.len)..@min(body + n, d.len)];
            } else if (t == 0xF4) {
                checks = d[@min(body, d.len)..@min(body + n, d.len)];
            } else if (t == 0xF2 and relocs.contains(@intCast(body))) {
                const sym = (try o.symbol(relocs.get(@intCast(body)).?.symbol)).name;
                const off = try rd32(d, body);
                var q = body + 12;
                const end = body + n;
                while (q + 12 <= end) {
                    const nl = try rd32(d, q + 4);
                    const cbb = try rd32(d, q + 8);
                    const fid = try rd32(d, q);
                    for (0..nl) |i| {
                        const lo = try rd32(d, q + 12 + 8 * i);
                        const ln = try rd32(d, q + 16 + 8 * i);
                        const g = try raw.getOrPut(gpa, sym);
                        if (!g.found_existing) g.value_ptr.* = .empty;
                        try g.value_ptr.append(gpa, .{ .off = off +% lo, .fid = fid, .ln = ln & 0xffffff });
                    }
                    q += @max(cbb, 12);
                }
            }
            p = (body + n + 3) & ~@as(usize, 3);
        }
    }
    var out: std.StringArrayHashMapUnmanaged([]Line) = .empty;
    var it = raw.iterator();
    while (it.next()) |e| {
        const ls = try gpa.alloc(Line, e.value_ptr.items.len);
        for (e.value_ptr.items, 0..) |x, k| {
            var fname: []const u8 = "?";
            if (@as(usize, x.fid) + 4 <= checks.len) {
                const no = std.mem.readInt(u32, checks[x.fid..][0..4], .little);
                if (no < strings.len) if (std.mem.indexOfScalarPos(u8, strings, no, 0)) |e2| {
                    fname = strings[no..e2];
                };
            }
            ls[k] = .{ .off = x.off, .file = fname, .line = x.ln };
        }
        std.mem.sort(Line, ls, {}, struct {
            fn lt(_: void, a: Line, b: Line) bool {
                if (a.off != b.off) return a.off < b.off;
                const o2 = std.mem.order(u8, a.file, b.file);
                if (o2 != .eq) return o2 == .lt;
                return a.line < b.line;
            }
        }.lt);
        try out.put(gpa, e.key_ptr.*, ls);
    }
    return out;
}

/// Bytes of a string-literal COMDAT (??_C@...), else null.
fn stringOf(o: *const coff.Object, sym: *const coff.Symbol) ?[]const u8 {
    if (sym.section <= 0 or !std.mem.startsWith(u8, sym.name, "??_C@")) return null;
    const sec = &o.sections[@intCast(sym.section)];
    const data = sec.data[@min(sym.value, sec.data.len)..];
    if (std.mem.startsWith(u8, sym.name, "??_C@_1")) { // wide: up to the first 0x0000 unit
        var k: usize = 0;
        while (k + 1 < data.len) : (k += 2) if (data[k] == 0 and data[k + 1] == 0) return data[0 .. k + 2];
        return data;
    }
    const k = std.mem.indexOfScalar(u8, data, 0) orelse return data;
    return data[0 .. k + 1];
}

/// Functions of the object `data` named `path`. labels: see coff.Object.functions.
pub fn functions(d: *dec.Decoder, path: []const u8, data: []const u8, labels: bool) ![]Function {
    const o = try gpa.create(coff.Object);
    o.* = try coff.Object.parse(data, path);
    var out: std.ArrayList(Function) = .empty;
    var lines = try lineTables(o);
    // Code addresses inside a function's section that other sections hold (EH unwind/catch
    // maps, SEH scope tables): extra descent roots.
    var roots: std.AutoHashMapUnmanaged(u32, std.AutoArrayHashMapUnmanaged(u64, void)) = .empty;
    for (o.sections[1..]) |*sec| {
        for (sec.relocs) |r| {
            const s = try o.symbol(r.symbol);
            if (r.type == coff.DIR32 and s.section > 0 and o.sections[@intCast(s.section)].isCode() and @as(u32, @intCast(s.section)) != sec.number) {
                const addend = try rd32(sec.data, r.offset);
                const g = try roots.getOrPut(gpa, @intCast(s.section));
                if (!g.found_existing) g.value_ptr.* = .empty;
                try g.value_ptr.put(gpa, @as(u64, s.value) + addend, {});
            }
        }
    }
    const funcs = try o.functions(labels);
    // Function starts by (section, offset): a section of an assembler or C library object
    // holds several functions.
    var starts: std.AutoHashMapUnmanaged(u64, []const u8) = .empty;
    for (funcs) |fr| try starts.put(gpa, (@as(u64, fr.sec.number) << 32) | fr.start, fr.sym.name);
    for (funcs) |fr| {
        if (!fr.sec.isCode()) continue;
        const start = fr.start;
        const end = fr.end;
        var f = Function{ .obj = path, .symbol = fr.sym.name, .name = try qualified(fr.sym.name), .static = fr.sym.storage == coff.C_STATIC, .section = fr.sec, .code = fr.sec.data[@min(start, fr.sec.data.len)..@min(end, fr.sec.data.len)] };
        f.lines = lines.get(fr.sym.name) orelse &.{};
        var refs: std.ArrayList(OurRef) = .empty;
        for (fr.sec.relocs) |r| {
            if (!(start <= r.offset and r.offset < end)) continue;
            const s = try o.symbol(r.symbol);
            var k = OurRef{ .off = r.offset - start, .kind = .other, .s = s.name };
            if (r.type == coff.REL32) {
                if (importName(s.name)) |m| {
                    k.kind = .import;
                    k.s = m;
                } else k.kind = .call;
            } else if (r.type == coff.DIR32) {
                if (importName(s.name)) |m| {
                    k.kind = .import;
                    k.s = m;
                } else if (stringOf(o, s)) |st| {
                    k.kind = .string;
                    k.s = st;
                } else if (s.section > 0 and @as(u32, @intCast(s.section)) == fr.sec.number) {
                    // Modulo 2**32: against a label after the target the addend is negative.
                    const at = s.value +% try rd32(fr.sec.data, r.offset);
                    const other: ?[]const u8 = if (!(start <= at and at < end)) starts.get((@as(u64, fr.sec.number) << 32) | at) else null;
                    if (other != null and other.?.len > 0) {
                        k.kind = .fptr;
                        k.s = other.?;
                    } else {
                        k.kind = .self;
                        k.int = at -% start;
                        k.s = "";
                    }
                } else if (s.section > 0 and o.sections[@intCast(s.section)].isCode() and s.isFunction()) {
                    k.kind = .fptr;
                } else k.kind = .data;
            }
            try f.relocs.put(gpa, k.off, k);
            try refs.append(gpa, k);
        }
        std.mem.sort(OurRef, refs.items, {}, OurRef.lessThan);
        f.refs = refs.items;
        f.decoded = try d.decode(gpa, f.code, 0, 0, @intCast(f.code.len), &f, null);
        var extra: std.ArrayList(u32) = .empty;
        if (roots.get(fr.sec.number)) |set| for (set.keys()) |x| if (start < x and x < end) try extra.append(gpa, @intCast(x - start));
        std.mem.sort(u32, extra.items, {}, std.sort.asc(u32));
        for (extra.items) |rt| {
            const g = try d.decode(gpa, f.code, 0, rt, @intCast(f.code.len), &f, null);
            try f.decoded.mergeRoot(gpa, &g, false);
        }
        f.decoded.sortInsns();
        try out.append(gpa, f);
    }
    return out.items;
}
