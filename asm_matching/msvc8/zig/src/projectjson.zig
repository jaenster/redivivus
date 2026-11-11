//! project-json (recon/publish-kit/tools/public-project-json.py): write the public
//! project.json, the module table of recon/project.json and nothing else.
//!
//!   oracle-zig project-json <recon/project.json> <out/project.json>
//!
//! The private file also holds server addresses, override paths and cross-platform links;
//! only "modules" is published, as json.dumps({'modules': ...}, indent=2, ensure_ascii=False)
//! writes it. A module table that names a local path is refused rather than leaked.
const std = @import("std");
const cli = @import("cli.zig");
const util = @import("util.zig");

const gpa = util.gpa;

fn indent(o: *std.ArrayList(u8), n: usize) !void {
    try o.append(gpa, '\n');
    for (0..n) |_| try o.append(gpa, ' ');
}

/// json's string encoder with ensure_ascii=False: only '"', '\\' and control characters.
fn string(o: *std.ArrayList(u8), s: []const u8) !void {
    try o.append(gpa, '"');
    for (s) |ch| switch (ch) {
        '"' => try o.appendSlice(gpa, "\\\""),
        '\\' => try o.appendSlice(gpa, "\\\\"),
        '\n' => try o.appendSlice(gpa, "\\n"),
        '\r' => try o.appendSlice(gpa, "\\r"),
        '\t' => try o.appendSlice(gpa, "\\t"),
        0x08 => try o.appendSlice(gpa, "\\b"),
        0x0c => try o.appendSlice(gpa, "\\f"),
        0...0x07, 0x0b, 0x0e...0x1f => try o.print(gpa, "\\u{x:0>4}", .{ch}),
        else => try o.append(gpa, ch),
    };
    try o.append(gpa, '"');
}

/// float.__repr__: the shortest digits that round-trip, positional for 1e-4 <= |x| < 1e16,
/// else d.ddde+XX.
fn floatRepr(o: *std.ArrayList(u8), v: f64) !void {
    if (std.math.isNan(v)) return o.appendSlice(gpa, "NaN");
    if (std.math.isInf(v)) return o.appendSlice(gpa, if (v < 0) "-Infinity" else "Infinity");
    if (v == 0) return o.appendSlice(gpa, if (std.math.signbit(v)) "-0.0" else "0.0");
    var buf: [64]u8 = undefined;
    const e = try std.fmt.bufPrint(&buf, "{e}", .{@abs(v)}); // d[.ddd]e[-]X, shortest digits
    const epos = std.mem.indexOfScalar(u8, e, 'e').?;
    var digits: std.ArrayList(u8) = .empty;
    for (e[0..epos]) |ch| if (ch != '.') try digits.append(gpa, ch);
    const exp = try std.fmt.parseInt(i32, e[epos + 1 ..], 10);
    if (v < 0) try o.append(gpa, '-');
    const d = digits.items;
    if (exp >= -4 and exp < 16) {
        if (exp < 0) {
            try o.appendSlice(gpa, "0.");
            for (0..@intCast(-exp - 1)) |_| try o.append(gpa, '0');
            try o.appendSlice(gpa, d);
        } else {
            const ip: usize = @intCast(exp + 1);
            if (d.len <= ip) {
                try o.appendSlice(gpa, d);
                for (d.len..ip) |_| try o.append(gpa, '0');
                try o.appendSlice(gpa, ".0");
            } else {
                try o.appendSlice(gpa, d[0..ip]);
                try o.append(gpa, '.');
                try o.appendSlice(gpa, d[ip..]);
            }
        }
    } else {
        try o.append(gpa, d[0]);
        if (d.len > 1) {
            try o.append(gpa, '.');
            try o.appendSlice(gpa, d[1..]);
        }
        try o.print(gpa, "e{c}{d:0>2}", .{ @as(u8, if (exp < 0) '-' else '+'), @abs(exp) });
    }
}

fn number(o: *std.ArrayList(u8), text: []const u8) !void {
    const is_float = std.mem.indexOfAny(u8, text, ".eE") != null;
    if (is_float) return floatRepr(o, try std.fmt.parseFloat(f64, text));
    if (std.fmt.parseInt(i128, text, 10)) |n| try o.print(gpa, "{d}", .{n}) else |_| try o.appendSlice(gpa, text);
}

/// json.dumps(v, indent=2, ensure_ascii=False) at nesting `level`.
pub fn dump(o: *std.ArrayList(u8), v: std.json.Value, level: usize) !void {
    switch (v) {
        .null => try o.appendSlice(gpa, "null"),
        .bool => |b| try o.appendSlice(gpa, if (b) "true" else "false"),
        .integer => |n| try o.print(gpa, "{d}", .{n}),
        .float => |f| try floatRepr(o, f),
        .number_string => |s| try number(o, s),
        .string => |s| try string(o, s),
        .array => |a| {
            if (a.items.len == 0) return o.appendSlice(gpa, "[]");
            try o.append(gpa, '[');
            for (a.items, 0..) |item, i| {
                if (i > 0) try o.append(gpa, ',');
                try indent(o, 2 * (level + 1));
                try dump(o, item, level + 1);
            }
            try indent(o, 2 * level);
            try o.append(gpa, ']');
        },
        .object => |m| {
            if (m.count() == 0) return o.appendSlice(gpa, "{}");
            try o.append(gpa, '{');
            for (m.keys(), m.values(), 0..) |k, item, i| {
                if (i > 0) try o.append(gpa, ',');
                try indent(o, 2 * (level + 1));
                try string(o, k);
                try o.appendSlice(gpa, ": ");
                try dump(o, item, level + 1);
            }
            try indent(o, 2 * level);
            try o.append(gpa, '}');
        },
    }
}

/// len() of a JSON value as Python counts it.
fn pyLen(v: std.json.Value) usize {
    return switch (v) {
        .object => |m| m.count(),
        .array => |a| a.items.len,
        .string => |s| std.unicode.utf8CountCodepoints(s) catch s.len,
        else => 0,
    };
}

pub fn main(args: []const []const u8) !void {
    if (args.len != 2) {
        cli.sayErr("usage: oracle-zig project-json <recon/project.json> <out/project.json>\n");
        std.process.exit(1);
    }
    const parsed = try std.json.parseFromSlice(std.json.Value, gpa, try util.readFile(args[0]), .{ .parse_numbers = false, .duplicate_field_behavior = .use_last });
    if (parsed.value != .object) return error.TypeError;
    const modules = parsed.value.object.get("modules") orelse {
        cli.exitMsg("KeyError: 'modules'", .{});
    };
    var o: std.ArrayList(u8) = .empty;
    try o.appendSlice(gpa, "{");
    try indent(&o, 2);
    try string(&o, "modules");
    try o.appendSlice(gpa, ": ");
    try dump(&o, modules, 1);
    try indent(&o, 0);
    try o.appendSlice(gpa, "}\n");
    for ([_][]const u8{ "/Users/", "~/", "ghidra://", "/private/", "C:\\\\" }) |bad| {
        if (std.mem.indexOf(u8, o.items, bad) != null) cli.exitMsg("public-project-json: the module table names a local path ({s})", .{bad});
    }
    try util.writeFile(args[1], o.items);
    var out = cli.Out{};
    try out.print("project.json: {d} modules\n", .{pyLen(modules)});
    try out.flush();
}
