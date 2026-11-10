//! refimage: carve the original Game.exe into functions (refimage.py, in Zig).
//! Output: build/ref/functions.tsv, the same columns and rows as the Python carve.
const std = @import("std");
const pe_mod = @import("pe.zig");
const dec = @import("decode.zig");
const cio = @import("c");

const gpa = std.heap.c_allocator;

// Reasons, in the alphabetical order Python's sorted() joins them.
const WHY = [_][]const u8{ "call", "data", "entry", "export", "fptr", "jmp", "pad", "seh", "vtbl" };
const W = struct {
    const call = 1 << 0;
    const data = 1 << 1;
    const entry = 1 << 2;
    const exp = 1 << 3;
    const fptr = 1 << 4;
    const jmp = 1 << 5;
    const pad = 1 << 6;
    const seh = 1 << 7;
    const vtbl = 1 << 8;
};

const Ref = struct {
    img: pe_mod.Image,
    code: []u8,
    ts: u32,
    te: u32,
    d: dec.Decoder,
    cache: std.AutoHashMapUnmanaged(u64, dec.Function) = .empty,

    fn inText(r: *const Ref, v: u32) bool {
        return r.ts <= v and v < r.te;
    }

    fn outer(ctx: *const anyopaque, va: u32) ?u32 {
        const img: *const pe_mod.Image = @ptrCast(@alignCast(ctx));
        return img.u32At(va);
    }

    fn fn_(r: *Ref, s: u32, limit: u32) !*dec.Function {
        const key = (@as(u64, s) << 32) | limit;
        const g = try r.cache.getOrPut(gpa, key);
        if (!g.found_existing) g.value_ptr.* = try r.d.decode(gpa, s, limit);
        return g.value_ptr;
    }

    fn padOnlyBetween(r: *const Ref, a: u32, b: u32) bool {
        for (r.code[a - r.ts .. b - r.ts]) |x| if (x != 0xCC) return false;
        return true;
    }

    fn u32c(r: *const Ref, k: usize) u32 {
        return std.mem.readInt(u32, r.code[k..][0..4], .little);
    }

    fn sehPrologs(r: *Ref, ptr_at: *const std.AutoHashMapUnmanaged(u32, u32), out: *std.AutoHashMapUnmanaged(u32, void)) !void {
        const code = r.code;
        var sites: std.AutoArrayHashMapUnmanaged(u32, std.ArrayList(u32)) = .empty;
        var k: usize = 0;
        while (k + 12 < code.len) : (k += 1) {
            var q: usize = undefined;
            if (code[k] == 0x6A) q = k + 2 else if (code[k] == 0x68 and !r.img.relocs.contains(r.ts + @as(u32, @intCast(k)) + 1)) q = k + 5 else continue;
            if (q + 10 > code.len or code[q] != 0x68 or !r.img.relocs.contains(r.ts + @as(u32, @intCast(q)) + 1) or code[q + 5] != 0xE8) continue;
            const tbl = r.u32c(q + 1);
            if (r.inText(tbl)) continue;
            var any = false;
            var a = tbl;
            while (a < tbl +% 0x40) : (a += 4) if (ptr_at.contains(a)) {
                any = true;
                break;
            };
            if (!any) continue;
            const rel: i32 = @bitCast(r.u32c(q + 6));
            const callee: u32 = @bitCast(@as(i32, @bitCast(r.ts + @as(u32, @intCast(q)) + 10)) +% rel);
            const g = try sites.getOrPut(gpa, callee);
            if (!g.found_existing) g.value_ptr.* = .empty;
            try g.value_ptr.append(gpa, r.ts + @as(u32, @intCast(k)));
        }
        var it = sites.iterator();
        while (it.next()) |e| if (e.value_ptr.items.len >= 8 and r.inText(e.key_ptr.*)) {
            for (e.value_ptr.items) |a| try out.put(gpa, a, {});
        };
    }

    fn ehTargets(r: *Ref, ptrs: *const std.AutoHashMapUnmanaged(u32, u32), out: *std.AutoHashMapUnmanaged(u32, void)) !void {
        const code = r.code;
        var k: usize = 0;
        while (k + 12 < code.len) : (k += 1) {
            if (code[k] != 0x6A and code[k] != 0x68) continue;
            const q = k + @as(usize, if (code[k] == 0x6A) 2 else 5);
            if (code[q] != 0x68) continue;
            if (!r.img.relocs.contains(r.ts + @as(u32, @intCast(q)) + 1)) continue;
            const tbl = r.u32c(q + 1);
            if (r.img.sectionAt(tbl) == null or r.inText(tbl)) continue;
            var rec = tbl +% 16;
            while (true) : (rec +%= 12) {
                const lvl = r.img.u32At(rec) orelse break;
                const h = ptrs.get(rec +% 8) orelse break;
                if (!(lvl == 0xFFFFFFFE or lvl < 0x100)) break;
                try out.put(gpa, h, {});
                if (ptrs.get(rec +% 4)) |f| try out.put(gpa, f, {});
            }
        }
        for (r.img.sections) |sec| {
            if (!std.mem.eql(u8, sec.name, ".rdata") and !std.mem.eql(u8, sec.name, ".data")) continue;
            const base = r.img.base + sec.va;
            const buf = try gpa.alloc(u8, sec.vsize);
            defer gpa.free(buf);
            _ = r.img.read(base, buf);
            var o: usize = 0;
            while (o + 20 < buf.len) : (o += 4) {
                const magic = std.mem.readInt(u32, buf[o..][0..4], .little);
                if (magic != 0x19930520 and magic != 0x19930521 and magic != 0x19930522) continue;
                const fi = base + @as(u32, @intCast(o));
                const nstate = std.mem.readInt(u32, buf[o + 4 ..][0..4], .little);
                const punw = std.mem.readInt(u32, buf[o + 8 ..][0..4], .little);
                const ntry = std.mem.readInt(u32, buf[o + 12 ..][0..4], .little);
                const ptry = std.mem.readInt(u32, buf[o + 16 ..][0..4], .little);
                if (nstate > 0x1000 or ntry > 0x100) continue;
                if (punw != 0 and r.img.relocs.contains(fi + 8)) {
                    for (0..nstate) |j| if (ptrs.get(punw +% 8 * @as(u32, @intCast(j)) +% 4)) |a| try out.put(gpa, a, {});
                }
                if (ptry != 0 and r.img.relocs.contains(fi + 16)) {
                    for (0..ntry) |j| {
                        const e = ptry +% 20 * @as(u32, @intCast(j));
                        const ncatch = r.img.u32At(e +% 12) orelse break;
                        const phand = r.img.u32At(e +% 16) orelse break;
                        for (0..@min(ncatch, 0x100)) |cc| if (ptrs.get(phand +% 16 * @as(u32, @intCast(cc)) +% 12)) |a| try out.put(gpa, a, {});
                    }
                }
            }
        }
    }
};

fn sortedKeys(why: *const std.AutoArrayHashMapUnmanaged(u32, u16)) ![]u32 {
    const ks = try gpa.dupe(u32, why.keys());
    std.mem.sort(u32, ks, {}, std.sort.asc(u32));
    return ks;
}

/// Index of the last start <= v (bisect_right - 1), or null.
fn floorIdx(starts: []const u32, v: u32) ?usize {
    const i = std.sort.upperBound(u32, starts, v, struct {
        fn cmp(key: u32, x: u32) std.math.Order {
            return std.math.order(key, x);
        }
    }.cmp);
    return if (i == 0) null else i - 1;
}

fn readFile(path: [*:0]const u8) ![]u8 {
    const f = cio.fopen(path, "rb") orelse return error.Open;
    defer _ = cio.fclose(f);
    _ = cio.fseek(f, 0, cio.SEEK_END);
    const n: usize = @intCast(cio.ftell(f));
    _ = cio.fseek(f, 0, cio.SEEK_SET);
    const buf = try gpa.alloc(u8, n);
    if (cio.fread(buf.ptr, 1, n, f) != n) return error.Read;
    return buf;
}

pub fn main(init: std.process.Init.Minimal) !void {
    var argv = try std.process.Args.Iterator.initAllocator(init.args, gpa);
    defer argv.deinit();
    _ = argv.next();
    const exe_path: [:0]const u8 = argv.next() orelse "../reference/Game.exe";
    const out_path: [:0]const u8 = argv.next() orelse "../build/ref/functions.tsv";
    const exe_z = exe_path.ptr;
    const data = try readFile(exe_z);
    const img = try pe_mod.Image.parse(gpa, data);
    const text = img.section(".text") orelse return error.NoText;
    var r = Ref{ .img = img, .code = undefined, .ts = img.base + text.va, .te = img.base + text.va + text.vsize, .d = undefined };
    r.code = try gpa.alloc(u8, text.vsize);
    _ = r.img.read(r.ts, r.code);
    r.d = try dec.Decoder.init(r.code, r.ts, &r.img.relocs, Ref.outer, &r.img);

    var why: std.AutoArrayHashMapUnmanaged(u32, u16) = .empty;
    const add = struct {
        fn f(w: *std.AutoArrayHashMapUnmanaged(u32, u16), a: u32, bit: u16) !void {
            const g = try w.getOrPut(gpa, a);
            if (!g.found_existing) g.value_ptr.* = 0;
            g.value_ptr.* |= bit;
        }
    }.f;
    const ts = r.ts;
    const te = r.te;
    try add(&why, ts, W.pad);
    var a: u32 = ts + 16;
    while (a < te) : (a += 16) if (r.code[a - ts - 1] == 0xCC and r.code[a - ts] != 0xCC) try add(&why, a, W.pad);
    const entry = img.base + img.entry_rva;
    if (r.inText(entry)) try add(&why, entry, W.entry);
    for (try img.exports(gpa)) |v| if (r.inText(v)) try add(&why, v, W.exp);

    var data_ptrs: std.AutoHashMapUnmanaged(u32, u32) = .empty;
    var others: std.AutoArrayHashMapUnmanaged(u32, void) = .empty;
    {
        var rit = img.relocs.keyIterator();
        while (rit.next()) |rp| {
            const rv = rp.*;
            if (r.inText(rv)) continue;
            const v = img.u32At(rv) orelse continue;
            if (r.inText(v)) try data_ptrs.put(gpa, rv, v);
        }
        var dit = data_ptrs.iterator();
        while (dit.next()) |e| {
            if (data_ptrs.contains(e.key_ptr.* -% 4) or data_ptrs.contains(e.key_ptr.* +% 4)) try add(&why, e.value_ptr.*, W.vtbl);
            try others.put(gpa, e.value_ptr.*, {});
        }
    }
    var seh_all: std.AutoHashMapUnmanaged(u32, void) = .empty;
    try r.sehPrologs(&data_ptrs, &seh_all);
    var eh: std.AutoHashMapUnmanaged(u32, void) = .empty;
    try r.ehTargets(&data_ptrs, &eh);

    while (true) {
        while (true) {
            const starts = try sortedKeys(&why);
            defer gpa.free(starts);
            var calls: std.AutoHashMapUnmanaged(u32, void) = .empty;
            defer calls.deinit(gpa);
            var fall: std.AutoHashMapUnmanaged(u32, void) = .empty;
            defer fall.deinit(gpa);
            for (starts, 0..) |s, i| {
                const limit = if (i + 1 < starts.len) starts[i + 1] else te;
                const f = try r.fn_(s, limit);
                for (f.calls.items) |t| if (r.inText(t)) try calls.put(gpa, t, {});
                if (f.insns.items.len > 0 and i + 1 < starts.len) {
                    const last = f.insns.items[f.insns.items.len - 1];
                    const end = last.addr + last.size;
                    if (end > limit or (end == limit and !dec.Function.terminates(last))) try fall.put(gpa, limit, {});
                    const beyond = if (i + 2 < starts.len) starts[i + 2] else te;
                    for (f.exits.items) |e| if (e.t) |t| if (limit < t and t < beyond) try fall.put(gpa, limit, {});
                }
            }
            var changed = false;
            var cit = calls.keyIterator();
            while (cit.next()) |t| {
                const cur = why.get(t.*) orelse 0;
                if (cur & W.call == 0) {
                    try add(&why, t.*, W.call);
                    changed = true;
                }
            }
            var fit = fall.keyIterator();
            while (fit.next()) |fa| if (why.get(fa.*)) |w| if (w == W.pad) {
                _ = why.swapRemove(fa.*);
                changed = true;
            };
            if (!changed) break;
        }
        const starts = try sortedKeys(&why);
        defer gpa.free(starts);
        var reached: std.AutoHashMapUnmanaged(u32, void) = .empty;
        defer reached.deinit(gpa);
        var targets: std.AutoArrayHashMapUnmanaged(u32, void) = .empty;
        defer targets.deinit(gpa);
        var tails: std.AutoArrayHashMapUnmanaged(u32, void) = .empty;
        defer tails.deinit(gpa);
        var ends: std.AutoHashMapUnmanaged(u32, u32) = .empty;
        defer ends.deinit(gpa);
        for (starts, 0..) |s, i| {
            const limit = if (i + 1 < starts.len) starts[i + 1] else te;
            const f = try r.fn_(s, limit);
            try ends.put(gpa, s, f.end);
            for (f.exits.items) |e| if (e.t) |t| if (r.inText(t)) {
                for (f.insns.items) |ins| if (ins.addr == e.at and ins.kind == .jmp) {
                    try tails.put(gpa, t, {});
                    break;
                };
            };
            for (f.insns.items) |ins| {
                try reached.put(gpa, ins.addr, {});
                if (ins.size < 5 or !img.relocs.contains(ins.addr + ins.size - 4)) continue;
                const b0 = ins.bytes[0];
                if (b0 == 0x68 or (b0 >= 0xB8 and b0 <= 0xBF) or b0 == 0xC7) {
                    const v = std.mem.readInt(u32, ins.bytes[ins.size - 4 ..][0..4], .little);
                    if (r.inText(v)) try targets.put(gpa, v, {});
                }
            }
        }
        var added: usize = 0;
        var pending: std.ArrayList(struct { a: u32, w: u16 }) = .empty;
        defer pending.deinit(gpa);
        for (targets.keys()) |v| if (!reached.contains(v) and !why.contains(v)) try pending.append(gpa, .{ .a = v, .w = W.fptr });
        var sit = seh_all.keyIterator();
        while (sit.next()) |v| if (!reached.contains(v.*) and !why.contains(v.*)) try pending.append(gpa, .{ .a = v.*, .w = W.seh });
        for (others.keys()) |v| {
            if (reached.contains(v) or why.contains(v) or eh.contains(v)) continue;
            const i = floorIdx(starts, v) orelse continue;
            const e = ends.get(starts[i]).?;
            if (e <= v and r.padOnlyBetween(e, v)) try pending.append(gpa, .{ .a = v, .w = W.data });
        }
        for (tails.keys()) |v| {
            if (reached.contains(v) or why.contains(v)) continue;
            const i = floorIdx(starts, v) orelse continue;
            const e = ends.get(starts[i]).?;
            if (e <= v and r.padOnlyBetween(e, v)) try pending.append(gpa, .{ .a = v, .w = W.jmp });
        }
        for (pending.items) |p| {
            try add(&why, p.a, p.w);
            added += 1;
        }
        if (added == 0) break;
    }

    const starts = try sortedKeys(&why);
    var roots: std.AutoHashMapUnmanaged(u32, std.ArrayList(u32)) = .empty;
    for (others.keys()) |v| {
        if (why.contains(v)) continue;
        const i = floorIdx(starts, v) orelse continue;
        const g = try roots.getOrPut(gpa, starts[i]);
        if (!g.found_existing) g.value_ptr.* = .empty;
        try g.value_ptr.append(gpa, v);
    }
    const out_z = out_path;
    const fo = cio.fopen(out_z, "w") orelse return error.OpenOut;
    defer _ = cio.fclose(fo);
    _ = cio.fprintf(fo, "start\tend\tinsns\twhy\troots\n");
    for (starts, 0..) |s, i| {
        const limit = if (i + 1 < starts.len) starts[i + 1] else te;
        const f = try r.fn_(s, limit);
        var end = f.end;
        var n: usize = f.insns.items.len;
        var rl: []u32 = &.{};
        if (roots.getPtr(s)) |lst| {
            rl = lst.items;
            std.mem.sort(u32, rl, {}, std.sort.asc(u32));
            var have: std.AutoHashMapUnmanaged(u32, void) = .empty;
            defer have.deinit(gpa);
            for (f.insns.items) |ins| try have.put(gpa, ins.addr, {});
            for (rl) |rt| {
                const g = try r.fn_(rt, limit);
                for (g.insns.items) |ins| if (!have.contains(ins.addr)) {
                    try have.put(gpa, ins.addr, {});
                    n += 1;
                };
                end = @max(end, g.end);
            }
        }
        var wbuf: [64]u8 = undefined;
        var wl: usize = 0;
        const bits = why.get(s).?;
        for (WHY, 0..) |name, bi| if (bits & (@as(u16, 1) << @intCast(bi)) != 0) {
            if (wl > 0) {
                wbuf[wl] = '+';
                wl += 1;
            }
            @memcpy(wbuf[wl .. wl + name.len], name);
            wl += name.len;
        };
        wbuf[wl] = 0;
        _ = cio.fprintf(fo, "%08x\t%08x\t%zu\t%s\t", s, end, n, @as([*c]const u8, &wbuf));
        for (rl, 0..) |rt, k| _ = cio.fprintf(fo, if (k == 0) "%08x" else ",%08x", rt);
        _ = cio.fprintf(fo, "\n");
    }
    _ = cio.printf("%zu functions -> %s\n", starts.len, out_z.ptr);
}
