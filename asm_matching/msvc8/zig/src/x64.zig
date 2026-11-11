//! x64: how far the tree is from a 64-bit build (x64/count, x64/setup.sh).
//!
//!   oracle-zig x64 compile <src> <out> --oracle <asm_matching/msvc8> [--compat F] [--zs]
//!                          [--jobs N] [--batch N] [PATTERN ...]
//!       compile the VC8-translated tree with the VC8 x64 cross compiler (x64/vc8x64.sh):
//!       <out>/obj/, log/<tu>.log, diag.tsv (tu, file, line, kind, code, msg), status.tsv
//!       (tu, ok/FAIL, errors, warnings), cl.rsp. The flags are the oracle's (flags.tsv per
//!       module) plus /W3 /Wp64, which makes this compiler report C4311 (pointer -> int) and
//!       C4312 (int -> pointer). --zs: syntax check only (/Zs), no objects.
//!   oracle-zig x64 compat <vc8_compat.h> <out>
//!       the compat header with x64 bodies for its two __asm helpers (x64 has no inline asm)
//!   oracle-zig x64 json <out dir> <x64.json> <commit> <ghidra version> <compiler banner>
//!       count diag.tsv/status.tsv into x64.json and print the one-line summary
//!   oracle-zig x64 layout <x64.tsv> <keyed-dir> <dest> <prefix>
//!       lay out the x64 compiler, libraries and SDK libraries from the cabinet members
const std = @import("std");
const c = @import("c");
const cli = @import("cli.zig");
const proc = @import("proc.zig");
const util = @import("util.zig");

const gpa = util.gpa;

const USAGE =
    \\usage: oracle-zig x64 compile <src> <out> --oracle <dir> [--compat F] [--zs] [--jobs N] [--batch N] [PATTERN ...]
    \\       oracle-zig x64 compat <vc8_compat.h> <out>
    \\       oracle-zig x64 json <out dir> <x64.json> <commit> <ghidra> <banner>
    \\       oracle-zig x64 layout <x64.tsv> <keyed-dir> <dest> <prefix>
    \\
;

pub fn main(args: []const []const u8) !void {
    if (args.len >= 1) {
        const sub = args[0];
        const rest = args[1..];
        if (std.mem.eql(u8, sub, "compile")) return compileMain(rest);
        if (std.mem.eql(u8, sub, "compat")) return compatMain(rest);
        if (std.mem.eql(u8, sub, "json")) return jsonMain(rest);
        if (std.mem.eql(u8, sub, "layout")) return layoutMain(rest);
    }
    cli.sayErr(USAGE);
    std.process.exit(2);
}

// ---- compile

const CFLAGS = "/O2 /Oy- /Ob1 /EHsc /GS /MT /W3 /Wp64 /D WIN32 /D NDEBUG /D _WINDOWS";

/// The path as wine sees it through Z: (= /).
fn zpath(path: []const u8) ![]const u8 {
    const r = try proc.realpath(path);
    const out = try gpa.alloc(u8, r.len + 2);
    out[0] = 'Z';
    out[1] = ':';
    for (r, 0..) |ch, i| out[i + 2] = if (ch == '/') '\\' else ch;
    return out;
}

/// A path the compiler printed, back as a host path.
fn host(winpath: []const u8) ![]const u8 {
    var p = proc.strip(winpath);
    if (p.len >= 2 and (p[0] == 'Z' or p[0] == 'z') and p[1] == ':') p = p[2..];
    const out = try gpa.dupe(u8, p);
    for (out) |*ch| if (ch.* == '\\') {
        ch.* = '/';
    };
    return out;
}

/// os.path.dirname of a relative path ('' when there is no directory).
fn dirOf(rel: []const u8) []const u8 {
    const i = std.mem.lastIndexOfScalar(u8, rel, '/') orelse return "";
    return rel[0..i];
}

const FlagRow = struct { prefix: []const u8, flags: []const u8 };

fn loadFlags(tsv: []const u8) ![]FlagRow {
    var rows: std.ArrayList(FlagRow) = .empty;
    var it = util.Lines{ .data = try proc.universalNewlines(try util.readFile(tsv)) };
    while (it.next()) |line| {
        if (std.mem.startsWith(u8, line, "#") or util.strip(line).len == 0) continue;
        const parts = try util.split(line, '\t');
        try rows.append(gpa, .{ .prefix = parts[0], .flags = if (parts.len > 1) parts[1] else "" });
    }
    const S = struct {
        fn lt(_: void, a: FlagRow, b: FlagRow) bool {
            return a.prefix.len > b.prefix.len;
        }
    };
    std.mem.sort(FlagRow, rows.items, {}, S.lt);
    return rows.items;
}

const Batch = struct { dir: []const u8, flags: []const u8, part: []const []const u8 };

const Diag = struct { file: []const u8, line: []const u8, kind: []const u8, code: []const u8, msg: []const u8 };

/// `(?:fatal )?error|warning` at s[p..]: the kind and where it ends.
fn matchKind(s: []const u8, p: usize) ?struct { kind: []const u8, end: usize } {
    const rest = s[p..];
    inline for (.{ "fatal error", "error", "warning" }) |k| {
        if (std.mem.startsWith(u8, rest, k)) return .{ .kind = k, .end = p + k.len };
    }
    return null;
}

fn digitsAt(s: []const u8, p: usize, n: usize) bool {
    if (p + n > s.len) return false;
    for (s[p .. p + n]) |ch| if (!std.ascii.isDigit(ch)) return false;
    return true;
}

/// ` (?P<kind>...) (?P<code>[letters]\d{4}): (?P<msg>.*)$` from s[p..], p just after the file
/// part's separator.
fn matchTail(s: []const u8, p0: usize, letters: []const u8) ?struct { kind: []const u8, code: []const u8, msg: []const u8 } {
    const k = matchKind(s, p0) orelse return null;
    var p = k.end;
    if (p >= s.len or s[p] != ' ') return null;
    p += 1;
    if (p >= s.len or std.mem.indexOfScalar(u8, letters, s[p]) == null or !digitsAt(s, p + 1, 4)) return null;
    const code = s[p .. p + 5];
    p += 5;
    if (!std.mem.startsWith(u8, s[p..], ": ")) return null;
    return .{ .kind = k.kind, .code = code, .msg = s[p + 2 ..] };
}

/// DIAG.match(line) or DIAG_NOLINE.match(line) of compile64.py.
fn matchDiag(s: []const u8) ?Diag {
    // ^(?P<file>.+?)\((?P<line>\d+)\) : kind (C\d{4}): msg$
    var i: usize = 1;
    while (i < s.len) : (i += 1) {
        if (s[i] != '(') continue;
        var j = i + 1;
        while (j < s.len and std.ascii.isDigit(s[j])) j += 1;
        if (j == i + 1 or !std.mem.startsWith(u8, s[j..], ") : ")) continue;
        if (matchTail(s, j + 4, "C")) |t| return .{ .file = s[0..i], .line = s[i + 1 .. j], .kind = t.kind, .code = t.code, .msg = t.msg };
    }
    // ^(?P<file>.+?) : kind ([CD]\d{4}): msg$
    i = 1;
    while (i < s.len) : (i += 1) {
        if (!std.mem.startsWith(u8, s[i..], " : ")) continue;
        if (matchTail(s, i + 3, "CD")) |t| return .{ .file = s[0..i], .line = "", .kind = t.kind, .code = t.code, .msg = t.msg };
    }
    return null;
}

const Result = struct {
    text: []const u8 = "",
    secs: f64 = 0,
    done: std.atomic.Value(bool) = .init(false),
};

const Pool = struct {
    batches: []const Batch,
    results: []Result,
    next: std.atomic.Value(usize) = .init(0),
    out: []const u8,
    src: []const u8,
    wrap: []const u8,
    rsp: []const u8,
};

fn runBatch(pool: *Pool, b: Batch) ![]const u8 {
    const objdir = if (b.dir.len > 0) try std.fmt.allocPrint(gpa, "{s}/obj/{s}", .{ pool.out, b.dir }) else try util.join(pool.out, "obj");
    util.makeDirs(objdir);
    for (b.part) |x| proc.remove(try std.fmt.allocPrint(gpa, "{s}/obj/{s}.obj", .{ pool.out, x[0 .. x.len - 4] }));
    var argv: std.ArrayList([]const u8) = .empty;
    try argv.appendSlice(gpa, &.{ pool.wrap, "cl", try std.mem.concat(gpa, u8, &.{ "@", try zpath(pool.rsp) }) });
    var fit = std.mem.tokenizeAny(u8, b.flags, " \t\n\r\x0b\x0c");
    while (fit.next()) |f| try argv.append(gpa, f);
    try argv.append(gpa, try std.mem.concat(gpa, u8, &.{ "/Fo", try zpath(objdir), "\\" }));
    for (b.part) |x| try argv.append(gpa, try zpath(try util.join(pool.src, x)));
    const r = try proc.run(argv.items, .{ .timeout_s = 600.0 * @as(f64, @floatFromInt(b.part.len)) });
    if (r.timed_out) return "TIMEOUT\n";
    return std.mem.concat(gpa, u8, &.{ try proc.universalNewlines(r.out), try proc.universalNewlines(r.err) });
}

fn worker(pool: *Pool) void {
    while (true) {
        const i = pool.next.fetchAdd(1, .monotonic);
        if (i >= pool.batches.len) return;
        const t0 = proc.now();
        const text = runBatch(pool, pool.batches[i]) catch |e| std.fmt.allocPrint(gpa, "oracle-zig: {s}\n", .{@errorName(e)}) catch "";
        pool.results[i].text = text;
        pool.results[i].secs = proc.now() - t0;
        pool.results[i].done.store(true, .release);
    }
}

fn lowerAscii(s: []const u8) ![]const u8 {
    return std.ascii.allocLowerString(gpa, s);
}

const Status = struct { ok: bool, errs: usize, warns: usize };

fn compileMain(args: []const []const u8) !void {
    const usage = "usage: oracle-zig x64 compile <src> <out> --oracle <dir> [--compat F] [--zs] [--jobs N] [--batch N] [PATTERN ...]\n";
    const p = cli.parse(args, &.{
        .{ .long = "--oracle" },
        .{ .long = "--compat" },
        .{ .long = "--zs", .value = false },
        .{ .long = "--jobs" },
        .{ .long = "--batch" },
    }, usage);
    if (p.pos.len < 2 or p.get("--oracle") == null) {
        cli.sayErr(usage);
        std.process.exit(2);
    }
    const oracle = p.get("--oracle").?;
    const zs = p.flag("--zs");
    const jobs: usize = @intCast(@max(1, p.int("--jobs", 6)));
    const batch_n: usize = @intCast(@max(1, p.int("--batch", 16)));
    const patterns = p.pos[2..];
    const src = try proc.realpath(p.pos[0]);
    const out = try proc.realpath(p.pos[1]);
    const flags_table = try loadFlags(try util.join(oracle, "flags.tsv"));

    // The TUs (sorted) and every directory holding a header.
    const Walk = struct {
        src: []const u8,
        patterns: []const []const u8,
        dirs: std.ArrayList([]const u8) = .empty,
        tus: std.ArrayList([]const u8) = .empty,
        fn visit(w: *@This(), d: *proc.WalkDir) !void {
            for (d.files) |f| if (std.mem.endsWith(u8, f, ".h")) {
                try w.dirs.append(gpa, d.path);
                break;
            };
            for (d.files) |f| {
                if (!std.mem.endsWith(u8, f, ".cpp")) continue;
                const rel = try proc.relpath(try util.join(d.path, f), w.src);
                if (w.patterns.len > 0) {
                    var hit = false;
                    for (w.patterns) |pat| hit = hit or std.mem.indexOf(u8, rel, pat) != null;
                    if (!hit) continue;
                }
                try w.tus.append(gpa, rel);
            }
        }
    };
    var w = Walk{ .src = src, .patterns = patterns };
    try proc.walk(src, &w, Walk.visit);
    const tus = w.tus.items;
    proc.sortStrings(tus);
    util.makeDirs(out);

    var lines: std.ArrayList([]const u8) = .empty;
    try lines.append(gpa, try std.mem.concat(gpa, u8, &.{ "/nologo /c ", CFLAGS, if (zs) " /Zs" else "" }));
    const compat = p.get("--compat") orelse try util.join(oracle, "vc8_compat.h");
    try lines.append(gpa, try std.mem.concat(gpa, u8, &.{ "/FI", try zpath(compat) }));
    try lines.append(gpa, try std.mem.concat(gpa, u8, &.{ "/FI", try zpath(try util.join(src, "Source/D2Headers/d2_platform.h")) }));
    try lines.append(gpa, try std.mem.concat(gpa, u8, &.{ "/I", try zpath(try util.join(oracle, "shim_include")) }));
    try lines.append(gpa, try std.mem.concat(gpa, u8, &.{ "/I", try zpath(src) }));
    const dirs = w.dirs.items;
    proc.sortStrings(dirs);
    for (dirs) |d| if (!std.mem.eql(u8, d, src)) try lines.append(gpa, try std.mem.concat(gpa, u8, &.{ "/I", try zpath(d) }));
    const rsp = try util.join(out, "cl.rsp");
    try util.writeFile(rsp, try std.mem.concat(gpa, u8, &.{ try std.mem.join(gpa, "\n", lines.items), "\n" }));

    // Groups by (directory, extra flags), split into batches of at most --batch.
    const Group = struct { dir: []const u8, flags: []const u8, items: std.ArrayList([]const u8) };
    var groups: std.ArrayList(Group) = .empty;
    for (tus) |rel| {
        var extra: []const u8 = "";
        for (flags_table) |row| if (std.mem.startsWith(u8, rel, row.prefix)) {
            extra = row.flags;
            break;
        };
        const d = dirOf(rel);
        var found: ?*Group = null;
        for (groups.items) |*g| if (std.mem.eql(u8, g.dir, d) and std.mem.eql(u8, g.flags, extra)) {
            found = g;
            break;
        };
        if (found == null) {
            try groups.append(gpa, .{ .dir = d, .flags = extra, .items = .empty });
            found = &groups.items[groups.items.len - 1];
        }
        try found.?.items.append(gpa, rel);
    }
    const GS = struct {
        fn lt(_: void, a: Group, b: Group) bool {
            return switch (std.mem.order(u8, a.dir, b.dir)) {
                .lt => true,
                .gt => false,
                .eq => std.mem.order(u8, a.flags, b.flags) == .lt,
            };
        }
    };
    std.mem.sort(Group, groups.items, {}, GS.lt);
    var batches: std.ArrayList(Batch) = .empty;
    for (groups.items) |g| {
        const items = g.items.items;
        const n = (items.len + batch_n - 1) / batch_n;
        for (0..n) |k| {
            var part: std.ArrayList([]const u8) = .empty;
            var i = k;
            while (i < items.len) : (i += n) try part.append(gpa, items[i]);
            var unique = true;
            for (part.items, 0..) |a, ai| for (part.items[ai + 1 ..]) |b| {
                if (std.ascii.eqlIgnoreCase(util.basename(a), util.basename(b))) unique = false;
            };
            if (unique) {
                try batches.append(gpa, .{ .dir = g.dir, .flags = g.flags, .part = part.items });
            } else for (part.items) |x| try batches.append(gpa, .{ .dir = g.dir, .flags = g.flags, .part = try gpa.dupe([]const u8, &.{x}) });
        }
    }
    const Sized = struct { b: Batch, size: u64 };
    var sized = try gpa.alloc(Sized, batches.items.len);
    for (batches.items, 0..) |b, i| {
        var s: u64 = 0;
        for (b.part) |x| s += proc.fileSize(try util.join(src, x));
        sized[i] = .{ .b = b, .size = s };
    }
    const SS = struct {
        fn lt(_: void, a: Sized, b: Sized) bool {
            return a.size > b.size;
        }
    };
    std.mem.sort(Sized, sized, {}, SS.lt);
    const work = try gpa.alloc(Batch, sized.len);
    for (sized, 0..) |s, i| work[i] = s.b;

    var srcpaths: std.StringHashMapUnmanaged([]const u8) = .empty;
    for (tus) |x| try srcpaths.put(gpa, try lowerAscii(try proc.realpath(try util.join(src, x))), x);

    const results = try gpa.alloc(Result, work.len);
    for (results) |*r| r.* = .{};
    var pool = Pool{
        .batches = work,
        .results = results,
        .out = out,
        .src = src,
        .wrap = try util.join(try proc.realpath(oracle), "x64/vc8x64.sh"),
        .rsp = rsp,
    };
    const t_start = proc.now();
    var threads: std.ArrayList(std.Thread) = .empty;
    for (0..@max(@as(usize, 1), @min(jobs, work.len))) |_| try threads.append(gpa, try std.Thread.spawn(.{}, worker, .{&pool}));

    var diag: std.ArrayList(u8) = .empty;
    try diag.appendSlice(gpa, "tu\tfile\tline\tkind\tcode\tmsg\n");
    var status: std.StringHashMapUnmanaged(Status) = .empty;
    var done: usize = 0;
    for (work, 0..) |b, bi| {
        while (!results[bi].done.load(.acquire)) _ = c.usleep(20_000);
        const text = results[bi].text;
        var per: std.StringHashMapUnmanaged(std.ArrayList([]const u8)) = .empty;
        for (b.part) |x| try per.put(gpa, x, .empty);
        var cur: ?[]const u8 = null;
        var backend = false;
        for (try proc.splitlines(text)) |line| {
            const s = proc.strip(line);
            if (std.mem.eql(u8, s, "Generating Code...")) {
                backend = true;
                continue;
            }
            if (!backend) {
                var named: ?[]const u8 = null;
                for (b.part) |x| if (std.mem.eql(u8, util.basename(x), s)) {
                    named = x;
                };
                if (named) |x| {
                    cur = x;
                    continue;
                }
            }
            if (matchDiag(line)) |m| {
                const f = try proc.latin1ToUtf8(try host(m.file));
                var tu: ?[]const u8 = if (backend) srcpaths.get(try lowerAscii(try proc.realpath(f))) else cur;
                if (tu == null) tu = cur orelse b.part[0];
                const rel = if (std.mem.startsWith(u8, f, src)) try proc.relpath(f, src) else f;
                const kind: []const u8 = if (std.mem.indexOf(u8, m.kind, "error") != null) "error" else "warning";
                const g = try per.getOrPut(gpa, tu.?);
                if (!g.found_existing) g.value_ptr.* = .empty;
                try g.value_ptr.append(gpa, line);
                const msg = try gpa.dupe(u8, m.msg);
                for (msg) |*ch| if (ch.* == '\t') {
                    ch.* = ' ';
                };
                try diag.appendSlice(gpa, try std.mem.join(gpa, "\t", &.{ tu.?, rel, m.line, kind, m.code, try proc.latin1ToUtf8(msg) }));
                try diag.append(gpa, '\n');
            } else if (cur) |x| {
                try per.getPtr(x).?.append(gpa, line);
            }
        }
        const timeout = std.mem.indexOf(u8, text, "TIMEOUT") != null;
        for (b.part) |x| {
            const stem = x[0 .. x.len - 4];
            const lp = try std.fmt.allocPrint(gpa, "{s}/log/{s}.log", .{ out, stem });
            util.makeDirs(util.dirname(lp));
            const pl = per.get(x).?.items;
            try util.writeFile(lp, try proc.latin1ToUtf8(try std.mem.concat(gpa, u8, &.{ try std.mem.join(gpa, "\n", pl), "\n" })));
            var errs: usize = 0;
            var warns: usize = 0;
            for (pl) |l| {
                if (std.mem.indexOf(u8, l, " error C") != null) errs += 1;
                if (std.mem.indexOf(u8, l, " warning C") != null) warns += 1;
            }
            const obj = try std.fmt.allocPrint(gpa, "{s}/obj/{s}.obj", .{ out, stem });
            var ok = if (zs) errs == 0 else (util.exists(obj) and errs == 0);
            if (timeout) ok = false;
            try status.put(gpa, x, .{ .ok = ok, .errs = errs, .warns = warns });
        }
        done += b.part.len;
        var line: std.ArrayList(u8) = .empty;
        try line.print(gpa, "{d: >4}/{d}  ", .{ done, tus.len });
        try util.fixed(&line, "%5.1f", results[bi].secs);
        try line.print(gpa, "s  {s} ({d})\n", .{ b.dir, b.part.len });
        util.say(line.items);
    }
    for (threads.items) |th| th.join();
    try util.writeFile(try util.join(out, "diag.tsv"), diag.items);
    var st: std.ArrayList(u8) = .empty;
    try st.appendSlice(gpa, "tu\tstatus\terrors\twarnings\n");
    var nfail: usize = 0;
    for (tus) |x| {
        const v = status.get(x) orelse Status{ .ok = false, .errs = 0, .warns = 0 };
        try st.print(gpa, "{s}\t{s}\t{d}\t{d}\n", .{ x, if (v.ok) "ok" else "FAIL", v.errs, v.warns });
    }
    var vit = status.valueIterator();
    while (vit.next()) |v| {
        if (!v.ok) nfail += 1;
    }
    try util.writeFile(try util.join(out, "status.tsv"), st.items);
    var fin: std.ArrayList(u8) = .empty;
    try fin.print(gpa, "TUs {d}, failed {d}, ", .{ tus.len, nfail });
    try util.fixed(&fin, "%.0f", proc.now() - t_start);
    try fin.appendSlice(gpa, "s\n");
    util.say(fin.items);
}

// ---- compat

const ASM_FRAME = "static __forceinline void* vc8_frame_address(void) { void* p; __asm { mov p, ebp } return p; }";
const ASM_NEARBYINT = "inline double nearbyint(double x) { double r; __asm { fld x } __asm { frndint } __asm { fstp r } return r; }";

fn compatMain(args: []const []const u8) !void {
    if (args.len != 2) {
        cli.sayErr("usage: oracle-zig x64 compat <vc8_compat.h> <out>\n");
        std.process.exit(2);
    }
    var s: []const u8 = try proc.universalNewlines(try util.readFile(args[0]));
    if (std.mem.indexOf(u8, s, ASM_FRAME) != null) {
        const rep = "#ifdef _WIN64\nextern \"C\" void* _AddressOfReturnAddress(void);\n#pragma intrinsic(_AddressOfReturnAddress)\n" ++
            "static __forceinline void* vc8_frame_address(void) { return _AddressOfReturnAddress(); }\n#else\n" ++ ASM_FRAME ++ "\n#endif";
        s = try std.mem.replaceOwned(u8, gpa, s, ASM_FRAME, rep);
    }
    if (std.mem.indexOf(u8, s, ASM_NEARBYINT) != null) {
        const rep = "#ifdef _WIN64\ninline double nearbyint(double x) { return (double)(__int64)(x < 0 ? x - 0.5 : x + 0.5); }\n#else\n" ++ ASM_NEARBYINT ++ "\n#endif";
        s = try std.mem.replaceOwned(u8, gpa, s, ASM_NEARBYINT, rep);
    }
    try util.writeFile(args[1], s);
}

// ---- json

/// A JSON string as json.dumps writes it (ensure_ascii): UTF-8 in, \uXXXX out.
fn jsonString(out: *std.ArrayList(u8), s: []const u8) !void {
    try out.append(gpa, '"');
    var i: usize = 0;
    while (i < s.len) {
        const n = std.unicode.utf8ByteSequenceLength(s[i]) catch 1;
        const cp: u21 = if (n > 1 and i + n <= s.len) (std.unicode.utf8Decode(s[i .. i + n]) catch s[i]) else s[i];
        i += if (n > 1 and i + n <= s.len) n else 1;
        switch (cp) {
            '"' => try out.appendSlice(gpa, "\\\""),
            '\\' => try out.appendSlice(gpa, "\\\\"),
            '\n' => try out.appendSlice(gpa, "\\n"),
            '\r' => try out.appendSlice(gpa, "\\r"),
            '\t' => try out.appendSlice(gpa, "\\t"),
            0x08 => try out.appendSlice(gpa, "\\b"),
            0x0c => try out.appendSlice(gpa, "\\f"),
            else => {
                if (cp < 0x20 or cp > 0x7e) {
                    if (cp >= 0x10000) {
                        const v = cp - 0x10000;
                        try out.print(gpa, "\\u{x:0>4}\\u{x:0>4}", .{ 0xd800 + (v >> 10), 0xdc00 + (v & 0x3ff) });
                    } else try out.print(gpa, "\\u{x:0>4}", .{cp});
                } else try out.append(gpa, @intCast(cp));
            },
        }
    }
    try out.append(gpa, '"');
}

/// The rows of a tab-separated file with a header, as csv.DictReader(QUOTE_NONE) reads them.
fn readTsv(path: []const u8) !struct { header: []const []const u8, rows: []const []const []const u8 } {
    var rows: std.ArrayList([]const []const u8) = .empty;
    var header: []const []const u8 = &.{};
    var it = util.Lines{ .data = try util.readFile(path) };
    var first = true;
    while (it.next()) |raw| {
        const line = if (std.mem.endsWith(u8, raw, "\r")) raw[0 .. raw.len - 1] else raw;
        if (line.len == 0) continue;
        const parts = try util.split(line, '\t');
        if (first) {
            header = parts;
            first = false;
        } else try rows.append(gpa, parts);
    }
    return .{ .header = header, .rows = rows.items };
}

fn column(header: []const []const u8, name: []const u8) ?usize {
    for (header, 0..) |h, i| if (std.mem.eql(u8, h, name)) return i;
    return null;
}

fn field(row: []const []const u8, i: ?usize) ?[]const u8 {
    const k = i orelse return null;
    return if (k < row.len) row[k] else null;
}

fn jsonMain(args: []const []const u8) !void {
    if (args.len != 5) {
        cli.sayErr("usage: oracle-zig x64 json <out dir> <x64.json> <commit> <ghidra> <banner>\n");
        std.process.exit(2);
    }
    const out = args[0];
    const dst = args[1];
    const sha = args[2];
    const ghidra = args[3];
    const banner = args[4];
    var c4311: std.StringHashMapUnmanaged(void) = .empty;
    var c4312: std.StringHashMapUnmanaged(void) = .empty;
    var both: std.StringHashMapUnmanaged(void) = .empty;
    const diag = try readTsv(try util.join(out, "diag.tsv"));
    const dcode = column(diag.header, "code");
    const dfile = column(diag.header, "file");
    const dline = column(diag.header, "line");
    for (diag.rows) |r| {
        const code = field(r, dcode) orelse continue;
        const set = if (std.mem.eql(u8, code, "C4311")) &c4311 else if (std.mem.eql(u8, code, "C4312")) &c4312 else continue;
        const key = try std.fmt.allocPrint(gpa, "{s}\x00{s}", .{ field(r, dfile) orelse "\x01None", field(r, dline) orelse "\x01None" });
        try set.put(gpa, key, {});
        try both.put(gpa, key, {});
    }
    const st = try readTsv(try util.join(out, "status.tsv"));
    const stu = column(st.header, "tu");
    const sst = column(st.header, "status");
    var failing: std.ArrayList([]const u8) = .empty;
    for (st.rows) |r| {
        const s = field(r, sst) orelse "";
        if (!std.mem.eql(u8, s, "ok")) try failing.append(gpa, field(r, stu) orelse "");
    }
    proc.sortStrings(failing.items);
    const compiler = if (std.mem.indexOf(u8, banner, "Version ")) |i| banner[i + "Version ".len ..] else banner;

    var t: c.time_t = c.time(null);
    var tm: c.struct_tm = undefined;
    _ = c.localtime_r(&t, &tm);
    var dbuf: [32]u8 = undefined;
    const dn = c.strftime(&dbuf, dbuf.len, "%Y-%m-%d", &tm);

    var j: std.ArrayList(u8) = .empty;
    try j.appendSlice(gpa, "{\n  \"compiler\": ");
    try jsonString(&j, try std.mem.concat(gpa, u8, &.{ "cl ", compiler, ", /Zs /W3 /Wp64" }));
    try j.appendSlice(gpa, ",\n  \"commit\": ");
    try jsonString(&j, sha);
    try j.appendSlice(gpa, ",\n  \"ghidra\": ");
    if (ghidra.len > 0) try jsonString(&j, ghidra) else try j.appendSlice(gpa, "null");
    try j.appendSlice(gpa, ",\n  \"measured\": ");
    try jsonString(&j, dbuf[0..dn]);
    try j.print(gpa, ",\n  \"c4311\": {d},\n  \"c4312\": {d},\n  \"lines\": {d},\n  \"tus_total\": {d},\n  \"tus_failing\": {d},\n  \"failing\": ", .{ c4311.count(), c4312.count(), both.count(), st.rows.len, failing.items.len });
    if (failing.items.len == 0) {
        try j.appendSlice(gpa, "[]");
    } else {
        try j.appendSlice(gpa, "[\n");
        for (failing.items, 0..) |f, i| {
            try j.appendSlice(gpa, "    ");
            try jsonString(&j, f);
            try j.appendSlice(gpa, if (i + 1 < failing.items.len) ",\n" else "\n");
        }
        try j.appendSlice(gpa, "  ]");
    }
    try j.appendSlice(gpa, "\n}\n");
    try util.writeFile(dst, j.items);
    util.say(try std.fmt.allocPrint(gpa, "C4311 {d}, C4312 {d}, {d} lines; {d} of {d} TUs fail\n", .{ c4311.count(), c4312.count(), both.count(), failing.items.len, st.rows.len }));
}

// ---- layout

const WHERE = [_][2][]const u8{
    .{ "bin/x86_amd64/", "rtm-x64/bin/" },
    .{ "lib/amd64/", "rtm-x64/lib/" },
    .{ "PlatformSDK/Lib/AMD64/", "psdk-x64/lib/" },
};

fn layoutMain(args: []const []const u8) !void {
    if (args.len != 4) {
        cli.sayErr("usage: oracle-zig x64 layout <x64.tsv> <keyed-dir> <dest> <prefix>\n");
        std.process.exit(2);
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
        var rel: ?[]const u8 = null;
        for (WHERE) |w| if (std.mem.startsWith(u8, rest, w[0])) {
            rel = try std.mem.concat(gpa, u8, &.{ w[1], rest[w[0].len..] });
            break;
        };
        if (rel == null) cli.exitMsg("no x64 place for {s}", .{p[1]});
        const src = try util.join(keyed, p[0]);
        if (!proc.isFile(src)) cli.exitMsg("missing cabinet member {s} for {s}", .{ p[0], p[1] });
        const out = try util.join(dest, rel.?);
        util.makeDirs(util.dirname(out));
        try util.writeFile(out, try util.readFile(src));
        n += 1;
    }
    util.say(try std.fmt.allocPrint(gpa, "{s}: {d} files\n", .{ dest, n }));
}
