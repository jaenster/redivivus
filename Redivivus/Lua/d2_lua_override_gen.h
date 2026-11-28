// What the generated override tables (Redivivus/Lua/generated/override/, written by
// Redivivus/Lua/tools/src/overrides.ts from the tree's own functions) are made of: one entry per
// function of the compiled tree, built by templates from the function's own type, so the thunk
// that takes the native calling convention, the call of the original from Lua and the kinds of
// its parameters all come from the compiler, never from a hand-written signature.
//
// A table is included at the end of the unit that defines its functions (the generated wrapper
// "#include"s the Source unit first), so static functions are reachable too.
//
// No Lua header is included here: the units that include this are game units. Everything that
// touches Lua is in d2_lua_override.cpp, behind the D2LuaOv_* functions below.
#pragma once

#if D2_FEATURE_MODS
#include <stddef.h>
#include <stdint.h>

#include <limits>
#include <tuple>
#include <type_traits>
#include <utility>

struct lua_State;
struct D2UnitStrc;

// Why a function cannot be overridden (D2LuaOvEntry::nRefused); 0 for none.
enum D2LuaOvRefusal {
    D2LUA_OV_OK = 0,
    D2LUA_OV_VARIADIC = 1,       // takes "...": a thunk cannot pass the arguments on
    D2LUA_OV_BY_VALUE = 2,       // a parameter or the result is a record by value
    D2LUA_OV_INLINED = 3,        // always_inline: its call sites hold copies of the body
    D2LUA_OV_HEADER_STATIC = 4,  // static in a header: every unit has its own copy
    D2LUA_OV_MEMBER = 5,         // a member function (needs an object)
    D2LUA_OV_NO_ENTRY = 6,       // its entry has no patch area (found at run time)
    D2LUA_OV_TOO_MANY = 7,       // more parameters than a frame holds
    D2LUA_OV_ABSENT = 8,         // compiled out on this target (an #if around it): no entry at all
};

enum { D2LUA_OV_CDECL = 0, D2LUA_OV_STDCALL = 1, D2LUA_OV_FASTCALL = 2, D2LUA_OV_THISCALL = 3 };
enum { D2LUA_OV_MAX_ARGS = 24 };

// One overridable function. Built by D2LUA_OV / D2LUA_OV_REFUSED.
struct D2LuaOvEntry {
    const char* pszPath;    // the engine path: "D2Common.Skills.GetSkillLevel"
    void* pEntry;           // the function
    void* pThunk;           // the thunk its entry jumps to when overridden (null when refused)
    int (*pfnCall)(lua_State* L);  // the original, called from Lua (the last next())
    const char* pszKinds;   // result then parameters: v void, n number, b boolean, w unit, p pointer, r reference
    const char* pszPointees;  // the types the pointers (w p r, in order) point to, "|"-separated (the raw tier)
    uint8_t nConv;
    uint8_t nSide;          // D2LUA_SIDE_SERVER | D2LUA_SIDE_CLIENT
    uint8_t nRefused;       // D2LuaOvRefusal
    int nId;
};

// One generated table (a Source unit's functions), listed in the generated index.
struct D2LuaOvTable {
    const D2LuaOvEntry* pEntries;
    int nCount;
};

// The frame of one overridden call, on the thunk's stack.
struct D2LuaOvFrame {
    struct D2LuaOvState* pState;
    lua_State* L;
    D2LuaOvFrame* pPrev;
    int nId;
    int nBase;   // stack index below the pushed arguments
    int nArgs;
    int nDepth;  // the host's owner depth on entry
    // The pointers the call was given and the ones next() returned, with their C type: the only
    // pointers a mod of the typed tier may hand back.
    uintptr_t aArg[D2LUA_OV_MAX_ARGS];
    const void* aArgType[D2LUA_OV_MAX_ARGS];
    uintptr_t aSeen[8];
    const void* aSeenType[8];
    int nSeen;
    bool bTrusted;  // the layer reading or returning pointers now is of the raw tier: taken as they are
};

// --- the native side of a thunk (d2_lua_override.cpp) ----------------------------------------------
// Whether this thread's state for the function's side has an override of it; fills the frame.
bool D2LuaOv_Begin(D2LuaOvFrame* pFrame, int nId);
void D2LuaOv_PushNumber(D2LuaOvFrame* pFrame, double nValue);
void D2LuaOv_PushBool(D2LuaOvFrame* pFrame, bool bValue);
void D2LuaOv_PushPointer(D2LuaOvFrame* pFrame, int i, const void* p, const void* pType, bool bUnit);
// Run the chain on the pushed arguments; false when no override ran to the end (every one faulted):
// the caller then runs the original.
bool D2LuaOv_Invoke(D2LuaOvFrame* pFrame);
// The result the chain returned, converted; false (logged, the mod faulted) when it is not of the type.
bool D2LuaOv_ResultNumber(D2LuaOvFrame* pFrame, double* pnOut, double nMin, double nMax, bool bIntegral);
bool D2LuaOv_ResultBool(D2LuaOvFrame* pFrame, bool* pbOut);
bool D2LuaOv_ResultPointer(D2LuaOvFrame* pFrame, void** ppOut, const void* pType, bool bUnit);
void D2LuaOv_End(D2LuaOvFrame* pFrame);

// --- the native side of next() at the bottom of a chain ----------------------------------------------
// The innermost frame of function nId on this state; raises when next() is called outside it.
D2LuaOvFrame* D2LuaOv_CallFrame(lua_State* L, int nId, int nArgs);
double D2LuaOv_ArgNumber(lua_State* L, D2LuaOvFrame* pFrame, int i, double nMin, double nMax, bool bIntegral);
bool D2LuaOv_ArgBool(lua_State* L, D2LuaOvFrame* pFrame, int i);
void* D2LuaOv_ArgPointer(lua_State* L, D2LuaOvFrame* pFrame, int i, const void* pType, bool bUnit);
int D2LuaOv_ReturnNumber(lua_State* L, double nValue);
int D2LuaOv_ReturnBool(lua_State* L, bool bValue);
int D2LuaOv_ReturnPointer(lua_State* L, D2LuaOvFrame* pFrame, const void* p, const void* pType, bool bUnit);
int D2LuaOv_ReturnNone(lua_State* L);

namespace D2LuaOv {

// The bytes a thunk's call of the original skips: the five at the entry that hold the jump to the
// thunk while the function is overridden.
#if defined(__i386__) || defined(__x86_64__)
constexpr int kSkip = 5;
#else
constexpr int kSkip = 0;
#endif

template <class P>
inline P Skip(P p) {
    return reinterpret_cast<P>(reinterpret_cast<uintptr_t>(p) + kSkip);
}

// A unique address per C type: what a pointer is checked against when a mod hands it back.
template <class T>
struct TypeTag {
    static const char c;
};
template <class T>
const char TypeTag<T>::c = 0;

template <class T>
constexpr char KindOf() {
    if constexpr (std::is_reference<T>::value) {
        return 'r';
    } else {
        using U = typename std::remove_cv<T>::type;
        if constexpr (std::is_void<U>::value) {
            return 'v';
        } else if constexpr (std::is_same<U, bool>::value) {
            return 'b';
        } else if constexpr (std::is_integral<U>::value || std::is_enum<U>::value || std::is_floating_point<U>::value) {
            return 'n';
        } else if constexpr (std::is_pointer<U>::value) {
            using P = typename std::remove_cv<typename std::remove_pointer<U>::type>::type;
            return std::is_same<P, D2UnitStrc>::value ? 'w' : 'p';
        } else {
            return 'x';
        }
    }
}

template <class R, class... A>
struct Kinds {
    static constexpr char s[] = { KindOf<R>(), KindOf<A>()..., 0 };
};
template <class R, class... A>
constexpr char Kinds<R, A...>::s[];

constexpr bool StrEq(const char* a, const char* b) {
    return *a == *b && (*a == 0 || StrEq(a + 1, b + 1));
}

constexpr bool AllSupported(const char* s) {
    return *s == 0 || (*s != 'x' && AllSupported(s + 1));
}

// The range a number must lie in to become a T: an integer of T's width, signed or unsigned
// (the engine passes -1 through unsigned parameters and back), or any number for floating point.
template <class T>
struct NumberRange {
    using U = typename std::conditional<std::is_enum<T>::value, std::underlying_type<T>, std::common_type<T>>::type::type;
    static constexpr bool kIntegral = !std::is_floating_point<U>::value;
    static constexpr double Min() {
        return kIntegral ? (sizeof(U) >= 8 ? -9007199254740992.0 : -(double)((uint64_t)1 << (sizeof(U) * 8 - 1))) : -std::numeric_limits<double>::infinity();
    }
    static constexpr double Max() {
        return kIntegral ? (sizeof(U) >= 8 ? 9007199254740992.0 : (double)(((uint64_t)1 << (sizeof(U) * 8)) - 1)) : std::numeric_limits<double>::infinity();
    }
    static T From(double v) {
        if constexpr (kIntegral) {
            if constexpr (sizeof(U) >= 8) {
                return (T)(U)(int64_t)v;
            } else {
                // Two's complement wrap of the value into the width: -1 becomes all ones.
                int64_t n = (int64_t)v;
                return (T)(U)(uint64_t)n;
            }
        } else {
            return (T)v;
        }
    }
};

template <class T>
inline void Push(D2LuaOvFrame* f, int i, T v) {
    constexpr char k = KindOf<T>();
    if constexpr (k == 'n') {
        D2LuaOv_PushNumber(f, (double)v);
    } else if constexpr (k == 'b') {
        D2LuaOv_PushBool(f, v);
    } else if constexpr (k == 'w' || k == 'p') {
        D2LuaOv_PushPointer(f, i, (const void*)v, &TypeTag<typename std::remove_cv<T>::type>::c, k == 'w');
    } else if constexpr (k == 'r') {
        D2LuaOv_PushPointer(f, i, (const void*)&v, &TypeTag<T>::c, false);
    }
}

template <class T>
struct Result {
    static bool Get(D2LuaOvFrame* f, T* pOut) {
        constexpr char k = KindOf<T>();
        if constexpr (k == 'n') {
            double n;
            if (!D2LuaOv_ResultNumber(f, &n, NumberRange<T>::Min(), NumberRange<T>::Max(), NumberRange<T>::kIntegral)) {
                return false;
            }
            *pOut = NumberRange<T>::From(n);
            return true;
        } else if constexpr (k == 'b') {
            return D2LuaOv_ResultBool(f, pOut);
        } else {
            static_assert(k == 'w' || k == 'p', "a result is a number, a boolean or a pointer");
            void* p;
            if (!D2LuaOv_ResultPointer(f, &p, &TypeTag<typename std::remove_cv<T>::type>::c, k == 'w')) {
                return false;
            }
            *pOut = (T)p;
            return true;
        }
    }
};

// Run the chain for one call; *pOut gets the result. False: run the original.
template <class R, class... A>
inline bool Run(D2LuaOvFrame* f, R* pOut, A... a) {
    int i = 0;
    (void)i;
    (Push<A>(f, i++, a), ...);
    if (!D2LuaOv_Invoke(f)) {
        D2LuaOv_End(f);
        return false;
    }
    bool bOk = true;
    if constexpr (!std::is_void<R>::value) {
        bOk = Result<R>::Get(f, pOut);
    }
    D2LuaOv_End(f);
    return bOk;
}

// One argument of next() at the bottom of the chain, read from Lua index i + 1.
template <class T>
struct Arg {
    static T Get(lua_State* L, D2LuaOvFrame* f, int i) {
        constexpr char k = KindOf<T>();
        if constexpr (k == 'n') {
            return NumberRange<T>::From(D2LuaOv_ArgNumber(L, f, i, NumberRange<T>::Min(), NumberRange<T>::Max(), NumberRange<T>::kIntegral));
        } else if constexpr (k == 'b') {
            return D2LuaOv_ArgBool(L, f, i);
        } else if constexpr (k == 'w' || k == 'p') {
            return (T)D2LuaOv_ArgPointer(L, f, i, &TypeTag<typename std::remove_cv<T>::type>::c, k == 'w');
        } else {
            static_assert(k == 'r', "an argument is a number, a boolean, a pointer or a reference");
            using P = typename std::remove_reference<T>::type;
            return *(P*)D2LuaOv_ArgPointer(L, f, i, &TypeTag<T>::c, false);
        }
    }
};

template <class T>
inline int Return(lua_State* L, D2LuaOvFrame* f, T v) {
    constexpr char k = KindOf<T>();
    if constexpr (k == 'n') {
        return D2LuaOv_ReturnNumber(L, (double)v);
    } else if constexpr (k == 'b') {
        return D2LuaOv_ReturnBool(L, v);
    } else {
        static_assert(k == 'w' || k == 'p', "a result is a number, a boolean or a pointer");
        return D2LuaOv_ReturnPointer(L, f, (const void*)v, &TypeTag<typename std::remove_cv<T>::type>::c, k == 'w');
    }
}

// next() at the bottom: read the arguments, run the original past its patch slot, push its result.
// Every argument is read (and checked, which may raise) before the call is made.
template <class P, class R, class... A, size_t... I>
inline int CallOriginal(lua_State* L, int nId, P pfn, std::index_sequence<I...>) {
    D2LuaOvFrame* f = D2LuaOv_CallFrame(L, nId, (int)sizeof...(A));
    std::tuple<A...> t{ Arg<A>::Get(L, f, (int)I)... };
    if constexpr (std::is_void<R>::value) {
        pfn(std::get<I>(t)...);
        return D2LuaOv_ReturnNone(L);
    } else {
        R r = pfn(std::get<I>(t)...);
        return Return<R>(L, f, r);
    }
}

template <class F>
struct Sig;

#define D2LUA_OV_SIG(CC, CONV)                                                                             \
    template <class R, class... A>                                                                         \
    struct Sig<R(CC*)(A...)> {                                                                             \
        typedef R(CC* Fn)(A...);                                                                           \
        static constexpr int kConv = CONV;                                                                 \
        static constexpr const char* Kinds() { return D2LuaOv::Kinds<R, A...>::s; }                       \
        template <R(CC* F)(A...), int ID> /* spelled out: a typedef here would lose the convention */       \
        static R CC Thunk(A... a) {                                                                        \
            D2LuaOvFrame f;                                                                                \
            if (D2LuaOv_Begin(&f, ID)) {                                                                   \
                if constexpr (std::is_void<R>::value) {                                                    \
                    if (Run<R, A...>(&f, (R*)nullptr, a...)) {                                             \
                        return;                                                                            \
                    }                                                                                      \
                } else {                                                                                   \
                    typename std::remove_cv<R>::type r{};                                                  \
                    if (Run<typename std::remove_cv<R>::type, A...>(&f, &r, a...)) {                       \
                        return r;                                                                          \
                    }                                                                                      \
                }                                                                                          \
            }                                                                                              \
            return Skip(F)(a...);                                                                          \
        }                                                                                                  \
        template <R(CC* F)(A...), int ID>                                                                  \
        static int Call(lua_State* L) {                                                                    \
            return CallOriginal<Fn, R, A...>(L, ID, Skip(F), std::index_sequence_for<A...>{});            \
        }                                                                                                  \
    };

#if defined(__i386__)
D2LUA_OV_SIG(__attribute__((cdecl)), D2LUA_OV_CDECL)
D2LUA_OV_SIG(__attribute__((stdcall)), D2LUA_OV_STDCALL)
D2LUA_OV_SIG(__attribute__((fastcall)), D2LUA_OV_FASTCALL)
D2LUA_OV_SIG(__attribute__((thiscall)), D2LUA_OV_THISCALL)
#else
// Every convention is the platform's one: the attributes are ignored on this target.
D2LUA_OV_SIG(, D2LUA_OV_CDECL)
#endif
#undef D2LUA_OV_SIG

// A function's entry, of any type.
template <class F>
inline void* Address(F f) {
    return reinterpret_cast<void*>(f);
}

}  // namespace D2LuaOv

// One entry. FN is the function (&Namespace::Name, or a cast that picks one overload), KINDS the
// kinds the generator read from the declaration: the build fails when the compiler sees others.
#define D2LUA_OV_SIGOF(FN) D2LuaOv::Sig<typename std::remove_const<decltype(FN)>::type>
#define D2LUA_OV(ID, PATH, FN, SIDE, KINDS, POINTEES)                                                                  \
    { PATH, D2LuaOv::Address(FN),                                                                                    \
      D2LuaOv::Address(static_cast<typename D2LUA_OV_SIGOF(FN)::Fn>(&D2LUA_OV_SIGOF(FN)::template Thunk<FN, ID>)),   \
      &D2LUA_OV_SIGOF(FN)::template Call<FN, ID>, KINDS, POINTEES, (uint8_t)D2LUA_OV_SIGOF(FN)::kConv, (uint8_t)(SIDE), 0, \
      ID }
#define D2LUA_OV_CHECK(PATH, FN, KINDS)                                          \
    static_assert(D2LuaOv::StrEq(D2LUA_OV_SIGOF(FN)::Kinds(), KINDS),            \
                  "the override table and the declaration of " PATH " disagree on its kinds")
// A function that is listed but cannot be overridden (the reason is D2LuaOvRefusal).
#define D2LUA_OV_REFUSED(ID, PATH, ENTRY, SIDE, KINDS, WHY) \
    { PATH, ENTRY, nullptr, nullptr, KINDS, nullptr, 0, (uint8_t)(SIDE), (uint8_t)(WHY), ID }
#endif
