//! svg (byte_match_svg.py): draw the audit summary's byte kinds as a stacked bar.
//!
//!   oracle-zig svg [--root R] [--summary evidence/audit.tsv.summary] [--out evidence/byte-match]
//!
//! Reads the `byte kinds` block the audit writes and draws <out>-light.svg and
//! <out>-dark.svg: one horizontal bar, a segment per kind in proportion to its bytes, and a
//! legend with each kind's bytes, share and meaning. Two files with fixed colours rather
//! than one with a media query: an SVG shown through <img> follows the operating system's
//! scheme, not the page's, so a README picks the file with <picture>.
const std = @import("std");
const cli = @import("cli.zig");
const util = @import("util.zig");

const gpa = util.gpa;

const Fill = struct { key: []const u8, light: []const u8, dark: []const u8, label: []const u8 };
// Kind -> (light fill, dark fill, legend label), in the summary's order.
const FILLS = [_]Fill{
    .{ .key = "whole", .light = "#1f6feb", .dark = "#4493f8", .label = "Whole function identical" },
    .{ .key = "ident", .light = "#79b8ff", .dark = "#2f6eb5", .label = "Matching instructions" },
    .{ .key = "operand", .light = "#e3b341", .dark = "#bb8009", .label = "Same instruction, other operands" },
    .{ .key = "differ", .light = "#d1d5da", .dark = "#5a6370", .label = "Different instructions" },
    .{ .key = "noncode", .light = "#eceff3", .dark = "#3d444d", .label = "Tables and padding" },
    .{ .key = "uncompared", .light = "#9aa4ae", .dark = "#8b949e", .label = "Not compared" },
};

const Theme = struct { name: []const u8, bg: []const u8, text: []const u8, muted: []const u8, edge: []const u8, dark: bool };
const THEMES = [_]Theme{
    .{ .name = "light", .bg = "#ffffff", .text = "#1f2328", .muted = "#59636e", .edge = "#d1d9e0", .dark = false },
    .{ .name = "dark", .bg = "#0d1117", .text = "#f0f6fc", .muted = "#9198a1", .edge = "#3d444d", .dark = true },
};

const Kind = struct { key: []const u8, n: u64, pct: f64, what: []const u8 };

fn fillOf(key: []const u8, dark: bool) []const u8 {
    for (FILLS) |f| if (std.mem.eql(u8, f.key, key)) return if (dark) f.dark else f.light;
    return "#888";
}

fn labelOf(key: []const u8) []const u8 {
    for (FILLS) |f| if (std.mem.eql(u8, f.key, key)) return f.label;
    return key;
}

fn isDigits(s: []const u8) bool {
    if (s.len == 0) return false;
    for (s) |ch| if (!std.ascii.isDigit(ch)) return false;
    return true;
}

fn isSpace(ch: u8) bool {
    return ch == ' ' or ch == '\t' or ch == '\n' or ch == '\r' or ch == 0x0b or ch == 0x0c;
}

/// ^  kind (\S+)\s+(\d+)\s+([\d.]+)% (.*)$
fn kindLine(line: []const u8) ?Kind {
    const pfx = "  kind ";
    if (!std.mem.startsWith(u8, line, pfx)) return null;
    var i: usize = pfx.len;
    const k0 = i;
    while (i < line.len and !isSpace(line[i])) i += 1;
    if (i == k0) return null;
    const key = line[k0..i];
    const s0 = i;
    while (i < line.len and isSpace(line[i])) i += 1;
    if (i == s0) return null;
    const d0 = i;
    while (i < line.len and std.ascii.isDigit(line[i])) i += 1;
    if (i == d0) return null;
    const n = std.fmt.parseInt(u64, line[d0..i], 10) catch return null;
    const s1 = i;
    while (i < line.len and isSpace(line[i])) i += 1;
    if (i == s1) return null;
    const p0 = i;
    while (i < line.len and (std.ascii.isDigit(line[i]) or line[i] == '.')) i += 1;
    if (i == p0 or i + 1 >= line.len + 0 or !std.mem.startsWith(u8, line[i..], "% ")) return null;
    const pct = std.fmt.parseFloat(f64, line[p0..i]) catch return null;
    return .{ .key = key, .n = n, .pct = pct, .what = line[i + 2 ..] };
}

/// (total, kinds) from the summary's byte kinds block.
fn parse(path: []const u8) !struct { u64, []Kind } {
    var total: ?u64 = null;
    var kinds: std.ArrayList(Kind) = .empty;
    var it = util.Lines{ .data = try util.readFile(path) };
    while (it.next()) |line| {
        const head = "byte kinds of the headline total: ";
        if (std.mem.startsWith(u8, line, head)) {
            var j: usize = head.len;
            while (j < line.len and std.ascii.isDigit(line[j])) j += 1;
            if (j > head.len and std.mem.startsWith(u8, line[j..], " bytes")) {
                total = try std.fmt.parseInt(u64, line[head.len..j], 10);
                continue;
            }
        }
        if (total != null) if (kindLine(line)) |k| try kinds.append(gpa, k);
    }
    if (total == null or kinds.items.len == 0) cli.exitMsg("{s}: no byte kinds block (run oracle audit)", .{path});
    var sum: u64 = 0;
    for (kinds.items) |k| sum += k.n;
    if (sum != total.?) cli.exitMsg("{s}: the byte kinds do not sum to {d}", .{ path, total.? });
    return .{ total.?, kinds.items };
}

/// format(n, ',')
fn thousands(out: *std.ArrayList(u8), n: u64) !void {
    var buf: [32]u8 = undefined;
    const s = try std.fmt.bufPrint(&buf, "{d}", .{n});
    for (s, 0..) |ch, i| {
        if (i > 0 and (s.len - i) % 3 == 0) try out.append(gpa, ',');
        try out.append(gpa, ch);
    }
}

/// xml.sax.saxutils.escape
fn escape(out: *std.ArrayList(u8), s: []const u8) !void {
    for (s) |ch| switch (ch) {
        '&' => try out.appendSlice(gpa, "&amp;"),
        '<' => try out.appendSlice(gpa, "&lt;"),
        '>' => try out.appendSlice(gpa, "&gt;"),
        else => try out.append(gpa, ch),
    };
}

/// Black or white, whichever reads on `fill`.
fn ink(fill: []const u8) ![]const u8 {
    if (fill.len < 7) return error.ValueError;
    const r: f64 = @floatFromInt(try std.fmt.parseInt(u8, fill[1..3], 16));
    const g: f64 = @floatFromInt(try std.fmt.parseInt(u8, fill[3..5], 16));
    const b: f64 = @floatFromInt(try std.fmt.parseInt(u8, fill[5..7], 16));
    return if (0.299 * r + 0.587 * g + 0.114 * b > 140) "#000000" else "#ffffff";
}

fn f2(o: *std.ArrayList(u8), v: f64) !void {
    try util.fixed(o, "%.2f", v);
}

fn svg(total: u64, kinds: []const Kind, t: Theme) ![]const u8 {
    const w = 800;
    const pad = 24;
    const bar_y = 52;
    const bar_h = 30;
    const row_h = 26;
    const legend_y = bar_y + bar_h + 30;
    const h = legend_y + row_h * kinds.len + 12;
    const font = "font-family=\"system-ui, -apple-system, Segoe UI, Helvetica, Arial, sans-serif\"";
    var o: std.ArrayList(u8) = .empty;
    try o.print(gpa, "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"{d}\" height=\"{d}\" viewBox=\"0 0 {d} {d}\" {s}>\n", .{ w, h, w, h, font });
    try o.appendSlice(gpa, "<title>Bytes of the original game code by kind of match</title>\n");
    try o.print(gpa, "<rect width=\"{d}\" height=\"{d}\" rx=\"6\" fill=\"{s}\"/>\n", .{ w, h, t.bg });
    try o.print(gpa, "<text x=\"{d}\" y=\"30\" font-size=\"15\" font-weight=\"600\" fill=\"{s}\">Original game code, C runtime, prebuilt PKWARE library and compiler EH stubs left out: ", .{ pad, t.text });
    try thousands(&o, total);
    try o.appendSlice(gpa, " bytes</text>\n");
    const span: u64 = w - 2 * pad;
    var x: f64 = pad;
    for (kinds) |k| {
        const seg = @as(f64, @floatFromInt(span * k.n)) / @as(f64, @floatFromInt(total));
        if (seg <= 0) continue;
        const fill = fillOf(k.key, t.dark);
        try o.appendSlice(gpa, "<rect x=\"");
        try f2(&o, x);
        try o.print(gpa, "\" y=\"{d}\" width=\"", .{bar_y});
        try f2(&o, seg);
        try o.print(gpa, "\" height=\"{d}\" fill=\"{s}\"/>\n", .{ bar_h, fill });
        if (seg >= 48) {
            try o.appendSlice(gpa, "<text x=\"");
            try f2(&o, x + seg / 2);
            try o.print(gpa, "\" y=\"{d}\" font-size=\"12\" text-anchor=\"middle\" fill=\"{s}\">", .{ bar_y + bar_h / 2 + 4, try ink(fill) });
            try util.fixed(&o, "%.1f", k.pct);
            try o.appendSlice(gpa, "%</text>\n");
        }
        x += seg;
    }
    try o.print(gpa, "<rect x=\"{d}\" y=\"{d}\" width=\"{d}\" height=\"{d}\" fill=\"none\" stroke=\"{s}\"/>\n", .{ pad, bar_y, span, bar_h, t.edge });
    for (kinds, 0..) |k, i| {
        const y = legend_y + row_h * i;
        try o.print(gpa, "<rect x=\"{d}\" y=\"{d}\" width=\"14\" height=\"14\" rx=\"2\" fill=\"{s}\" stroke=\"{s}\"/>\n", .{ pad, y - 11, fillOf(k.key, t.dark), t.edge });
        try o.print(gpa, "<text x=\"{d}\" y=\"{d}\" font-size=\"13\" fill=\"{s}\">", .{ pad + 22, y, t.text });
        try escape(&o, labelOf(k.key));
        try o.appendSlice(gpa, "</text>\n");
        try o.print(gpa, "<text x=\"{d}\" y=\"{d}\" font-size=\"13\" text-anchor=\"end\" fill=\"{s}\">", .{ pad + 310, y, t.text });
        try thousands(&o, k.n);
        try o.appendSlice(gpa, "</text>\n");
        try o.print(gpa, "<text x=\"{d}\" y=\"{d}\" font-size=\"13\" text-anchor=\"end\" fill=\"{s}\">", .{ pad + 374, y, t.text });
        try f2(&o, k.pct);
        try o.appendSlice(gpa, "%</text>\n");
        try o.print(gpa, "<text x=\"{d}\" y=\"{d}\" font-size=\"12\" fill=\"{s}\">", .{ pad + 392, y, t.muted });
        try escape(&o, k.what);
        try o.appendSlice(gpa, "</text>\n");
    }
    try o.appendSlice(gpa, "</svg>\n");
    return o.items;
}

const USAGE =
    \\usage: oracle svg [-h] [--summary SUMMARY] [--out OUT]
    \\
;

pub fn main(args: []const []const u8) !void {
    const p = cli.parse(args, &.{ .{ .long = "--root" }, .{ .long = "--summary" }, .{ .long = "--out" } }, USAGE);
    if (p.pos.len > 0) {
        cli.sayErr(USAGE);
        std.process.exit(2);
    }
    const root = try cli.root(p.get("--root"));
    const summary = p.get("--summary") orelse try util.join(root, "evidence/audit.tsv.summary");
    const out_base = p.get("--out") orelse try util.join(root, "evidence/byte-match");
    const total, const kinds = try parse(summary);
    var out = cli.Out{};
    for (THEMES) |t| {
        const path = try std.fmt.allocPrint(gpa, "{s}-{s}.svg", .{ out_base, t.name });
        try util.writeFile(path, try svg(total, kinds, t));
        try out.line(path);
    }
    try out.flush();
}
