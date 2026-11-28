// Win32Posix: a loopback test of the Winsock layer (Sockets.cpp, SocketsBsd.cpp), through the
// Winsock declarations the game reads - TCP and UDP, blocking and non-blocking, the select()
// sets and the WSAE* codes the shared code compares against.
//
// Built with the forced d2_platform.h, as a game unit is, and linked with the two layer units
// and the kernel: the d2_sockets_test target (cmake/mac-modern.cmake).
// Exits 0 when every check passes.
#if !defined(_WIN32) && D2_BACKEND_POSIX

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

namespace {

// Winsock's numbers, as the shared code spells them (literals).
constexpr int kWouldBlock = 0x2733;       // WSAEWOULDBLOCK
constexpr int kNotSock = 10038;           // WSAENOTSOCK
constexpr int kMsgSize = 10040;           // WSAEMSGSIZE
constexpr int kConnReset = 10054;         // WSAECONNRESET
constexpr int kConnRefused = 10061;       // WSAECONNREFUSED
constexpr int kHostNotFound = 11001;      // WSAHOST_NOT_FOUND
constexpr int kNotInitialised = 10093;    // WSANOTINITIALISED
constexpr long kFionbio = -0x7ffb9982;    // FIONBIO, as Safesock passes it
constexpr long kFionread = 0x4004667f;    // FIONREAD
constexpr int kSolSocket = 0xffff;
constexpr int kSoReuseAddr = 4;
constexpr int kSoKeepAlive = 8;
constexpr int kSoLinger = 0x80;
constexpr int kSoSndTimeo = 0x1005;
constexpr int kSoRcvTimeo = 0x1006;
constexpr int kSoError = 0x1007;
constexpr int kSoType = 0x1008;
constexpr int kTcpNoDelay = 1;
constexpr uint32_t kLoopback = 0x0100007f;  // 127.0.0.1, network order on a little-endian host

int gnFailures = 0;
int gnChecks = 0;

bool gbVerbose = false;

void Check(bool bOk, const char* szWhat, long nGot = 0, long nWant = 0) {
    gnChecks++;
    if (gbVerbose) {
        printf("%s %s\n", bOk ? "ok  " : "FAIL", szWhat);
    }
    if (!bOk) {
        gnFailures++;
        printf("FAIL %s (got %ld, want %ld)\n", szWhat, nGot, nWant);
    }
}

// Each argument is evaluated once: they are the calls under test.
void CheckEq(long nGot, long nWant, const char* szWhat) {
    Check(nGot == nWant, szWhat, nGot, nWant);
}
#define CHECK_EQ(a, b, what) CheckEq((long)(a), (long)(b), what)

sockaddr_in Loopback(uint16_t nPortNet) {
    sockaddr_in tAddr;
    memset(&tAddr, 0, sizeof(tAddr));
    tAddr.sin_family = AF_INET;
    tAddr.sin_port = nPortNet;
    tAddr.sin_addr = kLoopback;
    return tAddr;
}

uint16_t BoundPort(SOCKET s) {
    sockaddr_in tAddr;
    int nLen = sizeof(tAddr);
    int nResult = getsockname(s, (sockaddr*)&tAddr, &nLen);
    CHECK_EQ(nResult, 0, "getsockname");
    CHECK_EQ(nLen, sizeof(sockaddr_in), "getsockname length");
    CHECK_EQ(tAddr.sin_family, AF_INET, "getsockname family");
    return tAddr.sin_port;
}

SOCKET Listener(uint16_t* pPort) {
    SOCKET s = socket(AF_INET, SOCK_STREAM, 0);
    Check(s != INVALID_SOCKET, "listen socket");
    int nOn = 1;
    CHECK_EQ(setsockopt(s, kSolSocket, kSoReuseAddr, (char*)&nOn, 4), 0, "SO_REUSEADDR");
    sockaddr_in tAddr = Loopback(0);
    CHECK_EQ(_Wrappers::bind(s, (sockaddr*)&tAddr, sizeof(tAddr)), 0, "bind");
    CHECK_EQ(_Wrappers::listen(s, 5), 0, "listen");
    *pPort = BoundPort(s);
    Check(*pPort != 0, "bound port");
    return s;
}

int SelectOne(SOCKET s, bool bRead, bool bWrite, bool bExcept, int nMs, int* pnFlags) {
    fd_set tRead, tWrite, tExcept;
    FD_ZERO(&tRead);
    FD_ZERO(&tWrite);
    FD_ZERO(&tExcept);
    if (bRead) FD_SET(s, &tRead);
    if (bWrite) FD_SET(s, &tWrite);
    if (bExcept) FD_SET(s, &tExcept);
    timeval tTimeout;
    tTimeout.tv_sec = nMs / 1000;
    tTimeout.tv_usec = (nMs % 1000) * 1000;
    int nResult = select(0x40, bRead ? &tRead : nullptr, bWrite ? &tWrite : nullptr, bExcept ? &tExcept : nullptr,
                         &tTimeout);
    *pnFlags = (FD_ISSET(s, &tRead) && bRead ? 1 : 0) | (FD_ISSET(s, &tWrite) && bWrite ? 2 : 0) |
               (FD_ISSET(s, &tExcept) && bExcept ? 4 : 0);
    return nResult;
}

void TestTcp() {
    uint16_t nPort;
    SOCKET hListen = Listener(&nPort);

    // A non-blocking connect: WSAEWOULDBLOCK, then writable.
    SOCKET hClient = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    Check(hClient != INVALID_SOCKET, "client socket");
    u_long nNonBlocking = 1;
    CHECK_EQ(ioctlsocket(hClient, kFionbio, &nNonBlocking), 0, "ioctlsocket FIONBIO");
    sockaddr_in tServer = Loopback(nPort);
    int nResult = connect(hClient, (sockaddr*)&tServer, sizeof(tServer));
    if (nResult != 0) {
        CHECK_EQ(nResult, SOCKET_ERROR, "non-blocking connect result");
        CHECK_EQ(WSAGetLastError(), kWouldBlock, "non-blocking connect error");
    }
    int nFlags;
    nResult = SelectOne(hClient, false, true, true, 2000, &nFlags);
    CHECK_EQ(nResult, 1, "select after connect");
    CHECK_EQ(nFlags, 2, "connect reported writable only");

    sockaddr_in tPeer;
    int nPeerLen = sizeof(tPeer);
    SOCKET hServer = _Wrappers::accept(hListen, (sockaddr*)&tPeer, &nPeerLen);
    Check(hServer != INVALID_SOCKET, "accept");
    CHECK_EQ(nPeerLen, sizeof(sockaddr_in), "accept address length");
    CHECK_EQ(tPeer.sin_family, AF_INET, "accept address family");
    CHECK_EQ((uint32_t)tPeer.sin_addr, kLoopback, "accept address");
    Check(strcmp(inet_ntoa(tPeer.sin_addr), "127.0.0.1") == 0, "inet_ntoa of the peer");

    sockaddr_in tName;
    int nNameLen = sizeof(tName);
    CHECK_EQ(getpeername(hClient, (sockaddr*)&tName, &nNameLen), 0, "getpeername");
    CHECK_EQ(tName.sin_port, nPort, "getpeername port");

    // Nothing to read yet.
    char aBuf[64];
    CHECK_EQ(recv(hClient, aBuf, sizeof(aBuf), 0), SOCKET_ERROR, "empty non-blocking recv");
    CHECK_EQ(WSAGetLastError(), kWouldBlock, "empty non-blocking recv error");
    nResult = SelectOne(hClient, true, false, false, 50, &nFlags);
    CHECK_EQ(nResult, 0, "select timeout with nothing to read");

    // The options the game sets.
    int nOn = 1;
    CHECK_EQ(setsockopt(hClient, IPPROTO_TCP, kTcpNoDelay, (char*)&nOn, 4), 0, "TCP_NODELAY");
    CHECK_EQ(setsockopt(hClient, kSolSocket, kSoKeepAlive, (char*)&nOn, 4), 0, "SO_KEEPALIVE");
    int nValue = 0, nLen = 4;
    CHECK_EQ(getsockopt(hClient, IPPROTO_TCP, kTcpNoDelay, (char*)&nValue, &nLen), 0, "get TCP_NODELAY");
    CHECK_EQ(nValue, 1, "TCP_NODELAY value");
    uint32_t nTimeoutMs = 1500;
    CHECK_EQ(setsockopt(hClient, kSolSocket, kSoSndTimeo, (char*)&nTimeoutMs, 4), 0, "SO_SNDTIMEO");
    CHECK_EQ(setsockopt(hClient, kSolSocket, kSoRcvTimeo, (char*)&nTimeoutMs, 4), 0, "SO_RCVTIMEO");
    nLen = 4;
    CHECK_EQ(getsockopt(hClient, kSolSocket, kSoRcvTimeo, (char*)&nValue, &nLen), 0, "get SO_RCVTIMEO");
    CHECK_EQ(nValue, 1500, "SO_RCVTIMEO in milliseconds");
    nLen = 4;
    CHECK_EQ(getsockopt(hClient, kSolSocket, kSoType, (char*)&nValue, &nLen), 0, "get SO_TYPE");
    CHECK_EQ(nValue, SOCK_STREAM, "SO_TYPE");
    CHECK_EQ(setsockopt(hClient, kSolSocket, 0x7777, (char*)&nOn, 4), SOCKET_ERROR, "unknown option");
    CHECK_EQ(WSAGetLastError(), 10042, "unknown option error");

    // Data both ways.
    CHECK_EQ(send(hServer, "hello", 5, 0), 5, "server send");
    nResult = SelectOne(hClient, true, false, false, 2000, &nFlags);
    CHECK_EQ(nResult, 1, "select readable");
    CHECK_EQ(nFlags, 1, "readable flag");
    u_long nAvailable = 0;
    CHECK_EQ(ioctlsocket(hClient, kFionread, &nAvailable), 0, "FIONREAD");
    CHECK_EQ(nAvailable, 5, "FIONREAD count");
    CHECK_EQ(recv(hClient, aBuf, sizeof(aBuf), 0), 5, "client recv");
    Check(memcmp(aBuf, "hello", 5) == 0, "client recv data");
    CHECK_EQ(send(hClient, "ping", 4, 0), 4, "client send");
    CHECK_EQ(recv(hServer, aBuf, sizeof(aBuf), 0), 4, "server blocking recv");

    // Both sets at once count twice.
    CHECK_EQ(send(hServer, "x", 1, 0), 1, "server send one");
    {
        fd_set tRead, tWrite;
        FD_ZERO(&tRead);
        FD_ZERO(&tWrite);
        FD_SET(hClient, &tRead);
        FD_SET(hClient, &tWrite);
        FD_SET(hListen, &tRead);
        timeval tTimeout = {2, 0};
        // Wait until the byte is there, then count.
        for (int i = 0; i < 20; i++) {
            u_long nPending = 0;
            ioctlsocket(hClient, kFionread, &nPending);
            if (nPending) break;
            struct timespec tSleep = {0, 10 * 1000 * 1000};
            nanosleep(&tSleep, nullptr);
        }
        nResult = select(0, &tRead, &tWrite, nullptr, &tTimeout);
        CHECK_EQ(nResult, 2, "select counts each (socket, set) pair");
        CHECK_EQ(tRead.fd_count, 1, "read set rewritten to the ready socket");
        CHECK_EQ(tRead.fd_array[0], hClient, "read set keeps the client");
        CHECK_EQ(tWrite.fd_count, 1, "write set keeps the client");
        recv(hClient, aBuf, sizeof(aBuf), 0);
    }

    // A reset: linger 0 on the server, then close; the client sees WSAECONNRESET and its send
    // fails without a SIGPIPE.
    uint16_t aLinger[2] = {1, 0};
    CHECK_EQ(setsockopt(hServer, kSolSocket, kSoLinger, (char*)aLinger, 4), 0, "SO_LINGER");
    uint16_t aLingerBack[2] = {9, 9};
    nLen = 4;
    CHECK_EQ(getsockopt(hServer, kSolSocket, kSoLinger, (char*)aLingerBack, &nLen), 0, "get SO_LINGER");
    CHECK_EQ(aLingerBack[0], 1, "linger on");
    CHECK_EQ(aLingerBack[1], 0, "linger time");
    CHECK_EQ(closesocket(hServer), 0, "closesocket server");
    nResult = SelectOne(hClient, true, false, false, 2000, &nFlags);
    CHECK_EQ(nResult, 1, "reset makes the socket readable");
    CHECK_EQ(recv(hClient, aBuf, sizeof(aBuf), 0), SOCKET_ERROR, "recv after reset");
    CHECK_EQ(WSAGetLastError(), kConnReset, "recv after reset error");
    int nSendResult = 0;
    for (int i = 0; i < 4 && nSendResult != SOCKET_ERROR; i++) {
        nSendResult = send(hClient, "late", 4, 0);
    }
    CHECK_EQ(nSendResult, SOCKET_ERROR, "send after reset fails");
    Check(WSAGetLastError() == kConnReset || WSAGetLastError() == 10057 || WSAGetLastError() == 10058,
          "send after reset error", WSAGetLastError(), kConnReset);
    CHECK_EQ(closesocket(hClient), 0, "closesocket client");

    // A graceful close reads as 0.
    SOCKET hClient2 = socket(AF_INET, SOCK_STREAM, 0);
    CHECK_EQ(connect(hClient2, (sockaddr*)&tServer, sizeof(tServer)), 0, "blocking connect");
    SOCKET hServer2 = _Wrappers::accept(hListen, nullptr, nullptr);
    Check(hServer2 != INVALID_SOCKET, "accept without an address");
    CHECK_EQ(shutdown(hServer2, 1), 0, "shutdown send");
    CHECK_EQ(recv(hClient2, aBuf, sizeof(aBuf), 0), 0, "recv after a graceful close");
    closesocket(hServer2);
    closesocket(hClient2);

    // Refused: blocking, and non-blocking through the except set.
    closesocket(hListen);
    SOCKET hRefused = socket(AF_INET, SOCK_STREAM, 0);
    CHECK_EQ(connect(hRefused, (sockaddr*)&tServer, sizeof(tServer)), SOCKET_ERROR, "refused blocking connect");
    CHECK_EQ(WSAGetLastError(), kConnRefused, "refused blocking connect error");
    closesocket(hRefused);
    hRefused = socket(AF_INET, SOCK_STREAM, 0);
    nNonBlocking = 1;
    ioctlsocket(hRefused, kFionbio, &nNonBlocking);
    nResult = connect(hRefused, (sockaddr*)&tServer, sizeof(tServer));
    if (nResult == SOCKET_ERROR && WSAGetLastError() == kWouldBlock) {
        nResult = SelectOne(hRefused, false, true, true, 2000, &nFlags);
        CHECK_EQ(nResult, 1, "select after a refused connect");
        CHECK_EQ(nFlags, 4, "refused connect in the except set only");
        nLen = 4;
        CHECK_EQ(getsockopt(hRefused, kSolSocket, kSoError, (char*)&nValue, &nLen), 0, "get SO_ERROR");
        CHECK_EQ(nValue, kConnRefused, "SO_ERROR of a refused connect");
    } else {
        CHECK_EQ(WSAGetLastError(), kConnRefused, "refused non-blocking connect error");
    }
    closesocket(hRefused);

    // Not a socket.
    CHECK_EQ(closesocket(INVALID_SOCKET), SOCKET_ERROR, "closesocket(INVALID_SOCKET)");
    CHECK_EQ(WSAGetLastError(), kNotSock, "closesocket(INVALID_SOCKET) error");
    CHECK_EQ(recv((SOCKET)0x7ffffff0, aBuf, 1, 0), SOCKET_ERROR, "recv on a bad descriptor");
    CHECK_EQ(WSAGetLastError(), kNotSock, "recv on a bad descriptor error");
}

void TestUdp() {
    SOCKET hA = socket(AF_INET, SOCK_DGRAM, 0);
    SOCKET hB = socket(AF_INET, SOCK_DGRAM, 0);
    Check(hA != INVALID_SOCKET && hB != INVALID_SOCKET, "udp sockets");
    sockaddr_in tAny = Loopback(0);
    CHECK_EQ(_Wrappers::bind(hA, (sockaddr*)&tAny, sizeof(tAny)), 0, "udp bind a");
    CHECK_EQ(_Wrappers::bind(hB, (sockaddr*)&tAny, sizeof(tAny)), 0, "udp bind b");
    sockaddr_in tA = Loopback(BoundPort(hA));
    sockaddr_in tB = Loopback(BoundPort(hB));

    u_long nNonBlocking = 1;
    ioctlsocket(hB, kFionbio, &nNonBlocking);
    char aBuf[64];
    sockaddr_in tFrom;
    int nFromLen = sizeof(tFrom);
    CHECK_EQ(recvfrom(hB, aBuf, sizeof(aBuf), 0, (sockaddr*)&tFrom, &nFromLen), SOCKET_ERROR, "empty udp recvfrom");
    CHECK_EQ(WSAGetLastError(), kWouldBlock, "empty udp recvfrom error");

    CHECK_EQ(sendto(hA, "datagram", 8, 0, (sockaddr*)&tB, sizeof(tB)), 8, "udp sendto");
    int nFlags;
    CHECK_EQ(SelectOne(hB, true, false, false, 2000, &nFlags), 1, "udp select readable");
    nFromLen = sizeof(tFrom);
    CHECK_EQ(recvfrom(hB, aBuf, sizeof(aBuf), 0, (sockaddr*)&tFrom, &nFromLen), 8, "udp recvfrom");
    Check(memcmp(aBuf, "datagram", 8) == 0, "udp data");
    CHECK_EQ(nFromLen, sizeof(sockaddr_in), "udp from length");
    CHECK_EQ(tFrom.sin_port, tA.sin_port, "udp from port");
    CHECK_EQ((uint32_t)tFrom.sin_addr, kLoopback, "udp from address");

    // A datagram larger than the buffer: WSAEMSGSIZE.
    CHECK_EQ(sendto(hA, "0123456789", 10, 0, (sockaddr*)&tB, sizeof(tB)), 10, "udp sendto long");
    SelectOne(hB, true, false, false, 2000, &nFlags);
    CHECK_EQ(recvfrom(hB, aBuf, 4, 0, nullptr, nullptr), SOCKET_ERROR, "truncated datagram");
    CHECK_EQ(WSAGetLastError(), kMsgSize, "truncated datagram error");
    Check(memcmp(aBuf, "0123", 4) == 0, "truncated datagram keeps what fits");

    // Connected UDP with send/recv.
    CHECK_EQ(connect(hA, (sockaddr*)&tB, sizeof(tB)), 0, "udp connect");
    CHECK_EQ(send(hA, "conn", 4, 0), 4, "udp send");
    SelectOne(hB, true, false, false, 2000, &nFlags);
    CHECK_EQ(recv(hB, aBuf, sizeof(aBuf), 0), 4, "udp recv");
    closesocket(hA);
    closesocket(hB);
}

void TestNames() {
    CHECK_EQ(inet_addr("127.0.0.1"), kLoopback, "inet_addr");
    CHECK_EQ(inet_addr("24.105.29.30"), 0x1e1d6918u, "inet_addr of a realm address");
    CHECK_EQ(inet_addr("not an address"), INADDR_NONE, "inet_addr of a name");
    in_addr tAddr(0x1e1d6918u);
    Check(strcmp(inet_ntoa(tAddr), "24.105.29.30") == 0, "inet_ntoa");

    hostent* pHost = gethostbyname("localhost");
    Check(pHost != nullptr, "gethostbyname(localhost)");
    if (pHost) {
        CHECK_EQ(pHost->h_addrtype, AF_INET, "hostent address type");
        CHECK_EQ(pHost->h_length, 4, "hostent address length");
        bool bLoopback = false;
        for (int i = 0; pHost->h_addr_list[i]; i++) {
            uint32_t nAddr;
            memcpy(&nAddr, pHost->h_addr_list[i], sizeof(nAddr));
            bLoopback = bLoopback || nAddr == kLoopback;
        }
        Check(bLoopback, "localhost resolves to 127.0.0.1");
    }
    pHost = gethostbyname("127.0.0.1");
    Check(pHost && *(uint32_t*)pHost->h_addr == kLoopback, "gethostbyname of a dotted address");
    pHost = gethostbyname("no-such-host.invalid");
    Check(pHost == nullptr, "gethostbyname of an unknown name");
    CHECK_EQ(WSAGetLastError(), kHostNotFound, "unknown name error");

    char szHost[256];
    CHECK_EQ(gethostname(szHost, sizeof(szHost)), 0, "gethostname");
    Check(szHost[0] != 0, "gethostname gives a name");
    CHECK_EQ(gethostname(szHost, 1), SOCKET_ERROR, "gethostname into one byte");
    CHECK_EQ(WSAGetLastError(), 10014, "gethostname into one byte error");
}

}  // namespace

int main() {
    // W32P_VERBOSE=1 prints every check as it runs.
    gbVerbose = getenv("W32P_VERBOSE") != nullptr;
    setvbuf(stdout, nullptr, _IONBF, 0);
    WSADATA tData;
    CHECK_EQ(WSAStartup(0x101, &tData), 0, "WSAStartup");
    CHECK_EQ(tData.wVersion, 0x101, "WSAStartup version");
    CHECK_EQ(tData.wHighVersion, 0x202, "WSAStartup high version");
    CHECK_EQ(WSAStartup(0x0, &tData), 10092, "WSAStartup of version 0");
    TestTcp();
    TestUdp();
    TestNames();
    _Wrappers::WSASetLastError(1234);
    CHECK_EQ(WSAGetLastError(), 1234, "WSASetLastError");
    CHECK_EQ(WSACleanup(), 0, "WSACleanup");
    CHECK_EQ(WSACleanup(), SOCKET_ERROR, "WSACleanup without a WSAStartup");
    CHECK_EQ(WSAGetLastError(), kNotInitialised, "WSACleanup without a WSAStartup error");
    printf("%s: %d checks, %d failed\n", gnFailures ? "FAIL" : "OK", gnChecks, gnFailures);
    return gnFailures ? 1 : 0;
}

#endif  // !defined(_WIN32) && D2_BACKEND_POSIX
