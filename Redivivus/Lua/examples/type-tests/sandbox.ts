// The Lua library a mod has, and the parts the sandbox removes, which must not compile.

const s: string = string.format("%d-%s", 1, string.rep("a", 2));
const n: number = math.floor(math.max(1, 2)) + (6 & 3) + table.maxn([1]) + math.log10(10);
const ok = pcall(() => setmetatable({}, { __index: () => 1 }));
const [chunk] = load("return 1");
const kb: number = collectgarbage("count");
void [s, n, ok, chunk, kb, unpack([1, 2]), tostring(1), tonumber("2"), type(1), select("#", 1), rawequal(1, 1)];

// @ts-expect-error io is gone
io.write("x");
// @ts-expect-error os is gone
os.time();
// @ts-expect-error debug is gone
debug.traceback();
// @ts-expect-error getfenv is gone
getfenv(1);
// @ts-expect-error jit is gone
jit.off();
// @ts-expect-error string.dump is gone (no bytecode)
string.dump(() => 1);
// @ts-expect-error collectgarbage only counts
collectgarbage("collect");
