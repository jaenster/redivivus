//! The original image (refimage.py): the carve into functions, functions.tsv, and the decoded
//! model the comparisons read (Reference.model: each function decoded, with what it refers to).
const std = @import("std");
const pe_mod = @import("pe.zig");
const dec = @import("decode.zig");
const proc = @import("proc.zig");
const carve_seed = @import("carve_seed.zig");
const util = @import("util.zig");

const gpa = util.gpa;

// Reasons, in the alphabetical order Python's sorted() joins them.
const WHY = [_][]const u8{ "call", "data", "entry", "export", "fptr", "jmp", "pad", "seed", "seh", "vtbl" };
const W = struct {
    const call = 1 << 0;
    const data = 1 << 1;
    const entry = 1 << 2;
    const exp = 1 << 3;
    const fptr = 1 << 4;
    const jmp = 1 << 5;
    const pad = 1 << 6;
    const seed = 1 << 7;
    const seh = 1 << 8;
    const vtbl = 1 << 9;
};

/// One row of functions.tsv.
pub const Carved = struct { end: u32, ninsns: usize, why: []const u8, roots: []u32 };

pub const RefKind = enum { import, string, self, fptr, data, call };

/// (insn addr, field offset, size, kind, target) - refimage.refs_of. `int` holds the address
/// (self: the offset from the start); `bytes` the import's 'dll!name' or the string's bytes.
pub const Ref = struct { a: u32, off: u8, size: u8, kind: RefKind, int: u32 = 0, bytes: []const u8 = "" };

pub const ModelEntry = struct { f: dec.Function, refs: []Ref };

pub const Reference = struct {
    img: pe_mod.Image,
    code: []u8,
    ts: u32,
    te: u32,
    d: dec.Decoder,
    cache: std.AutoHashMapUnmanaged(u64, dec.Function) = .empty,
    /// start -> (end, ninsns, why, roots), from carve() or load().
    functions: std.AutoArrayHashMapUnmanaged(u32, Carved) = .empty,
    starts: []u32 = &.{},
    imports: ?std.AutoHashMapUnmanaged(u32, []const u8) = null,

    pub fn open(exe: []const u8) !*Reference {
        const data = try util.readFile(exe);
        const img = try pe_mod.Image.parse(gpa, data);
        const text = img.section(".text") orelse return error.NoText;
        const r = try gpa.create(Reference);
        r.* = .{ .img = img, .code = undefined, .ts = img.base + text.va, .te = img.base + text.va + text.vsize, .d = try dec.Decoder.init(null) };
        r.code = try gpa.alloc(u8, text.vsize);
        _ = r.img.read(r.ts, r.code);
        return r;
    }

    pub fn inText(r: *const Reference, v: u32) bool {
        return r.ts <= v and v < r.te;
    }

    pub fn relocAt(r: *const Reference, a: u32) ?u32 {
        if (!r.img.relocs.contains(a)) return null;
        if (r.inText(a)) return std.mem.readInt(u32, r.code[a - r.ts ..][0..4], .little);
        return r.img.u32At(a);
    }

    pub fn externalAt(_: *const Reference, _: u32) bool {
        return false;
    }

    fn fn_(r: *Reference, s: u32, limit: u32) !*dec.Function {
        const key = (@as(u64, s) << 32) | limit;
        const g = try r.cache.getOrPut(gpa, key);
        if (!g.found_existing) g.value_ptr.* = try r.d.decode(gpa, r.code, r.ts, s, limit, r, null);
        return g.value_ptr;
    }

    /// Reference.decode: the function and its extra roots (handlers, funclets), decoded with
    /// the same limit and merged.
    pub fn decodeWithRoots(r: *Reference, start: u32, limit: u32, roots: []const u32) !dec.Function {
        var f = try r.d.decode(gpa, r.code, r.ts, start, limit, r, null);
        if (roots.len > 0) {
            for (roots) |rt| {
                const g = try r.d.decode(gpa, r.code, r.ts, rt, limit, r, null);
                try f.mergeRoot(gpa, &g, true);
            }
            f.sortInsns();
        }
        return f;
    }

    fn padOnlyBetween(r: *const Reference, a: u32, b: u32) bool {
        for (r.code[a - r.ts .. b - r.ts]) |x| if (x != 0xCC) return false;
        return true;
    }

    fn u32c(r: *const Reference, k: usize) u32 {
        return std.mem.readInt(u32, r.code[k..][0..4], .little);
    }

    // -- the carve ------------------------------------------------------------

    fn sehPrologs(r: *Reference, ptr_at: *const std.AutoHashMapUnmanaged(u32, u32), out: *std.AutoHashMapUnmanaged(u32, void)) !void {
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

    fn ehTargets(r: *Reference, ptrs: *const std.AutoHashMapUnmanaged(u32, u32), out: *std.AutoHashMapUnmanaged(u32, void)) !void {
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

    /// refimage.carve: fills `functions`.
    pub fn carve(r: *Reference) !void {
        const img = &r.img;
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
        // Ghidra confirms NewNode starts immediately after a ret 4, without
        // the CC byte the pad-seed rule expects. Apply only to its exact image.
        if (carve_seed.newNodeStart(img.data)) |s| if (r.inText(s)) try add(&why, s, W.seed);
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
                var calls: std.AutoHashMapUnmanaged(u32, void) = .empty;
                var fall: std.AutoHashMapUnmanaged(u32, void) = .empty;
                for (starts, 0..) |s, i| {
                    const limit = if (i + 1 < starts.len) starts[i + 1] else te;
                    const f = try r.fn_(s, limit);
                    for (f.calls.items) |cl| if (r.inText(cl.t)) try calls.put(gpa, cl.t, {});
                    if (f.insns.items.len > 0 and i + 1 < starts.len) {
                        const last = f.insns.items[f.insns.items.len - 1];
                        const end = last.addr + last.size;
                        if (end > limit or (end == limit and !last.terminates())) try fall.put(gpa, limit, {});
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
            var reached: std.AutoHashMapUnmanaged(u32, void) = .empty;
            var targets: std.AutoArrayHashMapUnmanaged(u32, void) = .empty;
            var tails: std.AutoArrayHashMapUnmanaged(u32, void) = .empty;
            var ends: std.AutoHashMapUnmanaged(u32, u32) = .empty;
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
            var wl: std.ArrayList(u8) = .empty;
            const bits = why.get(s).?;
            for (WHY, 0..) |name, bi| if (bits & (@as(u16, 1) << @intCast(bi)) != 0) {
                if (wl.items.len > 0) try wl.append(gpa, '+');
                try wl.appendSlice(gpa, name);
            };
            try r.functions.put(gpa, s, .{ .end = end, .ninsns = n, .why = wl.items, .roots = rl });
        }
        r.starts = starts;
    }

    pub fn writeFunctions(r: *const Reference, path: []const u8) !void {
        var out: std.ArrayList(u8) = .empty;
        try out.appendSlice(gpa, "start\tend\tinsns\twhy\troots\n");
        for (r.starts) |s| {
            const f = r.functions.get(s).?;
            try out.print(gpa, "{x:0>8}\t{x:0>8}\t{d}\t{s}\t", .{ s, f.end, f.ninsns, f.why });
            for (f.roots, 0..) |rt, k| try out.print(gpa, "{s}{x:0>8}", .{ if (k == 0) "" else ",", rt });
            try out.append(gpa, '\n');
        }
        try util.writeFile(path, out.items);
    }

    /// Reference.load: functions.tsv as the carve wrote it.
    pub fn load(r: *Reference, path: []const u8) !void {
        var it = util.Lines{ .data = try util.readFile(path) };
        while (it.next()) |line| {
            if (std.mem.startsWith(u8, line, "#") or std.mem.startsWith(u8, line, "start")) continue;
            const p = try util.split(line, '\t');
            var roots: std.ArrayList(u32) = .empty;
            if (p.len > 4) {
                var ri = std.mem.splitScalar(u8, p[4], ',');
                while (ri.next()) |x| if (x.len > 0) try roots.append(gpa, try util.hex(x));
            }
            try r.functions.put(gpa, try util.hex(p[0]), .{ .end = try util.hex(p[1]), .ninsns = try std.fmt.parseInt(usize, p[2], 10), .why = p[3], .roots = roots.items });
        }
        r.starts = try gpa.dupe(u32, r.functions.keys());
        std.mem.sort(u32, r.starts, {}, std.sort.asc(u32));
    }

    // -- what each function refers to ------------------------------------------

    /// A NUL-terminated string (ASCII or UTF-16) starting at va in a data section, as the
    /// bytes a literal would hold, else null.
    pub fn stringAt(r: *const Reference, va: u32) !?[]const u8 {
        const s = r.img.sectionAt(va) orelse return null;
        if (!std.mem.eql(u8, s.name, ".rdata") and !std.mem.eql(u8, s.name, ".data")) return null;
        const buf = try gpa.alloc(u8, @min(512, r.img.base + s.end() - va));
        if (!r.img.read(va, buf)) return error.OutsideImage;
        if (std.mem.indexOfScalar(u8, buf, 0)) |k| if (k >= 1) {
            var ok = true;
            for (buf[0..k]) |ch| if (!((32 <= ch and ch < 127) or ch == 9 or ch == 10 or ch == 13)) {
                ok = false;
                break;
            };
            if (ok) return buf[0 .. k + 1];
        };
        if (buf.len >= 4 and buf[1] == 0 and 32 <= buf[0] and buf[0] < 127) { // UTF-16
            var j: usize = 0;
            while (j + 1 < buf.len) : (j += 2) {
                if (buf[j] == 0 and buf[j + 1] == 0) return if (j >= 2) buf[0 .. j + 2] else null;
            }
        }
        return null;
    }

    /// refs_of: every base-relocated field and every branch/call that leaves the function.
    pub fn refsOf(r: *Reference, start: u32, f: *const dec.Function) ![]Ref {
        if (r.imports == null) r.imports = try r.img.imports(gpa);
        const imports = &r.imports.?;
        const end = f.end;
        var out: std.ArrayList(Ref) = .empty;
        for (f.insns.items) |*i| {
            var field: u8 = 0;
            while (field + 3 < i.size) : (field += 1) {
                const a = i.addr + field;
                if (!r.img.relocs.contains(a)) continue;
                const v = std.mem.readInt(u32, i.bytes[field..][0..4], .little);
                var ref = Ref{ .a = i.addr, .off = field, .size = 4, .kind = .data, .int = v };
                if (imports.get(v)) |nm| {
                    ref.kind = .import;
                    ref.bytes = nm;
                } else if (start <= v and v < end) {
                    ref.kind = .self;
                    ref.int = v - start;
                } else if (r.inText(v)) {
                    ref.kind = .fptr;
                } else if (try r.stringAt(v)) |st| {
                    ref.kind = .string;
                    ref.bytes = st;
                }
                try out.append(gpa, ref);
            }
            // A call of the function's own entry is a call like any other (our object
            // relocates it against our symbol).
            if (i.target) |t| if (!(start <= t and t < end) or (i.bytes[0] == 0xE8 and t == start)) {
                const off: u8 = if (i.bytes[0] == 0xE8 or i.bytes[0] == 0xE9 or (i.bytes[0] == 0x0F and i.size >= 6)) i.size - 4 else i.size - 1;
                try out.append(gpa, .{ .a = i.addr, .off = off, .size = i.size - off, .kind = .call, .int = t });
            };
        }
        return out.items;
    }

    /// Reference.model: start -> (decoded function with text, refs), for every carved function.
    pub fn model(r: *Reference) !std.AutoHashMapUnmanaged(u32, ModelEntry) {
        var out: std.AutoHashMapUnmanaged(u32, ModelEntry) = .empty;
        try out.ensureTotalCapacity(gpa, @intCast(r.starts.len));
        r.d.text = gpa;
        defer r.d.text = null;
        for (r.starts, 0..) |s, k| {
            const limit = if (k + 1 < r.starts.len) r.starts[k + 1] else r.te;
            const f = try r.decodeWithRoots(s, limit, r.functions.get(s).?.roots);
            const refs = try r.refsOf(s, &f);
            out.putAssumeCapacity(s, .{ .f = f, .refs = refs });
        }
        return out;
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

/// `oracle-zig carve [Game.exe] [functions.tsv]`
pub fn carveMain(args: []const []const u8) !void {
    const exe = if (args.len > 0) args[0] else "../reference/Game.exe";
    const out = if (args.len > 1) args[1] else "../build/ref/functions.tsv";
    const r = try Reference.open(exe);
    try r.carve();
    try r.writeFunctions(out);
    util.say(try std.fmt.allocPrint(gpa, "{d} functions -> {s}\n", .{ r.starts.len, out }));
}

/// On a native Windows host the Bash `oracle` wrapper is unavailable. Derive
/// the reference cache from an already present Game.exe when it is missing.
pub fn ensureFunctions(oracle_root: []const u8) !void {
    const exe = try util.join(oracle_root, "reference/Game.exe");
    if (!util.exists(exe)) {
        std.debug.print("reference image missing: {s} (provide Game.exe before running the oracle)\n", .{exe});
        return error.ReferenceImageMissing;
    }
    const expected = "631066c1649c4ea9ffe48bf97e24c00bca1f7a6759c21150f1a79982589adaaf";
    var digest: [32]u8 = undefined;
    std.crypto.hash.sha2.Sha256.hash(try util.readFile(exe), &digest, .{});
    const actual = std.fmt.bytesToHex(digest, .lower);
    if (!std.mem.eql(u8, actual[0..], expected)) {
        std.debug.print("reference image SHA-256 mismatch: {s} has {s}, expected {s}\n", .{ exe, actual, expected });
        return error.ReferenceImageMismatch;
    }
    const out = try util.join(oracle_root, "build/ref/functions.tsv");
    if (util.exists(out)) return;
    util.makeDirs(util.dirname(out));
    const tmp = try std.mem.concat(gpa, u8, &.{ out, ".new" });
    const r = try Reference.open(exe);
    try r.carve();
    try r.writeFunctions(tmp);
    try proc.rename(tmp, out);
    util.say(try std.fmt.allocPrint(gpa, "{d} functions -> {s}\n", .{ r.starts.len, out }));
}
