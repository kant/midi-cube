/*
 * drums.h
 *
 *  Created on: 5 Jul 2020
 *      Author: jojo
 */

#ifndef MIDICUBE_SOUNDENGINE_DRUMS_H_
#define MIDICUBE_SOUNDENGINE_DRUMS_H_

#include "soundengine.h"
#include "../audiofile.h"
#include <unordered_map>

#define SAMPLE_DRUMS_POLYPHONY 32

struct SampleDrumKit {
	std::unordered_map<unsigned int, AudioSample> notes;
};

class SampleDrums : public BaseSoundEngine<SimpleVoice, SAMPLE_DRUMS_POLYPHONY> {

private:
	SampleDrumKit* drumkit;

public:

	SampleDrums();

	void process_note_sample(double& lsample, double& rsample, SampleInfo& info, SimpleVoice& note, KeyboardEnvironment& env, size_t note_index);

	bool note_finished(SampleInfo& info, SimpleVoice& note, KeyboardEnvironment& env, size_t note_index);

	std::string get_name() {
		return "Sample Drums";
	}

	~SampleDrums();

};

extern SampleDrumKit* load_drumkit(std::string folder);

#endif /* MIDICUBE_SOUNDENGINE_DRUMS_H_ */
