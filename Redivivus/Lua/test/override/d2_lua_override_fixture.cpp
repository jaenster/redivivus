// See d2_lua_override_fixture.h. Built through its table (test/generated/d2_lua_override_fixture.ov.cpp),
// which #includes this file; on an x86 target with the patchable entries a mods build gives the game.
#if D2_FEATURE_MODS && D2_LUA_HOST_TEST
#include "Redivivus/Lua/test/override/d2_lua_override_fixture.h"

#include <stdarg.h>

int g_nFixtureCount = 0;

namespace TestShared {

int __fastcall Add(int a, int b) {
    return a + b;
}

int __stdcall Scale(int a) {
    return a * 3;
}

double __cdecl Half(double v) {
    return v / 2;
}

bool __stdcall IsEven(int a) {
    return (a & 1) == 0;
}

uint32_t __fastcall Wrap(uint32_t a) {
    return a;
}

FixtureThing* __stdcall Echo(FixtureThing* p) {
    return p;
}

FixtureThing* __fastcall Other(FixtureThing* p, FixtureThing* q) {
    return q ? q : p;
}

int __thiscall Read(FixtureThing* pThis, int k) {
    return pThis->aData[k & 3];
}

void __fastcall Count(int n) {
    g_nFixtureCount += n;
}

int __stdcall Recurse(int n) {
    return n <= 0 ? 0 : n + Recurse(n - 1);
}

int __cdecl Sum(int n, ...) {
    va_list ap;
    va_start(ap, n);
    int s = 0;
    for (int i = 0; i < n; i++) {
        s += va_arg(ap, int);
    }
    va_end(ap);
    return s;
}

int __fastcall Hot(int a) {
    return a + 1;
}

int __fastcall Plain(int a) {
    return a + 1;
}

int __stdcall CallsInlined(int a) {
    return Inlined(a) + 1;
}

int __stdcall Area(FixturePoint p) {
    return p.x * p.y;
}

int __fastcall Fail(int a) {
    return a;
}

int __fastcall Slow(int a) {
    return a;
}

int __fastcall Yields(int a) {
    return a;
}

int __fastcall Chained(int a) {
    return a;
}

}  // namespace TestShared

namespace TestServer {

int __fastcall ServerOnly(int a) {
    return a;
}

D2UnitStrc* __fastcall Pick(D2UnitStrc* a, D2UnitStrc* b, int which) {
    return which ? b : a;
}

}  // namespace TestServer

namespace TestClient {

int __fastcall ClientOnly(int a) {
    return a;
}

}  // namespace TestClient
#endif
