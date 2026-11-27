// MACSEC.cpp - the Mac code-signature check: whether a file is signed by Blizzard
// Entertainment, Inc. (SecStaticCode for the validity, the signing certificate's subject for the
// organisation), and the fault-handler list's static initialiser that precedes it.
//
// 1.14d Intel Mac (DiabloII_macho) 002b70ca..002b78f1. The binary's file-map puts this in
// D2Hell/SRC/Archive.cpp's tail (the run after ARCHIVE_LoadMPQ); the Windows build compiles that
// file, so the Mac bodies sit in a file of their own.
//
// Mac-only: the guard is spelled `#if !defined(_WIN32) && D2_OS_MAC`.
#include "D2Arch.h"

#if !defined(_WIN32) && D2_OS_MAC

#include <Carbon/Carbon.h>
#include <Security/Security.h>
#include <dlfcn.h>
#include <stdint.h>
#include <string.h>
#include <strings.h>

// The 10.7 SDK headers do not declare it; flag 2 asks for the signing information.
extern "C" OSStatus SecCodeCopySigningInformation(SecStaticCodeRef pCode, uint32_t nFlags, CFDictionaryRef* ppInformation);

extern "C" int __cxa_atexit(void (*pfnDestructor)(void*), void* pObject, void* pDso);
extern "C" void* __dso_handle;

// The head of Storm's intrusive list as the fault-handler list lays it out.
struct MacFaultHandlerList {
    int nLinkOffset;   // 0x00
    int pPrevLink;     // 0x04: the previous link, or the terminator
    int pNext;         // 0x08: the next element; at the end the terminator's address | 1
};

// 1.14d mac: 002b712c
// TSLink::NextLink: the next link, or the terminator at the end of a list.
unsigned int TSLINK_NextLink(int* pLink, int nLinkOffset) {
    unsigned int pNext = pLink[1];
    if ((pNext == 0) | (pNext & 1)) {
        pNext = pNext & 0xfffffffe;
    } else {
        if (nLinkOffset < 0) {
            nLinkOffset = (int)pLink - *(int*)(*pLink + 4);
        }
        pNext = pNext + nLinkOffset;
    }
    return pNext;
}

// 1.14d mac: 002b70ec
// TSLink::Unlink.
void TSLINK_Unlink(int* pLink) {
    int pPrevLink = *pLink;
    if (pPrevLink != 0) {
        int* pNextLink = (int*)TSLINK_NextLink(pLink, -1);
        *pNextLink = pPrevLink;
        *(int*)(*pLink + 4) = pLink[1];
        *pLink = 0;
        pLink[1] = 0;
    }
}

// 1.14d mac: 002b7162
// Unlinks every node: the node the head's next link refers to, until there is none.
void FUN_002b7162(int* pList) {
    unsigned int pNode = pList[2];
    while (true) {
        if ((pNode == 0) | (pNode & 1)) {
            pNode = 0;
        }
        if (pNode == 0) {
            break;
        }
        TSLINK_Unlink((int*)(pNode + *pList));
        pNode = pList[2];
    }
}

// 1.14d mac: 002b70ca
// The list's destructor: unlink all, then the head link.
void TSLIST_Destructor(int pList) {
    FUN_002b7162((int*)pList);
    TSLINK_Unlink((int*)(pList + 4));
}

// 1.14d mac: 005493f4
static MacFaultHandlerList gFaultHandlerList;

// 1.14d mac: 002b71b0
// The static initialiser of gFaultHandlerList.
void TSLIST_InitFaultHandlerList(void) {
    gFaultHandlerList.nLinkOffset = 0;
    gFaultHandlerList.pPrevLink = (int)&gFaultHandlerList.pPrevLink;
    gFaultHandlerList.pNext = (int)&gFaultHandlerList.pPrevLink | 1;
    __cxa_atexit((void (*)(void*))TSLIST_Destructor, &gFaultHandlerList, &__dso_handle);
}

// 1.14d mac: 00549431
static bool s_bModernTrust;
// 1.14d mac: 00549432
static bool s_bOSChecked;

// 1.14d mac: 002b721d
// Copies the organisation name of the leaf signing certificate of the code at the path
// into the buffer.
bool MACSEC_GetSigningOrganizationName(const char* szPath, char* szOrgName, unsigned int nBufferSize) {
    if (szPath == 0 || *szPath == '\0') {
        return false;
    }
    size_t nLength = strlen(szPath);
    CFAllocatorRef pAllocator = kCFAllocatorDefault;
    SecCertificateRef pCertificate = 0;
    CFURLRef pUrl = CFURLCreateFromFileSystemRepresentation(pAllocator, (const UInt8*)szPath, nLength, 0);
    if (pUrl != 0) {
        if (!s_bOSChecked) {
            s_bOSChecked = true;
            SInt32 nOSMajor = 0;
            OSErr nMajorResult = Gestalt(gestaltSystemVersionMajor, &nOSMajor);
            SInt32 nOSMinor = 0;
            OSErr nMinorResult = Gestalt(gestaltSystemVersionMinor, &nOSMinor);
            if ((nMinorResult == 0 && nMajorResult == 0) && (10 < nOSMajor || (nOSMajor == 10 && 6 < nOSMinor))) {
                s_bModernTrust = true;
            }
        }
        SecStaticCodeRef pStaticCode = 0;
        CFDictionaryRef pSigningInfo = 0;
        pCertificate = 0;
        OSStatus nStatus = SecStaticCodeCreateWithPath(pUrl, 0, &pStaticCode);
        if (nStatus == 0 && pStaticCode != 0 &&
            (nStatus = SecCodeCopySigningInformation(pStaticCode, 2, &pSigningInfo), nStatus == 0)) {
            pCertificate = 0;
            if (pSigningInfo != 0) {
                SecTrustRef pTrust = (SecTrustRef)CFDictionaryGetValue(pSigningInfo, CFSTR("trust"));
                pCertificate = 0;
                if (pTrust != 0) {
                    if (s_bModernTrust) {
                        SecTrustResultType nTrustResult;
                        if (SecTrustEvaluate(pTrust, &nTrustResult) == 0 && (nTrustResult == 1 || nTrustResult == 4) &&
                            SecTrustGetCertificateCount(pTrust) > 0) {
                            SecCertificateRef pLeaf = SecTrustGetCertificateAtIndex(pTrust, 0);
                            if (pLeaf != 0) {
                                CFRetain(pLeaf);
                                pCertificate = pLeaf;
                            }
                        }
                    } else {
                        SecTrustResultType nTrustResultLegacy;
                        CFArrayRef pCertChain;
                        CSSM_TP_APPLE_EVIDENCE_INFO* pStatusChain;
                        OSStatus nResult = SecTrustGetResult(pTrust, &nTrustResultLegacy, &pCertChain, &pStatusChain);
                        if (pCertChain != 0) {
                            pCertificate = 0;
                            if (nResult == 0 && (nTrustResultLegacy == 1 || nTrustResultLegacy == 4) &&
                                CFArrayGetCount(pCertChain) > 0) {
                                pCertificate = (SecCertificateRef)CFArrayGetValueAtIndex(pCertChain, 0);
                                if (pCertificate != 0) {
                                    CFRetain(pCertificate);
                                }
                            }
                            CFRelease(pCertChain);
                        }
                    }
                }
            }
        }
        if (pSigningInfo != 0) {
            CFRelease(pSigningInfo);
        }
        if (pStaticCode != 0) {
            CFRelease(pStaticCode);
        }
        CFRelease(pUrl);
    }
    bool bResult = false;
    if (pCertificate != 0) {
        CFStringRef szOrganisation = 0;
        void* pSecurity = dlopen("/System/Library/Frameworks/Security.framework/Versions/Current/Security", RTLD_NOLOAD | RTLD_LAZY | RTLD_LOCAL);
        typedef CFDictionaryRef (*SecCertificateCopyValuesFn)(SecCertificateRef, CFArrayRef, CFErrorRef*);
        SecCertificateCopyValuesFn pfnCopyValues = 0;
        if (pSecurity != 0) {
            pfnCopyValues = (SecCertificateCopyValuesFn)dlsym(pSecurity, "SecCertificateCopyValues");
        }
        if (pSecurity == 0 || pfnCopyValues == 0) {
            const CSSM_DATA* pSubject = 0;
            if (SecCertificateGetSubject(pCertificate, (const CSSM_X509_NAME**)&pSubject) == 0) {
                // The legacy path walks the CSSM RDN list for the OrganizationName OID.
                const CSSM_X509_NAME* pName = (const CSSM_X509_NAME*)pSubject;
                if (pName != 0 && pName->numberOfRDNs != 0) {
                    for (uint32_t i = 0; i < pName->numberOfRDNs; i++) {
                        const CSSM_X509_RDN* pRdn = &pName->RelativeDistinguishedName[i];
                        if (pRdn->numberOfPairs == 0) {
                            continue;
                        }
                        const CSSM_X509_TYPE_VALUE_PAIR* pPair = pRdn->AttributeTypeAndValue;
                        if (pPair->type.Length == CSSMOID_OrganizationName.Length &&
                            memcmp(pPair->type.Data, CSSMOID_OrganizationName.Data, CSSMOID_OrganizationName.Length) == 0) {
                            if (pPair->valueType == 12) {
                                szOrganisation = CFStringCreateWithBytes(pAllocator, pPair->value.Data, pPair->value.Length,
                                                                         kCFStringEncodingUTF8, 0);
                            }
                            break;
                        }
                    }
                }
            }
        } else {
            CFTypeRef pKey = kSecOIDX509V1SubjectName;
            CFArrayRef pKeys = CFArrayCreate(pAllocator, &pKey, 1, 0);
            CFDictionaryRef pValues = pfnCopyValues(pCertificate, pKeys, 0);
            CFDictionaryRef pSubjectDict = (CFDictionaryRef)CFDictionaryGetValue(pValues, kSecOIDX509V1SubjectName);
            CFArrayRef pRdns = (CFArrayRef)CFDictionaryGetValue(pSubjectDict, kSecPropertyKeyValue);
            CFIndex nRdns = CFArrayGetCount(pRdns);
            CFIndex iRdn = 0;
            while (true) {
                if (nRdns <= iRdn) {
                    szOrganisation = 0;
                    break;
                }
                CFDictionaryRef pRdn = (CFDictionaryRef)CFArrayGetValueAtIndex(pRdns, iRdn);
                CFTypeRef pLabel = CFDictionaryGetValue(pRdn, kSecPropertyKeyLabel);
                iRdn++;
                if (CFEqual(pLabel, kSecOIDOrganizationName)) {
                    szOrganisation = (CFStringRef)CFDictionaryGetValue(pRdn, kSecPropertyKeyValue);
                    CFRetain(szOrganisation);
                    break;
                }
            }
            CFRelease(pValues);
        }
        if (szOrganisation != 0) {
            CFIndex nChars = CFStringGetLength(szOrganisation);
            CFIndex nBytesUsed;
            CFStringGetBytes(szOrganisation, CFRangeMake(0, nChars), kCFStringEncodingUTF8, '?', false,
                             (UInt8*)szOrgName, nBufferSize, &nBytesUsed);
            unsigned int nEnd = nBufferSize - 1;
            if ((unsigned int)nBytesUsed < nBufferSize) {
                nEnd = nBytesUsed;
            }
            szOrgName[nEnd] = 0;
            CFRelease(szOrganisation);
            bResult = true;
        }
        CFRelease(pCertificate);
    }
    return bResult;
}

// 1.14d mac: 002b7733
// SecStaticCodeCheckValidityWithErrors on the path, as a status 0..4: 0 valid, 1 unsigned,
// 2 modified, 3 not trusted, 4 anything else.
int MACSEC_CheckCodeSignature(const char* szPath) {
    int nResult = 4;
    if (szPath == 0) {
        return 4;
    }
    if (*szPath == '\0') {
        return 4;
    }
    size_t nLength = strlen(szPath);
    CFURLRef pUrl = CFURLCreateFromFileSystemRepresentation(kCFAllocatorDefault, (const UInt8*)szPath, nLength, 0);
    if (pUrl == 0) {
        return 4;
    }
    SecStaticCodeRef pStaticCode = 0;
    CFTypeRef pToRelease = pUrl;
    if (SecStaticCodeCreateWithPath(pUrl, 0, &pStaticCode) == 0 && pStaticCode != 0) {
        unsigned int nStatus = SecStaticCodeCheckValidityWithErrors(pStaticCode, 6, 0, 0);
        switch (nStatus) {
        case 0:
            nResult = 0;
            break;
        case 0x8001212a:
        case 0x80012436:
            nResult = 3;
            break;
        case 0xfffefa1b:
        case 0xfffefa0b:
            nResult = 2;
            break;
        case 0xfffefa0a:
            nResult = 1;
            break;
        default:
            nResult = 4;
            break;
        }
        CFRelease(pUrl);
        pToRelease = pStaticCode;
    }
    CFRelease(pToRelease);
    return nResult;
}

// 1.14d mac: 002b7854
// True when the signature is valid and the organisation is Blizzard Entertainment, Inc.
bool MACSEC_IsBlizzardSigned(const char* szPath) {
    char szOrgName[512];
    bool bResult = false;
    if (MACSEC_CheckCodeSignature(szPath) == 0) {
        if (MACSEC_GetSigningOrganizationName(szPath, szOrgName, 0x200)) {
            bResult = strncasecmp(szOrgName, "Blizzard Entertainment, Inc.", 0x200) == 0;
        }
    }
    return bResult;
}

#endif
