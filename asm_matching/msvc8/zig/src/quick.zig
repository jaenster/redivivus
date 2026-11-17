//! quick (quick.py): the whole measurement after a tree change, doing only what changed.
//!
//!   oracle-zig quick [--root R] [-j N] [--verify] [--exe-map FILE]
//!
//! Runs build, map, libmap and audit, each incrementally: build compiles only TUs no stored
//! compile matches (`oracle build`), map and libmap
//! skip the derivation when nothing they read changed (stage.zig), and the audit judges
//! every row again. The global parts - the funcmap, the libmap, the datamap the audit
//! writes - are always derived from all functions, so the result is the full run's.
//!
//! Prints the audit summary, the time of each step, and the functions that gained or lost
//! MATCH against the audit the run replaced (kept as build/audit.prev.tsv).
//!
//! --verify runs the incremental measurement, then the full one (ORACLE_NOCACHE=1: every TU
//! compiled, every object decoded, every function judged), and diffs the two runs' funcmap,
//! libmap, audit and datamap. Exit status 1 if they differ.
const std = @import("std");
const builtin = @import("builtin");
const audit = @import("audit.zig");
const cli = @import("cli.zig");
const proc = @import("proc.zig");
const refimage = @import("refimage.zig");
const util = @import("util.zig");

const gpa = util.gpa;

const OUTPUTS = [_][]const u8{ "funcmap.tsv", "funcmap.tsv.unmapped", "libmap.tsv", "libmap.tsv.summary", "audit.tsv", "audit.tsv.summary", "datamap.tsv" };

const USAGE =
    \\usage: oracle quick [-h] [-j JOBS] [--verify] [--exe-map EXE_MAP]
    \\
    \\build, map, libmap and audit, each doing only what changed; prints the audit
    \\summary and the functions that gained or lost MATCH. --verify: then the full run
    \\too (ORACLE_NOCACHE=1), and diff the two.
    \\
;

const Run = struct { rc: i32, timed_out: bool, text: []const u8, secs: f64 };

fn run(argv: []const []const u8, env: []const []const u8) !Run {
    const t0 = proc.now();
    const p = try proc.run(argv, .{ .merge_stderr = true, .env_set = env });
    return .{ .rc = p.status, .timed_out = p.timed_out, .text = try proc.universalNewlines(p.out), .secs = proc.now() - t0 };
}

fn secs(out: *cli.Out, comptime fmt: [:0]const u8, v: f64) !void {
    try util.fixed(&out.buf, fmt, v);
}

/// build, map, libmap, audit into directory `dir` (evidence/ for the real run).
fn pipeline(out: *cli.Out, root: []const u8, dir: []const u8, jobs: []const u8, exe_map: ?[]const u8, env: []const []const u8, force: bool) !void {
    const self = try cli.selfExe();
    const fm = try util.join(dir, "funcmap.tsv");
    const lm = try util.join(dir, "libmap.tsv");
    const au = try util.join(dir, "audit.tsv");
    var build: std.ArrayList([]const u8) = .empty;
    if (builtin.os.tag == .windows) {
        try build.appendSlice(gpa, &.{ self, "build", "--root", root, "-j", jobs });
    } else {
        try build.appendSlice(gpa, &.{ try util.join(root, "oracle"), "build", "-j", jobs });
    }
    if (force) try build.append(gpa, "--force");
    var libmap: std.ArrayList([]const u8) = .empty;
    try libmap.appendSlice(gpa, &.{ self, "libmap", "--root", root, "--funcmap", fm, "--out", lm });
    if (exe_map) |e| try libmap.appendSlice(gpa, &.{ "--exe-map", e });
    const Step = struct { name: []const u8, argv: []const []const u8 };
    const steps = [_]Step{
        .{ .name = "build", .argv = build.items },
        .{ .name = "map", .argv = &.{ self, "map", "--root", root, "--out", fm } },
        .{ .name = "libmap", .argv = libmap.items },
        .{ .name = "audit", .argv = &.{ self, "audit", "--root", root, "--funcmap", fm, "--libmap", lm, "--out", au } },
    };
    for (steps) |st| {
        const r = try run(st.argv, env);
        const lines = try proc.splitlines(r.text);
        if (std.mem.eql(u8, st.name, "build")) {
            try out.raw("build:  ");
            try secs(out, "%5.1f", r.secs);
            try out.raw("s  ");
            var first = true;
            for (lines) |l| {
                if (std.mem.startsWith(u8, l, "translate") or std.mem.startsWith(u8, l, "support") or std.mem.startsWith(u8, l, "compiled") or std.mem.indexOf(u8, l, " TUs: ") != null) {
                    if (!first) try out.raw(" | ");
                    try out.raw(l);
                    first = false;
                }
            }
            try out.raw("\n");
        } else if (!std.mem.eql(u8, st.name, "audit")) {
            try out.print("{s:<7} ", .{try std.fmt.allocPrint(gpa, "{s}:", .{st.name})});
            try secs(out, "%5.1f", r.secs);
            try out.print("s  {s}\n", .{if (lines.len > 0) lines[lines.len - 1] else ""});
        } else {
            try out.raw("audit:  ");
            try secs(out, "%5.1f", r.secs);
            try out.raw("s\n");
        }
        try out.flush();
        if (r.timed_out) {
            util.say(r.text);
            cli.exitMsg("{s} timed out; output may be incomplete", .{st.name});
        }
        if (r.rc != 0) {
            if (std.mem.eql(u8, st.name, "build") and std.mem.indexOf(u8, r.text, "TUs OK") != null) {
                try out.raw("  (some TUs failed to compile: build/compile_status.tsv)\n");
                try out.flush();
                continue;
            }
            util.say(r.text);
            cli.exitMsg("{s} failed (rc={d})", .{ st.name, r.rc });
        }
    }
}

const Status = struct { name: []const u8, st: []const u8 };

fn statuses(path: []const u8) !std.StringArrayHashMapUnmanaged(Status) {
    var out: std.StringArrayHashMapUnmanaged(Status) = .empty;
    const data = (try util.readFileOpt(path)) orelse return out;
    var it = util.Lines{ .data = data };
    while (it.next()) |line| {
        const p = try util.split(line, '\t');
        if (!std.mem.eql(u8, p[0], "start") and p.len > 9) try out.put(gpa, p[0], .{ .name = p[2], .st = p[9] });
    }
    return out;
}

fn lessStr(_: void, a: []const u8, b: []const u8) bool {
    return std.mem.order(u8, a, b) == .lt;
}

fn gainedLost(out: *cli.Out, prev: []const u8, cur: []const u8) !void {
    const limit = 40;
    var a = try statuses(prev);
    var b = try statuses(cur);
    var gained: std.ArrayList([]const u8) = .empty;
    var lost: std.ArrayList([]const u8) = .empty;
    for (b.keys(), b.values()) |k, v| if (std.mem.eql(u8, v.st, "MATCH") and !(if (a.get(k)) |o| std.mem.eql(u8, o.st, "MATCH") else false)) try gained.append(gpa, k);
    for (a.keys(), a.values()) |k, v| if (std.mem.eql(u8, v.st, "MATCH") and !(if (b.get(k)) |o| std.mem.eql(u8, o.st, "MATCH") else false)) try lost.append(gpa, k);
    std.mem.sort([]const u8, gained.items, {}, lessStr);
    std.mem.sort([]const u8, lost.items, {}, lessStr);
    try out.print("vs previous audit: gained {d}, lost {d} MATCH\n", .{ gained.items.len, lost.items.len });
    for ([_]struct { []const u8, []const []const u8, bool }{ .{ "GAINED", gained.items, true }, .{ "LOST", lost.items, false } }) |e| {
        const tag, const keys, const is_gained = e;
        const src = if (is_gained) &b else &a;
        for (keys[0..@min(limit, keys.len)]) |k| {
            try out.print("  {s} {s} {s}", .{ tag, k, src.get(k).?.name });
            if (!is_gained) try out.print(" -> {s}", .{if (b.get(k)) |o| o.st else "unmapped"});
            try out.raw("\n");
        }
        if (keys.len > limit) try out.print("  ... {d} more {s}\n", .{ keys.len - limit, if (is_gained) "gained" else "lost" });
    }
}

pub fn main(args: []const []const u8) !void {
    const p = cli.parse(args, &.{ .{ .long = "--root" }, .{ .long = "--jobs", .short = "-j" }, .{ .long = "--verify", .value = false }, .{ .long = "--exe-map" } }, USAGE);
    if (p.pos.len > 0) {
        cli.sayErr(USAGE);
        std.process.exit(2);
    }
    const root = try cli.root(p.get("--root"));
    const cfg = try audit.Config.init(root);
    if (cfg.experiment) cli.exitMsg("quick: the baseline only (it writes evidence/)", .{});
    if (builtin.os.tag == .windows) try refimage.ensureFunctions(root);
    const jobs = try std.fmt.allocPrint(gpa, "{d}", .{p.int("--jobs", @intCast(proc.cpuCount()))});
    const exe_map = p.get("--exe-map");
    const evidence = try util.join(root, "evidence");
    const prev = try util.join(cfg.build, "audit.prev.tsv");
    util.makeDirs(cfg.build);
    const cur = try util.join(evidence, "audit.tsv");
    if (util.exists(cur)) try util.writeFile(prev, try util.readFile(cur));
    var out = cli.Out{};
    const t0 = proc.now();
    try pipeline(&out, root, evidence, jobs, exe_map, &.{}, false);
    const total = proc.now() - t0;
    try out.raw(try util.readFile(try util.join(evidence, "audit.tsv.summary")));
    try gainedLost(&out, prev, cur);
    try out.raw("incremental: ");
    try secs(&out, "%.1f", total);
    try out.raw("s\n");
    try out.flush();
    if (!p.flag("--verify")) return;

    const incr = try util.join(cfg.build, "verify/incremental");
    const full = try util.join(cfg.build, "verify/full");
    util.makeDirs(incr);
    util.makeDirs(full);
    for (OUTPUTS) |n| try util.writeFile(try util.join(incr, n), try util.readFile(try util.join(evidence, n)));
    try out.raw("\n--verify: the full run (every TU compiled, every object decoded, every function judged)\n");
    try out.flush();
    const t1 = proc.now();
    try pipeline(&out, root, full, jobs, exe_map, &.{"ORACLE_NOCACHE=1"}, true);
    try out.raw("full: ");
    try secs(&out, "%.1f", proc.now() - t1);
    try out.raw("s\n");
    try out.flush();
    var differ: std.ArrayList([]const u8) = .empty;
    for (OUTPUTS) |n| {
        const a = try util.join(incr, n);
        const b = try util.join(full, n);
        const incremental = try util.readFile(a);
        const complete = try util.readFile(b);
        if (!std.mem.eql(u8, incremental, complete)) {
            try differ.append(gpa, n);
            if (builtin.os.tag == .windows) {
                var first: usize = 0;
                while (first < @min(incremental.len, complete.len) and incremental[first] == complete[first]) first += 1;
                const old_byte: []const u8 = if (first < incremental.len)
                    try std.fmt.allocPrint(gpa, "0x{x:0>2}", .{incremental[first]})
                else "EOF";
                const new_byte: []const u8 = if (first < complete.len)
                    try std.fmt.allocPrint(gpa, "0x{x:0>2}", .{complete[first]})
                else "EOF";
                util.say(try std.fmt.allocPrint(gpa,
                    "{s}: first difference at byte {d}: incremental {s}, full {s} ({d} vs {d} bytes)\n",
                    .{ n, first, old_byte, new_byte, incremental.len, complete.len }));
            } else {
                const d = try proc.run(&.{ "diff", a, b }, .{});
                util.say(d.out);
            }
        }
    }
    if (differ.items.len > 0) {
        try out.raw("VERIFY FAILED: ");
        for (differ.items, 0..) |n, i| try out.print("{s}{s}", .{ if (i == 0) "" else ", ", n });
        try out.print(" differ ({s} vs {s})\n", .{ incr, full });
        try out.flush();
        std.process.exit(1);
    }
    try out.raw("verify: incremental and full agree on ");
    for (OUTPUTS, 0..) |n, i| try out.print("{s}{s}", .{ if (i == 0) "" else ", ", n });
    try out.raw("\n");
    try out.flush();
}
