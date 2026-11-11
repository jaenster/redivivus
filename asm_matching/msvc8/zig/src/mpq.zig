//! mpq-extract (mpq_extract.py): pull one file out of a Blizzard Diablo II patch installer.
//!
//!   oracle-zig mpq-extract <LODPatch_114d.exe> <member-name> <out-file>
//!
//! The 1.14d patch installer is a stub executable with an MPQ v0 archive appended. Every
//! member is a 24-byte Blizzard PrePatch record plus a payload; `Game.exe` is a whole-file
//! record (kind 0x01xx, source size 0), so the 1.14d image comes out without any base install
//! to patch against. Read-only, and it never executes its input. Members are FIX_KEY-encrypted
//! and PKWARE-DCL-imploded; the key is derived from the member name, and the DCL decoder is a
//! port of zlib's contrib/blast/blast.c. A zlib sector is inflated too; a bzip2 one (which
//! no member of the 1.14d patch uses) is refused. Exit status 0 only if the member was found,
//! decoded and written.
const std = @import("std");
const cli = @import("cli.zig");
const util = @import("util.zig");

const gpa = util.gpa;

// ---- MPQ crypto

const CRYPT: [0x500]u32 = blk: {
    @setEvalBranchQuota(100000);
    var t: [0x500]u32 = undefined;
    var seed: u32 = 0x00100001;
    for (0..0x100) |i| {
        var idx = i;
        for (0..5) |_| {
            seed = (seed * 125 + 3) % 0x2AAAAB;
            const hi = (seed & 0xFFFF) << 16;
            seed = (seed * 125 + 3) % 0x2AAAAB;
            t[idx] = hi | (seed & 0xFFFF);
            idx += 0x100;
        }
    }
    break :blk t;
};

/// str.upper() of a Latin-1 character, where the result is one.
fn upper(ch: u8) u8 {
    return switch (ch) {
        'a'...'z' => ch - 32,
        0xe0...0xfe => if (ch == 0xf7) ch else ch - 32,
        else => ch,
    };
}

pub fn hashString(name: []const u8, kind: u32) u32 {
    var s1: u32 = 0x7FED7FED;
    var s2: u32 = 0xEEEEEEEE;
    for (name) |c0| {
        const ch: u32 = if (c0 == '/') '\\' else upper(c0);
        s1 = CRYPT[kind * 0x100 + ch] ^ (s1 +% s2);
        s2 = ch +% s1 +% s2 +% (s2 << 5) +% 3;
    }
    return s1;
}

pub fn decrypt(data: []const u8, key_in: u32) ![]u8 {
    const out = try gpa.dupe(u8, data);
    var key = key_in;
    var s2: u32 = 0xEEEEEEEE;
    for (0..data.len / 4) |i| {
        s2 +%= CRYPT[0x400 + (key & 0xFF)];
        const v = std.mem.readInt(u32, data[i * 4 ..][0..4], .little) ^ (key +% s2);
        key = ((~key << 0x15) +% 0x11111111) | (key >> 0x0B);
        s2 = v +% s2 +% (s2 << 5) +% 3;
        std.mem.writeInt(u32, out[i * 4 ..][0..4], v, .little);
    }
    return out;
}

// ---- PKWARE DCL explode (zlib contrib/blast/blast.c)

const LITLEN = [_]u8{
    11,  124, 8,   7,   28,  7,   188, 13,  76,  4,  10,  8,  12, 10, 12, 10, 8,  23, 8,
    9,   7,   6,   7,   8,   7,   6,   55,  8,   23, 24,  12, 11, 7,  9,  11, 12, 6,  7,
    22,  5,   7,   24,  6,   11,  9,   6,   7,   22, 7,   11, 38, 7,  9,  8,  25, 11, 8,
    11,  9,   12,  8,   12,  5,   38,  5,   38,  5,  11,  7,  5,  6,  21, 6,  10, 53, 8,
    7,   24,  10,  27,  44,  253, 253, 253, 252, 252, 252, 13, 12, 45, 12, 45, 12, 61, 12,
    45,  44,  173,
};
const LENLEN = [_]u8{ 2, 35, 36, 53, 38, 23 };
const DISTLEN = [_]u8{ 2, 20, 53, 230, 247, 151, 248 };
const BASE = [_]u32{ 3, 2, 4, 5, 6, 7, 8, 9, 10, 12, 16, 24, 40, 72, 136, 264 };
const EXTRA = [_]u32{ 0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 3, 4, 5, 6, 7, 8 };
const MAXBITS = 13;

const Huffman = struct { count: [MAXBITS + 1]u32, symbol: []const u32 };

fn construct(comptime rep: []const u8) Huffman {
    @setEvalBranchQuota(100000);
    var n_len: usize = 0;
    for (rep) |r| n_len += (r >> 4) + 1;
    var length: [n_len]u8 = undefined;
    var k: usize = 0;
    for (rep) |r| {
        for (0..(r >> 4) + 1) |_| {
            length[k] = r & 15;
            k += 1;
        }
    }
    var count: [MAXBITS + 1]u32 = @splat(0);
    for (length) |n| count[n] += 1;
    var offs: [MAXBITS + 2]u32 = @splat(0);
    for (1..MAXBITS) |n| offs[n + 1] = offs[n] + count[n];
    var symbol: [n_len]u32 = @splat(0);
    for (length, 0..) |n, s| if (n != 0) {
        symbol[offs[n]] = s;
        offs[n] += 1;
    };
    const final = symbol;
    return .{ .count = count, .symbol = &final };
}

const LIT = construct(&LITLEN);
const LEN = construct(&LENLEN);
const DIST = construct(&DISTLEN);

const Bits = struct {
    src: []const u8,
    pos: usize = 16, // bit position; the first two bytes are the header

    fn bit(b: *const Bits, at: usize) !u32 {
        if (at >> 3 >= b.src.len) return error.IndexError;
        return (b.src[at >> 3] >> @intCast(at & 7)) & 1;
    }

    fn bits(b: *Bits, k: u32) !u32 {
        var v: u32 = 0;
        for (0..k) |i| v |= (try b.bit(b.pos + i)) << @intCast(i);
        b.pos += k;
        return v;
    }

    fn decode(b: *Bits, h: *const Huffman) !u32 {
        var code: u32 = 0;
        var first: u32 = 0;
        var index: u32 = 0;
        for (1..MAXBITS + 1) |n| {
            code |= (try b.bit(b.pos)) ^ 1; // codes are stored inverted
            b.pos += 1;
            const c = h.count[n];
            if (code < first + c) return h.symbol[index + (code - first)];
            index += c;
            first = (first + c) << 1;
            code <<= 1;
        }
        return error.BadHuffmanCode;
    }
};

pub fn explode(src: []const u8, want: usize) ![]u8 {
    if (src.len < 2) return error.IndexError;
    const lit = src[0];
    const dictbits = src[1];
    if (lit > 1 or !(4 <= dictbits and dictbits <= 6)) return error.NotADclStream;
    var b = Bits{ .src = src };
    var out: std.ArrayList(u8) = .empty;
    const end = src.len * 8;
    while (out.items.len < want and b.pos < end) {
        if (try b.bits(1) != 0) {
            const sym = try b.decode(&LEN);
            const length = BASE[sym] + try b.bits(EXTRA[sym]);
            if (length == 519) break;
            const extra: u32 = if (length == 2) 2 else dictbits;
            const dist = ((try b.decode(&DIST)) << @intCast(extra)) + try b.bits(extra) + 1;
            if (dist > out.items.len) return error.DistanceBeforeStart;
            for (0..length) |_| try out.append(gpa, out.items[out.items.len - dist]);
        } else {
            try out.append(gpa, @intCast(if (lit != 0) try b.decode(&LIT) else try b.bits(8)));
        }
    }
    return out.items[0..@min(want, out.items.len)];
}

// ---- archive

const F_IMPLODE = 0x100;
const F_COMPRESS = 0x200;
const F_ENCRYPTED = 0x10000;
const F_FIX_KEY = 0x20000;
const F_SINGLE_UNIT = 0x1000000;
const F_EXISTS = 0x80000000;

fn inflate(body: []const u8) ![]u8 {
    var in: std.Io.Reader = .fixed(body);
    var d: std.compress.flate.Decompress = .init(&in, .zlib, &.{});
    var aw: std.Io.Writer.Allocating = .init(gpa);
    _ = try d.reader.streamRemaining(&aw.writer);
    return aw.written();
}

fn expand(chunk: []const u8, want: usize, flags: u32) ![]const u8 {
    if (chunk.len >= want) return chunk[0..want];
    if (flags & F_COMPRESS != 0) {
        if (chunk.len == 0) return error.IndexError;
        const mask = chunk[0];
        const body = chunk[1..];
        if (mask == 0x02) return inflate(body);
        if (mask == 0x08) return explode(body, want);
        if (mask == 0x10) {
            cli.sayErr("mpq-extract: a bzip2 sector (compression mask 0x10): not supported\n");
            return error.Unsupported;
        }
        cli.sayErr(try std.fmt.allocPrint(gpa, "unsupported compression mask 0x{x:0>2}\n", .{mask}));
        return error.Unsupported;
    }
    if (flags & F_IMPLODE != 0) return explode(chunk, want);
    return chunk;
}

fn sl(data: []const u8, a: usize, b: usize) []const u8 {
    return data[@min(a, data.len)..@min(b, data.len)];
}

fn rd(d: []const u8, comptime T: type, o: usize) T {
    return std.mem.readInt(T, d[o..][0..@sizeOf(T)], .little);
}

const Hash = struct { a: u32, b: u32, block: u32 };
const Block = struct { pos: u32, csize: u32, fsize: u32, flags: u32 };

pub const Archive = struct {
    data: []const u8,
    base: usize,
    sector: u32,
    hashes: []Hash,
    blocks: []Block,

    pub fn open(data: []const u8) !Archive {
        var off_opt = std.mem.indexOf(u8, data, "MPQ\x1a");
        while (off_opt) |off| {
            if (off + 12 <= data.len) {
                const hsz = rd(data, u32, off + 4);
                const asz = rd(data, u32, off + 8);
                if (hsz == 32 and 0 < asz and asz <= data.len - off) break;
            }
            off_opt = std.mem.indexOfPos(u8, data, off + 1, "MPQ\x1a");
        }
        const off = off_opt orelse return error.NoMpqArchive;
        if (off + 32 > data.len) return error.StructError;
        const shift = rd(data, u16, off + 14);
        const htpos = rd(data, u32, off + 16);
        const btpos = rd(data, u32, off + 20);
        const htn = rd(data, u32, off + 24);
        const btn = rd(data, u32, off + 28);
        const ht = try decrypt(sl(data, off + htpos, off + htpos + @as(usize, htn) * 16), hashString("(hash table)", 3));
        const bt = try decrypt(sl(data, off + btpos, off + btpos + @as(usize, btn) * 16), hashString("(block table)", 3));
        if (ht.len < @as(usize, htn) * 16 or bt.len < @as(usize, btn) * 16) return error.StructError;
        const hashes = try gpa.alloc(Hash, htn);
        for (hashes, 0..) |*h, i| h.* = .{ .a = rd(ht, u32, i * 16), .b = rd(ht, u32, i * 16 + 4), .block = rd(ht, u32, i * 16 + 12) };
        const blocks = try gpa.alloc(Block, btn);
        for (blocks, 0..) |*b, i| b.* = .{ .pos = rd(bt, u32, i * 16), .csize = rd(bt, u32, i * 16 + 4), .fsize = rd(bt, u32, i * 16 + 8), .flags = rd(bt, u32, i * 16 + 12) };
        return .{ .data = data, .base = off, .sector = @as(u32, 512) << @intCast(shift), .hashes = hashes, .blocks = blocks };
    }

    fn lookup(ar: *const Archive, name: []const u8) ?u32 {
        const a = hashString(name, 1);
        const b = hashString(name, 2);
        const n = ar.hashes.len;
        if (n == 0) return null;
        var i = hashString(name, 0) % n;
        for (0..n) |_| {
            const h = ar.hashes[i];
            if (h.block == 0xFFFFFFFF) return null;
            if (h.a == a and h.b == b and h.block != 0xFFFFFFFE) return h.block;
            i = (i + 1) % n;
        }
        return null;
    }

    pub fn read(ar: *const Archive, name: []const u8) ![]const u8 {
        const idx = ar.lookup(name) orelse return error.KeyError;
        if (idx >= ar.blocks.len) return error.IndexError;
        const bl = ar.blocks[idx];
        if (bl.flags & F_EXISTS == 0) return error.KeyError;
        var base_name = name;
        if (std.mem.lastIndexOfAny(u8, name, "/\\")) |k| base_name = name[k + 1 ..];
        var key = hashString(base_name, 3);
        if (bl.flags & F_FIX_KEY != 0) key = (key +% bl.pos) ^ bl.fsize;
        const raw = sl(ar.data, ar.base + bl.pos, ar.base + bl.pos + bl.csize);
        const enc = bl.flags & F_ENCRYPTED != 0;
        if (bl.flags & F_SINGLE_UNIT != 0) return expand(if (enc) try decrypt(raw, key) else raw, bl.fsize, bl.flags);
        const nsec = (bl.fsize + ar.sector - 1) / ar.sector;
        var out: std.ArrayList(u8) = .empty;
        if (bl.flags & (F_IMPLODE | F_COMPRESS) == 0) {
            for (0..nsec) |s| {
                const chunk = sl(raw, s * ar.sector, (s + 1) * ar.sector);
                try out.appendSlice(gpa, if (enc) try decrypt(chunk, key +% @as(u32, @intCast(s))) else chunk);
            }
            return out.items[0..@min(bl.fsize, out.items.len)];
        }
        var table = sl(raw, 0, (@as(usize, nsec) + 1) * 4);
        if (enc) table = try decrypt(table, key -% 1);
        if (table.len < (@as(usize, nsec) + 1) * 4) return error.StructError;
        for (0..nsec) |s| {
            var chunk = sl(raw, rd(table, u32, s * 4), rd(table, u32, s * 4 + 4));
            if (enc) chunk = try decrypt(chunk, key +% @as(u32, @intCast(s)));
            try out.appendSlice(gpa, try expand(chunk, @min(ar.sector, bl.fsize - s * ar.sector), bl.flags));
        }
        return out.items;
    }
};

/// The file a PrePatch record carries. Only whole-file records are handled: a delta would
/// need the retail file it applies to.
pub fn prepatchPayload(record: []const u8) ![]const u8 {
    if (record.len >= 2 and std.mem.eql(u8, record[0..2], "MZ")) return record; // BNUpdate.exe and friends ship unwrapped
    if (record.len < 16) return error.StructError;
    const hsz = rd(record, u16, 0);
    const kind = rd(record, u16, 2);
    const src_size = rd(record, u32, 8);
    const tgt_size = rd(record, u32, 12);
    if (hsz != 0x18) {
        cli.sayErr("ValueError: not a PrePatch record\n");
        return error.NotAPrePatchRecord;
    }
    if (kind >> 8 != 1 or src_size != 0) {
        cli.sayErr(try std.fmt.allocPrint(gpa, "ValueError: member is a delta (kind 0x{x:0>4}), not a whole file\n", .{kind}));
        return error.DeltaMember;
    }
    return sl(record, hsz, @as(usize, hsz) + tgt_size);
}

const USAGE =
    \\mpq-extract - pull one file out of a Blizzard Diablo II patch installer.
    \\
    \\    oracle-zig mpq-extract <LODPatch_114d.exe> <member-name> <out-file>
    \\
;

pub fn main(args: []const []const u8) !void {
    if (args.len != 3) {
        cli.sayErr(USAGE);
        std.process.exit(2);
    }
    const ar = try Archive.open(try util.readFile(args[0]));
    const body = try prepatchPayload(try ar.read(args[1]));
    try util.writeFile(args[2], body);
}
