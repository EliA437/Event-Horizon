#pragma once

#include <cmath>
#include <cstdint>

namespace EventHorizon {

//------------------------------------------------------------------------
// Equal-temperament sine voices (A4 = 440 Hz, MIDI note 69), matching Serum / VST MIDI.
//------------------------------------------------------------------------
class SineEngine
{
public:
	static constexpr int kMaxVoices = 16;
	static constexpr double kA4Hz = 440.0;
	static constexpr int kA4MidiNote = 69;

	void setSampleRate (double sampleRate);
	void reset ();
	void noteOn (int16_t pitch, int32_t noteId, float velocity, float tuningCents = 0.f);
	void noteOff (int16_t pitch, int32_t noteId);
	void process (float* outL, float* outR, int32_t numSamples);

private:
	struct Voice
	{
		bool active {false};
		int32_t noteId {-1};
		int16_t pitch {0};
		double phase {0.0};
		double phaseIncrement {0.0};
		float gain {0.f};
	};

	static double midiNoteToHz (int16_t midiNote, float tuningCents);
	Voice* allocateVoice ();

	Voice voices[kMaxVoices] {};
	double sampleRate {44100.0};
};

//------------------------------------------------------------------------
inline double SineEngine::midiNoteToHz (int16_t midiNote, float tuningCents)
{
	const double note = static_cast<double> (midiNote) + static_cast<double> (tuningCents) / 100.0;
	return kA4Hz * std::pow (2.0, (note - static_cast<double> (kA4MidiNote)) / 12.0);
}

//------------------------------------------------------------------------
inline void SineEngine::setSampleRate (double rate)
{
	sampleRate = rate > 0.0 ? rate : 44100.0;
}

//------------------------------------------------------------------------
inline void SineEngine::reset ()
{
	for (auto& voice : voices)
		voice = {};
}

//------------------------------------------------------------------------
inline SineEngine::Voice* SineEngine::allocateVoice ()
{
	for (auto& voice : voices)
	{
		if (!voice.active)
			return &voice;
	}
	return &voices[0];
}

//------------------------------------------------------------------------
inline void SineEngine::noteOn (int16_t pitch, int32_t noteId, float velocity, float tuningCents)
{
	if (pitch < 0 || pitch > 127)
		return;

	if (noteId >= 0)
	{
		for (auto& voice : voices)
		{
			if (voice.active && voice.noteId == noteId)
			{
				voice.pitch = pitch;
				const double hz = midiNoteToHz (pitch, tuningCents);
				voice.phaseIncrement = hz / sampleRate;
				voice.gain = velocity;
				return;
			}
		}
	}

	Voice* voice = allocateVoice ();
	voice->active = true;
	voice->noteId = noteId;
	voice->pitch = pitch;
	voice->phase = 0.0;
	const double hz = midiNoteToHz (pitch, tuningCents);
	voice->phaseIncrement = hz / sampleRate;
	voice->gain = velocity;
}

//------------------------------------------------------------------------
inline void SineEngine::noteOff (int16_t pitch, int32_t noteId)
{
	if (noteId >= 0)
	{
		for (auto& voice : voices)
		{
			if (voice.active && voice.noteId == noteId)
			{
				voice.active = false;
				return;
			}
		}
	}

	for (auto& voice : voices)
	{
		if (voice.active && voice.pitch == pitch)
		{
			voice.active = false;
			return;
		}
	}
}

//------------------------------------------------------------------------
inline void SineEngine::process (float* outL, float* outR, int32_t numSamples)
{
	constexpr float kMasterGain = 0.2f;
	constexpr double kTwoPi = 6.283185307179586;

	for (int32_t i = 0; i < numSamples; ++i)
	{
		float sample = 0.f;
		for (const auto& voice : voices)
		{
			if (!voice.active)
				continue;
			sample += static_cast<float> (std::sin (kTwoPi * voice.phase)) * voice.gain;
			double phase = voice.phase + voice.phaseIncrement;
			if (phase >= 1.0)
				phase -= 1.0;
			const_cast<Voice&> (voice).phase = phase;
		}
		sample *= kMasterGain;
		outL[i] = sample;
		outR[i] = sample;
	}
}

//------------------------------------------------------------------------
} // namespace EventHorizon
