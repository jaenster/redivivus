// Win32Posix: events (Win32Mac's Win32_Events.cpp). An event is a waitable object; SetEvent
// wakes every waiter, and a wait that an auto-reset event satisfies resets it (KernelHandles.cpp).
// A named event is not shared between processes: there is only the one game process.
#if !defined(_WIN32) && D2_BACKEND_POSIX

#include "Source/StormMac/Win32Posix/Win32PosixKernelPriv.h"

using namespace Win32Posix;

namespace {

HANDLE CreateEventObject(LPSECURITY_ATTRIBUTES lpEventAttributes, BOOL bManualReset, BOOL bInitialState) {
    (void)lpEventAttributes;
    auto pEvent = std::make_shared<Object>(OBJECT_EVENT);
    pEvent->bWaitable = true;
    pEvent->bManualReset = bManualReset != 0;
    pEvent->bSignaled = bInitialState != 0;
    HANDLE hEvent = Handle_Insert(pEvent);
    if (hEvent) {
        SetError(0);
    }
    return hEvent;
}

BOOL SetEventState(HANDLE hEvent, bool bSignaled) {
    auto pEvent = Handle_GetAs<Object>(hEvent, OBJECT_EVENT);
    if (!pEvent) {
        SetError(6);
        return FALSE;
    }
    Dispatcher_Signal(pEvent.get(), bSignaled);
    return TRUE;
}

}  // namespace

HANDLE CreateEventA(LPSECURITY_ATTRIBUTES lpEventAttributes, BOOL bManualReset, BOOL bInitialState, LPCSTR lpName) {
    (void)lpName;
    return CreateEventObject(lpEventAttributes, bManualReset, bInitialState);
}

HANDLE CreateEventW(LPSECURITY_ATTRIBUTES lpEventAttributes, BOOL bManualReset, BOOL bInitialState, LPCWSTR lpName) {
    (void)lpName;
    return CreateEventObject(lpEventAttributes, bManualReset, bInitialState);
}

BOOL SetEvent(HANDLE hEvent) {
    return SetEventState(hEvent, true);
}

BOOL ResetEvent(HANDLE hEvent) {
    return SetEventState(hEvent, false);
}

#endif  // !defined(_WIN32) && D2_BACKEND_POSIX
