#include "eventhorizon_processor.h"
#include "eventhorizon_controller.h"
#include "eventhorizon_cids.h"
#include "version.h"

#include "public.sdk/source/main/pluginfactory.h"

#define stringPluginName "EventHorizon"

using namespace Steinberg::Vst;
using namespace Steinberg;

//------------------------------------------------------------------------
BEGIN_FACTORY_DEF ("Eli Allen", "https://github.com/eliallen", "mailto:dev@null.invalid")

DEF_CLASS2 (INLINE_UID_FROM_FUID (EventHorizon::kEventHorizonProcessorUID),
            PClassInfo::kManyInstances, kVstAudioEffectClass, stringPluginName, Vst::kDistributable,
            Vst::PlugType::kInstrumentSynth, FULL_VERSION_STR, kVstVersionString,
            EventHorizon::EventHorizonProcessor::createInstance)

DEF_CLASS2 (INLINE_UID_FROM_FUID (EventHorizon::kEventHorizonControllerUID),
            PClassInfo::kManyInstances, kVstComponentControllerClass, stringPluginName "Controller",
            0, "", FULL_VERSION_STR, kVstVersionString,
            EventHorizon::EventHorizonController::createInstance)

END_FACTORY
