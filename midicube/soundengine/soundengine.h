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
#include "../device.h"
#include "../synthesis.h"
#include <string>
#include <array>

#define SOUND_ENGINE_POLYPHONY 30


class SoundEngine {

public:
	virtual void process_note_sample(std::array<double, OUTPUT_CHANNELS>& channels, SampleInfo& info, TriggeredNote& note) = 0;

	virtual void process_sample(std::array<double, OUTPUT_CHANNELS>& channels, SampleInfo& info) {

	};

	virtual void control_change(unsigned int control, unsigned int value) {

	};

	virtual bool note_finished(SampleInfo& info, TriggeredNote& note) {
		return !note.pressed;
	}

	virtual std::string get_name() = 0;

	virtual ~SoundEngine() {

	}

};

class SoundEngineDevice : public AudioDevice {

private:
	std::string identifier;
	std::array<TriggeredNote, SOUND_ENGINE_POLYPHONY> note;
	double pitch_bend = 0;
	SoundEngine* engine;

	size_t next_freq_slot();

public:

	AudioHandler* handler = nullptr; //TODO remove this reference and pass time through send() to be thread-safe

	SoundEngineDevice(SoundEngine* engine, std::string identifier);

	std::string get_identifier();

	bool is_audio_input() {
		return true;
	}

	void send(MidiMessage& message);

	void process_sample(std::array<double, OUTPUT_CHANNELS>& channels, SampleInfo& info);

	~SoundEngineDevice();

};




#endif /* MIDICUBE_SOUNDENGINE_SOUNDENGINE_H_ */
