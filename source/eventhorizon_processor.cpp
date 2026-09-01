#include "eventhorizon_processor.h"
#include "eventhorizon_cids.h"

#include <cstring>

using namespace Steinberg;

namespace EventHorizon {

//------------------------------------------------------------------------
EventHorizonProcessor::EventHorizonProcessor ()
{
	setControllerClass (kEventHorizonControllerUID);
}

//------------------------------------------------------------------------
tresult PLUGIN_API EventHorizonProcessor::initialize (FUnknown* context)
{
	tresult result = AudioEffect::initialize (context);
	if (result != kResultOk)
		return result;

	// Instrument: stereo audio out + MIDI/event in (no audio input).
	addAudioOutput (STR16 ("Stereo Out"), Vst::SpeakerArr::kStereo);
	addEventInput (STR16 ("Event In"), 1);

	return kResultOk;
}

//------------------------------------------------------------------------
tresult PLUGIN_API EventHorizonProcessor::setActive (TBool state)
{
	return AudioEffect::setActive (state);
}

//------------------------------------------------------------------------
tresult PLUGIN_API EventHorizonProcessor::process (Vst::ProcessData& data)
{
	if (data.numOutputs < 1 || data.outputs == nullptr || data.numSamples <= 0)
		return kResultOk;

	Vst::AudioBusBuffers& out = data.outputs[0];
	const int32 numChannels = out.numChannels;

	if (data.symbolicSampleSize == Vst::kSample32 && out.channelBuffers32)
	{
		for (int32 ch = 0; ch < numChannels; ++ch)
		{
			if (out.channelBuffers32[ch])
				std::memset (out.channelBuffers32[ch], 0,
				             static_cast<size_t> (data.numSamples) * sizeof (Vst::Sample32));
		}
	}
	else if (data.symbolicSampleSize == Vst::kSample64 && out.channelBuffers64)
	{
		for (int32 ch = 0; ch < numChannels; ++ch)
		{
			if (out.channelBuffers64[ch])
				std::memset (out.channelBuffers64[ch], 0,
				             static_cast<size_t> (data.numSamples) * sizeof (Vst::Sample64));
		}
	}

	if (numChannels > 0)
		out.silenceFlags = (static_cast<uint64> (1) << numChannels) - 1;

	return kResultOk;
}

//------------------------------------------------------------------------
tresult PLUGIN_API EventHorizonProcessor::setupProcessing (Vst::ProcessSetup& newSetup)
{
	return AudioEffect::setupProcessing (newSetup);
}

//------------------------------------------------------------------------
tresult PLUGIN_API EventHorizonProcessor::canProcessSampleSize (int32 symbolicSampleSize)
{
	if (symbolicSampleSize == Vst::kSample32)
		return kResultTrue;
	return kResultFalse;
}

//------------------------------------------------------------------------
tresult PLUGIN_API EventHorizonProcessor::setBusArrangements (Vst::SpeakerArrangement* inputs,
                                                             int32 numIns,
                                                             Vst::SpeakerArrangement* outputs,
                                                             int32 numOuts)
{
	// Stereo instrument: no audio inputs, one stereo output.
	if (numIns == 0 && numOuts == 1 && outputs && outputs[0] == Vst::SpeakerArr::kStereo)
		return AudioEffect::setBusArrangements (inputs, numIns, outputs, numOuts);
	return kResultFalse;
}

//------------------------------------------------------------------------
tresult PLUGIN_API EventHorizonProcessor::setState (IBStream* /*state*/)
{
	return kResultOk;
}

//------------------------------------------------------------------------
tresult PLUGIN_API EventHorizonProcessor::getState (IBStream* /*state*/)
{
	return kResultOk;
}

//------------------------------------------------------------------------
} // namespace EventHorizon
