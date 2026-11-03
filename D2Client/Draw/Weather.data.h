/**
 * Data defined in D2Client/Draw/Weather.cpp that other translation units name.
 * Binary: Game.exe
 */

#pragma once

#include "d2_platform.h"

// Forward declarations
struct D2DataArrayStrc;

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
