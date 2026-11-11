//! datamatch (datamatch.py): byte-compare our initialised data with the original's.
//!
//!   oracle-zig data extents --root <dir> [--snapshot DIR] [--out O] [--strings-out S]
//!   oracle-zig data strings --root <dir> [--snapshot DIR] [--strings-out S]
//!   oracle-zig data [audit] --root <dir> [--extents E] [--obj DIR[=PREFIX] ...] [--datamap D]
//!                   [--out O] [--trend T] [--fixes F] [--label L] [--strings S] [--compiler-data R]
//!
//! The same rows, summary, trend and fixes as datamatch.py, byte for byte; its docstring
//! defines every status. Function names follow the Python's in camelCase.
const std = @import("std");
const c = @import("c");
const audit = @import("audit.zig");
const coff = @import("coff.zig");
const pe_mod = @import("pe.zig");
const difflib = @import("difflib.zig");
const util = @import("util.zig");

const gpa = util.gpa;
const Str = []const u8;

const DATA_LO: u32 = 0x6cc000;
const DATA_HI: u32 = 0x995000; // .rdata and .data (with its zero-fill tail)

const EH_TYPES = [_]Str{
    "FuncInfo",                      "UnwindMapEntry",               "TryBlockMapEntry",        "HandlerType",         "ScopeTableRecord",
    "ESTypeList",                    "_s_FuncInfo",                  "_s_UnwindMapEntry",       "_s_TryBlockMapEntry", "_s_HandlerType",
    "RTTICompleteObjectLocator",     "RTTIClassHierarchyDescriptor", "RTTIBaseClassDescriptor", "RTTIBaseClassArray",  "TypeDescriptor",
    "IMAGE_LOAD_CONFIG_DIRECTORY32", "IMAGE_IMPORT_DESCRIPTOR",      "IMAGE_THUNK_DATA32",
};
const EXT_HEADER = "address\tsize\textent\tname\taliases\tscope\tnamespace\towner\ttype\tkind\treferencing";
const DS_HEADER = "address\tlength\tencoding\tlabel";
const AUDIT_HEADER = "address\tsize\tours_size\tsection\tmodule\tname\tsymbol\ttu\tkind\tmethod\ttype" ++
    "\tstatus\tflags\tfirst_diff\tdiff_bytes\tmatched_bytes\tnotes";
const SKIP_SECTIONS = [_]Str{ ".debug", ".drectve", ".CRT", ".xdata", ".sxdata", ".tls", ".text", ".idata", ".rsrc" };

// -- small Python equivalents ---------------------------------------------------------

fn eql(a: Str, b: Str) bool {
    return std.mem.eql(u8, a, b);
}

fn startsWith(s: Str, p: Str) bool {
    return std.mem.startsWith(u8, s, p);
}

fn startsWithAny(s: Str, ps: []const Str) bool {
    for (ps) |p| if (startsWith(s, p)) return true;
    return false;
}

fn eqlAny(s: Str, ws: []const Str) bool {
    for (ws) |w| if (eql(s, w)) return true;
    return false;
}

fn contains(s: Str, needle: Str) bool {
    return std.mem.indexOf(u8, s, needle) != null;
}

fn fmt(comptime f: []const u8, args: anytype) !Str {
    return std.fmt.allocPrint(gpa, f, args);
}

fn join(parts: []const Str, sep: Str) !Str {
    return std.mem.join(gpa, sep, parts);
}

/// `^(w1|w2|...)(\[|$)`
fn wordThenBracket(s: Str, words: []const Str) bool {
    for (words) |w| if (startsWith(s, w) and (s.len == w.len or s[w.len] == '[')) return true;
    return false;
}

fn isFloatType(s: Str) bool {
    return wordThenBracket(s, &.{ "float", "double", "long double", "float10", "real" });
}

fn isStringType(s: Str) bool {
    return wordThenBracket(s, &.{ "char", "wchar_t", "WCHAR", "TCHAR", "string", "unicode", "TerminatedCString", "TerminatedUnicode" }) or
        eqlAny(s, &.{ "ds", "string", "unicode" });
}

fn isEightByteType(s: Str) bool {
    return wordThenBracket(s, &.{ "double", "longlong", "ulonglong", "__int64", "qword", "undefined8", "LARGE_INTEGER", "ULARGE_INTEGER", "FILETIME", "int64_t", "uint64_t" });
}

/// `^char\[\d+\]$` (ci: case-insensitively)
fn isCharArray(s: Str, ci: bool) bool {
    if (s.len < 7) return false;
    if (ci) {
        if (!std.ascii.eqlIgnoreCase(s[0..5], "char[")) return false;
    } else if (!startsWith(s, "char[")) return false;
    if (s[s.len - 1] != ']') return false;
    const digits = s[5 .. s.len - 1];
    if (digits.len == 0) return false;
    for (digits) |ch| if (!std.ascii.isDigit(ch)) return false;
    return true;
}

fn isStringItemType(s: Str) bool {
    return isCharArray(s, false) or eqlAny(s, &.{ "string", "TerminatedCString" });
}

fn isUntypedStringEncoding(s: Str) bool {
    if (isCharArray(s, true)) return true;
    for ([_]Str{ "string", "TerminatedCString", "string-utf8", "char" }) |w| if (std.ascii.eqlIgnoreCase(s, w)) return true;
    return false;
}

/// POINTERISH = re.compile(r'\*|pointer|PTR|FARPROC|pfn|fn', re.I).search
fn isPointerish(s: Str) bool {
    if (std.mem.indexOfScalar(u8, s, '*') != null) return true;
    for ([_]Str{ "pointer", "ptr", "farproc", "fn" }) |w| {
        var i: usize = 0;
        while (i + w.len <= s.len) : (i += 1) if (std.ascii.eqlIgnoreCase(s[i .. i + w.len], w)) return true;
    }
    return false;
}

fn hex8(v: u64) !Str {
    return fmt("{x:0>8}", .{v});
}

/// Python's `%-Ns`.
fn padR(out: *std.ArrayList(u8), s: Str, w: usize) !void {
    try out.appendSlice(gpa, s);
    if (s.len < w) try out.appendNTimes(gpa, ' ', w - s.len);
}

/// Python's `%Nd`.
fn padLInt(out: *std.ArrayList(u8), v: i64, w: usize) !void {
    const s = try fmt("{d}", .{v});
    if (s.len < w) try out.appendNTimes(gpa, ' ', w - s.len);
    try out.appendSlice(gpa, s);
}

/// Python's `%-Nd`.
fn padRInt(out: *std.ArrayList(u8), v: i64, w: usize) !void {
    try padR(out, try fmt("{d}", .{v}), w);
}

fn pct(out: *std.ArrayList(u8), comptime cfmt: [:0]const u8, a: i64, b: i64) !void {
    try util.fixed(out, cfmt, 100.0 * @as(f64, @floatFromInt(a)) / @as(f64, @floatFromInt(@max(1, b))));
}

/// repr(float), Python's shortest round-trip form.
pub fn pyFloatRepr(x: f64) !Str {
    if (std.math.isNan(x)) return "nan";
    if (std.math.isInf(x)) return if (x > 0) "inf" else "-inf";
    if (x == 0) return if (std.math.signbit(x)) "-0.0" else "0.0";
    var buf: [64]u8 = undefined;
    const sci = try std.fmt.bufPrint(&buf, "{e}", .{x});
    // [-]d[.ddd]e[-]N
    var p: usize = 0;
    const neg = sci[0] == '-';
    if (neg) p = 1;
    const epos = std.mem.indexOfScalar(u8, sci, 'e').?;
    var digits: std.ArrayList(u8) = .empty;
    for (sci[p..epos]) |ch| if (ch != '.') try digits.append(gpa, ch);
    while (digits.items.len > 1 and digits.items[digits.items.len - 1] == '0') _ = digits.pop();
    const e = try std.fmt.parseInt(i32, sci[epos + 1 ..], 10);
    const d = digits.items;
    var out: std.ArrayList(u8) = .empty;
    if (neg) try out.append(gpa, '-');
    if (e < -4 or e >= 16) {
        try out.append(gpa, d[0]);
        if (d.len > 1) {
            try out.append(gpa, '.');
            try out.appendSlice(gpa, d[1..]);
        }
        try out.print(gpa, "e{c}{d:0>2}", .{ @as(u8, if (e < 0) '-' else '+'), @abs(e) });
    } else if (e < 0) {
        try out.appendSlice(gpa, "0.");
        try out.appendNTimes(gpa, '0', @intCast(-e - 1));
        try out.appendSlice(gpa, d);
    } else {
        const ip: usize = @intCast(e + 1);
        if (d.len <= ip) {
            try out.appendSlice(gpa, d);
            try out.appendNTimes(gpa, '0', ip - d.len);
            try out.appendSlice(gpa, ".0");
        } else {
            try out.appendSlice(gpa, d[0..ip]);
            try out.append(gpa, '.');
            try out.appendSlice(gpa, d[ip..]);
        }
    }
    return out.items;
}

/// repr(bytes).
pub fn pyBytesRepr(b: Str) !Str {
    const q: u8 = if (std.mem.indexOfScalar(u8, b, '\'') != null and std.mem.indexOfScalar(u8, b, '"') == null) '"' else '\'';
    var out: std.ArrayList(u8) = .empty;
    try out.append(gpa, 'b');
    try out.append(gpa, q);
    for (b) |ch| {
        if (ch == q or ch == '\\') {
            try out.append(gpa, '\\');
            try out.append(gpa, ch);
        } else if (ch == '\t') {
            try out.appendSlice(gpa, "\\t");
        } else if (ch == '\n') {
            try out.appendSlice(gpa, "\\n");
        } else if (ch == '\r') {
            try out.appendSlice(gpa, "\\r");
        } else if (ch < 0x20 or ch >= 0x7f) {
            try out.print(gpa, "\\x{x:0>2}", .{ch});
        } else try out.append(gpa, ch);
    }
    try out.append(gpa, q);
    return out.items;
}

/// Counter.most_common(): by count, ties in first-insertion order.
const Counter = struct {
    map: std.StringArrayHashMapUnmanaged(i64) = .empty,
    fn add(cn: *Counter, k: Str, n: i64) !void {
        const g = try cn.map.getOrPut(gpa, k);
        if (!g.found_existing) g.value_ptr.* = 0;
        g.value_ptr.* += n;
    }
    fn get(cn: *const Counter, k: Str) i64 {
        return cn.map.get(k) orelse 0;
    }
    const CItem = struct { k: Str, n: i64 };
    fn mostCommon(cn: *const Counter) ![]CItem {
        const items = try gpa.alloc(CItem, cn.map.count());
        for (cn.map.keys(), cn.map.values(), 0..) |k, n, i| items[i] = .{ .k = k, .n = n };
        std.mem.sort(CItem, items, {}, struct {
            fn lt(_: void, a: CItem, b: CItem) bool {
                return a.n > b.n;
            }
        }.lt);
        return items;
    }
};

fn lessStr(_: void, a: Str, b: Str) bool {
    return std.mem.order(u8, a, b) == .lt;
}

fn sortedStrs(xs: []const Str) ![]Str {
    const out = try gpa.dupe(Str, xs);
    std.mem.sort(Str, out, {}, lessStr);
    return out;
}

/// A set of byte strings, in insertion order.
const BSet = std.StringArrayHashMapUnmanaged(void);

fn realpath(p: Str) !Str {
    var buf: [4096]u8 = undefined;
    const pz = try util.z(p);
    const r = c.realpath(pz.ptr, &buf) orelse return p;
    return gpa.dupe(u8, std.mem.sliceTo(r, 0));
}

fn isDir(p: Str) bool {
    const pz = util.z(p) catch return false;
    var st: c.struct_stat = undefined;
    return c.stat(pz.ptr, &st) == 0 and (st.st_mode & c.S_IFMT) == c.S_IFDIR;
}

fn die(comptime f: []const u8, args: anytype) noreturn {
    std.debug.print(f ++ "\n", args);
    std.process.exit(1);
}

// -- JSON values as Python sees them ------------------------------------------------

const JV = std.json.Value;

fn jget(o: JV, key: Str) ?JV {
    if (o != .object) return null;
    return o.object.get(key);
}

/// Python truthiness.
fn truthy(v: ?JV) bool {
    const x = v orelse return false;
    return switch (x) {
        .null => false,
        .bool => |b| b,
        .integer => |i| i != 0,
        .float => |f| f != 0,
        .number_string => |s| s.len > 0,
        .string => |s| s.len > 0,
        .array => |a| a.items.len > 0,
        .object => |m| m.count() > 0,
    };
}

/// str(value).
fn pyStr(v: ?JV) !Str {
    const x = v orelse return "None";
    return switch (x) {
        .null => "None",
        .bool => |b| if (b) "True" else "False",
        .integer => |i| fmt("{d}", .{i}),
        .float => |f| pyFloatRepr(f),
        .number_string => |s| s,
        .string => |s| s,
        else => "?",
    };
}

/// `d.get(key, default)` formatted with %s.
fn getStrOr(o: JV, key: Str, default: Str) !Str {
    return if (jget(o, key)) |v| pyStr(v) else default;
}

/// `d.get(key) or ''`.
fn orEmpty(o: JV, key: Str) !Str {
    const v = jget(o, key);
    return if (truthy(v)) pyStr(v) else "";
}

/// int(d.get(key) or dflt).
fn intOr(o: JV, key: Str, dflt: i64) !i64 {
    const v = jget(o, key);
    if (!truthy(v)) return dflt;
    return switch (v.?) {
        .integer => |i| i,
        .float => |f| @intFromFloat(@trunc(f)),
        .bool => |b| @intFromBool(b),
        .string, .number_string => |s| std.fmt.parseInt(i64, util.strip(s), 10),
        else => error.BadInt,
    };
}

fn parseLine(line: Str) !JV {
    return std.json.parseFromSliceLeaky(JV, gpa, line, .{ .duplicate_field_behavior = .use_last });
}

// -- the original's extents ----------------------------------------------------------

/// A label Ghidra derives from another item: `gaTable[5].field` (not a string label).
fn isLabelName(name: Str) bool {
    if (startsWith(name, "s_") or startsWith(name, "u_")) return false;
    if (std.mem.indexOfScalar(u8, name, '[') != null) return true;
    // re.search(r'[A-Za-z0-9_\]]\.[A-Za-z_]')
    var i: usize = 0;
    while (i + 2 < name.len) : (i += 1) {
        const a = name[i];
        const b = name[i + 2];
        if ((std.ascii.isAlphanumeric(a) or a == '_' or a == ']') and name[i + 1] == '.' and (std.ascii.isAlphabetic(b) or b == '_')) return true;
    }
    return false;
}

const Head = struct { r: JV, name: Str, label: bool };

fn extentsMain(snap: Str, out_path: Str) !void {
    const manifest = try parseLine(try util.readFile(try util.join(snap, "manifest.json")));
    const prov: JV = jget(manifest, "provenance") orelse .{ .object = .empty };
    var by_addr: std.AutoArrayHashMapUnmanaged(u32, std.ArrayList(Head)) = .empty;
    var it = util.Lines{ .data = try util.readFile(try util.join(snap, "globals.ndjson")) };
    while (it.next()) |line| {
        if (util.strip(line).len == 0) continue;
        const r = try parseLine(line);
        const a = try util.hex((jget(r, "address") orelse return error.NoAddress).string);
        if (DATA_LO <= a and a < DATA_HI) {
            var name = (jget(r, "name") orelse return error.NoName).string;
            var label = false;
            if (isLabelName(name)) {
                // The generator writes `gaTable[3][0]` as `gaTable_3__0_`.
                const nm = try gpa.dupe(u8, name);
                for (nm) |*ch| if (ch.* == '[' or ch.* == ']' or ch.* == '.') {
                    ch.* = '_';
                };
                name = nm;
                label = true;
            }
            const g = try by_addr.getOrPut(gpa, a);
            if (!g.found_existing) g.value_ptr.* = .empty;
            try g.value_ptr.append(gpa, .{ .r = r, .name = name, .label = label });
        }
    }
    const addrs = try gpa.dupe(u32, by_addr.keys());
    std.mem.sort(u32, addrs, {}, std.sort.asc(u32));
    const ERow = struct { a: u32, size: i64, extent: i64, name: Str, aliases: Str, scope: Str, ns: Str, owner: Str, typ: Str, kind: Str, refs: Str };
    var out: std.ArrayList(ERow) = .empty;
    var container: ?struct { a: u32, end: i64, size: i64, typ: Str } = null;
    for (addrs) |a| {
        const rs = by_addr.getPtr(a).?.items;
        const key = struct {
            fn k(h: Head) u8 {
                const prim = jget(h.r, "primary");
                const is_true = prim != null and prim.? == .bool and prim.?.bool;
                const is_false = prim != null and prim.? == .bool and !prim.?.bool;
                const dat = startsWith(h.name, "DAT_") or startsWith(h.name, "PTR_") or startsWith(h.name, "LAB_");
                return @as(u8, @intFromBool(h.label)) * 8 + @as(u8, @intFromBool(!is_true)) * 4 + @as(u8, @intFromBool(is_false)) * 2 + @intFromBool(dat);
            }
            fn lt(_: void, x: Head, y: Head) bool {
                return k(x) < k(y);
            }
        };
        std.mem.sort(Head, rs, {}, key.lt);
        const h = rs[0];
        var al: std.ArrayList(Str) = .empty;
        for (rs[1..]) |x| try al.append(gpa, x.name);
        const size = try intOr(h.r, "size", 1);
        const typ = try orEmpty(h.r, "dataType");
        var kind: Str = "data";
        var extent = size;
        const ai: i64 = a;
        if (container != null and container.?.a < a and ai < container.?.end) {
            kind = "interior";
            if (size == container.?.size and eql(typ, container.?.typ)) {
                extent = container.?.end - ai; // it reports the item it sits in
            } else extent = @min(size, container.?.end - ai);
        } else container = .{ .a = a, .end = ai + size, .size = size, .typ = typ };
        const base_t = std.mem.trim(u8, typ[0 .. std.mem.indexOfScalar(u8, typ, '[') orelse typ.len], " *");
        if (truthy(jget(h.r, "importSlot"))) {
            kind = "import";
        } else if (eqlAny(base_t, &EH_TYPES)) kind = "compiler";
        var refs: std.ArrayList(Str) = .empty;
        if (truthy(jget(h.r, "referencingFunctions"))) for (jget(h.r, "referencingFunctions").?.array.items) |v| try refs.append(gpa, try pyStr(v));
        try out.append(gpa, .{ .a = a, .size = size, .extent = extent, .name = h.name, .aliases = try join(al.items, ","), .scope = try orEmpty(h.r, "scope"), .ns = try orEmpty(h.r, "namespace"), .owner = try orEmpty(h.r, "ownerFunction"), .typ = typ, .kind = kind, .refs = try join(refs.items, ",") });
    }
    var o: std.ArrayList(u8) = .empty;
    try o.print(gpa, "# Ghidra {s} v{s}, snapshot written {s}\n", .{ try getStrOr(prov, "programPath", "?"), try getStrOr(prov, "programVersion", "?"), try getStrOr(prov, "writtenAt", "?") });
    try o.appendSlice(gpa, EXT_HEADER ++ "\n");
    var interior: usize = 0;
    for (out.items) |r| {
        try o.print(gpa, "{x:0>8}\t{d}\t{d}\t{s}\t{s}\t{s}\t{s}\t{s}\t{s}\t{s}\t{s}\n", .{ r.a, r.size, r.extent, r.name, r.aliases, r.scope, r.ns, r.owner, r.typ, r.kind, r.refs });
        if (eql(r.kind, "interior")) interior += 1;
    }
    try util.writeFile(out_path, o.items);
    util.say(try fmt("{d} data items ({d} interior labels) from {s} v{s} -> {s}\n", .{ out.items.len, interior, try pyStr(jget(prov, "programPath")), try pyStr(jget(prov, "programVersion")), out_path }));
}

pub const Item = struct {
    addr: u32,
    size: i64,
    extent: i64,
    name: Str,
    aliases: Str,
    scope: Str,
    ns: Str,
    owner: Str,
    typ: Str,
    kind: Str,
    refs: []const Str,
    parent: ?*Item = null,
};

/// re.search(r' v(\d+)', line)
fn versionOf(line: Str) Str {
    var i: usize = 0;
    while (i + 2 < line.len + 0) : (i += 1) {
        if (line[i] == ' ' and line[i + 1] == 'v' and i + 2 < line.len and std.ascii.isDigit(line[i + 2])) {
            var j = i + 2;
            while (j < line.len and std.ascii.isDigit(line[j])) j += 1;
            return line[i + 2 .. j];
        }
    }
    return "?";
}

fn loadExtents(path: Str) !struct { items: []*Item, version: Str } {
    var items: std.ArrayList(*Item) = .empty;
    var version: Str = "?";
    var it = util.Lines{ .data = try util.readFile(path) };
    while (it.next()) |line| {
        if (startsWith(line, "# Ghidra")) {
            version = versionOf(line);
            continue;
        }
        if (startsWith(line, "address\t") or util.strip(line).len == 0) continue;
        const col = try util.split(line, '\t');
        var refs: std.ArrayList(Str) = .empty;
        var rit = std.mem.splitScalar(u8, col[10], ',');
        while (rit.next()) |x| if (x.len > 0) try refs.append(gpa, x);
        const item = try gpa.create(Item);
        item.* = .{ .addr = try util.hex(col[0]), .size = try std.fmt.parseInt(i64, col[1], 10), .extent = try std.fmt.parseInt(i64, col[2], 10), .name = col[3], .aliases = col[4], .scope = col[5], .ns = col[6], .owner = col[7], .typ = col[8], .kind = col[9], .refs = refs.items };
        try items.append(gpa, item);
    }
    // An interior label's container: the item before it whose extent covers it.
    var head: ?*Item = null;
    for (items.items) |item| {
        if (eql(item.kind, "interior")) {
            if (head) |h| if (h.addr < item.addr and @as(i64, item.addr) < @as(i64, h.addr) + h.extent) {
                item.parent = h;
            };
        } else head = item;
    }
    return .{ .items = items.items, .version = version };
}

// -- our side ------------------------------------------------------------------------

const IdxEnt = struct { off: u32, uid: Str };

const Obj = struct {
    o: coff.Object,
    tu: Str,
    /// section -> sorted (offset, uid): a section-relative reference's target
    data_index: std.AutoHashMapUnmanaged(i16, std.ArrayList(IdxEnt)) = .empty,
};

const Ours = struct {
    uid: Str,
    symbol: Str,
    tu: Str,
    obj: *Obj,
    sec: ?*const coff.Section,
    off: u32,
    extent: i64,
    alignment: i64,
    next_align: i64,
    bss: bool,
    common: bool,
};

fn ourUid(tu: Str, s: *const coff.Symbol) !Str {
    return if (s.storage == coff.C_STATIC) fmt("{s}|{s}", .{ tu, s.name }) else s.name;
}

fn secAlign(flags: u32) i64 {
    const n = (flags >> 20) & 0xf;
    return if (n != 0) @as(i64, 1) << @intCast(n - 1) else 16;
}

fn lowbit(v: i64, cap: i64) i64 {
    return if (v != 0) @min(cap, v & -v) else cap;
}

const ObjDir = struct { root: Str, sub: Str };
const ObjMap = std.StringArrayHashMapUnmanaged(*Obj);
const OursMap = std.StringArrayHashMapUnmanaged(*Ours);

/// sorted(glob.glob(os.path.join(root, '**', '*.obj'), recursive=True)), relative to root.
fn globObjs(root: Str, rel: Str, out: *std.ArrayList(Str)) !void {
    const dir_path = if (rel.len == 0) root else try util.join(root, rel);
    const dz = try util.z(dir_path);
    const dp = c.opendir(dz.ptr) orelse return;
    defer _ = c.closedir(dp);
    while (c.readdir(dp)) |ent| {
        const name = std.mem.sliceTo(@as([*:0]const u8, @ptrCast(&ent.*.d_name)), 0);
        if (name.len == 0 or name[0] == '.') continue; // glob skips hidden names
        const nm = try gpa.dupe(u8, name);
        const sub = if (rel.len == 0) nm else try util.join(rel, nm);
        var is_dir = ent.*.d_type == c.DT_DIR;
        if (ent.*.d_type == c.DT_UNKNOWN or ent.*.d_type == c.DT_LNK) is_dir = isDir(try util.join(dir_path, nm));
        if (is_dir) {
            try globObjs(root, sub, out);
        } else if (std.mem.endsWith(u8, nm, ".obj")) try out.append(gpa, sub);
    }
}

fn loadOurs(objdirs: []const ObjDir) !struct { ours: OursMap, dups: usize, objs: ObjMap } {
    var out: OursMap = .empty;
    var dups: std.StringHashMapUnmanaged(void) = .empty;
    var objs: ObjMap = .empty;
    for (objdirs) |od| {
        var rels: std.ArrayList(Str) = .empty;
        try globObjs(od.root, "", &rels);
        std.mem.sort(Str, rels.items, {}, lessStr);
        for (rels.items) |relp| {
            const rel = relp[0 .. relp.len - 4];
            const tu = if (od.sub.len > 0) try fmt("{s}/{s}", .{ od.sub, rel }) else rel;
            const path = try util.join(od.root, relp);
            const o = coff.Object.parse(try util.readFile(path), path) catch continue;
            const obj = try gpa.create(Obj);
            obj.* = .{ .o = o, .tu = tu };
            try objs.put(gpa, tu, obj);
            var per_sec: std.AutoArrayHashMapUnmanaged(i16, std.ArrayList(*const coff.Symbol)) = .empty;
            for (obj.o.symbols) |*sp| {
                const s = &(sp.* orelse continue);
                if (s.section == 0 and s.storage == coff.C_EXTERNAL and s.value > 0 and !s.isFunction()) {
                    const u = try gpa.create(Ours);
                    u.* = .{ .uid = s.name, .symbol = s.name, .tu = tu, .obj = obj, .sec = null, .off = 0, .extent = s.value, .alignment = lowbit(s.value, 8), .next_align = 16, .bss = true, .common = true };
                    if (out.contains(u.uid)) try dups.put(gpa, u.uid, {}) else try out.put(gpa, u.uid, u);
                    continue;
                }
                if (s.section <= 0 or (s.storage != coff.C_EXTERNAL and s.storage != coff.C_STATIC)) continue;
                const sec = &obj.o.sections[@intCast(s.section)];
                if (sec.isCode() or startsWithAny(sec.name, &SKIP_SECTIONS) or s.isFunction()) continue;
                if (s.storage == coff.C_STATIC and eql(s.name, sec.name)) continue;
                const g = try per_sec.getOrPut(gpa, s.section);
                if (!g.found_existing) g.value_ptr.* = .empty;
                try g.value_ptr.append(gpa, s);
            }
            for (per_sec.keys(), per_sec.values()) |secnum, syms| {
                const sec = &obj.o.sections[@intCast(secnum)];
                // Extent: to the next symbol of the section at a higher offset.
                var offs: std.ArrayList(u32) = .empty;
                for (syms.items) |s| if (std.mem.indexOfScalar(u32, offs.items, s.value) == null) try offs.append(gpa, s.value);
                std.mem.sort(u32, offs.items, {}, std.sort.asc(u32));
                for (syms.items) |s| {
                    const k = std.sort.upperBound(u32, offs.items, s.value, struct {
                        fn ord(ctx: u32, x: u32) std.math.Order {
                            return std.math.order(ctx, x);
                        }
                    }.ord);
                    const end: u32 = if (k < offs.items.len) offs.items[k] else sec.size;
                    const at = std.mem.indexOfScalar(u8, s.name, '@') orelse s.name.len;
                    if (startsWith(s.name, "$") or (at > 1 and std.mem.indexOfScalar(u8, s.name[1..at], '$') != null)) continue;
                    const u = try gpa.create(Ours);
                    u.* = .{ .uid = try ourUid(tu, s), .symbol = s.name, .tu = tu, .obj = obj, .sec = sec, .off = s.value, .extent = @as(i64, end) - s.value, .alignment = @min(secAlign(sec.flags), lowbit(s.value, 16)), .next_align = if (k < offs.items.len) lowbit(end, 16) else secAlign(sec.flags), .bss = startsWith(sec.name, ".bss") or (sec.flags & 0x40) == 0, .common = false };
                    const g = try obj.data_index.getOrPut(gpa, secnum);
                    if (!g.found_existing) g.value_ptr.* = .empty;
                    try g.value_ptr.append(gpa, .{ .off = s.value, .uid = u.uid });
                    if (out.contains(u.uid)) try dups.put(gpa, u.uid, {}) else try out.put(gpa, u.uid, u);
                }
                if (obj.data_index.getPtr(secnum)) |l| std.mem.sort(IdxEnt, l.items, {}, struct {
                    fn lt(_: void, a: IdxEnt, b: IdxEnt) bool {
                        if (a.off != b.off) return a.off < b.off;
                        return std.mem.order(u8, a.uid, b.uid) == .lt;
                    }
                }.lt);
            }
        }
    }
    return .{ .ours = out, .dups = dups.count(), .objs = objs };
}

const Identity = struct { name: Str, scope: Str, owner: Str };

fn scopeOf(path: Str) !Str {
    var parts: std.ArrayList(Str) = .empty;
    var it = std.mem.splitScalar(u8, path, '@');
    while (it.next()) |p| if (p.len > 0) try parts.append(gpa, p);
    std.mem.reverse(Str, parts.items);
    return join(parts.items, "::");
}

/// `[^@?]+@`-groups (or `[^@?$]+@` when no_dollar) from p: the end of the run.
fn groups(s: Str, p0: usize, no_dollar: bool) usize {
    var p = p0;
    while (true) {
        var q = p;
        while (q < s.len and s[q] != '@' and s[q] != '?' and !(no_dollar and s[q] == '$')) q += 1;
        if (q == p or q >= s.len or s[q] != '@') return p;
        p = q + 1;
    }
}

/// MANGLED_STATIC_LOCAL: `^\?([^@?]+)@\?(?:\d|[A-P]+@)\?\?([^@]+)@((?:[^@?]+@)*)@`
fn matchStaticLocal(s: Str) ?struct { name: Str, owner: Str, path: Str } {
    if (s.len == 0 or s[0] != '?') return null;
    var p: usize = 1;
    while (p < s.len and s[p] != '@' and s[p] != '?') p += 1;
    if (p == 1 or p >= s.len or s[p] != '@') return null;
    const name = s[1..p];
    p += 1;
    if (p >= s.len or s[p] != '?') return null;
    p += 1;
    if (p < s.len and std.ascii.isDigit(s[p]) and p + 2 < s.len and s[p + 1] == '?' and s[p + 2] == '?') {
        p += 1;
    } else {
        const q0 = p;
        while (p < s.len and s[p] >= 'A' and s[p] <= 'P') p += 1;
        if (p == q0 or p >= s.len or s[p] != '@') return null;
        p += 1;
    }
    if (p + 1 >= s.len or s[p] != '?' or s[p + 1] != '?') return null;
    p += 2;
    const o0 = p;
    while (p < s.len and s[p] != '@') p += 1;
    if (p == o0 or p >= s.len) return null;
    const owner = s[o0..p];
    p += 1;
    const g0 = p;
    p = groups(s, p, false);
    if (p >= s.len or s[p] != '@') return null;
    return .{ .name = name, .owner = owner, .path = s[g0..p] };
}

/// MANGLED_GLOBAL: `^\?([^@?]+)@((?:[^@?$]+@)*)@[0-9]`
fn matchGlobal(s: Str) ?struct { name: Str, path: Str } {
    if (s.len == 0 or s[0] != '?') return null;
    var p: usize = 1;
    while (p < s.len and s[p] != '@' and s[p] != '?') p += 1;
    if (p == 1 or p >= s.len or s[p] != '@') return null;
    const name = s[1..p];
    p += 1;
    const g0 = p;
    p = groups(s, p, true);
    if (p + 1 >= s.len or s[p] != '@' or !std.ascii.isDigit(s[p + 1])) return null;
    return .{ .name = name, .path = s[g0..p] };
}

/// (name, scope path, owner function) of a data symbol, from its mangled name.
fn ourIdentity(sym: Str) !?Identity {
    if (!startsWith(sym, "?")) {
        if (startsWith(sym, "__real@") or startsWith(sym, "__xmm@") or startsWith(sym, "__imp_")) return null;
        return .{ .name = if (startsWith(sym, "_")) sym[1..] else sym, .scope = "", .owner = "" };
    }
    if (startsWith(sym, "??")) return null;
    if (matchStaticLocal(sym)) |m| return .{ .name = m.name, .scope = try scopeOf(m.path), .owner = m.owner };
    if (matchGlobal(sym)) |m| return .{ .name = m.name, .scope = try scopeOf(m.path), .owner = "" };
    return null;
}

fn isLiteral(sym: Str) bool {
    return startsWithAny(sym, &.{ "__real@", "__xmm@", "??_C@", "??_7", "??_R" });
}

fn isLowerHex(ch: u8) bool {
    return std.ascii.isDigit(ch) or (ch >= 'a' and ch <= 'f');
}

/// re.search(r'_(00[67][0-9a-f]{5}|[67][0-9a-f]{5}|00[89][0-9a-f]{5}|[89][0-9a-f]{5})(?:$|_)')
fn embeddedAddress(name: Str) ?u32 {
    var i: usize = 0;
    while (i < name.len) : (i += 1) {
        if (name[i] != '_') continue;
        const alts = [_]struct { pre: Str, first: Str }{ .{ .pre = "00", .first = "67" }, .{ .pre = "", .first = "67" }, .{ .pre = "00", .first = "89" }, .{ .pre = "", .first = "89" } };
        for (alts) |alt| {
            const p = i + 1;
            const len = alt.pre.len + 6;
            if (p + len > name.len) continue;
            const g = name[p .. p + len];
            if (!startsWith(g, alt.pre)) continue;
            if (std.mem.indexOfScalar(u8, alt.first, g[alt.pre.len]) == null) continue;
            var ok = true;
            for (g[alt.pre.len + 1 ..]) |ch| if (!isLowerHex(ch)) {
                ok = false;
            };
            if (!ok) continue;
            if (p + len != name.len and name[p + len] != '_') continue;
            return std.fmt.parseInt(u32, g, 16) catch null;
        }
    }
    return null;
}

const DM = struct { addr: u32, flag: bool };

fn loadDatamap(path: Str) !std.StringHashMapUnmanaged(DM) {
    var out: std.StringHashMapUnmanaged(DM) = .empty;
    const data = (try util.readFileOpt(path)) orelse return out;
    var it = util.Lines{ .data = data };
    while (it.next()) |line| {
        if (startsWith(line, "symbol\t") or util.strip(line).len == 0) continue;
        const col = try util.split(line, '\t');
        try out.put(gpa, col[0], .{ .addr = try util.hex(col[1]), .flag = col.len > 3 and col[3].len > 0 });
    }
    return out;
}

const Pair = struct { it: *Item, method: Str, note: Str };
const PairMap = std.StringArrayHashMapUnmanaged(Pair);

/// Our uid -> (Item, method, note): name first, then the datamap's address, then the
/// address a generated name carries.
fn pair(ours: *const OursMap, items: []const *Item, datamap: *const std.StringHashMapUnmanaged(DM)) !PairMap {
    var by_name: std.StringHashMapUnmanaged(std.ArrayList(*Item)) = .empty;
    var by_addr: std.AutoHashMapUnmanaged(u32, *Item) = .empty;
    const add = struct {
        fn f(m: *std.StringHashMapUnmanaged(std.ArrayList(*Item)), k: Str, it: *Item) !void {
            const g = try m.getOrPut(gpa, k);
            if (!g.found_existing) g.value_ptr.* = .empty;
            try g.value_ptr.append(gpa, it);
        }
    }.f;
    for (items) |it| {
        const g = try by_addr.getOrPut(gpa, it.addr);
        if (!g.found_existing) g.value_ptr.* = it;
        try add(&by_name, it.name, it);
        var ait = std.mem.splitScalar(u8, it.aliases, ',');
        while (ait.next()) |a| if (a.len > 0) try add(&by_name, a, it);
    }
    var pairs: PairMap = .empty;
    for (ours.keys(), ours.values()) |uid, u| {
        const sym = u.symbol;
        const ident = try ourIdentity(sym);
        const dm = datamap.get(sym);
        var hit: ?*Item = null;
        var method: Str = "";
        if (ident) |id| {
            var cands: []*Item = if (by_name.get(id.name)) |l| l.items else &.{};
            if (cands.len > 1) {
                var narrowed: std.ArrayList(*Item) = .empty;
                if (embeddedAddress(id.name)) |ea| if (ea != 0) {
                    for (cands) |cd| if (cd.addr == ea) try narrowed.append(gpa, cd);
                };
                if (narrowed.items.len == 0 and id.owner.len > 0) for (cands) |cd| if (eql(cd.owner, id.owner)) try narrowed.append(gpa, cd);
                if (narrowed.items.len == 0) for (cands) |cd| if (eql(cd.ns, id.scope)) try narrowed.append(gpa, cd);
                if (narrowed.items.len > 1 and dm != null) {
                    var n2: std.ArrayList(*Item) = .empty;
                    for (narrowed.items) |cd| if (cd.addr == dm.?.addr) try n2.append(gpa, cd);
                    if (n2.items.len > 0) narrowed = n2;
                }
                cands = narrowed.items;
            }
            if (cands.len == 1) {
                hit = cands[0];
                method = "name";
            } else if (cands.len > 1) method = "ambiguous";
        }
        var where: ?u32 = null;
        var how: Str = "";
        if (hit == null and dm != null and !dm.?.flag) {
            where = dm.?.addr;
            how = "datamap";
        } else if (hit == null and ident != null and (embeddedAddress(ident.?.name) orelse 0) != 0) {
            where = embeddedAddress(ident.?.name).?;
            how = "name-address";
        }
        if (where) |w| {
            if (by_addr.get(w)) |it| {
                hit = it;
                method = how;
            } else if (DATA_LO <= w and w < DATA_HI) {
                // Ghidra has no item there (a literal, or a label it renamed): judge it over our extent.
                const it = try gpa.create(Item);
                it.* = .{ .addr = w, .size = u.extent, .extent = u.extent, .name = sym, .aliases = "", .scope = "", .ns = "", .owner = "", .typ = "", .kind = if (isLiteral(sym)) "literal" else "noitem", .refs = &.{} };
                hit = it;
                method = try fmt("{s}-noitem", .{how});
            }
        }
        const h = hit orelse continue;
        var note: Str = "";
        if (dm != null and dm.?.addr != h.addr and eql(method, "name")) note = try fmt("datamap implies {x:0>8}", .{dm.?.addr});
        try pairs.put(gpa, uid, .{ .it = h, .method = method, .note = note });
    }
    return pairs;
}

// -- the comparison ------------------------------------------------------------------

fn normImport(n0: Str) !Str {
    var n = n0;
    if (startsWith(n, "_") or startsWith(n, "@")) n = n[1..];
    // re.sub(r'@\d+$', '', n)
    if (std.mem.lastIndexOfScalar(u8, n, '@')) |at| {
        const tail = n[at + 1 ..];
        var ok = tail.len > 0;
        for (tail) |ch| if (!std.ascii.isDigit(ch)) {
            ok = false;
        };
        if (ok) n = n[0..at];
    }
    return std.ascii.allocLowerString(gpa, n);
}

/// peimage.imports, in its order: (IAT slot VA, 'dll!name').
fn importsOrdered(img: *const pe_mod.Image) ![]struct { va: u32, name: Str } {
    const E = struct { va: u32, name: Str };
    var out: std.ArrayList(E) = .empty;
    const rva = img.dirs[1][0];
    if (rva == 0) return @ptrCast(out.items);
    var p = img.base + rva;
    while (true) : (p += 20) {
        var d: [20]u8 = undefined;
        if (!img.read(p, &d)) return error.OutsideImage;
        const olt = std.mem.readInt(u32, d[0..4], .little);
        const name_rva = std.mem.readInt(u32, d[12..16], .little);
        const iat = std.mem.readInt(u32, d[16..20], .little);
        if (name_rva == 0) break;
        const dll = (try img.cstring(gpa, img.base + name_rva, 4096)) orelse return error.OutsideImage;
        const lookup = if (olt != 0) olt else iat;
        var i: u32 = 0;
        while (true) : (i += 1) {
            const ent = img.u32At(img.base + lookup + 4 * i) orelse return error.OutsideImage;
            if (ent == 0) break;
            const nm = if (ent & 0x80000000 != 0)
                try fmt("#{d}", .{ent & 0xffff})
            else
                (try img.cstring(gpa, img.base + ent + 2, 4096)) orelse return error.OutsideImage;
            try out.append(gpa, .{ .va = img.base + iat + 4 * i, .name = try fmt("{s}!{s}", .{ dll, nm }) });
        }
    }
    return @ptrCast(out.items);
}

const Target = struct { kind: Str, va: ?i64, sym: Str };

/// Where a relocation of ours points, in the original's address space.
const Resolver = struct {
    img: *const pe_mod.Image,
    data: std.StringHashMapUnmanaged(u32) = .empty,
    funcs: *const std.StringHashMapUnmanaged(u32),
    imports: std.StringHashMapUnmanaged(u32) = .empty,

    fn init(img: *const pe_mod.Image, pairs: *const PairMap, funcs: *const std.StringHashMapUnmanaged(u32)) !Resolver {
        var r = Resolver{ .img = img, .funcs = funcs };
        for (pairs.keys(), pairs.values()) |s, p| try r.data.put(gpa, s, p.it.addr);
        for (try importsOrdered(img)) |e| {
            const bang = std.mem.indexOfScalar(u8, e.name, '!').?;
            try r.imports.put(gpa, try normImport(e.name[bang + 1 ..]), e.va);
        }
        return r;
    }

    fn target(r: *const Resolver, obj: *const Obj, symidx: u32, addend: u32) !Target {
        const s = try obj.o.symbol(symidx);
        const name = s.name;
        if (s.storage == coff.C_STATIC and s.section > 0) {
            if (eql(name, obj.o.sections[@intCast(s.section)].name)) {
                // Section-relative: the data symbol of ours the addend lands in.
                const idx: []const IdxEnt = if (obj.data_index.get(s.section)) |l| l.items else &.{};
                var k: usize = 0;
                while (k < idx.len and idx[k].off <= addend) k += 1;
                if (k == 0) return .{ .kind = "section", .va = null, .sym = name };
                const e = idx[k - 1];
                if (r.data.get(e.uid)) |a| return .{ .kind = "data", .va = @as(i64, a) + addend - e.off, .sym = e.uid };
                return .{ .kind = "section", .va = null, .sym = e.uid };
            }
            const uid = try fmt("{s}|{s}", .{ obj.tu, name });
            if (r.data.get(uid)) |a| return .{ .kind = "data", .va = @as(i64, a) + addend, .sym = name };
        }
        if (r.data.get(name)) |a| return .{ .kind = "data", .va = @as(i64, a) + addend, .sym = name };
        if (r.funcs.get(name)) |f| return .{ .kind = "func", .va = @as(i64, f) + addend, .sym = name };
        if (startsWith(name, "__imp_")) {
            const va = r.imports.get(try normImport(name[6..]));
            return .{ .kind = "import", .va = if (va) |v| @as(i64, v) else null, .sym = name };
        }
        return .{ .kind = "unknown", .va = null, .sym = name };
    }

    /// A jump thunk's destination, else va itself.
    fn throughThunk(r: *const Resolver, va: i64) i64 {
        if (va < 0 or va > 0xffffffff) return va;
        const v: u32 = @intCast(va);
        const s = r.img.sectionAt(v) orelse return va;
        if (!eql(s.name, ".text")) return va;
        var b: [5]u8 = undefined;
        if (!r.img.read(v, &b)) return va;
        if (b[0] == 0xe9) {
            const rel = std.mem.readInt(i32, b[1..5], .little);
            return (va + 5 + rel) & 0xffffffff;
        }
        return va;
    }
};

/// Our bytes at symbol+addend (a string or float literal's content).
fn literalBytes(obj: *const Obj, symidx: u32, addend: u32, n: usize) !?Str {
    const s = try obj.o.symbol(symidx);
    if (s.section <= 0) return null;
    const sec = &obj.o.sections[@intCast(s.section)];
    const o: u64 = @as(u64, s.value) + addend;
    if (o >= sec.data.len) return "";
    return sec.data[@intCast(o)..@intCast(@min(o + n, sec.data.len))];
}

var ptr_stats: std.StringArrayHashMapUnmanaged(i64) = .empty; // "kind\x00verdict" -> fields

const LField = struct { where: Str, have: ?Str };
/// original string address -> [(where, our string)]
var literal_fields: std.AutoArrayHashMapUnmanaged(u32, std.ArrayList(LField)) = .empty;

const Verdict = struct { kind: Str, verdict: Str, sym: Str };

fn pointerVerdict(img: *const pe_mod.Image, res: *const Resolver, obj: *const Obj, r: coff.Reloc, ov: u32, uv: u32) !Verdict {
    const t = try res.target(obj, r.symbol, uv);
    var kind = t.kind;
    const sym = t.sym;
    if (startsWith(sym, "??_C@")) {
        kind = "string";
    } else if (startsWith(sym, "__real@")) kind = "float";
    const va = t.va orelse {
        // A literal of ours with no pair: compare its content.
        if (startsWith(sym, "??_C@_0")) {
            if (try img.cstring(gpa, ov, 4096)) |want| {
                if (try literalBytes(obj, r.symbol, uv, want.len + 1)) |have| {
                    const w0 = try std.mem.concat(gpa, u8, &.{ want, "\x00" });
                    const h = have[0..@min(have.len, want.len + 1)];
                    return .{ .kind = kind, .verdict = if (eql(h, w0)) "agree" else "string", .sym = sym };
                }
            }
        }
        if (startsWith(sym, "__real@")) {
            if (try literalBytes(obj, r.symbol, uv, 8)) |have| if (have.len > 0) {
                const buf = try gpa.alloc(u8, have.len);
                if (!img.read(ov, buf)) return error.OutsideImage;
                return .{ .kind = kind, .verdict = if (eql(buf, have)) "agree" else "bad", .sym = sym };
            };
        }
        return .{ .kind = kind, .verdict = "unverified", .sym = sym };
    };
    if (va == ov) return .{ .kind = kind, .verdict = "agree", .sym = sym };
    if (res.throughThunk(va) == res.throughThunk(ov)) return .{ .kind = kind, .verdict = "thunk", .sym = sym };
    return .{ .kind = kind, .verdict = "bad", .sym = sym };
}

/// The terminated narrow string a literal of ours holds at `off`, or null.
fn ourString(sec: ?*const coff.Section, off: i64) ?Str {
    const s = sec orelse return null;
    if (off < 0 or off >= s.data.len) return null;
    const o: usize = @intCast(off);
    const end = std.mem.indexOfScalarPos(u8, s.data, o, 0) orelse return null;
    return s.data[o..end];
}

fn relocsIn(sorted: []const u32, lo: u64, hi: u64) []const u32 {
    var i: usize = 0;
    var j: usize = sorted.len;
    while (i < j) {
        const m = (i + j) / 2;
        if (sorted[m] < lo) i = m + 1 else j = m;
    }
    var e = i;
    while (e < sorted.len and sorted[e] < hi) e += 1;
    return sorted[i..e];
}

const Judged = struct { status: Str, flags: std.ArrayList(Str), notes: std.ArrayList(Str), first: i64, diff: i64, matched: i64 };

fn has(l: []const Str, s: Str) bool {
    return eqlAny(s, l);
}

fn judge(img: *const pe_mod.Image, relocs: []const u32, it: *const Item, u: *const Ours, res: *const Resolver) !Judged {
    var flags: std.ArrayList(Str) = .empty;
    var notes: std.ArrayList(Str) = .empty;
    const n_orig: usize = @intCast(@max(0, it.extent));
    const ds = img.section(".data").?;
    const rawend: u64 = @as(u64, img.base) + ds.va + ds.rawsize;
    const orig_bss = it.addr >= rawend;
    const ours_ext: usize = @intCast(@max(0, u.extent));
    const n = @min(n_orig, ours_ext);
    const ob = try gpa.alloc(u8, n_orig);
    if (n_orig > 0 and !img.read(it.addr, ob)) return error.OutsideImage;
    const ub: Str = if (u.sec) |s| s.data[@min(u.off, s.data.len)..@min(@as(usize, u.off) + ours_ext, s.data.len)] else try gpa.alloc(u8, ours_ext);
    if (u.sec == null) @memset(@constCast(ub), 0);
    const is_float = isFloatType(it.typ);
    const is_string = isStringType(it.typ) or startsWith(it.name, "s_") or startsWith(it.name, "u_") or startsWith(u.symbol, "??_C@");
    const anyNz = struct {
        fn f(b: Str) bool {
            for (b) |x| if (x != 0) return true;
            return false;
        }
    }.f;
    // Size: ours may carry the padding the next symbol's alignment explains.
    if (ours_ext < n_orig) {
        try flags.append(gpa, "SIZE_SMALL");
        try notes.append(gpa, try fmt("ours {d} < ghidra {d}", .{ ours_ext, n_orig }));
    } else if (@as(i64, @intCast(ours_ext)) - (u.next_align - 1) > @as(i64, @intCast(n_orig)) and anyNz(ub[@min(n_orig, ub.len)..])) {
        try flags.append(gpa, "SIZE_LARGE");
        try notes.append(gpa, try fmt("ours {d} > ghidra {d}", .{ ours_ext, n_orig }));
    }
    const oa = lowbit(it.addr, 16);
    if (orig_bss) {
        if (anyNz(ub)) try flags.append(gpa, "BSS_INIT");
        // Only an 8-byte scalar has a natural alignment the original's address proves.
        const natural: i64 = if (isEightByteType(it.typ)) 8 else 1;
        if (u.alignment < @min(oa, natural)) {
            try flags.append(gpa, "ALIGN");
            try notes.append(gpa, try fmt("align ours {d}, original {d}", .{ u.alignment, oa }));
        }
        const status: Str = if (flags.items.len > 0) flags.items[0] else "MATCH";
        return .{ .status = status, .flags = flags, .notes = notes, .first = -1, .diff = 0, .matched = if (eql(status, "MATCH")) it.size else 0 };
    }
    // Relocation fields on both sides.
    var orel: std.AutoHashMapUnmanaged(u32, void) = .empty;
    for (relocsIn(relocs, it.addr, @as(u64, it.addr) + n_orig)) |va| try orel.put(gpa, va - it.addr, {});
    var urel: std.AutoHashMapUnmanaged(u32, coff.Reloc) = .empty;
    if (u.sec) |s| for (s.relocs) |r| {
        if (u.off <= r.offset and r.offset < @as(u64, u.off) + n and r.type == coff.DIR32) try urel.put(gpa, r.offset - u.off, r);
    };
    var all_p: std.ArrayList(u32) = .empty;
    {
        var k1 = orel.keyIterator();
        while (k1.next()) |p| try all_p.append(gpa, p.*);
        var k2 = urel.keyIterator();
        while (k2.next()) |p| if (!orel.contains(p.*)) try all_p.append(gpa, p.*);
    }
    std.mem.sort(u32, all_p.items, {}, std.sort.asc(u32));
    const masked = try gpa.alloc(bool, n);
    @memset(masked, false);
    var reloc_bad: i64 = 0;
    var raw: i64 = 0;
    var nulls: i64 = 0;
    var extra: i64 = 0;
    var unverified: i64 = 0;
    var strdiff: i64 = 0;
    var thunked: i64 = 0;
    var first_reloc: ?struct { p: u32, sym: Str } = null;
    for (all_p.items) |p| {
        if (@as(u64, p) + 4 > n) continue;
        @memset(masked[p .. p + 4], true);
        const ov = std.mem.readInt(u32, ob[p..][0..4], .little);
        const uv = std.mem.readInt(u32, ub[p..][0..4], .little);
        var kind: Str = undefined;
        var verdict: Str = undefined;
        var sym: Str = undefined;
        if (orel.contains(p) and urel.contains(p)) {
            const ur = urel.get(p).?;
            const v = try pointerVerdict(img, res, u.obj, ur, ov, uv);
            kind = v.kind;
            verdict = v.verdict;
            sym = v.sym;
            if (startsWith(sym, "??_C@_0")) {
                const t = try u.obj.o.symbol(ur.symbol);
                const have: ?Str = if (t.section > 0) ourString(&u.obj.o.sections[@intCast(t.section)], @as(i64, t.value) + uv) else null;
                const g = try literal_fields.getOrPut(gpa, ov);
                if (!g.found_existing) g.value_ptr.* = .empty;
                try g.value_ptr.append(gpa, .{ .where = try fmt("{s}+{d}", .{ it.name, p }), .have = have });
            }
        } else if (orel.contains(p)) {
            kind = "none";
            sym = "(no relocation)";
            verdict = if (uv == ov) "raw" else if (uv == 0) "null" else "bad";
        } else {
            kind = "none";
            verdict = "extra";
            sym = "(ours only)";
        }
        const g = try ptr_stats.getOrPut(gpa, try fmt("{s}\x00{s}", .{ kind, verdict }));
        if (!g.found_existing) g.value_ptr.* = 0;
        g.value_ptr.* += 1;
        if (eql(verdict, "bad")) {
            reloc_bad += 1;
        } else if (eql(verdict, "string")) {
            strdiff += 1;
        } else if (eql(verdict, "raw")) {
            raw += 1;
        } else if (eql(verdict, "null")) {
            nulls += 1;
        } else if (eql(verdict, "extra")) {
            extra += 1;
        } else if (eql(verdict, "unverified")) {
            unverified += 1;
        } else if (eql(verdict, "thunk")) thunked += 1;
        if (!eqlAny(verdict, &.{ "agree", "thunk", "unverified" })) {
            if (first_reloc == null) first_reloc = .{ .p = p, .sym = sym };
        }
    }
    var diffs: std.ArrayList(usize) = .empty;
    for (0..n) |i| if (!masked[i] and ob[i] != ub[i]) try diffs.append(gpa, i);
    var tail: usize = 0;
    for (n..n_orig) |i| if (ob[i] != 0) {
        tail += 1;
    };
    const all_zero = !anyNz(ub[0..n]) and urel.count() == 0;
    if (all_zero and (anyNz(ob) or orel.count() > 0)) try flags.insert(gpa, 0, "ZERO");
    if (raw > 0) try notes.append(gpa, try fmt("{d} raw original addresses", .{raw}));
    if (nulls > 0) try notes.append(gpa, try fmt("{d} null where the original points", .{nulls}));
    if (extra > 0) try notes.append(gpa, try fmt("{d} pointers only ours has", .{extra}));
    if (unverified > 0) try notes.append(gpa, try fmt("{d} pointers unverified", .{unverified}));
    if (thunked > 0) try notes.append(gpa, try fmt("{d} pointers name the function where the original names its jump thunk", .{thunked}));
    if ((reloc_bad > 0 or raw > 0 or nulls > 0 or extra > 0) and !has(flags.items, "ZERO")) {
        try flags.append(gpa, "RELOC");
        if (first_reloc) |fr| try notes.append(gpa, try fmt("first at +{d} {s}", .{ fr.p, fr.sym }));
    }
    if (strdiff > 0 or (is_string and diffs.items.len > 0)) try flags.append(gpa, "STRING");
    if (diffs.items.len > 0 and !is_string) {
        try flags.append(gpa, "VALUE");
        if (is_float) try notes.append(gpa, "FLOAT");
    }
    if (diffs.items.len > 0) try notes.append(gpa, try fmt("{d} bytes differ", .{diffs.items.len}));
    const first: i64 = if (diffs.items.len > 0) @intCast(diffs.items[0]) else if (tail > 0) @intCast(n) else -1;
    var status: Str = "MATCH";
    for ([_]Str{ "ZERO", "SPLIT", "SIZE_SMALL", "RELOC", "STRING", "VALUE", "SIZE_LARGE", "BSS_INIT", "ALIGN" }) |k| {
        if (has(flags.items, k)) {
            status = k;
            break;
        }
    }
    const nd: i64 = @intCast(diffs.items.len);
    var diff_bytes: i64 = nd + @as(i64, @intCast(tail)) + 4 * (reloc_bad + raw + nulls + extra + strdiff);
    if (eql(status, "ZERO")) {
        var nz: i64 = 0;
        for (ob) |b| if (b != 0) {
            nz += 1;
        };
        diff_bytes = nz + 4 * @as(i64, @intCast(orel.count()));
    }
    const matched: i64 = if (eql(status, "MATCH")) @intCast(n_orig) else @max(0, @as(i64, @intCast(n)) - nd - 4 * (reloc_bad + raw + nulls + extra + strdiff));
    return .{ .status = status, .flags = flags, .notes = notes, .first = first, .diff = diff_bytes, .matched = matched };
}

fn moduleOf(tu0: Str) Str {
    var tu = tu0;
    if (startsWith(tu, "3rdparty/crashy/")) return "Fog"; // its namespace, Fog::_3rdparty::crashy
    if (startsWith(tu, "Source/") or startsWith(tu, "3rdparty/")) tu = tu[std.mem.indexOfScalar(u8, tu, '/').? + 1 ..];
    const top = tu[0 .. std.mem.indexOfScalar(u8, tu, '/') orelse tu.len];
    if (eql(top, "globals") and std.mem.indexOfScalar(u8, tu, '/') != null) {
        const rest = tu[std.mem.indexOfScalar(u8, tu, '/').? + 1 ..];
        return if (eql(rest, "globals")) "Global" else rest;
    }
    return top;
}

fn moduleOfItem(ns: Str, refs: []const Str) !Str {
    if (ns.len > 0) return ns[0 .. std.mem.indexOf(u8, ns, "::") orelse ns.len];
    var tops: Counter = .{};
    for (refs) |r| if (contains(r, "::")) try tops.add(r[0..std.mem.indexOf(u8, r, "::").?], 1);
    if (tops.map.count() == 0) return "?";
    return (try tops.mostCommon())[0].k;
}

fn sectionOf(img: *const pe_mod.Image, va: u32) Str {
    const s = img.sectionAt(va) orelse return "?";
    if (eql(s.name, ".data") and va - img.base - s.va >= s.rawsize) return ".bss";
    return s.name;
}

const NameSet = std.StringArrayHashMapUnmanaged(void);

/// Our function symbol -> original start (funcmap, then libmap), as audit.sym_map; and
/// game function name -> the TUs of ours that define it.
fn loadFuncsyms(evidence: Str) !struct { syms: std.StringHashMapUnmanaged(u32), names: std.StringArrayHashMapUnmanaged(NameSet) } {
    var seen: std.StringArrayHashMapUnmanaged(std.AutoArrayHashMapUnmanaged(u32, void)) = .empty;
    var names: std.StringArrayHashMapUnmanaged(NameSet) = .empty;
    for ([_]Str{ "funcmap.tsv", "libmap.tsv" }) |f| {
        const data = (try util.readFileOpt(try util.join(evidence, f))) orelse continue;
        var it = util.Lines{ .data = data };
        while (it.next()) |line| {
            if (startsWith(line, "start\t")) continue;
            const col = try util.split(line, '\t');
            const g = try seen.getOrPut(gpa, col[4]);
            if (!g.found_existing) g.value_ptr.* = .empty;
            try g.value_ptr.put(gpa, try util.hex(col[0]), {});
            if (eql(f, "funcmap.tsv")) {
                const h = try names.getOrPut(gpa, col[2]);
                if (!h.found_existing) h.value_ptr.* = .empty;
                try h.value_ptr.put(gpa, col[3], {});
            }
        }
    }
    var out: std.StringHashMapUnmanaged(u32) = .empty;
    for (seen.keys(), seen.values()) |k, v| if (v.count() == 1) try out.put(gpa, k, v.keys()[0]);
    return .{ .syms = out, .names = names };
}

/// Whether our object initialises the symbol at run time.
fn dynamicInit(u: *const Ours) !bool {
    if (matchStaticLocal(u.symbol) != null) {
        const after = u.symbol[std.mem.indexOfScalar(u8, u.symbol, '@').? + 1 ..];
        const tail = if (std.mem.lastIndexOf(u8, after, "@4")) |k| after[0..k] else after;
        for (u.obj.o.symbols) |sp| if (sp) |s| if (startsWith(s.name, "?$S") and contains(s.name, tail)) return true;
        return false;
    }
    const ident = (try ourIdentity(u.symbol)) orelse return false;
    for (u.obj.o.symbols) |sp| if (sp) |s| if (startsWith(s.name, "??__E") and contains(s.name, ident.name)) return true;
    return false;
}

var used_set: ?std.StringHashMapUnmanaged(void) = null;

/// Whether code or data of ours relocates against this data symbol.
fn usedByCode(objs: *const ObjMap, u: *const Ours) !bool {
    if (used_set == null) {
        var set: std.StringHashMapUnmanaged(void) = .empty;
        for (objs.keys(), objs.values()) |tu, o| {
            for (o.o.sections[1..]) |sec| {
                if (startsWith(sec.name, ".debug")) continue;
                for (sec.relocs) |r| {
                    const s = try o.o.symbol(r.symbol);
                    try set.put(gpa, if (s.storage == coff.C_STATIC) try fmt("{s}|{s}", .{ tu, s.name }) else s.name, {});
                }
            }
        }
        used_set = set;
    }
    return used_set.?.contains(u.uid);
}

/// `__real@3ff0000000000000` -> its little-endian bytes (bytes.fromhex(h)[::-1]).
fn realLiteralBytes(sym: Str) !?Str {
    const h = sym["__real@".len..];
    var out: std.ArrayList(u8) = .empty;
    var i: usize = 0;
    while (i < h.len) {
        if (std.ascii.isWhitespace(h[i])) {
            i += 1;
            continue;
        }
        if (i + 1 >= h.len) return null;
        const hi = std.fmt.charToDigit(h[i], 16) catch return null;
        const lo = std.fmt.charToDigit(h[i + 1], 16) catch return null;
        try out.append(gpa, hi * 16 + lo);
        i += 2;
    }
    std.mem.reverse(u8, out.items);
    return out.items;
}

pub const Row = struct {
    addr: u32,
    size: i64,
    ours_size: i64,
    section: Str,
    module: Str,
    name: Str,
    symbol: Str,
    tu: Str,
    kind: Str,
    method: Str,
    typ: Str,
    status: Str,
    flags: Str,
    first: i64,
    diff: i64,
    matched: i64,
    notes: Str,
};

fn unpackFloat(b: Str) f64 {
    if (b.len == 8) return @bitCast(std.mem.readInt(u64, b[0..8], .little));
    return @as(f32, @bitCast(std.mem.readInt(u32, b[0..4], .little)));
}

fn floatConstants(img: *const pe_mod.Image, items: []const *Item, paired_addr: *const std.AutoArrayHashMapUnmanaged(u32, i64), objs: *const ObjMap, game_fns: *const std.StringArrayHashMapUnmanaged(NameSet), dead: *const std.AutoHashMapUnmanaged(u32, Str)) ![]Row {
    var lits: std.StringHashMapUnmanaged(BSet) = .empty;
    for (objs.keys(), objs.values()) |tu, o| {
        for (o.o.symbols) |sp| if (sp) |s| if (startsWith(s.name, "__real@")) {
            if (try realLiteralBytes(s.name)) |b| if (b.len > 0) {
                const g = try lits.getOrPut(gpa, tu);
                if (!g.found_existing) g.value_ptr.* = .empty;
                try g.value_ptr.put(gpa, b, {});
            };
        };
    }
    var out: std.ArrayList(Row) = .empty;
    for (items) |it| {
        if (paired_addr.contains(it.addr) or !eql(it.kind, "data") or !eql(sectionOf(img, it.addr), ".rdata")) continue;
        if (!isFloatType(it.typ) or (it.extent != 4 and it.extent != 8)) continue;
        var tus: NameSet = .empty;
        for (it.refs) |r| if (game_fns.get(r)) |set| for (set.keys()) |t| try tus.put(gpa, t, {});
        var tl: std.ArrayList(Str) = .empty;
        for (tus.keys()) |t| if (objs.contains(t)) try tl.append(gpa, t);
        if (tl.items.len == 0) continue;
        const sorted = try sortedStrs(tl.items);
        const ext: usize = @intCast(it.extent);
        const want = try gpa.alloc(u8, ext);
        if (!img.read(it.addr, want)) return error.OutsideImage;
        var have: BSet = .empty;
        for (tl.items) |t| if (lits.get(t)) |s| for (s.keys()) |b| try have.put(gpa, b, {});
        const value = unpackFloat(want);
        var notes: std.ArrayList(Str) = .empty;
        try notes.append(gpa, try fmt("{s} {s} read by {s}", .{ if (ext == 8) "double" else "float", try pyFloatRepr(value), try join(sorted, ",") }));
        var status: Str = undefined;
        var flags: Str = undefined;
        var matched: i64 = undefined;
        var diff: i64 = undefined;
        if (have.contains(want) or value == 0.0 or value == 1.0) {
            status = "MATCH";
            flags = "";
            matched = it.extent;
            diff = 0;
        } else {
            status = "VALUE";
            flags = "VALUE";
            matched = 0;
            diff = it.extent;
            var alt: [8]u8 = undefined;
            const alt_s: Str = if (ext == 8) blk: {
                std.mem.writeInt(u32, alt[0..4], @bitCast(@as(f32, @floatCast(value))), .little);
                break :blk alt[0..4];
            } else blk: {
                std.mem.writeInt(u64, alt[0..8], @bitCast(value), .little);
                break :blk alt[0..8];
            };
            var near: ?Str = null;
            for (have.keys()) |b| if (b.len == ext and @abs(unpackFloat(b) - value) <= @abs(value) * 1e-6) {
                if (near == null) near = b;
            };
            if (have.contains(alt_s)) {
                try notes.append(gpa, try fmt("ours has it as a {s}", .{if (ext == 8) "float" else "double"}));
            } else if (near) |nb| {
                try notes.append(gpa, try fmt("ours has a rounded {s}", .{try pyFloatRepr(unpackFloat(nb))}));
            } else try notes.append(gpa, "no literal of that value in those objects");
            try notes.append(gpa, "FLOAT");
        }
        if (dead.get(it.addr)) |d| try notes.append(gpa, try fmt("ours also defines {s}, which nothing reads", .{d}));
        try out.append(gpa, .{ .addr = it.addr, .size = it.extent, .ours_size = it.extent, .section = ".rdata", .module = try moduleOfItem(it.ns, it.refs), .name = it.name, .symbol = "", .tu = try join(sorted, ","), .kind = "const", .method = "tu-literal", .typ = it.typ, .status = status, .flags = flags, .first = if (eql(status, "MATCH")) -1 else 0, .diff = diff, .matched = matched, .notes = try join(notes.items, "; ") });
    }
    return out.items;
}

// -- string literals -------------------------------------------------------------------

fn pooledExtent(length: i64) i64 {
    return (length + 1 + 3) & ~@as(i64, 3);
}

const Shape = struct { text: ?Str, why: ?Str };

/// The generator's byte rule for a string it may spell as a literal.
pub fn stringLiteralShape(raw: Str) !Shape {
    const nul = std.mem.indexOfScalar(u8, raw, 0) orelse return .{ .text = null, .why = "no terminator inside the extent" };
    if (nul == 0) return .{ .text = null, .why = "empty" };
    for (raw[nul..]) |b| if (b != 0) return .{ .text = null, .why = "bytes after the terminator (two strings, or a table)" };
    if (@as(i64, @intCast(raw.len)) > pooledExtent(@intCast(nul))) return .{ .text = null, .why = try fmt("object of {d} bytes is larger than the literal padded to 4", .{raw.len}) };
    return .{ .text = raw[0..nul], .why = null };
}

/// Ghidra's label for an untyped string, as compiler-data.ts derives it (per character).
pub fn stringLabel(address: u32, text: Str) !Str {
    var out: std.ArrayList(u8) = .empty;
    var i: usize = 0;
    while (i < text.len) {
        if (out.items.len >= 33) break;
        const ch = text[i];
        const n: usize = if (ch < 0x80) 1 else if (ch >= 0xf0) 4 else if (ch >= 0xe0) 3 else if (ch >= 0xc0) 2 else 1;
        try out.append(gpa, if (n == 1 and ch < 0x80 and (std.ascii.isAlphanumeric(ch) or ch == '_')) ch else '_');
        i += n;
    }
    return fmt("s_{s}_{x:0>8}", .{ out.items, address });
}

fn stringsMain(snap: Str, out_path: Str) !void {
    const manifest = try parseLine(try util.readFile(try util.join(snap, "manifest.json")));
    const prov: JV = jget(manifest, "provenance") orelse .{ .object = .empty };
    const R = struct { a: u32, len: i64, enc: Str, label: Str };
    var out: std.ArrayList(R) = .empty;
    var seen: std.AutoHashMapUnmanaged(u32, void) = .empty;
    var it = util.Lines{ .data = try util.readFile(try util.join(snap, "strings.ndjson")) };
    while (it.next()) |line| {
        if (util.strip(line).len == 0) continue;
        const s = try parseLine(line);
        const a = try util.hex((jget(s, "address") orelse return error.NoAddress).string);
        if (!(DATA_LO <= a and a < DATA_HI) or seen.contains(a)) continue;
        try seen.put(gpa, a, {});
        try out.append(gpa, .{ .a = a, .len = try intOr(s, "length", 0), .enc = try orEmpty(s, "encoding"), .label = try stringLabel(a, try orEmpty(s, "value")) });
    }
    std.mem.sort(R, out.items, {}, struct {
        fn lt(_: void, x: R, y: R) bool {
            return x.a < y.a;
        }
    }.lt);
    var ro: std.ArrayList(Str) = .empty;
    if (jget(prov, "programInfo")) |pi| if (truthy(jget(pi, "readOnlyDataSections"))) for (jget(pi, "readOnlyDataSections").?.array.items) |span| {
        try ro.append(gpa, try fmt("{x:0>8}-{x:0>8}", .{ @as(u64, @intCast(span.array.items[0].integer)), @as(u64, @intCast(span.array.items[1].integer)) }));
    };
    var o: std.ArrayList(u8) = .empty;
    try o.print(gpa, "# Ghidra {s} v{s} strings, snapshot written {s}; read-only {s}\n", .{ try getStrOr(prov, "programPath", "?"), try getStrOr(prov, "programVersion", "?"), try getStrOr(prov, "writtenAt", "?"), try join(ro.items, ",") });
    try o.appendSlice(gpa, DS_HEADER ++ "\n");
    for (out.items) |r| try o.print(gpa, "{x:0>8}\t{d}\t{s}\t{s}\n", .{ r.a, r.len, r.enc, r.label });
    try util.writeFile(out_path, o.items);
    util.say(try fmt("{d} strings from {s} v{s} -> {s}\n", .{ out.items.len, try pyStr(jget(prov, "programPath")), try pyStr(jget(prov, "programVersion")), out_path }));
}

pub const DString = struct { addr: u32, length: i64, encoding: Str, label: Str };
pub const Span = struct { lo: u32, hi: u32 };

fn loadDatastrings(path: ?Str) !struct { rows: []DString, version: Str, ro: []Span } {
    var rows: std.ArrayList(DString) = .empty;
    var version: Str = "?";
    var ro: std.ArrayList(Span) = .empty;
    const p = path orelse return .{ .rows = rows.items, .version = version, .ro = ro.items };
    if (p.len == 0) return .{ .rows = rows.items, .version = version, .ro = ro.items };
    const data = (try util.readFileOpt(p)) orelse return .{ .rows = rows.items, .version = version, .ro = ro.items };
    var it = util.Lines{ .data = data };
    while (it.next()) |line| {
        if (startsWith(line, "# Ghidra")) {
            version = versionOf(line);
            if (std.mem.indexOf(u8, line, "read-only ")) |k| {
                var e = k + "read-only ".len;
                const s0 = e;
                while (e < line.len and !std.ascii.isWhitespace(line[e])) e += 1;
                if (e > s0) {
                    var sit = std.mem.splitScalar(u8, line[s0..e], ',');
                    while (sit.next()) |span| if (std.mem.indexOfScalar(u8, span, '-')) |d| {
                        try ro.append(gpa, .{ .lo = try util.hex(span[0..d]), .hi = try util.hex(span[d + 1 ..]) });
                    };
                }
            }
            continue;
        }
        if (startsWith(line, "address\t") or util.strip(line).len == 0) continue;
        const col = try util.split(line, '\t');
        try rows.append(gpa, .{ .addr = try util.hex(col[0]), .length = try std.fmt.parseInt(i64, col[1], 10), .encoding = col[2], .label = col[3] });
    }
    return .{ .rows = rows.items, .version = version, .ro = ro.items };
}

fn loadCompilerData(path: ?Str) !?std.AutoHashMapUnmanaged(u32, void) {
    const p = path orelse return null;
    if (p.len == 0) return null;
    var out: std.AutoHashMapUnmanaged(u32, void) = .empty;
    var it = util.Lines{ .data = try util.readFile(p) };
    while (it.next()) |line| {
        const col = try util.split(line, '\t');
        if (col.len > 2 and eql(col[2], "string-literal")) try out.put(gpa, try util.hex(col[0]), {});
    }
    return out;
}

pub const Lit = struct { addr: u32, text: Str, extent: i64, name: Str, typ: Str, source: Str, refs: []const Str, ns: Str };

fn spanLess(_: void, a: [2]i64, b: [2]i64) bool {
    return a[0] < b[0] or (a[0] == b[0] and a[1] < b[1]);
}

/// The strings the generator spells as literals, which no symbol of ours pairs with.
/// `reader.read(va, n)` reads the original.
pub fn literalCandidates(items: []const *Item, strings: []const DString, reader: anytype, covered: *const std.AutoHashMapUnmanaged(u32, void), read_only: []const Span, generator: ?*const std.AutoHashMapUnmanaged(u32, void)) ![]Lit {
    var startset: std.AutoArrayHashMapUnmanaged(u32, void) = .empty;
    for (items) |it| try startset.put(gpa, it.addr, {});
    for (strings) |s| try startset.put(gpa, s.addr, {});
    const starts = try gpa.dupe(u32, startset.keys());
    std.mem.sort(u32, starts, {}, std.sort.asc(u32));
    var spans: std.ArrayList([2]i64) = .empty;
    for (items) |it| if (!eql(it.kind, "interior")) try spans.append(gpa, .{ it.addr, @as(i64, it.addr) + @max(1, it.size) });
    for (strings) |s| try spans.append(gpa, .{ s.addr, @as(i64, s.addr) + s.length + 1 });
    std.mem.sort([2]i64, spans.items, {}, spanLess);
    const H = struct {
        starts: []const u32,
        spans: []const [2]i64,
        ro: []const Span,
        fn isRo(h: @This(), a: u32) bool {
            for (h.ro) |r| if (r.lo <= a and a < r.hi) return true;
            return false;
        }
        fn labelInside(h: @This(), lo: u32, hi: i64) bool {
            var k: usize = 0; // bisect_right
            while (k < h.starts.len and h.starts[k] <= lo) k += 1;
            return k < h.starts.len and h.starts[k] < hi;
        }
        fn insideAnother(h: @This(), a: u32) bool {
            var k: usize = 0; // bisect_left
            var hi = h.spans.len;
            while (k < hi) {
                const m = (k + hi) / 2;
                if (h.spans[m][0] < a) k = m + 1 else hi = m;
            }
            var j = if (k > 64) k - 64 else 0;
            while (j < k) : (j += 1) if (h.spans[j][1] > a) return true;
            return false;
        }
    };
    const h = H{ .starts = starts, .spans = spans.items, .ro = read_only };
    var out: std.ArrayList(Lit) = .empty;
    var typed: std.AutoHashMapUnmanaged(u32, void) = .empty;
    for (items) |it| {
        try typed.put(gpa, it.addr, {});
        if (!eql(it.kind, "data") or covered.contains(it.addr) or !isStringItemType(it.typ)) continue;
        if (generator) |g| if (!g.contains(it.addr)) continue;
        if (!h.isRo(it.addr)) continue;
        const sh = try stringLiteralShape(try reader.read(it.addr, @intCast(@max(0, it.extent))));
        if (sh.why != null) continue;
        const text = sh.text.?;
        if (generator == null and (h.labelInside(it.addr, @as(i64, it.addr) + @as(i64, @intCast(text.len)) + 1) or h.insideAnother(it.addr))) continue;
        try out.append(gpa, .{ .addr = it.addr, .text = text, .extent = it.extent, .name = it.name, .typ = it.typ, .source = "typed", .refs = it.refs, .ns = it.ns });
    }
    for (strings) |s| {
        if (typed.contains(s.addr) or covered.contains(s.addr) or !isUntypedStringEncoding(s.encoding) or s.length <= 0) continue;
        if (generator) |g| if (!g.contains(s.addr)) continue;
        if (!h.isRo(s.addr)) continue;
        const sh = try stringLiteralShape(try reader.read(s.addr, @intCast(s.length + 1)));
        if (sh.why != null or sh.text.?.len != s.length) continue;
        if (generator == null and (h.labelInside(s.addr, @as(i64, s.addr) + s.length + 1) or h.insideAnother(s.addr))) continue;
        try out.append(gpa, .{ .addr = s.addr, .text = sh.text.?, .extent = s.length + 1, .name = s.label, .typ = s.encoding, .source = "untyped", .refs = &.{}, .ns = "" });
    }
    std.mem.sort(Lit, out.items, {}, struct {
        fn lt(_: void, a: Lit, b: Lit) bool {
            return a.addr < b.addr;
        }
    }.lt);
    return out.items;
}

/// The `??_C@` narrow literals our objects hold: per function of ours, the strings it
/// references; and image-wide, every one.
const OurLiterals = struct {
    objs: *const ObjMap,
    fn_cache: std.StringHashMapUnmanaged(?*BSet) = .empty,
    everywhere: BSet = .empty,
    by_tu: std.StringHashMapUnmanaged(*BSet) = .empty,

    fn init(objs: *const ObjMap) !OurLiterals {
        var ol = OurLiterals{ .objs = objs };
        for (objs.keys(), objs.values()) |tu, o| {
            for (o.o.symbols) |sp| if (sp) |s| if (startsWith(s.name, "??_C@_0") and s.section > 0) {
                if (ourString(&o.o.sections[@intCast(s.section)], s.value)) |t| {
                    try ol.everywhere.put(gpa, t, {});
                    const g = try ol.by_tu.getOrPut(gpa, tu);
                    if (!g.found_existing) {
                        g.value_ptr.* = try gpa.create(BSet);
                        g.value_ptr.*.* = .empty;
                    }
                    try g.value_ptr.*.put(gpa, t, {});
                }
            };
        }
        return ol;
    }

    pub fn ofFunction(ol: *OurLiterals, tu: Str, symbol: Str) !?*const BSet {
        const key = try fmt("{s}\x00{s}", .{ tu, symbol });
        if (ol.fn_cache.get(key)) |v| return v;
        var got: ?*BSet = null;
        if (ol.objs.get(tu)) |obj| {
            const o = &obj.o;
            for (try o.functions(false)) |fr| {
                if (!eql(fr.sym.name, symbol)) continue;
                if (got == null) {
                    got = try gpa.create(BSet);
                    got.?.* = .empty;
                }
                for (fr.sec.relocs) |r| {
                    if (!(fr.start <= r.offset and r.offset < fr.end) or r.type != coff.DIR32) continue;
                    const t = o.symbol(r.symbol) catch continue;
                    if (!startsWith(t.name, "??_C@_0") or t.section <= 0) continue;
                    const add: i32 = if (@as(u64, r.offset) + 4 <= fr.sec.data.len) std.mem.readInt(i32, fr.sec.data[r.offset..][0..4], .little) else 0;
                    if (ourString(&o.sections[@intCast(t.section)], @as(i64, t.value) + add)) |v| try got.?.put(gpa, v, {});
                }
            }
        }
        try ol.fn_cache.put(gpa, key, got);
        return got;
    }

    pub fn byTu(ol: *OurLiterals, tu: Str) ?*const BSet {
        return ol.by_tu.get(tu);
    }
};

/// The literal of `haves` (less `theirs`) most like `want`, when one is like it at all.
fn nearest(want: Str, haves: *const BSet, theirs: *const BSet) !?Str {
    var best: ?Str = null;
    var score: f64 = 0.0;
    const a = try gpa.alloc(u32, want.len);
    for (want, 0..) |b, i| a[i] = b;
    for (haves.keys()) |hv| {
        if (theirs.contains(hv)) continue;
        const b = try gpa.alloc(u32, hv.len);
        for (hv, 0..) |x, i| b[i] = x;
        var sm = try difflib.SequenceMatcher.init(gpa, a, b);
        const r = try sm.ratio();
        if (r > score) {
            best = hv;
            score = r;
        }
    }
    return if (score >= 0.6) best else null;
}

pub const Site = struct { where: Str, haves: *const BSet, theirs: *const BSet };
pub const JL = struct { status: Str, method: Str, notes: []const Str };

fn one(s: Str) ![]const Str {
    const out = try gpa.alloc(Str, 1);
    out[0] = s;
    return out;
}

/// (status, method, notes) for one literal of the original holding `want`.
pub fn judgeLiteral(want: Str, sites: []const Site, everywhere: *const BSet, tu_haves: []const *const BSet) !JL {
    if (sites.len == 0) return .{ .status = "UNUSED", .method = "none", .notes = &.{} };
    var agree: usize = 0;
    const D = struct { w: Str, n: Str };
    var differ: std.ArrayList(D) = .empty;
    for (sites) |s| {
        if (s.haves.contains(want)) {
            agree += 1;
            continue;
        }
        if (try nearest(want, s.haves, s.theirs)) |n| try differ.append(gpa, .{ .w = s.where, .n = n });
    }
    if (agree > 0 and differ.items.len == 0) return .{ .status = "MATCH", .method = "use", .notes = try one(try fmt("{d} of {d} uses agree", .{ agree, sites.len })) };
    if (differ.items.len > 0 and agree == 0) {
        const d = differ.items[0];
        return .{ .status = "STRING", .method = "use", .notes = try one(try fmt("{s} names {s} where the original holds {s}", .{ d.w, try pyBytesRepr(d.n), try pyBytesRepr(want) })) };
    }
    if (agree > 0 and differ.items.len > 0) {
        const d = differ.items[0];
        return .{ .status = "STRING", .method = "use", .notes = try one(try fmt("{d} uses agree; {s} names {s}", .{ agree, d.w, try pyBytesRepr(d.n) })) };
    }
    for (tu_haves) |th| if (th.contains(want)) return .{ .status = "MATCH", .method = "tu", .notes = try one("no function of ours at a use names it; the object of a use holds it") };
    var ws: std.ArrayList(Str) = .empty;
    for (sites[0..@min(3, sites.len)]) |s| try ws.append(gpa, s.where);
    return .{ .status = "MISSING", .method = "use", .notes = try one(try fmt("{d} paired uses ({s}), none names it{s}", .{ sites.len, try join(ws.items, ", "), if (everywhere.contains(want)) "; the image holds it elsewhere" else "" })) };
}

pub const FmRow = struct { s: u32, e: u32, name: Str, tu: Str, sym: Str };

const empty_set: BSet = .empty;

/// Audit rows (kind `strlit`) for the literals, paired through the original's relocations
/// that point at each. `pe`: relocList, u32At, textRange, rdataRange, cstr, sectionOf.
pub fn literalRows(pe: anytype, lits: []const Lit, fm: []const FmRow, ours_lits: anytype) ![]Row {
    var starts = try gpa.alloc(u32, fm.len);
    for (fm, 0..) |r, i| starts[i] = r.s;
    var want_at: std.AutoHashMapUnmanaged(u32, void) = .empty;
    for (lits) |l| try want_at.put(gpa, l.addr, {});
    var targets: std.AutoHashMapUnmanaged(u32, std.ArrayList(u32)) = .empty;
    const relocs = pe.relocList();
    for (relocs) |va| {
        const t = pe.u32At(va) orelse continue;
        if (want_at.contains(t)) {
            const g = try targets.getOrPut(gpa, t);
            if (!g.found_existing) g.value_ptr.* = .empty;
            try g.value_ptr.append(gpa, va);
        }
    }
    const tr = pe.textRange();
    const rr = pe.rdataRange();
    var theirs_cache: std.AutoHashMapUnmanaged(u32, *BSet) = .empty;
    var out: std.ArrayList(Row) = .empty;
    for (lits) |l| {
        var sites: std.ArrayList(Site) = .empty;
        var tu_haves: std.ArrayList(*const BSet) = .empty;
        var readers: std.ArrayList(Str) = .empty;
        var code_uses: usize = 0;
        var data_uses: usize = 0;
        const tl: []u32 = if (targets.get(l.addr)) |x| x.items else &.{};
        std.mem.sort(u32, tl, {}, std.sort.asc(u32));
        for (tl) |va| {
            if (tr[0] <= va and va < tr[1]) {
                code_uses += 1;
                var k: usize = 0; // bisect_right(starts, va) - 1
                while (k < starts.len and starts[k] <= va) k += 1;
                if (k == 0) continue;
                const f = fm[k - 1];
                if (!(f.s <= va and va < f.e)) continue;
                try readers.append(gpa, f.name);
                if (try ours_lits.ofFunction(f.tu, f.sym)) |haves| {
                    const th = theirs_cache.get(f.s) orelse blk: {
                        const got = try gpa.create(BSet);
                        got.* = .empty;
                        for (relocsIn(relocs, f.s, f.e)) |rv| {
                            const t = pe.u32At(rv) orelse return error.OutsideImage;
                            if (rr[0] <= t and t < rr[1]) {
                                if (try pe.cstr(t)) |cs| if (cs.len > 0) try got.put(gpa, cs, {});
                            }
                        }
                        try theirs_cache.put(gpa, f.s, got);
                        break :blk got;
                    };
                    try sites.append(gpa, .{ .where = f.name, .haves = haves, .theirs = th });
                    try tu_haves.append(gpa, ours_lits.byTu(f.tu) orelse &empty_set);
                }
            } else data_uses += 1;
        }
        const lf: []const LField = if (literal_fields.get(l.addr)) |x| x.items else &.{};
        for (lf) |f| {
            const s = try gpa.create(BSet);
            s.* = .empty;
            if (f.have) |hv| try s.put(gpa, hv, {});
            try sites.append(gpa, .{ .where = f.where, .haves = s, .theirs = &empty_set });
        }
        const want = l.text;
        var jl: JL = undefined;
        if (tl.len == 0 and lf.len == 0) {
            jl = try judgeLiteral(want, &.{}, ours_lits.everywhereSet(), &.{});
        } else {
            jl = try judgeLiteral(want, sites.items, ours_lits.everywhereSet(), tu_haves.items);
            if (sites.items.len == 0) {
                // Every use sits in code or data no symbol of ours pairs with: the runtime's.
                jl = .{ .status = "UNPAIRED", .method = "none", .notes = try one(try fmt("{d} uses, none in code or data of ours that pairs", .{code_uses + data_uses})) };
            }
        }
        var notes: std.ArrayList(Str) = .empty;
        try notes.append(gpa, try fmt("{s} string, {d} code and {d} data uses in the original", .{ l.source, code_uses, data_uses }));
        try notes.appendSlice(gpa, jl.notes);
        var module = try moduleOfItem(l.ns, l.refs);
        if (eql(module, "?") and readers.items.len > 0) module = try moduleOfItem("", readers.items);
        const status = jl.status;
        const is_match = eql(status, "MATCH");
        const judged = eqlAny(status, &.{ "MATCH", "STRING", "MISSING" });
        const sm = eqlAny(status, &.{ "STRING", "MISSING" });
        const wl: i64 = @intCast(want.len + 1);
        try out.append(gpa, .{ .addr = l.addr, .size = l.extent, .ours_size = if (is_match) wl else 0, .section = pe.sectionOf(l.addr), .module = module, .name = l.name, .symbol = "", .tu = "", .kind = "strlit", .method = jl.method, .typ = l.typ, .status = status, .flags = if (sm) status else "", .first = if (sm) 0 else -1, .diff = if (is_match or !judged) 0 else wl, .matched = if (is_match) l.extent else 0, .notes = try join(notes.items, "; ") });
    }
    return out.items;
}

fn loadFuncmapRows(evidence: Str) ![]FmRow {
    var out: std.ArrayList(FmRow) = .empty;
    const data = (try util.readFileOpt(try util.join(evidence, "funcmap.tsv"))) orelse return out.items;
    var it = util.Lines{ .data = data };
    while (it.next()) |line| {
        if (startsWith(line, "start\t")) continue;
        const col = try util.split(line, '\t');
        try out.append(gpa, .{ .s = try util.hex(col[0]), .e = try util.hex(col[1]), .name = col[2], .tu = col[3], .sym = col[4] });
    }
    std.mem.sort(FmRow, out.items, {}, struct {
        fn lt(_: void, a: FmRow, b: FmRow) bool {
            if (a.s != b.s) return a.s < b.s;
            if (a.e != b.e) return a.e < b.e;
            inline for (.{ "name", "tu", "sym" }) |f| {
                const o = std.mem.order(u8, @field(a, f), @field(b, f));
                if (o != .eq) return o == .lt;
            }
            return false;
        }
    }.lt);
    return out.items;
}

fn runtimeNames(evidence: Str) !std.StringHashMapUnmanaged(void) {
    var out: std.StringHashMapUnmanaged(void) = .empty;
    const data = (try util.readFileOpt(try util.join(evidence, "runtime.tsv"))) orelse return out;
    var it = util.Lines{ .data = data };
    while (it.next()) |line| {
        const col = try util.split(line, '\t');
        if (col.len > 1 and !eql(col[0], "start")) try out.put(gpa, col[1], {});
    }
    return out;
}

const Cause = struct { cause: Str, risk: Str, why: Str };

/// (likely root cause, runtime risk, why) for a row that does not MATCH.
fn rootCause(r: *const Row) !Cause {
    const status = r.status;
    const typ = r.typ;
    const notes = r.notes;
    const kind = r.kind;
    const referenced = contains(notes, "referenced by our code");
    const risk: Str = if (referenced) "yes" else "no";
    if (eql(kind, "interior")) return .{ .cause = "generator", .risk = if (referenced) "yes" else "no", .why = try fmt("interior label emitted as an object of its own; the original's name is an alias into its container, so a reader of ours sees a separate copy{s}", .{if (referenced) "" else " (nothing reads it)"}) };
    if (eql(kind, "strlit")) return .{ .cause = "generator", .risk = "maybe", .why = try fmt("string the generator spells as a literal: {s}; read the function", .{if (eql(status, "STRING")) "our use names a literal of other content" else "no use of ours names it (an inlined copy split into pieces, or a call the body lost)"}) };
    if (contains(notes, "address claimed by") and contains(r.name, "COOKIE")) return .{ .cause = "generator", .risk = "no", .why = "a runtime (CRT) global the library defines too; ours shadows it" };
    if (eql(status, "RELOC")) {
        if (contains(notes, "raw original addresses")) {
            if (isPointerish(typ)) return .{ .cause = "generator", .risk = risk, .why = "pointer initialiser to an address with no symbol emitted as a raw 1.14d address cast; label the target in Ghidra or emit &symbol" };
            return .{ .cause = "ghidra-type", .risk = risk, .why = try fmt("a pointer slot of {s} is typed as an integer or bytes, so the generator emits the 1.14d address as a number; type that slot as a pointer", .{typ}) };
        }
        if (contains(notes, "null where the original points")) return .{ .cause = "generator", .risk = risk, .why = "pointer initialiser dropped (null where the original points)" };
        return .{ .cause = "generator", .risk = risk, .why = "pointer names another target than the original" };
    }
    if (eql(status, "ZERO")) {
        if (contains(notes, "dynamic initialisation")) return .{ .cause = "generator", .risk = "low", .why = "the initialiser calls a constructor, so VC8 builds the table at run time (zero in the image, a once-guard in the function); emit it as aggregate initialisation" };
        if (eql(typ, "undefined") or eql(typ, "undefined1") or r.size == 1) return .{ .cause = "ghidra-extent", .risk = "no", .why = "a one-byte undefined item where the original holds a wider value" };
        return .{ .cause = "generator", .risk = risk, .why = "initialiser not emitted as static data (zero in our image): a constructor-style initialiser compiles to dynamic initialisation, or none was written" };
    }
    if (eql(kind, "const") and contains(notes, "no literal of that value")) return .{ .cause = "generator", .risk = "maybe", .why = "the readers' objects hold no literal of this value: the constant is computed another way or another one is used; read the function" };
    if (eql(kind, "const") or (eql(status, "VALUE") and contains(notes, "FLOAT"))) return .{ .cause = "generator", .risk = if (eql(kind, "const") or referenced) "yes" else "no", .why = try fmt("floating-point value printed with too few digits (float needs %.9g, double %.17g); {s}", .{if (eql(kind, "const")) "the literal our code computes with differs" else "the table differs"}) };
    if ((eql(status, "VALUE") or eql(status, "STRING")) and isStringType(if (typ.len > 0) typ else "char") and r.first >= 0) return .{ .cause = "ghidra-extent", .risk = "no", .why = "string item runs past its terminator into bytes ours pads with zero" };
    return .{ .cause = "generator", .risk = risk, .why = "bytes differ" };
}

fn writeFixes(rows: []const Row, path: Str, top: usize) !void {
    var bad: std.ArrayList(Row) = .empty;
    var dead: std.ArrayList(Row) = .empty;
    for (rows) |r| {
        if (eqlAny(r.status, &.{ "MATCH", "UNUSED", "UNPAIRED" })) continue;
        if (eql(r.kind, "interior") and !contains(r.notes, "referenced by our code")) try dead.append(gpa, r) else try bad.append(gpa, r);
    }
    std.mem.sort(Row, bad.items, {}, struct {
        fn lt(_: void, a: Row, b: Row) bool {
            if (a.size != b.size) return a.size > b.size;
            return a.addr < b.addr;
        }
    }.lt);
    var o: std.ArrayList(u8) = .empty;
    try o.appendSlice(gpa, "rank\taddress\tbytes\tsection\tname\ttu\tstatus\troot_cause\truntime_risk\twhy\tnotes\n");
    var rank: usize = 0;
    for (bad.items, 0..) |*r, k| {
        const rc = try rootCause(r);
        if (!(k < top or eqlAny(rc.risk, &.{ "yes", "low", "maybe" }))) continue;
        rank += 1;
        try o.print(gpa, "{d}\t{x:0>8}\t{d}\t{s}\t{s}\t{s}\t{s}\t{s}\t{s}\t{s}\t{s}\n", .{ rank, r.addr, r.size, r.section, r.name, r.tu, r.status, rc.cause, rc.risk, rc.why, r.notes });
    }
    if (dead.items.len > 0) {
        var nb: usize = 0;
        for (dead.items) |r| if (eql(r.section, ".bss")) {
            nb += 1;
        };
        try o.print(gpa, "-\t-\t-\t-\t({d} interior labels)\t-\tSPLIT\tgenerator\tno\temitted as objects of their own that nothing of ours references ({d} .bss, {d} initialised): dead copies, see data-audit.tsv kind interior\t\n", .{ dead.items.len, nb, dead.items.len - nb });
    }
    try util.writeFile(path, o.items);
}

const TREND = [_]struct { lo: i64, hi: i64, name: Str }{
    .{ .lo = 1, .hi = 4, .name = "1-4" },                .{ .lo = 5, .hi = 16, .name = "5-16" },       .{ .lo = 17, .hi = 64, .name = "17-64" },
    .{ .lo = 65, .hi = 256, .name = "65-256" },          .{ .lo = 257, .hi = 1024, .name = "257-1K" }, .{ .lo = 1025, .hi = 4096, .name = "1K-4K" },
    .{ .lo = 4097, .hi = 1000000000000, .name = "4K+" },
};

/// The original image as literalRows and literalCandidates read it.
const RealPe = struct {
    img: *const pe_mod.Image,
    relocs: []const u32,
    pub fn relocList(p: *const RealPe) []const u32 {
        return p.relocs;
    }
    pub fn u32At(p: *const RealPe, va: u32) ?u32 {
        return p.img.u32At(va);
    }
    pub fn textRange(p: *const RealPe) [2]u64 {
        const t = p.img.section(".text").?;
        return .{ @as(u64, p.img.base) + t.va, @as(u64, p.img.base) + t.va + t.vsize };
    }
    pub fn rdataRange(p: *const RealPe) [2]u64 {
        const t = p.img.section(".rdata").?;
        return .{ @as(u64, p.img.base) + t.va, @as(u64, p.img.base) + t.end() };
    }
    pub fn cstr(p: *const RealPe, va: u32) !?Str {
        return p.img.cstring(gpa, va, 4096);
    }
    pub fn sectionOf(p: *const RealPe, va: u32) Str {
        return sectionOf_(p.img, va);
    }
    pub fn read(p: *const RealPe, va: u32, n: usize) !Str {
        const buf = try gpa.alloc(u8, n);
        if (n > 0 and !p.img.read(va, buf)) return error.OutsideImage;
        return buf;
    }
};

const sectionOf_ = sectionOf;

/// OurLiterals as literalRows reads it.
const OurLitView = struct {
    ol: *OurLiterals,
    pub fn ofFunction(v: *const OurLitView, tu: Str, sym: Str) !?*const BSet {
        return v.ol.ofFunction(tu, sym);
    }
    pub fn byTu(v: *const OurLitView, tu: Str) ?*const BSet {
        return v.ol.byTu(tu);
    }
    pub fn everywhereSet(v: *const OurLitView) *const BSet {
        return &v.ol.everywhere;
    }
};

const AuditArgs = struct {
    extents: Str,
    obj: []const Str,
    datamap: Str,
    out: Str,
    trend: Str,
    fixes: Str,
    label: Str,
    strings: ?Str,
    compiler_data: ?Str,
};

fn auditMain(cfg: *const audit.Config, args: AuditArgs) !void {
    const evidence = try util.join(cfg.oracle, "evidence");
    const img_data = try util.readFile(try util.join(cfg.oracle, "reference/Game.exe"));
    const img = try gpa.create(pe_mod.Image);
    img.* = try pe_mod.Image.parse(gpa, img_data);
    var rl: std.ArrayList(u32) = .empty;
    {
        var it = img.relocs.keyIterator();
        while (it.next()) |k| try rl.append(gpa, k.*);
    }
    std.mem.sort(u32, rl.items, {}, std.sort.asc(u32));
    const relocs = rl.items;
    const ext = try loadExtents(args.extents);
    const items = ext.items;
    var objdirs: std.ArrayList(ObjDir) = .empty;
    for (args.obj) |spec| {
        const eqp = std.mem.indexOfScalar(u8, spec, '=');
        const root = if (eqp) |k| spec[0..k] else spec;
        const sub = if (eqp) |k| spec[k + 1 ..] else "";
        if (isDir(root)) try objdirs.append(gpa, .{ .root = root, .sub = sub });
    }
    var lo = try loadOurs(objdirs.items);
    const ours = &lo.ours;
    const objs = &lo.objs;
    const datamap = try loadDatamap(args.datamap);
    var pairs = try pair(ours, items, &datamap);
    var fs = try loadFuncsyms(evidence);
    {
        var rn = try runtimeNames(evidence);
        var it = rn.keyIterator();
        while (it.next()) |n| _ = fs.names.orderedRemove(n.*);
    }
    const game_fns = &fs.names;
    var res = try Resolver.init(img, &pairs, &fs.syms);

    var rows: std.ArrayList(Row) = .empty;
    var paired_addr: std.AutoArrayHashMapUnmanaged(u32, i64) = .empty;
    var by_item: std.AutoHashMapUnmanaged(*Item, *Ours) = .empty;
    var dead_float: std.AutoHashMapUnmanaged(u32, Str) = .empty;
    for (pairs.keys(), pairs.values()) |uid, p| {
        const g = try by_item.getOrPut(gpa, p.it);
        if (!g.found_existing) g.value_ptr.* = ours.get(uid).?;
    }
    const order = try gpa.alloc(usize, pairs.count());
    for (order, 0..) |*x, i| x.* = i;
    const SortCtx = struct {
        p: *const PairMap,
        fn lt(ctx: @This(), a: usize, b: usize) bool {
            const ia = ctx.p.values()[a].it.addr;
            const ib = ctx.p.values()[b].it.addr;
            if (ia != ib) return ia < ib;
            return std.mem.order(u8, ctx.p.keys()[a], ctx.p.keys()[b]) == .lt;
        }
    };
    std.mem.sort(usize, order, SortCtx{ .p = &pairs }, SortCtx.lt);
    for (order) |idx| {
        const uid = pairs.keys()[idx];
        const p = pairs.values()[idx];
        const it = p.it;
        const u = ours.get(uid).?;
        const sym = u.symbol;
        if (eql(it.kind, "data") and isFloatType(it.typ) and (it.extent == 4 or it.extent == 8) and eql(sectionOf(img, it.addr), ".rdata") and !(try usedByCode(objs, u))) {
            // A constant our code never reads by name: this copy is dead.
            try dead_float.put(gpa, it.addr, sym);
            continue;
        }
        {
            const g = try paired_addr.getOrPut(gpa, it.addr);
            if (!g.found_existing) g.value_ptr.* = 0;
            g.value_ptr.* += 1;
        }
        var j = try judge(img, relocs, it, u, &res);
        if (p.note.len > 0) try j.notes.append(gpa, p.note);
        if (eql(it.kind, "interior")) {
            // Ours is an object of its own unless it sits where the container's symbol of
            // ours puts that offset.
            const cu: ?*Ours = if (it.parent) |par| by_item.get(par) else null;
            const cont = cu != null and cu.?.sec == u.sec and @as(i64, cu.?.off) + (@as(i64, it.addr) - it.parent.?.addr) == u.off;
            if (!cont) {
                try j.flags.insert(gpa, 0, "SPLIT");
                if (!eql(j.status, "ZERO")) j.status = "SPLIT";
                j.matched = 0;
            }
            try j.notes.append(gpa, try fmt("interior label of {s} +{d}{s}", .{ if (it.parent) |par| par.name else "?", if (it.parent) |par| @as(i64, it.addr) - par.addr else 0, if (cont) ", laid out inside it" else ", emitted as an object of its own" }));
        }
        if (eql(j.status, "ZERO") and u.bss and try dynamicInit(u)) try j.notes.append(gpa, "dynamic initialisation: our code builds it at run time");
        if (!eql(j.status, "MATCH")) try j.notes.append(gpa, if (try usedByCode(objs, u)) "referenced by our code" else "nothing of ours references it");
        const kind: Str = if (isLiteral(sym) or eql(it.kind, "literal")) "literal" else it.kind;
        try rows.append(gpa, .{ .addr = it.addr, .size = it.extent, .ours_size = u.extent, .section = sectionOf(img, it.addr), .module = moduleOf(u.tu), .name = it.name, .symbol = sym, .tu = u.tu, .kind = kind, .method = p.method, .typ = it.typ, .status = j.status, .flags = try join(j.flags.items, ","), .first = j.first, .diff = j.diff, .matched = j.matched, .notes = try join(j.notes.items, "; ") });
    }
    const consts = try floatConstants(img, items, &paired_addr, objs, game_fns, &dead_float);
    for (consts) |r| {
        const g = try paired_addr.getOrPut(gpa, r.addr);
        if (!g.found_existing) g.value_ptr.* = 0;
        g.value_ptr.* += 1;
    }
    try rows.appendSlice(gpa, consts);
    // Strings the generator spells as literals: no symbol of ours names them.
    const dstr = try loadDatastrings(args.strings);
    var read_only = dstr.ro;
    if (read_only.len == 0) {
        const s = img.section(".rdata").?;
        read_only = try gpa.dupe(Span, &.{.{ .lo = img.base + s.va, .hi = img.base + s.end() }});
    }
    var covered0: std.AutoHashMapUnmanaged(u32, void) = .empty;
    for (paired_addr.keys()) |a| try covered0.put(gpa, a, {});
    const rpe = RealPe{ .img = img, .relocs = relocs };
    var cdata = try loadCompilerData(args.compiler_data);
    const lits = try literalCandidates(items, dstr.rows, &rpe, &covered0, read_only, if (cdata) |*cd| cd else null);
    var ol = try OurLiterals.init(objs);
    const olv = OurLitView{ .ol = &ol };
    const lit_rows = try literalRows(&rpe, lits, try loadFuncmapRows(evidence), &olv);
    try rows.appendSlice(gpa, lit_rows);
    std.mem.sort(Row, rows.items, {}, struct {
        fn lt(_: void, a: Row, b: Row) bool {
            if (a.addr != b.addr) return a.addr < b.addr;
            return std.mem.order(u8, a.symbol, b.symbol) == .lt;
        }
    }.lt);
    // Twice-paired addresses: two of ours claim one original item.
    for (rows.items) |*r| if (paired_addr.get(r.addr)) |k| if (k > 1) {
        r.notes = try fmt("{s}{s}address claimed by {d} symbols", .{ r.notes, if (r.notes.len > 0) "; " else "", k });
    };

    util.makeDirs(util.dirname(args.out));
    {
        var o: std.ArrayList(u8) = .empty;
        try o.appendSlice(gpa, AUDIT_HEADER ++ "\n");
        for (rows.items) |r| try o.print(gpa, "{x:0>8}\t{d}\t{d}\t{s}\t{s}\t{s}\t{s}\t{s}\t{s}\t{s}\t{s}\t{s}\t{s}\t{d}\t{d}\t{d}\t{s}\n", .{ r.addr, r.size, r.ours_size, r.section, r.module, r.name, r.symbol, r.tu, r.kind, r.method, r.typ, r.status, r.flags, r.first, r.diff, r.matched, r.notes });
        try util.writeFile(args.out, o.items);
    }

    // The headline: data the game's own code reads.
    var covered: std.AutoHashMapUnmanaged(u32, void) = .empty;
    for (paired_addr.keys()) |a| try covered.put(gpa, a, {});
    for (lit_rows) |r| try covered.put(gpa, r.addr, {});
    var uncompared: std.ArrayList(*Item) = .empty;
    var runtime_items: std.ArrayList(*Item) = .empty;
    var excluded: Counter = .{};
    for (items) |it| {
        if (covered.contains(it.addr)) continue;
        if (eql(it.kind, "import") or eql(it.kind, "compiler")) {
            try excluded.add(it.kind, 1);
            continue;
        }
        if (eql(it.kind, "interior")) continue; // its bytes are its container's
        var game = false;
        for (it.refs) |r| if (game_fns.contains(r)) {
            game = true;
        };
        if (game) try uncompared.append(gpa, it) else try runtime_items.append(gpa, it);
    }
    var named: std.ArrayList(Row) = .empty;
    var interior: std.ArrayList(Row) = .empty;
    var lit: std.ArrayList(Row) = .empty;
    for (rows.items) |r| {
        if (!eqlAny(r.kind, &.{ "literal", "strlit", "import", "compiler", "interior" })) try named.append(gpa, r);
        if (eql(r.kind, "interior")) try interior.append(gpa, r);
        if (eql(r.kind, "literal")) try lit.append(gpa, r);
    }
    const tree = try audit.gitDescribe(cfg.repo);
    const initOf = struct {
        fn f(rs: []const Row) ![]Row {
            var o: std.ArrayList(Row) = .empty;
            for (rs) |r| if (!eql(r.section, ".bss")) try o.append(gpa, r);
            return o.items;
        }
    }.f;
    const sumSize = struct {
        fn f(rs: []const Row) i64 {
            var t: i64 = 0;
            for (rs) |r| t += r.size;
            return t;
        }
    }.f;
    const matchOf = struct {
        fn f(rs: []const Row) ![]Row {
            var o: std.ArrayList(Row) = .empty;
            for (rs) |r| if (eql(r.status, "MATCH")) try o.append(gpa, r);
            return o.items;
        }
    }.f;

    var lines: std.ArrayList(Str) = .empty;
    {
        var subs: std.ArrayList(Str) = .empty;
        for (objdirs.items) |od| try subs.append(gpa, try fmt("{s}/", .{if (od.sub.len > 0) od.sub else "build/obj"}));
        try lines.append(gpa, try fmt("tree {s}; Ghidra v{s} extents ({s}); {d} objects ({s})", .{ tree, ext.version, util.basename(args.extents), objs.count(), try join(subs.items, " + ") }));
    }
    // fig(named, uncompared)
    const ri_named = try initOf(named.items);
    const m_named = try matchOf(ri_named);
    var unc_init: std.ArrayList(*Item) = .empty;
    for (uncompared.items) |i| if (!eql(sectionOf(img, i.addr), ".bss")) try unc_init.append(gpa, i);
    var unc_b: i64 = 0;
    for (unc_init.items) |i| unc_b += i.extent;
    const mn: i64 = @intCast(m_named.len);
    const tn: i64 = @as(i64, @intCast(ri_named.len)) + @as(i64, @intCast(unc_init.items.len));
    const mb = sumSize(m_named);
    const tb = sumSize(ri_named) + unc_b;
    var ib: i64 = 0;
    for (ri_named) |r| ib += r.matched;
    {
        var l: std.ArrayList(u8) = .empty;
        try l.print(gpa, "Initialised game data: {d} of {d} symbols MATCH (", .{ mn, tn });
        try pct(&l, "%.2f", mn, tn);
        try l.print(gpa, "%), {d} of {d} bytes (", .{ mb, tb });
        try pct(&l, "%.2f", mb, tb);
        try l.print(gpa, "%); bytes equal in place {d} (", .{ib});
        try pct(&l, "%.2f", ib, tb);
        try l.appendSlice(gpa, "%)");
        try lines.append(gpa, l.items);
    }
    try lines.append(gpa, try fmt("  uncompared: {d} game items ({d} bytes) that no data symbol of ours pairs with", .{ unc_init.items.len, unc_b }));
    var rt_b: i64 = 0;
    for (runtime_items.items) |i| rt_b += i.extent;
    try lines.append(gpa, try fmt("  left out: {d} runtime-only items ({d} bytes), {d} import slots, {d} compiler EH/RTTI items", .{ runtime_items.items.len, rt_b, excluded.get("import"), excluded.get("compiler") }));
    var bss: std.ArrayList(Row) = .empty;
    for (named.items) |r| if (eql(r.section, ".bss")) try bss.append(gpa, r);
    const bm = try matchOf(bss.items);
    try lines.append(gpa, try fmt("Uninitialised (.bss), size and alignment only: {d} of {d} symbols agree, {d} of {d} bytes", .{ bm.len, bss.items.len, sumSize(bm), sumSize(bss.items) }));
    {
        var sp_n: usize = 0;
        var sp_bss: usize = 0;
        var sp_zero: usize = 0;
        for (interior.items) |r| if (!eql(r.status, "MATCH")) {
            sp_n += 1;
            if (eql(r.section, ".bss")) sp_bss += 1;
            if (eql(r.status, "ZERO")) sp_zero += 1;
        };
        try lines.append(gpa, try fmt("Interior labels (a name inside a larger Ghidra item): {d} paired, {d} emitted as an object of their own ({d} .bss, {d} initialised, {d} of those all zero)", .{ interior.items.len, sp_n, sp_bss, sp_n - sp_bss, sp_zero }));
    }
    const lm = try matchOf(lit.items);
    try lines.append(gpa, try fmt("Compiler literals (__real@, ??_C@, ??_7) paired by datamap: {d} of {d} MATCH, {d} of {d} bytes", .{ lm.len, lit.items.len, sumSize(lm), sumSize(lit.items) }));
    var lc: Counter = .{};
    var lb: Counter = .{};
    var used: std.ArrayList(Row) = .empty;
    for (lit_rows) |r| {
        try lc.add(r.status, 1);
        try lb.add(r.status, r.size);
        if (eqlAny(r.status, &.{ "MATCH", "STRING", "MISSING" })) try used.append(gpa, r);
    }
    {
        var diffs: std.ArrayList(Str) = .empty;
        for ([_]Str{ "STRING", "MISSING" }) |k| if (lc.get(k) != 0) try diffs.append(gpa, try fmt("{s} {d}", .{ k, lc.get(k) }));
        try lines.append(gpa, try fmt("String literals (read-only strings the generator spells at their uses; {s}, strings v{s}): {d} of {d} MATCH, {d} of {d} bytes; {s}; content compared, placement not", .{ if (args.compiler_data != null and args.compiler_data.?.len > 0) "its compiler-data report" else "its byte rule", dstr.version, lc.get("MATCH"), used.items.len, lb.get("MATCH"), sumSize(used.items), if (diffs.items.len > 0) try join(diffs.items, ", ") else "none differ" }));
    }
    try lines.append(gpa, try fmt("  left out: {d} the original never references (UNUSED), {d} used only by code or data no symbol of ours pairs with (UNPAIRED: the runtime's)", .{ lc.get("UNUSED"), lc.get("UNPAIRED") }));
    {
        var lm_by: Counter = .{};
        var typed_n: usize = 0;
        var untyped_n: usize = 0;
        for (lit_rows) |r| {
            if (eql(r.status, "MATCH")) try lm_by.add(r.method, 1);
            if (startsWith(r.notes, "typed")) typed_n += 1;
            if (startsWith(r.notes, "untyped")) untyped_n += 1;
        }
        const ks = try sortedStrs(lm_by.map.keys());
        var parts: std.ArrayList(Str) = .empty;
        for (ks) |k| try parts.append(gpa, try fmt("{s} {d}", .{ k, lm_by.get(k) }));
        try lines.append(gpa, try fmt("  paired by: {s}; {d} typed (dataextent), {d} untyped (datastrings)", .{ if (parts.items.len > 0) try join(parts.items, ", ") else "none", typed_n, untyped_n }));
    }
    {
        var lt: std.ArrayList(Row) = .empty;
        for (lit_rows) |r| if (startsWith(r.notes, "typed") and eqlAny(r.status, &.{ "MATCH", "STRING", "MISSING" }) and !eql(r.section, ".bss")) try lt.append(gpa, r);
        const ltm = try matchOf(lt.items);
        const a: i64 = mn + @as(i64, @intCast(ltm.len));
        const b: i64 = tn + @as(i64, @intCast(lt.items.len));
        const ab = mb + sumSize(ltm);
        const bb = tb + sumSize(lt.items);
        var l: std.ArrayList(u8) = .empty;
        try l.print(gpa, "Initialised game data with the typed string literals: {d} of {d} symbols MATCH (", .{ a, b });
        try pct(&l, "%.2f", a, b);
        try l.print(gpa, "%), {d} of {d} bytes (", .{ ab, bb });
        try pct(&l, "%.2f", ab, bb);
        try l.appendSlice(gpa, "%)");
        try lines.append(gpa, l.items);
    }
    try lines.append(gpa, "status of the initialised game data compared (symbols, original bytes):");
    var st: Counter = .{};
    var stb: Counter = .{};
    for (ri_named) |r| {
        try st.add(r.status, 1);
        try stb.add(r.status, r.size);
    }
    for ([_]Str{ "MATCH", "ZERO", "SIZE_SMALL", "RELOC", "STRING", "VALUE", "SIZE_LARGE", "BSS_INIT", "ALIGN" }) |k| {
        var l: std.ArrayList(u8) = .empty;
        try l.appendSlice(gpa, "  ");
        try padR(&l, k, 11);
        try l.append(gpa, ' ');
        try padLInt(&l, st.get(k), 6);
        try l.append(gpa, ' ');
        try padLInt(&l, stb.get(k), 9);
        try lines.append(gpa, l.items);
    }
    {
        var fl: Counter = .{};
        for (ri_named) |r| {
            var it = std.mem.splitScalar(u8, r.flags, ',');
            while (it.next()) |f| if (f.len > 0) try fl.add(f, 1);
        }
        var parts: std.ArrayList(Str) = .empty;
        for (try fl.mostCommon()) |kv| try parts.append(gpa, try fmt("{s} {d}", .{ kv.k, kv.n }));
        try lines.append(gpa, try fmt("flags (a symbol may carry several): {s}", .{try join(parts.items, ", ")}));
    }
    {
        var raw: usize = 0;
        for (named.items) |r| if (contains(r.notes, "raw original address")) {
            raw += 1;
        };
        try lines.append(gpa, try fmt("symbols holding a raw 1.14d address where the original relocates: {d}", .{raw}));
    }
    {
        const keys = try sortedStrs(ptr_stats.keys());
        var kinds: std.ArrayList(Str) = .empty;
        for (keys) |k| {
            const kind = k[0..std.mem.indexOfScalar(u8, k, 0).?];
            if (kinds.items.len == 0 or !eql(kinds.items[kinds.items.len - 1], kind)) try kinds.append(gpa, kind);
        }
        var parts: std.ArrayList(Str) = .empty;
        for (kinds.items) |kind| {
            var vs: std.ArrayList(Str) = .empty;
            for (keys) |k| {
                const z = std.mem.indexOfScalar(u8, k, 0).?;
                if (eql(k[0..z], kind)) try vs.append(gpa, try fmt("{s} {d}", .{ k[z + 1 ..], ptr_stats.get(k).? }));
            }
            try parts.append(gpa, try fmt("{s} {s}", .{ kind, try join(vs.items, ", ") }));
        }
        try lines.append(gpa, try fmt("pointer fields compared, by what ours names: {s}", .{try join(parts.items, "; ")}));
    }
    try lines.append(gpa, "by module (initialised; symbols MATCH/total, bytes MATCH/total):");
    {
        var mods: NameSet = .empty;
        for (named.items) |r| try mods.put(gpa, r.module, {});
        for (try sortedStrs(mods.keys())) |mod| {
            var sel: std.ArrayList(Row) = .empty;
            for (named.items) |r| if (eql(r.module, mod)) try sel.append(gpa, r);
            const rs = try initOf(sel.items);
            if (rs.len == 0) continue;
            const m = try matchOf(rs);
            var l: std.ArrayList(u8) = .empty;
            try l.appendSlice(gpa, "  ");
            try padR(&l, mod, 12);
            try l.append(gpa, ' ');
            try padLInt(&l, @intCast(m.len), 5);
            try l.append(gpa, '/');
            try padRInt(&l, @intCast(rs.len), 5);
            try l.append(gpa, ' ');
            try padLInt(&l, sumSize(m), 8);
            try l.append(gpa, '/');
            try padRInt(&l, sumSize(rs), 8);
            try l.append(gpa, ' ');
            try pct(&l, "%6.2f", sumSize(m), sumSize(rs));
            try l.append(gpa, '%');
            try lines.append(gpa, l.items);
        }
    }
    try lines.append(gpa, "by section (symbols MATCH/total, bytes MATCH/total):");
    for ([_]Str{ ".rdata", ".data", ".bss" }) |sec| {
        var rs: std.ArrayList(Row) = .empty;
        for (named.items) |r| if (eql(r.section, sec)) try rs.append(gpa, r);
        const m = try matchOf(rs.items);
        var l: std.ArrayList(u8) = .empty;
        try l.appendSlice(gpa, "  ");
        try padR(&l, sec, 7);
        try l.append(gpa, ' ');
        try padLInt(&l, @intCast(m.len), 5);
        try l.append(gpa, '/');
        try padRInt(&l, @intCast(rs.items.len), 5);
        try l.append(gpa, ' ');
        try padLInt(&l, sumSize(m), 8);
        try l.append(gpa, '/');
        try padRInt(&l, sumSize(rs.items), 8);
        try lines.append(gpa, l.items);
    }
    {
        var cm: usize = 0;
        for (consts) |r| if (eql(r.status, "MATCH")) {
            cm += 1;
        };
        try lines.append(gpa, try fmt("floating-point constants judged by the literals of their readers' objects: {d}, {d} MATCH", .{ consts.len, cm }));
    }
    {
        var pm: Counter = .{};
        for (rows.items) |r| if (!eql(r.kind, "strlit")) try pm.add(r.method, 1);
        var parts: std.ArrayList(Str) = .empty;
        for (try pm.mostCommon()) |kv| try parts.append(gpa, try fmt("{s} {d}", .{ kv.k, kv.n }));
        var unp_lit: usize = 0;
        for (ours.keys(), ours.values()) |k, u| if (!pairs.contains(k) and isLiteral(u.symbol)) {
            unp_lit += 1;
        };
        try lines.append(gpa, try fmt("pairings: {s}; data symbols of ours unpaired {d} ({d} of them compiler literals); defined twice {d}", .{ try join(parts.items, ", "), ours.count() - pairs.count(), unp_lit, lo.dups }));
    }
    {
        var dis: usize = 0;
        for (rows.items) |r| if (contains(r.notes, "datamap implies")) {
            dis += 1;
        };
        try lines.append(gpa, try fmt("name pair disagrees with the datamap address: {d}", .{dis}));
    }
    const text = try fmt("{s}\n", .{try join(lines.items, "\n")});
    try util.writeFile(try fmt("{s}.summary", .{args.out}), text);
    util.say(text);

    try writeFixes(rows.items, args.fixes, 50);

    // datatrend: MATCH rate by Ghidra size.
    var trend: std.ArrayList(Str) = .empty;
    try trend.append(gpa, if (args.label.len > 0) try fmt("# datatrend {s}", .{args.label}) else "# datatrend");
    try trend.append(gpa, "bucket\tsymbols\tmatch_pct\tbytes_pct\tzero\tsize_small\treloc\tvalue");
    const tot = sumSize(ri_named);
    for (TREND) |b| {
        var rs: std.ArrayList(Row) = .empty;
        for (ri_named) |r| if (b.lo <= r.size and r.size <= b.hi) try rs.append(gpa, r);
        var cn: Counter = .{};
        for (rs.items) |r| try cn.add(r.status, 1);
        var l: std.ArrayList(u8) = .empty;
        try l.print(gpa, "{s}\t{d}\t", .{ b.name, rs.items.len });
        try pct(&l, "%.2f", cn.get("MATCH"), @intCast(rs.items.len));
        try l.append(gpa, '\t');
        try pct(&l, "%.2f", sumSize(rs.items), tot);
        try l.print(gpa, "\t{d}\t{d}\t{d}\t{d}", .{ cn.get("ZERO"), cn.get("SIZE_SMALL"), cn.get("RELOC"), cn.get("VALUE") + cn.get("STRING") });
        try trend.append(gpa, l.items);
    }
    try util.writeFile(args.trend, try fmt("{s}\n", .{try join(trend.items, "\n")}));
}

fn defaultSnapshot() !?Str {
    var cands: std.ArrayList(Str) = .empty;
    if (util.getenv("ORACLE_GHIDRA_SNAPSHOT")) |p| try cands.append(gpa, p);
    const home = util.getenv("HOME") orelse "~";
    try cands.append(gpa, try fmt("{s}/code/ts/gr-daemon/project/.ghidra-mcp/codegen-snapshot", .{home}));
    try cands.append(gpa, try fmt("{s}/code/ts/ghidra-reconstruct/project/.ghidra-mcp/codegen-snapshot", .{home}));
    for (cands.items) |p| if (p.len > 0 and util.exists(try util.join(p, "globals.ndjson"))) return p;
    return null;
}

const USAGE =
    \\usage: oracle-zig data [extents|strings|audit] --root <asm_matching/msvc8> [options]
    \\  extents [--snapshot DIR] [--out O] [--strings-out S]
    \\  strings [--snapshot DIR] [--strings-out S]
    \\  audit   [--extents E] [--obj DIR[=PREFIX] ...] [--datamap D] [--out O] [--trend T]
    \\          [--fixes F] [--label L] [--strings S] [--compiler-data R]
    \\
;

pub fn main(args: []const []const u8) !void {
    var cmd: ?Str = null;
    const Opt = struct { name: Str, val: ?Str = null, multi: std.ArrayList(Str) = .empty };
    var opts = [_]Opt{ .{ .name = "--root" }, .{ .name = "--snapshot" }, .{ .name = "--out" }, .{ .name = "--strings-out" }, .{ .name = "--extents" }, .{ .name = "--obj" }, .{ .name = "--datamap" }, .{ .name = "--trend" }, .{ .name = "--fixes" }, .{ .name = "--label" }, .{ .name = "--strings" }, .{ .name = "--compiler-data" } };
    const rest = args;
    var k: usize = 0;
    while (k < rest.len) : (k += 1) {
        const a = rest[k];
        if (cmd == null and !startsWith(a, "-")) {
            cmd = a;
            continue;
        }
        var hit = false;
        for (&opts) |*o| {
            var v: ?Str = null;
            if (eql(a, o.name) and k + 1 < rest.len) {
                v = rest[k + 1];
                k += 1;
            } else if (startsWith(a, o.name) and a.len > o.name.len and a[o.name.len] == '=') v = a[o.name.len + 1 ..];
            if (v) |x| {
                o.val = x;
                try o.multi.append(gpa, x);
                hit = true;
                break;
            }
        }
        if (!hit) {
            std.debug.print(USAGE, .{});
            std.process.exit(2);
        }
    }
    const get = struct {
        fn f(os: []Opt, name: Str) ?Str {
            for (os) |o| if (eql(o.name, name)) return o.val;
            return null;
        }
    }.f;
    const root = get(&opts, "--root") orelse {
        std.debug.print("oracle-zig data: --root is required\n", .{});
        std.process.exit(2);
    };
    const cfg = try audit.Config.init(root);
    const command = cmd orelse "audit";
    const evidence = try util.join(root, "evidence");
    if (eql(command, "extents") or eql(command, "strings")) {
        const snap = get(&opts, "--snapshot") orelse (try defaultSnapshot()) orelse
            die("no codegen snapshot found; pass --snapshot DIR (holding {s}.ndjson)", .{if (eql(command, "extents")) "globals" else "strings"});
        const strings_out = get(&opts, "--strings-out") orelse try util.join(evidence, "datastrings.tsv");
        if (eql(command, "extents")) try extentsMain(snap, get(&opts, "--out") orelse try util.join(evidence, "dataextent.tsv"));
        try stringsMain(snap, strings_out);
        return;
    }
    if (!eql(command, "audit")) {
        std.debug.print(USAGE, .{});
        std.process.exit(2);
    }
    var out = get(&opts, "--out");
    if (out == null) {
        out = try util.join(if (cfg.experiment) cfg.build else evidence, "data-audit.tsv");
    } else if (cfg.experiment and eql(try realpath(util.dirname(out.?)), try realpath(evidence))) {
        die("refusing to write an experiment data audit into evidence/", .{});
    }
    var objs: []const Str = undefined;
    for (opts) |o| if (eql(o.name, "--obj")) {
        objs = o.multi.items;
    };
    if (objs.len == 0) {
        // Source/globals/ holds most of the data and is not compiled into build/obj: the
        // whole-image build compiles it into build-exe/obj/Source/globals.
        const list = try gpa.alloc(Str, 2);
        list[0] = try util.join(cfg.build, "obj");
        list[1] = try fmt("{s}/build-exe/obj/Source/globals=Source/globals", .{root});
        for (list) |spec| {
            const r = spec[0 .. std.mem.indexOfScalar(u8, spec, '=') orelse spec.len];
            if (!isDir(r)) std.debug.print("warning: {s} missing; its data is not compared\n", .{r});
        }
        objs = list;
    }
    const extents = get(&opts, "--extents") orelse try util.join(evidence, "dataextent.tsv");
    if (!util.exists(extents)) die("{s} missing: run `oracle data extents` first", .{extents});
    try auditMain(&cfg, .{
        .extents = extents,
        .obj = objs,
        .datamap = get(&opts, "--datamap") orelse try util.join(evidence, "datamap.tsv"),
        .out = out.?,
        .trend = get(&opts, "--trend") orelse try util.join(util.dirname(out.?), "datatrend.tsv"),
        .fixes = get(&opts, "--fixes") orelse try util.join(util.dirname(out.?), "data-fixes.tsv"),
        .label = get(&opts, "--label") orelse "",
        .strings = get(&opts, "--strings") orelse try util.join(evidence, "datastrings.tsv"),
        .compiler_data = get(&opts, "--compiler-data"),
    });
}

// -- tests: test_datamatch_literals.py --------------------------------------------------

/// An image with .text at 0x401000 and .rdata at 0x6cc000, from address -> bytes and
/// (relocation VA, the address it holds).
const FakePe = struct {
    mem: std.AutoHashMapUnmanaged(u32, u8) = .empty,
    relocs: std.ArrayList(u32) = .empty,

    fn init(relocs: []const [2]u32) !FakePe {
        var p = FakePe{};
        for (relocs) |r| {
            var b: [4]u8 = undefined;
            std.mem.writeInt(u32, &b, r[1], .little);
            try p.put(r[0], &b);
            try p.relocs.append(gpa, r[0]);
        }
        std.mem.sort(u32, p.relocs.items, {}, std.sort.asc(u32));
        return p;
    }
    fn put(p: *FakePe, va: u32, data: Str) !void {
        for (data, 0..) |b, k| try p.mem.put(gpa, va + @as(u32, @intCast(k)), b);
    }
    pub fn read(p: *const FakePe, va: u32, n: usize) !Str {
        const out = try gpa.alloc(u8, n);
        for (out, 0..) |*b, k| b.* = p.mem.get(va + @as(u32, @intCast(k))) orelse 0;
        return out;
    }
    pub fn relocList(p: *const FakePe) []const u32 {
        return p.relocs.items;
    }
    pub fn u32At(p: *const FakePe, va: u32) ?u32 {
        const b = p.read(va, 4) catch return null;
        return std.mem.readInt(u32, b[0..4], .little);
    }
    pub fn textRange(_: *const FakePe) [2]u64 {
        return .{ 0x400000 + 0x1000, 0x400000 + 0x1000 + 0x2cb000 };
    }
    pub fn rdataRange(_: *const FakePe) [2]u64 {
        return .{ 0x400000 + 0x2cc000, 0x400000 + 0x2cc000 + 0x39000 };
    }
    pub fn cstr(p: *const FakePe, va: u32) !?Str {
        const b = try p.read(va, 256);
        return b[0 .. std.mem.indexOfScalar(u8, b, 0) orelse b.len];
    }
    pub fn sectionOf(_: *const FakePe, _: u32) Str {
        return "?";
    }
};

fn testItem(addr: u32, size: i64, name: Str, typ: Str, kind: Str, ns: Str) !*Item {
    const it = try gpa.create(Item);
    it.* = .{ .addr = addr, .size = size, .extent = size, .name = name, .aliases = "", .scope = "", .ns = ns, .owner = "", .typ = typ, .kind = kind, .refs = &.{} };
    return it;
}

fn bset(xs: []const Str) !*BSet {
    const s = try gpa.create(BSet);
    s.* = .empty;
    for (xs) |x| try s.put(gpa, x, {});
    return s;
}

const RO = [_]Span{.{ .lo = 0x6cc000, .hi = 0x705000 }};
const no_cover: std.AutoHashMapUnmanaged(u32, void) = .empty;

fn candidatesPe() !FakePe {
    var p = try FakePe.init(&.{});
    try p.put(0x6cc7a4, "Russian\x00");
    try p.put(0x6cc838, "Diablo 2 EXE\x00\x00\x00\x00");
    try p.put(0x6cc900, "abc\x00");
    try p.put(0x6cc910, "two\x00str\x00");
    try p.put(0x6cca00, "Writable\x00");
    return p;
}

const tt = std.testing;

test "shape: terminated and padded" {
    const a = try stringLiteralShape("Russian\x00");
    try tt.expectEqualStrings("Russian", a.text.?);
    try tt.expect(a.why == null);
    try tt.expectEqualStrings("ab", (try stringLiteralShape("ab\x00\x00")).text.?);
}

test "shape: rejects" {
    try tt.expect((try stringLiteralShape("abcd")).why != null);
    try tt.expect((try stringLiteralShape("a\x00b\x00")).why != null);
    try tt.expect((try stringLiteralShape("ab\x00\x00\x00\x00\x00\x00")).why != null);
    try tt.expect((try stringLiteralShape("\x00\x00\x00\x00")).why != null);
}

test "shape: pooled extent" {
    try tt.expectEqual(@as(i64, 4), pooledExtent(0));
    try tt.expectEqual(@as(i64, 4), pooledExtent(3));
    try tt.expectEqual(@as(i64, 8), pooledExtent(4));
    try tt.expectEqual(@as(i64, 8), pooledExtent(7));
}

test "shape: label" {
    try tt.expectEqualStrings("s_Diablo_2_EXE_006cc838", try stringLabel(0x6cc838, "Diablo 2 EXE"));
}

test "candidates: typed and untyped" {
    const p = try candidatesPe();
    const items = [_]*Item{try testItem(0x6cc7a4, 8, "s_Russian_006cc7a4", "char[8]", "data", "Fog::Src::ErrorManager")};
    const strings = [_]DString{.{ .addr = 0x6cc838, .length = 12, .encoding = "string", .label = "s_Diablo_2_EXE_006cc838" }};
    const lits = try literalCandidates(&items, &strings, &p, &no_cover, &RO, null);
    try tt.expectEqual(@as(usize, 2), lits.len);
    try tt.expectEqual(@as(u32, 0x6cc7a4), lits[0].addr);
    try tt.expectEqualStrings("Russian", lits[0].text);
    try tt.expectEqualStrings("typed", lits[0].source);
    try tt.expectEqual(@as(u32, 0x6cc838), lits[1].addr);
    try tt.expectEqualStrings("Diablo 2 EXE", lits[1].text);
    try tt.expectEqualStrings("untyped", lits[1].source);
}

test "candidates: paired by name is not a literal" {
    const p = try candidatesPe();
    const items = [_]*Item{try testItem(0x6cc7a4, 8, "s_Russian_006cc7a4", "char[8]", "data", "")};
    var cov: std.AutoHashMapUnmanaged(u32, void) = .empty;
    try cov.put(gpa, 0x6cc7a4, {});
    try tt.expectEqual(@as(usize, 0), (try literalCandidates(&items, &.{}, &p, &cov, &RO, null)).len);
}

test "candidates: two strings in one item is a table" {
    const p = try candidatesPe();
    const items = [_]*Item{try testItem(0x6cc910, 8, "gaPair", "char[8]", "data", "")};
    try tt.expectEqual(@as(usize, 0), (try literalCandidates(&items, &.{}, &p, &no_cover, &RO, null)).len);
}

test "candidates: label inside keeps it" {
    const p = try candidatesPe();
    const items = [_]*Item{ try testItem(0x6cc7a4, 8, "s_Russian_006cc7a4", "char[8]", "data", ""), try testItem(0x6cc7a8, 4, "s_ian_006cc7a8", "char[8]", "interior", "") };
    for (try literalCandidates(&items, &.{}, &p, &no_cover, &RO, null)) |l| try tt.expect(l.addr != 0x6cc7a4);
}

test "candidates: outside read-only" {
    const p = try candidatesPe();
    const items = [_]*Item{try testItem(0x6cca00, 9, "gszWritable", "char[9]", "data", "")};
    const ro = [_]Span{.{ .lo = 0x6cc000, .hi = 0x6cca00 }};
    try tt.expectEqual(@as(usize, 0), (try literalCandidates(&items, &.{}, &p, &no_cover, &ro, null)).len);
}

test "candidates: not a string type" {
    const p = try candidatesPe();
    const items = [_]*Item{try testItem(0x6cc900, 4, "gdwMagic", "uint", "data", "")};
    try tt.expectEqual(@as(usize, 0), (try literalCandidates(&items, &.{}, &p, &no_cover, &RO, null)).len);
}

test "candidates: generator report overrides the rule" {
    const p = try candidatesPe();
    const items = [_]*Item{ try testItem(0x6cc7a4, 8, "s_Russian_006cc7a4", "char[8]", "data", ""), try testItem(0x6cc900, 4, "s_abc_006cc900", "char[4]", "data", "") };
    var g: std.AutoHashMapUnmanaged(u32, void) = .empty;
    try g.put(gpa, 0x6cc900, {});
    const lits = try literalCandidates(&items, &.{}, &p, &no_cover, &RO, &g);
    try tt.expectEqual(@as(usize, 1), lits.len);
    try tt.expectEqual(@as(u32, 0x6cc900), lits[0].addr);
}

const nothing: BSet = .empty;

test "judge: match at every use" {
    const sites = [_]Site{ .{ .where = "f", .haves = try bset(&.{ "Russian", "x" }), .theirs = &nothing }, .{ .where = "t+44", .haves = try bset(&.{"Russian"}), .theirs = &nothing } };
    const r = try judgeLiteral("Russian", &sites, &nothing, &.{});
    try tt.expectEqualStrings("MATCH", r.status);
    try tt.expectEqualStrings("use", r.method);
}

test "judge: other content is string" {
    const sites = [_]Site{.{ .where = "f", .haves = try bset(&.{"Windows XQ"}), .theirs = &nothing }};
    const r = try judgeLiteral("Windows XP", &sites, &nothing, &.{});
    try tt.expectEqualStrings("STRING", r.status);
    try tt.expect(contains(r.notes[0], "Windows XQ"));
}

test "judge: one use differs" {
    const sites = [_]Site{ .{ .where = "f", .haves = try bset(&.{"abcdef"}), .theirs = &nothing }, .{ .where = "g", .haves = try bset(&.{"abcdeg"}), .theirs = &nothing } };
    try tt.expectEqualStrings("STRING", (try judgeLiteral("abcdef", &sites, &nothing, &.{})).status);
}

test "judge: the original's other string is not a misspelling" {
    const sites = [_]Site{.{ .where = "f", .haves = try bset(&.{"MacOS X"}), .theirs = try bset(&.{ "MacOS X", "MacOS" }) }};
    try tt.expectEqualStrings("MISSING", (try judgeLiteral("MacOS", &sites, &nothing, &.{})).status);
}

test "judge: missing notes the image" {
    const sites = [_]Site{.{ .where = "f", .haves = try bset(&.{"Diablo II"}), .theirs = try bset(&.{"unknown"}) }};
    const r = try judgeLiteral("unknown", &sites, try bset(&.{"unknown"}), &.{});
    try tt.expectEqualStrings("MISSING", r.status);
    try tt.expect(contains(r.notes[0], "the image holds it elsewhere"));
}

test "judge: object of the use holds it" {
    const sites = [_]Site{.{ .where = "f", .haves = &nothing, .theirs = &nothing }};
    const th = [_]*const BSet{try bset(&.{"Caught"})};
    const r = try judgeLiteral("Caught", &sites, &nothing, &th);
    try tt.expectEqualStrings("MATCH", r.status);
    try tt.expectEqualStrings("tu", r.method);
}

test "judge: no use" {
    try tt.expectEqualStrings("UNUSED", (try judgeLiteral("x", &.{}, try bset(&.{"x"}), &.{})).status);
}

const FakeOurs = struct {
    fns: std.StringHashMapUnmanaged(*BSet) = .empty,
    every: BSet = .empty,
    pub fn ofFunction(f: *const FakeOurs, tu: Str, sym: Str) !?*const BSet {
        return f.fns.get(try fmt("{s}\x00{s}", .{ tu, sym }));
    }
    pub fn byTu(_: *const FakeOurs, _: Str) ?*const BSet {
        return null;
    }
    pub fn everywhereSet(f: *const FakeOurs) *const BSet {
        return &f.every;
    }
};

test "rows: pairing through the original's relocations" {
    literal_fields = .empty;
    var p = try FakePe.init(&.{ .{ 0x401005, 0x6cc7a4 }, .{ 0x402005, 0x6cc838 }, .{ 0x6f0000, 0x6cc900 }, .{ 0x403005, 0x6cc910 } });
    try p.put(0x6cc7a4, "Russian\x00");
    try p.put(0x6cc838, "Diablo II\x00\x00\x00");
    try p.put(0x6cc900, "abc\x00");
    try p.put(0x6cc910, "rt\x00\x00");
    try p.put(0x6cc920, "dead\x00");
    const fm = [_]FmRow{ .{ .s = 0x401000, .e = 0x401100, .name = "Fog::Src::ErrorManager::Write", .tu = "Source/Fog/Src/ErrorManager", .sym = "?Write@" }, .{ .s = 0x402000, .e = 0x402100, .name = "D2Client::Main", .tu = "Source/D2Client/Main", .sym = "?Main@" } };
    const items = [_]*Item{try testItem(0x6cc7a4, 8, "s_Russian_006cc7a4", "char[8]", "data", "Fog::Src::ErrorManager")};
    const strings = [_]DString{ .{ .addr = 0x6cc838, .length = 9, .encoding = "string", .label = "s_Diablo_II_006cc838" }, .{ .addr = 0x6cc900, .length = 3, .encoding = "string", .label = "s_abc_006cc900" }, .{ .addr = 0x6cc910, .length = 2, .encoding = "string", .label = "s_rt_006cc910" }, .{ .addr = 0x6cc920, .length = 4, .encoding = "string", .label = "s_dead_006cc920" } };
    const lits = try literalCandidates(&items, &strings, &p, &no_cover, &RO, null);
    var ours = FakeOurs{};
    try ours.fns.put(gpa, "Source/Fog/Src/ErrorManager\x00?Write@", try bset(&.{"Russian"}));
    try ours.fns.put(gpa, "Source/D2Client/Main\x00?Main@", try bset(&.{"Diablo 2"}));
    var vit = ours.fns.valueIterator();
    while (vit.next()) |s| for (s.*.keys()) |k| try ours.every.put(gpa, k, {});
    const g = try literal_fields.getOrPut(gpa, 0x6cc900);
    g.value_ptr.* = .empty;
    try g.value_ptr.append(gpa, .{ .where = "gaTable+4", .have = "abc" });
    const rows = try literalRows(&p, lits, &fm, &ours);
    const find = struct {
        fn f(rs: []const Row, a: u32) Row {
            for (rs) |r| if (r.addr == a) return r;
            unreachable;
        }
    }.f;
    try tt.expectEqualStrings("MATCH", find(rows, 0x6cc7a4).status); // code use, our function names it
    try tt.expectEqualStrings("Fog", find(rows, 0x6cc7a4).module);
    try tt.expectEqualStrings("STRING", find(rows, 0x6cc838).status); // code use, ours spells another
    try tt.expectEqualStrings("D2Client", find(rows, 0x6cc838).module); // module from the reader
    try tt.expectEqualStrings("MATCH", find(rows, 0x6cc900).status); // data use: the pointer field of ours
    try tt.expectEqualStrings("UNPAIRED", find(rows, 0x6cc910).status); // used only by code the funcmap does not pair
    try tt.expectEqualStrings("UNUSED", find(rows, 0x6cc920).status); // nothing in the original points at it
    try tt.expectEqualStrings("strlit", find(rows, 0x6cc7a4).kind);
    try tt.expectEqual(@as(i64, 8), find(rows, 0x6cc7a4).matched); // matched bytes: the original extent
    literal_fields = .empty;
}

test "python reprs" {
    try tt.expectEqualStrings("0.1", try pyFloatRepr(0.1));
    try tt.expectEqualStrings("1e-05", try pyFloatRepr(1e-5));
    try tt.expectEqualStrings("0.0001", try pyFloatRepr(1e-4));
    try tt.expectEqualStrings("1e+16", try pyFloatRepr(1e16));
    try tt.expectEqualStrings("1000000000000000.0", try pyFloatRepr(1e15));
    try tt.expectEqualStrings("0.10000000149011612", try pyFloatRepr(@as(f32, 0.1)));
    try tt.expectEqualStrings("-2.5", try pyFloatRepr(-2.5));
    try tt.expectEqualStrings("3.0", try pyFloatRepr(3.0));
    try tt.expectEqualStrings("b\"it's\"", try pyBytesRepr("it's"));
    try tt.expectEqualStrings("b'a\\x00\\\\'", try pyBytesRepr("a\x00\\"));
}
