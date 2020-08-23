/*
 * organ.cpp
 *
 *  Created on: 5 Jul 2020
 *      Author: jojo
 */


#include "organ.h"
#include <cmath>

//B3OrganTonewheel
bool B3OrganTonewheel::has_turned_since(double time) {
	return time <= last_turn;
}

double B3OrganTonewheel::process(SampleInfo& info, double freq) {
	double volume = curr_vol + dynamic_vol;
	//Rotation
	double old_rot = rotation;
	rotation += freq * info.time_step;
	if ((int) (old_rot/0.5) != (int)(rotation/0.5)) {
		last_turn = info.time;
		curr_vol = static_vol;
	}
	//Reset
	static_vol = 0;
	dynamic_vol = 0;
	//Signal
	if (volume) {
		return sin(freq_to_radians(rotation)) * volume;
	}
	else {
		return 0;
	}
}


//B3Organ
B3Organ::B3Organ() {
	drawbar_harmonics = { 0.5, 0.5 * 3, 1, 2, 3, 4, 5, 6, 8 };
	drawbar_notes = {-12, 7, 0, 12, 19, 24, 28, 31, 36};
	foldback_freq = note_to_freq(ORGAN_FOLDBACK_NOTE);

	for (size_t i = 0; i < tonewheel_frequencies.size(); ++i) {
		tonewheel_frequencies[i] = note_to_freq(ORGAN_LOWEST_TONEWHEEL_NOTE + i);
	}

	for (cutoff_tonewheel = 0; cutoff_tonewheel < tonewheel_frequencies.size(); ++cutoff_tonewheel) {
		if (tonewheel_frequencies[cutoff_tonewheel] > ROTARY_CUTOFF) {
			break;
		}
	}
}

static inline double sound_delay(double rotation, double max_delay, unsigned int sample_rate) {
	return (1 + rotation) * max_delay * 0.5 * sample_rate;
}

void B3Organ::process_note_sample(std::array<double, OUTPUT_CHANNELS>& channels, SampleInfo &info, TriggeredNote& note, KeyboardEnvironment& env, SoundEngineData& d, size_t note_index) {
	B3OrganData& data = dynamic_cast<B3OrganData&>(d);

	//Organ sound
	for (size_t i = 0; i < data.preset.drawbars.size(); ++i) {
		int tonewheel = note.note + drawbar_notes.at(i) - ORGAN_LOWEST_TONEWHEEL_NOTE;
		double volume = 1;

		while (tonewheel >= (int) data.tonewheels.size()) {
			tonewheel -= 12;
			volume *= data.preset.harmonic_foldback_volume;
		}
		if (tonewheel >= 0) {
			data.tonewheels[tonewheel].static_vol += data.preset.drawbars[i] / (double) ORGAN_DRAWBAR_MAX / data.preset.drawbars.size() * volume;
		}
	}
	//Percussion
	double decay = data.preset.percussion_fast_decay ? data.preset.percussion_fast_decay_time : data.preset.percussion_slow_decay_time;
	if (data.preset.percussion && info.time - data.percussion_start <= decay) {
		double vol = (1 - (info.time - data.percussion_start)/decay) * (data.preset.percussion_soft ? data.preset.percussion_soft_volume : data.preset.percussion_hard_volume);
		int tonewheel = note.note + (data.preset.percussion_third_harmonic ? 19 : 12);

		while (tonewheel >= (int) data.tonewheels.size()) {
			tonewheel -= 12;
			vol *= data.preset.harmonic_foldback_volume;
		}
		if (tonewheel >= 0 /*&& data.tonewheels[tonewheel].has_turned_since(data.percussion_start)*/) {
			data.tonewheels[tonewheel].static_vol += vol;
		}
	}
}

void B3Organ::process_sample(std::array<double, OUTPUT_CHANNELS>& channels, SampleInfo &info, KeyboardEnvironment& env, EngineStatus& status, SoundEngineData& d) {
	B3OrganData& data = dynamic_cast<B3OrganData&>(d);

	//Play organ sound
	if (data.preset.rotary) {
		double horn_sample = 0;
		double bass_sample = 0;

		//Compute samples
		size_t i = 0;
		for (; i < cutoff_tonewheel && i < data.tonewheels.size(); ++i) {
			bass_sample += data.tonewheels[i].process(info, tonewheel_frequencies[i] * (env.pitch_bend + 1));
		}
		for (; i < data.tonewheels.size(); ++i) {
			horn_sample += data.tonewheels[i].process(info, tonewheel_frequencies[i] * (env.pitch_bend + 1));
		}

		//Horn
		double horn_pitch_rot = data.preset.rotary_type ? sin(freq_to_radians(data.horn_rotation)) :  cos(freq_to_radians(data.horn_rotation));
		double lhorn_delay = sound_delay(horn_pitch_rot, data.preset.rotary_delay, info.sample_rate);
		double rhorn_delay = sound_delay(-horn_pitch_rot, data.preset.rotary_delay, info.sample_rate);
		//Bass
		double bass_pitch_rot = data.preset.rotary_type ? sin(freq_to_radians(data.bass_rotation)) : cos(freq_to_radians(data.bass_rotation));
		double lbass_delay = sound_delay(bass_pitch_rot, data.preset.rotary_delay, info.sample_rate);
		double rbass_delay = sound_delay(-bass_pitch_rot, data.preset.rotary_delay, info.sample_rate);

		//Gain
		bass_sample *= data.preset.rotary_gain;
		horn_sample *= data.preset.rotary_gain;

		//Process
		data.left_horn_del.add_isample(horn_sample, lhorn_delay);
		data.left_bass_del.add_isample(bass_sample, lbass_delay);
		data.right_horn_del.add_isample(horn_sample, rhorn_delay);
		data.right_bass_del.add_isample(bass_sample, rbass_delay);
	}
	else {
		//Compute samples
		double sample = 0;
		for (size_t i = 0; i < data.tonewheels.size(); ++i) {
			sample += data.tonewheels[i].process(info, tonewheel_frequencies[i] * (env.pitch_bend + 1));
		}
		//Play
		for (size_t i = 0; i < channels.size() ; ++i) {
			channels[i] += sample;
		}
	}

	//Trigger percussion
	if (status.pressed_notes == 0) {
		data.reset_percussion = true;
	}
	else if (data.reset_percussion) {
		data.reset_percussion = false;
		data.percussion_start = info.time + info.time_step; //TODO first pressed frame no percussion can be heard
	}

	//Switch speaker speed
	if (data.curr_rotary_fast != data.preset.rotary_fast) {
		data.curr_rotary_fast = data.preset.rotary_fast;
		if (data.curr_rotary_fast) {
			data.horn_speed.set(ROTARY_HORN_FAST_FREQUENCY, info.time, ROTARY_HORN_FAST_RAMP);
			data.bass_speed.set(ROTARY_BASS_FAST_FREQUENCY, info.time, ROTARY_BASS_FAST_RAMP);
		}
		else {
			data.horn_speed.set(ROTARY_HORN_SLOW_FREQUENCY, info.time, ROTARY_HORN_SLOW_RAMP);
			data.bass_speed.set(ROTARY_BASS_SLOW_FREQUENCY, info.time, ROTARY_BASS_SLOW_RAMP);
		}
	}

	//Rotate speakers
	data.horn_rotation += data.horn_speed.get(info.time) * info.time_step;
	data.bass_rotation -= data.bass_speed.get(info.time) * info.time_step;

	//Play delay
	double horn_rot = sin(freq_to_radians(data.horn_rotation));
	double bass_rot = sin(freq_to_radians(data.bass_rotation));

	double left = (data.left_horn_del.process() * (0.5 + horn_rot * 0.5) + data.left_bass_del.process() * (0.5 + bass_rot * 0.5));
	double right = (data.right_horn_del.process() * (0.5 - horn_rot * 0.5) + data.right_bass_del.process() * (0.5 - bass_rot * 0.5));

	for (size_t i = 0; i < channels.size(); ++i) {
		if (i % 2 == 0) {
			channels[i] += left + right * data.preset.rotary_stereo_mix;
		}
		else {
			channels[i] += right + left * data.preset.rotary_stereo_mix;
		}
	}
}

void B3Organ::control_change(unsigned int control, unsigned int value, SoundEngineData& d) {
	B3OrganData& data = dynamic_cast<B3OrganData&>(d);
	//Drawbars
	for (size_t i = 0; i < data.preset.drawbar_ccs.size(); ++i) {
		if (data.preset.drawbar_ccs[i] == control) {
			data.preset.drawbars[i] = round((double) value/127 * ORGAN_DRAWBAR_MAX);
		}
	}
	//Rotary
	if (control == data.preset.rotary_cc) {
		data.preset.rotary = value > 0;
	}
	if (control == data.preset.rotary_speed_cc) {
		data.preset.rotary_fast = value > 0;
	}
	//Percussion
	if (control == data.preset.percussion_cc) {
		data.preset.percussion = value > 0;
	}
	//Percussion Third Harmonic
	if (control == data.preset.percussion_third_harmonic_cc) {
		data.preset.percussion_third_harmonic = value > 0;
	}
	//Percussion Fast Decay
	if (control == data.preset.percussion_fast_decay_cc) {
		data.preset.percussion_fast_decay = value > 0;
	}
	//Percussion Soft
	if (control == data.preset.percussion_soft_cc) {
		data.preset.percussion_soft = value > 0;
	}
}

SoundEngineData* B3Organ::create_data() {
	return new B3OrganData();
}

std::string B3Organ::get_name() {
	return "B3 Organ";
}

