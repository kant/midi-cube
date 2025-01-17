/*
 * amplifier_simulation.cpp
 *
 *  Created on: Jan 7, 2021
 *      Author: jojo
 */

#include "amplifier_simulation.h"

AmplifierSimulationEffect::AmplifierSimulationEffect() {
	cc.register_binding(new TemplateControlBinding<bool>("on", preset.on, false, true, 28));
	cc.register_binding(new TemplateControlBinding<double>("post_gain", preset.post_gain, 0, 1, 35));
	cc.register_binding(new TemplateControlBinding<double>("drive", preset.drive, 0, 1, 36));
	cc.register_binding(new TemplateControlBinding<double>("tone", preset.tone, 0, 1, 37));
}

static inline double cubic_distortion(double sample) {
	if (sample < 0) {
		double t = sample + 1;
		sample = t * t * t - 1;
	}
	else {
		double t = sample - 1;
		sample = t * t * t + 1;
	}
	return sample;
}

static double apply_distortion(double sample, double drive, DistortionType type) {
	switch (type) {
	/*case DistortionType::TUBE_AMP_DISTORTION:
	{
		double a = sin((drive * 100.0 + 1)/102 * (M_PI/2.0));
		double k = 2 * a/(1 - a);
		sample = (1 + k) * sample / (1 + k * abs(sample));
	}
		break;*/
	case DistortionType::DIGITAL_DISTORTION:
	{
		double clip = (1 - drive);
		sample = fmax(fmin(sample, clip), -clip);
		sample *= clip ? 1/clip : 0;
	}
		break;
	case DistortionType::POLYNOMAL_DISTORTION:
	{
		sample -= (sample * sample * sample) * drive;
	}
		break;
	case DistortionType::ARCTAN_DISTORTION:
	{
		sample = atan(sample * (1 + drive * 4));
	}
		break;
	case DistortionType::CUBIC_DISTORTION:
	{
		sample = cubic_distortion(sample * (0.3 + drive * 4.5));
	}
	break;
	case DistortionType::FUZZ_DISTORTION:
	{
		sample = cubic_distortion(cubic_distortion(cubic_distortion(sample * (0.3 + drive * 2)))) / 3.0;
	}
	break;
	}
	return sample;
}

void AmplifierSimulationEffect::apply(double &lsample, double &rsample, SampleInfo &info) {
	if (preset.on) {
		//Distortion
		lsample = apply_distortion(lsample, preset.drive, preset.type);
		rsample = apply_distortion(rsample, preset.drive, preset.type);

		//Low-pass
		FilterData data;
		data.type = FilterType::LP_24;
		data.cutoff = 200 + preset.tone * 20000;
		lsample = lfilter.apply(data, lsample, info.time_step);
		rsample = rfilter.apply(data, rsample, info.time_step);

		//Gain
		double gain = preset.post_gain + 1;
		lsample *= gain;
		rsample *= gain;
	}
}

AmplifierSimulationEffect::~AmplifierSimulationEffect() {

}

template<>
std::string get_effect_name<AmplifierSimulationEffect>() {
	return "Amplifier";
}

void AmplifierSimulationProgram::load(boost::property_tree::ptree tree) {
	EffectProgram::load(tree);
	preset.on = tree.get<bool>("on", true);
	preset.post_gain = tree.get<double>("post_gain", 0);
	preset.drive = tree.get<double>("drive", 0);
	preset.tone = tree.get<double>("tone", 0.6);
}

boost::property_tree::ptree AmplifierSimulationProgram::save() {
	boost::property_tree::ptree tree = EffectProgram::save();
	tree.put("on", preset.on);
	tree.put("post_gain", preset.post_gain);
	tree.put("drive", preset.drive);
	tree.put("tone", preset.tone);
	return tree;
}

template <>
EffectProgram* create_effect_program<AmplifierSimulationEffect>() {
	return new AmplifierSimulationProgram();
}

void AmplifierSimulationEffect::save_program(EffectProgram **prog) {
	AmplifierSimulationProgram* p = dynamic_cast<AmplifierSimulationProgram*>(*prog);
	//Create new
	if (!p) {
		delete *prog;
		p = new AmplifierSimulationProgram();
	}
	p->ccs = cc.get_ccs();
	p->preset = preset;
	*prog = p;
}

void AmplifierSimulationEffect::apply_program(EffectProgram *prog) {
	AmplifierSimulationProgram* p = dynamic_cast<AmplifierSimulationProgram*>(prog);
	//Create new
	if (p) {
		cc.set_ccs(p->ccs);
		preset = p->preset;
	}
	else {
		cc.set_ccs({});
		preset = {};
	}
}
