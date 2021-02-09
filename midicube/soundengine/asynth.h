/*
 * presetsynth.h
 *
 *  Created on: 5 Jul 2020
 *      Author: jojo
 */

#ifndef MIDICUBE_SOUNDENGINE_PRESETSYNTH_H_
#define MIDICUBE_SOUNDENGINE_PRESETSYNTH_H_

#include "soundengine.h"
#include "../oscilator.h"
#include "../filter.h"
#include "../util.h"
#include "../property.h"

#define ANALOG_SYNTH_PART_COUNT 8
#define ANALOG_SYNTH_CONTROL_COUNT 128
#define ANALOG_SYNTH_POLYPHONY 32

const FixedScale VOLUME_SCALE = {0, {}, 1};
const FixedScale SYNC_SCALE = {1, {}, 10};
const FixedScale PULSE_WIDTH_SCALE = {0, {}, 0.5};
const FixedScale UNISON_DETUNE_SCALE = {0, {}, 1};
const FixedScale FILTER_CUTOFF_SCALE(0, {}, 1);
const FixedScale FILTER_RESONANCE_SCALE(0, {}, 1);
const FixedScale PITCH_SCALE(-2, {}, 2);
const FixedScale PANNING_SCALE(-1, {}, 1);

struct PropertyModulation {
	double value = 0;
	size_t mod_env = 0;
	double mod_env_amount = 0;
	size_t lfo = 0;
	double lfo_amount = 0;
	double velocity_amount = 0;
	size_t cc = 1;
	double cc_amount = 0;
};

struct FrequencyModulatotion {
	double fm_amount = 0;
	size_t modulator = 0;
};

struct OscilatorEntity {
	bool audible = true;
	ADSREnvelopeData env{0.0, 0, 1, 0.0};
	AnalogWaveForm waveform = AnalogWaveForm::SAW_DOWN_WAVE;
	bool analog = true;
	bool sync = false;
	bool reset = false;
	bool randomize = false;

	size_t unison_amount = 0;
	PropertyModulation volume = {1};
	PropertyModulation sync_mul = {0.0};
	PropertyModulation pulse_width = {1};
	PropertyModulation unison_detune = {0.1};
	PropertyModulation panning = {0.5};
	double semi = 0;
	double transpose = 1;
	PropertyModulation pitch = {0.5};

	bool filter = false;
	FilterType filter_type = FilterType::LP_12;
	PropertyModulation filter_cutoff = {1};
	PropertyModulation filter_resonance = {0};
	double filter_kb_track = 0;
	unsigned int filter_kb_track_note = 36;

	std::vector<FrequencyModulatotion> fm;
};

struct ModEnvelopeEntity {
	PropertyModulation volume = {1};
	ADSREnvelopeData env{0.0, 0, 1, 0.0};
};

struct LFOEntity {
	PropertyModulation volume = {1};
	double freq = 1;
	AnalogWaveForm waveform = AnalogWaveForm::SINE_WAVE;
};

struct AnalogSynthPreset {
	std::array<LFOEntity, ANALOG_SYNTH_PART_COUNT> lfos;
	std::array<ModEnvelopeEntity, ANALOG_SYNTH_PART_COUNT> mod_envs;
	std::array<OscilatorEntity, ANALOG_SYNTH_PART_COUNT> oscilators;

	size_t lfo_count = 1;
	size_t mod_env_count = 0;
	size_t osc_count = 0;

	bool mono = false;
	bool legato = false;
	double portamendo = 0;

	//Delay effect
	double delay_time = 0;
	double delay_feedback = 0;
	double delay_mix = 0;
};

enum SynthProperty {
	pSynthOscCount,
	pSynthModEnvCount,
	pSynthLFOCount,

	pSynthMono,
	pSynthLegato,
	pSynthPortamendo,

	pSynthDelayTime,
	pSynthDelayFeedback,
	pSynthDelayMix
};


enum SynthModulationProperty {
	pModValue,
	pModModEnv,
	pModModEnvAmount,
	pModLFO,
	pModLFOAmount,
	pModVelocityAmount,
	pModCC,
	pModCCAmount,
};

enum SynthPartProperty {
	pSynthOscAudible,
	pSynthOscAttack,
	pSynthOscDecay,
	pSynthOscSustain,
	pSynthOscRelease,
	pSynthOscWaveForm,
	pSynthOscAnalog,
	pSynthOscSync,
	pSynthOscReset,
	pSynthOscRandomize,

	pSynthOscUnisonAmount,

	pSynthOscVolume,
	pSynthOscSyncMul,
	pSynthOscPulseWidth,
	pSynthOscUnisonDetune,
	pSynthOscPanning,
	pSynthOscSemi,
	pSynthOscTranspose,
	pSynthOscPitch,

	pSynthOscFilter,
	pSynthOscFilterType,
	pSynthOscFilterCutoff,
	pSynthOscFilterResonance,
	pSynthOscFilterKBTrack,
	pSynthOscFilterKBTrackNote,

	pSynthEnvVolume,
	pSynthEnvAttack,
	pSynthEnvDecay,
	pSynthEnvSustain,
	pSynthEnvRelease,

	pSynthLFOVolume,
	pSynthLFOFrequency,
	pSynthLFOWaveForm,
};

class SynthPartPropertyHolder : public PropertyHolder {
public:
	AnalogSynthPreset* preset;
	size_t part;
	SynthPartPropertyHolder(AnalogSynthPreset* p = nullptr, size_t part = 0);
	PropertyValue get(size_t prop, size_t sub_prop = 0);
	void set(size_t prop, PropertyValue value, size_t sub_prop = 0);
	void update_properties();

protected:
	inline void submit_change(size_t prop, PropertyModulation& mod) {
		PropertyHolder::submit_change(prop, mod.value, SynthModulationProperty::pModValue);
		PropertyHolder::submit_change(prop, (int) mod.mod_env, SynthModulationProperty::pModModEnv);
		PropertyHolder::submit_change(prop, mod.mod_env_amount, SynthModulationProperty::pModModEnvAmount);
		PropertyHolder::submit_change(prop, (int) mod.lfo, SynthModulationProperty::pModLFO);
		PropertyHolder::submit_change(prop, mod.lfo_amount, SynthModulationProperty::pModLFOAmount);
		PropertyHolder::submit_change(prop, mod.velocity_amount, SynthModulationProperty::pModVelocityAmount);
		PropertyHolder::submit_change(prop, (int) mod.cc, SynthModulationProperty::pModCC);
		PropertyHolder::submit_change(prop, mod.cc_amount, SynthModulationProperty::pModCCAmount);
	}
};

struct AnalogSynthData {

};

class AnalogSynth : public BaseSoundEngine<AnalogSynthData, SimpleVoice, ANALOG_SYNTH_POLYPHONY>, public PropertyHolder {

private:
	AnalogOscilatorBank<SOUND_ENGINE_POLYPHONY * ANALOG_SYNTH_PART_COUNT, 8> oscilators;
	std::array<double, SOUND_ENGINE_POLYPHONY * ANALOG_SYNTH_PART_COUNT> modulators = {};
	std::array<Filter, SOUND_ENGINE_POLYPHONY * ANALOG_SYNTH_PART_COUNT> filters;
	std::array<ADSREnvelope, SOUND_ENGINE_POLYPHONY * ANALOG_SYNTH_PART_COUNT> amp_envs;
	std::array<ADSREnvelope, SOUND_ENGINE_POLYPHONY * ANALOG_SYNTH_PART_COUNT> mod_envs;
	std::array<double, ANALOG_SYNTH_PART_COUNT> env_val = {};
	std::array<AnalogOscilator, ANALOG_SYNTH_PART_COUNT> lfos;
	std::array<double, ANALOG_SYNTH_PART_COUNT> lfo_val = {};
	std::array<double, ANALOG_SYNTH_PART_COUNT> lfo_mod = {};
	std::array<double, ANALOG_SYNTH_CONTROL_COUNT> controls;

	bool first_port = true;
	unsigned int last_note = 0;
	PortamendoBuffer note_port{0, 0};
	DelayBuffer ldelay;
	DelayBuffer rdelay;

	inline void process_note(double& lsample, double& rsample, SampleInfo& info, SimpleVoice& voice, KeyboardEnvironment& env, size_t note_index);

	inline bool amp_finished(SampleInfo& info, TriggeredNote& note, KeyboardEnvironment& env, size_t note_index);

public:
	AnalogSynthPreset preset;
	std::array<SynthPartPropertyHolder, ANALOG_SYNTH_PART_COUNT> parts;

	AnalogSynth();

	void process_voice_sample(double& lsample, double& rsample, SampleInfo& info, SimpleVoice& voice, AnalogSynthData& channel, KeyboardEnvironment& env, size_t note_index);

	void process_sample(double& lsample, double& rsample, SampleInfo& info, AnalogSynthData& channel, KeyboardEnvironment& env, VoiceStatus<SimpleVoice>& status);

	void control_change(unsigned int control, unsigned int value);

	bool voice_finished(SampleInfo& info, SimpleVoice& voice, KeyboardEnvironment& env, size_t note_index);
	
	void set(size_t prop, PropertyValue value, size_t sub_prop);
	
	PropertyValue get(size_t prop, size_t sub_prop);

	void update_properties();

	std::string get_name() {
		return "Analog Synth";
	}

	~AnalogSynth();

};

enum SynthFactoryPreset {
	SQUASHY_BASS,
	UNISON_SAWS,
	POLY_SWEEP,
	SPOOKY_SINE,
	LUSH_LEAD,
	PULSE_BASS,
	DELAY_CHORDS,
	STRONG_PAD,
	CLEAN_SAW,
	FM_BASS,
	FM_E_PIANO,
	BRASS_PAD,
	FM_KALIMBA,
	SYNTH_BRASS,
	BELL_LEAD
};

void apply_preset(SynthFactoryPreset type, AnalogSynthPreset& preset);


#endif /* MIDICUBE_SOUNDENGINE_PRESETSYNTH_H_ */
