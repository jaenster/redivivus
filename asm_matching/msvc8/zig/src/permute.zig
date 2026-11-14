//! The oracle side of ghidra-reconstruct's vc8form form permuter
//! (scripts/vc8form-permute/permute.ts): compile trial sources beside a built tree and judge
//! the objects against the original, the way the audit judges.
//!
//!   oracle-zig trial --root <asm_matching/msvc8> --state
//!       print the key of what every trial compiles against (cl.rsp, the precompiled
//!       header's key, mirror's key, the tree's commit), a tab, and the build directory
//!   oracle-zig trial --root <asm_matching/msvc8> --out DIR [-j N] [--batch N] [--expect KEY] < trials
//!       stdin: one `tu<TAB>source` per trial, the source a translated TU written beside its
//!       own in build/src. Under the build's lock (build.py's and `oracle-zig build`'s), and
//!       only while the key is still KEY (else exit 3), the trials are compiled with the
//!       build's command line and precompiled header, grouped by their flags.tsv flags, in
//!       batches of up to N per cl.exe (spread over every job when there are few), into
//!       DIR/<source name>.obj; a batch the compiler failed leaves DIR/<first source>.log.
//!       Prints, per trial in order, the object's path or `-` when there is no valid one.
//!   oracle-zig judge --root <asm_matching/msvc8>
//!       one JSON request per stdin line, one reply per stdout line:
//!       {"tu": T, "obj": O|null}  ->  {"fns": [[start, status, "%.4f similarity",
//!            register-only, [called symbols], our frame, the original's frame], ...]}: every
//!            funcmap function of T, in funcmap order; register-only is true for an INSN_MATCH
//!            whose differing instructions differ in register names alone; the frames are the
//!            `sub esp, N` of each prologue as classify's frame tag reads them (0 for none)
//!       {"bh": [[start, size], ...]}  ->  {"bh": ["size-sha256[:8]", ...]}: the original's
//!            bytes with every reference masked, as a vc8form tag's bh records them
const std = @import("std");
const builtin = @import("builtin");
const c = @import("c");
const util = @import("util.zig");
const proc = @import("proc.zig");
const compile = @import("compile.zig");
const audit = @import("audit.zig");
const classify = @import("classify.zig");
const compare = @import("compare.zig");
const dec = @import("decode.zig");
const objfuncs = @import("objfuncs.zig");
const ours = @import("ours.zig");
const refimage = @import("refimage.zig");
const objcache = @import("objcache.zig");
const gpa = util.gpa;

const BATCH = 16;
const TIMEOUT_PER_TRIAL = 300.0;

fn usage() noreturn {
    std.debug.print(
        \\usage: oracle-zig trial --root <asm_matching/msvc8> --state
        \\       oracle-zig trial --root <asm_matching/msvc8> --out DIR [-j N] [--batch N] [--expect KEY] < trials
        \\       oracle-zig judge --root <asm_matching/msvc8> < requests
        \\
    , .{});
    std.process.exit(2);
}

fn readStdin() ![]const u8 {
    var input: std.ArrayList(u8) = .empty;
    var chunk: [65536]u8 = undefined;
    while (true) {
        const n = c.read(0, &chunk, chunk.len);
        if (n <= 0) break;
        try input.appendSlice(gpa, chunk[0..@intCast(n)]);
    }
    return input.items;
}

// ---- trial

/// sha256 over cl.rsp, pch/oracle.key, mirror.key (each `-` when missing) and the tree's
/// HEAD: a build or a checkout move meanwhile makes every baseline stale.
fn stateKey() ![]const u8 {
    var h = std.crypto.hash.sha2.Sha256.init(.{});
    for ([_][]const u8{ "cl.rsp", "pch/oracle.key", "mirror.key" }) |p| {
        if (try util.readFileOpt(try util.join(compile.BUILD, p))) |d| h.update(d) else h.update("-");
    }
    const r = try proc.run(&.{ "git", "-C", compile.REPO, "rev-parse", "HEAD" }, .{});
    h.update(if (r.status == 0) util.strip(r.out) else "-");
    var d: [32]u8 = undefined;
    h.final(&d);
    return gpa.dupe(u8, &std.fmt.bytesToHex(d, .lower));
}

const Work = struct { ks: []const usize, tu: []const u8 };

const Round = struct {
    work: []const Work,
    srcs: []const []const u8,
    rsp: []const u8,
    out_win: []const u8,
    scratch: []const u8,
    next: std.atomic.Value(usize) = .init(0),
    err: ?anyerror = null,
    lock: proc.Mutex = .{},
};

fn stem(path: []const u8) []const u8 {
    const b = util.basename(path);
    return if (std.mem.endsWith(u8, b, ".cpp")) b[0 .. b.len - 4] else b;
}

fn runBatch(r: *Round, w: Work) !void {
    var srcs: std.ArrayList([]const u8) = .empty;
    for (w.ks) |k| try srcs.append(gpa, r.srcs[k]);
    const cmd = try compile.clCommand(r.rsp, try std.mem.concat(gpa, u8, &.{ w.tu, ".cpp" }), r.out_win, srcs.items);
    const p = try proc.run(cmd, .{ .timeout_s = TIMEOUT_PER_TRIAL * @as(f64, @floatFromInt(w.ks.len)) });
    if (p.timed_out) return;
    const out = try proc.universalNewlines(p.out);
    if ((p.status != 0 and p.status != 137 and p.status != -9) or std.mem.indexOf(u8, out, "error") != null) {
        const text = try std.mem.concat(gpa, u8, &.{ out, try proc.universalNewlines(p.err) });
        const log = try std.fmt.allocPrint(gpa, "{s}/{s}.log", .{ r.scratch, stem(r.srcs[w.ks[0]]) });
        try util.writeFile(log, try proc.latin1ToUtf8(text));
    }
}

fn worker(r: *Round) void {
    while (true) {
        const i = r.next.fetchAdd(1, .monotonic);
        if (i >= r.work.len) return;
        runBatch(r, r.work[i]) catch |e| {
            r.lock.lock();
            r.err = e;
            r.lock.unlock();
        };
    }
}

fn trialOutputDir(scratch: []const u8) ![]u8 {
    const path = if (builtin.os.tag == .windows)
        try std.mem.concat(gpa, u8, &.{ scratch, "/" })
    else
        try std.mem.concat(gpa, u8, &.{ "Z:", scratch, "/" });
    std.mem.replaceScalar(u8, path, '/', '\\');
    return path;
}

test "trial output directory uses host path on Windows" {
    const path = try trialOutputDir("C:/scratch");
    if (builtin.os.tag == .windows)
        try std.testing.expectEqualStrings("C:\\scratch\\", path)
    else
        try std.testing.expectEqualStrings("Z:C:\\scratch\\", path);
}

fn trialMain(args: []const []const u8) !void {
    var root: ?[]const u8 = null;
    var out_dir: ?[]const u8 = null;
    var expect: ?[]const u8 = null;
    var jobs: usize = 4;
    var batch: usize = BATCH;
    var state_only = false;
    var k: usize = 0;
    while (k < args.len) : (k += 1) {
        const a = args[k];
        if (std.mem.eql(u8, a, "--state")) {
            state_only = true;
            continue;
        }
        if (k + 1 >= args.len) usage();
        if (std.mem.eql(u8, a, "--root")) root = args[k + 1] else if (std.mem.eql(u8, a, "--out")) out_dir = args[k + 1] else if (std.mem.eql(u8, a, "--expect")) expect = args[k + 1] else if (std.mem.eql(u8, a, "-j") or std.mem.eql(u8, a, "--jobs")) jobs = try std.fmt.parseInt(usize, args[k + 1], 10) else if (std.mem.eql(u8, a, "--batch")) batch = try std.fmt.parseInt(usize, args[k + 1], 10) else usage();
        k += 1;
    }
    try compile.configure(root orelse usage());
    if (state_only) {
        util.say(try std.fmt.allocPrint(gpa, "{s}\t{s}\n", .{ try stateKey(), compile.BUILD }));
        return;
    }
    const scratch = try proc.realpath(out_dir orelse usage());
    var tus: std.ArrayList([]const u8) = .empty;
    var srcs: std.ArrayList([]const u8) = .empty;
    var it = util.Lines{ .data = try readStdin() };
    while (it.next()) |line| {
        if (line.len == 0) continue;
        const tab = std.mem.indexOfScalar(u8, line, '\t') orelse usage();
        try tus.append(gpa, line[0..tab]);
        try srcs.append(gpa, line[tab + 1 ..]);
    }
    // Grouped by the flags each TU compiles with, in first-seen order.
    var groups: std.StringArrayHashMapUnmanaged(std.ArrayList(usize)) = .empty;
    for (tus.items, 0..) |tu, i| {
        const g = try groups.getOrPut(gpa, compile.extraFlags(try std.mem.concat(gpa, u8, &.{ tu, ".cpp" })));
        if (!g.found_existing) g.value_ptr.* = .empty;
        try g.value_ptr.append(gpa, i);
    }
    // A small round is spread over every job: one cl.exe per 16 trials would leave most of
    // them idle when a step has a few dozen.
    const n = tus.items.len;
    const size = @max(1, @min(batch, (n + jobs - 1) / @max(1, jobs)));
    var work: std.ArrayList(Work) = .empty;
    for (groups.values()) |ks| {
        var i: usize = 0;
        while (i < ks.items.len) : (i += size) {
            const part = ks.items[i..@min(ks.items.len, i + size)];
            try work.append(gpa, .{ .ks = part, .tu = tus.items[part[0]] });
        }
    }
    const out_win = try trialOutputDir(scratch);
    var round = Round{ .work = work.items, .srcs = srcs.items, .rsp = try util.join(compile.BUILD, "cl.rsp"), .out_win = out_win, .scratch = scratch };
    round.lock.init();
    {
        const fd = try proc.flockPath(try util.join(compile.BUILD, ".build.lock"));
        defer proc.closeFd(fd);
        if (expect) |e| if (!std.mem.eql(u8, e, try stateKey())) {
            std.debug.print("the checkout changed under the run (a build or an update): start again\n", .{});
            std.process.exit(3);
        };
        var threads: std.ArrayList(std.Thread) = .empty;
        for (0..@max(@as(usize, 1), @min(jobs, work.items.len))) |_| try threads.append(gpa, try std.Thread.spawn(.{}, worker, .{&round}));
        for (threads.items) |t| t.join();
    }
    if (round.err) |e| return e;
    var o: std.ArrayList(u8) = .empty;
    for (srcs.items) |s| {
        const obj = try std.fmt.allocPrint(gpa, "{s}/{s}.obj", .{ scratch, stem(s) });
        try o.appendSlice(gpa, if (util.exists(obj) and compile.validObject(obj)) obj else "-");
        try o.append(gpa, '\n');
    }
    util.say(o.items);
}

// ---- judge

const REGS = [_][]const u8{ "eax", "ebx", "ecx", "edx", "esi", "edi", "ebp", "esp", "ax", "bx", "cx", "dx", "si", "di", "bp", "sp", "al", "ah", "bl", "bh", "cl", "ch", "dl", "dh" };

fn isReg(w: []const u8) bool {
    for (REGS) |r| if (std.mem.eql(u8, r, w)) return true;
    return false;
}

/// re.findall(r'\w+', text.lower())
fn words(text: []const u8) ![]const []const u8 {
    const low = try proc.lower(text);
    var out: std.ArrayList([]const u8) = .empty;
    var i: usize = 0;
    while (i < low.len) {
        if (!wordByte(low[i])) {
            i += 1;
            continue;
        }
        const s = i;
        while (i < low.len and wordByte(low[i])) i += 1;
        try out.append(gpa, low[s..i]);
    }
    return out.items;
}

fn wordByte(ch: u8) bool {
    return std.ascii.isAlphanumeric(ch) or ch == '_' or ch >= 0x80;
}

/// An INSN_MATCH whose differing instructions differ in register names alone.
fn registerOnly(v: *const compare.Viewer, ov: []const compare.NInsn, rv: []const compare.NInsn) !bool {
    for (ov[0..@min(ov.len, rv.len)], rv[0..@min(ov.len, rv.len)]) |x, y| {
        if (std.mem.eql(u8, x.masked, y.masked)) continue;
        const a = try words(v.intern.str(x.text));
        const b = try words(v.intern.str(y.text));
        if (a.len != b.len) return false;
        for (a, b) |p, q| if (!std.mem.eql(u8, p, q) and !(isReg(p) and isReg(q))) return false;
    }
    return true;
}

const Judge = struct {
    ref: *refimage.Reference,
    model: compare.Model,
    by_tu: std.StringHashMapUnmanaged(std.ArrayList(audit.MapRow)) = .empty,
    syms: audit.SymMap,
    quarantined: std.StringHashMapUnmanaged([]const u8),
    d: dec.Decoder,
    viewer: compare.Viewer = undefined,

    fn judge(j: *Judge, out: *std.ArrayList(u8), tu: []const u8, obj: ?[]const u8) !void {
        var fns: std.StringHashMapUnmanaged(*const ours.OurFunc) = .empty;
        var by_name: std.StringHashMapUnmanaged(std.ArrayList(*const ours.OurFunc)) = .empty;
        if (obj) |path| if (util.exists(path)) {
            const data = try util.readFile(path);
            const list = try objfuncs.functions(&j.d, path, data, false);
            for (list, 0..) |f, k| {
                const o = try gpa.create(ours.OurFunc);
                o.* = .{ .tu = tu, .order = k, .f = f };
                const g = try fns.getOrPut(gpa, f.symbol);
                if (!g.found_existing) g.value_ptr.* = o;
            }
            var it = fns.valueIterator();
            while (it.next()) |o| {
                const g = try by_name.getOrPut(gpa, o.*.f.name);
                if (!g.found_existing) g.value_ptr.* = .empty;
                try g.value_ptr.append(gpa, o.*);
            }
        };
        try out.appendSlice(gpa, "{\"fns\": [");
        const rows: []const audit.MapRow = if (j.by_tu.get(tu)) |l| l.items else &.{};
        for (rows, 0..) |r, n| {
            if (n > 0) try out.appendSlice(gpa, ", ");
            var f = fns.get(r.sym);
            if (f == null) if (by_name.get(r.name)) |l| if (l.items.len == 1) {
                f = l.items[0];
            };
            var status: []const u8 = "MISSING";
            var sim: f64 = 0;
            var reg = false;
            var frame_ours: u64 = 0;
            var frame_ref: u64 = 0;
            if (f != null and j.model.contains(r.s)) {
                frame_ours = try classify.frameSize(f.?.f.decoded.insns.items);
                frame_ref = try classify.frameSize(j.model.getPtr(r.s).?.f.insns.items);
                if (j.quarantined.get(try audit.qkey(r.tu, audit.lastPart(r.name))) != null) {
                    status = "QUARANTINE";
                } else {
                    const ov = try j.viewer.ourView(f.?, &j.syms);
                    const rv = try j.viewer.refViewSpan(r.s, r.e, &j.model);
                    const res = try compare.compare(ov, rv);
                    status = res.status;
                    sim = res.similarity;
                    reg = std.mem.eql(u8, status, "INSN_MATCH") and try registerOnly(&j.viewer, ov, rv);
                }
            }
            try out.print(gpa, "[{d}, ", .{r.s});
            try objcache.jsonString(out, status);
            try out.appendSlice(gpa, ", \"");
            try util.fixed(out, "%.4f", sim);
            try out.print(gpa, "\", {s}, [", .{if (reg) "true" else "false"});
            if (f) |o| {
                var first = true;
                for (o.f.refs) |ref| if (ref.kind == .call) {
                    if (!first) try out.appendSlice(gpa, ", ");
                    first = false;
                    try objcache.jsonString(out, ref.s);
                };
            }
            try out.print(gpa, "], {d}, {d}]", .{ frame_ours, frame_ref });
        }
        try out.appendSlice(gpa, "]}\n");
    }

    fn bh(j: *Judge, out: *std.ArrayList(u8), start: u32, size: u32) !void {
        var b: std.ArrayList(u8) = .empty;
        try b.resize(gpa, size);
        if (!j.ref.img.read(start, b.items)) @memset(b.items, 0);
        if (j.model.getPtr(start)) |e| for (e.refs) |r| {
            const at = @as(i64, r.a) - start + r.off;
            if (at >= 0 and at < size) {
                // b[k:k + n] = b'\0' * n: a reference running past the end grows the bytes.
                const lo: usize = @intCast(at);
                if (lo + r.size > b.items.len) try b.resize(gpa, lo + r.size);
                @memset(b.items[lo .. lo + r.size], 0);
            }
        };
        var d: [32]u8 = undefined;
        std.crypto.hash.sha2.Sha256.hash(b.items, &d, .{});
        try out.print(gpa, "\"{d}-{s}\"", .{ size, std.fmt.bytesToHex(d, .lower)[0..8] });
    }
};

fn judgeMain(args: []const []const u8) !void {
    var root: ?[]const u8 = null;
    var k: usize = 0;
    while (k < args.len) : (k += 1) {
        if (std.mem.eql(u8, args[k], "--root") and k + 1 < args.len) {
            root = args[k + 1];
            k += 1;
        } else usage();
    }
    const cfg = try audit.Config.init(root orelse usage());
    const ref = try refimage.Reference.open(try util.join(cfg.oracle, "reference/Game.exe"));
    try ref.load(try util.join(cfg.refcache, "functions.tsv"));
    var j = Judge{
        .ref = ref,
        .model = try ref.model(),
        .syms = undefined,
        .quarantined = try audit.loadQuarantine(try util.join(cfg.build, "quarantine.tsv")),
        .d = try dec.Decoder.init(gpa),
    };
    const rows = try audit.loadFuncmap(try cfg.evidence("funcmap.tsv"));
    const libmap_file = try cfg.evidence("libmap.tsv");
    const librows: []audit.MapRow = if (util.exists(libmap_file)) try audit.loadFuncmap(libmap_file) else &.{};
    j.syms = try audit.symMap(rows, librows, try audit.loadCrtmap(try cfg.evidence("crtmap.tsv")), &ref.img, &j.model);
    for (rows) |r| {
        const g = try j.by_tu.getOrPut(gpa, r.tu);
        if (!g.found_existing) g.value_ptr.* = .empty;
        try g.value_ptr.append(gpa, r);
    }
    j.viewer = .{ .d = &j.d, .with_text = true };
    util.say("ready\n");

    // One request per line, answered as it comes.
    var pending: std.ArrayList(u8) = .empty;
    var chunk: [65536]u8 = undefined;
    while (true) {
        const n = c.read(0, &chunk, chunk.len);
        if (n <= 0) break;
        try pending.appendSlice(gpa, chunk[0..@intCast(n)]);
        while (std.mem.indexOfScalar(u8, pending.items, '\n')) |nl| {
            const line = try gpa.dupe(u8, pending.items[0..nl]);
            const rest = try gpa.dupe(u8, pending.items[nl + 1 ..]);
            pending.clearRetainingCapacity();
            try pending.appendSlice(gpa, rest);
            if (util.strip(line).len == 0) continue;
            var out: std.ArrayList(u8) = .empty;
            const v = try std.json.parseFromSliceLeaky(std.json.Value, gpa, line, .{});
            if (v.object.get("bh")) |list| {
                try out.appendSlice(gpa, "{\"bh\": [");
                for (list.array.items, 0..) |p, i| {
                    if (i > 0) try out.appendSlice(gpa, ", ");
                    try j.bh(&out, @intCast(p.array.items[0].integer), @intCast(p.array.items[1].integer));
                }
                try out.appendSlice(gpa, "]}\n");
            } else {
                const tu = v.object.get("tu").?.string;
                const obj = v.object.get("obj");
                try j.judge(&out, tu, if (obj != null and obj.? == .string) obj.?.string else null);
            }
            util.say(out.items);
        }
    }
}

pub fn main(cmd: []const u8, args: []const []const u8) !void {
    if (std.mem.eql(u8, cmd, "trial")) return trialMain(args);
    return judgeMain(args);
}
