//! Independently confirmed starts, supplied as evidence data for an exact image.
const std = @import("std");

pub fn read(allocator: std.mem.Allocator, data: []const u8, image: []const u8) ![]u32 {
    var digest: [32]u8 = undefined;
    std.crypto.hash.sha2.Sha256.hash(image, &digest, .{});
    const actual = std.fmt.bytesToHex(digest, .lower);
    var out: std.ArrayList(u32) = .empty;
    errdefer out.deinit(allocator);
    var lines = std.mem.splitScalar(u8, data, '\n');
    const header = std.mem.trim(u8, lines.next() orelse return error.InvalidConfirmedStarts, "\r");
    if (!std.mem.eql(u8, header, "address\timage_sha256\treason")) return error.InvalidConfirmedStarts;
    while (lines.next()) |raw| {
        const line = std.mem.trim(u8, raw, "\r");
        if (line.len == 0 or line[0] == '#') continue;
        var fields = std.mem.splitScalar(u8, line, '\t');
        const address = fields.next() orelse return error.InvalidConfirmedStarts;
        const hash = fields.next() orelse return error.InvalidConfirmedStarts;
        const reason = fields.next() orelse return error.InvalidConfirmedStarts;
        if (fields.next() != null or address.len != 8 or hash.len != 64 or std.mem.trim(u8, reason, " ").len == 0) return error.InvalidConfirmedStarts;
        for (address) |c| if (!std.ascii.isHex(c)) return error.InvalidConfirmedStarts;
        for (hash) |c| if (!std.ascii.isHex(c)) return error.InvalidConfirmedStarts;
        const value = try std.fmt.parseInt(u32, address, 16);
        if (!std.ascii.eqlIgnoreCase(hash, &actual)) continue;
        for (out.items) |existing| if (existing == value) return error.DuplicateConfirmedStart;
        try out.append(allocator, value);
    }
    return out.toOwnedSlice(allocator);
}

const test_header = "address\timage_sha256\treason\n";
fn fixture(allocator: std.mem.Allocator, image: []const u8) ![]u8 {
    var digest: [32]u8 = undefined;
    std.crypto.hash.sha2.Sha256.hash(image, &digest, .{});
    const hex = std.fmt.bytesToHex(digest, .lower);
    return std.fmt.allocPrint(allocator, test_header ++ "00401000\t{s}\tSynthetic first entry\r\n00402000\t{s}\tSynthetic second entry\n", .{ hex, hex });
}

test "starts come from data only for its exact image" {
    const allocator = std.testing.allocator;
    const data = try fixture(allocator, "fixture image");
    defer allocator.free(data);
    const starts = try read(allocator, data, "fixture image");
    defer allocator.free(starts);
    try std.testing.expectEqualSlices(u32, &.{ 0x00401000, 0x00402000 }, starts);
    const unrelated = try read(allocator, data, "another image");
    defer allocator.free(unrelated);
    try std.testing.expectEqual(@as(usize, 0), unrelated.len);
}

test "duplicate active starts refuse" {
    const allocator = std.testing.allocator;
    const data = try fixture(allocator, "fixture image");
    defer allocator.free(data);
    const duplicate = try std.mem.replaceOwned(u8, allocator, data, "00402000", "00401000");
    defer allocator.free(duplicate);
    try std.testing.expectError(error.DuplicateConfirmedStart, read(allocator, duplicate, "fixture image"));
}

test "malformed evidence refuses even for another image" {
    for ([_][]const u8{
        "",
        "address\thash\treason\n",
        test_header ++ "not-hex!\t" ++ "0000000000000000000000000000000000000000000000000000000000000000" ++ "\treason\n",
        test_header ++ "00401000\t" ++ "gggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggg" ++ "\treason\n",
        test_header ++ "00401000\t" ++ "0000000000000000000000000000000000000000000000000000000000000000" ++ "\t\n",
        test_header ++ "00401000\t" ++ "0000000000000000000000000000000000000000000000000000000000000000" ++ "\treason\textra\n",
    }) |data| try std.testing.expectError(error.InvalidConfirmedStarts, read(std.testing.allocator, data, "other"));
}
