// Win32Posix: the BSD side of Winsock (SocketsBsd.cpp), as Sockets.cpp calls it. The Winsock
// records (sockaddr, in_addr, fd_set, hostent) and the BSD ones share their names and differ
// in layout, so no unit can read both: Sockets.cpp reads Win32PosixSockets.h, SocketsBsd.cpp
// the system headers, and this interface between them passes only scalars, the Winsock
// sockaddr as bytes and descriptor lists. Not part of the surface the game sees.
//
// A socket is its file descriptor. Every function that fails returns -1 (or the value the
// Winsock call fails with) and leaves the Winsock error code in LastError(), which is what
// WSAGetLastError reads; the errno values are mapped to WSAE* codes here.
#pragma once

#if !defined(_WIN32) && D2_BACKEND_POSIX

#include <stdint.h>

namespace Win32PosixBsd {

// The calling thread's Winsock error (Winsock keeps it per thread).
int& LastError();

// One socket of a select(): the sets it was in (want) and the sets it is ready for (ready).
enum : uint8_t { POLL_READ = 1, POLL_WRITE = 2, POLL_EXCEPT = 4 };
struct PollEntry {
    int nFd;
    uint8_t nWant;
    uint8_t nReady;
};

int Socket(int nAf, int nType, int nProtocol);
int Close(int nFd);
// pWinAddr is a Winsock sockaddr (a 16-bit family, then the family's bytes).
int Connect(int nFd, const void* pWinAddr, int nLen);
int Bind(int nFd, const void* pWinAddr, int nLen);
int Listen(int nFd, int nBacklog);
int Accept(int nFd, void* pWinAddr, int* pnLen);
int Send(int nFd, const char* pBuf, int nLen, int nFlags);
int Recv(int nFd, char* pBuf, int nLen, int nFlags);
int SendTo(int nFd, const char* pBuf, int nLen, int nFlags, const void* pWinAddr, int nAddrLen);
int RecvFrom(int nFd, char* pBuf, int nLen, int nFlags, void* pWinAddr, int* pnAddrLen);
int Shutdown(int nFd, int nHow);
int SetSockOpt(int nFd, int nLevel, int nOptName, const char* pOptVal, int nOptLen);
int GetSockOpt(int nFd, int nLevel, int nOptName, char* pOptVal, int* pnOptLen);
int IoctlSocket(int nFd, uint32_t nCmd, uint32_t* pArg);
int GetSockName(int nFd, void* pWinAddr, int* pnLen);
int GetPeerName(int nFd, void* pWinAddr, int* pnLen);
// Winsock's select over the entries: the number of (socket, set) pairs that are ready, 0 on a
// timeout, -1 on an error. nTimeoutMs < 0 waits for ever.
int Select(PollEntry* pEntries, int nCount, int nTimeoutMs);
// The IPv4 addresses of a name, network order: their count, or -1 with a WSAHOST_* error.
int GetHostByName(const char* szName, char* szCanonName, int nCanonSize, uint32_t* pAddrs, int nMaxAddrs);
int GetHostName(char* szName, int nNameLen);
// inet_addr: the address in network order, or 0xffffffff (INADDR_NONE).
uint32_t InetAddr(const char* szText);

}  // namespace Win32PosixBsd

#endif  // !defined(_WIN32) && D2_BACKEND_POSIX
