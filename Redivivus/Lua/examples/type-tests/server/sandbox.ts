// The Lua library a mod has, and the parts the sandbox removes, which must not compile.

const s: string = string.format("%d-%s", 1, string.rep("a", 2));
const n: number = math.floor(math.max(1, 2)) + (6 & 3) + table.maxn([1]) + math.log10(10);
const ok = pcall(() => setmetatable({}, { __index: () => 1 }));
const [chunk] = load("return 1");
const [chunk2] = loadstring("return 2");
const kb: number = collectgarbage("count");
const co = coroutine.create(() => coroutine.yield(1));
void [s, n, ok, chunk, chunk2, kb, co, unpack([1, 2]), tostring(1), tonumber("2"), type(1), select("#", 1), rawequal(1, 1)];

// @ts-expect-error io is gone
io.write("x");
// @ts-expect-error os is gone
os.time();
// @ts-expect-error debug is gone
debug.traceback();
// @ts-expect-error package is gone
package.path;
// @ts-expect-error dofile is gone
dofile("x.lua");
// @ts-expect-error loadfile is gone
loadfile("x.lua");
// @ts-expect-error getfenv is gone
getfenv(1);
// @ts-expect-error setfenv is gone
setfenv(1, {});
// @ts-expect-error newproxy is gone
newproxy(true);
// @ts-expect-error jit is gone
jit.off();
// @ts-expect-error ffi is gone
ffi.cdef("int x;");
// @ts-expect-error string.dump is gone (no bytecode)
string.dump(() => 1);
// @ts-expect-error collectgarbage only counts
collectgarbage("collect");
// @ts-expect-error module is gone
module("x");
// @ts-expect-error rawlen is not in LuaJIT's 5.1 base library
rawlen({});
