//! Whole-stage reuse (ours.stage_key/stage_reuse/stage_done): a derivation whose inputs,
//! this executable among them, hash to the key its last run recorded, and whose outputs are
//! still the ones that run wrote, need not run again. <build>/<name>.zig-stage holds the key
//! and the outputs' hashes. ORACLE_NOCACHE=1 reads nothing (and still records).
const std = @import("std");
const builtin = @import("builtin");
const c = @import("c");
const util = @import("util.zig");

const gpa = util.gpa;
const Blake3 = std.crypto.hash.Blake3;

extern fn _NSGetExecutablePath(buf: [*]u8, size: *u32) c_int;

/// This executable's own path: a rebuilt oracle-zig may derive other results.
fn selfPath() ![]const u8 {
    var buf: [4096]u8 = undefined;
    if (builtin.os.tag == .macos) {
        var n: u32 = buf.len;
        if (_NSGetExecutablePath(&buf, &n) != 0) return error.SelfPath;
        return gpa.dupe(u8, std.mem.sliceTo(&buf, 0));
    }
    const n = c.readlink("/proc/self/exe", &buf, buf.len);
    if (n <= 0) return error.SelfPath;
    return gpa.dupe(u8, buf[0..@intCast(n)]);
}

/// The hash of a file's bytes, streamed; dashes for a file that does not exist.
pub fn fileHash(path: []const u8) ![64]u8 {
    const pz = try util.z(path);
    const f = c.fopen(pz.ptr, "rb") orelse return @as([64]u8, @splat('-'));
    defer _ = c.fclose(f);
    var h = Blake3.init(.{});
    var buf: [1 << 16]u8 = undefined;
    while (true) {
        const n = c.fread(&buf, 1, buf.len, f);
        if (n == 0) break;
        h.update(buf[0..n]);
    }
    var out: [32]u8 = undefined;
    h.final(&out);
    return std.fmt.bytesToHex(out, .lower);
}

pub const Key = struct {
    h: Blake3 = Blake3.init(.{}),

    pub fn init(name: []const u8) !Key {
        var k = Key{};
        k.str(name);
        try k.file(try selfPath());
        return k;
    }

    pub fn str(k: *Key, s: []const u8) void {
        var n: [8]u8 = undefined;
        std.mem.writeInt(u64, &n, s.len, .little);
        k.h.update(&n);
        k.h.update(s);
    }

    /// A file by path and contents (absent files hash as absent).
    pub fn file(k: *Key, path: []const u8) !void {
        k.str(path);
        k.str(&(try fileHash(path)));
    }

    pub fn hex(k: *Key) [64]u8 {
        var out: [32]u8 = undefined;
        k.h.final(&out);
        return std.fmt.bytesToHex(out, .lower);
    }
};

fn recordPath(build: []const u8, name: []const u8) ![]const u8 {
    return std.fmt.allocPrint(gpa, "{s}/{s}.zig-stage", .{ build, name });
}

/// True if `outputs` already hold the result for `key`.
pub fn reuse(build: []const u8, name: []const u8, key: []const u8, outputs: []const []const u8) !bool {
    if (util.getenv("ORACLE_NOCACHE")) |v| if (std.mem.eql(u8, v, "1")) return false;
    const rec = (try util.readFileOpt(try recordPath(build, name))) orelse return false;
    var it = util.Lines{ .data = rec };
    const first = it.next() orelse return false;
    if (!std.mem.eql(u8, first, key)) return false;
    var n: usize = 0;
    while (it.next()) |line| {
        const tab = std.mem.indexOfScalar(u8, line, '\t') orelse return false;
        if (n >= outputs.len or !std.mem.eql(u8, line[0..tab], outputs[n])) return false;
        if (!std.mem.eql(u8, line[tab + 1 ..], &(try fileHash(outputs[n])))) return false;
        n += 1;
    }
    return n == outputs.len;
}

pub fn done(build: []const u8, name: []const u8, key: []const u8, outputs: []const []const u8) !void {
    var o: std.ArrayList(u8) = .empty;
    try o.appendSlice(gpa, key);
    try o.append(gpa, '\n');
    for (outputs) |p| try o.print(gpa, "{s}\t{s}\n", .{ p, &(try fileHash(p)) });
    util.makeDirs(build);
    const path = try recordPath(build, name);
    const tmp = try std.fmt.allocPrint(gpa, "{s}.{d}.tmp", .{ path, c.getpid() });
    try util.writeFile(tmp, o.items);
    _ = c.rename((try util.z(tmp)).ptr, (try util.z(path)).ptr);
}
