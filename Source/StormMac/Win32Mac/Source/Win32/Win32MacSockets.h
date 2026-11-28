// Storm's Win32-on-Mac layer: the Winsock surface over the Mac's BSD sockets.
//
// 1.14d Mac calls BSD's socket API directly (select, ioctl FIONBIO, close, inet_ntoa) and
// has none of Winsock's types; the layer's own Winsock code is WSOCK_WSAStartup and
// WSOCK_CloseSocket (Win32_Process.cpp). The shared code (Fog's Safesock and QServer,
// D2Net, Bnclient) is written against Winsock's in_addr (the S_un union), fd_set
// (fd_count / fd_array) and the entry points of Winsock 1.1. This header gives those
// names, with the same layouts the Windows build has, on top of the system types:
//   - in_addr and sockaddr_in keep BSD's size and offsets (sin_len, sin_family, sin_port,
//     sin_addr), so a pointer to them is a valid `struct sockaddr *` for connect/bind/accept;
//   - fd_set is Winsock's counted array, and select() converts it to and from BSD's bitmap.
// Not part of Win32Mac.h: the layer's own units use BSD's in_addr and fd_set. The shared
// code takes it through d2_platform.h.
#pragma once

#include "Win32Mac.h"

#if !defined(_WIN32) && D2_MAC_CARBON

#include <stdint.h>
#include <limits.h>
// The three names are renamed while the system headers are read.
#define in_addr bsd_in_addr
#define sockaddr_in bsd_sockaddr_in
#define fd_set bsd_fd_set
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>
#undef in_addr
#undef sockaddr_in
#undef fd_set
// netdb.h's getaddrinfo flag; the shared code has an AI enum of its own.
#undef AI_DEFAULT

struct _struct_1227 {
    unsigned char s_b1; unsigned char s_b2; unsigned char s_b3; unsigned char s_b4;
    operator uint32_t() const { return *reinterpret_cast<const uint32_t*>(this); }
};
struct _struct_1228 {
    unsigned short s_w1; unsigned short s_w2;
    operator uint32_t() const { return *reinterpret_cast<const uint32_t*>(this); }
};
union _union_1226 {
    _struct_1227 S_un_b;
    _struct_1228 S_un_w;
    uint32_t S_addr;
    _union_1226() {}
    _union_1226(uint32_t a) : S_addr(a) {}
    _union_1226(_struct_1227 b) : S_un_b(b) {}
    _union_1226(_struct_1228 w) : S_un_w(w) {}
    operator uint32_t() const { return S_addr; }
};
struct in_addr {
    _union_1226 S_un;
    in_addr() {}
    in_addr(_union_1226 u) : S_un(u) {}
    in_addr(uint32_t a) : S_un(a) {}
    in_addr(_struct_1227 b) : S_un(b) {}
    in_addr(_struct_1228 w) : S_un(w) {}
    operator uint32_t() const { return S_un.S_addr; }
};
typedef struct in_addr IN_ADDR;
#define s_addr S_un.S_addr

struct sockaddr_in {
    uint8_t sin_len;
    uint8_t sin_family;
    uint16_t sin_port;
    struct in_addr sin_addr;
    char sin_zero[8];
};
typedef struct sockaddr SOCKADDR;
typedef struct sockaddr_in SOCKADDR_IN;
typedef struct timeval TIMEVAL;
typedef struct hostent HOSTENT;

// Winsock's counted descriptor array.
struct fd_set {
    unsigned int fd_count;
    int fd_array[64];
};

static inline int __WSAFDIsSet(int s, fd_set* pSet) {
    for (unsigned int i = 0; i < pSet->fd_count; i++) {
        if (pSet->fd_array[i] == s) return 1;
    }
    return 0;
}

// select() over Winsock's sets: BSD's bitmap is built from each, and the sets are
// rewritten to the descriptors that came back ready.
static inline int select(int, fd_set* pRead, fd_set* pWrite, fd_set* pExcept, struct timeval* pTimeout) {
    bsd_fd_set aBits[3];
    fd_set* apSets[3] = {pRead, pWrite, pExcept};
    int nMax = -1;
    for (int k = 0; k < 3; k++) {
        FD_ZERO(&aBits[k]);
        if (!apSets[k]) continue;
        for (unsigned int i = 0; i < apSets[k]->fd_count; i++) {
            FD_SET(apSets[k]->fd_array[i], &aBits[k]);
            if (apSets[k]->fd_array[i] > nMax) nMax = apSets[k]->fd_array[i];
        }
    }
    int nReady = ::select(nMax + 1, pRead ? &aBits[0] : 0, pWrite ? &aBits[1] : 0, pExcept ? &aBits[2] : 0, pTimeout);
    if (nReady < 0) return nReady;
    for (int k = 0; k < 3; k++) {
        if (!apSets[k]) continue;
        unsigned int nKept = 0;
        for (unsigned int i = 0; i < apSets[k]->fd_count; i++) {
            if (FD_ISSET(apSets[k]->fd_array[i], &aBits[k])) apSets[k]->fd_array[nKept++] = apSets[k]->fd_array[i];
        }
        apSets[k]->fd_count = nKept;
    }
    return nReady;
}

static inline char* inet_ntoa(in_addr in) {
    bsd_in_addr a;
    *reinterpret_cast<uint32_t*>(&a) = in.S_un.S_addr;
    return ::inet_ntoa(a);
}

// ioctlsocket(FIONBIO) is ioctl(fd, FIONBIO (0x8004667e), &arg) in the Mac binary.
static inline int ioctlsocket(int s, long nCmd, unsigned long* pArg) {
    int nArg = (int)*pArg;
    return ::ioctl(s, (unsigned long)nCmd, &nArg);
}
// Winsock's name arguments count in int, BSD's in socklen_t.
static inline int getsockname(int s, struct sockaddr* pName, int* pnLen) {
    socklen_t nLen = (socklen_t)*pnLen;
    int nResult = ::getsockname(s, pName, &nLen);
    *pnLen = (int)nLen;
    return nResult;
}
static inline int getpeername(int s, struct sockaddr* pName, int* pnLen) {
    socklen_t nLen = (socklen_t)*pnLen;
    int nResult = ::getpeername(s, pName, &nLen);
    *pnLen = (int)nLen;
    return nResult;
}

// Winsock's start-up and close are the Win32 layer's WSOCK_ functions (Win32_Process.cpp).
int WSOCK_WSAStartup(uint16_t wVersionRequested, void* pWsaData);
int WSOCK_CloseSocket(int nSocket);
int WSOCK_WSACleanup(void);
static inline int WSAStartup(uint16_t wVersionRequested, void* pWsaData) { return WSOCK_WSAStartup(wVersionRequested, pWsaData); }
static inline int closesocket(int s) { return WSOCK_CloseSocket(s); }
static inline int WSACleanup() { return WSOCK_WSACleanup(); }
// The Mac's socket error is errno (the binary reads ___error()).
static inline int WSAGetLastError() { return errno; }

// The Winsock calls the shared code reaches through _Wrappers are BSD's, called directly.
namespace _Wrappers {
static inline int accept(int s, struct sockaddr* pAddr, int* pnLen) {
    socklen_t nLen = (socklen_t)*pnLen;
    int nResult = ::accept(s, pAddr, &nLen);
    *pnLen = (int)nLen;
    return nResult;
}
static inline int bind(int s, const struct sockaddr* pAddr, int nLen) { return ::bind(s, pAddr, (socklen_t)nLen); }
static inline int listen(int s, int nBacklog) { return ::listen(s, nBacklog); }
static inline void WSASetLastError(int nError) { errno = nError; }
}  // namespace _Wrappers

#endif  // D2_OS_MAC
