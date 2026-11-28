// Win32Posix: Winsock's calls on BSD sockets (SocketsBsd.h). The one unit that reads the
// system socket headers, so it must be built without the forced d2_platform.h: Winsock's
// records there share the BSD ones' names (sockaddr, in_addr, hostent, fd_set) and Winsock's
// entry points share their signatures (socket, inet_addr, gethostbyname). cmake/mac-modern.cmake
// names this unit among those built without the forced include; it reads no part of the Win32
// surface.
//
// What differs between the two, and what this unit does about it:
//   - the sockaddr: Winsock's starts with a 16-bit family, BSD's with a length byte and an
//     8-bit family; the bytes after the family are the same for AF_INET;
//   - the error: Winsock's WSAE* codes, read per thread, against errno; a non-blocking connect
//     fails with WSAEWOULDBLOCK where BSD says EINPROGRESS;
//   - select: Winsock's counted arrays, any descriptor value, a count of (socket, set) pairs
//     ready, and a failed non-blocking connect reported in the except set (BSD reports it
//     writable, with SO_ERROR set); done here on poll();
//   - the option and ioctl numbers, and Winsock's DWORD millisecond timeouts and 16-bit linger;
//   - a write to a closed connection fails with WSAECONNRESET and never raises SIGPIPE.
#if !defined(_WIN32) && D2_BACKEND_POSIX

#if defined(INVALID_SOCKET)
#error "SocketsBsd.cpp reads the BSD socket headers: build it without the forced d2_platform.h"
#endif

#include "Source/StormMac/Win32Posix/SocketsBsd.h"

#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <poll.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/uio.h>
#include <time.h>
#include <unistd.h>

#include <mutex>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace Win32PosixBsd {

namespace {

// ---- Winsock's numbers (winsock.h) ----
enum : int {
    WSA_EINTR = 10004,
    WSA_EBADF = 10009,
    WSA_EACCES = 10013,
    WSA_EFAULT = 10014,
    WSA_EINVAL = 10022,
    WSA_EMFILE = 10024,
    WSA_EWOULDBLOCK = 10035,
    WSA_EINPROGRESS = 10036,
    WSA_EALREADY = 10037,
    WSA_ENOTSOCK = 10038,
    WSA_EDESTADDRREQ = 10039,
    WSA_EMSGSIZE = 10040,
    WSA_EPROTOTYPE = 10041,
    WSA_ENOPROTOOPT = 10042,
    WSA_EPROTONOSUPPORT = 10043,
    WSA_ESOCKTNOSUPPORT = 10044,
    WSA_EOPNOTSUPP = 10045,
    WSA_EPFNOSUPPORT = 10046,
    WSA_EAFNOSUPPORT = 10047,
    WSA_EADDRINUSE = 10048,
    WSA_EADDRNOTAVAIL = 10049,
    WSA_ENETDOWN = 10050,
    WSA_ENETUNREACH = 10051,
    WSA_ENETRESET = 10052,
    WSA_ECONNABORTED = 10053,
    WSA_ECONNRESET = 10054,
    WSA_ENOBUFS = 10055,
    WSA_EISCONN = 10056,
    WSA_ENOTCONN = 10057,
    WSA_ESHUTDOWN = 10058,
    WSA_ETIMEDOUT = 10060,
    WSA_ECONNREFUSED = 10061,
    WSA_ENAMETOOLONG = 10063,
    WSA_EHOSTDOWN = 10064,
    WSA_EHOSTUNREACH = 10065,
    WSA_HOST_NOT_FOUND = 11001,
    WSA_TRY_AGAIN = 11002,
    WSA_NO_RECOVERY = 11003,
    WSA_NO_DATA = 11004,
};

enum : int {
    WIN_AF_INET = 2,
    WIN_SOCK_STREAM = 1,
    WIN_SOCK_DGRAM = 2,
    WIN_SOCK_RAW = 3,
    WIN_SOL_SOCKET = 0xffff,
    WIN_IPPROTO_IP = 0,
    WIN_IPPROTO_TCP = 6,
    WIN_IPPROTO_UDP = 17,
    WIN_TCP_NODELAY = 1,
    WIN_SO_DEBUG = 0x1,
    WIN_SO_ACCEPTCONN = 0x2,
    WIN_SO_REUSEADDR = 0x4,
    WIN_SO_KEEPALIVE = 0x8,
    WIN_SO_DONTROUTE = 0x10,
    WIN_SO_BROADCAST = 0x20,
    WIN_SO_LINGER = 0x80,
    WIN_SO_OOBINLINE = 0x100,
    WIN_SO_DONTLINGER = (int)(~0x80u),
    WIN_SO_SNDBUF = 0x1001,
    WIN_SO_RCVBUF = 0x1002,
    WIN_SO_SNDLOWAT = 0x1003,
    WIN_SO_RCVLOWAT = 0x1004,
    WIN_SO_SNDTIMEO = 0x1005,
    WIN_SO_RCVTIMEO = 0x1006,
    WIN_SO_ERROR = 0x1007,
    WIN_SO_TYPE = 0x1008,
    WIN_MSG_OOB = 0x1,
    WIN_MSG_PEEK = 0x2,
    WIN_MSG_DONTROUTE = 0x4,
    WIN_SD_RECEIVE = 0,
    WIN_SD_SEND = 1,
    WIN_SD_BOTH = 2,
};

constexpr uint32_t WIN_FIONREAD = 0x4004667Fu;
constexpr uint32_t WIN_FIONBIO = 0x8004667Eu;
constexpr uint32_t WIN_SIOCATMARK = 0x40047307u;
// The Winsock sockaddr_in: a 16-bit family, the port, the address, eight bytes of zero.
constexpr int WIN_SOCKADDR_IN_SIZE = 16;
// Winsock's linger: two u_shorts.
constexpr int WIN_LINGER_SIZE = 4;

thread_local int tl_nLastError = 0;

// Non-blocking connects still in progress, and the errors of those select() saw fail (it has
// read and so cleared their SO_ERROR, which getsockopt(SO_ERROR) then reports once).
std::mutex gConnectLock;
std::unordered_set<int> gConnecting;
std::unordered_map<int, int> gConnectErrors;

int MapErrno(int nErrno) {
    switch (nErrno) {
    case 0: return 0;
    case EINTR: return WSA_EINTR;
    case EBADF: return WSA_ENOTSOCK;
    case EACCES: return WSA_EACCES;
    case EPERM: return WSA_EACCES;
    case EFAULT: return WSA_EFAULT;
    case EINVAL: return WSA_EINVAL;
    case EMFILE: return WSA_EMFILE;
    case ENFILE: return WSA_EMFILE;
#if EAGAIN != EWOULDBLOCK
    case EAGAIN: return WSA_EWOULDBLOCK;
#endif
    case EWOULDBLOCK: return WSA_EWOULDBLOCK;
    case EINPROGRESS: return WSA_EINPROGRESS;
    case EALREADY: return WSA_EALREADY;
    case ENOTSOCK: return WSA_ENOTSOCK;
    case EDESTADDRREQ: return WSA_EDESTADDRREQ;
    case EMSGSIZE: return WSA_EMSGSIZE;
    case EPROTOTYPE: return WSA_EPROTOTYPE;
    case ENOPROTOOPT: return WSA_ENOPROTOOPT;
    case EPROTONOSUPPORT: return WSA_EPROTONOSUPPORT;
    case ESOCKTNOSUPPORT: return WSA_ESOCKTNOSUPPORT;
#if defined(ENOTSUP) && ENOTSUP != EOPNOTSUPP
    case ENOTSUP: return WSA_EOPNOTSUPP;
#endif
    case EOPNOTSUPP: return WSA_EOPNOTSUPP;
    case EPFNOSUPPORT: return WSA_EPFNOSUPPORT;
    case EAFNOSUPPORT: return WSA_EAFNOSUPPORT;
    case EADDRINUSE: return WSA_EADDRINUSE;
    case EADDRNOTAVAIL: return WSA_EADDRNOTAVAIL;
    case ENETDOWN: return WSA_ENETDOWN;
    case ENETUNREACH: return WSA_ENETUNREACH;
    case ENETRESET: return WSA_ENETRESET;
    case ECONNABORTED: return WSA_ECONNABORTED;
    case ECONNRESET: return WSA_ECONNRESET;
    // A write to a connection the peer has closed: Winsock says reset.
    case EPIPE: return WSA_ECONNRESET;
    case ENOBUFS: return WSA_ENOBUFS;
    case ENOMEM: return WSA_ENOBUFS;
    case EISCONN: return WSA_EISCONN;
    case ENOTCONN: return WSA_ENOTCONN;
    case ESHUTDOWN: return WSA_ESHUTDOWN;
    case ETIMEDOUT: return WSA_ETIMEDOUT;
    case ECONNREFUSED: return WSA_ECONNREFUSED;
    case ENAMETOOLONG: return WSA_ENAMETOOLONG;
    case EHOSTDOWN: return WSA_EHOSTDOWN;
    case EHOSTUNREACH: return WSA_EHOSTUNREACH;
    default: return WSA_EINVAL;
    }
}

int Fail(int nWsaError) {
    tl_nLastError = nWsaError;
    return -1;
}

int FailErrno() {
    return Fail(MapErrno(errno));
}

int Succeed(int nResult) {
    tl_nLastError = 0;
    return nResult;
}

int MapFamily(int nWinAf) {
    return nWinAf == WIN_AF_INET ? AF_INET : -1;
}

// A Winsock sockaddr to a BSD one: only AF_INET (Winsock 1.1's only family).
bool ToBsdAddr(const void* pWinAddr, int nLen, sockaddr_in* pOut) {
    if (!pWinAddr || nLen < WIN_SOCKADDR_IN_SIZE) {
        Fail(WSA_EFAULT);
        return false;
    }
    const uint8_t* pBytes = (const uint8_t*)pWinAddr;
    uint16_t nFamily;
    memcpy(&nFamily, pBytes, sizeof(nFamily));
    if (nFamily != WIN_AF_INET) {
        Fail(WSA_EAFNOSUPPORT);
        return false;
    }
    memset(pOut, 0, sizeof(*pOut));
#if defined(__APPLE__) || defined(__FreeBSD__)
    pOut->sin_len = sizeof(*pOut);
#endif
    pOut->sin_family = AF_INET;
    memcpy(&pOut->sin_port, pBytes + 2, sizeof(pOut->sin_port));
    memcpy(&pOut->sin_addr, pBytes + 4, sizeof(pOut->sin_addr));
    return true;
}

// A BSD address back to a Winsock sockaddr_in; *pnLen is the room in and the size out.
bool FromBsdAddr(const sockaddr_storage& tAddr, void* pWinAddr, int* pnLen) {
    if (!pWinAddr) {
        return true;
    }
    if (!pnLen || *pnLen < WIN_SOCKADDR_IN_SIZE) {
        Fail(WSA_EFAULT);
        return false;
    }
    uint8_t aBytes[WIN_SOCKADDR_IN_SIZE] = {};
    if (tAddr.ss_family == AF_INET) {
        const sockaddr_in* pIn = (const sockaddr_in*)&tAddr;
        uint16_t nFamily = WIN_AF_INET;
        memcpy(aBytes, &nFamily, sizeof(nFamily));
        memcpy(aBytes + 2, &pIn->sin_port, sizeof(pIn->sin_port));
        memcpy(aBytes + 4, &pIn->sin_addr, sizeof(pIn->sin_addr));
    }
    memcpy(pWinAddr, aBytes, sizeof(aBytes));
    *pnLen = WIN_SOCKADDR_IN_SIZE;
    return true;
}

int MapMsgFlags(int nWinFlags) {
    int nFlags = 0;
    if (nWinFlags & WIN_MSG_OOB) nFlags |= MSG_OOB;
    if (nWinFlags & WIN_MSG_PEEK) nFlags |= MSG_PEEK;
    if (nWinFlags & WIN_MSG_DONTROUTE) nFlags |= MSG_DONTROUTE;
    return nFlags;
}

// Writes to a closed connection fail with EPIPE instead of killing the process.
int SendFlags(int nWinFlags) {
#if defined(MSG_NOSIGNAL)
    return MapMsgFlags(nWinFlags) | MSG_NOSIGNAL;
#else
    return MapMsgFlags(nWinFlags);
#endif
}

void PrepareSocket(int nFd) {
#if defined(SO_NOSIGPIPE)
    int nOn = 1;
    setsockopt(nFd, SOL_SOCKET, SO_NOSIGPIPE, &nOn, sizeof(nOn));
#endif
    fcntl(nFd, F_SETFD, FD_CLOEXEC);
}

void ForgetConnect(int nFd) {
    std::lock_guard<std::mutex> tLock(gConnectLock);
    gConnecting.erase(nFd);
    gConnectErrors.erase(nFd);
}

bool IsNonBlocking(int nFd) {
    int nFlags = fcntl(nFd, F_GETFL, 0);
    return nFlags != -1 && (nFlags & O_NONBLOCK) != 0;
}

int64_t NowMs() {
    timespec tNow;
    clock_gettime(CLOCK_MONOTONIC, &tNow);
    return (int64_t)tNow.tv_sec * 1000 + tNow.tv_nsec / 1000000;
}

// The BSD level and option of a Winsock one; false when BSD has none.
bool MapOption(int nLevel, int nOptName, int* pnLevel, int* pnOptName) {
    if (nLevel == WIN_SOL_SOCKET) {
        *pnLevel = SOL_SOCKET;
        switch (nOptName) {
        case WIN_SO_DEBUG: *pnOptName = SO_DEBUG; return true;
        case WIN_SO_ACCEPTCONN: *pnOptName = SO_ACCEPTCONN; return true;
        case WIN_SO_REUSEADDR: *pnOptName = SO_REUSEADDR; return true;
        case WIN_SO_KEEPALIVE: *pnOptName = SO_KEEPALIVE; return true;
        case WIN_SO_DONTROUTE: *pnOptName = SO_DONTROUTE; return true;
        case WIN_SO_BROADCAST: *pnOptName = SO_BROADCAST; return true;
        case WIN_SO_LINGER: *pnOptName = SO_LINGER; return true;
        case WIN_SO_DONTLINGER: *pnOptName = SO_LINGER; return true;
        case WIN_SO_OOBINLINE: *pnOptName = SO_OOBINLINE; return true;
        case WIN_SO_SNDBUF: *pnOptName = SO_SNDBUF; return true;
        case WIN_SO_RCVBUF: *pnOptName = SO_RCVBUF; return true;
        case WIN_SO_SNDLOWAT: *pnOptName = SO_SNDLOWAT; return true;
        case WIN_SO_RCVLOWAT: *pnOptName = SO_RCVLOWAT; return true;
        case WIN_SO_SNDTIMEO: *pnOptName = SO_SNDTIMEO; return true;
        case WIN_SO_RCVTIMEO: *pnOptName = SO_RCVTIMEO; return true;
        case WIN_SO_ERROR: *pnOptName = SO_ERROR; return true;
        case WIN_SO_TYPE: *pnOptName = SO_TYPE; return true;
        default: return false;
        }
    }
    if (nLevel == WIN_IPPROTO_TCP && nOptName == WIN_TCP_NODELAY) {
        *pnLevel = IPPROTO_TCP;
        *pnOptName = TCP_NODELAY;
        return true;
    }
    return false;
}

// A Winsock BOOL or int option value: an int, or a single byte when optlen is less.
bool ReadIntOption(const char* pOptVal, int nOptLen, int* pnValue) {
    if (!pOptVal || nOptLen < 1) {
        Fail(WSA_EFAULT);
        return false;
    }
    if (nOptLen >= (int)sizeof(int)) {
        memcpy(pnValue, pOptVal, sizeof(int));
    } else {
        *pnValue = (unsigned char)pOptVal[0];
    }
    return true;
}

}  // namespace

int& LastError() {
    return tl_nLastError;
}

int Socket(int nAf, int nType, int nProtocol) {
    int nFamily = MapFamily(nAf);
    if (nFamily < 0) {
        return Fail(WSA_EAFNOSUPPORT);
    }
    int nSockType;
    switch (nType) {
    case WIN_SOCK_STREAM: nSockType = SOCK_STREAM; break;
    case WIN_SOCK_DGRAM: nSockType = SOCK_DGRAM; break;
    case WIN_SOCK_RAW: nSockType = SOCK_RAW; break;
    default: return Fail(WSA_ESOCKTNOSUPPORT);
    }
    int nProto;
    switch (nProtocol) {
    case WIN_IPPROTO_IP: nProto = 0; break;
    case WIN_IPPROTO_TCP: nProto = IPPROTO_TCP; break;
    case WIN_IPPROTO_UDP: nProto = IPPROTO_UDP; break;
    default: nProto = nProtocol; break;
    }
    int nFd = ::socket(nFamily, nSockType, nProto);
    if (nFd < 0) {
        return FailErrno();
    }
    PrepareSocket(nFd);
    ForgetConnect(nFd);
    return Succeed(nFd);
}

int Close(int nFd) {
    ForgetConnect(nFd);
    if (::close(nFd) != 0 && errno != EINTR) {
        return FailErrno();
    }
    return Succeed(0);
}

int Connect(int nFd, const void* pWinAddr, int nLen) {
    sockaddr_in tAddr;
    if (!ToBsdAddr(pWinAddr, nLen, &tAddr)) {
        return -1;
    }
    {
        std::lock_guard<std::mutex> tLock(gConnectLock);
        gConnectErrors.erase(nFd);
        if (gConnecting.count(nFd)) {
            return Fail(WSA_EALREADY);
        }
    }
    if (::connect(nFd, (const sockaddr*)&tAddr, sizeof(tAddr)) == 0) {
        return Succeed(0);
    }
    int nErrno = errno;
    bool bNonBlocking = IsNonBlocking(nFd);
    if (nErrno == EINPROGRESS || (nErrno == EINTR && bNonBlocking)) {
        // Winsock's non-blocking connect: WSAEWOULDBLOCK, then select() says how it went.
        std::lock_guard<std::mutex> tLock(gConnectLock);
        gConnecting.insert(nFd);
        return Fail(WSA_EWOULDBLOCK);
    }
    if (nErrno == EINTR) {
        // A blocking connect a signal interrupted goes on: wait for it here.
        pollfd tPoll = {nFd, POLLOUT, 0};
        while (::poll(&tPoll, 1, -1) < 0 && errno == EINTR) {
        }
        int nSoError = 0;
        socklen_t nSoLen = sizeof(nSoError);
        getsockopt(nFd, SOL_SOCKET, SO_ERROR, &nSoError, &nSoLen);
        return nSoError ? Fail(MapErrno(nSoError)) : Succeed(0);
    }
    return Fail(MapErrno(nErrno));
}

int Bind(int nFd, const void* pWinAddr, int nLen) {
    sockaddr_in tAddr;
    if (!ToBsdAddr(pWinAddr, nLen, &tAddr)) {
        return -1;
    }
    if (::bind(nFd, (const sockaddr*)&tAddr, sizeof(tAddr)) != 0) {
        return FailErrno();
    }
    return Succeed(0);
}

int Listen(int nFd, int nBacklog) {
    if (::listen(nFd, nBacklog) != 0) {
        return FailErrno();
    }
    return Succeed(0);
}

int Accept(int nFd, void* pWinAddr, int* pnLen) {
    if (pWinAddr && (!pnLen || *pnLen < WIN_SOCKADDR_IN_SIZE)) {
        return Fail(WSA_EFAULT);
    }
    sockaddr_storage tAddr;
    socklen_t nAddrLen;
    int nClient;
    do {
        nAddrLen = sizeof(tAddr);
        nClient = ::accept(nFd, (sockaddr*)&tAddr, &nAddrLen);
    } while (nClient < 0 && errno == EINTR);
    if (nClient < 0) {
        return FailErrno();
    }
    PrepareSocket(nClient);
    ForgetConnect(nClient);
    // Winsock: the accepted socket has the listening socket's properties, non-blocking among them.
    if (IsNonBlocking(nFd)) {
        fcntl(nClient, F_SETFL, fcntl(nClient, F_GETFL, 0) | O_NONBLOCK);
    } else {
        fcntl(nClient, F_SETFL, fcntl(nClient, F_GETFL, 0) & ~O_NONBLOCK);
    }
    FromBsdAddr(tAddr, pWinAddr, pnLen);
    return Succeed(nClient);
}

int Send(int nFd, const char* pBuf, int nLen, int nFlags) {
    if (nLen < 0) {
        return Fail(WSA_EFAULT);
    }
    ssize_t nSent;
    do {
        nSent = ::send(nFd, pBuf, (size_t)nLen, SendFlags(nFlags));
    } while (nSent < 0 && errno == EINTR);
    if (nSent < 0) {
        return FailErrno();
    }
    return Succeed((int)nSent);
}

int Recv(int nFd, char* pBuf, int nLen, int nFlags) {
    return RecvFrom(nFd, pBuf, nLen, nFlags, nullptr, nullptr);
}

int SendTo(int nFd, const char* pBuf, int nLen, int nFlags, const void* pWinAddr, int nAddrLen) {
    if (!pWinAddr) {
        return Send(nFd, pBuf, nLen, nFlags);
    }
    sockaddr_in tAddr;
    if (nLen < 0) {
        return Fail(WSA_EFAULT);
    }
    if (!ToBsdAddr(pWinAddr, nAddrLen, &tAddr)) {
        return -1;
    }
    ssize_t nSent;
    do {
        nSent = ::sendto(nFd, pBuf, (size_t)nLen, SendFlags(nFlags), (const sockaddr*)&tAddr, sizeof(tAddr));
    } while (nSent < 0 && errno == EINTR);
    if (nSent < 0) {
        return FailErrno();
    }
    return Succeed((int)nSent);
}

int RecvFrom(int nFd, char* pBuf, int nLen, int nFlags, void* pWinAddr, int* pnAddrLen) {
    if (nLen < 0 || (pWinAddr && (!pnAddrLen || *pnAddrLen < WIN_SOCKADDR_IN_SIZE))) {
        return Fail(WSA_EFAULT);
    }
    sockaddr_storage tAddr;
    memset(&tAddr, 0, sizeof(tAddr));
    iovec tIov = {pBuf, (size_t)nLen};
    msghdr tMsg;
    ssize_t nRead;
    do {
        memset(&tMsg, 0, sizeof(tMsg));
        tMsg.msg_name = &tAddr;
        tMsg.msg_namelen = sizeof(tAddr);
        tMsg.msg_iov = &tIov;
        tMsg.msg_iovlen = 1;
        nRead = ::recvmsg(nFd, &tMsg, MapMsgFlags(nFlags));
    } while (nRead < 0 && errno == EINTR);
    if (nRead < 0) {
        return FailErrno();
    }
    FromBsdAddr(tAddr, pWinAddr, pnAddrLen);
    // A datagram larger than the buffer: Winsock fills the buffer and fails the call.
    if (tMsg.msg_flags & MSG_TRUNC) {
        return Fail(WSA_EMSGSIZE);
    }
    return Succeed((int)nRead);
}

int Shutdown(int nFd, int nHow) {
    int nBsdHow;
    switch (nHow) {
    case WIN_SD_RECEIVE: nBsdHow = SHUT_RD; break;
    case WIN_SD_SEND: nBsdHow = SHUT_WR; break;
    case WIN_SD_BOTH: nBsdHow = SHUT_RDWR; break;
    default: return Fail(WSA_EINVAL);
    }
    if (::shutdown(nFd, nBsdHow) != 0) {
        return FailErrno();
    }
    return Succeed(0);
}

int SetSockOpt(int nFd, int nLevel, int nOptName, const char* pOptVal, int nOptLen) {
    int nBsdLevel, nBsdOpt;
    if (!MapOption(nLevel, nOptName, &nBsdLevel, &nBsdOpt)) {
        return Fail(WSA_ENOPROTOOPT);
    }
    int nResult;
    if (nOptName == WIN_SO_LINGER && nLevel == WIN_SOL_SOCKET) {
        if (!pOptVal || nOptLen < WIN_LINGER_SIZE) {
            return Fail(WSA_EFAULT);
        }
        uint16_t aLinger[2];
        memcpy(aLinger, pOptVal, sizeof(aLinger));
        linger tLinger = {aLinger[0] != 0, aLinger[1]};
        nResult = ::setsockopt(nFd, nBsdLevel, nBsdOpt, &tLinger, sizeof(tLinger));
    } else if (nOptName == WIN_SO_DONTLINGER && nLevel == WIN_SOL_SOCKET) {
        int nValue;
        if (!ReadIntOption(pOptVal, nOptLen, &nValue)) {
            return -1;
        }
        linger tLinger = {nValue == 0, 0};
        nResult = ::setsockopt(nFd, nBsdLevel, nBsdOpt, &tLinger, sizeof(tLinger));
    } else if ((nOptName == WIN_SO_SNDTIMEO || nOptName == WIN_SO_RCVTIMEO) && nLevel == WIN_SOL_SOCKET) {
        // Winsock's timeout is a DWORD of milliseconds, 0 for none.
        if (!pOptVal || nOptLen < (int)sizeof(uint32_t)) {
            return Fail(WSA_EFAULT);
        }
        uint32_t nMs;
        memcpy(&nMs, pOptVal, sizeof(nMs));
        timeval tTimeout;
        tTimeout.tv_sec = nMs / 1000;
        tTimeout.tv_usec = (int)(nMs % 1000) * 1000;
        nResult = ::setsockopt(nFd, nBsdLevel, nBsdOpt, &tTimeout, sizeof(tTimeout));
    } else {
        int nValue;
        if (!ReadIntOption(pOptVal, nOptLen, &nValue)) {
            return -1;
        }
        if (nOptName == WIN_SO_ERROR || nOptName == WIN_SO_TYPE || nOptName == WIN_SO_ACCEPTCONN) {
            return Fail(WSA_ENOPROTOOPT);
        }
        nResult = ::setsockopt(nFd, nBsdLevel, nBsdOpt, &nValue, sizeof(nValue));
    }
    if (nResult != 0) {
        return FailErrno();
    }
    return Succeed(0);
}

int GetSockOpt(int nFd, int nLevel, int nOptName, char* pOptVal, int* pnOptLen) {
    int nBsdLevel, nBsdOpt;
    if (!pOptVal || !pnOptLen) {
        return Fail(WSA_EFAULT);
    }
    if (!MapOption(nLevel, nOptName, &nBsdLevel, &nBsdOpt)) {
        return Fail(WSA_ENOPROTOOPT);
    }
    if (nLevel == WIN_SOL_SOCKET && (nOptName == WIN_SO_LINGER || nOptName == WIN_SO_DONTLINGER)) {
        linger tLinger;
        socklen_t nLen = sizeof(tLinger);
        if (::getsockopt(nFd, nBsdLevel, nBsdOpt, &tLinger, &nLen) != 0) {
            return FailErrno();
        }
        if (nOptName == WIN_SO_LINGER) {
            if (*pnOptLen < WIN_LINGER_SIZE) {
                return Fail(WSA_EFAULT);
            }
            uint16_t aLinger[2] = {(uint16_t)(tLinger.l_onoff != 0), (uint16_t)tLinger.l_linger};
            memcpy(pOptVal, aLinger, sizeof(aLinger));
            *pnOptLen = WIN_LINGER_SIZE;
            return Succeed(0);
        }
        int nValue = tLinger.l_onoff == 0;
        if (*pnOptLen < (int)sizeof(nValue)) {
            return Fail(WSA_EFAULT);
        }
        memcpy(pOptVal, &nValue, sizeof(nValue));
        *pnOptLen = sizeof(nValue);
        return Succeed(0);
    }
    if (*pnOptLen < (int)sizeof(int)) {
        return Fail(WSA_EFAULT);
    }
    int nValue = 0;
    if (nLevel == WIN_SOL_SOCKET && (nOptName == WIN_SO_SNDTIMEO || nOptName == WIN_SO_RCVTIMEO)) {
        timeval tTimeout;
        socklen_t nLen = sizeof(tTimeout);
        if (::getsockopt(nFd, nBsdLevel, nBsdOpt, &tTimeout, &nLen) != 0) {
            return FailErrno();
        }
        nValue = (int)(tTimeout.tv_sec * 1000 + tTimeout.tv_usec / 1000);
    } else if (nLevel == WIN_SOL_SOCKET && nOptName == WIN_SO_ERROR) {
        socklen_t nLen = sizeof(nValue);
        int nStored = 0;
        {
            std::lock_guard<std::mutex> tLock(gConnectLock);
            auto it = gConnectErrors.find(nFd);
            if (it != gConnectErrors.end()) {
                nStored = it->second;
                gConnectErrors.erase(it);
            }
        }
        if (::getsockopt(nFd, nBsdLevel, nBsdOpt, &nValue, &nLen) != 0) {
            return FailErrno();
        }
        nValue = nStored ? nStored : MapErrno(nValue);
    } else if (nLevel == WIN_SOL_SOCKET && nOptName == WIN_SO_TYPE) {
        socklen_t nLen = sizeof(nValue);
        if (::getsockopt(nFd, nBsdLevel, nBsdOpt, &nValue, &nLen) != 0) {
            return FailErrno();
        }
        nValue = nValue == SOCK_STREAM ? WIN_SOCK_STREAM : nValue == SOCK_DGRAM ? WIN_SOCK_DGRAM : WIN_SOCK_RAW;
    } else {
        socklen_t nLen = sizeof(nValue);
        if (::getsockopt(nFd, nBsdLevel, nBsdOpt, &nValue, &nLen) != 0) {
            return FailErrno();
        }
        // The BOOL options read back as 0 or 1, as Winsock gives them.
        if (nOptName != WIN_SO_SNDBUF && nOptName != WIN_SO_RCVBUF && nOptName != WIN_SO_SNDLOWAT &&
            nOptName != WIN_SO_RCVLOWAT) {
            nValue = nValue != 0;
        }
    }
    memcpy(pOptVal, &nValue, sizeof(nValue));
    *pnOptLen = sizeof(nValue);
    return Succeed(0);
}

int IoctlSocket(int nFd, uint32_t nCmd, uint32_t* pArg) {
    if (!pArg) {
        return Fail(WSA_EFAULT);
    }
    if (nCmd == WIN_FIONBIO) {
        int nFlags = fcntl(nFd, F_GETFL, 0);
        if (nFlags == -1) {
            return FailErrno();
        }
        nFlags = *pArg ? (nFlags | O_NONBLOCK) : (nFlags & ~O_NONBLOCK);
        if (fcntl(nFd, F_SETFL, nFlags) == -1) {
            return FailErrno();
        }
        return Succeed(0);
    }
    if (nCmd == WIN_FIONREAD) {
        int nAvailable = 0;
        if (::ioctl(nFd, FIONREAD, &nAvailable) != 0) {
            return FailErrno();
        }
        *pArg = (uint32_t)nAvailable;
        return Succeed(0);
    }
    if (nCmd == WIN_SIOCATMARK) {
        int nAtMark = 0;
        if (::ioctl(nFd, SIOCATMARK, &nAtMark) != 0) {
            return FailErrno();
        }
        // Winsock: TRUE when no out-of-band data waits.
        *pArg = nAtMark ? 1 : 0;
        return Succeed(0);
    }
    return Fail(WSA_EINVAL);
}

int GetSockName(int nFd, void* pWinAddr, int* pnLen) {
    if (!pWinAddr || !pnLen) {
        return Fail(WSA_EFAULT);
    }
    sockaddr_storage tAddr;
    socklen_t nLen = sizeof(tAddr);
    if (::getsockname(nFd, (sockaddr*)&tAddr, &nLen) != 0) {
        return FailErrno();
    }
    if (!FromBsdAddr(tAddr, pWinAddr, pnLen)) {
        return -1;
    }
    return Succeed(0);
}

int GetPeerName(int nFd, void* pWinAddr, int* pnLen) {
    if (!pWinAddr || !pnLen) {
        return Fail(WSA_EFAULT);
    }
    sockaddr_storage tAddr;
    socklen_t nLen = sizeof(tAddr);
    if (::getpeername(nFd, (sockaddr*)&tAddr, &nLen) != 0) {
        return FailErrno();
    }
    if (!FromBsdAddr(tAddr, pWinAddr, pnLen)) {
        return -1;
    }
    return Succeed(0);
}

int Select(PollEntry* pEntries, int nCount, int nTimeoutMs) {
    if (nCount <= 0) {
        return Fail(WSA_EINVAL);
    }
    std::vector<pollfd> aPoll((size_t)nCount);
    std::vector<uint8_t> aConnecting((size_t)nCount, 0);
    {
        std::lock_guard<std::mutex> tLock(gConnectLock);
        for (int i = 0; i < nCount; i++) {
            aConnecting[i] = gConnecting.count(pEntries[i].nFd) != 0;
        }
    }
    for (int i = 0; i < nCount; i++) {
        short nEvents = 0;
        if (pEntries[i].nWant & POLL_READ) nEvents |= POLLIN;
        if (pEntries[i].nWant & POLL_WRITE) nEvents |= POLLOUT;
        if (pEntries[i].nWant & POLL_EXCEPT) nEvents |= aConnecting[i] ? POLLOUT : POLLPRI;
        aPoll[i].fd = pEntries[i].nFd;
        aPoll[i].events = nEvents;
        aPoll[i].revents = 0;
        pEntries[i].nReady = 0;
    }
    int64_t nDeadline = nTimeoutMs < 0 ? -1 : NowMs() + nTimeoutMs;
    int nPolled;
    for (;;) {
        int nWait = -1;
        if (nDeadline >= 0) {
            int64_t nLeft = nDeadline - NowMs();
            nWait = nLeft > 0 ? (int)nLeft : 0;
        }
        nPolled = ::poll(aPoll.data(), (nfds_t)nCount, nWait);
        if (nPolled >= 0 || errno != EINTR) {
            break;
        }
    }
    if (nPolled < 0) {
        return FailErrno();
    }
    int nReady = 0;
    for (int i = 0; i < nCount; i++) {
        short nRevents = aPoll[i].revents;
        if (nRevents & POLLNVAL) {
            return Fail(WSA_ENOTSOCK);
        }
        uint8_t nWant = pEntries[i].nWant;
        uint8_t nGot = 0;
        if (aConnecting[i] && (nRevents & (POLLOUT | POLLERR | POLLHUP))) {
            // The connect has finished: writable when it worked, in the except set when not.
            int nSoError = 0;
            socklen_t nSoLen = sizeof(nSoError);
            getsockopt(aPoll[i].fd, SOL_SOCKET, SO_ERROR, &nSoError, &nSoLen);
            {
                std::lock_guard<std::mutex> tLock(gConnectLock);
                gConnecting.erase(aPoll[i].fd);
                if (nSoError) {
                    gConnectErrors[aPoll[i].fd] = MapErrno(nSoError);
                }
            }
            if (nSoError) {
                nGot |= nWant & POLL_EXCEPT;
                if (nWant & POLL_READ) nGot |= POLL_READ;
            } else {
                nGot |= nWant & POLL_WRITE;
                if ((nWant & POLL_READ) && (nRevents & POLLIN)) nGot |= POLL_READ;
            }
        } else if (!aConnecting[i]) {
            if ((nWant & POLL_READ) && (nRevents & (POLLIN | POLLHUP | POLLERR))) nGot |= POLL_READ;
            if ((nWant & POLL_WRITE) && (nRevents & (POLLOUT | POLLERR | POLLHUP))) nGot |= POLL_WRITE;
            if ((nWant & POLL_EXCEPT) && (nRevents & POLLPRI)) nGot |= POLL_EXCEPT;
        }
        pEntries[i].nReady = nGot;
        nReady += (nGot & POLL_READ) != 0;
        nReady += (nGot & POLL_WRITE) != 0;
        nReady += (nGot & POLL_EXCEPT) != 0;
    }
    return Succeed(nReady);
}

int GetHostByName(const char* szName, char* szCanonName, int nCanonSize, uint32_t* pAddrs, int nMaxAddrs) {
    if (!szName) {
        return Fail(WSA_EFAULT);
    }
    addrinfo tHints;
    memset(&tHints, 0, sizeof(tHints));
    tHints.ai_family = AF_INET;
    tHints.ai_socktype = SOCK_STREAM;
    tHints.ai_flags = AI_CANONNAME;
    addrinfo* pResult = nullptr;
    int nError = ::getaddrinfo(szName, nullptr, &tHints, &pResult);
    if (nError != 0) {
        switch (nError) {
        case EAI_AGAIN: return Fail(WSA_TRY_AGAIN);
        case EAI_FAIL: return Fail(WSA_NO_RECOVERY);
#if defined(EAI_NODATA) && EAI_NODATA != EAI_NONAME
        case EAI_NODATA: return Fail(WSA_NO_DATA);
#endif
        case EAI_ADDRFAMILY: return Fail(WSA_NO_DATA);
        default: return Fail(WSA_HOST_NOT_FOUND);
        }
    }
    int nCount = 0;
    if (szCanonName && nCanonSize > 0) {
        const char* szCanon = pResult && pResult->ai_canonname ? pResult->ai_canonname : szName;
        strncpy(szCanonName, szCanon, (size_t)nCanonSize - 1);
        szCanonName[nCanonSize - 1] = '\0';
    }
    for (addrinfo* p = pResult; p && nCount < nMaxAddrs; p = p->ai_next) {
        if (p->ai_family != AF_INET) {
            continue;
        }
        uint32_t nAddr = ((const sockaddr_in*)p->ai_addr)->sin_addr.s_addr;
        bool bSeen = false;
        for (int i = 0; i < nCount; i++) {
            bSeen = bSeen || pAddrs[i] == nAddr;
        }
        if (!bSeen) {
            pAddrs[nCount++] = nAddr;
        }
    }
    freeaddrinfo(pResult);
    if (nCount == 0) {
        return Fail(WSA_NO_DATA);
    }
    return Succeed(nCount);
}

int GetHostName(char* szName, int nNameLen) {
    if (!szName || nNameLen <= 0) {
        return Fail(WSA_EFAULT);
    }
    char szHost[256];
    if (::gethostname(szHost, sizeof(szHost)) != 0) {
        return FailErrno();
    }
    szHost[sizeof(szHost) - 1] = '\0';
    if ((int)strlen(szHost) >= nNameLen) {
        return Fail(WSA_EFAULT);
    }
    strcpy(szName, szHost);
    return Succeed(0);
}

uint32_t InetAddr(const char* szText) {
    if (!szText) {
        return 0xffffffffu;
    }
    return (uint32_t)::inet_addr(szText);
}

}  // namespace Win32PosixBsd

#endif  // !defined(_WIN32) && D2_BACKEND_POSIX
