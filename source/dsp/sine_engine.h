#pragma once

#include <cmath>
#include <cstdint>

namespace EventHorizon {

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
		bool releasing {false};
		int32_t noteId {-1};
		int16_t pitch {0};
		double phase {0.0};
		double phaseIncrement {0.0};
		float gain {0.f};
		float envelope {0.f};
		float releaseStep {0.f};
	};

	static constexpr double kReleaseTimeSec = 0.035;

	static double midiNoteToHz (int16_t midiNote, float tuningCents);
	Voice* allocateVoice ();
	void beginRelease (Voice& voice);

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
inline void SineEngine::beginRelease (Voice& voice)
{
	voice.releasing = true;
	voice.releaseStep = static_cast<float> (1.0 / (sampleRate * kReleaseTimeSec));
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
				voice.releasing = false;
				voice.envelope = 1.f;
				return;
			}
		}
	}

	Voice* voice = allocateVoice ();
	voice->active = true;
	voice->releasing = false;
	voice->noteId = noteId;
	voice->pitch = pitch;
	voice->phase = 0.0;
	const double hz = midiNoteToHz (pitch, tuningCents);
	voice->phaseIncrement = hz / sampleRate;
	voice->gain = velocity;
	voice->envelope = 1.f;
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
				if (!voice.releasing)
					beginRelease (voice);
				return;
			}
		}
	}

	for (auto& voice : voices)
	{
		if (voice.active && voice.pitch == pitch)
		{
			if (!voice.releasing)
				beginRelease (voice);
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
		for (auto& voice : voices)
		{
			if (!voice.active)
				continue;

			sample += static_cast<float> (std::sin (kTwoPi * voice.phase)) * voice.gain *
			          voice.envelope;

			double phase = voice.phase + voice.phaseIncrement;
			if (phase >= 1.0)
				phase -= 1.0;
			voice.phase = phase;

			if (voice.releasing)
			{
				voice.envelope -= voice.releaseStep;
				if (voice.envelope <= 0.f)
				{
					voice.active = false;
					voice.releasing = false;
					voice.envelope = 0.f;
				}
			}
		}
		sample *= kMasterGain;
		outL[i] = sample;
		outR[i] = sample;
	}
}

//------------------------------------------------------------------------
} // namespace EventHorizon
