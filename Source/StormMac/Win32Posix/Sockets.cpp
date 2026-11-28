// Win32Posix: Winsock 1.1 (Win32PosixSockets.h) over BSD sockets, as Storm's Mac layer gives it
// (Win32MacSockets.h) but with Winsock's own records and error codes, which the shared code
// takes its Windows path for: it compares WSAGetLastError() with 0x2733 (WSAEWOULDBLOCK) and
// reads the fd_set as a counted array.
//
// This unit holds the Winsock side - the records, WSAStartup, the per-thread hostent and
// inet_ntoa buffers - and SocketsBsd.cpp the system calls.
//
// A SOCKET is the file descriptor, widened to the pointer-sized SOCKET as Windows' socket
// handles are; INVALID_SOCKET is ~0. The shared code keeps sockets only in SOCKET-typed
// fields, so nothing narrows them.
#if !defined(_WIN32) && D2_BACKEND_POSIX

#include "Source/D2Headers/d2_platform.h"
#include "Source/StormMac/Win32Posix/SocketsBsd.h"

#include <stdio.h>
#include <string.h>

namespace {

enum : int {
    WSA_EFAULT = 10014,
    WSA_EINVAL = 10022,
    WSA_ENOTSOCK = 10038,
    WSA_VERNOTSUPPORTED = 10092,
    WSA_NOTINITIALISED = 10093,
};

// The Winsock version this layer reports: Windows Sockets 2.2, which answers 1.1 callers too.
constexpr WORD kHighVersion = 0x0202;
// Winsock's limits for 1.1 callers (the values Windows reports).
constexpr unsigned short kMaxSockets = 0x7fff;
constexpr unsigned short kMaxUdpDatagram = 0xffbb;

int gnStartupCount = 0;

// The descriptor of a SOCKET, or -1 when it cannot be one.
int ToFd(SOCKET s) {
    if (s == INVALID_SOCKET || s > (SOCKET)INT_MAX) {
        return -1;
    }
    return (int)s;
}

SOCKET ToSocket(int nFd) {
    return nFd < 0 ? INVALID_SOCKET : (SOCKET)nFd;
}

int BadSocket() {
    Win32PosixBsd::LastError() = WSA_ENOTSOCK;
    return SOCKET_ERROR;
}

// gethostbyname's result, per thread as Winsock keeps it.
constexpr int kMaxHostAddrs = 16;
struct HostEntryStorage {
    hostent tEntry;
    char szName[256];
    char* apAliases[1];
    uint32_t aAddrs[kMaxHostAddrs];
    char* apAddrs[kMaxHostAddrs + 1];
};
thread_local HostEntryStorage tl_tHost;
thread_local char tl_szNtoa[16];

}  // namespace

int WSAStartup(WORD wVersionRequested, LPWSADATA lpWSAData) {
    BYTE nMajor = (BYTE)(wVersionRequested & 0xff);
    BYTE nMinor = (BYTE)(wVersionRequested >> 8);
    // Windows Sockets 2.2 serves every version from 1.0 up.
    if (nMajor < 1) {
        return WSA_VERNOTSUPPORTED;
    }
    if (!lpWSAData) {
        return WSA_EFAULT;
    }
    memset(lpWSAData, 0, sizeof(*lpWSAData));
    WORD wVersion = (nMajor > 2 || (nMajor == 2 && nMinor >= 2)) ? kHighVersion : wVersionRequested;
    lpWSAData->wVersion = wVersion;
    lpWSAData->wHighVersion = kHighVersion;
    strcpy(lpWSAData->szDescription, "WinSock 2.0");
    strcpy(lpWSAData->szSystemStatus, "Running");
    lpWSAData->iMaxSockets = kMaxSockets;
    lpWSAData->iMaxUdpDg = kMaxUdpDatagram;
    lpWSAData->lpVendorInfo = nullptr;
    __atomic_add_fetch(&gnStartupCount, 1, __ATOMIC_SEQ_CST);
    return 0;
}

int WSACleanup() {
    int nCount = __atomic_load_n(&gnStartupCount, __ATOMIC_SEQ_CST);
    while (nCount > 0) {
        if (__atomic_compare_exchange_n(&gnStartupCount, &nCount, nCount - 1, false, __ATOMIC_SEQ_CST,
                                        __ATOMIC_SEQ_CST)) {
            Win32PosixBsd::LastError() = 0;
            return 0;
        }
    }
    Win32PosixBsd::LastError() = WSA_NOTINITIALISED;
    return SOCKET_ERROR;
}

int WSAGetLastError() {
    return Win32PosixBsd::LastError();
}

namespace _Wrappers {
void WSASetLastError(int iError) {
    Win32PosixBsd::LastError() = iError;
}
}  // namespace _Wrappers

int __WSAFDIsSet(SOCKET fd, fd_set* pSet) {
    if (!pSet) {
        return 0;
    }
    for (u_int i = 0; i < pSet->fd_count && i < FD_SETSIZE; i++) {
        if (pSet->fd_array[i] == fd) {
            return 1;
        }
    }
    return 0;
}

SOCKET socket(int af, int type, int protocol) {
    return ToSocket(Win32PosixBsd::Socket(af, type, protocol));
}

int closesocket(SOCKET s) {
    int nFd = ToFd(s);
    if (nFd < 0) {
        return BadSocket();
    }
    return Win32PosixBsd::Close(nFd);
}

int connect(SOCKET s, const sockaddr* name, int namelen) {
    int nFd = ToFd(s);
    return nFd < 0 ? BadSocket() : Win32PosixBsd::Connect(nFd, name, namelen);
}

namespace _Wrappers {
SOCKET accept(SOCKET s, sockaddr* addr, int* addrlen) {
    int nFd = ToFd(s);
    if (nFd < 0) {
        BadSocket();
        return INVALID_SOCKET;
    }
    return ToSocket(Win32PosixBsd::Accept(nFd, addr, addrlen));
}

int bind(SOCKET s, const sockaddr* name, int namelen) {
    int nFd = ToFd(s);
    return nFd < 0 ? BadSocket() : Win32PosixBsd::Bind(nFd, name, namelen);
}

int listen(SOCKET s, int backlog) {
    int nFd = ToFd(s);
    return nFd < 0 ? BadSocket() : Win32PosixBsd::Listen(nFd, backlog);
}
}  // namespace _Wrappers

int send(SOCKET s, const char* buf, int len, int flags) {
    int nFd = ToFd(s);
    return nFd < 0 ? BadSocket() : Win32PosixBsd::Send(nFd, buf, len, flags);
}

int recv(SOCKET s, char* buf, int len, int flags) {
    int nFd = ToFd(s);
    return nFd < 0 ? BadSocket() : Win32PosixBsd::Recv(nFd, buf, len, flags);
}

int sendto(SOCKET s, const char* buf, int len, int flags, const sockaddr* to, int tolen) {
    int nFd = ToFd(s);
    return nFd < 0 ? BadSocket() : Win32PosixBsd::SendTo(nFd, buf, len, flags, to, tolen);
}

int recvfrom(SOCKET s, char* buf, int len, int flags, sockaddr* from, int* fromlen) {
    int nFd = ToFd(s);
    return nFd < 0 ? BadSocket() : Win32PosixBsd::RecvFrom(nFd, buf, len, flags, from, fromlen);
}

int shutdown(SOCKET s, int how) {
    int nFd = ToFd(s);
    return nFd < 0 ? BadSocket() : Win32PosixBsd::Shutdown(nFd, how);
}

int setsockopt(SOCKET s, int level, int optname, const char* optval, int optlen) {
    int nFd = ToFd(s);
    return nFd < 0 ? BadSocket() : Win32PosixBsd::SetSockOpt(nFd, level, optname, optval, optlen);
}

int getsockopt(SOCKET s, int level, int optname, char* optval, int* optlen) {
    int nFd = ToFd(s);
    return nFd < 0 ? BadSocket() : Win32PosixBsd::GetSockOpt(nFd, level, optname, optval, optlen);
}

int ioctlsocket(SOCKET s, long cmd, u_long* argp) {
    int nFd = ToFd(s);
    // The command is Winsock's 32-bit code; the callers pass FIONBIO as a negative long.
    return nFd < 0 ? BadSocket() : Win32PosixBsd::IoctlSocket(nFd, (uint32_t)cmd, argp);
}

int getsockname(SOCKET s, sockaddr* name, int* namelen) {
    int nFd = ToFd(s);
    return nFd < 0 ? BadSocket() : Win32PosixBsd::GetSockName(nFd, name, namelen);
}

int getpeername(SOCKET s, sockaddr* name, int* namelen) {
    int nFd = ToFd(s);
    return nFd < 0 ? BadSocket() : Win32PosixBsd::GetPeerName(nFd, name, namelen);
}

// Winsock's select: nfds is ignored, the sets are counted arrays rewritten to the sockets that
// are ready, and the result counts each (socket, set) pair.
int select(int nfds, fd_set* readfds, fd_set* writefds, fd_set* exceptfds, const timeval* timeout) {
    (void)nfds;
    fd_set* apSets[3] = {readfds, writefds, exceptfds};
    const uint8_t aBits[3] = {Win32PosixBsd::POLL_READ, Win32PosixBsd::POLL_WRITE, Win32PosixBsd::POLL_EXCEPT};
    Win32PosixBsd::PollEntry aEntries[3 * FD_SETSIZE];
    SOCKET aSockets[3 * FD_SETSIZE];
    int nEntries = 0;
    for (int k = 0; k < 3; k++) {
        if (!apSets[k]) {
            continue;
        }
        if (apSets[k]->fd_count > FD_SETSIZE) {
            Win32PosixBsd::LastError() = WSA_EINVAL;
            return SOCKET_ERROR;
        }
        for (u_int i = 0; i < apSets[k]->fd_count; i++) {
            SOCKET s = apSets[k]->fd_array[i];
            int nFd = ToFd(s);
            if (nFd < 0) {
                return BadSocket();
            }
            int j = 0;
            while (j < nEntries && aSockets[j] != s) {
                j++;
            }
            if (j == nEntries) {
                aSockets[j] = s;
                aEntries[j].nFd = nFd;
                aEntries[j].nWant = 0;
                aEntries[j].nReady = 0;
                nEntries++;
            }
            aEntries[j].nWant |= aBits[k];
        }
    }
    if (nEntries == 0) {
        Win32PosixBsd::LastError() = WSA_EINVAL;
        return SOCKET_ERROR;
    }
    int nTimeoutMs = -1;
    if (timeout) {
        long long nMs = (long long)timeout->tv_sec * 1000 + ((long long)timeout->tv_usec + 999) / 1000;
        nTimeoutMs = nMs < 0 ? 0 : nMs > INT_MAX ? INT_MAX : (int)nMs;
    }
    int nReady = Win32PosixBsd::Select(aEntries, nEntries, nTimeoutMs);
    if (nReady < 0) {
        return SOCKET_ERROR;
    }
    for (int k = 0; k < 3; k++) {
        if (!apSets[k]) {
            continue;
        }
        u_int nKept = 0;
        for (u_int i = 0; i < apSets[k]->fd_count; i++) {
            SOCKET s = apSets[k]->fd_array[i];
            for (int j = 0; j < nEntries; j++) {
                if (aSockets[j] == s) {
                    if (aEntries[j].nReady & aBits[k]) {
                        apSets[k]->fd_array[nKept++] = s;
                    }
                    break;
                }
            }
        }
        apSets[k]->fd_count = nKept;
    }
    return nReady;
}

hostent* gethostbyname(const char* name) {
    HostEntryStorage& tHost = tl_tHost;
    int nCount = Win32PosixBsd::GetHostByName(name, tHost.szName, sizeof(tHost.szName), tHost.aAddrs, kMaxHostAddrs);
    if (nCount < 0) {
        return nullptr;
    }
    tHost.apAliases[0] = nullptr;
    for (int i = 0; i < nCount; i++) {
        tHost.apAddrs[i] = (char*)&tHost.aAddrs[i];
    }
    tHost.apAddrs[nCount] = nullptr;
    tHost.tEntry.h_name = tHost.szName;
    tHost.tEntry.h_aliases = tHost.apAliases;
    tHost.tEntry.h_addrtype = AF_INET;
    tHost.tEntry.h_length = sizeof(uint32_t);
    tHost.tEntry.h_addr_list = tHost.apAddrs;
    return &tHost.tEntry;
}

int gethostname(char* name, int namelen) {
    return Win32PosixBsd::GetHostName(name, namelen) < 0 ? SOCKET_ERROR : 0;
}

u_long inet_addr(const char* cp) {
    return Win32PosixBsd::InetAddr(cp);
}

// The dotted form in a per-thread buffer, as Winsock's.
char* inet_ntoa(in_addr in) {
    uint32_t nAddr = in.S_un.S_addr;
    const uint8_t* pBytes = (const uint8_t*)&nAddr;
    snprintf(tl_szNtoa, sizeof(tl_szNtoa), "%u.%u.%u.%u", pBytes[0], pBytes[1], pBytes[2], pBytes[3]);
    return tl_szNtoa;
}

#endif  // !defined(_WIN32) && D2_BACKEND_POSIX
