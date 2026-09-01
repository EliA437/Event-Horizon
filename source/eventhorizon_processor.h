#pragma once

#include "dsp/sine_engine.h"
#include "pluginterfaces/vst/ivstevents.h"
#include "public.sdk/source/vst/utility/ringbuffer.h"
#include "public.sdk/source/vst/vstaudioeffect.h"

namespace EventHorizon {

//------------------------------------------------------------------------
class EventHorizonProcessor : public Steinberg::Vst::AudioEffect
{
public:
	EventHorizonProcessor ();
	~EventHorizonProcessor () SMTG_OVERRIDE = default;

	static Steinberg::FUnknown* createInstance (void* /*context*/)
	{
		return (Steinberg::Vst::IAudioProcessor*)new EventHorizonProcessor;
	}

	Steinberg::tresult PLUGIN_API initialize (Steinberg::FUnknown* context) SMTG_OVERRIDE;
	Steinberg::tresult PLUGIN_API setActive (Steinberg::TBool state) SMTG_OVERRIDE;
	Steinberg::tresult PLUGIN_API process (Steinberg::Vst::ProcessData& data) SMTG_OVERRIDE;
	Steinberg::tresult PLUGIN_API setupProcessing (Steinberg::Vst::ProcessSetup& newSetup)
	    SMTG_OVERRIDE;
	Steinberg::tresult PLUGIN_API canProcessSampleSize (Steinberg::int32 symbolicSampleSize)
	    SMTG_OVERRIDE;
	Steinberg::tresult PLUGIN_API setBusArrangements (Steinberg::Vst::SpeakerArrangement* inputs,
	                                                  Steinberg::int32 numIns,
	                                                  Steinberg::Vst::SpeakerArrangement* outputs,
	                                                  Steinberg::int32 numOuts) SMTG_OVERRIDE;
	Steinberg::tresult PLUGIN_API notify (Steinberg::Vst::IMessage* message) SMTG_OVERRIDE;
	Steinberg::tresult PLUGIN_API setState (Steinberg::IBStream* state) SMTG_OVERRIDE;
	Steinberg::tresult PLUGIN_API getState (Steinberg::IBStream* state) SMTG_OVERRIDE;

private:
	void handleEvent (const Steinberg::Vst::Event& evt);
	void processUiEvents ();
	void processInputEvents (Steinberg::Vst::IEventList* events);

	SineEngine sineEngine;
	Steinberg::OneReaderOneWriter::RingBuffer<Steinberg::Vst::Event> uiEvents {64};
};

//------------------------------------------------------------------------
} // namespace EventHorizon
