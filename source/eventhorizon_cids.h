#pragma once

#include "pluginterfaces/base/funknown.h"
#include "pluginterfaces/vst/vsttypes.h"

namespace EventHorizon {

//------------------------------------------------------------------------
static const Steinberg::FUID kEventHorizonProcessorUID (0x83823DA6, 0x53924C22, 0xA70D0CD0,
                                                       0x998CC58F);
static const Steinberg::FUID kEventHorizonControllerUID (0x3FFECEED, 0x88C34DED, 0xBE9757EE,
                                                        0x4E21660A);

//------------------------------------------------------------------------
} // namespace EventHorizon
