/*
 * organ.h
 *
 *  Created on: 5 Jul 2020
 *      Author: jojo
 */

#ifndef MIDICUBE_SOUNDENGINE_ORGAN_H_
#define MIDICUBE_SOUNDENGINE_ORGAN_H_


#include "soundengine.h"
#include "../synthesis.h"
#include "../effect/rotary_speaker.h"
#include "../effect/amplifier_simulation.h"
#include "../property.h"

#define ORGAN_DRAWBAR_COUNT 9
#define ORGAN_DRAWBAR_MAX 8
#define ORGAN_FOLDBACK_NOTE 114
#define ORGAN_TONEWHEEL_AMOUNT 91
#define ORGAN_LOWEST_TONEWHEEL_NOTE 24

#define MIN_SWELL 0.1
#define SWELL_RANGE (1 - MIN_SWELL)


enum B3OrganProperty {
	pB3Drawbar1,
	pB3Drawbar2,
	pB3Drawbar3,
	pB3Drawbar4,
	pB3Drawbar5,
	pB3Drawbar6,
	pB3Drawbar7,
	pB3Drawbar8,
	pB3Drawbar9,

	pB3DrawbarCC1,
	pB3DrawbarCC2,
	pB3DrawbarCC3,
	pB3DrawbarCC4,
	pB3DrawbarCC5,
	pB3DrawbarCC6,
	pB3DrawbarCC7,
	pB3DrawbarCC8,
	pB3DrawbarCC9,

	pB3HarmonicFoldbackVolume,

	pB3AmpOn,
	pB3AmpDrive,
	pB3AmpBoost,
	pB3AmpTone,
	pB3AmpDistortionType,
	pB3AmpOnCC,
	pB3AmpBoostCC,
	pB3AmpDriveCC,
	pB3AmpToneCC,

	pB3MultiNoteGain,

	pB3Rotary,
	pB3RotarySpeed,
	pB3RotaryCC,
	pB3RotarySpeedCC,

	pB3RotaryStereoMix,
	pB3RotaryType,
	pB3RotaryDelay,

	pB3Percussion,
	pB3PercussionThirdHarmonic,
	pB3PercussionSoft,
	pB3PercussionFastDecay,

	pB3PercussionSoftVolume,
	pB3PercussionHardVolume,
	pB3PercussionFastDecayTime,
	pB3PercussionSlowDecayTime,

	pB3PercussionCC,
	pB3PercussionThirdHarmonicCC,
	pB3PercussionSoftCC,
	pB3PercussionFastDecayCC,

	pB3SwellCC,
};

struct B3OrganPreset {
	std::array<unsigned int, ORGAN_DRAWBAR_COUNT> drawbars;
	std::array<unsigned int, ORGAN_DRAWBAR_COUNT> drawbar_ccs;
	double harmonic_foldback_volume{1.0};

	AmplifierSimulationPreset amplifier;
	unsigned int amp_cc{28};
	unsigned int amp_boost_cc{35};
	unsigned int amp_drive_cc{36};
	unsigned int amp_tone_cc{37};

	double multi_note_gain{0.75};

	RotarySpeakerPreset rotary;
	unsigned int rotary_cc{22};
	unsigned int rotary_speed_cc{23};

	bool percussion{false};
	bool percussion_third_harmonic{true};
	bool percussion_soft{true};
	bool percussion_fast_decay{true};

	double percussion_soft_volume{0.5};
	double percussion_hard_volume{1.0};
	double percussion_fast_decay_time{0.2};
	double percussion_slow_decay_time{1.0};

	unsigned int percussion_cc{24};
	unsigned int percussion_third_harmonic_cc{25};
	unsigned int percussion_soft_cc{26};
	unsigned int percussion_fast_decay_cc{27};

	unsigned int swell_cc{11};

	B3OrganPreset () {
		for (size_t i = 0; i < drawbars.size(); ++i) {
			drawbars[i] = 8;
		}
		size_t i = 0;
		drawbar_ccs.at(i++) = 67;
		drawbar_ccs.at(i++) = 68;
		drawbar_ccs.at(i++) = 69;
		drawbar_ccs.at(i++) = 70;
		drawbar_ccs.at(i++) = 87;
		drawbar_ccs.at(i++) = 88;
		drawbar_ccs.at(i++) = 89;
		drawbar_ccs.at(i++) = 90;
		drawbar_ccs.at(i++) = 92;
	}
};

class B3OrganTonewheel {
private:
	double rotation = 0;
public:
	double volume = 0;
	double process(SampleInfo& info, double freq);
};

class B3OrganData : public SoundEngineData{
public:
	B3OrganPreset preset;
	AmplifierSimulationEffect amplifier;
	RotarySpeakerEffect rotary_speaker;
	std::array<B3OrganTonewheel, ORGAN_TONEWHEEL_AMOUNT> tonewheels;

	bool reset_percussion = true;
	double percussion_start = 0;

	double swell = 1;

	virtual SoundEngineData* copy() {
		return new B3OrganData();	//TODO
	}
};

class B3Organ : public BaseSoundEngine, public PropertyHolder {

private:
	//Static values
	std::array<double, ORGAN_DRAWBAR_COUNT> drawbar_harmonics;
	std::array<int, ORGAN_DRAWBAR_COUNT> drawbar_notes;
	std::array<double, ORGAN_TONEWHEEL_AMOUNT> tonewheel_frequencies;

public:
	B3OrganData data;

	B3Organ();

	void process_note_sample(std::array<double, OUTPUT_CHANNELS>& channels, SampleInfo& info, TriggeredNote& note, KeyboardEnvironment& env, size_t note_index);

	void process_sample(std::array<double, OUTPUT_CHANNELS>& channels, SampleInfo& info, KeyboardEnvironment& env, EngineStatus& status);

	void control_change(unsigned int control, unsigned int value);

	PropertyValue get(size_t prop);

	void set(size_t prop, PropertyValue value);

};

#endif /* MIDICUBE_SOUNDENGINE_ORGAN_H_ */
