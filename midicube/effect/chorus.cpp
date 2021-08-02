/*
 * BitCrusher.cpp
 *
 *  Created on: Jan 2, 2021
 *      Author: jojo
 */

#include "chorus.h"
#include <cmath>

ChorusEffect::ChorusEffect() {
	cc.add_binding(&preset.on);
	cc.add_binding(&preset.vibrato_rate);
	cc.add_binding(&preset.vibrato_depth);
	cc.add_binding(&preset.mix);

	cc.add_binding(&preset.delay);
}

void ChorusEffect::apply(double& lsample, double& rsample, SampleInfo& info) {
	if (preset.on) {
		double l = ldelay.process();
		double r = rdelay.process();

		//Delay
		AnalogOscilatorData data = {preset.vibrato_waveform};
		osc.process(preset.vibrato_rate, info.time_step, data);
		double del = (1 + osc.carrier(preset.vibrato_rate, info.time_step, data) * preset.vibrato_depth * 0.2) * preset.delay * info.sample_rate;
		ldelay.add_isample(lsample, del);
		rdelay.add_isample(rsample, del);

		//Mix
		lsample *= 1 - (fmax(0, preset.mix - 0.5) * 2);
		rsample *= 1 - (fmax(0, preset.mix - 0.5) * 2);

		lsample += l * fmin(0.5, preset.mix) * 2;
		rsample += r * fmin(0.5, preset.mix) * 2;
	}
}

ChorusEffect::~ChorusEffect() {

}

template<>
std::string get_effect_name<ChorusEffect>() {
	return "Chorus";
}

template <>
EffectProgram* create_effect_program<ChorusEffect>() {
	return new ChorusProgram();
}


void ChorusProgram::load(boost::property_tree::ptree tree) {
	EffectProgram::load(tree);
	preset.on.load(tree, "on", true);
	preset.vibrato_rate.load(tree, "vibrato_rate", 2);
	preset.vibrato_depth.load(tree, "vibrato_depth", 0.5);
	preset.mix.load(tree, "mix", 0.5);

	preset.delay.load(tree, "delay", 0.015);
	preset.vibrato_waveform = (AnalogWaveForm) tree.get<unsigned int>("vibrato_waveform", (unsigned int) AnalogWaveForm::TRIANGLE_WAVE);
}

boost::property_tree::ptree ChorusProgram::save() {
	boost::property_tree::ptree tree = EffectProgram::save();
	tree.add_child("on", preset.on.save());
	tree.add_child("vibrato_rate", preset.vibrato_rate.save());
	tree.add_child("vibrato_depth", preset.vibrato_depth.save());
	tree.add_child("mix", preset.mix.save());

	tree.add_child("delay", preset.delay.save());
	tree.put("vibrato_waveform", (unsigned int) preset.vibrato_waveform);
	return tree;
}

void ChorusEffect::save_program(EffectProgram **prog) {
	ChorusProgram* p = dynamic_cast<ChorusProgram*>(*prog);
	//Create new
	if (!p) {
		delete *prog;
		p = new ChorusProgram();
	}

	p->preset = preset;
	*prog = p;
}

void ChorusEffect::apply_program(EffectProgram *prog) {
	ChorusProgram* p = dynamic_cast<ChorusProgram*>(prog);
	//Create new
	if (p) {
		preset = p->preset;
	}
	else {
		
		preset = {};
	}
}
