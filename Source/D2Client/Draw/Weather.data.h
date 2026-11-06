/**
 * Data defined in D2Client/Draw/Weather.cpp that other translation units name.
 * Binary: Game.exe
 */

#pragma once
#if defined(D2_MODULE_LIBRARIES) && defined(D2_MODULE_STRICT) && !defined(D2_GATE_FROM_ROOT) && !defined(D2_SEES_D2Client)
#error "D2Client header included by a module that may not use D2Client (original DLL import graph; see module-gates.tsv)"
#endif

#include "Source/D2Headers/d2_platform.h"

// Forward declarations
struct D2DataArrayStrc;

extern int gnWeatherCurrentTransitionState;

namespace D2Client::Draw::Weather {
extern int gnWeatherRngRangeMax;
extern int gnWeatherRngRangeMaxAct5;
extern int gnWeatherOffsetXModulo;
extern int gnWeatherParticleOffsetYModulo;
extern int gnWeatherRngRangeMin;
extern int gnWeatherRngRangeMinAct5;
extern D2DataArrayStrc * pEnvironmentSplashes;
extern D2DataArrayStrc * pEnvironmentBubbles;
extern D2DataArrayStrc * pEnvironmentParticles;
} // namespace D2Client::Draw::Weather
