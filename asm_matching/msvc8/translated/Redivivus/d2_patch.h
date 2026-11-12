#pragma once
#include <cstddef>

// Runtime patches applied to the reconstruction from OUR side, so the generated
// engine tree stays pure generator output. See d2_patch.cpp for the rationale.

// Rewrite EVERY direct `CALL pOldCallee` in the module's .text to call
// pNewCallee instead. Returns the number of sites rewritten, or -1 if the count
// did not match nExpectedSites, in which case NOTHING is written.
//
// Pass nExpectedSites < 0 to accept whatever is found (and have it logged).
int D2PATCH_RedirectAllCalls(void* pOldCallee, void* pNewCallee, int nExpectedSites);

// Install every registered patch. Idempotent; safe to call from the pump.
void D2PATCH_InstallAll(void);
