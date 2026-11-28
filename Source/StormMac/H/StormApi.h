// The Storm entry points StormMac defines and the shared code calls, in the namespaces the shared
// code (Source/Storm/Storm.h) and the binary's symbol namespaces give them. The definitions in
// StormMac/SOURCE carry these qualified names and these parameter types, so the shared callers
// and the Mac units reach the same symbols. Mac-only: see Win32Mac/Source/Win32/Win32Mac.h.
#pragma once

#include "D2Arch.h"

#if !defined(_WIN32) && D2_MAC_CARBON

#include <stddef.h>
#include <stdint.h>

struct ARGLIST;
struct BigData;
struct CMDPARAMS;
struct D2ArchiveStrc;
struct D2MPQFileStrc;
struct D2Sha1ContextStrc;
struct D2StormAsyncCompletionStrc;
struct IDirectSound;
struct IDirectSoundBuffer;
struct OVERLAPPED;

typedef void (*Fn0)(void*);
typedef int (*Fn1)(char*);
typedef int (*Fn2)(uint32_t*);
typedef int (*Fn3)(CMDPARAMS*, char*);
typedef int (*Fn4)(void*, uint32_t, int);

struct Win32MacWindow;
namespace Storm { namespace WindowHandle {
void SMsgRegisterKeyUp(Win32MacWindow*, uint32_t, Fn0);
int SMsgDispatchMessage(Win32MacWindow*, uint32_t, uint32_t, int32_t, int*, int*);
void SMsgRegisterCommand(Win32MacWindow*, uint32_t, Fn0);
void SMsgRegisterKeyDown(Win32MacWindow*, uint32_t, Fn0);
void SMsgRegisterMessage(Win32MacWindow*, uint32_t, Fn0);
void SMsgUnregisterKeyUp(Win32MacWindow*, uint32_t, Fn0);
int SMsgPopRegisterState(Win32MacWindow*);
int SMsgPushRegisterState(Win32MacWindow*);
void SMsgUnregisterCommand(Win32MacWindow*, uint32_t, Fn0);
void SMsgUnregisterKeyDown(Win32MacWindow*, uint32_t, Fn0);
void SMsgUnregisterMessage(Win32MacWindow*, uint32_t, Fn0);
}}

namespace Storm {
int SMsgBreakHandlerChain(void*);
}

namespace Storm { namespace Source { namespace SSignature {
void Sha1_Append(D2Sha1ContextStrc*, uint8_t*, uint32_t);
void Sha1_Finalize(D2Sha1ContextStrc*, uint8_t*);
}}}

namespace Storm { namespace Source {
int SCmdProcess(char*, int, Fn1, Fn2);
int SCmdRegisterArgList(ARGLIST*, uint32_t);
int SCmdRegisterArgument(uint32_t, uint32_t, char*, void*, uint32_t, uint32_t, uint32_t, Fn3);
}}

namespace Storm { namespace Source { namespace SBig {
void SBigPowMod(BigData*, BigData*, BigData*, BigData*);
void SBigFromBinary(BigData*, const void*, uint32_t);
void SBigFromUnsigned(BigData*, uint32_t);
void SBigToBinaryBuffer(BigData*, void*, uint32_t, uint32_t*);
int32_t SBIG_CompareBigBuffers_ForSha1(BigData*, BigData*);
void SBigAdd(BigData*, BigData*, BigData*);
void SBigDel(BigData*);
void SBigMul(BigData*, BigData*, BigData*);
void SBigNew(BigData**);
void SBigSub(BigData*, BigData*, BigData*);
}}}

namespace Storm { namespace Source { namespace SBMP {
int SBmpDecodeImage(int, const void*, uint32_t, void*, uint8_t*, int, uint32_t*, uint32_t*, uint32_t*);
}}}

namespace Storm { namespace Source { namespace SErr {
void SErrSetLastError(uint32_t);
}}}

namespace Storm { namespace Source { namespace SLog {
int SLogCreate(char const*, uint32_t, int*);
void SLogFlushAll();
uint32_t SLogSetIndent(uint32_t, int);
void SLogInitialize();
void SLogClose(uint32_t);
void SLogWrite(uint32_t, const char*, ...);
}}}

namespace Storm { namespace Source { namespace SMem {
void* SMemReAlloc(void*, uint32_t, const char*, int, uint32_t);
void SMemInitialize();
int SMemFree(void*, const char*, int, uint32_t);
void* SMemAlloc(uint32_t, const char*, uint32_t, uint32_t);
void SMemCopy(void*, void*, unsigned long);
void SMemFill(void*, unsigned long, uint8_t);
void SMemZero(void*, unsigned long);
}}}

namespace Storm { namespace Source { namespace SReg {
int SRegLoadData(char*, char*, int, uint8_t*, uint32_t, uint32_t*);
int SRegSaveData(char*, char const*, uint32_t, uint8_t*, uint32_t);
int SRegLoadValue(const char*, const char*, int, int*);
int SRegSaveValue(char*, char const*, int, int);
int SRegLoadString(const char*, const char*, int, char*, uint32_t);
int SRegSaveString(char*, char const*, int, char*);
int SRegDeleteValue(const char*, char const*, uint32_t);
}}}

namespace Storm { namespace Source { namespace SSTR {
uint32_t SStrHashHT(uint8_t*);
uint32_t SStrPrintf(char*, uint32_t, const char*, ...);
uint32_t SStrVPrintf(char*, uint32_t, const char*, char*);
void SStrTokenize(char**, char*, uint32_t, const char*, uint32_t*);
int UTF16_ToUTF8(uint8_t*, int, uint16_t*, int, int*, int*);
int UTF8_ToUTF16(short*, int, uint8_t*, int, int*, int*);
int TRANSTBL_Latin1ToUnicode(uint16_t*, uint8_t*, int);
char* SStrChr(const char*, char);
int SStrCmp(const char*, const char*, uint32_t);
char* SStrDup(const char*);
uint32_t SStrLen(const char*);
char* SStrChrR(const char*, char);
int SStrCmpI(const char*, const char*, uint32_t);
uint32_t SStrCopy(char*, const char*, int);
uint32_t SStrHash(char*, uint8_t, uint32_t);
uint32_t SStrPack(char*, const char*, uint32_t);
void SStrLower(char*);
void SStrUpper(char*);
}}}

namespace Storm { namespace Source { namespace SFile {
int SFileDdaEnd(D2MPQFileStrc*);
uint32_t SFileLoadFile(const char*, void**, uint32_t*, uint32_t, OVERLAPPED*);
int32_t SFileOpenFile(const char*, D2MPQFileStrc**);
int SFileReadFile(D2MPQFileStrc*, void*, uint32_t, uint32_t*, OVERLAPPED*);
int SFileCloseFile(D2MPQFileStrc*);
int SFileDdaGetPos(D2MPQFileStrc*, uint32_t*, uint32_t*);
int SFileDdaBeginEx(D2MPQFileStrc*, uint32_t, uint32_t, uint32_t, int, int, IDirectSoundBuffer*);
int SFileDdaDestroy();
uint32_t SFileLoadFileEx(D2ArchiveStrc*, const char*, void**, uint32_t*, uint32_t, uint32_t, OVERLAPPED*);
int SFileOpenFileEx(D2ArchiveStrc*, const char*, uint32_t, void**);
int32_t SFileUnloadFile(void*);
int SFileGetFileName(D2MPQFileStrc*, char*, int);
uint32_t SFileGetFileSize(D2MPQFileStrc*, uint32_t*);
int SFileOpenArchive(const char*, int*, uint32_t, D2ArchiveStrc**);
int SFileReadFileEx2(D2MPQFileStrc*, void*, uint32_t, uint32_t*, OVERLAPPED*, int, D2StormAsyncCompletionStrc*);
int32_t SFileSetBasePath(const char*);
int SFileCloseArchive(D2ArchiveStrc*);
int SFileDdaInitialize(IDirectSound*);
void SFileSetAsyncBudget(uint32_t);
uint32_t SFileSetFilePointer(D2MPQFileStrc*, int, int*, uint32_t);
int SFileSetIoErrorMode(int, Fn4);
void SFileSetDataChunkSize(uint32_t);
int32_t SFileEnableDirectAccess(int);
int SFileAuthenticateArchive(D2ArchiveStrc*, uint32_t*);
void SFileEnableSeekOptimization(int);
void SFILE_CancelAsyncRequestNoReturn(void*);
// The Mac units hold an archive as an opaque handle and the priority as the plain number it is
// (Storm.h declares the priority a pointer, which the shared callers cast around).
inline int SFileOpenArchive(const char* szArchiveName, int nPriority, uint32_t dwFlags, void** phArchive) {
    return SFileOpenArchive((char*)szArchiveName, (int*)(intptr_t)nPriority, dwFlags, (D2ArchiveStrc**)phArchive);
}
inline int SFileCloseArchive(void* hArchive) { return SFileCloseArchive((D2ArchiveStrc*)hArchive); }
}}}


// Entry points of StormMac without a shared caller: global, as the Mac units declare them.
int SBmpLoadImage(const char*, uint8_t*, uint8_t*, uint32_t, uint32_t*, uint32_t*, uint32_t*);
int SFileFileExistsEx(D2ArchiveStrc*, const char*, uint32_t);

// Storm's own code names these unqualified, as the original's source does.
using namespace Storm;
using namespace Storm::WindowHandle;
using namespace Storm::Source;
using namespace Storm::Source::SBig;
using namespace Storm::Source::SBMP;
using namespace Storm::Source::SErr;
using namespace Storm::Source::SFile;
using namespace Storm::Source::SLog;
using namespace Storm::Source::SMem;
using namespace Storm::Source::SReg;
using namespace Storm::Source::SSignature;
using namespace Storm::Source::SSTR;

#endif  // D2_OS_MAC
