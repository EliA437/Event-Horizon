#include "eventhorizon_processor.h"
#include "eventhorizon_cids.h"
#include "eventhorizon_messaging.h"

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

	addAudioOutput (STR16 ("Stereo Out"), Vst::SpeakerArr::kStereo);
	addEventInput (STR16 ("Event In"), 1);

	return kResultOk;
}

//------------------------------------------------------------------------
tresult PLUGIN_API EventHorizonProcessor::setActive (TBool state)
{
	if (!state)
		sineEngine.reset ();
	return AudioEffect::setActive (state);
}

//------------------------------------------------------------------------
void EventHorizonProcessor::handleEvent (const Vst::Event& evt)
{
	switch (evt.type)
	{
		case Vst::Event::EventTypes::kNoteOnEvent:
		{
			if (evt.noteOn.velocity <= 0.f)
			{
				sineEngine.noteOff (evt.noteOn.pitch, evt.noteOn.noteId);
				break;
			}
			sineEngine.noteOn (evt.noteOn.pitch, evt.noteOn.noteId, evt.noteOn.velocity,
			                   evt.noteOn.tuning);
			break;
		}
		case Vst::Event::EventTypes::kNoteOffEvent:
			sineEngine.noteOff (evt.noteOff.pitch, evt.noteOff.noteId);
			break;
		default:
			break;
	}
}

//------------------------------------------------------------------------
void EventHorizonProcessor::processUiEvents ()
{
	Vst::Event evt {};
	while (uiEvents.pop (evt))
		handleEvent (evt);
}

//------------------------------------------------------------------------
void EventHorizonProcessor::processInputEvents (Vst::IEventList* events)
{
	if (!events)
		return;
	const int32 numEvents = events->getEventCount ();
	for (int32 i = 0; i < numEvents; ++i)
	{
		Vst::Event evt {};
		if (events->getEvent (i, evt) == kResultOk)
			handleEvent (evt);
	}
}

//------------------------------------------------------------------------
tresult PLUGIN_API EventHorizonProcessor::process (Vst::ProcessData& data)
{
	processUiEvents ();
	if (data.inputEvents)
		processInputEvents (data.inputEvents);

	if (data.numOutputs < 1 || data.outputs == nullptr || data.numSamples <= 0)
		return kResultOk;

	Vst::AudioBusBuffers& out = data.outputs[0];
	const int32 numChannels = out.numChannels;
	if (numChannels < 1)
		return kResultOk;

	if (data.symbolicSampleSize == Vst::kSample32 && out.channelBuffers32)
	{
		float* left = out.channelBuffers32[0];
		float* right = numChannels > 1 ? out.channelBuffers32[1] : left;
		std::memset (left, 0, static_cast<size_t> (data.numSamples) * sizeof (Vst::Sample32));
		if (right != left)
			std::memset (right, 0, static_cast<size_t> (data.numSamples) * sizeof (Vst::Sample32));
		sineEngine.process (left, right, data.numSamples);
		out.silenceFlags = 0;
	}
	else if (data.symbolicSampleSize == Vst::kSample64 && out.channelBuffers64)
	{
		for (int32 ch = 0; ch < numChannels; ++ch)
		{
			if (out.channelBuffers64[ch])
				std::memset (out.channelBuffers64[ch], 0,
				             static_cast<size_t> (data.numSamples) * sizeof (Vst::Sample64));
		}
		// Engine renders float; convert path omitted until 64-bit output is enabled.
		out.silenceFlags = (static_cast<uint64> (1) << numChannels) - 1;
	}

	return kResultOk;
}

//------------------------------------------------------------------------
tresult PLUGIN_API EventHorizonProcessor::notify (Vst::IMessage* message)
{
	if (!message)
		return kResultFalse;

	if (strcmp (message->getMessageID (), MsgIDEvent) != 0)
		return kResultFalse;

	if (auto attr = message->getAttributes ())
	{
		const void* msgData = nullptr;
		uint32 msgSize = 0;
		if (attr->getBinary (MsgIDEvent, msgData, msgSize) == kResultTrue &&
		    msgSize == sizeof (Vst::Event))
		{
			auto evt = *reinterpret_cast<const Vst::Event*> (msgData);
			uiEvents.push (evt);
		}
	}
	return kResultTrue;
}

//------------------------------------------------------------------------
tresult PLUGIN_API EventHorizonProcessor::setupProcessing (Vst::ProcessSetup& newSetup)
{
	sineEngine.setSampleRate (newSetup.sampleRate);
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
