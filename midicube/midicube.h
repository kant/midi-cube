/*
 * midicube.h
 *
 *  Created on: Oct 3, 2020
 *      Author: jojo
 */

#ifndef MIDICUBE_MIDICUBE_H_
#define MIDICUBE_MIDICUBE_H_

#include "audio.h"
#include "program.h"
#include "data.h"
#include "soundengine/soundengine.h"
#include "audioloader.h"
#include "util.h"

struct MidiCubeInput {
	MidiInput* in = nullptr;
	std::string name = "";
};

struct MidiMessageWithInput {
	size_t input = 0;
	MidiMessage msg;
};

class MidiCube : public ProgramUser {
private:
	AudioHandler audio_handler;
	std::vector<MidiCubeInput> inputs;

	inline void process_midi(MidiMessage& message, size_t input);
public:
	std::array<MidiSource, SOUND_ENGINE_MIDI_CHANNELS> sources;
	size_t used_sources = 1;

	ProgramManager prog_mgr;
	SoundEngineDevice engine;
	ActionHandler action_handler;

	std::atomic<bool> updated{false};

	SpinLock lock;

	virtual void save_program(Program *prog);
	virtual void apply_program(Program *prog);
	MidiCube();
	void init(int out_device = -1, int in_device = -1);
	inline void process(double& lsample, double& rsample, SampleInfo& info);
	std::vector<EffectBuilder*> get_effect_builders() {
		return engine.get_effect_builders();
	}
	std::vector<MidiCubeInput> get_inputs();

	~MidiCube();
};


#endif /* MIDICUBE_MIDICUBE_H_ */
