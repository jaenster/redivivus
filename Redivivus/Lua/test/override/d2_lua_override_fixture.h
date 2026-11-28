// The functions the host test overrides, written the way the tree writes its own (namespaces, one
// definition head per line, the calling conventions), so the generator reads them as it reads the
// game: test/generated/d2_lua_override_fixture.ov.cpp is their table. TestShared runs on both sides,
// TestServer on the server's, TestClient on the client's.
#pragma once

#if D2_FEATURE_MODS && D2_LUA_HOST_TEST
#include <stdint.h>

#if !defined(_WIN32)
#if defined(__i386__)
#define __fastcall __attribute__((fastcall))
#define __stdcall __attribute__((stdcall))
#define __thiscall __attribute__((thiscall))
#define __cdecl __attribute__((cdecl))
#else
#define __fastcall
#define __stdcall
#define __thiscall
#define __cdecl
#endif
#endif
#ifndef D2_FORCEINLINE
#define D2_FORCEINLINE inline __attribute__((always_inline))
#endif

struct D2UnitStrc;
struct FixtureThing {
    int nValue;
    int aData[4];
};
struct FixturePoint {
    int x, y;
};

extern int g_nFixtureCount;

namespace TestShared {
int __fastcall Add(int a, int b);
int __stdcall Scale(int a);
double __cdecl Half(double v);
bool __stdcall IsEven(int a);
uint32_t __fastcall Wrap(uint32_t a);
FixtureThing* __stdcall Echo(FixtureThing* p);
FixtureThing* __fastcall Other(FixtureThing* p, FixtureThing* q);
int __thiscall Read(FixtureThing* pThis, int k);
void __fastcall Count(int n);
int __stdcall Recurse(int n);
int __cdecl Sum(int n, ...);
int __fastcall Hot(int a);
int __fastcall Plain(int a);
D2_FORCEINLINE int Inlined(int a) {
    return a * 5;
}
int __stdcall CallsInlined(int a);
int __stdcall Area(FixturePoint p);
int __fastcall Fail(int a);
int __fastcall Slow(int a);
int __fastcall Yields(int a);
int __fastcall Chained(int a);
}  // namespace TestShared

namespace TestServer {
int __fastcall ServerOnly(int a);
D2UnitStrc* __fastcall Pick(D2UnitStrc* a, D2UnitStrc* b, int which);
}  // namespace TestServer

namespace TestClient {
int __fastcall ClientOnly(int a);
}  // namespace TestClient
#endif
