//! The VC8 translation of the tree: ghidra-reconstruct's TypeScript CLI
//! (packages/reconstruct/src/vc8), or the translated tree an export ships beside the oracle.
//!
//! The translator: GR=<checkout> selects one; without GR the live daemon's checkout
//! (~/code/ts/gr-daemon) when it has the VC8 mode and a built cpp-parser. One process serves
//! the whole build: it mirrors the tree into build/src, then translates single files on
//! request (a quarantine round, a support TU) with the same per-tree state.
//!
//! The shipped tree: `oracle-zig translate` writes the translator's mirror of a tree with
//! `sources.tsv`, the hash of each source it was translated from. The public repository
//! carries one as asm_matching/msvc8/translated/, so a checkout with no translator (CI)
//! compiles exactly what the translator wrote. A shipped tree can blank no function: a TU
//! VC8 cannot compile there fails instead of being quarantined.
const std = @import("std");
const util = @import("util.zig");
const proc = @import("proc.zig");
const objcache = @import("objcache.zig");
const gpa = util.gpa;

const CLI = "packages/reconstruct/src/vc8/cli.ts";
const PARSER = "packages/reconstruct/node_modules/@ghidra-mcp/cpp-parser/dist/index.js";
pub const MANIFEST = "sources.tsv";
pub const MANIFEST_HEADER = "sha256\tfile\n";

/// The checkout to translate with, or null. A GR without the VC8 mode is an error, not a
/// silent fall-back.
pub fn checkout() !?[]const u8 {
    if (util.getenv("GR")) |gr| if (gr.len > 0) {
        if (!util.exists(try util.join(gr, CLI))) {
            std.debug.print("GR={s} has no {s}\n", .{ gr, CLI });
            std.process.exit(1);
        }
        return gr;
    };
    const daemon = try std.fmt.allocPrint(gpa, "{s}/code/ts/gr-daemon", .{util.getenv("HOME") orelse ""});
    if (util.exists(try util.join(daemon, CLI)) and util.exists(try util.join(daemon, PARSER))) return daemon;
    return null;
}

/// ORACLE_TRANSLATED, when set: a shipped tree chosen over any translator.
pub fn forcedShipped() ?[]const u8 {
    const d = util.getenv("ORACLE_TRANSLATED") orelse return null;
    return if (d.len > 0) d else null;
}

/// The shipped tree beside the oracle (<oracle>/translated), when it has its manifest.
pub fn shippedBeside(oracle: []const u8) !?[]const u8 {
    const d = try util.join(oracle, "translated");
    return if (util.exists(try util.join(d, MANIFEST))) d else null;
}

/// Where the translation comes from: the translator (gr) or a shipped tree. Null: neither.
pub const Origin = union(enum) { gr: []const u8, shipped: []const u8 };

pub fn origin(oracle: []const u8) !?Origin {
    if (forcedShipped()) |d| return .{ .shipped = d };
    if (try checkout()) |gr| return .{ .gr = gr };
    if (try shippedBeside(oracle)) |d| return .{ .shipped = d };
    return null;
}

pub const NO_ORIGIN =
    \\no VC8 translation of the tree: neither a ghidra-reconstruct checkout to translate with
    \\(GR=<checkout>, else ~/code/ts/gr-daemon) nor a shipped translated tree
    \\(asm_matching/msvc8/translated/sources.tsv, or ORACLE_TRANSLATED=<dir>)
    \\
;

/// A shipped tree's manifest: file -> sha256 of the source it was translated from.
pub fn manifest(dir: []const u8) !std.StringArrayHashMapUnmanaged([]const u8) {
    var out: std.StringArrayHashMapUnmanaged([]const u8) = .empty;
    const data = (try util.readFileOpt(try util.join(dir, MANIFEST))) orelse {
        std.debug.print("{s}: no {s}\n", .{ dir, MANIFEST });
        std.process.exit(1);
    };
    var it = util.Lines{ .data = data };
    while (it.next()) |line| {
        if (std.mem.eql(u8, line, MANIFEST_HEADER[0 .. MANIFEST_HEADER.len - 1])) continue;
        const tab = std.mem.indexOfScalar(u8, line, '\t') orelse continue;
        try out.put(gpa, line[tab + 1 ..], line[0..tab]);
    }
    return out;
}

/// Does the shipped tree translate `repo` as it is now: the same files, each translated from
/// a source with the hash the manifest records. Prints what differs and exits 1 when not.
pub fn checkShipped(dir: []const u8, repo: []const u8, rels: []const []const u8) !void {
    var m = try manifest(dir);
    var bad: std.ArrayList([]const u8) = .empty;
    var seen: std.StringHashMapUnmanaged(void) = .empty;
    for (rels) |rel| {
        try seen.put(gpa, rel, {});
        const want = m.get(rel) orelse {
            try bad.append(gpa, try std.fmt.allocPrint(gpa, "  not in the shipped tree: {s}", .{rel}));
            continue;
        };
        const have = try objcache.shaFile(try util.join(repo, rel));
        if (!std.mem.eql(u8, have, want)) try bad.append(gpa, try std.fmt.allocPrint(gpa, "  changed since it was translated: {s}", .{rel}));
    }
    for (m.keys()) |rel| if (!seen.contains(rel)) try bad.append(gpa, try std.fmt.allocPrint(gpa, "  no longer in the tree: {s}", .{rel}));
    if (bad.items.len == 0) return;
    std.debug.print("{s} does not translate this tree ({d} files differ):\n", .{ dir, bad.items.len });
    for (bad.items[0..@min(bad.items.len, 20)]) |b| std.debug.print("{s}\n", .{b});
    if (bad.items.len > 20) std.debug.print("  ...\n", .{});
    std.debug.print("It is written by `oracle-zig translate` (the export does); to compile this tree as it is, translate it with a ghidra-reconstruct checkout (GR=<checkout>).\n", .{});
    std.process.exit(1);
}

/// Copy the shipped tree's files into `out`, rewriting only those that differ; returns
/// (written, unchanged).
fn copyShipped(dir: []const u8, out: []const u8) ![2]u64 {
    var m = try manifest(dir);
    var written: u64 = 0;
    var kept: u64 = 0;
    for (m.keys()) |rel| {
        const data = try util.readFile(try util.join(dir, rel));
        const dst = try util.join(out, rel);
        const old = try util.readFileOpt(dst);
        if (old != null and std.mem.eql(u8, old.?, data)) {
            kept += 1;
            continue;
        }
        util.makeDirs(util.dirname(dst));
        try util.writeFile(dst, data);
        written += 1;
    }
    return .{ written, kept };
}

/// Mirror `repo` into the empty directory `out` with the translator at `gr`, then write
/// `out`/sources.tsv. Returns the translator's summary line.
pub fn translateTree(gr: []const u8, repo: []const u8, out: []const u8) ![]const u8 {
    const r = try proc.run(&.{ "pnpm", "--silent", "--dir", gr, "exec", "tsx", CLI, repo, out }, .{});
    if (r.status != 0 or !std.mem.startsWith(u8, r.out, "vc8_translate: ")) {
        std.debug.print("{s}{s}ghidra-reconstruct translation failed (exit {d})\n", .{ r.out, r.err, r.status });
        std.process.exit(1);
    }
    var rels: std.ArrayList([]const u8) = .empty;
    for (try proc.allFiles(out)) |p| try rels.append(gpa, try proc.relpath(p, out));
    proc.sortStrings(rels.items);
    var text: std.ArrayList(u8) = .empty;
    try text.appendSlice(gpa, MANIFEST_HEADER);
    for (rels.items) |rel| {
        const src = try util.join(repo, rel);
        if (!util.exists(src)) {
            std.debug.print("{s}: the translation wrote {s}, which the tree does not have\n", .{ out, rel });
            std.process.exit(1);
        }
        try text.print(gpa, "{s}\t{s}\n", .{ try objcache.shaFile(src), rel });
    }
    try util.writeFile(try util.join(out, MANIFEST), text.items);
    return std.mem.trimEnd(u8, r.out, "\r\n");
}

/// Copy a shipped tree into the empty directory `out`, manifest included (a checkout with no
/// translator: x64/count).
pub fn copyTree(dir: []const u8, out: []const u8) !void {
    _ = try copyShipped(dir, out);
    try util.writeFile(try util.join(out, MANIFEST), try util.readFile(try util.join(dir, MANIFEST)));
}

pub const Function = struct { line: u32, name: []const u8 };

pub const Translator = struct {
    gr: []const u8 = "",
    /// A shipped tree instead of the translator: files are read from it, nothing is blanked.
    shipped: ?[]const u8 = null,
    repo: []const u8,
    pipe: ?proc.Pipe = null,
    lock: proc.Mutex = .{},

    pub fn init(t: *Translator) void {
        t.lock.init();
    }

    /// Translate every source into `out`; returns (written, unchanged).
    pub fn mirror(t: *Translator, out: []const u8, quarantine_tsv: []const u8) ![2]u64 {
        if (t.shipped) |d| return copyShipped(d, out);
        var args: std.ArrayList([]const u8) = .empty;
        try args.appendSlice(gpa, &.{ "pnpm", "--silent", "--dir", t.gr, "exec", "tsx", CLI, t.repo, out, "--serve" });
        if (util.exists(quarantine_tsv)) try args.appendSlice(gpa, &.{ "--quarantine", quarantine_tsv });
        t.pipe = try proc.Pipe.spawn(args.items);
        const line = try t.pipe.?.readLine();
        if (!std.mem.startsWith(u8, line, "vc8_translate: ")) {
            t.pipe.?.closeIn();
            const rc = t.pipe.?.wait();
            std.debug.print("ghidra-reconstruct translation failed (exit {d})\n", .{rc});
            std.process.exit(1);
        }
        var words: std.ArrayList([]const u8) = .empty;
        var it = std.mem.tokenizeAny(u8, line, " \t\r\n");
        while (it.next()) |w| try words.append(gpa, w);
        return .{ try std.fmt.parseInt(u64, words.items[1], 10), try std.fmt.parseInt(u64, words.items[3], 10) };
    }

    fn ask(t: *Translator, rel: []const u8, lines: []const u32, at: ?[]const u32) !std.json.Value {
        var req: std.ArrayList(u8) = .empty;
        try req.appendSlice(gpa, "{\"rel\": ");
        try objcache.jsonString(&req, rel);
        try req.appendSlice(gpa, ", \"lines\": [");
        for (lines, 0..) |l, i| try req.print(gpa, "{s}{d}", .{ if (i > 0) ", " else "", l });
        try req.append(gpa, ']');
        if (at) |a| {
            try req.appendSlice(gpa, ", \"at\": [");
            for (a, 0..) |l, i| try req.print(gpa, "{s}{d}", .{ if (i > 0) ", " else "", l });
            try req.append(gpa, ']');
        }
        try req.appendSlice(gpa, "}\n");
        t.lock.lock();
        const reply = blk: {
            defer t.lock.unlock();
            try t.pipe.?.write(req.items);
            break :blk try t.pipe.?.readLine();
        };
        const v = try std.json.parseFromSliceLeaky(std.json.Value, gpa, reply, .{});
        if (v.object.get("error")) |e| {
            std.debug.print("{s}/{s}: {s}\n", .{ t.repo, rel, if (e == .string) e.string else "?" });
            std.process.exit(1);
        }
        return v;
    }

    /// File `rel` of the tree translated, the functions holding `lines` quarantined.
    pub fn translate(t: *Translator, rel: []const u8, lines: []const u32) ![]const u8 {
        if (t.shipped) |d| {
            if (lines.len > 0) {
                std.debug.print("{s}: a shipped translation cannot blank a function (quarantine needs a ghidra-reconstruct checkout)\n", .{rel});
                std.process.exit(1);
            }
            return (try util.readFileOpt(try util.join(d, rel))) orelse {
                std.debug.print("{s}: not in the shipped tree {s}\n", .{ rel, d });
                std.process.exit(1);
            };
        }
        const v = try t.ask(rel, lines, null);
        const b64 = v.object.get("text").?.string;
        const dec = std.base64.standard.Decoder;
        const out = try gpa.alloc(u8, try dec.calcSizeForSlice(b64));
        try dec.decode(out, b64);
        return out;
    }

    /// For each 1-based line of `at`: the line of the body's `{` and the name of the function
    /// it lies in, in the translation under `lines`; null outside one.
    pub fn functions(t: *Translator, rel: []const u8, lines: []const u32, at: []const u32) ![]?Function {
        const v = try t.ask(rel, lines, at);
        const found = v.object.get("functions").?.array.items;
        const out = try gpa.alloc(?Function, found.len);
        for (found, 0..) |f, i| out[i] = if (f == .object) .{
            .line = @intCast(f.object.get("line").?.integer),
            .name = f.object.get("name").?.string,
        } else null;
        return out;
    }

    pub fn close(t: *Translator) void {
        if (t.pipe) |*p| {
            p.closeIn();
            _ = p.wait();
            t.pipe = null;
        }
    }
};
