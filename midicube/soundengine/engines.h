/*
 * engines.h
 *
 *  Created on: 5 Jul 2020
 *      Author: jojo
 */

#ifndef MIDICUBE_SOUNDENGINE_ENGINES_H_
#define MIDICUBE_SOUNDENGINE_ENGINES_H_

#include "organ.h"
#include "drums.h"
#include "sampler.h"
#include "sfsynth.h"
#include "asynth.h"

extern void fill_sound_engine_device(SoundEngineDevice* device) {
	device->add_sound_engine(new SoundFontSynth());
	device->add_sound_engine(new B3Organ());
	device->add_sound_engine(new AnalogSynth());
	device->add_sound_engine(new SampleDrums());
	device->add_sound_engine(new Sampler());
}



#endif /* MIDICUBE_SOUNDENGINE_ENGINES_H_ */
