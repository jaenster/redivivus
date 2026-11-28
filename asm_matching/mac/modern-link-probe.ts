// Link probe of the modern macOS build: how far are d2_modern_objs and the Win32Posix layer from
// linking into the d2_modern executable (cmake/mac-modern.cmake)?
//
//   node asm_matching/mac/modern-link-probe.ts --build <modern build dir> [--out <dir>]
//
// Build the objects first: `ninja -k 0 d2_modern_objs d2_win32posix_kernel d2_win32posix
// d2_modern_link_deps` (d2_win32posix exists once the layer has units beside the kernel's;
// d2_modern_link_deps holds the third-party stand-ins). The probe links every
// object of those targets with Apple clang from Xcode, on the Xcode SDK, against SDL3 and
// OpenGL, as the d2_modern target does, and reads what ld says. ld64 stops at its first failing
// phase, so it links twice: `-undefined error` for the undefined symbols, `-undefined
// dynamic_lookup` for the duplicate-symbol pass. Nothing is written into the tree; the result
// is <out>/modern-link-probe.tsv (one row per undefined symbol, with its class and owner),
// <out>/duplicates.tsv and a summary on stdout.
//
// Environment:
//   D2_XCODE   the Xcode developer directory (default /Applications/Xcode.app/Contents/Developer)
//   D2_SDL3    the SDL3 prefix (default /opt/homebrew)
//   D2_ARCH    the slice to link (default arm64)
//
// Classes (owner in brackets):
//   kernel    a Win32Posix kernel entry point - Win32PosixKernel.h, or a d2_platform.h one that
//             is not user or sockets (Kernel*.cpp)
//   user      a Win32PosixUser.h entry point, or a d2_platform.h window, message, rectangle,
//             keyboard or message-box one (User*.cpp)
//   sockets   Winsock (Win32PosixSockets.h, the WSA calls) and DirectSound (Sockets*.cpp,
//             DirectSound*.cpp)
//   shell     Win32PosixShell.h: the shell calls, the crash handler
//   renderer  the D2OpenGL renderer's Carbon, AGL and Core Graphics calls
//   3rdparty  Bink, the Intel JPEG Library, crashy
//   runtime   the MSVC runtime symbols a shared unit still reaches
//   tree      a game function declared and never defined for this build
import { execFileSync } from "node:child_process";
import { existsSync, mkdirSync, readdirSync, readFileSync, statSync, writeFileSync } from "node:fs";
import { dirname, join, resolve } from "node:path";
import { fileURLToPath } from "node:url";

const HERE = dirname(fileURLToPath(import.meta.url));
const ROOT = resolve(HERE, "../..");
const xcode = process.env.D2_XCODE ?? "/Applications/Xcode.app/Contents/Developer";
const toolchain = join(xcode, "Toolchains/XcodeDefault.xctoolchain/usr/bin");
const sdk = join(xcode, "Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk");
const sdl3 = process.env.D2_SDL3 ?? "/opt/homebrew";
const arch = process.env.D2_ARCH ?? "arm64";

const argv = process.argv.slice(2);
let buildDir = "";
let outDir = "";
for (let i = 0; i < argv.length; i++) {
    if (argv[i] === "--build") buildDir = resolve(argv[++i]);
    else if (argv[i] === "--out") outDir = resolve(argv[++i]);
}
if (!buildDir) {
    console.error("usage: node asm_matching/mac/modern-link-probe.ts --build <modern build dir> [--out <dir>]");
    process.exit(2);
}
outDir ||= join(buildDir, "modern-link-probe");
mkdirSync(outDir, { recursive: true });

// ---- the objects ----
function walk(dir: string, out: string[]) {
    for (const e of readdirSync(dir)) {
        const p = join(dir, e);
        if (statSync(p).isDirectory()) walk(p, out);
        else if (e.endsWith(".o")) out.push(p);
    }
}
const objs: string[] = [];
for (const [t, required] of [["d2_modern_objs", true], ["d2_win32posix_kernel", true], ["d2_win32posix", false],
    ["d2_modern_link_deps", false]] as const) {
    const d = join(buildDir, "CMakeFiles", `${t}.dir`);
    if (!existsSync(d)) {
        if (required) { console.error(`no ${d}: build ${t} first`); process.exit(2); }
        continue;
    }
    walk(d, objs);
}
objs.sort();
const objList = join(outDir, "objects.txt");
writeFileSync(objList, objs.map((o) => `"${o}"`).join("\n") + "\n");

function link(undefinedMode: string): string {
    const args = ["-arch", arch, "-isysroot", sdk, "-mmacosx-version-min=12.0", `@${objList}`,
        "-o", join(outDir, "probe.out"), "-L", join(sdl3, "lib"), "-lSDL3", "-framework", "OpenGL",
        "-Wl,-w", `-Wl,-undefined,${undefinedMode}`];
    try {
        return execFileSync(join(toolchain, "clang++"), args,
            { encoding: "utf8", maxBuffer: 1 << 30, stdio: ["ignore", "pipe", "pipe"] });
    } catch (e: any) {
        return String(e.stdout ?? "") + String(e.stderr ?? "");
    }
}

// ---- who declares what: the Win32Posix headers, by file ----
const headerOf = new Map<string, string>();
const posixDir = join(ROOT, "Source/StormMac/Win32Posix");
const HEADERS: [string, string][] = [["Win32PosixKernel.h", "kernel"], ["Win32PosixUser.h", "user"],
    ["Win32PosixSockets.h", "sockets"], ["Win32PosixDirectSound.h", "sockets"], ["Win32PosixShell.h", "shell"]];
for (const [file, cls] of HEADERS) {
    const p = join(posixDir, file);
    if (!existsSync(p)) continue;
    // a declaration: a return type, the name, an opening parenthesis (macros and comments aside)
    for (const m of readFileSync(p, "utf8").matchAll(/^(?!\s*(?:#|\/\/|typedef|return))[^\n;{}()]*?\b([A-Za-z_]\w*)\s*\(/gm)) {
        if (!headerOf.has(m[1])) headerOf.set(m[1], cls);
    }
}
// d2_platform.h declares the rest for every non-Windows build; these are the user and Winsock ones
const PLATFORM_USER = /^(PostQuitMessage|PostMessageA|GetSystemMetrics|SetWindowPos|ShowWindow|DestroyWindow|CreateWindowExA|GetClientRect|GetDC|ReleaseDC|MessageBoxA|SendMessageA|GetKeyState|SetRect)$/;
const PLATFORM_SOCKETS = /^(WSA\w+|DirectSoundCreate)$/;

const undefSyms: string[] = [];
const undefInfo = new Map<string, { n: number; ref: string; file: string }>();
{
    const out = link("error");
    writeFileSync(join(outDir, "ld-undefined.txt"), out);
    let cur = "";
    for (const l of out.split("\n")) {
        let m = l.match(/^ {2}"(.*)", referenced from:$/);
        if (m) { cur = m[1]; undefSyms.push(cur); undefInfo.set(cur, { n: 0, ref: "", file: "" }); continue; }
        m = l.match(/^ {6}(.*) in (.*)$/);
        if (m && cur) {
            const u = undefInfo.get(cur)!;
            if (!u.n) { u.ref = m[1]; u.file = m[2]; }
            u.n++;
        }
    }
}
const dups = new Map<string, Set<string>>();
{
    const out = link("dynamic_lookup");
    writeFileSync(join(outDir, "ld-duplicates.txt"), out);
    let cur = "";
    for (const l of out.split("\n")) {
        const m = l.match(/^duplicate symbol '?([^' ]+)'? in:$/);
        if (m) { cur = m[1]; if (!dups.has(cur)) dups.set(cur, new Set()); continue; }
        if (l.startsWith("    ") && cur) dups.get(cur)!.add(l.trim().replace(/^.*\/CMakeFiles\//, ""));
    }
}

// ld64 prints a C++ symbol demangled and a C one as its Mach-O name, underscore first
const dem = undefSyms.map((s) => (/[(:]/.test(s) || !s.startsWith("_") ? s : s.slice(1)));
const OWNER: Record<string, string> = { kernel: "M4", user: "M5", sockets: "M6" };
const rows: string[][] = [];
undefSyms.forEach((s, i) => {
    const d = dem[i];
    const paren = d.indexOf("(");
    const base = paren >= 0 ? d.slice(0, paren) : d;
    const parts = base.split("::");
    const last = parts[parts.length - 1];
    const ns = parts.slice(0, -1).join("::").replace(/^_+/, "");
    let cls: string;
    if (/^D2OpenGL::/.test(base)) cls = "renderer";
    else if (/^(Bink|ijl)|crashy|^D2CRASH_/.test(base)) cls = "3rdparty";
    else if (/^_*(eh_vector_|except_handler|purecall$)/.test(base)) cls = "runtime";
    else if ((ns === "" || ns === "Wrappers") && headerOf.has(last)) cls = headerOf.get(last)!;
    else if (ns === "" && PLATFORM_USER.test(last)) cls = "user";
    else if (ns === "" && PLATFORM_SOCKETS.test(last)) cls = "sockets";
    else if (ns === "" && /^[A-Z]\w*[a-z]\w*$/.test(last) && readPlatform().has(last)) cls = "kernel";
    else cls = "tree";
    const u = undefInfo.get(s)!;
    rows.push([cls, OWNER[cls] ?? "", s, String(u.n), u.ref, u.file.replace(/^.*\/CMakeFiles\//, "")]);
});
rows.sort((a, b) => a[0].localeCompare(b[0]) || a[2].localeCompare(b[2]));

// the names d2_platform.h declares for a build that is not Windows (read once)
function readPlatform(): Set<string> {
    const self = readPlatform as unknown as { cache?: Set<string> };
    if (!self.cache) {
        self.cache = new Set();
        const text = readFileSync(join(ROOT, "Source/D2Headers/d2_platform.h"), "utf8");
        for (const m of text.matchAll(/\b([A-Z][A-Za-z0-9]+)\s*\(/g)) self.cache.add(m[1]);
    }
    return self.cache;
}

writeFileSync(join(outDir, "modern-link-probe.tsv"),
    "class\towner\tsymbol\trefs\tfirst_referrer\tfirst_object\n" + rows.map((r) => r.join("\t")).join("\n") + "\n");
writeFileSync(join(outDir, "duplicates.tsv"), "symbol\tdefinitions\tfirst_objects\n" +
    [...dups].map(([s, f]) => `${s}\t${f.size}\t${[...f].slice(0, 3).join(" ")}`).join("\n") + "\n");

const count = new Map<string, number>();
for (const r of rows) count.set(r[0], (count.get(r[0]) ?? 0) + 1);
console.log(`objects: ${objs.length} (${arch})`);
console.log(`undefined symbols: ${rows.length}  (${[...count].sort().map(([k, v]) => `${k}=${v}`).join(" ")})`);
console.log(`duplicate symbols: ${dups.size}`);
console.log(`wrote ${outDir}/{modern-link-probe.tsv,duplicates.tsv,ld-undefined.txt,ld-duplicates.txt}`);
