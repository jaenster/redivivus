//! MSVC i386 COFF objects and archives (coffobj.py): sections, symbols, relocations.
//! Our side of every comparison is an object straight out of cl.exe, never a linked image:
//! each relocation names the symbol it binds.
const std = @import("std");
const util = @import("util.zig");

const gpa = util.gpa;

pub const DIR32 = 0x06;
pub const DIR32NB = 0x07;
pub const SECTION = 0x0A;
pub const SECREL = 0x0B;
pub const TOKEN = 0x0C;
pub const SECREL7 = 0x0D;
pub const REL32 = 0x14;

pub const SCN_CNT_CODE = 0x20;
pub const SCN_LNK_COMDAT = 0x1000;
pub const SCN_MEM_EXECUTE = 0x20000000;

pub const C_EXTERNAL = 2;
pub const C_STATIC = 3;
pub const C_LABEL = 6;
pub const C_FUNCTION = 101;
pub const C_FILE = 103;

pub const Reloc = struct { offset: u32, symbol: u32, type: u16 };

pub const Symbol = struct {
    index: u32,
    name: []const u8,
    value: u32,
    section: i16,
    type: u16,
    storage: u8,
    aux: []const []const u8,

    pub fn isFunction(s: *const Symbol) bool {
        return (s.type >> 4) & 0xf == 2; // DTYPE_FUNCTION in the derived-type nibble
    }
};

pub const Section = struct {
    number: u32,
    name: []const u8,
    data: []const u8,
    size: u32,
    flags: u32,
    relocs: []Reloc,
    comdat: ?u8 = null, // COMDAT selection kind, when SCN_LNK_COMDAT
    comdat_assoc: u16 = 0, // associated section number (selection 5)

    pub fn isCode(s: *const Section) bool {
        return s.flags & SCN_CNT_CODE != 0;
    }
};

/// (symbol, section, start, end) - CoffObject.functions.
pub const FuncRange = struct { sym: *const Symbol, sec: *const Section, start: u32, end: u32 };

fn rd(d: []const u8, comptime T: type, o: usize) T {
    return std.mem.readInt(T, d[o..][0..@sizeOf(T)], .little);
}

pub const Object = struct {
    name: []const u8,
    data: []const u8,
    strtab: []const u8,
    /// By symbol-table index; aux slots are null.
    symbols: []?Symbol,
    /// 1-based like COFF: sections[0] is unused.
    sections: []Section,

    pub fn parse(data: []const u8, name: []const u8) !Object {
        if (data.len < 20) return error.Truncated;
        const machine = rd(data, u16, 0);
        const nsec = rd(data, u16, 2);
        const symptr = rd(data, u32, 8);
        const nsym = rd(data, u32, 12);
        const optsize = rd(data, u16, 16);
        if (machine == 0 and nsec == 0xffff) return error.BigObj;
        if (machine != 0x14c) return error.NotI386;
        const strtab_off: usize = @as(usize, symptr) + 18 * @as(usize, nsym);
        if (nsym != 0 and strtab_off + 4 > data.len) return error.Truncated;
        const strtab_len: usize = if (nsym != 0) rd(data, u32, strtab_off) else 0;
        if (strtab_off + strtab_len > data.len) return error.Truncated;
        var o = Object{ .name = name, .data = data, .strtab = if (nsym != 0) data[strtab_off .. strtab_off + strtab_len] else "", .symbols = try gpa.alloc(?Symbol, nsym), .sections = try gpa.alloc(Section, @as(usize, nsec) + 1) };
        @memset(o.symbols, null);
        var i: u32 = 0;
        while (i < nsym) {
            const so: usize = @as(usize, symptr) + 18 * @as(usize, i);
            const raw = data[so .. so + 8];
            const nm = if (std.mem.eql(u8, raw[0..4], "\x00\x00\x00\x00")) try o.string(rd(raw, u32, 4)) else std.mem.trimEnd(u8, raw, "\x00");
            const naux = data[so + 17];
            const aux = try gpa.alloc([]const u8, naux);
            for (0..naux) |k| {
                const ao: usize = @as(usize, symptr) + 18 * (@as(usize, i) + 1 + k);
                aux[k] = data[@min(ao, data.len)..@min(ao + 18, data.len)];
            }
            o.symbols[i] = .{ .index = i, .name = nm, .value = rd(data, u32, so + 8), .section = @bitCast(rd(data, u16, so + 12)), .type = rd(data, u16, so + 14), .storage = data[so + 16], .aux = aux };
            i += 1 + naux;
        }
        o.sections[0] = .{ .number = 0, .name = "", .data = "", .size = 0, .flags = 0, .relocs = &.{} };
        for (0..nsec) |n| {
            const so = 20 + @as(usize, optsize) + 40 * n;
            const raw = std.mem.trimEnd(u8, data[so .. so + 8], "\x00");
            const sname = if (raw.len > 0 and raw[0] == '/') try o.string(try std.fmt.parseInt(u32, raw[1..], 10)) else raw;
            const size = rd(data, u32, so + 16);
            const ptr = rd(data, u32, so + 20);
            const relptr = rd(data, u32, so + 24);
            const nrel = rd(data, u16, so + 32);
            const flags = rd(data, u32, so + 36);
            if ((ptr != 0 and @as(usize, ptr) + size > data.len) or @as(usize, relptr) + 10 * @as(usize, nrel) > data.len) return error.Truncated;
            const body: []const u8 = if (ptr != 0) data[ptr .. ptr + size] else try gpa.alloc(u8, size);
            if (ptr == 0) @memset(@constCast(body), 0);
            const relocs = try gpa.alloc(Reloc, nrel);
            for (0..nrel) |k| {
                const ro = relptr + 10 * k;
                relocs[k] = .{ .offset = rd(data, u32, ro), .symbol = rd(data, u32, ro + 4), .type = rd(data, u16, ro + 8) };
            }
            o.sections[n + 1] = .{ .number = @intCast(n + 1), .name = sname, .data = body, .size = size, .flags = flags, .relocs = relocs };
        }
        // COMDAT selection lives in the section symbol's aux record.
        for (o.symbols) |*sp| {
            const s = &(sp.* orelse continue);
            if (s.storage != C_STATIC or s.section <= 0 or s.aux.len == 0) continue;
            if (@as(usize, @intCast(s.section)) >= o.sections.len) return error.BadSection;
            const sec = &o.sections[@intCast(s.section)];
            if (sec.flags & SCN_LNK_COMDAT != 0 and std.mem.eql(u8, s.name, sec.name) and s.value == 0) {
                if (s.aux[0].len < 15) return error.Truncated;
                sec.comdat_assoc = rd(s.aux[0], u16, 12);
                sec.comdat = s.aux[0][14];
            }
        }
        return o;
    }

    fn string(o: *const Object, off: u32) ![]const u8 {
        if (off > o.strtab.len) return error.BadString;
        const e = std.mem.indexOfScalarPos(u8, o.strtab, off, 0) orelse return error.BadString;
        return o.strtab[off..e];
    }

    pub fn symbol(o: *const Object, index: u32) !*const Symbol {
        if (index >= o.symbols.len) return error.BadSymbol;
        return &(o.symbols[index] orelse return error.BadSymbol);
    }

    /// Every function defined here; the end is the next function symbol in the same section,
    /// or the section end. labels: also the public labels of a code section with no function
    /// symbol at all (an assembler object), aliases at one address once.
    pub fn functions(o: *const Object, labels: bool) ![]FuncRange {
        // Section number (negative: a labels-only section) -> symbols, in first-seen order.
        var by_sec: std.AutoArrayHashMapUnmanaged(i32, std.ArrayList(*const Symbol)) = .empty;
        for (o.symbols) |*sp| {
            const s = &(sp.* orelse continue);
            if (s.section <= 0 or !s.isFunction()) continue;
            if (s.storage != C_EXTERNAL and s.storage != C_STATIC) continue;
            const g = try by_sec.getOrPut(gpa, s.section);
            if (!g.found_existing) g.value_ptr.* = .empty;
            try g.value_ptr.append(gpa, s);
        }
        if (labels) {
            for (o.symbols) |*sp| {
                const s = &(sp.* orelse continue);
                if (s.section > 0 and s.storage == C_EXTERNAL and !by_sec.contains(s.section)) {
                    if (@as(usize, @intCast(s.section)) >= o.sections.len) return error.BadSection;
                    if (!o.sections[@intCast(s.section)].isCode()) continue;
                    const g = try by_sec.getOrPut(gpa, -@as(i32, s.section));
                    if (!g.found_existing) g.value_ptr.* = .empty;
                    try g.value_ptr.append(gpa, s);
                }
            }
            var negs: std.ArrayList(i32) = .empty;
            for (by_sec.keys()) |k| if (k < 0) try negs.append(gpa, k);
            for (negs.items) |k| {
                const syms = by_sec.get(k).?;
                _ = by_sec.orderedRemove(k);
                std.mem.sort(*const Symbol, syms.items, {}, struct {
                    fn lt(_: void, x: *const Symbol, y: *const Symbol) bool {
                        return x.value < y.value or (x.value == y.value and x.index < y.index);
                    }
                }.lt);
                var kept: std.ArrayList(*const Symbol) = .empty;
                var seen: std.AutoHashMapUnmanaged(u32, void) = .empty;
                for (syms.items) |s| if (!seen.contains(s.value)) {
                    try seen.put(gpa, s.value, {});
                    try kept.append(gpa, s);
                };
                try by_sec.put(gpa, -k, kept);
            }
        }
        var out: std.ArrayList(FuncRange) = .empty;
        var it = by_sec.iterator();
        while (it.next()) |e| {
            const sec = &o.sections[@intCast(e.key_ptr.*)];
            const syms = e.value_ptr.items;
            std.mem.sort(*const Symbol, syms, {}, struct {
                fn lt(_: void, x: *const Symbol, y: *const Symbol) bool {
                    return x.value < y.value;
                }
            }.lt);
            for (syms, 0..) |s, k| {
                const end = if (k + 1 < syms.len) syms[k + 1].value else sec.size;
                try out.append(gpa, .{ .sym = s, .sec = sec, .start = s.value, .end = end });
            }
        }
        std.mem.sort(FuncRange, out.items, {}, struct {
            fn lt(_: void, x: FuncRange, y: FuncRange) bool {
                return x.sec.number < y.sec.number or (x.sec.number == y.sec.number and x.start < y.start);
            }
        }.lt);
        return out.items;
    }

    /// The @comp.id stamp (product id << 16 | build) of the compiler.
    pub fn compId(o: *const Object) ?u32 {
        for (o.symbols) |sp| if (sp) |s| if (std.mem.eql(u8, s.name, "@comp.id")) return s.value;
        return null;
    }
};

pub const Member = struct { name: []const u8, body: []const u8 };

/// Members of a COFF import/static library. Long names resolve through the '//' member;
/// the two linker members are skipped.
pub fn readArchive(data: []const u8) ![]Member {
    if (data.len < 8 or !std.mem.eql(u8, data[0..8], "!<arch>\n")) return error.NotArchive;
    var p: usize = 8;
    var longnames: []const u8 = "";
    var out: std.ArrayList(Member) = .empty;
    while (p + 60 <= data.len) {
        const hdr = data[p .. p + 60];
        const name = std.mem.trimEnd(u8, hdr[0..16], " \t\n\r\x0b\x0c");
        const size = try std.fmt.parseInt(usize, util.strip(hdr[48..58]), 10);
        const body = data[@min(p + 60, data.len)..@min(p + 60 + size, data.len)];
        if (std.mem.eql(u8, name, "//")) {
            longnames = body;
        } else if (std.mem.eql(u8, name, "/")) {} else if (name.len > 1 and name[0] == '/' and allDigits(name[1..])) {
            const off = try std.fmt.parseInt(usize, name[1..], 10);
            const end = std.mem.indexOfScalarPos(u8, longnames, off, 0);
            const nm = if (end) |e| longnames[off..e] else longnames[@min(off, longnames.len)..];
            try out.append(gpa, .{ .name = std.mem.trimEnd(u8, nm, "/"), .body = body });
        } else {
            try out.append(gpa, .{ .name = std.mem.trimEnd(u8, name, "/"), .body = body });
        }
        p += 60 + size + (size & 1);
    }
    return out.items;
}

fn allDigits(s: []const u8) bool {
    for (s) |ch| if (ch < '0' or ch > '9') return false;
    return s.len > 0;
}
