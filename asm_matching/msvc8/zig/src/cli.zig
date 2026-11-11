//! Command lines as argparse reads them, for the tools that kept their Python CLI: long options
//! with a value (`--out F`, `--out=F`) or without (`--verify`), unique prefixes of them
//! (`--ex` for `--examples`), short options (`-j 4`, `-j4`), and positionals in between.
//! Also where the oracle lives when no `--root` names it, and stdout as Python prints it.
const std = @import("std");
const c = @import("c");
const util = @import("util.zig");
const proc = @import("proc.zig");

const gpa = util.gpa;

pub const Opt = struct {
    long: []const u8, // "--out"
    short: ?[]const u8 = null, // "-j"
    value: bool = true,
    many: bool = false, // action='append'
};

pub const Parsed = struct {
    opts: []const Opt,
    vals: [][]const []const u8,
    pos: []const []const u8,

    fn index(p: *const Parsed, long: []const u8) usize {
        for (p.opts, 0..) |o, i| if (std.mem.eql(u8, o.long, long)) return i;
        @panic("cli: no such option");
    }

    /// The last value given, or null.
    pub fn get(p: *const Parsed, long: []const u8) ?[]const u8 {
        const v = p.vals[p.index(long)];
        return if (v.len > 0) v[v.len - 1] else null;
    }

    pub fn all(p: *const Parsed, long: []const u8) []const []const u8 {
        return p.vals[p.index(long)];
    }

    pub fn flag(p: *const Parsed, long: []const u8) bool {
        return p.vals[p.index(long)].len > 0;
    }

    pub fn int(p: *const Parsed, long: []const u8, default: i64) i64 {
        const v = p.get(long) orelse return default;
        return std.fmt.parseInt(i64, v, 10) catch fail(p, "invalid int value: '{s}'", .{v});
    }

    fn fail(_: *const Parsed, comptime fmt: []const u8, args: anytype) noreturn {
        std.debug.print("error: " ++ fmt ++ "\n", args);
        std.process.exit(2);
    }
};

fn usageExit(usage: []const u8, comptime fmt: []const u8, args: anytype) noreturn {
    std.debug.print("{s}", .{usage});
    std.debug.print("error: " ++ fmt ++ "\n", args);
    std.process.exit(2);
}

/// Parse `args` against `opts`; a malformed line prints `usage` and exits 2 (argparse's status).
/// `-h`/`--help` prints `usage` and exits 0.
pub fn parse(args: []const []const u8, opts: []const Opt, usage: []const u8) Parsed {
    return parseImpl(args, opts, usage) catch {
        std.debug.print("out of memory\n", .{});
        std.process.exit(1);
    };
}

fn parseImpl(args: []const []const u8, opts: []const Opt, usage: []const u8) !Parsed {
    const lists = try gpa.alloc(std.ArrayList([]const u8), opts.len);
    for (lists) |*l| l.* = .empty;
    var pos: std.ArrayList([]const u8) = .empty;
    var k: usize = 0;
    var only_pos = false;
    while (k < args.len) : (k += 1) {
        const a = args[k];
        if (only_pos or a.len < 2 or a[0] != '-') {
            try pos.append(gpa, a);
            continue;
        }
        if (std.mem.eql(u8, a, "--")) {
            only_pos = true;
            continue;
        }
        if (std.mem.eql(u8, a, "-h") or std.mem.eql(u8, a, "--help")) {
            util.say(usage);
            std.process.exit(0);
        }
        var oi: ?usize = null;
        var inline_val: ?[]const u8 = null;
        if (std.mem.startsWith(u8, a, "--")) {
            const eq = std.mem.indexOfScalar(u8, a, '=');
            const name = if (eq) |e| a[0..e] else a;
            if (eq) |e| inline_val = a[e + 1 ..];
            var hits: usize = 0;
            for (opts, 0..) |o, i| {
                if (std.mem.eql(u8, o.long, name)) {
                    oi = i;
                    hits = 1;
                    break;
                }
                if (std.mem.startsWith(u8, o.long, name)) {
                    oi = i;
                    hits += 1;
                }
            }
            if (hits != 1) usageExit(usage, "{s} {s}", .{ if (hits == 0) "unrecognized argument:" else "ambiguous option:", a });
        } else {
            for (opts, 0..) |o, i| if (o.short) |s| if (std.mem.startsWith(u8, a, s)) {
                oi = i;
                if (a.len > s.len) inline_val = a[s.len..];
            };
            if (oi == null) usageExit(usage, "unrecognized argument: {s}", .{a});
        }
        const o = opts[oi.?];
        if (!o.value) {
            if (inline_val != null) usageExit(usage, "argument {s}: ignored explicit argument", .{o.long});
            try lists[oi.?].append(gpa, "1");
            continue;
        }
        const v = inline_val orelse blk: {
            if (k + 1 >= args.len or (args[k + 1].len > 1 and args[k + 1][0] == '-' and !isNumber(args[k + 1])))
                usageExit(usage, "argument {s}: expected one argument", .{o.long});
            k += 1;
            break :blk args[k];
        };
        if (!o.many) lists[oi.?].clearRetainingCapacity();
        try lists[oi.?].append(gpa, v);
    }
    const vals = try gpa.alloc([]const []const u8, opts.len);
    for (lists, 0..) |l, i| vals[i] = l.items;
    return .{ .opts = opts, .vals = vals, .pos = pos.items };
}

fn isNumber(s: []const u8) bool {
    _ = std.fmt.parseFloat(f64, s) catch return false;
    return true;
}

/// asm_matching/msvc8: --root when given, else $ORACLE_ROOT, else where this executable sits
/// (<root>/zig/zig-out/bin/oracle-zig).
pub fn root(given: ?[]const u8) ![]const u8 {
    if (given) |g| return absolute(g);
    if (util.getenv("ORACLE_ROOT")) |r| if (r.len > 0) return absolute(r);
    const exe = try selfExe();
    return util.dirname(util.dirname(util.dirname(util.dirname(exe))));
}

fn absolute(p: []const u8) ![]const u8 {
    if (p.len > 0 and p[0] == '/') return proc.normpath(p);
    var buf: [4096]u8 = undefined;
    const cwd = std.mem.sliceTo(c.getcwd(&buf, buf.len) orelse return error.Cwd, 0);
    return proc.joinNorm(cwd, p);
}

/// The path of this executable, resolved.
pub fn selfExe() ![]const u8 {
    var buf: [4096]u8 = undefined;
    const n = std.process.executablePath(std.Io.Threaded.global_single_threaded.io(), &buf) catch return error.SelfExe;
    return proc.realpath(try gpa.dupe(u8, buf[0..n]));
}

/// sys.exit('message'): the message on stderr, status 1.
pub fn exitMsg(comptime fmt: []const u8, args: anytype) noreturn {
    sayErr(std.fmt.allocPrint(gpa, fmt ++ "\n", args) catch "out of memory\n");
    std.process.exit(1);
}

pub fn sayErr(s: []const u8) void {
    var p = s;
    while (p.len > 0) {
        const n = c.write(2, p.ptr, p.len);
        if (n <= 0) return;
        p = p[@intCast(n)..];
    }
}

/// Text on its way to stdout. Python holds what it read from a binary as Latin-1 (symbol
/// names and string literals from objects and the image, so the views and listings) and what
/// it read from a text file as UTF-8 (the maps, the audit, sources), and prints both as UTF-8:
/// `lat` converts the first, everything else is written as it is.
pub const Out = struct {
    buf: std.ArrayList(u8) = .empty,

    pub fn print(o: *Out, comptime fmt: []const u8, args: anytype) !void {
        try o.buf.print(gpa, fmt, args);
    }

    pub fn line(o: *Out, s: []const u8) !void {
        try o.buf.appendSlice(gpa, s);
        try o.buf.append(gpa, '\n');
    }

    pub fn raw(o: *Out, s: []const u8) !void {
        try o.buf.appendSlice(gpa, s);
    }

    /// Latin-1 text, as UTF-8.
    pub fn lat(o: *Out, s: []const u8) !void {
        try o.buf.appendSlice(gpa, try proc.latin1ToUtf8(s));
    }

    pub fn latLine(o: *Out, s: []const u8) !void {
        try o.lat(s);
        try o.buf.append(gpa, '\n');
    }

    pub fn flush(o: *Out) !void {
        util.say(o.buf.items);
        o.buf.clearRetainingCapacity();
    }
};
