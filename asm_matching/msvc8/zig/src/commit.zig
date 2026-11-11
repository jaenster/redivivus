//! commit (commit.py): commit the way the oracle branch keeps its history.
//!
//!   oracle-zig commit [--base main] [--tooling LINE] [-m MESSAGE]
//!
//! The oracle branch is one commit holding all of the ASM-matching tooling - asm_matching/,
//! CLAUDE.md, the CMakeLists.txt line that keeps the oracle out of the game's build -
//! followed by commits that change the Diablo II source. So the working tree's changes are
//! split: changes to the tooling paths are amended into the branch's first commit (the
//! first after BASE; --tooling LINE adds `- LINE` to its message); the later commits are
//! re-created on top unchanged, except that the tooling paths now come from the new first
//! commit; the remaining changes, the source, become a new commit with MESSAGE (required if
//! there are any). Only refs and indexes are written, never the working tree; the branch
//! moves to the new tip, and the old one stays in the reflog. Nothing is pushed.
const std = @import("std");
const c = @import("c");
const cli = @import("cli.zig");
const proc = @import("proc.zig");
const util = @import("util.zig");

const gpa = util.gpa;

const TOOLING = [_][]const u8{ "asm_matching", "CLAUDE.md", "CMakeLists.txt" };

const USAGE =
    \\usage: oracle commit [-h] [--base BASE] [--tooling TOOLING] [-m MESSAGE]
    \\
    \\Tooling changes are amended into the branch's first commit after BASE (default main),
    \\the later commits re-created on top, the rest of the working tree's changes committed
    \\as MESSAGE. Only refs and indexes are written; nothing is pushed.
    \\
;

var tmpdir: []const u8 = "";
var tmp_n: usize = 0;

/// git <args>: stdout stripped; a failure ends the run with git's message.
fn git(args: []const []const u8, index: ?[]const u8, input: ?[]const u8) ![]const u8 {
    var argv: std.ArrayList([]const u8) = .empty;
    try argv.append(gpa, "git");
    try argv.appendSlice(gpa, args);
    var stdin_path: ?[]const u8 = null;
    if (input) |text| {
        tmp_n += 1;
        stdin_path = try std.fmt.allocPrint(gpa, "{s}/input{d}", .{ tmpdir, tmp_n });
        try util.writeFile(stdin_path.?, text);
    }
    const env: []const []const u8 = if (index) |i| &.{try std.fmt.allocPrint(gpa, "GIT_INDEX_FILE={s}", .{i})} else &.{};
    const p = try proc.run(argv.items, .{ .env_set = env, .stdin_path = stdin_path });
    if (p.status != 0) {
        var a: std.ArrayList(u8) = .empty;
        for (args, 0..) |x, k| try a.print(gpa, "{s}{s}", .{ if (k == 0) "" else " ", x });
        cleanup();
        cli.exitMsg("git {s}: {s}", .{ a.items, util.strip(p.err) });
    }
    return util.strip(p.out);
}

fn cleanup() void {
    if (tmpdir.len == 0) return;
    _ = proc.run(&.{ "rm", "-rf", tmpdir }, .{}) catch {};
}

fn cat(parts: []const []const u8) ![]const []const u8 {
    return gpa.dupe([]const u8, parts);
}

pub fn main(args: []const []const u8) !void {
    const p = cli.parse(args, &.{ .{ .long = "--base" }, .{ .long = "--tooling", .many = true }, .{ .long = "--message", .short = "-m" } }, USAGE);
    if (p.pos.len > 0) {
        cli.sayErr(USAGE);
        std.process.exit(2);
    }
    const base = p.get("--base") orelse "main";
    const tooling_lines = p.all("--tooling");
    const message = p.get("--message");
    var tmpl = "/tmp/oracle-commit.XXXXXX".*;
    if (util.getenv("TMPDIR")) |t| if (t.len > 0) {
        const s = try std.fmt.allocPrintSentinel(gpa, "{s}/oracle-commit.XXXXXX", .{std.mem.trimEnd(u8, t, "/")}, 0);
        tmpdir = std.mem.sliceTo(c.mkdtemp(s.ptr) orelse return error.TempDir, 0);
    };
    if (tmpdir.len == 0) tmpdir = std.mem.sliceTo(c.mkdtemp(&tmpl) orelse return error.TempDir, 0);
    defer cleanup();

    const top = try git(&.{ "rev-parse", "--show-toplevel" }, null, null);
    if (c.chdir((try util.z(top)).ptr) != 0) return error.Chdir;
    const head = try git(&.{ "rev-parse", "HEAD" }, null, null);
    const listed = try git(&.{ "rev-list", "--reverse", try std.fmt.allocPrint(gpa, "{s}..HEAD", .{base}) }, null, null);
    var commits: std.ArrayList([]const u8) = .empty;
    var it = std.mem.tokenizeAny(u8, listed, " \t\n\r\x0b\x0c");
    while (it.next()) |x| try commits.append(gpa, x);
    if (commits.items.len == 0) {
        cleanup();
        cli.exitMsg("no commits after {s}", .{base});
    }
    const first = commits.items[0];
    const later = commits.items[1..];
    const index = try util.join(tmpdir, "index");

    // The first commit, with the working tree's tooling paths.
    _ = try git(&.{ "read-tree", first }, index, null);
    _ = try git(try std.mem.concat(gpa, []const u8, &.{ &.{ "add", "-A", "--" }, &TOOLING }), index, null);
    var tree = try git(&.{"write-tree"}, index, null);
    var msg = try git(&.{ "log", "-1", "--format=%B", first }, null, null);
    if (tooling_lines.len > 0) {
        var m: std.ArrayList(u8) = .empty;
        try m.appendSlice(gpa, std.mem.trimEnd(u8, msg, "\n"));
        try m.append(gpa, '\n');
        for (tooling_lines) |l| try m.print(gpa, "- {s}\n", .{l});
        msg = m.items;
    }
    var tip = try git(&.{ "commit-tree", tree, "-p", try git(&.{ "rev-parse", try std.fmt.allocPrint(gpa, "{s}^", .{first}) }, null, null), "-F", "-" }, null, msg);
    const tooling = try git(try std.mem.concat(gpa, []const u8, &.{ &.{ "ls-tree", "-r", tip, "--" }, &TOOLING }), null, null);
    // The later commits, re-parented, their tooling paths from the new first commit.
    for (later) |cm| {
        _ = try git(&.{ "read-tree", cm }, index, null);
        _ = try git(try std.mem.concat(gpa, []const u8, &.{ &.{ "rm", "-r", "--cached", "--quiet", "--ignore-unmatch", "--" }, &TOOLING }), index, null);
        _ = try git(&.{ "update-index", "--index-info" }, index, try std.fmt.allocPrint(gpa, "{s}\n", .{tooling}));
        tip = try git(&.{ "commit-tree", try git(&.{"write-tree"}, index, null), "-p", tip, "-F", "-" }, null, try git(&.{ "log", "-1", "--format=%B", cm }, null, null));
    }
    // What is left in the working tree is source: a new commit.
    _ = try git(&.{ "read-tree", tip }, index, null);
    _ = try git(&.{ "add", "-A", "." }, index, null);
    tree = try git(&.{"write-tree"}, index, null);
    if (!std.mem.eql(u8, tree, try git(&.{ "rev-parse", try std.fmt.allocPrint(gpa, "{s}^{{tree}}", .{tip}) }, null, null))) {
        const m = message orelse {
            cleanup();
            cli.exitMsg("source changes in the working tree: give -m MESSAGE", .{});
        };
        tip = try git(&.{ "commit-tree", tree, "-p", tip, "-F", "-" }, null, m);
    }
    _ = try git(&.{ "update-ref", "-m", "commit.py", "HEAD", tip, head }, null, null);
    _ = try git(&.{ "read-tree", "HEAD" }, null, null);
    util.say(try std.fmt.allocPrint(gpa, "{s}\n", .{try git(&.{ "log", "--oneline", try std.fmt.allocPrint(gpa, "{s}..HEAD", .{base}) }, null, null)}));
}
