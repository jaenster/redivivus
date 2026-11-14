//! The tool stamps in images and objects:
//!
//!   oracle-zig rich <image.exe|dll>          the Rich header's records (decode_rich_header.py)
//!   oracle-zig compid <file.obj | file.lib> ...   the @comp.id stamps (compid.py)
//!
//! Every object a Microsoft tool writes carries an @comp.id symbol (product id << 16 | build);
//! the linker counts them into the "Rich" header in the DOS stub, the authoritative
//! fingerprint of the toolchain that built an image. The build number cannot tell service
//! packs apart that share one (VS2005 RTM .42 and SP1 .762 both stamp 50727).
const std = @import("std");
const builtin = @import("builtin");
const c = @import("c");
const cli = @import("cli.zig");
const coff = @import("coff.zig");
const util = @import("util.zig");

const gpa = util.gpa;

fn prodName(pid: u32) ?[]const u8 {
    for (PRODID) |e| if (e[0] == pid) return e[1];
    return null;
}

fn buildName(b: u32) []const u8 {
    for (BUILD) |e| if (e[0] == b) return e[1];
    return "";
}

fn rd32(d: []const u8, o: usize) u32 {
    return std.mem.readInt(u32, d[o..][0..4], .little);
}

const Entry = struct { pid: u32, build: u32, count: u32 };

fn richEntries(data: []const u8, key_out: *?u32) ![]Entry {
    var out: std.ArrayList(Entry) = .empty;
    const pe = rd32(data, 0x3c);
    const stub = data[@min(0x40, data.len)..@min(@max(pe, 0x40), data.len)];
    const ri = std.mem.indexOf(u8, stub, "Rich") orelse return out.items;
    const key = rd32(stub, ri + 4);
    key_out.* = key;
    var dans: [4]u8 = undefined;
    std.mem.writeInt(u32, &dans, 0x536E6144 ^ key, .little);
    const di = std.mem.indexOf(u8, stub, &dans) orelse return out.items;
    var i = di + 16;
    while (i < ri) : (i += 8) {
        const cid = rd32(stub, i) ^ key;
        const cnt = rd32(stub, i + 4) ^ key;
        try out.append(gpa, .{ .pid = cid >> 16, .build = cid & 0xffff, .count = cnt });
    }
    return out.items;
}

const RICH_USAGE =
    \\decode_rich_header - list the toolchain records a PE's Rich header carries.
    \\
    \\    oracle rich <image.exe|dll>
    \\
;

pub fn richMain(args: []const []const u8) !void {
    if (args.len != 1) {
        cli.sayErr(RICH_USAGE);
        std.process.exit(1);
    }
    const path = args[0];
    const data = try util.readFile(path);
    const pe = rd32(data, 0x3c);
    if (pe + 4 > data.len or !std.mem.eql(u8, data[pe .. pe + 4], "PE\x00\x00")) cli.exitMsg("not a PE image: {s}", .{path});
    const stamp = rd32(data, pe + 8);
    const opt = pe + 24;
    var out = cli.Out{};
    try out.print("{s}\n", .{path});
    var t: c.time_t = @intCast(stamp);
    var tm: c.struct_tm = undefined;
    if (builtin.os.tag == .windows) {
        _ = c._gmtime64_s(&tm, &t);
    } else {
        _ = c.gmtime_r(&t, &tm);
    }
    var buf: [64]u8 = undefined;
    const n = c.strftime(&buf, buf.len, "%Y-%m-%d %H:%M:%S", &tm);
    try out.print("  TimeDateStamp  0x{x:0>8}  {s} UTC\n", .{ stamp, buf[0..n] });
    try out.print("  LinkerVersion  {d}.{d}\n", .{ data[opt + 2], data[opt + 3] });
    var key: ?u32 = null;
    const entries = try richEntries(data, &key);
    if (entries.len == 0) {
        try out.raw("  no Rich header\n");
        return out.flush();
    }
    try out.print("  Rich header    {d} records, XOR key 0x{x:0>8}\n", .{ entries.len, key.? });
    try out.print("  {s:>6}  {s:<22} {s:>6} {s:>6}  {s}\n", .{ "prodid", "tool", "build", "count", "release" });
    for (entries) |e| try out.print("  0x{x:0>4}  {s:<22} {d:>6} {d:>6}  {s}\n", .{ e.pid, prodName(e.pid) orelse "?", e.build, e.count, buildName(e.build) });
    try out.flush();
}

const COMPID_USAGE =
    \\compid - the @comp.id stamps of COFF objects and library members.
    \\
    \\    oracle compid <file.obj | file.lib> ...
    \\
;

pub fn compidMain(args: []const []const u8) !void {
    if (args.len < 1) {
        cli.sayErr(COMPID_USAGE);
        std.process.exit(1);
    }
    var out = cli.Out{};
    for (args) |path| {
        const data = try util.readFile(path);
        const members: []const coff.Member = if (data.len >= 8 and std.mem.eql(u8, data[0..8], "!<arch>\n")) try coff.readArchive(data) else &.{.{ .name = path, .body = data }};
        var counts: std.AutoArrayHashMapUnmanaged(u32, usize) = .empty;
        for (members) |m| {
            if (m.body.len < 20 or std.mem.eql(u8, m.body[0..4], "\x00\x00\xff\xff")) continue;
            const o = coff.Object.parse(m.body, m.name) catch continue;
            const cid = o.compId() orelse continue;
            const g = try counts.getOrPut(gpa, cid);
            if (!g.found_existing) g.value_ptr.* = 0;
            g.value_ptr.* += 1;
        }
        const cids = try gpa.dupe(u32, counts.keys());
        std.mem.sort(u32, cids, {}, std.sort.asc(u32));
        try out.print("{s}: ", .{util.basename(path)});
        if (cids.len == 0) try out.raw("no stamps");
        for (cids, 0..) |cid, i| {
            if (i > 0) try out.raw(", ");
            if (prodName(cid >> 16)) |nm| try out.raw(nm) else try out.print("0x{x:0>4}", .{cid >> 16});
            try out.print("/{d} x{d}", .{ cid & 0xffff, counts.get(cid).? });
        }
        try out.raw("\n");
    }
    try out.flush();
}

// Product ids (saferwall/pe's prodIDtoStrMap, MIT, copied verbatim) and build numbers, as
// decode_rich_header.py has them.
pub const PRODID = [_]struct { u16, []const u8 }{
    .{ 0x0000, "Unknown" },
    .{ 0x0001, "Import0" },
    .{ 0x0002, "Linker510" },
    .{ 0x0003, "Cvtomf510" },
    .{ 0x0004, "Linker600" },
    .{ 0x0005, "Cvtomf600" },
    .{ 0x0006, "Cvtres500" },
    .{ 0x0007, "Utc11_Basic" },
    .{ 0x0008, "Utc11_C" },
    .{ 0x0009, "Utc12_Basic" },
    .{ 0x000a, "Utc12_C" },
    .{ 0x000b, "Utc12_CPP" },
    .{ 0x000c, "AliasObj60" },
    .{ 0x000d, "VisualBasic60" },
    .{ 0x000e, "Masm613" },
    .{ 0x000f, "Masm710" },
    .{ 0x0010, "Linker511" },
    .{ 0x0011, "Cvtomf511" },
    .{ 0x0012, "Masm614" },
    .{ 0x0013, "Linker512" },
    .{ 0x0014, "Cvtomf512" },
    .{ 0x0015, "Utc12_C_Std" },
    .{ 0x0016, "Utc12_CPP_Std" },
    .{ 0x0017, "Utc12_C_Book" },
    .{ 0x0018, "Utc12_CPP_Book" },
    .{ 0x0019, "Implib700" },
    .{ 0x001a, "Cvtomf700" },
    .{ 0x001b, "Utc13_Basic" },
    .{ 0x001c, "Utc13_C" },
    .{ 0x001d, "Utc13_CPP" },
    .{ 0x001e, "Linker610" },
    .{ 0x001f, "Cvtomf610" },
    .{ 0x0020, "Linker601" },
    .{ 0x0021, "Cvtomf601" },
    .{ 0x0022, "Utc12_1_Basic" },
    .{ 0x0023, "Utc12_1_C" },
    .{ 0x0024, "Utc12_1_CPP" },
    .{ 0x0025, "Linker620" },
    .{ 0x0026, "Cvtomf620" },
    .{ 0x0027, "AliasObj70" },
    .{ 0x0028, "Linker621" },
    .{ 0x0029, "Cvtomf621" },
    .{ 0x002a, "Masm615" },
    .{ 0x002b, "Utc13_LTCG_C" },
    .{ 0x002c, "Utc13_LTCG_CPP" },
    .{ 0x002d, "Masm620" },
    .{ 0x002e, "ILAsm100" },
    .{ 0x002f, "Utc12_2_Basic" },
    .{ 0x0030, "Utc12_2_C" },
    .{ 0x0031, "Utc12_2_CPP" },
    .{ 0x0032, "Utc12_2_C_Std" },
    .{ 0x0033, "Utc12_2_CPP_Std" },
    .{ 0x0034, "Utc12_2_C_Book" },
    .{ 0x0035, "Utc12_2_CPP_Book" },
    .{ 0x0036, "Implib622" },
    .{ 0x0037, "Cvtomf622" },
    .{ 0x0038, "Cvtres501" },
    .{ 0x0039, "Utc13_C_Std" },
    .{ 0x003a, "Utc13_CPP_Std" },
    .{ 0x003b, "Cvtpgd1300" },
    .{ 0x003c, "Linker622" },
    .{ 0x003d, "Linker700" },
    .{ 0x003e, "Export622" },
    .{ 0x003f, "Export700" },
    .{ 0x0040, "Masm700" },
    .{ 0x0041, "Utc13_POGO_I_C" },
    .{ 0x0042, "Utc13_POGO_I_CPP" },
    .{ 0x0043, "Utc13_POGO_O_C" },
    .{ 0x0044, "Utc13_POGO_O_CPP" },
    .{ 0x0045, "Cvtres700" },
    .{ 0x0046, "Cvtres710p" },
    .{ 0x0047, "Linker710p" },
    .{ 0x0048, "Cvtomf710p" },
    .{ 0x0049, "Export710p" },
    .{ 0x004a, "Implib710p" },
    .{ 0x004b, "Masm710p" },
    .{ 0x004c, "Utc1310p_C" },
    .{ 0x004d, "Utc1310p_CPP" },
    .{ 0x004e, "Utc1310p_C_Std" },
    .{ 0x004f, "Utc1310p_CPP_Std" },
    .{ 0x0050, "Utc1310p_LTCG_C" },
    .{ 0x0051, "Utc1310p_LTCG_CPP" },
    .{ 0x0052, "Utc1310p_POGO_I_C" },
    .{ 0x0053, "Utc1310p_POGO_I_CPP" },
    .{ 0x0054, "Utc1310p_POGO_O_C" },
    .{ 0x0055, "Utc1310p_POGO_O_CPP" },
    .{ 0x0056, "Linker624" },
    .{ 0x0057, "Cvtomf624" },
    .{ 0x0058, "Export624" },
    .{ 0x0059, "Implib624" },
    .{ 0x005a, "Linker710" },
    .{ 0x005b, "Cvtomf710" },
    .{ 0x005c, "Export710" },
    .{ 0x005d, "Implib710" },
    .{ 0x005e, "Cvtres710" },
    .{ 0x005f, "Utc1310_C" },
    .{ 0x0060, "Utc1310_CPP" },
    .{ 0x0061, "Utc1310_C_Std" },
    .{ 0x0062, "Utc1310_CPP_Std" },
    .{ 0x0063, "Utc1310_LTCG_C" },
    .{ 0x0064, "Utc1310_LTCG_CPP" },
    .{ 0x0065, "Utc1310_POGO_I_C" },
    .{ 0x0066, "Utc1310_POGO_I_CPP" },
    .{ 0x0067, "Utc1310_POGO_O_C" },
    .{ 0x0068, "Utc1310_POGO_O_CPP" },
    .{ 0x0069, "AliasObj710" },
    .{ 0x006a, "AliasObj710p" },
    .{ 0x006b, "Cvtpgd1310" },
    .{ 0x006c, "Cvtpgd1310p" },
    .{ 0x006d, "Utc1400_C" },
    .{ 0x006e, "Utc1400_CPP" },
    .{ 0x006f, "Utc1400_C_Std" },
    .{ 0x0070, "Utc1400_CPP_Std" },
    .{ 0x0071, "Utc1400_LTCG_C" },
    .{ 0x0072, "Utc1400_LTCG_CPP" },
    .{ 0x0073, "Utc1400_POGO_I_C" },
    .{ 0x0074, "Utc1400_POGO_I_CPP" },
    .{ 0x0075, "Utc1400_POGO_O_C" },
    .{ 0x0076, "Utc1400_POGO_O_CPP" },
    .{ 0x0077, "Cvtpgd1400" },
    .{ 0x0078, "Linker800" },
    .{ 0x0079, "Cvtomf800" },
    .{ 0x007a, "Export800" },
    .{ 0x007b, "Implib800" },
    .{ 0x007c, "Cvtres800" },
    .{ 0x007d, "Masm800" },
    .{ 0x007e, "AliasObj800" },
    .{ 0x007f, "PhoenixPrerelease" },
    .{ 0x0080, "Utc1400_CVTCIL_C" },
    .{ 0x0081, "Utc1400_CVTCIL_CPP" },
    .{ 0x0082, "Utc1400_LTCG_MSIL" },
    .{ 0x0083, "Utc1500_C" },
    .{ 0x0084, "Utc1500_CPP" },
    .{ 0x0085, "Utc1500_C_Std" },
    .{ 0x0086, "Utc1500_CPP_Std" },
    .{ 0x0087, "Utc1500_CVTCIL_C" },
    .{ 0x0088, "Utc1500_CVTCIL_CPP" },
    .{ 0x0089, "Utc1500_LTCG_C" },
    .{ 0x008a, "Utc1500_LTCG_CPP" },
    .{ 0x008b, "Utc1500_LTCG_MSIL" },
    .{ 0x008c, "Utc1500_POGO_I_C" },
    .{ 0x008d, "Utc1500_POGO_I_CPP" },
    .{ 0x008e, "Utc1500_POGO_O_C" },
    .{ 0x008f, "Utc1500_POGO_O_CPP" },
    .{ 0x0090, "Cvtpgd1500" },
    .{ 0x0091, "Linker900" },
    .{ 0x0092, "Export900" },
    .{ 0x0093, "Implib900" },
    .{ 0x0094, "Cvtres900" },
    .{ 0x0095, "Masm900" },
    .{ 0x0096, "AliasObj900" },
    .{ 0x0097, "Resource" },
    .{ 0x0098, "AliasObj1000" },
    .{ 0x0099, "Cvtpgd1600" },
    .{ 0x009a, "Cvtres1000" },
    .{ 0x009b, "Export1000" },
    .{ 0x009c, "Implib1000" },
    .{ 0x009d, "Linker1000" },
    .{ 0x009e, "Masm1000" },
    .{ 0x009f, "Phx1600_C" },
    .{ 0x00a0, "Phx1600_CPP" },
    .{ 0x00a1, "Phx1600_CVTCIL_C" },
    .{ 0x00a2, "Phx1600_CVTCIL_CPP" },
    .{ 0x00a3, "Phx1600_LTCG_C" },
    .{ 0x00a4, "Phx1600_LTCG_CPP" },
    .{ 0x00a5, "Phx1600_LTCG_MSIL" },
    .{ 0x00a6, "Phx1600_POGO_I_C" },
    .{ 0x00a7, "Phx1600_POGO_I_CPP" },
    .{ 0x00a8, "Phx1600_POGO_O_C" },
    .{ 0x00a9, "Phx1600_POGO_O_CPP" },
    .{ 0x00aa, "Utc1600_C" },
    .{ 0x00ab, "Utc1600_CPP" },
    .{ 0x00ac, "Utc1600_CVTCIL_C" },
    .{ 0x00ad, "Utc1600_CVTCIL_CPP" },
    .{ 0x00ae, "Utc1600_LTCG_C" },
    .{ 0x00af, "Utc1600_LTCG_CPP" },
    .{ 0x00b0, "Utc1600_LTCG_MSIL" },
    .{ 0x00b1, "Utc1600_POGO_I_C" },
    .{ 0x00b2, "Utc1600_POGO_I_CPP" },
    .{ 0x00b3, "Utc1600_POGO_O_C" },
    .{ 0x00b4, "Utc1600_POGO_O_CPP" },
    .{ 0x00b5, "AliasObj1010" },
    .{ 0x00b6, "Cvtpgd1610" },
    .{ 0x00b7, "Cvtres1010" },
    .{ 0x00b8, "Export1010" },
    .{ 0x00b9, "Implib1010" },
    .{ 0x00ba, "Linker1010" },
    .{ 0x00bb, "Masm1010" },
    .{ 0x00bc, "Utc1610_C" },
    .{ 0x00bd, "Utc1610_CPP" },
    .{ 0x00be, "Utc1610_CVTCIL_C" },
    .{ 0x00bf, "Utc1610_CVTCIL_CPP" },
    .{ 0x00c0, "Utc1610_LTCG_C" },
    .{ 0x00c1, "Utc1610_LTCG_CPP" },
    .{ 0x00c2, "Utc1610_LTCG_MSIL" },
    .{ 0x00c3, "Utc1610_POGO_I_C" },
    .{ 0x00c4, "Utc1610_POGO_I_CPP" },
    .{ 0x00c5, "Utc1610_POGO_O_C" },
    .{ 0x00c6, "Utc1610_POGO_O_CPP" },
    .{ 0x00c7, "AliasObj1100" },
    .{ 0x00c8, "Cvtpgd1700" },
    .{ 0x00c9, "Cvtres1100" },
    .{ 0x00ca, "Export1100" },
    .{ 0x00cb, "Implib1100" },
    .{ 0x00cc, "Linker1100" },
    .{ 0x00cd, "Masm1100" },
    .{ 0x00ce, "Utc1700_C" },
    .{ 0x00cf, "Utc1700_CPP" },
    .{ 0x00d0, "Utc1700_CVTCIL_C" },
    .{ 0x00d1, "Utc1700_CVTCIL_CPP" },
    .{ 0x00d2, "Utc1700_LTCG_C" },
    .{ 0x00d3, "Utc1700_LTCG_CPP" },
    .{ 0x00d4, "Utc1700_LTCG_MSIL" },
    .{ 0x00d5, "Utc1700_POGO_I_C" },
    .{ 0x00d6, "Utc1700_POGO_I_CPP" },
    .{ 0x00d7, "Utc1700_POGO_O_C" },
    .{ 0x00d8, "Utc1700_POGO_O_CPP" },
    .{ 0x00d9, "AliasObj1200" },
    .{ 0x00da, "Cvtpgd1800" },
    .{ 0x00db, "Cvtres1200" },
    .{ 0x00dc, "Export1200" },
    .{ 0x00dd, "Implib1200" },
    .{ 0x00de, "Linker1200" },
    .{ 0x00df, "Masm1200" },
    .{ 0x00e0, "Utc1800_C" },
    .{ 0x00e1, "Utc1800_CPP" },
    .{ 0x00e2, "Utc1800_CVTCIL_C" },
    .{ 0x00e3, "Utc1800_CVTCIL_CPP" },
    .{ 0x00e4, "Utc1800_LTCG_C" },
    .{ 0x00e5, "Utc1800_LTCG_CPP" },
    .{ 0x00e6, "Utc1800_LTCG_MSIL" },
    .{ 0x00e7, "Utc1800_POGO_I_C" },
    .{ 0x00e8, "Utc1800_POGO_I_CPP" },
    .{ 0x00e9, "Utc1800_POGO_O_C" },
    .{ 0x00ea, "Utc1800_POGO_O_CPP" },
    .{ 0x00eb, "AliasObj1210" },
    .{ 0x00ec, "Cvtpgd1810" },
    .{ 0x00ed, "Cvtres1210" },
    .{ 0x00ee, "Export1210" },
    .{ 0x00ef, "Implib1210" },
    .{ 0x00f0, "Linker1210" },
    .{ 0x00f1, "Masm1210" },
    .{ 0x00f2, "Utc1810_C" },
    .{ 0x00f3, "Utc1810_CPP" },
    .{ 0x00f4, "Utc1810_CVTCIL_C" },
    .{ 0x00f5, "Utc1810_CVTCIL_CPP" },
    .{ 0x00f6, "Utc1810_LTCG_C" },
    .{ 0x00f7, "Utc1810_LTCG_CPP" },
    .{ 0x00f8, "Utc1810_LTCG_MSIL" },
    .{ 0x00f9, "Utc1810_POGO_I_C" },
    .{ 0x00fa, "Utc1810_POGO_I_CPP" },
    .{ 0x00fb, "Utc1810_POGO_O_C" },
    .{ 0x00fc, "Utc1810_POGO_O_CPP" },
    .{ 0x00fd, "AliasObj1400" },
    .{ 0x00fe, "Cvtpgd1900" },
    .{ 0x00ff, "Cvtres1400" },
    .{ 0x0100, "Export1400" },
    .{ 0x0101, "Implib1400" },
    .{ 0x0102, "Linker1400" },
    .{ 0x0103, "Masm1400" },
    .{ 0x0104, "Utc1900_C" },
    .{ 0x0105, "Utc1900_CPP" },
    .{ 0x0106, "Utc1900_CVTCIL_C" },
    .{ 0x0107, "Utc1900_CVTCIL_CPP" },
    .{ 0x0108, "Utc1900_LTCG_C" },
    .{ 0x0109, "Utc1900_LTCG_CPP" },
    .{ 0x010a, "Utc1900_LTCG_MSIL" },
    .{ 0x010b, "Utc1900_POGO_I_C" },
    .{ 0x010c, "Utc1900_POGO_I_CPP" },
    .{ 0x010d, "Utc1900_POGO_O_C" },
    .{ 0x010e, "Utc1900_POGO_O_CPP" },
};
pub const BUILD = [_]struct { u32, []const u8 }{
    .{ 7299, "VC6 tools (MASM 6.13)" },
    .{ 8168, "VC6 RTM" },
    .{ 8447, "VC6 SP3 (c2.dll back end)" },
    .{ 9044, "VC6 SP5/SP6 era" },
    .{ 9466, "VS2002 (7.0) RTM" },
    .{ 3077, "VS2003 (7.1) RTM" },
    .{ 4035, "VC 7.1 as shipped in the Windows Server 2003 SP1 SDK/DDK" },
    .{ 6030, "VS2003 (7.1) SP1" },
    .{ 50727, "VS2005 (8.0): RTM .42 and SP1 .762 both stamp 50727" },
    .{ 21022, "VS2008 (9.0) RTM" },
    .{ 30729, "VS2008 (9.0) SP1" },
    .{ 30319, "VS2010 (10.0) RTM" },
    .{ 40219, "VS2010 (10.0) SP1" },
};
