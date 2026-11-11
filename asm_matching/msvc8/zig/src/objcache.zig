//! objcache.py: the content-addressed cache of the oracle's compiles. The same layout, keys
//! and JSON as the Python (json.dumps' separators and ASCII escapes), so the two share one
//! cache: a compile either stored is a hit for the other.
//!
//!   manifest/<b[:2]>/<b>.json   base key -> [{"deps": [[rel, hash], ...], "key": result key}, ...]
//!   <kind>/<k[:2]>/<k>/         result key -> its files and meta.json
//!
//! ORACLE_CACHE (default $XDG_CACHE_HOME/d2-msvc8-oracle, else ~/.cache/d2-msvc8-oracle;
//! 'off' disables it), ORACLE_NOCACHE=1 (never read, still write).
const std = @import("std");
const c = @import("c");
const util = @import("util.zig");
const proc = @import("proc.zig");
const gpa = util.gpa;

const MANIFEST_KEEP = 16;

pub var root: ?[]const u8 = null;
pub var read_enabled = false;

pub fn init() !void {
    const env = util.getenv("ORACLE_CACHE");
    const r = if (env != null and env.?.len > 0) env.? else blk: {
        const xdg = util.getenv("XDG_CACHE_HOME");
        const base = if (xdg != null and xdg.?.len > 0) xdg.? else try util.join(util.getenv("HOME") orelse "", ".cache");
        break :blk try util.join(base, "d2-msvc8-oracle");
    };
    if (std.mem.eql(u8, r, "off")) {
        root = null;
        read_enabled = false;
        return;
    }
    root = r;
    const nc = util.getenv("ORACLE_NOCACHE") orelse "0";
    read_enabled = !std.mem.eql(u8, nc, "1");
}

pub const Hex = [64]u8;

fn hexOf(d: [32]u8) ![]const u8 {
    const h = std.fmt.bytesToHex(d, .lower);
    return gpa.dupe(u8, &h);
}

/// sha_bytes: every part length-prefixed (8 bytes, little-endian).
pub fn shaBytes(parts: []const []const u8) ![]const u8 {
    var h = std.crypto.hash.sha2.Sha256.init(.{});
    for (parts) |p| {
        var len: [8]u8 = undefined;
        std.mem.writeInt(u64, &len, p.len, .little);
        h.update(&len);
        h.update(p);
    }
    return hexOf(h.finalResult());
}

var memo_lock: proc.Mutex = .{};
var memo: std.StringHashMapUnmanaged([]const u8) = .empty;

pub fn initLocks() void {
    memo_lock.init();
}

/// sha_file: the content hash of a file, or "-" when it does not exist; memoised per
/// (path, inode, size, mtime).
pub fn shaFile(path: []const u8) ![]const u8 {
    const st = proc.stat(path) orelse return "-";
    const k = try std.fmt.allocPrint(gpa, "{s}\x00{d}\x00{d}\x00{d}", .{ path, st.ino, st.size, st.mtime_ns });
    memo_lock.lock();
    const hit = memo.get(k);
    memo_lock.unlock();
    if (hit) |v| return v;
    const data = (try util.readFileOpt(path)) orelse return "-";
    var d: [32]u8 = undefined;
    std.crypto.hash.sha2.Sha256.hash(data, &d, .{});
    const v = try hexOf(d);
    memo_lock.lock();
    defer memo_lock.unlock();
    try memo.put(gpa, k, v);
    return v;
}

fn dir(kind: []const u8, key: []const u8) ![]const u8 {
    return std.fmt.allocPrint(gpa, "{s}/{s}/{s}/{s}", .{ root.?, kind, key[0..2], key });
}

/// The result directory for `key`, or null.
pub fn get(kind: []const u8, key: []const u8) !?[]const u8 {
    if (!read_enabled or root == null) return null;
    const d = try dir(kind, key);
    return if (proc.isFile(try util.join(d, "meta.json"))) d else null;
}

pub fn meta(d: []const u8) !std.json.Value {
    const text = try util.readFile(try util.join(d, "meta.json"));
    return std.json.parseFromSliceLeaky(std.json.Value, gpa, text, .{});
}

pub const File = struct { name: []const u8, path: ?[]const u8 = null, data: ?[]const u8 = null };

/// Store `files` under `key`: written aside and renamed, so a reader sees a whole entry or none.
pub fn put(kind: []const u8, key: []const u8, files: []const File, info_json: []const u8) !void {
    if (root == null) return;
    const d = try dir(kind, key);
    if (proc.isFile(try util.join(d, "meta.json"))) return;
    const parent = util.dirname(d);
    util.makeDirs(parent);
    const tmpl = try std.fmt.allocPrintSentinel(gpa, "{s}/.tmp-XXXXXXXX", .{parent}, 0);
    const tmp = std.mem.sliceTo(c.mkdtemp(tmpl.ptr) orelse return error.Mkdtemp, 0);
    defer rmTree(tmp);
    for (files) |f| {
        const dst = try util.join(tmp, f.name);
        try util.writeFile(dst, f.data orelse try util.readFile(f.path.?));
    }
    try util.writeFile(try util.join(tmp, "meta.json"), info_json);
    proc.rename(tmp, d) catch {}; // another process stored the same key first
}

fn rmTree(path: []const u8) void {
    if (!proc.isDir(path)) return;
    const names = proc.listDir(path) catch return;
    for (names) |n| {
        const p = util.join(path, n) catch continue;
        if (proc.isDir(p)) rmTree(p) else proc.remove(p);
    }
    const pz = util.z(path) catch return;
    _ = c.rmdir(pz.ptr);
}

fn manifestPath(base: []const u8) ![]const u8 {
    return std.fmt.allocPrint(gpa, "{s}/manifest/{s}/{s}.json", .{ root.?, base[0..2], base });
}

/// The result key a manifest entry of `base` names whose dependencies all hash as
/// recorded, or null.
pub fn lookup(base: []const u8, ctx: anytype, comptime hashDep: fn (@TypeOf(ctx), []const u8) anyerror![]const u8) !?[]const u8 {
    if (!read_enabled or root == null) return null;
    const text = (try util.readFileOpt(try manifestPath(base))) orelse return null;
    const v = std.json.parseFromSliceLeaky(std.json.Value, gpa, text, .{}) catch return null;
    if (v != .array) return null;
    for (v.array.items) |e| {
        const deps = e.object.get("deps").?.array.items;
        var all = true;
        for (deps) |pair| {
            const rel = pair.array.items[0].string;
            const h = pair.array.items[1].string;
            if (!std.mem.eql(u8, try hashDep(ctx, rel), h)) {
                all = false;
                break;
            }
        }
        if (all) return e.object.get("key").?.string;
    }
    return null;
}

pub const Dep = struct { rel: []const u8, hash: []const u8 };

/// Add the entry (deps -> key) to the manifest of `base`, newest first.
pub fn record(base: []const u8, deps: []const Dep, key: []const u8) !void {
    if (root == null) return;
    const p = try manifestPath(base);
    util.makeDirs(util.dirname(p));
    const fd = try proc.flockPath(try std.mem.concat(gpa, u8, &.{ p, ".lock" }));
    defer proc.closeFd(fd);
    var old: []std.json.Value = &.{};
    if (try util.readFileOpt(p)) |text| {
        if (std.json.parseFromSliceLeaky(std.json.Value, gpa, text, .{})) |v| {
            if (v == .array) old = v.array.items;
        } else |_| {}
    }
    var out: std.ArrayList(u8) = .empty;
    try out.appendSlice(gpa, "[{\"deps\": ");
    try depsJson(&out, deps);
    try out.appendSlice(gpa, ", \"key\": ");
    try jsonString(&out, key);
    try out.append(gpa, '}');
    var kept: usize = 0;
    for (old) |e| {
        if (kept >= MANIFEST_KEEP - 1) break;
        if (e == .object) if (e.object.get("key")) |k| if (k == .string and std.mem.eql(u8, k.string, key)) continue;
        try out.appendSlice(gpa, ", ");
        try jsonValue(&out, e);
        kept += 1;
    }
    try out.append(gpa, ']');
    try proc.writeAtomic(p, out.items);
}

/// Copy a cached file into place atomically.
pub fn install(src: []const u8, dst: []const u8) !void {
    try proc.copyAtomic(src, dst);
}

// ---- json.dumps, Python's default formatting

/// [[rel, hash], ...]
pub fn depsJson(out: *std.ArrayList(u8), deps: []const Dep) !void {
    try out.append(gpa, '[');
    for (deps, 0..) |d, i| {
        if (i > 0) try out.appendSlice(gpa, ", ");
        try out.append(gpa, '[');
        try jsonString(out, d.rel);
        try out.appendSlice(gpa, ", ");
        try jsonString(out, d.hash);
        try out.append(gpa, ']');
    }
    try out.append(gpa, ']');
}

pub fn stringList(out: *std.ArrayList(u8), items: []const []const u8) !void {
    try out.append(gpa, '[');
    for (items, 0..) |s, i| {
        if (i > 0) try out.appendSlice(gpa, ", ");
        try jsonString(out, s);
    }
    try out.append(gpa, ']');
}

fn hex4(out: *std.ArrayList(u8), v: u32) !void {
    var b: [6]u8 = undefined;
    _ = try std.fmt.bufPrint(&b, "\\u{x:0>4}", .{v});
    try out.appendSlice(gpa, &b);
}

/// A JSON string as json.dumps(ensure_ascii=True) writes it; `s` is UTF-8 (a byte that is
/// not is written as its surrogateescape code point).
pub fn jsonString(out: *std.ArrayList(u8), s: []const u8) !void {
    try out.append(gpa, '"');
    var i: usize = 0;
    while (i < s.len) {
        const ch = s[i];
        if (ch < 0x80) {
            switch (ch) {
                '"' => try out.appendSlice(gpa, "\\\""),
                '\\' => try out.appendSlice(gpa, "\\\\"),
                '\n' => try out.appendSlice(gpa, "\\n"),
                '\r' => try out.appendSlice(gpa, "\\r"),
                '\t' => try out.appendSlice(gpa, "\\t"),
                0x08 => try out.appendSlice(gpa, "\\b"),
                0x0c => try out.appendSlice(gpa, "\\f"),
                else => if (ch < 0x20) try hex4(out, ch) else try out.append(gpa, ch),
            }
            i += 1;
            continue;
        }
        const n = std.unicode.utf8ByteSequenceLength(ch) catch 0;
        const cp: ?u21 = if (n > 0 and i + n <= s.len) std.unicode.utf8Decode(s[i .. i + n]) catch null else null;
        if (cp) |v| {
            if (v >= 0x10000) {
                const u = v - 0x10000;
                try hex4(out, 0xd800 + (u >> 10));
                try hex4(out, 0xdc00 + (u & 0x3ff));
            } else try hex4(out, v);
            i += n;
        } else {
            try hex4(out, 0xdc00 + @as(u32, ch));
            i += 1;
        }
    }
    try out.append(gpa, '"');
}

/// A parsed value back as json.dumps writes it (objects keep their order).
pub fn jsonValue(out: *std.ArrayList(u8), v: std.json.Value) !void {
    switch (v) {
        .null => try out.appendSlice(gpa, "null"),
        .bool => |b| try out.appendSlice(gpa, if (b) "true" else "false"),
        .integer => |n| try out.print(gpa, "{d}", .{n}),
        .float => |f| try out.print(gpa, "{d}", .{f}),
        .number_string => |s| try out.appendSlice(gpa, s),
        .string => |s| try jsonString(out, s),
        .array => |a| {
            try out.append(gpa, '[');
            for (a.items, 0..) |x, i| {
                if (i > 0) try out.appendSlice(gpa, ", ");
                try jsonValue(out, x);
            }
            try out.append(gpa, ']');
        },
        .object => |o| {
            try out.append(gpa, '{');
            var it = o.iterator();
            var i: usize = 0;
            while (it.next()) |e| : (i += 1) {
                if (i > 0) try out.appendSlice(gpa, ", ");
                try jsonString(out, e.key_ptr.*);
                try out.appendSlice(gpa, ": ");
                try jsonValue(out, e.value_ptr.*);
            }
            try out.append(gpa, '}');
        },
    }
}
