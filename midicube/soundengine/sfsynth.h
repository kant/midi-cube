/*
 * sfsynth.h
 *
 *  Created on: 21 Oct 2020
 *      Author: jojo
 */

#ifndef MIDICUBE_SOUNDENGINE_SFSYNTH_H_
#define MIDICUBE_SOUNDENGINE_SFSYNTH_H_

#include "soundengine.h"
#include <fluidsynth.h>

class SoundFontSynth : public SoundEngine {

private:
	fluid_settings_t* settings = nullptr;
	fluid_synth_t* synth = nullptr;

public:
	SoundFontSynth();

	void midi_message(MidiMessage& msg, SampleInfo& info);

	void press_note(SampleInfo& info, unsigned int channel, unsigned int note, double velocity);

	void release_note(SampleInfo& info, unsigned int channel, unsigned int note);

	void process_voices(std::array<double, SOUND_ENGINE_MIDI_CHANNELS>& lsample, std::array<double, SOUND_ENGINE_MIDI_CHANNELS>& rsample, SampleInfo& info,  ssize_t index, std::array<SoundEngineChannel, SOUND_ENGINE_MIDI_CHANNELS>& channels) {

	}

	void process_channel(double& lsample, double& rsample, unsigned int channel, SampleInfo& info) {

	};

	void process_sample(double& lsample, double& rsample, SampleInfo& info);

	std::string get_name() {
		return "Soundfont Synth";
	}

	virtual ~SoundFontSynth();

};



#endif /* MIDICUBE_SOUNDENGINE_SFSYNTH_H_ */
