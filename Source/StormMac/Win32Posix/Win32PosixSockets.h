// Win32Posix: Winsock 1.1 as the shared code (Fog's Safesock and QServer, D2Net, Bnclient)
// calls it, declared over Winsock's own types. The BSD socket headers stay out of the shared
// code: their sockaddr_in, fd_set, timeval and hostent are not Winsock's. The implementation
// translates to BSD sockets. Read through Win32Posix.h.
#pragma once

#if !defined(_WIN32) && D2_BACKEND_POSIX

// Winsock's in_addr: the S_un union, the shapes Storm's Mac layer gives it (Win32MacSockets.h),
// whose member names the shared code spells.
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
typedef in_addr IN_ADDR;
#define s_addr S_un.S_addr

struct sockaddr {
    u_short sa_family;
    char sa_data[14];
};
typedef sockaddr SOCKADDR;

// Winsock's start-up record (Windows Sockets 1.1), the SDK's layout, which moves the
// description strings to the end on 64-bit Windows.
struct WSAData {
    WORD wVersion;
    WORD wHighVersion;
#if UINTPTR_MAX == 0xFFFFFFFFu
    char szDescription[257];
    char szSystemStatus[129];
    unsigned short iMaxSockets;
    unsigned short iMaxUdpDg;
    char* lpVendorInfo;
#else
    unsigned short iMaxSockets;
    unsigned short iMaxUdpDg;
    char* lpVendorInfo;
    char szDescription[257];
    char szSystemStatus[129];
#endif
};
typedef WSAData WSADATA;
typedef WSAData* LPWSADATA;
WIN32POSIX_SIZE(WSADATA, 0x190, 0x198);

// The Winsock records the shared code holds by value, the SDK's layouts: the BSD headers'
// sockaddr_in, fd_set and hostent differ (sin_len, a bit-set fd_set).
struct sockaddr_in {
    short sin_family;
    u_short sin_port;
    in_addr sin_addr;
    char sin_zero[8];
};
typedef sockaddr_in SOCKADDR_IN;
WIN32POSIX_SIZE(sockaddr_in, 0x10, 0x10);

struct hostent {
    char* h_name;
    char** h_aliases;
    short h_addrtype;
    short h_length;
    char** h_addr_list;
};
typedef hostent HOSTENT;
#define h_addr h_addr_list[0]
WIN32POSIX_SIZE(hostent, 0x10, 0x20);

// timeval is the C library's (<sys/resource.h>, read through <cstdlib>, defines it before this
// layer can): 16 bytes on LP64 where Winsock's is 8. The shared code keeps it only as a local
// select timeout, by member name, so the layout does not reach it.

#ifndef FD_SETSIZE
#define FD_SETSIZE 64
#endif
struct fd_set {
    u_int fd_count;
    SOCKET fd_array[FD_SETSIZE];
};
WIN32POSIX_SIZE(fd_set, 0x104, 0x208);
#define FD_ZERO(set) (((fd_set*)(set))->fd_count = 0)
#define FD_ISSET(fd, set) __WSAFDIsSet((SOCKET)(fd), (fd_set*)(set))
#define FD_SET(fd, set)                                                                    \
    do {                                                                                   \
        u_int nFdIndex_;                                                                   \
        for (nFdIndex_ = 0; nFdIndex_ < ((fd_set*)(set))->fd_count; nFdIndex_++)           \
            if (((fd_set*)(set))->fd_array[nFdIndex_] == (fd)) break;                      \
        if (nFdIndex_ == ((fd_set*)(set))->fd_count && ((fd_set*)(set))->fd_count < FD_SETSIZE) \
            ((fd_set*)(set))->fd_array[((fd_set*)(set))->fd_count++] = (fd);               \
    } while (0)

#define AF_INET 2
#define SOCK_STREAM 1
#define SOCK_DGRAM 2
#define IPPROTO_TCP 6
#define INADDR_ANY ((u_long)0x00000000)
#define INADDR_NONE 0xffffffff
#define INVALID_SOCKET ((SOCKET)(~0))
#define SOCKET_ERROR (-1)

// WSAGetLastError and WSACleanup are d2_platform.h's.
int WSAStartup(WORD wVersionRequested, LPWSADATA lpWSAData);
int __WSAFDIsSet(SOCKET fd, fd_set* pSet);
SOCKET socket(int af, int type, int protocol);
int closesocket(SOCKET s);
int connect(SOCKET s, const sockaddr* name, int namelen);
int send(SOCKET s, const char* buf, int len, int flags);
int recv(SOCKET s, char* buf, int len, int flags);
int select(int nfds, fd_set* readfds, fd_set* writefds, fd_set* exceptfds, const timeval* timeout);
int setsockopt(SOCKET s, int level, int optname, const char* optval, int optlen);
int ioctlsocket(SOCKET s, long cmd, u_long* argp);
int getsockname(SOCKET s, sockaddr* name, int* namelen);
int getpeername(SOCKET s, sockaddr* name, int* namelen);
hostent* gethostbyname(const char* name);
int gethostname(char* name, int namelen);
unsigned long inet_addr(const char* cp);
char* inet_ntoa(in_addr in);

// The Winsock calls the shared code reaches through _Wrappers.
namespace _Wrappers {
SOCKET accept(SOCKET s, sockaddr* addr, int* addrlen);
int bind(SOCKET s, const sockaddr* name, int namelen);
int listen(SOCKET s, int backlog);
void WSASetLastError(int iError);
}  // namespace _Wrappers

#endif  // !defined(_WIN32) && D2_BACKEND_POSIX
