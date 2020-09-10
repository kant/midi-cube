/*
 * presetsynth.cpp
 *
 *  Created on: 5 Jul 2020
 *      Author: jojo
 */

#include "presetsynth.h"
#include "../synthesis.h"


//PresetSynth
PresetSynth::PresetSynth() {
	/*detune = note_to_freq_transpose(0.1);
	ndetune = note_to_freq_transpose(-0.1);
	vibrato = 0;
	filter.set_cutoff(6300);

	AdditiveOscilator* aosc = new AdditiveOscilator();
	std::vector<double> harmonics = { 0.5, 0.5 * 3, 1, 2, 3, 4, 5, 6, 8 };
	for (size_t i = 0; i < harmonics.size(); ++i) {
		//aosc->add_sine({harmonics[i], 1});
	}

	osc = new OscilatorSlot(aosc);
	osc->set_volume(0.0);
	osc->set_unison(3);
	osc->set_unison_detune(0.05);

	osc2 = new OscilatorSlot(new AnalogOscilator(AnalogWaveForm::SQUARE));
	osc2->set_volume(1);
	osc2->set_unison(0);*/

	detune = note_to_freq_transpose(0.1);
	ndetune = note_to_freq_transpose(-0.1);
	vibrato = 0;
	filter.set_cutoff(1200);

	osc.data.analog = true;
	osc.data.waveform = AnalogWaveForm::SAW;
	osc.data.unison_amount = 3;
}

void PresetSynth::process_note_sample(std::array<double, OUTPUT_CHANNELS>& channels, SampleInfo &info, TriggeredNote& note, KeyboardEnvironment& env, SoundEngineData& data, size_t note_index) {
	double sample = 0.0;
	double freq = note.freq;
	double t = info.time + note.phase_shift;
	sample = osc.signal(t, freq, info.time_step, note_index);

	if (vibrato) {
		note.phase_shift += info.time_step * (note_to_freq_transpose(SYNTH_VIBRATO_DETUNE * sine_wave(info.time, SYNTH_VIBRATO_RATE) * vibrato) - 1);
	}

	double amp = this->env.amplitude(info.time, note, env);
	sample *= amp;

	for (size_t i = 0; i < channels.size() ; ++i) {
		channels[i] += sample;
	}
}

void PresetSynth::process_sample(std::array<double, OUTPUT_CHANNELS>& channels, SampleInfo& info, KeyboardEnvironment& env, EngineStatus& status, SoundEngineData& data) {
	filter.apply(channels, info.time_step);
};

bool PresetSynth::note_finished(SampleInfo& info, TriggeredNote& note, KeyboardEnvironment& env, SoundEngineData& data) {
	return this->env.is_finished(info.time, note, env);
}

void PresetSynth::control_change(unsigned int control, unsigned int value, SoundEngineData& data) {
	if (control == 1) {
		vibrato = value/127.0;
	}
	if (control == 35) {
		filter.set_cutoff(21000/(128.0 - value));
	}
}

std::string PresetSynth::get_name() {
	return "Preset Synth";
}

PresetSynth::~PresetSynth() {

}

