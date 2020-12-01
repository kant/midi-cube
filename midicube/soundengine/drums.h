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



struct SampleDrumKit {
	std::unordered_map<unsigned int, AudioSample> notes;
};



class SampleDrums : public BaseSoundEngine {

private:
	SampleDrumKit* drumkit;

public:

	SampleDrums();

	void process_note_sample(std::array<double, OUTPUT_CHANNELS>& channels, SampleInfo& info, TriggeredNote& note, KeyboardEnvironment& env, size_t note_index);

	bool note_finished(SampleInfo& info, TriggeredNote& note, KeyboardEnvironment& env, size_t note_index);

	~SampleDrums();

};

extern SampleDrumKit* load_drumkit(std::string folder);

#endif /* MIDICUBE_SOUNDENGINE_DRUMS_H_ */
