// Win32Posix: the Microsoft C++ runtime's helpers the shared code calls by name, which no POSIX
// runtime has (Redivivus/d2_crt_impl.cpp holds the same two for the Windows builds that lack
// them). Built with the forced d2_platform.h, which declares them.
#if !defined(_WIN32) && D2_BACKEND_POSIX

// MSVC's array construct and destruct helpers: nCount elements of nElementSize bytes, each
// handed to the constructor or destructor. The CRT's constructor also destroys the elements
// already built when one throws; nothing in the tree throws out of these, so the loop is the
// whole behaviour. Destruction runs back to front, as the CRT's does.
extern "C" void __stdcall _eh_vector_constructor_iterator_(void* pArray, uint32_t nElementSize, int nCount,
                                                          void(__fastcall* pfnConstructor)(void*),
                                                          void(__fastcall* pfnDestructor)(void*)) {
    (void)pfnDestructor;
    if (!pArray || !pfnConstructor) {
        return;
    }
    char* p = (char*)pArray;
    for (int i = 0; i < nCount; ++i) {
        pfnConstructor(p + (size_t)i * nElementSize);
    }
}

extern "C" void __stdcall _eh_vector_destructor_iterator_(void* pArray, uint32_t nElementSize, int nCount,
                                                         void(__fastcall* pfnDestructor)(void*)) {
    if (!pArray || !pfnDestructor) {
        return;
    }
    char* p = (char*)pArray;
    for (int i = nCount - 1; i >= 0; --i) {
        pfnDestructor(p + (size_t)i * nElementSize);
    }
}

#endif  // !defined(_WIN32) && D2_BACKEND_POSIX
