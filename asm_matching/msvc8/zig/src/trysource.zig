//! try-source: one hand-written function body compiled by VC8 in its own TU and judged
//! against the original, for agents writing bodies by hand.
//!
//!   oracle-zig try-source [--root R] <address> <body file> [--helpers FILE] [--expect STATUS]
//!                         [--context N]
//!       The body file holds the whole definition (signature and body) or only the body from
//!       its `{`, which keeps the tree's signature. It replaces the tree's definition after the
//!       function's `// 1.14d: <address>` anchor in a copy of its TU; the helpers file, when
//!       given, goes in front of the anchor. Prints one JSON line (below). Exits 2 on an
//!       {"error"} reply; with --expect, 1 unless the status is STATUS.
//!   oracle-zig try-serve [--root R] [--idle SECONDS] [--max-requests N]
//!       The queue every try-source talks to, over the socket build/try-source.sock; the first
//!       client starts it, and it exits after SECONDS idle (900) or N requests (400; the
//!       process arena is never freed). Requests that arrive together go to one round: each
//!       spliced copy and each TU's unchanged copy (once per state of the tree) is translated
//!       by ghidra-reconstruct and compiled as `trial` compiles (16 per cl.exe, D2_VC8_JOBS
//!       cl.exe at a time, default 6), under WINE_BENCH_LOCK (flock, when set) and the build's
//!       lock, then judged as `judge` judges.
//!
//! The reply: {"addr", "name", "tu", "status", "baseline" (the tree's body, same compile),
//! "our_insns", "ref_insns", "byte_diffs", "ref_diffs", "similarity", "register_only",
//! "frame": {"ours", "ref", "equal"}, "body_lines" (the body file's lines with differing code),
//! "asm" (compare.sideBySide's lines that differ, N lines of context around them, "..."
//! between runs; our source line given as the body file's, blank outside the body), "collateral" ([{addr, name, before, after}] for every other function of the
//! TU whose status moved against the unchanged copy), "errors" ([{line, body_line, text}] when
//! VC8 rejected the TU), "ms"} or {"error": message}. The status is the audit's (MATCH,
//! INSN_MATCH, OUR+N, ...) or COMPILE_ERROR, MISSING (no such function in the object). The
//! original's bytes never leave the server: only its listing beside ours where they differ.
const std = @import("std");
const builtin = @import("builtin");
const c = @import("c");
const util = @import("util.zig");
const proc = @import("proc.zig");
const cli = @import("cli.zig");
const compile = @import("compile.zig");
const audit = @import("audit.zig");
const compare = @import("compare.zig");
const objcache = @import("objcache.zig");
const permute = @import("permute.zig");
const ts_translate = @import("ts_translate.zig");
const gpa = util.gpa;

const BATCH = 16;
const DEFAULT_JOBS = 6;
const DEFAULT_IDLE = 900;
const DEFAULT_MAX_REQUESTS = 400;
const DEFAULT_CONTEXT = 2;
/// A round that is not yet full waits while requests keep arriving: until none came for
/// GATHER_QUIET_S, at most GATHER_MAX_S.
const GATHER_QUIET_S = 0.25;
const GATHER_MAX_S = 1.5;
const MAX_REQUEST = 8 << 20;
const MAX_ERRORS = 30;
const DIFF_WIDTH = 66;

fn usage() noreturn {
    std.debug.print(
        \\usage: oracle-zig try-source [--root R] <address> <body file> [--helpers FILE] [--expect STATUS] [--context N]
        \\       oracle-zig try-serve [--root R] [--idle SECONDS] [--max-requests N]
        \\
    , .{});
    std.process.exit(2);
}

// ---- the splice

pub const Splice = struct {
    text: []const u8,
    /// 1-based line of the result where the body file's first line is.
    body_line: u32,
    body_lines: u32,
};

pub const SpliceResult = union(enum) { ok: Splice, err: []const u8 };

/// The address an anchor line names: `// 1.14d: 00402c10`, `// 1.14d: 0x5956c0 | ...`.
fn anchorAddress(line: []const u8) ?u32 {
    const t = std.mem.trimStart(u8, line, " \t");
    const pfx = "// 1.14d: ";
    if (!std.mem.startsWith(u8, t, pfx)) return null;
    var w = t[pfx.len..];
    if (std.mem.startsWith(u8, w, "0x")) w = w[2..];
    var n: usize = 0;
    while (n < w.len and std.ascii.isHex(w[n])) n += 1;
    if (n == 0 or n > 8) return null;
    if (n < w.len and (std.ascii.isAlphanumeric(w[n]) or w[n] == '_')) return null;
    return std.fmt.parseInt(u32, w[0..n], 16) catch null;
}

/// Offsets of the function definition that follows `from`: its body's `{` and the byte after
/// its `}`. Comments, string and character literals are skipped; a `;` or another anchor
/// before the body means no definition follows.
fn definitionAfter(src: []const u8, from: usize) ?struct { open: usize, end: usize } {
    var i = from;
    var parens: usize = 0;
    var depth: usize = 0;
    var open: ?usize = null;
    var line_start = true;
    while (i < src.len) {
        const ch = src[i];
        if (line_start and open == null and anchorAddress(src[i..(std.mem.indexOfScalarPos(u8, src, i, '\n') orelse src.len)]) != null) return null;
        line_start = ch == '\n';
        if (ch == '/' and i + 1 < src.len and src[i + 1] == '/') {
            i = std.mem.indexOfScalarPos(u8, src, i, '\n') orelse src.len;
            continue;
        }
        if (ch == '/' and i + 1 < src.len and src[i + 1] == '*') {
            i = if (std.mem.indexOfPos(u8, src, i + 2, "*/")) |e| e + 2 else src.len;
            continue;
        }
        if (ch == '"' or ch == '\'') {
            i += 1;
            while (i < src.len and src[i] != ch) : (i += 1) {
                if (src[i] == '\\') i += 1;
            }
            i += 1;
            continue;
        }
        if (open == null) {
            switch (ch) {
                '(' => parens += 1,
                ')' => parens -|= 1,
                ';' => if (parens == 0) return null,
                '{' => if (parens == 0) {
                    open = i;
                    depth = 1;
                },
                else => {},
            }
        } else switch (ch) {
            '{' => depth += 1,
            '}' => {
                depth -= 1;
                if (depth == 0) return .{ .open = open.?, .end = i + 1 };
            },
            else => {},
        }
        i += 1;
    }
    return null;
}

fn countLines(s: []const u8) u32 {
    return @intCast(std.mem.count(u8, s, "\n"));
}

/// `src` with the definition after `addr`'s anchor replaced by `body` (a whole definition, or
/// a body from its `{` that keeps the tree's signature), `helpers` in front of the anchor.
pub fn splice(src: []const u8, addr: u32, body: []const u8, helpers: ?[]const u8) !SpliceResult {
    var anchor: ?usize = null;
    var found: usize = 0;
    var pos: usize = 0;
    while (pos < src.len) {
        const e = std.mem.indexOfScalarPos(u8, src, pos, '\n') orelse src.len;
        if (anchorAddress(src[pos..e])) |a| if (a == addr) {
            if (anchor == null) anchor = pos;
            found += 1;
        };
        pos = e + 1;
    }
    if (found != 1) return .{ .err = try std.fmt.allocPrint(gpa, "the TU has {d} `// 1.14d: {x:0>8}` anchors, expected one", .{ found, addr }) };
    const after = (std.mem.indexOfScalarPos(u8, src, anchor.?, '\n') orelse return .{ .err = "the anchor is the last line" }) + 1;
    const def = definitionAfter(src, after) orelse return .{ .err = try std.fmt.allocPrint(gpa, "no function definition follows the anchor of {x:0>8}", .{addr}) };
    const trimmed = std.mem.trimStart(u8, body, " \t\r\n");
    const keep_signature = trimmed.len > 0 and trimmed[0] == '{';
    var out: std.ArrayList(u8) = .empty;
    try out.appendSlice(gpa, src[0..anchor.?]);
    if (helpers) |h| if (h.len > 0) {
        try out.appendSlice(gpa, h);
        if (h[h.len - 1] != '\n') try out.append(gpa, '\n');
    };
    try out.appendSlice(gpa, src[anchor.?..after]);
    if (keep_signature) try out.appendSlice(gpa, src[after..def.open]);
    const body_line = countLines(out.items) + 1;
    try out.appendSlice(gpa, body);
    try out.appendSlice(gpa, src[def.end..]);
    return .{ .ok = .{ .text = out.items, .body_line = body_line, .body_lines = countLines(body) + 1 } };
}

test "splice replaces the definition after the anchor" {
    const src = "int a;\n// 1.14d: 00401000\nint f(int x) {\n  if (x) { return 1; }\n  return \"}\"[0];\n}\n\n// 1.14d: 00401020\nint g() { return 2; }\n";
    const r = try splice(src, 0x401000, "int f(int x) {\n  return x;\n}", null);
    try std.testing.expectEqualStrings("int a;\n// 1.14d: 00401000\nint f(int x) {\n  return x;\n}\n\n// 1.14d: 00401020\nint g() { return 2; }\n", r.ok.text);
    try std.testing.expectEqual(@as(u32, 3), r.ok.body_line);
    const b = try splice(src, 0x401020, "{ return 3; }", "static int h() { return 4; }");
    try std.testing.expectEqualStrings("int a;\n// 1.14d: 00401000\nint f(int x) {\n  if (x) { return 1; }\n  return \"}\"[0];\n}\n\nstatic int h() { return 4; }\n// 1.14d: 00401020\nint g() { return 3; }\n", b.ok.text);
    try std.testing.expectEqual(@as(u32, 10), b.ok.body_line);
    try std.testing.expect((try splice(src, 0x401040, "{}", null)) == .err);
    try std.testing.expect((try splice("// 1.14d: 00401000\nint f();\n", 0x401000, "{}", null)) == .err);
}

// ---- the server

const Request = struct {
    fd: c_int,
    addr: u32,
    body: []const u8,
    helpers: ?[]const u8,
    context: usize,
    t0: f64,
    retries: u32 = 0,
    // filled by the round
    row: audit.MapRow = undefined,
    splice: Splice = undefined,
    src_sha: []const u8 = "",
    raw: []const u8 = "",
    trial: []const u8 = "",
    slot: ?usize = null,
    reply: ?[]const u8 = null,
};

/// The statuses of a TU's functions in its unchanged copy.
const Baseline = std.AutoHashMapUnmanaged(u32, []const u8);

const Server = struct {
    root: []const u8,
    repo: []const u8,
    build: []const u8,
    key: []const u8 = "",
    judge: *permute.Judge = undefined,
    by_start: std.AutoHashMapUnmanaged(u32, audit.MapRow) = .empty,
    tr: ts_translate.Translator = undefined,
    tr_up: bool = false,
    baselines: std.StringHashMapUnmanaged(Baseline) = .empty,
    jobs: usize,
    bench_lock: ?[]const u8,
    rounds: usize = 0,

    lock: proc.Mutex = .{},
    cond: c.pthread_cond_t = undefined,
    queue: std.ArrayList(*Request) = .empty,

    fn load(s: *Server) !void {
        s.key = try permute.stateKey();
        s.judge = try permute.openJudge(s.root);
        s.by_start = .empty;
        var it = s.judge.by_tu.valueIterator();
        while (it.next()) |rows| for (rows.items) |r| try s.by_start.put(gpa, r.s, r);
        s.baselines = .empty;
        if (s.tr_up) s.tr.close();
        const gr = (try ts_translate.checkout()) orelse {
            std.debug.print("try-serve: no ghidra-reconstruct checkout to translate with (GR=<checkout>, else ~/code/ts/gr-daemon)\n", .{});
            return error.NoTranslator;
        };
        s.tr = .{ .gr = gr, .repo = s.repo };
        s.tr.init();
        try s.tr.serve();
        s.tr_up = true;
    }

    fn fail(r: *Request, comptime fmt: []const u8, args: anytype) !void {
        var o: std.ArrayList(u8) = .empty;
        try o.appendSlice(gpa, "{\"error\": ");
        try objcache.jsonString(&o, try proc.latin1ToUtf8(try std.fmt.allocPrint(gpa, fmt, args)));
        try o.appendSlice(gpa, "}");
        r.reply = o.items;
    }

    /// Translate `raw` (a file under the tree) and write it as `dst`; the message when the
    /// translator refuses it.
    fn translateTo(s: *Server, raw: []const u8, dst: []const u8) !?[]const u8 {
        const rel = try proc.relpath(raw, s.repo);
        const t = s.tr.translateChecked(rel) catch |e| {
            s.tr.close();
            s.tr_up = false;
            return try std.fmt.allocPrint(gpa, "the translator failed ({s})", .{@errorName(e)});
        };
        switch (t) {
            .err => |m| return m,
            .text => |text| {
                util.makeDirs(util.dirname(dst));
                try util.writeFile(dst, text);
                return null;
            },
        }
    }

    fn round(s: *Server, reqs: []*Request) !void {
        s.rounds += 1;
        const key = try permute.stateKey();
        if (!std.mem.eql(u8, key, s.key) or !s.tr_up) try s.load();
        const tag = try std.fmt.allocPrint(gpa, "try{d}r{d}", .{ c.getpid(), s.rounds });
        const raw_dir = try util.join(s.build, "try/src");
        const scratch = try std.fmt.allocPrint(gpa, "{s}/try/{s}", .{ s.build, tag });
        util.makeDirs(scratch);
        var tus: std.ArrayList([]const u8) = .empty;
        var srcs: std.ArrayList([]const u8) = .empty;
        var made: std.ArrayList([]const u8) = .empty;
        // TU key -> the slot of its unchanged copy in this round
        var base_slot: std.StringHashMapUnmanaged(usize) = .empty;

        for (reqs, 0..) |r, n| {
            r.row = s.by_start.get(r.addr) orelse {
                try fail(r, "{x:0>8} is not in evidence/funcmap.tsv", .{r.addr});
                continue;
            };
            const src_path = try std.fmt.allocPrint(gpa, "{s}/{s}.cpp", .{ s.repo, r.row.tu });
            const src = (try util.readFileOpt(src_path)) orelse {
                try fail(r, "{s} is not in the tree", .{src_path});
                continue;
            };
            switch (try splice(src, r.addr, r.body, r.helpers)) {
                .err => |m| {
                    try fail(r, "{s}", .{m});
                    continue;
                },
                .ok => |sp| r.splice = sp,
            }
            var d: [32]u8 = undefined;
            std.crypto.hash.sha2.Sha256.hash(src, &d, .{});
            r.src_sha = try std.fmt.allocPrint(gpa, "{s}\x00{s}", .{ r.row.tu, std.fmt.bytesToHex(d, .lower) });
            r.raw = try std.fmt.allocPrint(gpa, "{s}/{s}.{s}n{d}.cpp", .{ raw_dir, r.row.tu, tag, n });
            util.makeDirs(util.dirname(r.raw));
            try util.writeFile(r.raw, r.splice.text);
            try made.append(gpa, r.raw);
            r.trial = try std.fmt.allocPrint(gpa, "{s}/src/{s}.{s}n{d}.cpp", .{ s.build, r.row.tu, tag, n });
            if (try s.translateTo(r.raw, r.trial)) |m| {
                try fail(r, "translation: {s}", .{m});
                continue;
            }
            try made.append(gpa, r.trial);
            r.slot = srcs.items.len;
            try tus.append(gpa, r.row.tu);
            try srcs.append(gpa, r.trial);
            if (s.baselines.get(r.src_sha) == null and base_slot.get(r.src_sha) == null) {
                const dst = try std.fmt.allocPrint(gpa, "{s}/src/{s}.{s}b{d}.cpp", .{ s.build, r.row.tu, tag, n });
                if (try s.translateTo(src_path, dst)) |m| {
                    try fail(r, "translation of the unchanged TU: {s}", .{m});
                    r.slot = null;
                    _ = tus.pop();
                    _ = srcs.pop();
                    continue;
                }
                try made.append(gpa, dst);
                try base_slot.put(gpa, r.src_sha, srcs.items.len);
                try tus.append(gpa, r.row.tu);
                try srcs.append(gpa, dst);
            }
        }
        defer {
            for (made.items) |p| proc.remove(p);
            if (proc.listDir(scratch)) |names| {
                for (names) |nm| proc.remove(util.join(scratch, nm) catch continue);
            } else |_| {}
            if (util.z(scratch)) |sz| {
                _ = c.rmdir(sz.ptr);
            } else |_| {}
        }
        if (srcs.items.len == 0) return;

        const objs = blk: {
            const bench_fd: ?c_int = if (s.bench_lock) |p| try proc.flockPath(p) else null;
            defer if (bench_fd) |fd| proc.closeFd(fd);
            break :blk try permute.compileTrials(tus.items, srcs.items, scratch, s.jobs, BATCH, s.key);
        } orelse {
            // The checkout moved between the translation and the compile: the next round reloads.
            for (reqs) |r| if (r.reply == null) {
                r.retries += 1;
                if (r.retries > 2) try fail(r, "the checkout kept changing under the run (a build or an update)", .{});
            };
            s.key = "";
            return;
        };
        var bit = base_slot.iterator();
        while (bit.next()) |e| {
            var b: Baseline = .empty;
            const obj = objs[e.value_ptr.*];
            const tu = tus.items[e.value_ptr.*];
            if (obj != null) for (try s.judge.evaluate(tu, obj)) |ev| try b.put(gpa, ev.row.s, ev.status);
            try s.baselines.put(gpa, e.key_ptr.*, b);
        }
        const logs = try readLogs(scratch);
        for (reqs) |r| if (r.reply == null) if (r.slot) |slot| {
            r.reply = try s.result(r, objs[slot], logs);
        };
    }

    fn result(s: *Server, r: *Request, obj: ?[]const u8, logs: []const u8) ![]const u8 {
        var o: std.ArrayList(u8) = .empty;
        const base = s.baselines.get(r.src_sha);
        const base_status: []const u8 = if (base) |b| b.get(r.addr) orelse "MISSING" else "COMPILE_ERROR";
        try o.print(gpa, "{{\"addr\": \"{x:0>8}\", \"name\": ", .{r.addr});
        try objcache.jsonString(&o, r.row.name);
        try o.appendSlice(gpa, ", \"tu\": ");
        try objcache.jsonString(&o, r.row.tu);
        if (obj == null) {
            try o.appendSlice(gpa, ", \"status\": \"COMPILE_ERROR\", \"baseline\": ");
            try objcache.jsonString(&o, base_status);
            try o.appendSlice(gpa, ", \"errors\": [");
            var n: usize = 0;
            const name = try proc.lower(util.basename(r.trial));
            var it = util.Lines{ .data = logs };
            while (it.next()) |line| {
                if (n >= MAX_ERRORS) break;
                const low = try proc.lower(line);
                if (!compile.isError(line) or std.mem.indexOf(u8, low, name) == null) continue;
                const at = std.mem.indexOf(u8, low, name).? + name.len;
                var ln: ?u32 = null;
                if (at < line.len and line[at] == '(') if (std.mem.indexOfScalarPos(u8, line, at, ')')) |e| {
                    ln = std.fmt.parseInt(u32, line[at + 1 .. e], 10) catch null;
                };
                if (n > 0) try o.appendSlice(gpa, ", ");
                n += 1;
                try o.appendSlice(gpa, "{\"line\": ");
                if (ln) |l| try o.print(gpa, "{d}", .{l}) else try o.appendSlice(gpa, "null");
                try o.appendSlice(gpa, ", \"body_line\": ");
                if (ln != null and ln.? >= r.splice.body_line and ln.? < r.splice.body_line + r.splice.body_lines)
                    try o.print(gpa, "{d}", .{ln.? - r.splice.body_line + 1})
                else
                    try o.appendSlice(gpa, "null");
                try o.appendSlice(gpa, ", \"text\": ");
                try objcache.jsonString(&o, try proc.latin1ToUtf8(util.strip(line[std.mem.indexOf(u8, low, name).?..])));
                try o.appendSlice(gpa, "}");
            }
            try o.print(gpa, "], \"ms\": {d}}}", .{msSince(r.t0)});
            return o.items;
        }
        const evals = try s.judge.evaluate(r.row.tu, obj);
        var target: ?*const permute.Eval = null;
        for (evals) |*e| if (e.row.s == r.addr) {
            target = e;
        };
        const t = target orelse return error.TargetNotInTu;
        try o.appendSlice(gpa, ", \"status\": ");
        try objcache.jsonString(&o, t.status);
        try o.appendSlice(gpa, ", \"baseline\": ");
        try objcache.jsonString(&o, base_status);
        const res = t.res orelse compare.Result{};
        try o.print(gpa, ", \"our_insns\": {d}, \"ref_insns\": {d}, \"byte_diffs\": {d}, \"ref_diffs\": {d}, \"similarity\": \"", .{ res.our_insns, res.ref_insns, res.byte_diffs, res.ref_diffs });
        try util.fixed(&o, "%.4f", t.sim);
        try o.print(gpa, "\", \"register_only\": {s}, \"frame\": {{\"ours\": {d}, \"ref\": {d}, \"equal\": {s}}}", .{
            if (t.reg) "true" else "false", t.frame_ours, t.frame_ref, if (t.frame_ours == t.frame_ref) "true" else "false",
        });
        try o.appendSlice(gpa, ", \"body_lines\": [");
        if (t.res != null) {
            const lines = try compare.differingLines(t.ov, t.rv);
            std.mem.sort(u32, lines, {}, std.sort.asc(u32));
            var n: usize = 0;
            for (lines) |ln| if (ln >= r.splice.body_line and ln < r.splice.body_line + r.splice.body_lines) {
                if (n > 0) try o.appendSlice(gpa, ", ");
                n += 1;
                try o.print(gpa, "{d}", .{ln - r.splice.body_line + 1});
            };
        }
        try o.appendSlice(gpa, "], \"asm\": [");
        if (t.res != null and !std.mem.eql(u8, t.status, "MATCH")) {
            const all = try compare.sideBySide(&s.judge.viewer, t.ov, t.rv, DIFF_WIDTH);
            const keep = try gpa.alloc(bool, all.len);
            @memset(keep, false);
            for (all, 0..) |l, k| if (l.len > DIFF_WIDTH + 1 and l[DIFF_WIDTH + 1] != ' ') {
                const lo = k -| r.context;
                const hi = @min(all.len, k + r.context + 1);
                @memset(keep[lo..hi], true);
            };
            var first = true;
            var gap = false;
            for (all, 0..) |l, k| {
                if (!keep[k]) {
                    gap = true;
                    continue;
                }
                if (!first) try o.appendSlice(gpa, ", ");
                if (gap and !first) try o.appendSlice(gpa, "\"...\", ");
                first = false;
                gap = false;
                try objcache.jsonString(&o, try proc.latin1ToUtf8(std.mem.trimEnd(u8, try bodyRelative(l, r.splice), " ")));
            }
        }
        try o.appendSlice(gpa, "], \"collateral\": [");
        var n: usize = 0;
        for (evals) |*e| if (e.row.s != r.addr) {
            const before: []const u8 = if (base) |b| b.get(e.row.s) orelse "MISSING" else "COMPILE_ERROR";
            if (std.mem.eql(u8, before, e.status)) continue;
            if (n > 0) try o.appendSlice(gpa, ", ");
            n += 1;
            try o.print(gpa, "{{\"addr\": \"{x:0>8}\", \"name\": ", .{e.row.s});
            try objcache.jsonString(&o, e.row.name);
            try o.appendSlice(gpa, ", \"before\": ");
            try objcache.jsonString(&o, before);
            try o.appendSlice(gpa, ", \"after\": ");
            try objcache.jsonString(&o, e.status);
            try o.appendSlice(gpa, "}");
        };
        try o.print(gpa, "], \"ms\": {d}}}", .{msSince(r.t0)});
        return o.items;
    }
};

/// A sideBySide line with our source line (columns 6..11, the spliced TU's) given as the body
/// file's line, blank outside the body.
fn bodyRelative(line: []const u8, sp: Splice) ![]const u8 {
    if (line.len < 12) return line;
    const field = std.mem.trim(u8, line[6..11], " ");
    if (field.len == 0) return line;
    const ln = std.fmt.parseInt(u32, field, 10) catch return line;
    var out = try gpa.dupe(u8, line);
    var buf: [5]u8 = .{ ' ', ' ', ' ', ' ', ' ' };
    if (ln >= sp.body_line and ln < sp.body_line + sp.body_lines) {
        var tmp: [5]u8 = undefined;
        const txt = std.fmt.bufPrint(&tmp, "{d}", .{ln - sp.body_line + 1}) catch return line;
        @memcpy(buf[5 - txt.len ..], txt);
    }
    @memcpy(out[6..11], &buf);
    return out;
}

test "bodyRelative renumbers our column" {
    const sp = Splice{ .text = "", .body_line = 1039, .body_lines = 3 };
    try std.testing.expectEqualStrings("    3     2 mov eax", try bodyRelative("    3  1040 mov eax", sp));
    try std.testing.expectEqualStrings("    3       mov eax", try bodyRelative("    3  1090 mov eax", sp));
}

fn msSince(t0: f64) u64 {
    return @intFromFloat(@max(0, (proc.now() - t0) * 1000));
}

/// Every compiler log a round left (a failed batch leaves one per cl.exe).
fn readLogs(dir: []const u8) ![]const u8 {
    var out: std.ArrayList(u8) = .empty;
    for (try proc.listDir(dir)) |nm| if (std.mem.endsWith(u8, nm, ".log")) {
        try out.appendSlice(gpa, try util.readFile(try util.join(dir, nm)));
        try out.append(gpa, '\n');
    };
    return out.items;
}

fn writeAll(fd: c_int, data: []const u8) void {
    var p = data;
    while (p.len > 0) {
        const n = c.write(fd, p.ptr, p.len);
        if (n < 0) {
            if (std.c._errno().* == c.EINTR) continue;
            return;
        }
        p = p[@intCast(n)..];
    }
}

/// Bytes from `fd` up to the first newline (excluded) or end of file; null past MAX_REQUEST.
fn readLine(fd: c_int) !?[]const u8 {
    var buf: std.ArrayList(u8) = .empty;
    var chunk: [65536]u8 = undefined;
    while (true) {
        if (std.mem.indexOfScalar(u8, buf.items, '\n')) |nl| return buf.items[0..nl];
        if (buf.items.len > MAX_REQUEST) return null;
        const n = c.read(fd, &chunk, chunk.len);
        if (n < 0) {
            if (std.c._errno().* == c.EINTR) continue;
            return error.Read;
        }
        if (n == 0) return buf.items;
        try buf.appendSlice(gpa, chunk[0..@intCast(n)]);
    }
}

fn reader(s: *Server, fd: c_int) void {
    readRequest(s, fd) catch {
        writeAll(fd, "{\"error\": \"bad request\"}\n");
        _ = c.close(fd);
    };
}

fn readRequest(s: *Server, fd: c_int) !void {
    const line = (try readLine(fd)) orelse return error.TooLarge;
    const v = try std.json.parseFromSliceLeaky(std.json.Value, gpa, line, .{});
    const obj = v.object;
    const addr_s = (obj.get("addr") orelse return error.NoAddress).string;
    const a = if (std.mem.startsWith(u8, addr_s, "0x")) addr_s[2..] else addr_s;
    const r = try gpa.create(Request);
    r.* = .{
        .fd = fd,
        .addr = try std.fmt.parseInt(u32, a, 16),
        .body = (obj.get("body") orelse return error.NoBody).string,
        .helpers = if (obj.get("helpers")) |h| (if (h == .string) h.string else null) else null,
        .context = if (obj.get("context")) |x| (if (x == .integer) @intCast(x.integer) else DEFAULT_CONTEXT) else DEFAULT_CONTEXT,
        .t0 = proc.now(),
    };
    s.lock.lock();
    try s.queue.append(gpa, r);
    _ = c.pthread_cond_signal(&s.cond);
    s.lock.unlock();
}

fn socketPath(build: []const u8) ![]const u8 {
    const p = try util.join(build, "try-source.sock");
    const room = @typeInfo(@FieldType(c.struct_sockaddr_un, "sun_path")).array.len;
    if (p.len >= room) cli.exitMsg("try-source: the socket path {s} is longer than {d} bytes", .{ p, room - 1 });
    return p;
}

fn sockAddr(path: []const u8) c.struct_sockaddr_un {
    var sa = std.mem.zeroes(c.struct_sockaddr_un);
    sa.sun_family = c.AF_UNIX;
    @memcpy(sa.sun_path[0..path.len], path);
    return sa;
}

fn unixSocket() !c_int {
    const fd = c.socket(c.AF_UNIX, c.SOCK_STREAM, 0);
    if (fd < 0) return error.Socket;
    _ = c.fcntl(fd, c.F_SETFD, c.FD_CLOEXEC);
    return fd;
}

fn serveMain(args: []const []const u8) !void {
    var root_arg: ?[]const u8 = null;
    var idle: f64 = DEFAULT_IDLE;
    var max_requests: usize = DEFAULT_MAX_REQUESTS;
    var k: usize = 0;
    while (k < args.len) : (k += 2) {
        if (k + 1 >= args.len) usage();
        if (std.mem.eql(u8, args[k], "--root")) root_arg = args[k + 1] else if (std.mem.eql(u8, args[k], "--idle")) idle = try std.fmt.parseFloat(f64, args[k + 1]) else if (std.mem.eql(u8, args[k], "--max-requests")) max_requests = try std.fmt.parseInt(usize, args[k + 1], 10) else usage();
    }
    const root = try cli.root(root_arg);
    try compile.configure(root);
    // One server per build: the loser of the lock leaves the socket to the winner.
    const lock_path = try util.join(compile.BUILD, "try-source.lock");
    const lfd = c.open((try util.z(lock_path)).ptr, c.O_WRONLY | c.O_CREAT | c.O_CLOEXEC, @as(c_uint, 0o666));
    if (lfd < 0) return error.Open;
    if (c.flock(lfd, c.LOCK_EX | c.LOCK_NB) != 0) return;
    const path = try socketPath(compile.BUILD);
    proc.remove(path);
    const lsock = try unixSocket();
    var sa = sockAddr(path);
    if (c.bind(lsock, @ptrCast(&sa), @sizeOf(c.struct_sockaddr_un)) != 0) cli.exitMsg("try-serve: cannot bind {s}", .{path});
    if (c.listen(lsock, 128) != 0) cli.exitMsg("try-serve: cannot listen on {s}", .{path});
    // A client gone before its reply must not take the server with it: the write fails instead.
    _ = c.signal(c.SIGPIPE, ignoreSignal);

    const jobs_env = util.getenv("D2_VC8_JOBS");
    const bench = util.getenv("WINE_BENCH_LOCK");
    var s = Server{
        .root = root,
        .repo = try proc.realpath(compile.REPO),
        .build = compile.BUILD,
        .jobs = if (jobs_env) |j| std.fmt.parseInt(usize, j, 10) catch DEFAULT_JOBS else DEFAULT_JOBS,
        .bench_lock = if (bench != null and bench.?.len > 0) bench.? else null,
    };
    s.lock.init();
    _ = c.pthread_cond_init(&s.cond, null);
    std.debug.print("try-serve: pid {d}, {s}, {d} jobs, bench lock {s}\n", .{ c.getpid(), path, s.jobs, s.bench_lock orelse "-" });
    try s.load();
    std.debug.print("try-serve: ready\n", .{});
    const t = try std.Thread.spawn(.{}, worker, .{ &s, idle, max_requests, lsock, path });
    t.detach();
    while (true) {
        const fd = c.accept(lsock, null, null);
        if (fd < 0) {
            if (std.c._errno().* == c.EINTR) continue;
            return;
        }
        _ = c.fcntl(fd, c.F_SETFD, c.FD_CLOEXEC);
        const rt = std.Thread.spawn(.{}, reader, .{ &s, fd }) catch {
            _ = c.close(fd);
            continue;
        };
        rt.detach();
    }
}

fn ignoreSignal(_: c_int) callconv(.c) void {}

fn deadline(at: f64) c.struct_timespec {
    const sec = @floor(at);
    return .{ .tv_sec = @intFromFloat(sec), .tv_nsec = @intFromFloat((at - sec) * 1e9) };
}

fn worker(s: *Server, idle: f64, max_requests: usize, lsock: c_int, path: []const u8) void {
    var served: usize = 0;
    var last = proc.now();
    while (true) {
        s.lock.lock();
        while (s.queue.items.len == 0) {
            if (served >= max_requests or proc.now() - last >= idle) {
                // Nothing waits: stop taking connections, then go. A client that connected
                // in between sees the socket close and starts a new server.
                _ = c.close(lsock);
                proc.remove(path);
                s.lock.unlock();
                std.debug.print("try-serve: exiting after {d} requests\n", .{served});
                std.process.exit(0);
            }
            var ts = deadline(last + idle);
            _ = c.pthread_cond_timedwait(&s.cond, &s.lock.m, &ts);
        }
        const cap = s.jobs * BATCH / 2;
        const gather_end = proc.now() + GATHER_MAX_S;
        while (s.queue.items.len < cap) {
            const had = s.queue.items.len;
            var ts = deadline(@min(gather_end, proc.now() + GATHER_QUIET_S));
            _ = c.pthread_cond_timedwait(&s.cond, &s.lock.m, &ts);
            if (s.queue.items.len == had or proc.now() >= gather_end) break;
        }
        const n = @min(cap, s.queue.items.len);
        const batch = gpa.dupe(*Request, s.queue.items[0..n]) catch {
            s.lock.unlock();
            continue;
        };
        std.mem.copyForwards(*Request, s.queue.items[0 .. s.queue.items.len - n], s.queue.items[n..]);
        s.queue.shrinkRetainingCapacity(s.queue.items.len - n);
        s.lock.unlock();

        const t0 = proc.now();
        s.round(batch) catch |e| {
            for (batch) |r| if (r.reply == null) Server.fail(r, "try-serve: {s}", .{@errorName(e)}) catch {};
        };
        var again: std.ArrayList(*Request) = .empty;
        for (batch) |r| {
            if (r.reply) |reply| {
                writeAll(r.fd, reply);
                writeAll(r.fd, "\n");
                _ = c.close(r.fd);
                served += 1;
            } else again.append(gpa, r) catch {};
        }
        std.debug.print("try-serve: round {d}: {d} requests in {d} ms\n", .{ s.rounds, n, msSince(t0) });
        if (again.items.len > 0) {
            s.lock.lock();
            s.queue.insertSlice(gpa, 0, again.items) catch {};
            s.lock.unlock();
        }
        last = proc.now();
    }
}

// ---- the client

fn connectTo(path: []const u8) ?c_int {
    const fd = unixSocket() catch return null;
    var sa = sockAddr(path);
    if (c.connect(fd, @ptrCast(&sa), @sizeOf(c.struct_sockaddr_un)) == 0) return fd;
    _ = c.close(fd);
    return null;
}

/// Start the server in its own session, its output appended to build/try-source.log.
fn startServer(root: []const u8) !void {
    const self = try util.z(try cli.selfExe());
    const argv = [_:null]?[*:0]const u8{ self.ptr, "try-serve", "--root", (try util.z(root)).ptr, null };
    const log = try util.z(try util.join(compile.BUILD, "try-source.log"));
    const pid = c.fork();
    if (pid < 0) return error.Fork;
    if (pid > 0) return;
    _ = c.setsid();
    const null_fd = c.open("/dev/null", c.O_RDONLY);
    const log_fd = c.open(log.ptr, c.O_WRONLY | c.O_CREAT | c.O_APPEND, @as(c_uint, 0o666));
    if (null_fd >= 0) _ = c.dup2(null_fd, 0);
    if (log_fd >= 0) {
        _ = c.dup2(log_fd, 1);
        _ = c.dup2(log_fd, 2);
    }
    _ = c.execv(self.ptr, @ptrCast(&argv));
    c._exit(127);
}

fn sleepMs(ms: u64) void {
    var ts = c.struct_timespec{ .tv_sec = @intCast(ms / 1000), .tv_nsec = @intCast((ms % 1000) * 1_000_000) };
    _ = c.nanosleep(&ts, null);
}

fn clientMain(args: []const []const u8) !void {
    var root_arg: ?[]const u8 = null;
    var helpers_path: ?[]const u8 = null;
    var expect: ?[]const u8 = null;
    var context: usize = DEFAULT_CONTEXT;
    var pos: std.ArrayList([]const u8) = .empty;
    var k: usize = 0;
    while (k < args.len) : (k += 1) {
        const a = args[k];
        const opt = std.mem.eql(u8, a, "--root") or std.mem.eql(u8, a, "--helpers") or std.mem.eql(u8, a, "--expect") or std.mem.eql(u8, a, "--context");
        if (!opt) {
            try pos.append(gpa, a);
            continue;
        }
        if (k + 1 >= args.len) usage();
        const v = args[k + 1];
        k += 1;
        if (std.mem.eql(u8, a, "--root")) root_arg = v else if (std.mem.eql(u8, a, "--helpers")) helpers_path = v else if (std.mem.eql(u8, a, "--expect")) expect = v else context = try std.fmt.parseInt(usize, v, 10);
    }
    if (pos.items.len != 2) usage();
    const root = try cli.root(root_arg);
    try compile.configure(root);
    const body = try util.readFile(pos.items[1]);
    var req: std.ArrayList(u8) = .empty;
    try req.appendSlice(gpa, "{\"addr\": ");
    try objcache.jsonString(&req, pos.items[0]);
    try req.appendSlice(gpa, ", \"body\": ");
    try objcache.jsonString(&req, body);
    if (helpers_path) |h| {
        try req.appendSlice(gpa, ", \"helpers\": ");
        try objcache.jsonString(&req, try util.readFile(h));
    }
    try req.print(gpa, ", \"context\": {d}}}\n", .{context});

    const path = try socketPath(compile.BUILD);
    var reply: []const u8 = "";
    var attempt: usize = 0;
    while (reply.len == 0) : (attempt += 1) {
        if (attempt == 3) cli.exitMsg("try-source: the server closed the connection three times; see {s}/try-source.log", .{compile.BUILD});
        var fd = connectTo(path);
        if (fd == null) {
            try startServer(root);
            // The server binds before it loads, so this waits for its start-up only.
            var waited: u64 = 0;
            while (fd == null and waited < 60_000) : (waited += 100) {
                sleepMs(100);
                fd = connectTo(path);
            }
            if (fd == null) cli.exitMsg("try-source: no server on {s}; see {s}/try-source.log", .{ path, compile.BUILD });
        }
        writeAll(fd.?, req.items);
        reply = (try readLine(fd.?)) orelse "";
        _ = c.close(fd.?);
    }
    util.say(reply);
    util.say("\n");
    const v = std.json.parseFromSliceLeaky(std.json.Value, gpa, reply, .{}) catch std.process.exit(2);
    if (v != .object or v.object.get("error") != null) std.process.exit(2);
    if (expect) |want| {
        const st = v.object.get("status") orelse std.process.exit(1);
        if (st != .string or !std.mem.eql(u8, st.string, want)) std.process.exit(1);
    }
}

pub fn main(cmd: []const u8, args: []const []const u8) !void {
    if (builtin.os.tag == .windows) cli.exitMsg("{s}: POSIX hosts only (wine)", .{cmd});
    if (std.mem.eql(u8, cmd, "try-serve")) return serveMain(args);
    return clientMain(args);
}
