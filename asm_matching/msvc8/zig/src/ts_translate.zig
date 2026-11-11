//! ts_translate.py: the VC8 translation by ghidra-reconstruct's TypeScript CLI
//! (packages/reconstruct/src/vc8), which stays TypeScript. GR=<checkout> selects one; without
//! GR the live daemon's checkout (~/code/ts/gr-daemon) when it has the VC8 mode and a built
//! cpp-parser. With neither, `oracle build` runs build.py, which translates with
//! vc8_translate.py.
//!
//! One process serves the whole build: it mirrors the tree into build/src, then translates
//! single files on request (a quarantine round, a support TU) with the same per-tree state.
const std = @import("std");
const util = @import("util.zig");
const proc = @import("proc.zig");
const objcache = @import("objcache.zig");
const gpa = util.gpa;

const CLI = "packages/reconstruct/src/vc8/cli.ts";
const PARSER = "packages/reconstruct/node_modules/@ghidra-mcp/cpp-parser/dist/index.js";

/// The checkout to translate with, or null (build.py's vc8_translate.py). A GR without the
/// VC8 mode is an error, not a silent fall-back.
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

pub const Function = struct { line: u32, name: []const u8 };

pub const Translator = struct {
    gr: []const u8,
    repo: []const u8,
    pipe: ?proc.Pipe = null,
    lock: proc.Mutex = .{},

    pub fn init(t: *Translator) void {
        t.lock.init();
    }

    /// Translate every source into `out`; returns (written, unchanged).
    pub fn mirror(t: *Translator, out: []const u8, quarantine_tsv: []const u8) ![2]u64 {
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
