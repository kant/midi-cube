/*
 * Equalizer.h
 *
 *  Created on: May 27, 2021
 *      Author: jojo
 */

#ifndef MIDICUBE_EFFECT_EQUALIZER_H_
#define MIDICUBE_EFFECT_EQUALIZER_H_

#include "../audio.h"
#include "../oscilator.h"
#include "effect.h"

/*
	cc.register_binding(new TemplateControlBinding<bool>("on", preset.on, false, true));
	cc.register_binding(new TemplateControlBinding<double>("low_freq", preset.low_freq, 20, 400));
	cc.register_binding(new TemplateControlBinding<double>("low_gain", preset.low_gain, -1, 1));
	cc.register_binding(new TemplateControlBinding<double>("low_mid_freq", preset.low_mid_freq, 100, 1000));
	cc.register_binding(new TemplateControlBinding<double>("low_mid_gain", preset.low_mid_gain, -1, 1));
	cc.register_binding(new TemplateControlBinding<double>("mid_freq", preset.mid_freq, 200, 8000));
	cc.register_binding(new TemplateControlBinding<double>("mid_gain", preset.mid_gain, -1, 1));
	cc.register_binding(new TemplateControlBinding<double>("high_freq", preset.high_freq, 1000, 20000));
	cc.register_binding(new TemplateControlBinding<double>("high_gain", preset.high_gain, -1, 1));
 */

struct EqualizerPreset {
	BindableBooleanValue on = true;
	BindableTemplateValue<double> low_freq{100, 20, 400};
	BindableTemplateValue<double> low_gain{0, -1, 5};
	BindableTemplateValue<double> low_mid_freq{400, 100, 1000};
	BindableTemplateValue<double> low_mid_gain{0, -1, 5};
	BindableTemplateValue<double> mid_freq{1000, 200, 8000};
	BindableTemplateValue<double> mid_gain{0, -1, 5};
	BindableTemplateValue<double> high_freq{4000, 1000, 20000};
	BindableTemplateValue<double> high_gain{0, -1, 5};
};


class EqualizerProgram : public EffectProgram {
public:
	EqualizerPreset preset;

	virtual void load(boost::property_tree::ptree tree);
	virtual boost::property_tree::ptree save();

	virtual ~EqualizerProgram() {

	}
};

class EqualizerEffect : public Effect {
private:
	Filter llowfilter;
	Filter rlowfilter;
	Filter llow_midfilter;
	Filter rlow_midfilter;
	Filter lmidfilter;
	Filter rmidfilter;
	Filter lhighfilter;
	Filter rhighfilter;

public:
	EqualizerPreset preset;

	EqualizerEffect();
	void apply(double& lsample, double& rsample, SampleInfo& info);
	void save_program(EffectProgram **prog);
	void apply_program(EffectProgram *prog);
	~EqualizerEffect();
};

#endif /* MIDICUBE_EFFECT_EQUALIZER_H_ */
