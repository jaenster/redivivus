//! Every function in the compiled objects, with what it refers to (ours.py). Decoding here
//! takes about as long as reading a cached pickle did, so nothing is cached.
const std = @import("std");
const builtin = @import("builtin");
const c = @import("c");
const objfuncs = @import("objfuncs.zig");
const dec = @import("decode.zig");
const util = @import("util.zig");
const proc = @import("proc.zig");

const gpa = util.gpa;

pub const OurFunc = struct {
    tu: []const u8, // 'D2Common/Environment' (object path without .obj)
    order: usize, // position of its COMDAT in the object
    f: objfuncs.Function,

    pub fn key(o: *const OurFunc) ![]const u8 {
        return std.fmt.allocPrint(gpa, "{s}|{s}", .{ o.tu, o.f.symbol });
    }

    /// (source file, line) of the instruction at `off`, or null.
    pub fn lineAt(o: *const OurFunc, off: u32) ?objfuncs.Line {
        var best: ?objfuncs.Line = null;
        for (o.f.lines) |l| {
            if (l.off <= off) best = l else break;
        }
        return best;
    }
};

/// Every function of one object, decoded.
pub fn loadObject(d: *dec.Decoder, objdir: []const u8, rel: []const u8, out: *std.ArrayList(OurFunc)) !void {
    const path = try util.join(objdir, rel);
    const data = try util.readFile(path);
    const tu = rel[0 .. rel.len - 4];
    const fns = try objfuncs.functions(d, path, data, false);
    for (fns, 0..) |f, k| try out.append(gpa, .{ .tu = tu, .order = k, .f = f });
}

fn lessStr(_: void, a: []const u8, b: []const u8) bool {
    return std.mem.order(u8, a, b) == .lt;
}

/// os.walk(objdir) with sorted directories and files: every completed object (an .obj with
/// its .d beside it - build.py writes the .d only on success), as paths relative to objdir.
pub fn completedObjects(objdir: []const u8) ![][]const u8 {
    var out: std.ArrayList([]const u8) = .empty;
    try walk(objdir, "", &out);
    return out.items;
}

fn walk(root: []const u8, rel: []const u8, out: *std.ArrayList([]const u8)) !void {
    const dir_path = if (rel.len == 0) root else try util.join(root, rel);
    var files: std.ArrayList([]const u8) = .empty;
    var dirs: std.ArrayList([]const u8) = .empty;
    if (builtin.os.tag == .windows) {
        for (try proc.listDir(dir_path)) |name| {
            const full = try util.join(dir_path, name);
            if (proc.isDir(full)) {
                if (!proc.isLink(full)) try dirs.append(gpa, name);
            } else try files.append(gpa, name);
        }
    } else {
        const dz = try util.z(dir_path);
        const dp = c.opendir(dz.ptr) orelse return;
        while (c.readdir(dp)) |ent| {
            const name = std.mem.sliceTo(@as([*:0]const u8, @ptrCast(&ent.*.d_name)), 0);
            if (std.mem.eql(u8, name, ".") or std.mem.eql(u8, name, "..")) continue;
            const nm = try gpa.dupe(u8, name);
            var is_dir = ent.*.d_type == c.DT_DIR;
            if (ent.*.d_type == c.DT_UNKNOWN) {
                var st: c.struct_stat = undefined;
                const full = try util.z(try util.join(dir_path, nm));
                is_dir = c.lstat(full.ptr, &st) == 0 and (st.st_mode & c.S_IFMT) == c.S_IFDIR;
            }
            if (is_dir) try dirs.append(gpa, nm) else try files.append(gpa, nm);
        }
        _ = c.closedir(dp);
    }
    std.mem.sort([]const u8, files.items, {}, lessStr);
    std.mem.sort([]const u8, dirs.items, {}, lessStr);
    var have: std.StringHashMapUnmanaged(void) = .empty;
    for (files.items) |f| try have.put(gpa, f, {});
    for (files.items) |f| {
        if (!std.mem.endsWith(u8, f, ".obj")) continue;
        const dname = try std.fmt.allocPrint(gpa, "{s}.d", .{f[0 .. f.len - 4]});
        if (!have.contains(dname)) continue;
        try out.append(gpa, if (rel.len == 0) f else try util.join(rel, f));
    }
    for (dirs.items) |dn| try walk(root, if (rel.len == 0) dn else try util.join(rel, dn), out);
}

/// Every function of every completed object.
pub fn loadAll(d: *dec.Decoder, objdir: []const u8) ![]OurFunc {
    var out: std.ArrayList(OurFunc) = .empty;
    for (try completedObjects(objdir)) |rel| try loadObject(d, objdir, rel, &out);
    return out.items;
}

/// The first line of its own .cpp it was compiled from: where it is defined. Null for code
/// from headers only (inline functions, templates).
pub fn sourceLine(o: *const OurFunc) !?u32 {
    const own = try std.fmt.allocPrint(gpa, "{s}.cpp", .{o.tu});
    for (own) |*ch| ch.* = std.ascii.toLower(ch.*);
    var best: ?u32 = null;
    for (o.f.lines) |l| {
        const fname = try gpa.dupe(u8, l.file);
        for (fname) |*ch| ch.* = if (ch.* == '\\') '/' else std.ascii.toLower(ch.*);
        if (std.mem.endsWith(u8, fname, own)) best = if (best) |b| @min(b, l.line) else l.line;
    }
    return best;
}

/// ours.OurFunc.layout_key: (0, line, order), or (1, 0, order) for no line of its own.
pub const LayoutKey = struct {
    a: u8,
    line: u32,
    order: usize,
    pub fn lessThan(x: LayoutKey, y: LayoutKey) bool {
        if (x.a != y.a) return x.a < y.a;
        if (x.line != y.line) return x.line < y.line;
        return x.order < y.order;
    }
};

pub fn layoutKey(o: *const OurFunc) !LayoutKey {
    return if (try sourceLine(o)) |ln| .{ .a = 0, .line = ln, .order = o.order } else .{ .a = 1, .line = 0, .order = o.order };
}

/// The functions the link keeps (ours.link_kept): of an external COMDAT several objects
/// define, the copy of the first object in link order; a static function always.
pub fn linkKept(fns: []OurFunc, repo: []const u8) ![]OurFunc {
    const modules = @import("modules.zig");
    const ms = try modules.load(repo);
    const tus = try gpa.alloc([]const u8, fns.len);
    for (fns, 0..) |f, i| tus[i] = f.tu;
    var rank: std.StringHashMapUnmanaged(usize) = .empty;
    for (try ms.linkOrder(tus), 0..) |t, i| try rank.put(gpa, t, i);
    var first: std.StringHashMapUnmanaged(usize) = .empty;
    for (fns, 0..) |f, i| {
        if (f.f.static) continue;
        const g = try first.getOrPut(gpa, f.f.symbol);
        if (!g.found_existing or rank.get(f.tu).? < rank.get(fns[g.value_ptr.*].tu).?) g.value_ptr.* = i;
    }
    var out: std.ArrayList(OurFunc) = .empty;
    for (fns, 0..) |f, i| if (f.f.static or first.get(f.f.symbol).? == i) try out.append(gpa, f);
    return out.items;
}
