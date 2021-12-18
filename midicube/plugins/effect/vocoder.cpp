/*
 * vocoder.cpp
 *
 *  Created on: Dec 3, 2020
 *      Author: jojo
 */

#include "vocoder.h"
#include "../view/EffectView.h"

void VocoderEffect::process(const SampleInfo& info) {
	outputs[0] = inputs[0];
	outputs[1] = inputs[1];
	double modulator = inputs[2];

	if (preset.on) {
		//Gate
		modulator *= preset.modulator_amplification;
		modulator_env.apply(modulator, info.time_step);
		if (modulator_env.volume() < preset.gate) {
			modulator = 0;
		}
		//Vocode
		double lvocoded = 0;
		double rvocoded = 0;
		//Filters
		double vol_sum = 0;

		for (size_t i = 0; i < bands.size(); ++i) {
			VocoderBand& band = bands[i];

			FilterData filter_data{preset.filter_type, preset.formant_mode ? frequencies[i] : ((preset.max_freq - preset.min_freq)/VOCODER_BAND_COUNT * i + preset.min_freq), preset.resonance};
			//Filter
			double lf = band.lfilter.apply(filter_data, outputs[0], info.time_step);
			double rf = band.rfilter.apply(filter_data, outputs[1], info.time_step);
			double m = band.mfilter.apply(filter_data, modulator, info.time_step);

			//Modulator amp
			band.env.apply(m, info.time_step);
			double vol = band.env.volume();

			band.port.set(vol, info.time, preset.slope);
			vol = band.port.get(info.time);
			vol_sum += vol;

			//Vocode carrier
			lvocoded += lf * vol;
			rvocoded += rf * vol;
		}

		if (vol_sum) {
			lvocoded /= vol_sum + (1 - vol_sum) * (1 - preset.normalization);
			rvocoded /= vol_sum + (1 - vol_sum) * (1 - preset.normalization);
		}
		//Highpass
		if (preset.mod_highpass) {
			FilterData data = {FilterType::HP_24, preset.mod_highpass};
			modulator = mfilter.apply(data, modulator, info.time_step);
		}
		//Modulator Mix
		lvocoded *= 1 - (fmax(0, preset.modulator_mix - 0.5) * 2);
		rvocoded *= 1 - (fmax(0, preset.modulator_mix - 0.5) * 2);

		lvocoded += modulator * fmin(0.5, preset.modulator_mix) * 2;
		rvocoded += modulator * fmin(0.5, preset.modulator_mix) * 2;

		//Carrier Mix
		outputs[0] *= fmin(0.5, preset.mix) * 2;
		outputs[1] *= fmin(0.5, preset.mix) * 2;

		outputs[0] += lvocoded * (1 - (fmax(0, preset.mix - 0.5) * 2));
		outputs[1] += rvocoded * (1 - (fmax(0, preset.mix - 0.5) * 2));

		outputs[0]*= preset.post_amplification;
		outputs[1] *= preset.post_amplification;
	}
}

Menu* VocoderEffect::create_menu() {
	return new FunctionMenu([this](){ return new EffectView(this); }, nullptr);
}


void VocoderProgram::load(boost::property_tree::ptree tree) {
	preset.on = tree.get<bool>("on", true);
	preset.modulator_amplification = tree.get<double>("modulator_amplification", 5);
	preset.post_amplification = tree.get<double>("post_amplification", 10);

	preset.modulator_mix = tree.get<double>("modulator_mix", 0.2);
	preset.mix = tree.get<double>("mix", 0);

	preset.gate = tree.get<double>("gate", 0);
	preset.mod_highpass = tree.get<double>("mod_highpass", 1200);

	preset.normalization = tree.get<double>("normalization", 0.2);
	preset.slope = tree.get<double>("slope", 0);

	preset.formant_mode = tree.get<bool>("formant_mode", true);
	preset.min_freq = tree.get<double>("min_freq", 120);
	preset.max_freq = tree.get<double>("max_freq", 360);
	preset.resonance = tree.get<double>("resonance", 0);
}

boost::property_tree::ptree VocoderProgram::save() {
	boost::property_tree::ptree tree;
	tree.put("on", preset.on);
	tree.put("modulator_amplification", preset.modulator_amplification);
	tree.put("post_amplification", preset.post_amplification);

	tree.put("modulator_mix", preset.modulator_mix);
	tree.put("mix", preset.mix);

	tree.put("gate", preset.gate);
	tree.put("mod_highpass", preset.mod_highpass);

	tree.put("normalization", preset.normalization);
	tree.put("slope", preset.slope);

	tree.put("formant_mode", preset.formant_mode);
	tree.put("min_freq", preset.min_freq);
	tree.put("max_freq", preset.max_freq);
	tree.put("resonance", preset.resonance);
	return tree;
}

void VocoderEffect::save_program(PluginProgram **prog) {
	VocoderProgram* p = dynamic_cast<VocoderProgram*>(*prog);
	//Create new
	if (!p) {
		delete *prog;
		p = new VocoderProgram();
	}
	
	p->preset = preset;
	*prog = p;
}

void VocoderEffect::apply_program(PluginProgram *prog) {
	VocoderProgram* p = dynamic_cast<VocoderProgram*>(prog);
	//Create new
	if (p) {
		preset = p->preset;
	}
	else {
		
		preset = {};
	}
}

std::string VocoderProgram::get_plugin_name() {
	return VOCODER_IDENTIFIER;
}
