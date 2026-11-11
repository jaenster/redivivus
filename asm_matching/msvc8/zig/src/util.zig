//! Shared plumbing: one process-wide arena, files through libc stdio, tab-separated lines,
//! and the printf formats Python's %-formatting needs (%.3f rounds as C does, as Python does).
const std = @import("std");
const c = @import("c");

var arena_state = std.heap.ArenaAllocator.init(std.heap.c_allocator);
/// Everything lives until the process exits: a run is one pass over its inputs.
pub const gpa = arena_state.allocator();

pub fn z(s: []const u8) ![:0]u8 {
    return gpa.dupeSentinel(u8, s, 0);
}

/// The whole file, or null when it does not exist (os.path.exists is false).
pub fn readFileOpt(path: []const u8) !?[]u8 {
    const pz = try z(path);
    const f = c.fopen(pz.ptr, "rb") orelse return null;
    defer _ = c.fclose(f);
    _ = c.fseek(f, 0, c.SEEK_END);
    const n: usize = @intCast(c.ftell(f));
    _ = c.fseek(f, 0, c.SEEK_SET);
    const buf = try gpa.alloc(u8, n);
    if (n > 0 and c.fread(buf.ptr, 1, n, f) != n) return error.Read;
    return buf;
}

pub fn readFile(path: []const u8) ![]u8 {
    return (try readFileOpt(path)) orelse {
        std.debug.print("cannot open {s}\n", .{path});
        return error.Open;
    };
}

pub fn exists(path: []const u8) bool {
    const pz = z(path) catch return false;
    var st: c.struct_stat = undefined;
    return c.stat(pz.ptr, &st) == 0;
}

pub fn writeFile(path: []const u8, data: []const u8) !void {
    const pz = try z(path);
    const f = c.fopen(pz.ptr, "wb") orelse {
        std.debug.print("cannot write {s}\n", .{path});
        return error.OpenOut;
    };
    defer _ = c.fclose(f);
    if (data.len > 0 and c.fwrite(data.ptr, 1, data.len, f) != data.len) return error.Write;
}

/// mkdir -p.
pub fn makeDirs(path: []const u8) void {
    var i: usize = 1;
    while (i <= path.len) : (i += 1) {
        if (i == path.len or path[i] == '/') {
            const pz = z(path[0..i]) catch return;
            _ = c.mkdir(pz.ptr, 0o777);
        }
    }
}

pub fn dirname(p: []const u8) []const u8 {
    const i = std.mem.lastIndexOfScalar(u8, p, '/') orelse return ".";
    return if (i == 0) "/" else p[0..i];
}

pub fn basename(p: []const u8) []const u8 {
    const i = std.mem.lastIndexOfScalar(u8, p, '/') orelse return p;
    return p[i + 1 ..];
}

pub fn join(a: []const u8, b: []const u8) ![]u8 {
    return std.fmt.allocPrint(gpa, "{s}/{s}", .{ a, b });
}

pub fn getenv(name: [:0]const u8) ?[]const u8 {
    const v = c.getenv(name.ptr) orelse return null;
    return std.mem.sliceTo(v, 0);
}

/// Lines of a text file without their '\n' (Python's `for line in open(...)` + rstrip('\n')).
pub const Lines = struct {
    data: []const u8,
    pos: usize = 0,
    pub fn next(it: *Lines) ?[]const u8 {
        if (it.pos >= it.data.len) return null;
        const rest = it.data[it.pos..];
        const n = std.mem.indexOfScalar(u8, rest, '\n') orelse rest.len;
        it.pos += @min(rest.len, n + 1);
        return rest[0..n];
    }
};

pub fn split(line: []const u8, sep: u8) ![][]const u8 {
    var out: std.ArrayList([]const u8) = .empty;
    var it = std.mem.splitScalar(u8, line, sep);
    while (it.next()) |p| try out.append(gpa, p);
    return out.items;
}

pub fn hex(s: []const u8) !u32 {
    return std.fmt.parseInt(u32, s, 16);
}

/// Python's '%.Nf' (and '%W.Nf'): C's printf rounds the exact binary value correctly, as Python does.
pub fn fixed(out: *std.ArrayList(u8), comptime cfmt: [:0]const u8, v: f64) !void {
    var buf: [64]u8 = undefined;
    const n = c.snprintf(&buf, buf.len, cfmt.ptr, v);
    try out.appendSlice(gpa, buf[0..@intCast(n)]);
}

/// Python str.strip() for ASCII whitespace.
pub fn strip(s: []const u8) []const u8 {
    return std.mem.trim(u8, s, " \t\n\r\x0b\x0c");
}

/// To stdout (Python's print).
pub fn say(s: []const u8) void {
    var p = s;
    while (p.len > 0) {
        const n = c.write(1, p.ptr, p.len);
        if (n <= 0) return;
        p = p[@intCast(n)..];
    }
}
