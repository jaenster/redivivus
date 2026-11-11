//! oracle-zig: the msvc8 oracle's core in Zig, one executable with a subcommand each.
//!
//!   oracle-zig carve [Game.exe] [functions.tsv]   carve the original into functions (refimage.py)
//!   oracle-zig audit --root <dir> [--funcmap F] [--libmap L] [--out O]
//!                                                 compare every mapped function (audit.py)
//!   oracle-zig crt --root <dir> [--out F]         the linked CRT, by bytes (crtmap.py)
//!   oracle-zig map --root <dir> [--out F]         our functions -> original addresses (funcmap.py)
//!   oracle-zig libmap --root <dir> [--exe-map F] [--funcmap F] [--out F]
//!                                                 the linked runtime's functions (libmap.py)
//!   oracle-zig objects --root <dir> [--funcmap F] [--out F]
//!                                                 functions filed apart from their callers (objects.py)
//!   oracle-zig modules order --repo <tree> < tus  our TUs in link order (modules.py)
//!   oracle-zig data --root <dir> [extents|strings|audit] [...]
//!                                                 compare our initialised data (datamatch.py)
//!   oracle-zig classify --root <dir> [--out O] [--tag T] [--single]
//!                                                 tag why each function differs (classify.py)
//!   oracle-zig permodule --root <dir> --map M [--audit A] [--data D] [--out O]
//!                                                 the figures per module (permodule.py)
//!   oracle-zig build --root <dir> [build.py's options]
//!                                                 translate + compile the tree under VC8 (build.py)
//!   oracle-zig tucache lookup|store|prune [...]   the per-TU gcc result cache (recon/runs/tucache.py)
//!   oracle-zig trial --root <dir> --state | --out DIR [-j N] [--batch N] [--expect KEY] < trials
//!                                                 compile form-permuter trials beside the build
//!   oracle-zig judge --root <dir> < requests      judge trial objects against the original
//!
//! The tools around them, each with its script's command line (`--root` optional, cli.root):
//!   quick, check, diff, snippet, objdiff, diffgrams, sizetrend, abcompare, tulayout, svg,
//!   rich, compid, commit; and the setup and export helpers mpq-extract, msi-paths,
//!   vc8-layout, psdk-layout, project-json.
const std = @import("std");
const builtin = @import("builtin");
const cli = @import("cli.zig");
const proc = @import("proc.zig");
const util = @import("util.zig");
const refimage = @import("refimage.zig");
const audit = @import("audit.zig");
const crtmap = @import("crtmap.zig");
const funcmap = @import("funcmap.zig");
const libmap = @import("libmap.zig");
const objects = @import("objects.zig");
const modules = @import("modules.zig");
const datamatch = @import("datamatch.zig");
const classify = @import("classify.zig");
const permodule = @import("permodule.zig");
const compile = @import("compile.zig");
const tucache = @import("tucache.zig");
const permute = @import("permute.zig");
const asmdiff = @import("asmdiff.zig");
const check = @import("check.zig");
const tools = @import("tools.zig");
const stamps = @import("stamps.zig");
const svg = @import("svg.zig");
const projectjson = @import("projectjson.zig");
const snippet = @import("snippet.zig");
const quick = @import("quick.zig");
const mpq = @import("mpq.zig");
const layout = @import("layout.zig");
const commit = @import("commit.zig");
const x64 = @import("x64.zig");

const USAGE =
    \\usage: oracle-zig carve [Game.exe] [functions.tsv]
    \\       oracle-zig audit --root <asm_matching/msvc8> [--funcmap F] [--libmap L] [--out O]
    \\       oracle-zig crt --root <asm_matching/msvc8> [--out F]
    \\       oracle-zig map --root <asm_matching/msvc8> [--out F]
    \\       oracle-zig libmap --root <asm_matching/msvc8> [--exe-map F] [--funcmap F] [--out F]
    \\       oracle-zig objects --root <asm_matching/msvc8> [--funcmap F] [--out F]
    \\       oracle-zig modules order --repo <tree> < tus
    \\       oracle-zig data --root <asm_matching/msvc8> [extents|strings|audit] [options]
    \\       oracle-zig classify --root <asm_matching/msvc8> [--out O] [--tag T] [--single]
    \\       oracle-zig permodule --root <asm_matching/msvc8> --map M [--audit A] [--data D] [--out O]
    \\       oracle-zig build --root <asm_matching/msvc8> [-j N] [--batch N] [--force] [--with-data]
    \\                        [--extra DIR --support "A.cpp B.cpp"] [PATTERN ...]
    \\       oracle-zig tucache lookup|store|prune [tucache.py's options]
    \\       oracle-zig trial --root <asm_matching/msvc8> --state
    \\       oracle-zig trial --root <asm_matching/msvc8> --out DIR [-j N] [--batch N] [--expect KEY] < trials
    \\       oracle-zig judge --root <asm_matching/msvc8> < requests
    \\       oracle-zig quick [-j N] [--verify] [--exe-map F]
    \\       oracle-zig all --root <asm_matching/msvc8> [build options]  (Windows host)
    \\       oracle-zig check [--update] [PATTERN ...]
    \\       oracle-zig diff [--funcmap F] <name|addr>
    \\       oracle-zig snippet FILE [--func NAME [--ref ADDR]] [--cflags FLAGS] [--tu TU]
    \\       oracle-zig objdiff <objdir A> <objdir B> [PATTERN ...]
    \\       oracle-zig diffgrams [--top N] [--max-len K] [--examples]
    \\       oracle-zig sizetrend [--audit A] [--funcmap F] [--out O] [--label T]
    \\       oracle-zig abcompare <baseline audit.tsv> <experiment audit.tsv> [--depth N] [--list]
    \\       oracle-zig tulayout [--tu PATTERN] [--intruders]
    \\       oracle-zig svg [--summary S] [--out O]
    \\       oracle-zig rich <image>  |  compid <obj|lib> ...
    \\       oracle-zig commit [--base B] [--tooling LINE] [-m MESSAGE]
    \\       oracle-zig mpq-extract <patch.exe> <member> <out>  |  msi-paths [--cabs] <msi>
    \\       oracle-zig vc8-layout <paths.tsv> <keyed> <dest>  |  psdk-layout <tsv> <keyed> <dest> <prefix>
    \\       oracle-zig project-json <recon/project.json> <out>
    \\
;

fn mapRoot(args: []const []const u8) ?[]const u8 {
    for (args, 0..) |arg, i| {
        if (std.mem.eql(u8, arg, "--root") and i + 1 < args.len) return args[i + 1];
    }
    return null;
}

fn runAllStep(name: []const u8, argv: []const []const u8) !void {
    util.say(try std.fmt.allocPrint(util.gpa, "all: {s}\n", .{name}));
    const result = try proc.run(argv, .{});
    util.say(result.out);
    cli.sayErr(result.err);
    if (result.timed_out or result.status != 0)
        cli.exitMsg("all: {s} failed (rc={d})", .{ name, result.status });
}

fn allWindows(args: []const []const u8) !void {
    var given_root: ?[]const u8 = null;
    var build_args: std.ArrayList([]const u8) = .empty;
    var i: usize = 0;
    while (i < args.len) : (i += 1) {
        const arg = args[i];
        if (std.mem.eql(u8, arg, "--root")) {
            i += 1;
            if (i >= args.len or args[i].len == 0) cli.exitMsg("all: --root requires a directory", .{});
            given_root = args[i];
        } else if (std.mem.startsWith(u8, arg, "--root=")) {
            if (arg.len == "--root=".len) cli.exitMsg("all: --root requires a directory", .{});
            given_root = arg["--root=".len..];
        } else try build_args.append(util.gpa, arg);
    }
    const root = try cli.root(given_root);
    const cfg = try audit.Config.init(root);
    if (cfg.experiment) cli.exitMsg("all: the baseline only (map, libmap, and audit write evidence/)", .{});
    try refimage.ensureFunctions(root);

    const self = try cli.selfExe();
    var build: std.ArrayList([]const u8) = .empty;
    try build.appendSlice(util.gpa, &.{ self, "build", "--root", root });
    try build.appendSlice(util.gpa, build_args.items);
    try runAllStep("build", build.items);
    try runAllStep("map", &.{ self, "map", "--root", root });
    try runAllStep("libmap", &.{ self, "libmap", "--root", root });
    try runAllStep("audit", &.{ self, "audit", "--root", root });
}

pub fn main(init: std.process.Init.Minimal) !void {
    var it = try std.process.Args.Iterator.initAllocator(init.args, util.gpa);
    defer it.deinit();
    _ = it.next();
    var args: std.ArrayList([]const u8) = .empty;
    while (it.next()) |a| try args.append(util.gpa, try util.gpa.dupe(u8, a));
    if (args.items.len == 0) {
        std.debug.print(USAGE, .{});
        std.process.exit(2);
    }
    const cmd = args.items[0];
    const rest = args.items[1..];
    if (std.mem.eql(u8, cmd, "carve")) return refimage.carveMain(rest);
    if (std.mem.eql(u8, cmd, "audit")) return audit.main(rest);
    if (std.mem.eql(u8, cmd, "crt")) {
        if (builtin.os.tag == .windows) if (mapRoot(rest)) |root| try refimage.ensureFunctions(try cli.root(root));
        return crtmap.main(rest);
    }
    if (std.mem.eql(u8, cmd, "map")) {
        if (builtin.os.tag == .windows) if (mapRoot(rest)) |root| try refimage.ensureFunctions(try cli.root(root));
        return funcmap.main(rest);
    }
    if (std.mem.eql(u8, cmd, "libmap")) return libmap.main(rest);
    if (std.mem.eql(u8, cmd, "objects")) return objects.main(rest);
    if (std.mem.eql(u8, cmd, "modules")) return modules.main(rest);
    if (std.mem.eql(u8, cmd, "data")) return datamatch.main(rest);
    if (std.mem.eql(u8, cmd, "classify")) return classify.main(rest);
    if (std.mem.eql(u8, cmd, "permodule")) return permodule.main(rest);
    if (std.mem.eql(u8, cmd, "build")) return compile.main(rest);
    if (std.mem.eql(u8, cmd, "tucache")) return tucache.main(rest);
    if (std.mem.eql(u8, cmd, "trial") or std.mem.eql(u8, cmd, "judge")) return permute.main(cmd, rest);
    if (std.mem.eql(u8, cmd, "diff")) return asmdiff.main(rest);
    if (std.mem.eql(u8, cmd, "check")) return check.main(rest);
    if (std.mem.eql(u8, cmd, "objdiff")) return tools.objdiffMain(rest);
    if (std.mem.eql(u8, cmd, "diffgrams")) return tools.diffgramsMain(rest);
    if (std.mem.eql(u8, cmd, "sizetrend")) return tools.sizetrendMain(rest);
    if (std.mem.eql(u8, cmd, "abcompare")) return tools.abcompareMain(rest);
    if (std.mem.eql(u8, cmd, "tulayout")) return tools.tulayoutMain(rest);
    if (std.mem.eql(u8, cmd, "rich")) return stamps.richMain(rest);
    if (std.mem.eql(u8, cmd, "compid")) return stamps.compidMain(rest);
    if (std.mem.eql(u8, cmd, "svg")) return svg.main(rest);
    if (std.mem.eql(u8, cmd, "project-json")) return projectjson.main(rest);
    if (std.mem.eql(u8, cmd, "snippet")) return snippet.main(rest);
    if (std.mem.eql(u8, cmd, "quick")) return quick.main(rest);
    if (builtin.os.tag == .windows and std.mem.eql(u8, cmd, "all")) return allWindows(rest);
    if (std.mem.eql(u8, cmd, "mpq-extract")) return mpq.main(rest);
    if (std.mem.eql(u8, cmd, "msi-paths")) return layout.msiPathsMain(rest);
    if (std.mem.eql(u8, cmd, "vc8-layout")) return layout.vc8LayoutMain(rest);
    if (std.mem.eql(u8, cmd, "psdk-layout")) return layout.psdkLayoutMain(rest);
    if (std.mem.eql(u8, cmd, "commit")) return commit.main(rest);
    if (std.mem.eql(u8, cmd, "x64")) return x64.main(rest);
    std.debug.print(USAGE, .{});
    std.process.exit(2);
}

test {
    _ = @import("carve_seed.zig");
    _ = datamatch;
    _ = classify;
    _ = permodule;
    _ = permute;
}
