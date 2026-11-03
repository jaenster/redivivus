/**
 * Data defined in Fog/File.cpp that other translation units name.
 * Binary: Game.exe
 */

#pragma once

#include "d2_platform.h"

// Forward declarations
struct D2CrashReportStrc;
typedef int (__fastcall *pfnFogExceptionFilter)(D2CrashReportStrc * pExceptionReport, void * pUserContext);

extern char gdwFileReadPosition[3];
extern char gszCrashDumpRegisterFormatString[4];
extern pfnFogExceptionFilter gpCurrentExceptionFilter;
extern void * gpCurrentExceptionContext;
