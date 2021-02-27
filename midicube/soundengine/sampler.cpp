/*
 * sampler.cpp
 *
 *  Created on: 5 Jul 2020
 *      Author: jojo
 */

#include "sampler.h"
#include <iostream>
#include <fstream>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>

namespace pt = boost::property_tree;


//SampleSound
SampleSound::SampleSound() {

}

SampleZone* SampleSound::get_sample(double freq, double velocity) {
	//Find regions
	SampleZone* zone = nullptr;
	const size_t velocity_size = samples.size();
	if (velocity_size >= 1) {
		size_t i = 0;
		for (; i < velocity_size; ++i) {
			if (velocity <= samples[i]->max_velocity) {
				break;
			}
		}
		SampleVelocityLayer* layer = samples[std::max((ssize_t) 0, (ssize_t) i - 1)];
		const size_t zones_size = layer->zones.size();
		if (zones_size >= 1) {
			size_t j = 0;
			for (; j < zones_size; ++j) {
				if (freq <= layer->zones[j]->max_freq) {
					break;
				}
			}
			zone = layer->zones[std::max((ssize_t) 0, (ssize_t) j - 1)];
		}
	}
	return zone;
}

SampleSound::~SampleSound() {
	for (size_t i = 0; i < samples.size(); ++i) {
		delete samples[i];
	}
	samples.clear();
}

//SampleSoundStore
SampleSound* SampleSoundStore::get_sound(std::string name) {
	return samples[name];
}

void SampleSoundStore::load_sounds(std::string folder) {
	//TODO
}

void SampleSoundStore::load_sound(std::string folder) {
	//TODO
}

SampleSoundStore::~SampleSoundStore() {
	for (std::pair<std::string, SampleSound*> s : samples) {
		delete s.second;
	}
}

//Sampler
Sampler::Sampler() {
	sample = load_sound("./data/samples/piano");
}

void Sampler::process_note_sample(double& lsample, double& rsample, SampleInfo& info, SamplerVoice& note, KeyboardEnvironment& env, size_t note_index) {
	if (note.zone) {
		double vol = 1;
		if (note.env_data) {
			//Volume
			double vol = note.env.amplitude(note.env_data->env, info.time_step, note.pressed, env.sustain);
			vol *= note.env_data->amp_velocity_amount * (note.velocity - 1) + 1;
		}
		//Sound
		double time = (info.time - note.start_time) * note.freq/note.zone->freq * env.pitch_bend;
		double l = note.zone->sample.isample(0, time, info.sample_rate) * vol;
		double r = note.zone->sample.isample(1, time, info.sample_rate) * vol;
		//Filter
		if (note.filter) {
			FilterData filter { note.filter->filter_type };
			filter.cutoff = scale_cutoff(fmax(0, fmin(1, note.filter->filter_cutoff + note.filter->filter_velocity_amount * note.velocity))); //TODO optimize
			filter.resonance = note.filter->filter_resonance;

			if (note.filter->filter_kb_track) {
				double cutoff = filter.cutoff;
				//KB track
				cutoff *= 1 + ((double) note.note - 36) / 12.0 * note.filter->filter_kb_track;
				filter.cutoff = cutoff;
			}

			l = note.lfilter.apply(filter, l, info.time_step);
			r = note.rfilter.apply(filter, r, info.time_step);
		}
		//Playback
		lsample += l;
		rsample += r;
	}
	else {
		ADSREnvelopeData data = {0, 0, 0, 0};
		note.env.amplitude(data, info.time_step, note.pressed, env.sustain);
	}
}

bool Sampler::note_finished(SampleInfo& info, SamplerVoice& note, KeyboardEnvironment& env, size_t note_index) {
	return note.env.is_finished();
}

void Sampler::press_note(SampleInfo& info, unsigned int note, double velocity) {
	size_t slot = this->note.press_note(info, note, velocity);
	SamplerVoice& voice = this->note.note[slot];
	voice.env.reset();
	voice.zone = this->sample->get_sample(voice.freq, voice.velocity);
	if (voice.zone) {
		if (voice.zone->env < sample->envelopes.size()) {
			voice.env_data = &sample->envelopes[voice.zone->env];
		}
		if (voice.zone->filter < sample->filters.size()) {
			voice.filter = &sample->filters[voice.zone->filter];
		}
	}
}

void Sampler::release_note(SampleInfo& info, unsigned int note) {
	BaseSoundEngine<SamplerVoice, SAMPLER_POLYPHONY>::release_note(info, note);
}

std::string Sampler::get_name() {
	return "Sampler";
}

Sampler::~Sampler() {
	delete sample;
	sample = nullptr;
}


extern SampleSound* load_sound(std::string folder) {
	//Load file
	pt::ptree tree;
	SampleSound* sound = nullptr;
	try {
		pt::read_xml(folder + "/sound.xml", tree);

		//Parse
		sound = new SampleSound();
		sound->name = tree.get<std::string>("sound.name", "Sound");
		//Load velocity layers
		for (auto r : tree.get_child("sound.velocity_layers")) {
			SampleVelocityLayer* layer = new SampleVelocityLayer();
			layer->max_velocity = r.second.get<double>("velocity", 1.0);
			//Load zones
			for (auto z : r.second.get_child("zones")) {
				SampleZone* zone = new SampleZone();
				zone->freq = note_to_freq(z.second.get<double>("note", 60.0));
				zone->max_freq = note_to_freq(z.second.get<double>("max_note", 127.0));
				zone->env.attack = z.second.get<double>("amp_env.attack", 0);
				zone->env.decay = z.second.get<double>("amp_env.decay", 0);
				zone->env.sustain = z.second.get<double>("amp_env.sustain", 1);
				zone->env.release = z.second.get<double>("amp_env.release", 0);
				std::string file = folder + "/" + z.second.get<std::string>("sample", "");
				if (!read_audio_file(zone->sample, file)) {
					std::cerr << "Couldn't load sample file " << file << std::endl;
				}
				layer->zones.push_back(zone);
			}
			sound->samples.push_back(layer);
		}
	}
	catch (pt::xml_parser_error& e) {
		std::cerr << "Couldn't load sound.xml: " << e.message() << std::endl;
		delete sound;
		sound = nullptr;
	}
	return sound;
}

extern void save_sound(std::string file) {

}

template<>
std::string get_engine_name<Sampler>() {
	return "Sampler";
}

void __fix_link_sampler_name__ () {
	get_engine_name<Sampler>();
}

