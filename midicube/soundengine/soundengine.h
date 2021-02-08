/*
 * soundengine.h
 *
 *  Created on: 5 Jul 2020
 *      Author: jojo
 */

#ifndef MIDICUBE_SOUNDENGINE_SOUNDENGINE_H_
#define MIDICUBE_SOUNDENGINE_SOUNDENGINE_H_

#include "../midi.h"
#include "../envelope.h"
#include "../audio.h"
#include "../synthesis.h"
#include "../metronome.h"
#include "../looper.h"
#include "../effect/vocoder.h"
#include "../effect/bitcrusher.h"
#include "../property.h"
#include "voice.h"
#include <string>
#include <array>
#include <functional>

#define SOUND_ENGINE_POLYPHONY 30
#define SOUND_ENGINE_MIDI_CHANNELS 16
#define SOUND_ENGINE_COUNT 5

class SoundEngineDevice;

template<typename V>
struct EngineStatus {
	size_t pressed_notes;
	size_t latest_note_index;
	V* latest_note;
};

class SoundEngineChannel;

class SoundEngine {

public:
	virtual void midi_message(MidiMessage& msg, SampleInfo& info) = 0;

	virtual void press_note(SampleInfo& info, unsigned int channel, unsigned int note, double velocity) = 0;

	virtual void release_note(SampleInfo& info, unsigned int channel, unsigned int note) = 0;

	//Process voices
	virtual void process_voices(std::array<double, SOUND_ENGINE_MIDI_CHANNELS>& lsample, std::array<double, SOUND_ENGINE_MIDI_CHANNELS>& rsample, SampleInfo& info,  ssize_t index, std::array<SoundEngineChannel, SOUND_ENGINE_MIDI_CHANNELS>& channels) = 0;

	//(Post-) process each channel
	virtual void process_channel(double& lsample, double& rsample, unsigned int channel, SampleInfo& info) = 0;

	//(Post-) process the entire sound engine
	virtual void process_sample(double& lsample, double& rsample, SampleInfo& info) = 0;

	virtual std::string get_name() = 0;

	virtual ~SoundEngine() {

	};

};

template<typename V, size_t N>
class BaseSoundEngine : public SoundEngine {
private:
	KeyboardEnvironment environment;
	VoiceManager<V, N> voice_mgr;

public:
	std::atomic<unsigned int> sustain_control{64};
	std::atomic<bool> sustain{true};

	void midi_message(MidiMessage& msg, SampleInfo& info);

	void press_note(SampleInfo& info, unsigned int channel, unsigned int note, double velocity);

	void release_note(SampleInfo& info, unsigned int channel, unsigned int note);

	//Override methods
	virtual void process_sample(double& lsample, double& rsample, SampleInfo& info) {

	};

	void process_channel_sample(double& lsample, double& rsample, SampleInfo& info);

	void process_voices(std::array<double, SOUND_ENGINE_MIDI_CHANNELS>& lsample, std::array<double, SOUND_ENGINE_MIDI_CHANNELS>& rsample, SampleInfo& info,  ssize_t index, std::array<SoundEngineChannel, SOUND_ENGINE_MIDI_CHANNELS>& channels);

	//Virtual methods
	virtual void process_voice_sample(double& lsample, double& rsample, SampleInfo& info, V& voice, KeyboardEnvironment& env, size_t note_index) = 0;

	virtual void process_sample(double& lsample, double& rsample, SampleInfo& info, KeyboardEnvironment& env, EngineStatus<V>& status) {

	};

	virtual void control_change(unsigned int channel, unsigned int control, unsigned int value) {

	};

	virtual bool voice_finished(SampleInfo& info, SimpleVoice& voice, KeyboardEnvironment& env, size_t note_index) {
		return !voice.note.pressed || (env.sustain && voice.note.release_time >= env.sustain_time);
	};

	virtual ~BaseSoundEngine() {

	};

};

template<typename V, size_t N>
void BaseSoundEngine<V, N>::midi_message(MidiMessage& message, SampleInfo& info) {
	double pitch;
	switch (message.type) {
		case MessageType::NOTE_ON:
			press_note(info, message.channel, message.note(), message.velocity()/127.0);
			break;
		case MessageType::NOTE_OFF:
			release_note(info, message.channel, message.note());
			break;
		case MessageType::CONTROL_CHANGE:
			control_change(message.channel, message.control(), message.value());
			//Sustain
			if (message.control() == sustain_control) {
				bool new_sustain = message.value() != 0;
				if (new_sustain != environment.sustain) {
					if (new_sustain) {
						environment.sustain_time = info.time;
					}
					else {
						environment.sustain_release_time = info.time;
					}
					environment.sustain = new_sustain;
				}
			}
			break;
		case MessageType::PITCH_BEND:
			pitch = (message.get_pitch_bend()/8192.0 - 1.0) * 2;
			environment.pitch_bend = note_to_freq_transpose(pitch);
			break;
		default:
			break;
	}
}

template<typename V, size_t N>
void BaseSoundEngine<V, N>::press_note(SampleInfo& info, unsigned int channel, unsigned int note, double velocity) {
	this->voice_mgr.press_note(info, channel, note, velocity);
}

template<typename V, size_t N>
void BaseSoundEngine<V, N>::release_note(SampleInfo& info, unsigned int channel, unsigned int note) {
	this->voice_mgr.release_note(info, channel, note);
}

template<typename V, size_t N>
void BaseSoundEngine<V, N>::process_channel_sample(double& lsample, double& rsample, SampleInfo& info) {
	EngineStatus<V> status = {0, 0, nullptr};
	//Notes
	for (size_t i = 0; i < SOUND_ENGINE_POLYPHONY; ++i) {
		TriggeredNote& n = voice_mgr.note[i].note;
		if (n.valid) {
			if (note_finished(info, voice_mgr.note[i], environment, i)) {
				n.valid = false;
			}
			else {
				++status.pressed_notes; //TODO might cause problems in the future
				n.phase_shift += (environment.pitch_bend - 1) * info.time_step;
				process_note_sample(lsample, rsample, info, voice_mgr.note[i], environment, i);
				if (!status.latest_note || status.latest_note->note.start_time < n.start_time) {
					status.latest_note = &voice_mgr.note[i];
					status.latest_note_index = i;
				}
			}
		}
	}
	//Static sample
	process_sample(lsample, rsample, info, environment, status);
}

template <typename T>
std::string get_engine_name();

struct ChannelSource {
	ssize_t input = -1;
	unsigned int channel = 0;
	unsigned int start_note = 0;
	unsigned int end_note = 127;
	unsigned int start_velocity = 0;
	unsigned int end_velocity = 127;
	int octave = 0;
	bool transfer_channel_aftertouch = true;
	bool transfer_pitch_bend = true;
	bool transfer_cc = true;
	bool transfer_prog_change = true;
	bool transfer_other = true;
};

enum SoundEngineChannelProperty {
	pChannelActive,
	pChannelVolume,
	pChannelPanning,
	pChannelSoundEngine,

	pChannelInputDevice,
	pChannelInputChannel,
	pChannelStartNote,
	pChannelEndNote,
	pChannelStartVelocity,
	pChannelEndVelocity,
	pChannelOctave,
	pChannelTransferChannelAftertouch,
	pChannelTransferPitchBend,
	pChannelTransferCC,
	pChannelTransferProgChange,
	pChannelTransferOther,

	pArpeggiatorOn,
	pArpeggiatorPattern,
	pArpeggiatorOctaves,
	pArpeggiatorStep,
	pArpeggiatorHold,
	pArpeggiatorBPM
};

class SoundEngineChannel : public PropertyHolder {
public:
	ssize_t engine_index{0};

	double volume{0.3};
	bool active{false};
	double panning = 0;
	ChannelSource source;
	Arpeggiator arp;
	Looper looper;

	//Effects
	VocoderPreset vocoder_preset;
	VocoderEffect vocoder;
	BitCrusherPreset bitcrusher_preset;
	BitCrusherEffect bitcrusher;

	SoundEngineChannel();

	void send(MidiMessage& message, SampleInfo& info, SoundEngine& engine);

	void process_sample(double& lsample, double& rsample, SampleInfo& info, unsigned int channel, Metronome& metronome, SoundEngine* engine);

	PropertyValue get(size_t prop, size_t sub_prop);

	void set(size_t prop, PropertyValue value, size_t sub_prop);

	void update_properties();

	/**
	 * May only be called from GUI thread after GUI has started
	 */
	void set_engine(ssize_t engine);

	ssize_t get_engine();

	~SoundEngineChannel();

};

enum SoundEngineProperty {
	pEngineMetronomeOn,
	pEngineMetronomeBPM
};

class SoundEngineDevice : public PropertyHolder {

private:
	ADSREnvelopeData metronome_env_data{0.0005, 0.02, 0, 0};
	ADSREnvelope metronome_env;

public:
	Metronome metronome;
	std::atomic<bool> play_metronome{false};
	std::array<SoundEngineChannel, SOUND_ENGINE_MIDI_CHANNELS> channels;

	SoundEngineDevice();

	std::array<SoundEngine*, SOUND_ENGINE_COUNT> sound_engines;

	void send(MidiMessage& message, SampleInfo& info);

	void process_sample(double& lsample, double& rsample, SampleInfo& info);

	void solo (unsigned int channel);

	PropertyValue get(size_t prop, size_t sub_prop = 0);

	void set(size_t prop, PropertyValue value, size_t sub_prop = 0);

	void update_properties();

	~SoundEngineDevice();

};




#endif /* MIDICUBE_SOUNDENGINE_SOUNDENGINE_H_ */
