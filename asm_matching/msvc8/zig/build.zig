const std = @import("std");

// oracle-zig: the oracle's core (carve, decode, compare, audit) in Zig. Capstone 5.0.7, the
// version the Python side uses, is compiled from source: same decoder, same text, same rows.
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

    const exe = b.addExecutable(.{ .name = "oracle-zig", .root_module = mod });
    b.installArtifact(exe);

    const test_step = b.step("test", "run the unit tests");
    const tests = b.addTest(.{ .root_module = mod });
    test_step.dependOn(&b.addRunArtifact(tests).step);

    // The conditional platform router does not make the Windows backend's
    // own tests visible to Zig's root test discovery, so compile it explicitly.
    if (target.result.os.tag == .windows) {
        const proc_test_mod = b.createModule(.{
            .root_source_file = b.path("src/proc_windows.zig"),
            .target = target,
            .optimize = .Debug,
            .link_libc = true,
        });
        proc_test_mod.addImport("c", tc.createModule());
        const proc_tests = b.addTest(.{ .root_module = proc_test_mod });
        test_step.dependOn(&b.addRunArtifact(proc_tests).step);
    }
}
