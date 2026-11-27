/*
 * difftest - call the same 1.14d function in the original Game.exe and in the
 * rebuilt exe with the same inputs, and compare what each one did.
 *
 *   difftest.exe <Game.exe> <ours.exe> <spec.txt> <out.jsonl> [iters] [seed] [only-id]
 *
 * Both images are mapped by hand, never started: no entry point, no CRT
 * initialisers, no DllMain. Game.exe sits at its own base 0x400000 (this host is
 * linked high so the range is free); ours is relocated to wherever it lands.
 * Game.exe's imports are all traps (a leaf function makes no calls). Ours get the
 * system DLLs for real, because GCC calls memcpy/memset for plain assignments,
 * and traps for everything else.
 *
 * Per input set, each side starts from the same state:
 *   - registers and stack arguments as the spec places them for that side;
 *   - the arena (the memory every pointer argument points into) byte-identical,
 *     at the same address for both, so pointer values compare as values;
 *   - each mapped global filled with the same bytes on both sides;
 *   - a private stack filled with the same pattern, so uninitialised reads agree
 *     unless the function itself differs;
 *   - every other byte of both images at its file contents (restored after each
 *     call from a pristine copy, found dirty by the write watch).
 * Compared afterwards: the return value (masked to its declared size), the
 * arena, the mapped globals, every other write into either image, the callee-saved
 * registers, the stack the callee popped, and whether and how it faulted.
 */
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>

/* ---------------------------------------------------------------- the call */

typedef struct CallCtx {
    uint32_t fn;                                   /*   0 */
    uint32_t eax, ecx, edx, ebx, esi, edi;         /*   4 */
    uint32_t nstack;                               /*  28 */
    uint32_t stack[16];                            /*  32 */
    uint32_t stack_top;                            /*  96 */
    uint32_t want_fpu;                             /* 100 */
    uint32_t r_eax, r_edx, r_ebx, r_esi, r_edi, r_ebp; /* 104 */
    int32_t  esp_delta;                            /* 128 */
    uint32_t status;                               /* 132: 0 returned, 1 fault, 2 timeout */
    double   st0;                                  /* 136 */
    uint32_t fault_code;                           /* 144 */
    uint32_t fault_eip;                            /* 148 */
    uint32_t stack_limit;                          /* 152 */
    uint32_t ebp_in;                               /* 156 */
    uint32_t fault_addr;                           /* 160 */
} CallCtx;
_Static_assert(offsetof(CallCtx, stack_top) == 96, "ctx layout");
_Static_assert(offsetof(CallCtx, r_eax) == 104, "ctx layout");
_Static_assert(offsetof(CallCtx, st0) == 136, "ctx layout");
_Static_assert(offsetof(CallCtx, stack_limit) == 152, "ctx layout");
_Static_assert(offsetof(CallCtx, ebp_in) == 156, "ctx layout");

extern int tramp_call(CallCtx *ctx);
extern void tramp_escape(void);
extern CallCtx *g_tramp_ctx;

static volatile LONG g_in_call;
static volatile LONG g_call_seq;
static HANDLE g_main_thread;
static DWORD g_timeout_ms = 100;
static uint32_t g_code_lo[2], g_code_hi[2];   /* the two images: where a hung call may be stopped */

static LONG CALLBACK on_exception(EXCEPTION_POINTERS *ep)
{
    DWORD code = ep->ExceptionRecord->ExceptionCode;
    if (!g_in_call || GetCurrentThreadId() != GetThreadId(g_main_thread))
        return EXCEPTION_CONTINUE_SEARCH;
    if (code == 0x406D1388 || code == DBG_PRINTEXCEPTION_C || code == 0x4001000A)
        return EXCEPTION_CONTINUE_SEARCH;
    CallCtx *c = g_tramp_ctx;
    c->status = 1;
    c->fault_code = code;
    c->fault_eip = ep->ContextRecord->Eip;
    c->fault_addr = ep->ExceptionRecord->NumberParameters >= 2
                  ? (uint32_t)ep->ExceptionRecord->ExceptionInformation[1] : 0;
    ep->ContextRecord->Eip = (DWORD)(uintptr_t)tramp_escape;
    InterlockedExchange(&g_in_call, 0);
    return EXCEPTION_CONTINUE_EXECUTION;
}

/* the process-level net: a wild write can land in wine's own memory and wedge the
 * main thread outside any image, where it cannot be redirected. With no call
 * finishing for this long the host records the function as wedged and exits. */
static const char *g_out_path;
static volatile LONG g_cur_id = -1;
static char g_cur_name[200];

static void wedged_exit(void)
{
    HANDLE h = CreateFileA(g_out_path, FILE_APPEND_DATA, FILE_SHARE_READ | FILE_SHARE_WRITE, 0, OPEN_ALWAYS, 0, 0);
    if (h != INVALID_HANDLE_VALUE) {
        char line[400]; DWORD w;
        int n = snprintf(line, sizeof line, "{\"id\":%ld,\"name\":\"%s\",\"wedged\":true}\n", g_cur_id, g_cur_name);
        WriteFile(h, line, n, &w, 0);
        CloseHandle(h);
    }
    TerminateProcess(GetCurrentProcess(), 3);
}

static DWORD WINAPI watchdog(LPVOID unused)
{
    (void)unused;
    LONG seen = -1; DWORD since = 0;
    LONG last_seq = -1; DWORD last_change = GetTickCount();
    for (;;) {
        Sleep(10);
        LONG seq = g_call_seq;
        if (seq != last_seq) { last_seq = seq; last_change = GetTickCount(); }
        else if (g_cur_id >= 0 && GetTickCount() - last_change > g_timeout_ms + 8000) wedged_exit();
        if (!g_in_call) { seen = -1; continue; }
        if (seq != seen) { seen = seq; since = GetTickCount(); continue; }
        if (GetTickCount() - since < g_timeout_ms) continue;
        SuspendThread(g_main_thread);
        if (g_in_call && g_call_seq == seen) {
            CONTEXT cx; memset(&cx, 0, sizeof cx);
            cx.ContextFlags = CONTEXT_CONTROL;
            /* only stop it inside one of the images: redirecting a thread that is
             * in wine's own code (exception dispatch, a heap lock) wedges the process */
            int in_code = 0;
            if (GetThreadContext(g_main_thread, &cx))
                for (int q = 0; q < 2; q++) in_code |= cx.Eip >= g_code_lo[q] && cx.Eip < g_code_hi[q];
            if (in_code) {
                g_tramp_ctx->status = 2;
                g_tramp_ctx->fault_eip = cx.Eip;
                cx.Eip = (DWORD)(uintptr_t)tramp_escape;
                SetThreadContext(g_main_thread, &cx);
                InterlockedExchange(&g_in_call, 0);
            }
        }
        ResumeThread(g_main_thread);
        if (!g_in_call) seen = -1;
    }
    return 0;
}

static void run_call(CallCtx *c)
{
    c->status = 0; c->fault_code = 0; c->fault_eip = 0; c->fault_addr = 0;
    InterlockedIncrement(&g_call_seq);
    InterlockedExchange(&g_in_call, 1);
    tramp_call(c);
    InterlockedExchange(&g_in_call, 0);
}

/* ---------------------------------------------------------------- images */

typedef struct Range { uint32_t lo, hi; } Range;

typedef struct Image {
    const char *path;
    uint8_t *base;          /* where it is mapped */
    uint32_t pref;          /* ImageBase in the header */
    uint32_t size;          /* bytes mapped */
    uint8_t *pristine;      /* copy of the mapped bytes after load */
    int watch;              /* MEM_WRITE_WATCH is live */
    Range writable[16]; int nwritable;
    uint32_t trap_base, trap_count; /* import trap addresses */
    char **trap_names;
} Image;

static void die(const char *fmt, const char *a)
{
    fprintf(stderr, fmt, a); fputc('\n', stderr); exit(2);
}

static uint8_t *read_file(const char *path, DWORD *len)
{
    HANDLE h = CreateFileA(path, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
    if (h == INVALID_HANDLE_VALUE) die("cannot open %s", path);
    DWORD n = GetFileSize(h, 0), got = 0;
    uint8_t *buf = malloc(n);
    if (!buf || !ReadFile(h, buf, n, &got, 0) || got != n) die("cannot read %s", path);
    CloseHandle(h);
    *len = n;
    return buf;
}

static int is_debug_section(const IMAGE_SECTION_HEADER *s)
{
    return memcmp(s->Name, ".debug_", 7) == 0 || s->Name[0] == '/';
}

/* real_imports: resolve through LoadLibrary where the DLL is a system one, trap
 * otherwise. Every trap is a distinct no-access address, so a fault names the
 * import that was called. */
static void map_image(Image *im, const char *path, int at_pref, int real_imports)
{
    DWORD flen;
    uint8_t *f = read_file(path, &flen);
    IMAGE_DOS_HEADER *dos = (IMAGE_DOS_HEADER *)f;
    IMAGE_NT_HEADERS32 *nt = (IMAGE_NT_HEADERS32 *)(f + dos->e_lfanew);
    IMAGE_SECTION_HEADER *sec = IMAGE_FIRST_SECTION(nt);
    int nsec = nt->FileHeader.NumberOfSections;
    uint32_t end = nt->OptionalHeader.SizeOfHeaders;
    for (int i = 0; i < nsec; i++) {
        if (is_debug_section(&sec[i])) continue;
        uint32_t e = sec[i].VirtualAddress + (sec[i].Misc.VirtualSize ? sec[i].Misc.VirtualSize : sec[i].SizeOfRawData);
        if (e > end) end = e;
    }
    end = (end + 0xfff) & ~0xfffu;
    im->path = path;
    im->pref = nt->OptionalHeader.ImageBase;
    im->size = end;
    DWORD type = MEM_RESERVE | MEM_COMMIT;
    void *want = at_pref ? (void *)(uintptr_t)im->pref : 0;
    im->base = VirtualAlloc(want, end, type | MEM_WRITE_WATCH, PAGE_EXECUTE_READWRITE);
    im->watch = im->base != 0;
    if (!im->base) im->base = VirtualAlloc(want, end, type, PAGE_EXECUTE_READWRITE);
    if (!im->base && want) {
        /* wine maps its NLS and locale files low before main() runs, so
         * 0x400000 is usually taken; every image here has a .reloc */
        im->base = VirtualAlloc(0, end, type | MEM_WRITE_WATCH, PAGE_EXECUTE_READWRITE);
        im->watch = im->base != 0;
        if (!im->base) im->base = VirtualAlloc(0, end, type, PAGE_EXECUTE_READWRITE);
    }
    if (!im->base) die("cannot map %s", path);

    memcpy(im->base, f, nt->OptionalHeader.SizeOfHeaders);
    for (int i = 0; i < nsec; i++) {
        if (is_debug_section(&sec[i])) continue;
        uint32_t raw = sec[i].SizeOfRawData, vs = sec[i].Misc.VirtualSize;
        uint32_t n = vs && vs < raw ? vs : raw;
        if (n && sec[i].PointerToRawData + n <= flen)
            memcpy(im->base + sec[i].VirtualAddress, f + sec[i].PointerToRawData, n);
        if ((sec[i].Characteristics & IMAGE_SCN_MEM_WRITE) && im->nwritable < 16) {
            im->writable[im->nwritable].lo = sec[i].VirtualAddress;
            im->writable[im->nwritable].hi = sec[i].VirtualAddress + (vs > raw ? vs : raw);
            im->nwritable++;
        }
    }

    /* relocations */
    int32_t delta = (int32_t)((uint32_t)(uintptr_t)im->base - im->pref);
    IMAGE_DATA_DIRECTORY *rd = &nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC];
    if (delta && rd->Size) {
        uint8_t *p = im->base + rd->VirtualAddress, *pe = p + rd->Size;
        while (p < pe) {
            IMAGE_BASE_RELOCATION *b = (IMAGE_BASE_RELOCATION *)p;
            if (!b->SizeOfBlock) break;
            uint16_t *e = (uint16_t *)(b + 1);
            int cnt = (b->SizeOfBlock - sizeof *b) / 2;
            for (int i = 0; i < cnt; i++) {
                int t = e[i] >> 12, off = e[i] & 0xfff;
                if (t == IMAGE_REL_BASED_HIGHLOW)
                    *(uint32_t *)(im->base + b->VirtualAddress + off) += delta;
            }
            p += b->SizeOfBlock;
        }
    } else if (delta) die("%s needs relocating and has no .reloc", path);

    /* imports */
    IMAGE_DATA_DIRECTORY *id = &nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT];
    uint32_t ntrap = 0;
    if (id->Size) {
        for (IMAGE_IMPORT_DESCRIPTOR *d = (void *)(im->base + id->VirtualAddress); d->Name; d++)
            for (uint32_t *t = (uint32_t *)(im->base + d->FirstThunk); *t; t++) ntrap++;
        uint8_t *traps = VirtualAlloc(0, ((ntrap + 0xfff) & ~0xfffu) + 0x1000,
                                      MEM_RESERVE, PAGE_NOACCESS);
        im->trap_base = (uint32_t)(uintptr_t)traps;
        im->trap_count = ntrap;
        im->trap_names = calloc(ntrap + 1, sizeof(char *));
        uint32_t k = 0;
        for (IMAGE_IMPORT_DESCRIPTOR *d = (void *)(im->base + id->VirtualAddress); d->Name; d++) {
            const char *dll = (const char *)(im->base + d->Name);
            HMODULE h = 0;
            if (real_imports) {
                /* the C runtime and the kernel only: GCC calls memcpy/memset and
                 * friends for plain C, anything else a leaf reaches is a finding.
                 * Loading the graphics and sound DLLs would also start wine's
                 * Vulkan layer for nothing. */
                static const char *ok[] = {"msvcrt.dll", "kernel32.dll", "ntdll.dll", 0};
                for (int q = 0; ok[q]; q++)
                    if (!lstrcmpiA(dll, ok[q])) h = LoadLibraryA(dll);
            }
            uint32_t *iat = (uint32_t *)(im->base + d->FirstThunk);
            uint32_t *ilt = d->OriginalFirstThunk ? (uint32_t *)(im->base + d->OriginalFirstThunk) : iat;
            for (; *iat; iat++, ilt++, k++) {
                const char *nm; char ord[32];
                FARPROC fp = 0;
                if (*ilt & 0x80000000u) {
                    sprintf(ord, "#%u", *ilt & 0xffff); nm = ord;
                    if (h) fp = GetProcAddress(h, (LPCSTR)(uintptr_t)(*ilt & 0xffff));
                } else {
                    nm = (const char *)(im->base + *ilt + 2);
                    if (h) fp = GetProcAddress(h, nm);
                }
                size_t L = strlen(dll) + strlen(nm) + 2;
                im->trap_names[k] = malloc(L);
                snprintf(im->trap_names[k], L, "%s!%s", dll, nm);
                *iat = fp ? (uint32_t)(uintptr_t)fp : im->trap_base + k;
            }
        }
    }
    im->pristine = malloc(im->size);
    memcpy(im->pristine, im->base, im->size);
    free(f);
}


/* ---------------------------------------------------------------- spec */

enum { LOC_EAX, LOC_ECX, LOC_EDX, LOC_EBX, LOC_ESI, LOC_EDI, LOC_STACK, LOC_NONE };
enum { GEN_INT, GEN_UINT, GEN_BOOL, GEN_PTR, GEN_STR, GEN_F32, GEN_F64 };

typedef struct Param { int gen, size, oloc, uloc; char name[64]; } Param;
typedef struct Glob { uint32_t ova, uva, size; int randomize; char name[96]; } Glob;

typedef struct Spec {
    int id;
    char name[200];
    uint32_t ova, uva;          /* 1.14d va; ours va as nm prints it (pref base) */
    char rkind; int rsize;      /* v i u p b f d q */
    int ocleanup, ucleanup;     /* bytes the callee pops, per the spec; -1 unknown */
    Param p[12]; int np;
    Glob g[48]; int ng;
} Spec;

static int parse_loc(const char *s)
{
    static const char *n[] = {"eax", "ecx", "edx", "ebx", "esi", "edi", "stack"};
    for (int i = 0; i < 7; i++) if (!strcmp(s, n[i])) return i;
    return LOC_NONE;
}

static int parse_gen(const char *s)
{
    static const char *n[] = {"int", "uint", "bool", "ptr", "str", "f32", "f64"};
    for (int i = 0; i < 7; i++) if (!strcmp(s, n[i])) return i;
    return -1;
}

/* Text format, one block per function:
 *   F <id> <name>
 *   A <orig va> <ours va> <ret kind> <ret size> <orig cleanup> <ours cleanup>
 *   P <name> <gen> <size> <orig loc> <ours loc>
 *   G <orig va> <ours va> <size> <randomize 0|1> <name>
 *   E
 */
static int load_specs(const char *path, Spec **out)
{
    FILE *fp = fopen(path, "r");
    if (!fp) die("cannot open spec %s", path);
    int cap = 64, n = 0;
    Spec *v = calloc(cap, sizeof *v);
    char line[512];
    Spec *cur = 0;
    while (fgets(line, sizeof line, fp)) {
        line[strcspn(line, "\r\n")] = 0;
        if (!line[0] || line[0] == '#') continue;
        if (line[0] == 'F') {
            if (n == cap) { cap *= 2; v = realloc(v, cap * sizeof *v); }
            cur = &v[n++]; memset(cur, 0, sizeof *cur);
            sscanf(line, "F %d %199s", &cur->id, cur->name);
        } else if (!cur) {
            continue;
        } else if (line[0] == 'A') {
            char rk[8];
            sscanf(line, "A %x %x %7s %d %d %d", &cur->ova, &cur->uva, rk, &cur->rsize, &cur->ocleanup, &cur->ucleanup);
            cur->rkind = rk[0];
        } else if (line[0] == 'P' && cur->np < 12) {
            Param *p = &cur->p[cur->np++];
            char g[16], o[16], u[16];
            sscanf(line, "P %63s %15s %d %15s %15s", p->name, g, &p->size, o, u);
            p->gen = parse_gen(g); p->oloc = parse_loc(o); p->uloc = parse_loc(u);
        } else if (line[0] == 'G' && cur->ng < 48) {
            Glob *g = &cur->g[cur->ng++];
            sscanf(line, "G %x %x %u %d %95s", &g->ova, &g->uva, &g->size, &g->randomize, g->name);
        }
    }
    fclose(fp);
    *out = v;
    return n;
}

/* ---------------------------------------------------------------- inputs */

#define GLOB_MAX 0x8000u        /* bytes of one global that are filled and compared */
#define ARENA_BASE  0x20000000u
#define ARENA_CHUNK 0x8000u          /* usable bytes per chunk */
#define ARENA_NCHUNK 8               /* each chunk followed by a no-access page */
#define ARENA_SPAN  (ARENA_CHUNK + 0x1000u)
#define ARENA_SIZE  (ARENA_SPAN * ARENA_NCHUNK)
#define STACK_SIZE  0x40000u

static uint8_t *g_arena;
static uint8_t *g_arena_init, *g_arena_orig;
static uint8_t *g_stack;

typedef struct Rng { uint64_t s; } Rng;
static uint64_t rng_next(Rng *r)
{
    uint64_t z = (r->s += 0x9E3779B97F4A7C15ull);
    z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ull;
    z = (z ^ (z >> 27)) * 0x94D049BB133111EBull;
    return z ^ (z >> 31);
}
static uint32_t rnd(Rng *r, uint32_t n) { return n ? (uint32_t)(rng_next(r) % n) : 0; }

static uint32_t arena_ptr(Rng *r)
{
    uint32_t chunk = rnd(r, ARENA_NCHUNK);
    uint32_t off = rnd(r, ARENA_CHUNK / 2) & ~3u;   /* leave half a chunk after it */
    return (uint32_t)(uintptr_t)g_arena + chunk * ARENA_SPAN + off;
}

static uint32_t mixed_dword(Rng *r)
{
    uint32_t k = rnd(r, 100);
    if (k < 30) return 0;
    if (k < 55) return rnd(r, 65);
    if (k < 65) return rnd(r, 0x10000);
    if (k < 85) return arena_ptr(r);
    return (uint32_t)rng_next(r);
}

static uint32_t gen_int(Rng *r)
{
    static const uint32_t edge[] = {0, 1, 2, 3, 0xffffffffu, 0x7fffffffu, 0x80000000u, 0xff, 0x100,
                                    0xffff, 0x10000, 0xfffffffeu, 0x7f, 0x80, 0x8000, 0x40000000u};
    uint32_t k = rnd(r, 100);
    if (k < 35) return (uint32_t)((int32_t)rnd(r, 45) - 4);
    if (k < 50) return edge[rnd(r, sizeof edge / sizeof *edge)];
    if (k < 65) return rnd(r, 256);
    if (k < 75) return arena_ptr(r);
    if (k < 80) return 1u << rnd(r, 32);
    return (uint32_t)rng_next(r);
}

typedef struct Input {
    uint32_t val[12];
    uint32_t regs_seed;           /* fills unused registers and the stack */
    int arena_mode;               /* 0 dword mix, 1 text, 2 zero, 3 pointer-rich */
    uint64_t arena_seed;
    int glob_zero;
} Input;

static uint32_t narrow(uint32_t v, int size, int gen)
{
    if (gen == GEN_BOOL) return v & 1;
    if (size == 1) return gen == GEN_INT ? (uint32_t)(int32_t)(int8_t)v : (v & 0xff);
    if (size == 2) return gen == GEN_INT ? (uint32_t)(int32_t)(int16_t)v : (v & 0xffff);
    return v;
}

static void make_input(const Spec *s, Rng *r, Input *in)
{
    memset(in, 0, sizeof *in);
    for (int i = 0; i < s->np; i++) {
        const Param *p = &s->p[i];
        uint32_t v;
        switch (p->gen) {
        case GEN_PTR: case GEN_STR: v = rnd(r, 100) < 6 ? 0 : arena_ptr(r); break;
        case GEN_BOOL: v = rnd(r, 2); break;
        case GEN_F32: { float f = (float)((int32_t)rnd(r, 2001) - 1000) / (float)(1 + rnd(r, 64)); memcpy(&v, &f, 4); break; }
        default: v = narrow(gen_int(r), p->size, p->gen);
        }
        in->val[i] = v;
    }
    in->regs_seed = (uint32_t)rng_next(r);
    uint32_t m = rnd(r, 100);
    in->arena_mode = m < 40 ? 0 : m < 70 ? 3 : m < 90 ? 1 : 2;
    in->arena_seed = rng_next(r);
}

static void fill_arena_init(const Input *in)
{
    Rng r = { in->arena_seed };
    for (int c = 0; c < ARENA_NCHUNK; c++) {
        uint8_t *p = g_arena_init + c * ARENA_SPAN;
        if (in->arena_mode == 2) { memset(p, 0, ARENA_CHUNK); continue; }
        if (in->arena_mode == 1) {
            for (uint32_t i = 0; i < ARENA_CHUNK; i++) {
                uint32_t k = rnd(&r, 16);
                p[i] = k == 0 ? 0 : (k < 12 ? (uint8_t)('a' + rnd(&r, 26)) : (uint8_t)(' ' + rnd(&r, 95)));
            }
            continue;
        }
        for (uint32_t i = 0; i < ARENA_CHUNK; i += 4) {
            /* mode 3: pointer-rich, so struct chains a function walks mostly resolve */
            uint32_t d = in->arena_mode == 3 && rnd(&r, 100) < 60 ? arena_ptr(&r) : mixed_dword(&r);
            memcpy(p + i, &d, 4);
        }
    }
}

static void load_arena(void)
{
    for (int c = 0; c < ARENA_NCHUNK; c++)
        memcpy(g_arena + c * ARENA_SPAN, g_arena_init + c * ARENA_SPAN, ARENA_CHUNK);
}

static uint8_t g_glob_bytes[48][GLOB_MAX];

static void fill_globals(const Spec *s, const Input *in)
{
    Rng r = { in->arena_seed ^ 0xA5A5A5A55A5A5A5Aull };
    for (int i = 0; i < s->ng; i++) {
        uint32_t n = s->g[i].size > GLOB_MAX ? GLOB_MAX : s->g[i].size;
        for (uint32_t k = 0; k < n; k += 4) {
            uint32_t d = in->glob_zero ? 0 : mixed_dword(&r);
            memcpy(g_glob_bytes[i] + k, &d, n - k < 4 ? n - k : 4);
        }
    }
}

/* ---------------------------------------------------------------- one side */

typedef struct Side {
    CallCtx c;
    uint8_t glob_after[48][GLOB_MAX];
    uint32_t stray_n;               /* image bytes written outside every mapped global */
    uint32_t stray_addr[8];         /* the first few, absolute */
    uint8_t stray_val[8];
} Side;

static int in_globs(const Spec *s, int ours, uint32_t rva_plus_pref)
{
    for (int i = 0; i < s->ng; i++) {
        uint32_t a = ours ? s->g[i].uva : s->g[i].ova;
        uint32_t n = s->g[i].size > GLOB_MAX ? GLOB_MAX : s->g[i].size;
        if (rva_plus_pref >= a && rva_plus_pref < a + n) return 1;
    }
    return 0;
}

static void restore_page(Image *im, uint32_t off, uint32_t n)
{
    memcpy(im->base + off, im->pristine + off, n);
}

/* Bytes of `im` that differ from its pristine copy, outside the globals this
 * side mapped (own image only), are appended to the side's stray list as
 * ABSOLUTE addresses and restored. Both images are scanned after each call: a
 * wild write (NULL plus a huge offset) lands at the same absolute address for both
 * sides, which may be inside either image, and compares equal that way. */
static void note_stray(Side *sd, uint32_t abs, uint8_t val)
{
    if (sd->stray_n < 8) { sd->stray_addr[sd->stray_n] = abs; sd->stray_val[sd->stray_n] = val; }
    sd->stray_n++;
}

static void scan_writes(Image *im, const Spec *s, int own, int ours, Side *sd)
{
    uint32_t b = (uint32_t)(uintptr_t)im->base;
    if (im->watch) {
        static void *pages[65536];
        ULONG_PTR count = 65536; ULONG gran;
        if (GetWriteWatch(0, im->base, im->size, pages, &count, &gran) == 0) {
            for (ULONG_PTR i = 0; i < count; i++) {
                uint32_t off = (uint32_t)((uint8_t *)pages[i] - im->base);
                for (uint32_t k = 0; k < gran; k++)
                    if (im->base[off + k] != im->pristine[off + k]
                        && !(own && in_globs(s, ours, im->pref + off + k)))
                        note_stray(sd, b + off + k, im->base[off + k]);
                restore_page(im, off, gran);
            }
            return;
        }
    }
    for (int w = 0; w < im->nwritable; w++) {
        Range r = im->writable[w];
        for (uint32_t off = r.lo; off < r.hi; off++) {
            if (im->base[off] != im->pristine[off]) {
                if (!(own && in_globs(s, ours, im->pref + off))) note_stray(sd, b + off, im->base[off]);
                im->base[off] = im->pristine[off];
            }
        }
    }
}

static Image g_orig, g_ours;
static int g_trace;     /* DIFFTEST_TRACE=1: print each mapped global before every call */

static void setup_side(const Spec *s, const Input *in, int ours, CallCtx *c)
{
    Image *im = ours ? &g_ours : &g_orig;
    memset(c, 0, sizeof *c);
    uint32_t delta = (uint32_t)(uintptr_t)im->base - im->pref;
    c->fn = (ours ? s->uva : s->ova) + delta;
    uint32_t sent = 0x5EED0000u ^ (in->regs_seed & 0xfff0u);
    c->eax = sent ^ 0xA; c->ecx = sent ^ 0xC; c->edx = sent ^ 0xD;
    c->ebx = sent ^ 0xB; c->esi = sent ^ 0x5; c->edi = sent ^ 0x7; c->ebp_in = sent ^ 0xE;
    uint32_t *regs[] = {&c->eax, &c->ecx, &c->edx, &c->ebx, &c->esi, &c->edi};
    for (int i = 0; i < s->np; i++) {
        int loc = ours ? s->p[i].uloc : s->p[i].oloc;
        uint32_t v = s->p[i].gen == GEN_F64 ? 0 : in->val[i];
        if (loc == LOC_STACK) {
            if (s->p[i].gen == GEN_F64) { c->stack[c->nstack++] = in->val[i]; c->stack[c->nstack++] = 0; }
            else c->stack[c->nstack++] = v;
        } else if (loc < LOC_STACK) *regs[loc] = v;
    }
    /* the stack sits at the same address for both sides and carries the same
     * pattern, so a read below ESP sees the same bytes in both */
    uint32_t pat = 0xC0DE0000u ^ in->regs_seed;
    for (uint32_t i = 0; i < STACK_SIZE; i += 4) { uint32_t d = pat + i * 0x01000193u; memcpy(g_stack + i, &d, 4); }
    c->stack_top = (uint32_t)(uintptr_t)g_stack + STACK_SIZE - 64;
    c->stack_limit = (uint32_t)(uintptr_t)g_stack;
    c->want_fpu = s->rkind == 'f' || s->rkind == 'd';
    for (int i = 0; i < s->ng; i++) {
        uint32_t a = (ours ? s->g[i].uva : s->g[i].ova) + delta;
        uint32_t n = s->g[i].size > GLOB_MAX ? GLOB_MAX : s->g[i].size;
        if (s->g[i].randomize) memcpy((void *)(uintptr_t)a, g_glob_bytes[i], n);
    }
}

static void run_side(const Spec *s, const Input *in, int ours, Side *sd)
{
    Image *im = ours ? &g_ours : &g_orig;
    load_arena();
    setup_side(s, in, ours, &sd->c);
    uint32_t delta0 = (uint32_t)(uintptr_t)im->base - im->pref;
    if (g_trace)
        for (int i = 0; i < s->ng; i++) {
            uint32_t a = (ours ? s->g[i].uva : s->g[i].ova) + delta0;
            printf("trace %s pre  %-28s %08x: %08x %08x\n", ours ? "ours" : "orig", s->g[i].name, a,
                   ((uint32_t *)(uintptr_t)a)[0], s->g[i].size > 4 ? ((uint32_t *)(uintptr_t)a)[1] : 0);
        }
    if (g_orig.watch) ResetWriteWatch(g_orig.base, g_orig.size);
    if (g_ours.watch) ResetWriteWatch(g_ours.base, g_ours.size);
    run_call(&sd->c);
    uint32_t delta = (uint32_t)(uintptr_t)im->base - im->pref;
    for (int i = 0; i < s->ng; i++) {
        uint32_t a = (ours ? s->g[i].uva : s->g[i].ova) + delta;
        uint32_t n = s->g[i].size > GLOB_MAX ? GLOB_MAX : s->g[i].size;
        memcpy(sd->glob_after[i], (void *)(uintptr_t)a, n);
    }
    sd->stray_n = 0;
    scan_writes(&g_orig, s, !ours, ours, sd);
    scan_writes(&g_ours, s, ours, ours, sd);
    for (int i = 0; i < s->ng; i++) {
        uint32_t a = (ours ? s->g[i].uva : s->g[i].ova);
        uint32_t n = s->g[i].size > GLOB_MAX ? GLOB_MAX : s->g[i].size;
        restore_page(im, a - im->pref, n);
    }
}

/* ---------------------------------------------------------------- compare */

enum { OUT_EQ, OUT_EQ_FAULT, OUT_EQ_TIMEOUT, OUT_DIFF, OUT_FAULT_ORIG, OUT_FAULT_OURS, OUT_TIMEOUT_ONE, OUT_N };
static const char *out_name[] = {"equal", "equal_fault", "equal_timeout", "diff", "fault_orig_only",
                                 "fault_ours_only", "timeout_one_side"};

typedef struct Verdict {
    int outcome;
    char what[512];
} Verdict;

static const char *image_of(uint32_t a)
{
    uint32_t ob = (uint32_t)(uintptr_t)g_orig.base, ub = (uint32_t)(uintptr_t)g_ours.base;
    return a >= ob && a < ob + g_orig.size ? "Game.exe" : a >= ub && a < ub + g_ours.size ? "ours" : "?";
}

static uint32_t rva_of(uint32_t a)
{
    uint32_t ob = (uint32_t)(uintptr_t)g_orig.base, ub = (uint32_t)(uintptr_t)g_ours.base;
    return a >= ob && a < ob + g_orig.size ? a - ob : a >= ub && a < ub + g_ours.size ? a - ub : a;
}

static uint32_t mask_ret(const Spec *s, uint32_t v)
{
    if (s->rkind == 'b') return v & 0xff ? 1 : 0;
    if (s->rsize == 1) return v & 0xff;
    if (s->rsize == 2) return v & 0xffff;
    return v;
}

/* An address into the original image that names a mapped global compares equal
 * to the address of the same global in ours. */
static uint32_t translate_ptr(const Spec *s, uint32_t v)
{
    uint32_t od = (uint32_t)(uintptr_t)g_orig.base - g_orig.pref;
    uint32_t ud = (uint32_t)(uintptr_t)g_ours.base - g_ours.pref;
    for (int i = 0; i < s->ng; i++) {
        uint32_t a = s->g[i].ova + od;
        if (v >= a && v <= a + s->g[i].size) return v - a + s->g[i].uva + ud;
    }
    return v;
}

static void compare(const Spec *s, Side *o, Side *u, Verdict *v)
{
    v->what[0] = 0;
    int of = o->c.status, uf = u->c.status;
    if (of == 2 || uf == 2) {
        /* a hang against a fault is not a disagreement: a runaway loop ends in a
         * guard page on the faster side and in the watchdog on the slower one */
        v->outcome = (of == 2 && uf == 2) || of == 1 || uf == 1 ? OUT_EQ_TIMEOUT : OUT_TIMEOUT_ONE;
        snprintf(v->what, sizeof v->what, "timeout orig=%d ours=%d", of == 2, uf == 2);
        return;
    }
    if (of == 1 || uf == 1) {
        if (of == 1 && uf == 1 && o->c.fault_code == u->c.fault_code) { v->outcome = OUT_EQ_FAULT; }
        else v->outcome = of == 1 ? (uf == 1 ? OUT_DIFF : OUT_FAULT_ORIG) : OUT_FAULT_OURS;
        snprintf(v->what, sizeof v->what, "fault orig=%08x@%08x(%08x) ours=%08x@%08x(%08x)",
                 of == 1 ? o->c.fault_code : 0, o->c.fault_eip, o->c.fault_addr,
                 uf == 1 ? u->c.fault_code : 0, u->c.fault_eip, u->c.fault_addr);
        return;
    }
    char *w = v->what; size_t left = sizeof v->what;
#define ADD(...) do { int k_ = snprintf(w, left, __VA_ARGS__); if (k_ > 0 && (size_t)k_ < left) { w += k_; left -= k_; } } while (0)
    int diff = 0;
    if (s->rkind == 'f' || s->rkind == 'd') {
        if (memcmp(&o->c.st0, &u->c.st0, 8)) { diff = 1; ADD("ret st0 %.17g vs %.17g; ", o->c.st0, u->c.st0); }
    } else if (s->rkind == 'q') {
        if (o->c.r_eax != u->c.r_eax || o->c.r_edx != u->c.r_edx) {
            diff = 1; ADD("ret %08x%08x vs %08x%08x; ", o->c.r_edx, o->c.r_eax, u->c.r_edx, u->c.r_eax);
        }
    } else if (s->rkind != 'v') {
        uint32_t a = mask_ret(s, o->c.r_eax), b = mask_ret(s, u->c.r_eax);
        if (s->rkind == 'p') a = translate_ptr(s, a);
        if (a != b) { diff = 1; ADD("ret %08x vs %08x; ", a, b); }
    }
    /* arena */
    {
        int nd = 0; uint32_t first = 0;
        for (int c = 0; c < ARENA_NCHUNK; c++) {
            uint8_t *a = g_arena_orig + c * ARENA_SPAN, *b = g_arena + c * ARENA_SPAN;
            if (!memcmp(a, b, ARENA_CHUNK)) continue;
            for (uint32_t i = 0; i < ARENA_CHUNK; i += 4) {
                uint32_t x, y; memcpy(&x, a + i, 4); memcpy(&y, b + i, 4);
                if (x == y || translate_ptr(s, x) == y) continue;
                if (!nd) first = c * ARENA_SPAN + i;
                nd++;
            }
        }
        if (nd) {
            uint32_t x, y; memcpy(&x, g_arena_orig + first, 4); memcpy(&y, g_arena + first, 4);
            diff = 1; ADD("arena %d dwords differ, first +%x: %08x vs %08x (init %08x); ", nd, first, x, y,
                          *(uint32_t *)(g_arena_init + first));
        }
    }
    for (int i = 0; i < s->ng; i++) {
        uint32_t n = s->g[i].size > GLOB_MAX ? GLOB_MAX : s->g[i].size;
        if (!memcmp(o->glob_after[i], u->glob_after[i], n)) continue;
        uint32_t k = 0; while (k < n && o->glob_after[i][k] == u->glob_after[i][k]) k++;
        uint32_t k4 = k & ~3u, x = 0, y = 0;
        memcpy(&x, o->glob_after[i] + k4, n - k4 < 4 ? n - k4 : 4);
        memcpy(&y, u->glob_after[i] + k4, n - k4 < 4 ? n - k4 : 4);
        if (translate_ptr(s, x) == y) continue;
        diff = 1; ADD("global %s +%x: %08x vs %08x; ", s->g[i].name, k4, x, y);
    }
    {
        int same = o->stray_n == u->stray_n;
        for (uint32_t k = 0; same && k < o->stray_n && k < 8; k++)
            same = o->stray_addr[k] == u->stray_addr[k] && o->stray_val[k] == u->stray_val[k];
        if (!same) {
            diff = 1;
            ADD("stray image writes orig=%u (first %s+%x) ours=%u (first %s+%x); ",
                o->stray_n, o->stray_n ? image_of(o->stray_addr[0]) : "-", o->stray_n ? rva_of(o->stray_addr[0]) : 0,
                u->stray_n, u->stray_n ? image_of(u->stray_addr[0]) : "-", u->stray_n ? rva_of(u->stray_addr[0]) : 0);
        }
    }
    /* callee-saved registers: only a difference where the original kept one */
    const uint32_t oin[] = {o->c.ebx, o->c.esi, o->c.edi, o->c.ebp_in};
    const uint32_t oout[] = {o->c.r_ebx, o->c.r_esi, o->c.r_edi, o->c.r_ebp};
    const uint32_t uin[] = {u->c.ebx, u->c.esi, u->c.edi, u->c.ebp_in};
    const uint32_t uout[] = {u->c.r_ebx, u->c.r_esi, u->c.r_edi, u->c.r_ebp};
    static const char *rn[] = {"ebx", "esi", "edi", "ebp"};
    for (int i = 0; i < 4; i++)
        if (oin[i] == oout[i] && uin[i] != uout[i]) { diff = 1; ADD("ours clobbers %s; ", rn[i]); }
    if (s->ocleanup >= 0 && o->c.esp_delta != s->ocleanup) ADD("orig popped %d not %d; ", o->c.esp_delta, s->ocleanup);
    if (s->ucleanup >= 0 && u->c.esp_delta != s->ucleanup) ADD("ours popped %d not %d; ", u->c.esp_delta, s->ucleanup);
#undef ADD
    v->outcome = diff ? OUT_DIFF : OUT_EQ;
}

static Side g_so, g_su;

static void run_pair(const Spec *s, const Input *in, Verdict *v)
{
    fill_arena_init(in);
    fill_globals(s, in);
    run_side(s, in, 0, &g_so);
    for (int c = 0; c < ARENA_NCHUNK; c++)
        memcpy(g_arena_orig + c * ARENA_SPAN, g_arena + c * ARENA_SPAN, ARENA_CHUNK);
    run_side(s, in, 1, &g_su);
    compare(s, &g_so, &g_su, v);
    if (v->outcome == OUT_TIMEOUT_ONE && g_timeout_ms < 3000) {
        /* a long but finite loop: our -O0 code is several times slower than VC8's,
         * so give both sides 30x the time before calling it a hang */
        DWORD keep = g_timeout_ms;
        g_timeout_ms = 3000;
        run_pair(s, in, v);
        g_timeout_ms = keep;
    }
}

static int is_failure(int o) { return o == OUT_DIFF || o == OUT_FAULT_ORIG || o == OUT_FAULT_OURS || o == OUT_TIMEOUT_ONE; }

/* Shrink a failing input: each argument towards 0 and small values, then the
 * arena and the globals to all-zero, keeping each step only while it still fails. */
static void shrink(const Spec *s, Input *in, Verdict *v)
{
    for (int pass = 0; pass < 2; pass++) {
        for (int i = 0; i < s->np; i++) {
            if (s->p[i].gen == GEN_PTR || s->p[i].gen == GEN_STR) continue;
            uint32_t cand[] = {0, 1, 2, (uint32_t)-1, in->val[i] >> 16, in->val[i] >> 8, in->val[i] / 2};
            for (unsigned k = 0; k < sizeof cand / sizeof *cand; k++) {
                if (cand[k] == in->val[i]) continue;
                Input t = *in; t.val[i] = narrow(cand[k], s->p[i].size, s->p[i].gen);
                Verdict tv; run_pair(s, &t, &tv);
                if (is_failure(tv.outcome)) { *in = t; *v = tv; break; }
            }
        }
        if (in->arena_mode != 2) {
            Input t = *in; t.arena_mode = 2; Verdict tv; run_pair(s, &t, &tv);
            if (is_failure(tv.outcome)) { *in = t; *v = tv; }
        }
        if (!in->glob_zero && s->ng) {
            Input t = *in; t.glob_zero = 1; Verdict tv; run_pair(s, &t, &tv);
            if (is_failure(tv.outcome)) { *in = t; *v = tv; }
        }
    }
}

static void json_str(FILE *f, const char *s)
{
    fputc('"', f);
    for (; *s; s++) { if (*s == '"' || *s == '\\') fputc('\\', f); fputc(*s, f); }
    fputc('"', f);
}

static void dump_input(FILE *f, const Spec *s, const Input *in)
{
    fprintf(f, "{\"args\":{");
    for (int i = 0; i < s->np; i++) {
        uint32_t v = in->val[i];
        fprintf(f, "%s\"%s\":", i ? "," : "", s->p[i].name);
        if (v >= ARENA_BASE && v < ARENA_BASE + ARENA_SIZE)
            fprintf(f, "\"arena+0x%x\"", v - ARENA_BASE);
        else fprintf(f, "\"0x%x\"", v);
    }
    fprintf(f, "},\"pointee\":{");
    /* the first 32 bytes each pointer argument points at, as the call saw them */
    fill_arena_init(in);
    int first = 1;
    for (int i = 0; i < s->np; i++) {
        uint32_t v = in->val[i];
        if (v < ARENA_BASE || v + 32 > ARENA_BASE + ARENA_SIZE) continue;
        fprintf(f, "%s\"%s\":\"", first ? "" : ",", s->p[i].name); first = 0;
        for (int k = 0; k < 32; k += 4) fprintf(f, "%08x ", *(uint32_t *)(g_arena_init + (v - ARENA_BASE) + k));
        fprintf(f, "\"");
    }
    fprintf(f, "},\"arena\":\"%s\",\"arena_seed\":\"0x%llx\",\"globals\":\"%s\",\"regs_seed\":\"0x%x\"}",
            in->arena_mode == 0 ? "mixed" : in->arena_mode == 1 ? "text" : in->arena_mode == 3 ? "pointers" : "zero",
            (unsigned long long)in->arena_seed, in->glob_zero ? "zero" : "random", in->regs_seed);
}

int main(int argc, char **argv)
{
    if (argc < 5) {
        fprintf(stderr, "usage: difftest <Game.exe> <ours.exe> <spec.txt> <out.jsonl> [iters] [seed] [only-id]\n");
        return 2;
    }
    int iters = argc > 5 ? atoi(argv[5]) : 1000;
    uint64_t seed = argc > 6 ? strtoull(argv[6], 0, 0) : 1;
    int only = argc > 7 ? atoi(argv[7]) : -1;
    setvbuf(stdout, 0, _IONBF, 0);
    g_trace = getenv("DIFFTEST_TRACE") != 0;

    map_image(&g_orig, argv[1], 1, 0);
    map_image(&g_ours, argv[2], 0, 1);
    g_code_lo[0] = (uint32_t)(uintptr_t)g_orig.base; g_code_hi[0] = g_code_lo[0] + g_orig.size;
    g_code_lo[1] = (uint32_t)(uintptr_t)g_ours.base; g_code_hi[1] = g_code_lo[1] + g_ours.size;
    printf("orig %s at %p size %x watch=%d\n", argv[1], g_orig.base, g_orig.size, g_orig.watch);
    printf("ours %s at %p size %x watch=%d\n", argv[2], g_ours.base, g_ours.size, g_ours.watch);

    /* the arena: chunks, each followed by a no-access guard page */
    g_arena = VirtualAlloc((void *)(uintptr_t)ARENA_BASE, ARENA_SIZE, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    if (!g_arena || (uint32_t)(uintptr_t)g_arena != ARENA_BASE) die("arena not at its address%s", "");
    for (int c = 0; c < ARENA_NCHUNK; c++) {
        DWORD old; VirtualProtect(g_arena + c * ARENA_SPAN + ARENA_CHUNK, 0x1000, PAGE_NOACCESS, &old);
    }
    g_arena_init = malloc(ARENA_SIZE);
    g_arena_orig = malloc(ARENA_SIZE);
    g_stack = VirtualAlloc(0, STACK_SIZE + 0x1000, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    { DWORD old; VirtualProtect(g_stack + STACK_SIZE, 0x1000, PAGE_NOACCESS, &old); }

    DuplicateHandle(GetCurrentProcess(), GetCurrentThread(), GetCurrentProcess(), &g_main_thread, 0, FALSE, DUPLICATE_SAME_ACCESS);
    AddVectoredExceptionHandler(1, on_exception);
    CreateThread(0, 0, watchdog, 0, 0, 0);

    Spec *specs; int n = load_specs(argv[3], &specs);
    g_out_path = argv[4];
    FILE *out = fopen(argv[4], "a");
    if (!out) die("cannot write %s", argv[4]);
    for (int si = 0; si < n; si++) {
        const Spec *s = &specs[si];
        if (only >= 0 && s->id != only) continue;
        snprintf(g_cur_name, sizeof g_cur_name, "%s", s->name);
        g_cur_id = s->id;
        int count[OUT_N] = {0};
        int nex = 0;
        char abi[256] = "";
        Input fail_in = {0}; Verdict fail_v = {0}; int have_fail = 0;
        Verdict first_fault; int have_fault = 0;
        for (int it = 0; it < iters; it++) {
            Rng r = { seed * 0x100000001B3ull ^ ((uint64_t)s->id << 32) ^ (uint64_t)it };
            Input in; make_input(s, &r, &in);
            Verdict v; run_pair(s, &in, &v);
            count[v.outcome]++;
            if (strstr(v.what, "popped") && !abi[0]) snprintf(abi, sizeof abi, "%s", strstr(v.what, "orig popped") ? strstr(v.what, "orig popped") : strstr(v.what, "ours popped"));
            if (is_failure(v.outcome) && !have_fail) { fail_in = in; fail_v = v; have_fail = 1; }
            if (v.outcome == OUT_EQ_FAULT && !have_fault) { first_fault = v; have_fault = 1; }
            (void)nex;
        }
        Input orig_fail = fail_in; Verdict orig_v = fail_v;
        if (have_fail) shrink(s, &fail_in, &fail_v);
        fprintf(out, "{\"id\":%d,\"name\":", s->id); json_str(out, s->name);
        fprintf(out, ",\"orig\":\"%08x\",\"ours\":\"%08x\",\"iters\":%d", s->ova, s->uva, iters);
        for (int k = 0; k < OUT_N; k++) fprintf(out, ",\"%s\":%d", out_name[k], count[k]);
        if (abi[0]) { fprintf(out, ",\"abi\":"); json_str(out, abi); }
        if (have_fault) { fprintf(out, ",\"sample_fault\":"); json_str(out, first_fault.what); }
        if (have_fail) {
            fprintf(out, ",\"first_failure\":{\"input\":"); dump_input(out, s, &orig_fail);
            fprintf(out, ",\"what\":"); json_str(out, orig_v.what);
            fprintf(out, "},\"minimal\":{\"input\":"); dump_input(out, s, &fail_in);
            fprintf(out, ",\"what\":"); json_str(out, fail_v.what); fprintf(out, "}");
        }
        fprintf(out, "}\n");
        fflush(out);
        printf("%3d %-60s eq=%d eqf=%d diff=%d fo=%d fu=%d to=%d\n", s->id, s->name, count[OUT_EQ],
               count[OUT_EQ_FAULT], count[OUT_DIFF], count[OUT_FAULT_ORIG], count[OUT_FAULT_OURS],
               count[OUT_TIMEOUT_ONE] + count[OUT_EQ_TIMEOUT]);
    }
    g_cur_id = -1;
    fclose(out);
    return 0;
}
