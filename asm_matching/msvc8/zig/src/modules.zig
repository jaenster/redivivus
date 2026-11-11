//! The tree's modules as the CMake build links them (modules.py): Source/modules.cmake names
//! every module, its directory and data unit, in the executable's link order; the link order
//! of our TUs follows from it (`linkOrder`), and with it which copy of a COMDAT the link keeps.
//! `linked` and `moduleOf` by repository are what permodule reads.
const std = @import("std");
const c = @import("c");
const util = @import("util.zig");
const coff = @import("coff.zig");
const cli = @import("cli.zig");

const gpa = util.gpa;

pub const ROOT_LIB = "root"; // CMake's d2_root: what no module owns
const SKIP_KIND = "headers-only";
const SKIP_MODULE = "libbacktrace"; // GCC only; CMake leaves it out of the module libraries too

pub const Module = struct { name: []const u8, dir: []const u8, data: []const u8, kind: []const u8 };

pub const Modules = struct {
    mods: []Module,
    /// D2_MODULE_<m>_UNITS, for each module that has a non-empty list.
    units: std.StringHashMapUnmanaged([]const []const u8),

    fn skipped(m: Module) bool {
        return std.mem.eql(u8, m.kind, SKIP_KIND) or std.mem.eql(u8, m.name, SKIP_MODULE);
    }

    /// The library a TU goes into: the module whose data unit it is, else the module with
    /// the longest directory above it, else the root library.
    pub fn moduleOf(ms: *const Modules, tu: []const u8) ![]const u8 {
        const cpp = try std.fmt.allocPrint(gpa, "{s}.cpp", .{tu});
        for (ms.mods) |m| if (m.data.len > 0 and std.mem.eql(u8, cpp, m.data) and !std.mem.eql(u8, m.name, SKIP_MODULE)) return m.name;
        var best: []const u8 = ROOT_LIB;
        var blen: isize = -1;
        for (ms.mods) |m| {
            if (m.dir.len > 0 and tu.len > m.dir.len and std.mem.startsWith(u8, tu, m.dir) and tu[m.dir.len] == '/' and @as(isize, @intCast(m.dir.len)) > blen and !skipped(m)) {
                best = m.name;
                blen = @intCast(m.dir.len);
            }
        }
        return best;
    }

    /// [(module, [tu, ...])] in link order, empty libraries left out (modules.libraries).
    /// `tus` may repeat a TU, as Python's generator of every function's TU does.
    pub fn linkOrder(ms: *const Modules, tus: []const []const u8) ![][]const u8 {
        var data: std.StringHashMapUnmanaged([]const u8) = .empty; // data unit (no .cpp) -> module
        for (ms.mods) |m| if (std.mem.endsWith(u8, m.data, ".cpp")) try data.put(gpa, m.data[0 .. m.data.len - 4], m.name);
        var groups: std.StringHashMapUnmanaged(std.ArrayList([]const u8)) = .empty;
        for (tus) |tu| {
            const g = try groups.getOrPut(gpa, try ms.moduleOf(tu));
            if (!g.found_existing) g.value_ptr.* = .empty;
            try g.value_ptr.append(gpa, tu);
        }
        var libs: std.ArrayList([]const u8) = .empty;
        for (ms.mods) |m| if (!skipped(m)) try libs.append(gpa, m.name);
        try libs.append(gpa, ROOT_LIB);
        var out: std.ArrayList([]const u8) = .empty;
        for (libs.items) |n| {
            const grp = groups.get(n) orelse continue;
            if (grp.items.len == 0) continue;
            var have: std.StringHashMapUnmanaged(void) = .empty;
            for (grp.items) |t| try have.put(gpa, t, {});
            var first: std.ArrayList([]const u8) = .empty;
            var in_first: std.StringHashMapUnmanaged(void) = .empty;
            if (ms.units.get(n)) |listed| for (listed) |t| {
                if (have.contains(t) and !isDataOf(&data, t, n)) {
                    try first.append(gpa, t);
                    try in_first.put(gpa, t, {});
                }
            };
            try out.appendSlice(gpa, first.items);
            for (try sort1(grp.items)) |t| if (!isDataOf(&data, t, n) and !in_first.contains(t)) try out.append(gpa, t);
            for (grp.items) |t| if (isDataOf(&data, t, n)) try out.append(gpa, t);
        }
        return out.items;
    }
};

fn isDataOf(data: *const std.StringHashMapUnmanaged([]const u8), t: []const u8, n: []const u8) bool {
    const m = data.get(t) orelse return false;
    return std.mem.eql(u8, m, n);
}

fn isWord(ch: u8) bool {
    return std.ascii.isAlphanumeric(ch) or ch == '_' or ch >= 0x80;
}

fn isSpace(ch: u8) bool {
    return ch == ' ' or ch == '\t' or ch == '\n' or ch == '\r' or ch == 0x0b or ch == 0x0c;
}

fn skipSpace(s: []const u8, i: usize) usize {
    var k = i;
    while (k < s.len and isSpace(s[k])) k += 1;
    return k;
}

/// `\s*set\(D2_MODULE_` at the start of the line: the index after it.
fn afterPrefix(line: []const u8, prefix: []const u8) ?usize {
    const k = skipSpace(line, 0);
    if (!std.mem.startsWith(u8, line[k..], prefix)) return null;
    return k + prefix.len;
}

fn lessStr(_: void, a: []const u8, b: []const u8) bool {
    return std.mem.order(u8, a, b) == .lt;
}

fn stripCpp(t: []const u8) []const u8 {
    return if (std.mem.endsWith(u8, t, ".cpp")) t[0 .. t.len - 4] else t;
}

fn splitWs(s: []const u8) ![][]const u8 {
    var out: std.ArrayList([]const u8) = .empty;
    var it = std.mem.tokenizeAny(u8, s, " \t\n\r\x0b\x0c");
    while (it.next()) |t| try out.append(gpa, t);
    return out.items;
}

/// set(D2_MODULE_<m>_(DIR|DATA|KIND) "<v>") - `(\w+?)` lazy, `"(.*)"\)\s*$` greedy.
fn attrLine(body: []const u8) ?struct { []const u8, []const u8, []const u8 } {
    var k: usize = 1;
    while (k <= body.len) : (k += 1) {
        if (!isWord(body[k - 1])) return null;
        const rest = body[k..];
        for ([_][]const u8{ "_DIR", "_DATA", "_KIND" }) |tag| {
            if (!std.mem.startsWith(u8, rest, tag)) continue;
            var p = tag.len;
            if (p >= rest.len or !isSpace(rest[p])) continue;
            p = skipSpace(rest, p);
            if (p >= rest.len or rest[p] != '"') continue;
            // The value runs to the last `")` followed by nothing but whitespace (\s*$).
            var e = rest.len;
            while (e > p + 1) : (e -= 1) {
                if (rest[e - 1] == ')' and rest[e - 2] == '"' and e - 2 > p and skipSpace(rest, e) == rest.len) {
                    const v = rest[p + 1 .. e - 2];
                    if (std.mem.indexOfScalar(u8, v, '\n') == null) return .{ body[0..k], tag[1..], v };
                }
            }
        }
    }
    return null;
}

var cached: ?struct { repo: []const u8, ms: Modules } = null;

/// load(), read once per repository.
pub fn loadCached(repo: []const u8) !*const Modules {
    if (cached == null or !std.mem.eql(u8, cached.?.repo, repo)) cached = .{ .repo = repo, .ms = try load(repo) };
    return &cached.?.ms;
}

/// The module libraries the executable links, in order, the root library last.
pub fn linked(repo: []const u8) ![][]const u8 {
    const ms = try loadCached(repo);
    var out: std.ArrayList([]const u8) = .empty;
    for (ms.mods) |m| if (!Modules.skipped(m)) try out.append(gpa, m.name);
    try out.append(gpa, ROOT_LIB);
    return out.items;
}

/// The library a TU goes into (modules.module_of).
pub fn moduleOf(tu: []const u8, repo: []const u8) ![]const u8 {
    return (try loadCached(repo)).moduleOf(tu);
}

/// Source/modules.cmake of `repo`, read as modules.load does.
pub fn load(repo: []const u8) !Modules {
    const path = try std.fmt.allocPrint(gpa, "{s}/Source/modules.cmake", .{repo});
    const text = try util.readFile(path);
    var order: [][]const u8 = &.{};
    const Attrs = struct { dir: []const u8 = "", data: []const u8 = "", kind: []const u8 = "" };
    var attrs: std.StringHashMapUnmanaged(Attrs) = .empty;
    var units: std.StringArrayHashMapUnmanaged(std.ArrayList([]const u8)) = .empty;
    var listing: ?[]const u8 = null;
    var it = util.Lines{ .data = text };
    while (it.next()) |line| {
        if (listing) |l| {
            const hash = std.mem.indexOfScalar(u8, line, '#') orelse line.len;
            const body = util.strip(line[0..hash]);
            if (std.mem.startsWith(u8, body, ")")) {
                listing = null;
            } else if (body.len > 0) try units.getPtr(l).?.append(gpa, stripCpp(body));
            continue;
        }
        if (afterPrefix(line, "set(D2_MODULE_")) |p| {
            // set\(D2_MODULE_(\w+?)_UNITS\s*(.*)$
            var k: usize = 1;
            var name: ?[]const u8 = null;
            while (p + k <= line.len) : (k += 1) {
                if (!isWord(line[p + k - 1])) break;
                if (std.mem.startsWith(u8, line[p + k ..], "_UNITS")) {
                    name = line[p .. p + k];
                    break;
                }
            }
            if (name) |nm| {
                const after = skipSpace(line, p + nm.len + 6);
                const hash = std.mem.indexOfScalarPos(u8, line, after, '#') orelse line.len;
                const rest = util.strip(line[after..hash]);
                const g = try units.getOrPut(gpa, nm);
                g.value_ptr.* = .empty;
                for (try splitWs(std.mem.trimEnd(u8, rest, ")"))) |t| try g.value_ptr.append(gpa, stripCpp(t));
                if (!std.mem.endsWith(u8, rest, ")")) listing = nm;
                continue;
            }
        }
        if (afterPrefix(line, "set(D2_MODULES")) |p| {
            // set\(D2_MODULES\s+(.*)\)\s*$
            if (p < line.len and isSpace(line[p])) {
                var e = line.len;
                while (e > p and isSpace(line[e - 1])) e -= 1;
                if (e > p and line[e - 1] == ')') {
                    const inner = line[skipSpace(line, p)..@max(skipSpace(line, p), e - 1)];
                    order = try splitWs(inner);
                    continue;
                }
            }
        }
        if (afterPrefix(line, "set(D2_MODULE_")) |p| {
            if (attrLine(line[p..])) |a| {
                const g = try attrs.getOrPut(gpa, a[0]);
                if (!g.found_existing) g.value_ptr.* = .{};
                if (std.mem.eql(u8, a[1], "DIR")) g.value_ptr.dir = a[2] else if (std.mem.eql(u8, a[1], "DATA")) g.value_ptr.data = a[2] else g.value_ptr.kind = a[2];
            }
        }
    }
    if (order.len == 0) {
        std.debug.print("{s}: no D2_MODULES\n", .{path});
        std.process.exit(1);
    }
    const mods = try gpa.alloc(Module, order.len);
    for (order, 0..) |n, i| {
        const a = attrs.get(n) orelse Attrs{};
        mods[i] = .{ .name = n, .dir = a.dir, .data = a.data, .kind = a.kind };
    }
    var u: std.StringHashMapUnmanaged([]const []const u8) = .empty;
    var ui = units.iterator();
    while (ui.next()) |e| if (e.value_ptr.items.len > 0) try u.put(gpa, e.key_ptr.*, e.value_ptr.items);
    return .{ .mods = mods, .units = u };
}

/// The TUs through sort(1), as the build scripts sort paths (the locale's collation); plain
/// sorted() if sort(1) fails or does not return the same set (modules.sort1).
pub fn sort1(tus_in: []const []const u8) ![][]const u8 {
    var uniq: std.StringArrayHashMapUnmanaged(void) = .empty;
    for (tus_in) |t| try uniq.put(gpa, t, {});
    const tus = try gpa.dupe([]const u8, uniq.keys());
    std.mem.sort([]const u8, tus, {}, lessStr);
    var input: std.ArrayList(u8) = .empty;
    for (tus) |t| {
        try input.appendSlice(gpa, t);
        try input.append(gpa, '\n');
    }
    var tmpl = "/tmp/oracle-sort1-XXXXXX".*;
    const fd = c.mkstemp(&tmpl);
    if (fd < 0) return tus;
    const tmp = std.mem.sliceTo(&tmpl, 0);
    defer _ = c.unlink(&tmpl);
    _ = c.close(fd);
    try util.writeFile(tmp, input.items);
    const cmd = try util.z(try std.fmt.allocPrint(gpa, "sort < '{s}'", .{tmp}));
    const p = c.popen(cmd.ptr, "r") orelse return tus;
    var buf: std.ArrayList(u8) = .empty;
    var chunk: [65536]u8 = undefined;
    while (true) {
        const n = c.fread(&chunk, 1, chunk.len, p);
        if (n == 0) break;
        try buf.appendSlice(gpa, chunk[0..n]);
    }
    const rc = c.pclose(p);
    var out: std.ArrayList([]const u8) = .empty;
    var li = std.mem.splitScalar(u8, buf.items, '\n');
    while (li.next()) |l| if (l.len > 0) try out.append(gpa, l);
    if (rc != 0) return tus;
    const chk = try gpa.dupe([]const u8, out.items);
    std.mem.sort([]const u8, chk, {}, lessStr);
    if (chk.len != tus.len) return tus;
    for (chk, tus) |a, b| if (!std.mem.eql(u8, a, b)) return tus;
    return out.items;
}

// ---- includes: one /INCLUDE symbol per library member (modules.include_symbols)

const SCN_LNK_INFO = 0x200;
const SCN_LNK_REMOVE = 0x800;
const SCN_MEM_DISCARDABLE = 0x02000000;
const ENTRY_ROOTS = [_][]const u8{"_WinMain@16"}; // what the C runtime's entry calls into ours

/// (object index, section number)
const Node = struct { obj: u32, sec: u32 };
const NodeSet = std.AutoHashMapUnmanaged(Node, void);

fn reaches(sec: *const coff.Section) bool {
    return sec.flags & (SCN_LNK_INFO | SCN_LNK_REMOVE | SCN_MEM_DISCARDABLE) == 0;
}

fn isComdat(sec: *const coff.Section) bool {
    return sec.flags & coff.SCN_LNK_COMDAT != 0;
}

/// The external symbols an object defines: (symbol, its section).
fn externs(o: *const coff.Object) ![]*const coff.Symbol {
    var out: std.ArrayList(*const coff.Symbol) = .empty;
    for (o.symbols) |*sp| {
        if (sp.*) |*s| if (s.storage == coff.C_EXTERNAL and s.section > 0) try out.append(gpa, s);
    }
    return out.items;
}

/// The sections /OPT:REF keeps when the objects are linked whole in this order (or, with
/// noref, what /OPT:NOREF keeps): modules.kept_sections.
fn keptSections(objs: []const coff.Object, noref: bool) !NodeSet {
    var kept: NodeSet = .empty;
    if (noref) {
        var owner: std.StringHashMapUnmanaged(Node) = .empty; // COMDAT symbol -> first (obj, section)
        for (objs, 0..) |*o, i| for (try externs(o)) |s| {
            if (isComdat(&o.sections[@intCast(s.section)])) {
                const g = try owner.getOrPut(gpa, s.name);
                if (!g.found_existing) g.value_ptr.* = .{ .obj = @intCast(i), .sec = @intCast(s.section) };
            }
        };
        var dup: NodeSet = .empty; // later objects' copies, discarded
        for (objs, 0..) |*o, i| for (try externs(o)) |s| {
            if (isComdat(&o.sections[@intCast(s.section)])) {
                const own = owner.get(s.name).?;
                const here = Node{ .obj = @intCast(i), .sec = @intCast(s.section) };
                if (!std.meta.eql(own, here)) try dup.put(gpa, here, {});
            }
        };
        for (objs, 0..) |*o, i| for (o.sections[1..]) |*sec| {
            if (sec.size == 0 or !reaches(sec)) continue;
            const k = Node{ .obj = @intCast(i), .sec = sec.number };
            const assoc_dup = sec.comdat != null and sec.comdat.? == 5 and sec.comdat_assoc != 0 and
                dup.contains(.{ .obj = @intCast(i), .sec = sec.comdat_assoc });
            if (!dup.contains(k) and !assoc_dup) try kept.put(gpa, k, {});
        };
        return kept;
    }
    var first: std.StringHashMapUnmanaged(Node) = .empty; // external symbol -> (obj, section)
    for (objs, 0..) |*o, i| for (try externs(o)) |s| {
        const g = try first.getOrPut(gpa, s.name);
        if (!g.found_existing) g.value_ptr.* = .{ .obj = @intCast(i), .sec = @intCast(s.section) };
    };
    var assoc: std.AutoHashMapUnmanaged(Node, std.ArrayList(Node)) = .empty;
    var work: std.ArrayList(Node) = .empty;
    for (objs, 0..) |*o, i| for (o.sections[1..]) |*sec| {
        if (sec.comdat != null and sec.comdat.? == 5 and sec.comdat_assoc != 0) {
            const g = try assoc.getOrPut(gpa, .{ .obj = @intCast(i), .sec = sec.comdat_assoc });
            if (!g.found_existing) g.value_ptr.* = .empty;
            try g.value_ptr.append(gpa, .{ .obj = @intCast(i), .sec = sec.number });
        } else if (!isComdat(sec) and sec.size != 0 and reaches(sec)) {
            try work.append(gpa, .{ .obj = @intCast(i), .sec = sec.number });
        }
    };
    for (ENTRY_ROOTS) |r| if (first.get(r)) |n| try work.append(gpa, n);
    while (work.pop()) |node| {
        if (kept.contains(node)) continue;
        try kept.put(gpa, node, {});
        const o = &objs[node.obj];
        if (assoc.get(node)) |list| try work.appendSlice(gpa, list.items);
        for (o.sections[node.sec].relocs) |r| {
            if (r.symbol >= o.symbols.len) continue;
            const sym = &(o.symbols[r.symbol] orelse continue);
            if (sym.storage == coff.C_EXTERNAL) {
                if (first.get(sym.name)) |t| try work.append(gpa, t);
            } else if (sym.section > 0) {
                try work.append(gpa, .{ .obj = node.obj, .sec = @intCast(sym.section) });
            }
        }
    }
    return kept;
}

/// A COMDAT section's shape for /OPT:ICF: its flags, bytes and relocation targets, as one key.
fn shape(objs: []const coff.Object, i: u32, n: u32) ![]const u8 {
    const o = &objs[i];
    const sec = &o.sections[n];
    var k: std.ArrayList(u8) = .empty;
    try k.print(gpa, "{d}:{d}:", .{ sec.flags, sec.data.len });
    try k.appendSlice(gpa, sec.data);
    for (sec.relocs) |r| {
        try k.print(gpa, "|{d},{d},", .{ r.offset, r.type });
        const sym: ?*const coff.Symbol = if (r.symbol < o.symbols.len) (if (o.symbols[r.symbol]) |*s| s else null) else null;
        if (sym) |s| {
            if (s.storage == coff.C_EXTERNAL) {
                try k.print(gpa, "n{d}:", .{s.name.len});
                try k.appendSlice(gpa, s.name);
            } else try k.print(gpa, "t{d},{d},{d}", .{ i, s.section, s.value });
        } else try k.append(gpa, '-');
    }
    return k.items;
}

const Include = struct { sym: ?[]const u8, how: []const u8 };
const Cand = struct { comdat: bool, code: bool, foldable: bool, name: []const u8 };

/// {obj path: (symbol, how)}: modules.include_symbols.
fn includeSymbols(objects: []const []const u8, libs: []const []const u8, noref: bool) !std.StringHashMapUnmanaged(Include) {
    const objs = try gpa.alloc(coff.Object, objects.len);
    for (objects, 0..) |p, i| objs[i] = try coff.Object.parse(try util.readFile(p), p);
    const kept = try keptSections(objs, noref);
    var first: std.StringHashMapUnmanaged(u32) = .empty;
    var inlib: std.StringHashMapUnmanaged(u32) = .empty; // library \x00 symbol -> its members defining it
    for (objs, 0..) |*o, i| for (try externs(o)) |s| {
        const g = try first.getOrPut(gpa, s.name);
        if (!g.found_existing) g.value_ptr.* = @intCast(i);
        const key = try std.fmt.allocPrint(gpa, "{s}\x00{s}", .{ libs[i], s.name });
        const h = try inlib.getOrPut(gpa, key);
        if (!h.found_existing) h.value_ptr.* = 0;
        h.value_ptr.* += 1;
    };
    var shapes: std.StringHashMapUnmanaged(u32) = .empty;
    var ki = kept.keyIterator();
    while (ki.next()) |nd| {
        if (!isComdat(&objs[nd.obj].sections[nd.sec])) continue;
        const g = try shapes.getOrPut(gpa, try shape(objs, nd.obj, nd.sec));
        if (!g.found_existing) g.value_ptr.* = 0;
        g.value_ptr.* += 1;
    }
    var out: std.StringHashMapUnmanaged(Include) = .empty;
    for (objects, objs, 0..) |path, *o, ii| {
        const i: u32 = @intCast(ii);
        // min() over (comdat, not leaf, foldable, name)
        var best: ?Cand = null;
        for (try externs(o)) |s| {
            const sn: u32 = @intCast(s.section);
            const key = try std.fmt.allocPrint(gpa, "{s}\x00{s}", .{ libs[i], s.name });
            if (first.get(s.name).? != i or inlib.get(key).? != 1 or !kept.contains(.{ .obj = i, .sec = sn })) continue;
            const sec = &o.sections[sn];
            const comdat = isComdat(sec);
            const leaf = !sec.isCode() and sec.relocs.len == 0;
            const foldable = comdat and shapes.get(try shape(objs, i, sn)).? > 1;
            const cand = Cand{ .comdat = comdat, .code = !leaf, .foldable = foldable, .name = s.name };
            if (best == null or lessCand(cand, best.?)) best = cand;
        }
        var res: Include = undefined;
        if (best) |b| {
            res = if (b.foldable) .{ .sym = b.name, .how = "foldable" } else .{ .sym = b.name, .how = if (b.comdat) "comdat" else "data" };
        } else {
            var any = false;
            var it = kept.keyIterator();
            while (it.next()) |nd| if (nd.obj == i) {
                any = true;
                break;
            };
            res = .{ .sym = null, .how = if (any) "none" else "empty" };
        }
        try out.put(gpa, path, res);
    }
    return out;
}

fn lessCand(a: Cand, b: Cand) bool {
    if (a.comdat != b.comdat) return !a.comdat;
    if (a.code != b.code) return !a.code;
    if (a.foldable != b.foldable) return !a.foldable;
    return std.mem.order(u8, a.name, b.name) == .lt;
}

/// `oracle-zig modules includes [--noref] <list>`: module<TAB>tu<TAB>symbol|-<TAB>how per row
/// of <list> (module<TAB>tu<TAB>obj, in link order).
fn includesMain(list: []const u8, noref: bool) !void {
    const text = try util.readFile(list);
    var rows: std.ArrayList([][]const u8) = .empty;
    var it = util.Lines{ .data = text };
    while (it.next()) |l| {
        if (util.strip(l).len == 0) continue;
        const r = try util.split(std.mem.trimEnd(u8, l, "\n"), '\t');
        if (r.len < 3) cli.exitMsg("{s}: a row with fewer than three fields\n", .{list});
        try rows.append(gpa, r);
    }
    const paths = try gpa.alloc([]const u8, rows.items.len);
    const libs = try gpa.alloc([]const u8, rows.items.len);
    for (rows.items, 0..) |r, i| {
        paths[i] = r[2];
        libs[i] = r[0];
    }
    const syms = try includeSymbols(paths, libs, noref);
    var out: cli.Out = .{};
    for (rows.items) |r| {
        const s = syms.get(r[2]).?;
        try out.print("{s}\t{s}\t", .{ r[0], r[1] });
        if (s.sym) |n| try out.lat(n) else try out.raw("-");
        try out.print("\t{s}\n", .{s.how});
    }
    try out.flush();
}

/// `oracle-zig modules order --repo R < tus`: module<TAB>tu, in link order.
/// `oracle-zig modules includes [--noref] <list>`: the /INCLUDE symbol of each library member.
pub fn main(args: []const []const u8) !void {
    if (args.len >= 1 and std.mem.eql(u8, args[0], "includes")) {
        if (args.len == 2) return includesMain(args[1], false);
        if (args.len == 3 and std.mem.eql(u8, args[1], "--noref")) return includesMain(args[2], true);
        std.debug.print("usage: oracle-zig modules includes [--noref] <list>\n", .{});
        std.process.exit(2);
    }
    var repo: ?[]const u8 = null;
    var k: usize = 0;
    var cmd: ?[]const u8 = null;
    while (k < args.len) : (k += 1) {
        if (std.mem.eql(u8, args[k], "--repo") and k + 1 < args.len) {
            repo = args[k + 1];
            k += 1;
        } else cmd = args[k];
    }
    if (cmd == null or !std.mem.eql(u8, cmd.?, "order") or repo == null) {
        std.debug.print("usage: oracle-zig modules order --repo <tree> < tus\n       oracle-zig modules includes [--noref] <list>\n", .{});
        std.process.exit(2);
    }
    var input: std.ArrayList(u8) = .empty;
    var chunk: [65536]u8 = undefined;
    while (true) {
        const n = c.read(0, &chunk, chunk.len);
        if (n <= 0) break;
        try input.appendSlice(gpa, chunk[0..@intCast(n)]);
    }
    var tus: std.ArrayList([]const u8) = .empty;
    var it = util.Lines{ .data = input.items };
    while (it.next()) |l| {
        const t = util.strip(l);
        if (t.len > 0) try tus.append(gpa, t);
    }
    const ms = try load(repo.?);
    var out: std.ArrayList(u8) = .empty;
    for (try ms.linkOrder(tus.items)) |t| try out.print(gpa, "{s}\t{s}\n", .{ try ms.moduleOf(t), t });
    util.say(out.items);
}

test "modules.cmake attribute lines" {
    const a = attrLine("crashy_DIR \"3rdparty/crashy\")").?;
    try std.testing.expectEqualStrings("crashy", a[0]);
    try std.testing.expectEqualStrings("DIR", a[1]);
    try std.testing.expectEqualStrings("3rdparty/crashy", a[2]);
    try std.testing.expect(attrLine("crashy_UNIT_ORDER original-address)") == null);
}
