const std = @import("std");

// The oracle's carve (refimage.py) in Zig. Capstone 5.0.7, the version the Python side
// used, is compiled from source: same decoder, same instructions, comparable output.
pub fn build(b: *std.Build) void {
    const target = b.standardTargetOptions(.{});
    const optimize = b.standardOptimizeOption(.{ .preferred_optimize_mode = .ReleaseFast });

    const mod = b.createModule(.{ .root_source_file = b.path("src/main.zig"), .target = target, .optimize = optimize, .link_libc = true });
    const cs = b.dependency("capstone", .{}).path("");
    mod.addIncludePath(cs.path(b, "include"));
    mod.addIncludePath(cs);
    const flags = [_][]const u8{ "-DCAPSTONE_HAS_X86", "-DCAPSTONE_USE_SYS_DYN_MEM", "-std=c99", "-fno-sanitize=undefined" };
    const files = [_][]const u8{
        "cs.c", "Mapping.c", "MCInst.c", "MCInstrDesc.c", "MCRegisterInfo.c", "SStream.c", "utils.c",
        "arch/X86/X86Disassembler.c", "arch/X86/X86DisassemblerDecoder.c", "arch/X86/X86IntelInstPrinter.c",
        "arch/X86/X86ATTInstPrinter.c", "arch/X86/X86InstPrinterCommon.c", "arch/X86/X86Mapping.c", "arch/X86/X86Module.c",
    };
    mod.addCSourceFiles(.{ .root = cs, .files = &files, .flags = &flags });

    const tc = b.addTranslateC(.{ .root_source_file = b.path("src/c.h"), .target = target, .optimize = optimize });
    tc.addIncludePath(cs.path(b, "include"));
    mod.addImport("c", tc.createModule());

    const exe = b.addExecutable(.{ .name = "refimage", .root_module = mod });
    b.installArtifact(exe);
}
