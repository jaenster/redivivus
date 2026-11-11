//! The toolchain from Microsoft's own packages, laid out with cabextract alone (no installer):
//!
//!   oracle-zig msi-paths [--cabs] <package.msi>      File-table key -> install path (msi_paths.py)
//!   oracle-zig vc8-layout <paths.tsv> <keyed-dir> <dest>    the VC8 tree (vc8_layout.py)
//!   oracle-zig psdk-layout <psdk.tsv> <keyed-dir> <dest> <prefix>
//!                                                    the Platform SDK tree (setup_psdk.sh)
//!
//! Cabinet members in a Windows Installer package are named by File-table key, not by file
//! name; File -> Component -> Directory gives the path the installer would have written.
//! msi-paths needs msiinfo (msitools). Paths use each directory's long target name
//! (DefaultDir is "target[:source]", each part "short|long"); "." folds into the parent.
//! The tables are read as Latin-1 and printed as UTF-8, and the layouts read them back as
//! Latin-1, as the Python did: a path names the same file either way.
const std = @import("std");
const cli = @import("cli.zig");
const proc = @import("proc.zig");
const util = @import("util.zig");

const gpa = util.gpa;

// ---- msi-paths

/// The rows of one table of `msi` (msiinfo export): column names, types and the table name
/// are dropped.
fn table(msi: []const u8, name: []const u8) ![][]const []const u8 {
    const p = try proc.run(&.{ "msiinfo", "export", msi, name }, .{});
    const out = try proc.universalNewlines(p.out);
    var rows: std.ArrayList([]const []const u8) = .empty;
    var it = std.mem.splitScalar(u8, out, '\n');
    while (it.next()) |line| if (line.len > 0) try rows.append(gpa, try util.split(line, '\t'));
    return if (rows.items.len > 3) rows.items[3..] else &.{};
}

fn longTarget(default_dir: []const u8) []const u8 {
    const tgt = default_dir[0 .. std.mem.indexOfScalar(u8, default_dir, ':') orelse default_dir.len];
    const bar = std.mem.lastIndexOfScalar(u8, tgt, '|') orelse return tgt;
    return tgt[bar + 1 ..];
}

fn isDigits(s: []const u8) bool {
    if (s.len == 0) return false;
    for (s) |ch| if (!std.ascii.isDigit(ch)) return false;
    return true;
}

const Dir = struct { parent: []const u8, default: []const u8 };

const Paths = struct {
    dirs: std.StringHashMapUnmanaged(Dir) = .empty,
    cache: std.StringHashMapUnmanaged([]const u8) = .empty,

    fn path(x: *Paths, d: []const u8) ![]const u8 {
        if (x.cache.get(d)) |p| return p;
        const e = x.dirs.get(d) orelse Dir{ .parent = "", .default = d };
        const name = longTarget(e.default);
        var p: []const u8 = undefined;
        if (e.parent.len == 0 or std.mem.eql(u8, e.parent, d) or std.mem.eql(u8, d, "TARGETDIR")) {
            p = if (std.mem.eql(u8, name, "SourceDir") or std.mem.eql(u8, name, ".") or std.mem.eql(u8, name, "SOURCEDIR")) "" else name;
        } else {
            const base = try x.path(e.parent);
            p = if (std.mem.eql(u8, name, ".")) base else if (base.len > 0) try std.fmt.allocPrint(gpa, "{s}/{s}", .{ base, name }) else name;
        }
        try x.cache.put(gpa, d, p);
        return p;
    }
};

const MSI_USAGE =
    \\msi-paths - map an MSI's File-table keys to install paths.
    \\
    \\    oracle-zig msi-paths <package.msi>          prints "<file key>\t<relative path>" lines
    \\    oracle-zig msi-paths --cabs <package.msi>   adds a third column: the cabinet holding
    \\                                                the file (a leading '#' means a stream
    \\                                                inside the .msi itself)
    \\
;

pub fn msiPathsMain(args_in: []const []const u8) !void {
    var args = args_in;
    const cabs = args.len > 0 and std.mem.eql(u8, args[0], "--cabs");
    if (cabs) args = args[1..];
    if (args.len != 1) {
        cli.sayErr(MSI_USAGE);
        std.process.exit(1);
    }
    const msi = args[0];
    var x = Paths{};
    for (try table(msi, "Directory")) |r| if (r.len >= 3) try x.dirs.put(gpa, r[0], .{ .parent = r[1], .default = r[2] });
    var comps: std.StringHashMapUnmanaged([]const u8) = .empty;
    for (try table(msi, "Component")) |r| if (r.len >= 3) try comps.put(gpa, r[0], r[2]);
    // Media rows: a file belongs to the first medium whose LastSequence >= its Sequence.
    const Medium = struct { last: u64, cab: []const u8 };
    var media: std.ArrayList(Medium) = .empty;
    for (try table(msi, "Media")) |r| if (r.len >= 4 and isDigits(r[1])) try media.append(gpa, .{ .last = try std.fmt.parseInt(u64, r[1], 10), .cab = r[3] });
    std.mem.sort(Medium, media.items, {}, struct {
        fn lt(_: void, a: Medium, b: Medium) bool {
            if (a.last != b.last) return a.last < b.last;
            return std.mem.order(u8, a.cab, b.cab) == .lt;
        }
    }.lt);
    var out: std.ArrayList(u8) = .empty;
    for (try table(msi, "File")) |r| {
        if (r.len < 8) continue;
        const d = comps.get(r[1]) orelse continue;
        const base = try x.path(d);
        const fname = r[2][if (std.mem.lastIndexOfScalar(u8, r[2], '|')) |b| b + 1 else 0..];
        try out.print(gpa, "{s}\t{s}{s}{s}", .{ r[0], base, if (base.len > 0) "/" else "", fname });
        if (cabs) {
            const seq: u64 = if (isDigits(r[7])) try std.fmt.parseInt(u64, r[7], 10) else 0;
            var cab: []const u8 = "";
            for (media.items) |m| if (seq <= m.last) {
                cab = m.cab;
                break;
            };
            try out.print(gpa, "\t{s}", .{cab});
        }
        try out.append(gpa, '\n');
    }
    util.say(try proc.latin1ToUtf8(out.items));
}

// ---- vc8-layout

const VS = "Program Files/Microsoft Visual Studio 8/";
const PDB_FAMILY = [_][]const u8{ "mspdb80.dll", "msobj80.dll", "mspdbcore.dll", "mspdbsrv.exe" };

fn target(path: []const u8) !?[]const u8 {
    const bin = VS ++ "VC/bin/";
    const inc = VS ++ "VC/include/";
    const lib = VS ++ "VC/lib/";
    const ide = VS ++ "Common7/IDE/";
    if (std.mem.startsWith(u8, path, bin)) {
        const rest = path[bin.len..];
        if (std.mem.indexOfScalar(u8, rest, '/') == null or std.mem.startsWith(u8, rest, "1033/")) return try std.fmt.allocPrint(gpa, "bin/{s}", .{rest});
    } else if (std.mem.startsWith(u8, path, inc)) {
        return try std.fmt.allocPrint(gpa, "include/{s}", .{path[inc.len..]});
    } else if (std.mem.startsWith(u8, path, lib)) {
        const rest = path[lib.len..];
        if (std.mem.indexOfScalar(u8, rest, '/') == null) return try std.fmt.allocPrint(gpa, "lib/{s}", .{rest});
    } else if (std.mem.startsWith(u8, path, ide)) {
        const base = path[std.mem.lastIndexOfScalar(u8, path, '/').? + 1 ..];
        for (PDB_FAMILY) |f| if (std.mem.eql(u8, base, f)) return try std.fmt.allocPrint(gpa, "bin/{s}", .{base});
    }
    return null;
}

fn copyFile(src: []const u8, dst: []const u8) !void {
    util.makeDirs(util.dirname(dst));
    try util.writeFile(dst, try util.readFile(src));
}

const LAYOUT_USAGE =
    \\vc8-layout - lay out a VC8 toolchain from installer cabinet members.
    \\
    \\    oracle-zig vc8-layout <paths.tsv> <keyed-dir> <dest>
    \\
    \\<paths.tsv> is msi-paths output (file key -> install path) and <keyed-dir> holds
    \\cabinet members named by file key (cabextract output). Compilers, headers, libraries
    \\and PDB support go to <dest>/bin, include, lib; members missing from <keyed-dir> are
    \\skipped, so the same run over the SP1 cabinet overlays only what SP1 replaces.
    \\
;

pub fn vc8LayoutMain(args: []const []const u8) !void {
    if (args.len != 3) {
        cli.sayErr(LAYOUT_USAGE);
        std.process.exit(1);
    }
    const keyed = args[1];
    const dest = args[2];
    var placed: usize = 0;
    var it = util.Lines{ .data = try proc.latin1ToUtf8(try util.readFile(args[0])) };
    while (it.next()) |line| {
        const p = try util.split(line, '\t');
        if (p.len != 2) return error.ValueError;
        const t = try target(p[1]) orelse continue;
        const src = try util.join(keyed, p[0]);
        if (!proc.isFile(src)) continue;
        try copyFile(src, try util.join(dest, t));
        placed += 1;
    }
    util.say(try std.fmt.allocPrint(gpa, "{s}: {d} files\n", .{ dest, placed }));
}

// ---- psdk-layout

const PSDK_USAGE =
    \\psdk-layout - lay out the Platform SDK from the VS2005 Professional cabinets.
    \\
    \\    oracle-zig psdk-layout <psdk.tsv> <keyed-dir> <dest> <prefix>
    \\
    \\<psdk.tsv> is msi-paths --cabs output narrowed to <prefix>Include/ and <prefix>Lib/;
    \\each member goes to <dest>/<include|lib, lower-cased>/<the rest of its path>.
    \\
;

pub fn psdkLayoutMain(args: []const []const u8) !void {
    if (args.len != 4) {
        cli.sayErr(PSDK_USAGE);
        std.process.exit(1);
    }
    const keyed = args[1];
    const dest = args[2];
    const prefix = args[3];
    var n: usize = 0;
    var it = util.Lines{ .data = try proc.latin1ToUtf8(try util.readFile(args[0])) };
    while (it.next()) |line| {
        const p = try util.split(line, '\t');
        if (p.len != 3) return error.ValueError;
        const rest = p[1][@min(prefix.len, p[1].len)..];
        const slash = std.mem.indexOfScalar(u8, rest, '/') orelse return error.ValueError;
        const sub = try std.ascii.allocLowerString(gpa, rest[0..slash]);
        const out = try std.fmt.allocPrint(gpa, "{s}/{s}/{s}", .{ dest, sub, rest[slash + 1 ..] });
        const src = try util.join(keyed, p[0]);
        if (!proc.isFile(src)) cli.exitMsg("missing cabinet member {s} for {s}", .{ p[0], p[1] });
        try copyFile(src, out);
        n += 1;
    }
    util.say(try std.fmt.allocPrint(gpa, "{s}: {d} files\n", .{ dest, n }));
}
