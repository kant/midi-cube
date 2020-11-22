/*
 * asynth.cpp
 *
 *  Created on: Nov 15, 2020
 *      Author: jojo
 */
#include "asynth.h"
#include <cmath>


AnalogSynth::AnalogSynth() {
	//Squashy Synth Basss
	/*ModEnvelopeEntity& mod_env = preset.mod_envs.at(0);
	mod_env.active = true;
	mod_env.env = {0, 0.1, 0, 0.003};

	OscilatorEntity& osc = preset.oscilators.at(0);
	osc.active = true;
	osc.env = {0.0005, 0.35, 0, 0.06};
	osc.filter = true;
	osc.filter_type = FilterType::LP_24;
	osc.filter_cutoff.value = 0.05;
	osc.filter_cutoff.mod_env = 0;
	osc.filter_cutoff.mod_amount = 0.15;
	osc.filter_resonance.value = 0.8;*/

	OscilatorEntity& osc = preset.oscilators.at(0);
	osc.unison_amount = 2;
	osc.active = true;
	osc.env = {0.0005, 0, 1, 0.003};
	osc.filter = true;
	osc.filter_type = FilterType::LP_24;
	osc.filter_cutoff.value = 0.0;
	osc.filter_cutoff.cc_amount = 1;
}

static double apply_modulation(const FixedScale& scale, PropertyModulation& mod, std::array<double, ANALOG_MOD_ENV_COUNT>& env_val, std::array<double, ANALOG_LFO_COUNT>& lfo_val, std::array<double, ANALOG_CONTROL_COUNT>& controls, double velocity) {
	double prog = mod.value;
	prog += env_val.at(mod.mod_env) * mod.mod_amount + lfo_val.at(mod.lfo) * mod.lfo_amount + controls.at(mod.cc) * mod.cc_amount + velocity * mod.velocity_amount;
	prog = fmin(fmax(prog, 0.0), 1.0);
	return scale.value(prog);
}

void AnalogSynth::process_note_sample(std::array<double, OUTPUT_CHANNELS>& channels, SampleInfo& info, TriggeredNote& note, KeyboardEnvironment& env, size_t note_index) {
	std::array<double, ANALOG_MOD_ENV_COUNT> env_val = {};
	std::array<double, ANALOG_LFO_COUNT> lfo_val = {};
	double freq = note.freq * env.pitch_bend;
	//Mod Envs
	for (size_t i = 0; i < preset.mod_envs.size(); ++i) {
		ModEnvelopeEntity& mod_env = preset.mod_envs[i];
		if (mod_env.active) {
			double volume = apply_modulation(VOLUME_SCALE, mod_env.volume, env_val, lfo_val, controls, note.velocity);
			env_val.at(i) = mod_env.env.amplitude(info.time, note, env) * volume;
		}
	}
	//LFOs
	for (size_t i = 0; i < preset.lfos.size(); ++i) {
		LFOEntity& lfo = preset.lfos[i];
		if (lfo.active) {
			double volume = apply_modulation(VOLUME_SCALE, lfo.volume, env_val, lfo_val, controls, note.velocity);
			AnalogOscilatorData d = {lfo.waveform};
			lfo_val.at(i) = lfos.at(i).signal(lfo.freq, info.time_step, d, false) * volume;
		}
	}
	//Synthesize
	double sample = 0;
	for (size_t i = 0; i < preset.oscilators.size(); ++i) {
		OscilatorEntity& osc = preset.oscilators[i];
		if (osc.active) {
			AnalogOscilatorData data = {osc.waveform, osc.analog, osc.sync};
			AnalogOscilatorBankData bdata = {0.1, osc.unison_amount};
			//Apply modulation
			double volume = apply_modulation(VOLUME_SCALE, osc.volume, env_val, lfo_val, controls, note.velocity) * osc.env.amplitude(info.time, note, env);
			data.sync_mul = apply_modulation(SYNC_SCALE, osc.sync_mul, env_val, lfo_val, controls, note.velocity);
			data.pulse_width = apply_modulation(PULSE_WIDTH_SCALE, osc.pulse_width, env_val, lfo_val, controls, note.velocity);
			bdata.unison_detune = apply_modulation(UNISON_DETUNE_SCALE, osc.unison_detune, env_val, lfo_val, controls, note.velocity);

			double signal =  oscilators.signal(freq, info.time_step, note_index + i * SOUND_ENGINE_POLYPHONY, data, bdata, false);
			//Filter
			if (osc.filter) {
				FilterData filter{osc.filter_type};
				filter.cutoff = apply_modulation(FILTER_CUTOFF_SCALE, osc.filter_cutoff, env_val, lfo_val, controls, note.velocity);
				filter.resonance = apply_modulation(FILTER_RESONANCE_SCALE, osc.filter_resonance, env_val, lfo_val, controls, note.velocity);
				signal = filters.at(note_index + i * SOUND_ENGINE_POLYPHONY).apply(filter, signal, info.time_step);
			}

			sample += signal * volume;
		}
	}

	//Play
	for (size_t i = 0; i < channels.size(); ++i) {
		channels[i] += sample;
	}
}

void AnalogSynth::process_sample(std::array<double, OUTPUT_CHANNELS>& channels, SampleInfo& info, KeyboardEnvironment& env, EngineStatus& status) {

}

void AnalogSynth::control_change(unsigned int control, unsigned int value) {
	controls.at(control) = value/127.0;
}

bool AnalogSynth::note_finished(SampleInfo& info, TriggeredNote& note, KeyboardEnvironment& env) {
	bool finished = true;
	for (size_t i = 0; i < preset.oscilators.size() && finished; ++i) {
		OscilatorEntity& osc = preset.oscilators[i];
		if (osc.active) {
			finished = osc.env.is_finished(info.time, note, env);
		}
	}
	return finished;
}

AnalogSynth::~AnalogSynth() {

}

template<>
std::string get_engine_name<AnalogSynth>() {
	return "Analog Synthesizer";
}

