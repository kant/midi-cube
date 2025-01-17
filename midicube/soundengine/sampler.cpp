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
#include <boost/filesystem.hpp>

namespace pt = boost::property_tree;


SampleSoundStore global_sample_store = {};

//SampleSound
SampleSound::SampleSound() {

}

SampleSound::~SampleSound() {
	for (size_t i = 0; i < samples.size(); ++i) {
		delete samples[i];
	}
	samples.clear();
}

//SampleSoundStore
SampleSound* SampleSoundStore::get_sound(size_t index) {
	return samples.at(index);
}

void SampleSoundStore::load_sounds(std::string folder) {
	for (const auto& f : boost::filesystem::directory_iterator(folder)) {
		std::string file = f.path().string();
		SampleSound* s = load_sound(file, pool);
		if (s) {
			samples.push_back(s);
		}
	}
}

std::vector<SampleSound*> SampleSoundStore::get_sounds() {
	return samples;
}

SampleSound* SampleSoundStore::get_sound(std::string name) {
	for (auto s : samples) {
		if (s->name == name) {
			return s;
		}
	}
	return nullptr;
}

SampleSoundStore::~SampleSoundStore() {
	for (auto s : samples) {
		delete s;
	}
}

//Sampler
Sampler::Sampler() {
	set_sample(global_sample_store.get_sound(0));
}

inline size_t find_floor_block(double time, unsigned int sample_rate, size_t size) {
	return floor(time * sample_rate) / size;
}

inline size_t find_ceil_block(double time, unsigned int sample_rate, size_t size) {
	return ceil(time * sample_rate) / size;
}

inline size_t find_buffer_index(size_t block, size_t block_count) {
	return (block - 1) % block_count;
}

void Sampler::process_note_sample(double& lsample, double& rsample, SampleInfo& info, SamplerVoice& note, KeyboardEnvironment& env, size_t note_index) {
	if (note.region && note.sample) {
		double vol = 1;
		double vel_amount = 0;

		double l = 0;
		double r = 0;
		double crossfade = 1;
		//Sound
		//Loop
		if (note.region->loop != NO_LOOP) {
			double loop_start_time = (double) note.sample->loop_start / note.sample->sample->sample_rate;
			double loop_crossfade_time = (double) note.sample->loop_crossfade / note.sample->sample->sample_rate;
			double loop_end_time = (double) note.sample->loop_end / note.sample->sample->sample_rate;
			double loop_duration_time = loop_end_time - loop_start_time;
			double crossfade_start_time = loop_start_time - loop_crossfade_time;
			double crossfade_end_time = loop_end_time - loop_crossfade_time;

			if (note.hit_loop) {
				//Loop again
				if (note.time >= loop_end_time) {
					note.time = loop_start_time + fmod(note.time - loop_start_time, loop_duration_time);
				}
				//Crossfade
				else if (note.time > crossfade_end_time) {
					double diff = note.time - crossfade_end_time;
					crossfade = 1 - (diff / loop_crossfade_time);
					lsample += note.sample->sample->isample(0, crossfade_start_time + diff, info.sample_rate) * (1 - crossfade);
					rsample += note.sample->sample->isample(1, crossfade_start_time + diff, info.sample_rate) * (1 - crossfade);
				}
			}
			else if (note.time >= loop_start_time) {
				note.hit_loop = true;
			}
		}

		if (note.region->trigger == TriggerType::ATTACK_TRIGGER || !note.pressed) {
			l += note.sample->sample->isample(0, note.time, info.sample_rate) * crossfade;
			r += note.sample->sample->isample(1, note.time, info.sample_rate) * crossfade;
		}

		//Filter
		if (note.region->filter.on) {
			FilterData filter { note.region->filter.filter_type };
			filter.cutoff = scale_cutoff(fmax(0, fmin(1, note.region->filter.filter_cutoff + note.region->filter.filter_velocity_amount * note.velocity))); //TODO optimize
			filter.resonance = note.region->filter.filter_resonance;

			if (note.region->filter.filter_kb_track) {
				double cutoff = filter.cutoff;
				//KB track
				cutoff *= 1 + ((double) note.note - 36) / 12.0 * note.region->filter.filter_kb_track;
				filter.cutoff = cutoff;
			}

			l = note.lfilter.apply(filter, l, info.time_step);
			r = note.rfilter.apply(filter, r, info.time_step);
		}

		//Env
		if (!note.region->env.sustain_entire_sample) {
			//Volume
			vol = note.env.amplitude(note.region->env.env, info.time_step, note.pressed, env.sustain);
		}
		vel_amount += note.region->env.velocity_amount;

		vol *= vel_amount * (note.velocity - 1) + 1;
		vol *= note.layer_amp;

		//Playback
		if (note.region->trigger == TriggerType::ATTACK_TRIGGER || !note.pressed) {
			double freq = note_to_freq(note.region->note + (note.note - note.region->note) * note.region->pitch_keytrack);
			note.time += freq/note_to_freq(note.region->note) * env.pitch_bend * info.time_step;
			lsample += l * vol;
			rsample += r * vol;
		}
	}
	else {
		ADSREnvelopeData data = {0, 0, 0, 0};
		note.env.amplitude(data, info.time_step, note.pressed, env.sustain);
	}
}

bool Sampler::note_finished(SampleInfo& info, SamplerVoice& note, KeyboardEnvironment& env, size_t note_index) {
	if (note.region && note.sample) {
		return (note.region->loop == NO_LOOP && note.time > note.sample->sample->total_duration()) || (!note.region->env.sustain_entire_sample && note.env.is_finished()) || note.layer_amp <= 0.0005;
	}
	return true;
}

void Sampler::press_note(SampleInfo& info, unsigned int real_note, unsigned int note, double velocity, size_t polyphony_limit) {
	if (this->sample) {
		for (SampleRegion* region : index.velocities[velocity * 127][note]) {
			size_t slot = this->note.press_note(info, real_note, note, velocity, polyphony_limit);
			SamplerVoice& voice = this->note.note[slot];
			voice.current_buffer = 0;
			voice.region = region;
			voice.layer_amp = (1 - voice.region->layer_velocity_amount * (1 - (velocity - voice.region->min_velocity/127.0)/(voice.region->max_velocity/127.0 - voice.region->min_velocity/127.0))) * region->volume * sample->volume;
			voice.sample = /*(sustain && voice.region->sustain_sample.sample.samples.size()) ? &voice.region->sustain_sample : &voice.region->sample*/ &voice.region->sample; //FIXME

			//TODO preload at start time
			if (voice.region && voice.region->loop == LoopType::ALWAYS_LOOP) {
				voice.time = (double) voice.sample->loop_start/voice.sample->sample->sample_rate;
			}
			else {
				voice.time = (double) voice.sample->start/voice.sample->sample->sample_rate;
			}
			voice.hit_loop = false;
			voice.env.reset();

			//Load sample
			LoadRequest req;
			req.sample = voice.sample->sample;
			global_sample_store.pool.queue_request(req);
		}
	}
}

void Sampler::release_note(SampleInfo& info, unsigned int note) {
	for (size_t i = 0; i < SAMPLER_POLYPHONY; ++i) {
		if (this->note.note[i].real_note == note && this->note.note[i].pressed) {
			this->note.note[i].pressed = false;
			this->note.note[i].release_time = info.time;
			if (this->note.note[i].region->trigger == TriggerType::RELEASE_TRIGGER) {
				this->note.note[i].layer_amp *= pow(this->note.note[i].region->release_decay, this->note.note[i].release_time - this->note.note[i].start_time);
			}
		}
	}
//	BaseSoundEngine<SamplerVoice, SAMPLER_POLYPHONY>::release_note(info, note);
}

std::string Sampler::get_name() {
	return "Sampler";
}

ssize_t Sampler::get_sound_index() {
	auto sounds = global_sample_store.get_sounds();
	ssize_t index = std::find(sounds.begin(), sounds.end(), sample) - sounds.begin();
	if (index >= (ssize_t) sounds.size()) {
		index = -1;
	}
	return index;
}

void Sampler::set_sound_index(ssize_t index) {
	if (index < 0) {
		set_sample(nullptr);
	}
	else {
		set_sample(global_sample_store.get_sound(index));
	}
}

void Sampler::save_program(EngineProgram **prog) {
	SamplerProgram* p = dynamic_cast<SamplerProgram*>(*prog);
	//Create new
	if (!p) {
		delete *prog;
		p = new SamplerProgram();
	}
	p->sound_name = this->sample ? this->sample->name : "";
	*prog = p;
}

void Sampler::apply_program(EngineProgram *prog) {
	SamplerProgram* p = dynamic_cast<SamplerProgram*>(prog);
	//Create new
	if (p) {
		set_sample(global_sample_store.get_sound(p->sound_name));
	}
	else {
		set_sample(nullptr);
	}
}

void Sampler::set_sample(SampleSound *sample) {
	this->sample = sample;
	index = {};
	if (sample) {
		for (SampleRegion* region : sample->samples) {
			size_t max_vel = std::min((size_t) MIDI_NOTES, (size_t) region->max_velocity);
			size_t max_note = std::min((size_t) MIDI_NOTES, (size_t) region->max_note);
			for (size_t vel = region->min_velocity; vel <= max_vel; ++vel) {
				for (size_t note = region->min_note; note <= max_note; ++note) {
					index.velocities[vel][note].push_back(region);
				}
			}
		}
	}
}

Sampler::~Sampler() {
	set_sample(nullptr);
}

void load_region(pt::ptree tree, SampleRegion& region, bool load_sample, std::string folder, StreamedAudioPool& pool) {
	region.env.env.pedal_catch = true;
	region.env.velocity_amount = tree.get<double>("envelope.velocity_amount", region.env.velocity_amount);
	region.env.env.attack = tree.get<double>("envelope.attack", region.env.env.attack);
	region.env.env.decay = tree.get<double>("envelope.decay", region.env.env.decay);
	region.env.env.sustain = tree.get<double>("envelope.sustain", region.env.env.sustain);
	region.env.env.release = tree.get<double>("envelope.release", region.env.env.release);
	region.env.env.attack_hold = tree.get<double>("envelope.attack_hold", region.env.env.attack_hold);
	region.env.sustain_entire_sample = tree.get<bool>("envelope.sustain_entire_sample", region.env.sustain_entire_sample);

	region.filter.filter_cutoff = tree.get<double>("filter.cutoff", region.filter.filter_cutoff);
	region.filter.filter_kb_track = tree.get<double>("filter.kb_track", region.filter.filter_kb_track);
	region.filter.filter_kb_track_note = tree.get<unsigned int>("filter.kb_track_note", region.filter.filter_kb_track_note);
	region.filter.filter_resonance = tree.get<double>("filter.resonance", region.filter.filter_resonance);
	region.filter.filter_velocity_amount = tree.get<double>("filter.velocity_amount", region.filter.filter_velocity_amount);

	region.pitch_keytrack = tree.get<double>("pitch_keytrack", region.pitch_keytrack);
	region.release_decay = tree.get<double>("release_decay", region.release_decay);

	std::string type = tree.get<std::string>("filter.type", "");
	if (type == "LP_12") {
		region.filter.filter_type = FilterType::LP_12;
	}
	else if (type == "LP_24") {
		region.filter.filter_type = FilterType::LP_24;
	}
	else if (type == "HP_12") {
		region.filter.filter_type = FilterType::HP_12;
	}
	else if (type == "HP_24") {
		region.filter.filter_type = FilterType::HP_24;
	}
	else if (type == "LP_12_BP") {
		region.filter.filter_type = FilterType::LP_12_BP;
	}
	else if (type == "LP_24_BP") {
		region.filter.filter_type = FilterType::LP_24_BP;
	}
	else if (type == "LP_6") {
		region.filter.filter_type = FilterType::LP_6;
	}
	else if (type == "HP_6") {
		region.filter.filter_type = FilterType::HP_6;
	}
	else if (type == "BP_12") {
		region.filter.filter_type = FilterType::BP_12;
	}

	if (tree.get_child_optional("note")) {
		region.note = tree.get<double>("note", 60);
	}
	region.min_note = tree.get<unsigned int>("min_note", region.min_note);
	region.max_note = tree.get<unsigned int>("max_note", region.max_note);
	region.min_velocity = tree.get<unsigned int>("min_velocity", region.min_velocity);
	region.max_velocity = tree.get<unsigned int>("max_velocity", region.max_velocity);

	region.volume = tree.get<double>("volume", region.volume);

	std::string trigger = tree.get<std::string>("trigger", "");
	if (trigger == "attack") {
		region.trigger = TriggerType::ATTACK_TRIGGER;
	}
	else if (trigger == "release") {
		region.trigger = TriggerType::RELEASE_TRIGGER;
	}

	std::string file = "";
	//Sample
	file = tree.get<std::string>("sample.name", "");

	std::string loop = tree.get<std::string>("loop_type", "");
	if (loop == "no_loop") {
		region.loop = LoopType::NO_LOOP;
	}
	else if (loop == "attack_loop") {
		region.loop = LoopType::ATTACK_LOOP;
	}
	else if (loop == "always_loop") {
		region.loop = LoopType::ALWAYS_LOOP;
	}

	region.sample.start = tree.get<unsigned int>("sample.start", region.sample.start);
	region.sample.loop_start = tree.get<unsigned int>("sample.loop_start", region.sample.loop_start);
	region.sample.loop_end = tree.get<unsigned int>("sample.loop_end", region.sample.loop_end);
	region.sample.loop_crossfade = tree.get<unsigned int>("sample.loop_crossfade", region.sample.loop_crossfade);

	if (load_sample && file != "") {
		region.sample.sample = pool.load_sample(folder + "/" + file);
		if (!region.sample.sample) {
			std::cerr << "Couldn't load sample file " << folder + "/" + file << std::endl;
		}
		if (region.sample.loop_start == 0 && region.sample.loop_end == 0) {
			region.sample.loop_start = region.sample.sample->loop_start;
			region.sample.loop_end = region.sample.sample->loop_end;
		}
		if (region.sample.loop_end < region.sample.loop_start) {
			region.sample.loop_end = region.sample.sample->total_size/region.sample.sample->channels;
		}
	}

	//Sustain Sample
	std::string sfile = tree.get<std::string>("sustain_sample.name", "");
	region.sustain_sample.start = tree.get<unsigned int>("sustain_sample.start", region.sustain_sample.start);
	region.sustain_sample.loop_start = tree.get<unsigned int>("sustain_sample.loop_start", region.sustain_sample.loop_start);
	region.sustain_sample.loop_end = tree.get<unsigned int>("sustain_sample.loop_end", region.sustain_sample.loop_end);
	region.sustain_sample.loop_crossfade = tree.get<unsigned int>("sustain_sample.loop_crossfade", region.sustain_sample.loop_start);

	if (load_sample && sfile != "") {
		region.sustain_sample.sample = pool.load_sample(folder + "/" + sfile);
		if (region.sustain_sample.sample) {
			std::cerr << "Couldn't load sample file " << folder + "/" + file << std::endl;
		}
		if (region.sustain_sample.loop_start == 0 && region.sample.loop_end == 0) {
			region.sustain_sample.loop_start = region.sample.sample->loop_start;
			region.sustain_sample.loop_end = region.sample.sample->loop_end;
		}
		if (region.sustain_sample.loop_end < region.sustain_sample.loop_start) {
			region.sustain_sample.loop_end = region.sustain_sample.sample->total_size/region.sustain_sample.sample->channels;
		}
	}
}

void load_groups(pt::ptree tree, std::vector<SampleRegion*>& regions, SampleRegion region, std::string folder, StreamedAudioPool& pool) {
	for (auto c : tree) {
		SampleRegion r = region;
		pt::ptree t = c.second;

		load_region(t, r, false, folder, pool);

		//Regions
		auto rs = t.get_child_optional("regions");
		if (rs) {
			for (auto re : rs.get()) {
				regions.push_back(new SampleRegion(r));
				load_region(re.second, *regions.at(regions.size() - 1), true, folder, pool);
			}
		}
		//Groups
		auto groups = t.get_child_optional("groups");
		if (groups) {
			load_groups(groups.get(), regions, r, folder, pool);
		}

	}
}

extern SampleSound* load_sound(std::string folder, StreamedAudioPool& pool) {
	//Load file
	pt::ptree tree;
	SampleSound* sound = nullptr;
	try {
		pt::read_xml(folder + "/sound.xml", tree);

		//Parse
		sound = new SampleSound();
		sound->name = tree.get<std::string>("sound.name", "Sound");
		sound->volume = tree.get<double>("sound.master_volume", 1);
		SampleRegion master;
		if (tree.get_child_optional("sound")) {
			load_region(tree.get_child("sound"), master, false, folder, pool);
		}
		//Load groups
		auto groups = tree.get_child_optional("sound.groups");
		if (groups) {
			load_groups(groups.get(), sound->samples, master, folder, pool);
		}
	}
	catch (pt::xml_parser_error& e) {
		std::cerr << "Couldn't load " << folder << "/sound.xml: " << e.message() << std::endl;
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

//SamplerProgram
void SamplerProgram::load(boost::property_tree::ptree tree) {
	sound_name = tree.get("sound_name", "");
}

boost::property_tree::ptree SamplerProgram::save() {
	boost::property_tree::ptree tree;
	tree.put("sound_name", sound_name);
	return tree;
}
