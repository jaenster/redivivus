//! Manually confirmed reference starts that the automatic carve cannot infer.
//! They apply only to the exact Game.exe build whose code was inspected.
const std = @import("std");

pub const new_node: u32 = 0x00610630;
const game_sha256 = "631066c1649c4ea9ffe48bf97e24c00bca1f7a6759c21150f1a79982589adaaf";

pub fn newNodeStart(image: []const u8) ?u32 {
    var digest: [32]u8 = undefined;
    std.crypto.hash.sha2.Sha256.hash(image, &digest, .{});
    const actual = std.fmt.bytesToHex(digest, .lower);
    return if (std.mem.eql(u8, actual[0..], game_sha256)) new_node else null;
}

test "unrelated image does not receive the NewNode seed" {
    var other_pe: [128]u8 = @splat(0);
    other_pe[0] = 'M';
    other_pe[1] = 'Z';
    try std.testing.expectEqual(@as(?u32, null), newNodeStart(&other_pe));
}
