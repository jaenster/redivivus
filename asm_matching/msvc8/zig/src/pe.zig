//! A PE32 image, read the way peimage.py reads it.
const std = @import("std");

pub const Section = struct {
    name: []const u8,
    va: u32,
    vsize: u32,
    raw: u32,
    rawsize: u32,
    pub fn end(s: Section) u32 {
        return s.va + @max(s.vsize, s.rawsize);
    }
};

pub const Image = struct {
    data: []const u8,
    base: u32,
    entry_rva: u32,
    dirs: [16][2]u32,
    sections: []Section,
    /// VAs of every HIGHLOW base relocation.
    relocs: std.AutoHashMapUnmanaged(u32, void) = .empty,

    fn rd(d: []const u8, comptime T: type, o: usize) T {
        return std.mem.readInt(T, d[o..][0..@sizeOf(T)], .little);
    }

    pub fn parse(gpa: std.mem.Allocator, data: []const u8) !Image {
        const pe = rd(data, u32, 0x3c);
        if (!std.mem.eql(u8, data[pe .. pe + 4], "PE\x00\x00")) return error.NotPE;
        const nsec = rd(data, u16, pe + 6);
        const optsize = rd(data, u16, pe + 20);
        const opt = pe + 24;
        if (rd(data, u16, opt) != 0x10b) return error.NotPE32;
        var img = Image{ .data = data, .base = rd(data, u32, opt + 28), .entry_rva = rd(data, u32, opt + 16), .dirs = std.mem.zeroes([16][2]u32), .sections = try gpa.alloc(Section, nsec) };
        const ndirs = @min(rd(data, u32, opt + 92), 16);
        for (0..ndirs) |i| img.dirs[i] = .{ rd(data, u32, opt + 96 + 8 * i), rd(data, u32, opt + 100 + 8 * i) };
        const sh = opt + optsize;
        for (0..nsec) |i| {
            const o = sh + 40 * i;
            const raw_name = data[o .. o + 8];
            const n = std.mem.indexOfScalar(u8, raw_name, 0) orelse 8;
            img.sections[i] = .{ .name = raw_name[0..n], .vsize = rd(data, u32, o + 8), .va = rd(data, u32, o + 12), .rawsize = rd(data, u32, o + 16), .raw = rd(data, u32, o + 20) };
        }
        try img.loadRelocs(gpa);
        return img;
    }

    pub fn section(img: *const Image, name: []const u8) ?Section {
        for (img.sections) |s| if (std.mem.eql(u8, s.name, name)) return s;
        return null;
    }

    pub fn sectionAt(img: *const Image, va: u32) ?Section {
        const rva = va -% img.base;
        for (img.sections) |s| if (s.va <= rva and rva < s.end()) return s;
        return null;
    }

    /// n bytes at va; the zero-filled tail of a section reads as zeros. Null outside the image.
    pub fn read(img: *const Image, va0: u32, out: []u8) bool {
        var va = va0;
        var w: usize = 0;
        while (w < out.len) {
            const s = img.sectionAt(va) orelse return false;
            const rva = va - img.base;
            const avail = s.va + @max(s.vsize, s.rawsize) - rva;
            const take: u32 = @intCast(@min(out.len - w, avail));
            const in_sec = rva - s.va;
            const inraw: u32 = if (s.rawsize > in_sec) @min(take, s.rawsize - in_sec) else 0;
            if (inraw > 0) @memcpy(out[w .. w + inraw], img.data[s.raw + in_sec ..][0..inraw]);
            @memset(out[w + inraw .. w + take], 0);
            w += take;
            va += take;
        }
        return true;
    }

    pub fn u32At(img: *const Image, va: u32) ?u32 {
        var b: [4]u8 = undefined;
        if (!img.read(va, &b)) return null;
        return std.mem.readInt(u32, &b, .little);
    }

    fn offset(img: *const Image, va: u32) ?u32 {
        const s = img.sectionAt(va) orelse return null;
        const rva = va - img.base;
        if (rva - s.va >= s.rawsize) return null;
        return s.raw + rva - s.va;
    }

    fn loadRelocs(img: *Image, gpa: std.mem.Allocator) !void {
        const rva = img.dirs[5][0];
        const size = img.dirs[5][1];
        if (rva == 0) return;
        var p: usize = img.offset(img.base + rva) orelse return;
        const end = p + size;
        while (p < end) {
            const page = rd(img.data, u32, p);
            const block = rd(img.data, u32, p + 4);
            if (block < 8) break;
            var i: usize = 0;
            while (i < (block - 8) / 2) : (i += 1) {
                const e = rd(img.data, u16, p + 8 + 2 * i);
                switch (e >> 12) {
                    3 => try img.relocs.put(gpa, img.base + page + (e & 0xfff), {}),
                    0 => {},
                    else => return error.UnexpectedRelocType,
                }
            }
            p += block;
        }
    }

    /// VAs of the named exports.
    pub fn exports(img: *const Image, gpa: std.mem.Allocator) ![]u32 {
        var out: std.ArrayList(u32) = .empty;
        const rva = img.dirs[0][0];
        if (rva == 0) return out.toOwnedSlice(gpa);
        var hdr: [40]u8 = undefined;
        if (!img.read(img.base + rva, &hdr)) return out.toOwnedSlice(gpa);
        const nnames = rd(&hdr, u32, 24);
        const afun = rd(&hdr, u32, 28);
        const aords = rd(&hdr, u32, 36);
        for (0..nnames) |i| {
            var ob: [2]u8 = undefined;
            if (!img.read(img.base + aords + 2 * @as(u32, @intCast(i)), &ob)) continue;
            const o = std.mem.readInt(u16, &ob, .little);
            const f = img.u32At(img.base + afun + 4 * @as(u32, o)) orelse continue;
            try out.append(gpa, img.base + f);
        }
        return out.toOwnedSlice(gpa);
    }
};
