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
	device->sound_engines.at(0) = new SoundFontSynth();
	device->sound_engines.at(1) = new B3Organ();
	device->sound_engines.at(2) = new AnalogSynth();
	device->sound_engines.at(3) = new SampleDrums();
	device->sound_engines.at(4) = new Sampler();
}



#endif /* MIDICUBE_SOUNDENGINE_ENGINES_H_ */
