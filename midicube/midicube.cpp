/*
 * midicube.cpp
 *
 *  Created on: Oct 3, 2020
 *      Author: jojo
 */

#include "midicube.h"

MidiCube::MidiCube() {

}

void MidiCube::init() {
	//MIDI Inputs
	//Input-Devices
	MidiInput input_dummy;
	size_t n_ports = input_dummy.available_ports();
	for (size_t i = 0; i < n_ports; ++i) {
		MidiInput* input = new MidiInput();
		std::string name;
		try {
			name = input->port_name(i);
			input->open(i);
		}
		catch (MidiException& exc) {
			delete input;
			input = nullptr;
		}
		if (input != nullptr) {
			inputs.push_back({input, name});
			size_t index = inputs.size() - 1;
			MidiCube* self = this;
			input->set_callback([index, self](double delta, MidiMessage& msg) {
				self->midi_callback(msg, index);
			});
		}
	}
	//Init audio
	audio_handler.init();
}

void MidiCube::process(std::array<double, OUTPUT_CHANNELS>& channels, SampleInfo& info) {

}

void MidiCube::midi_callback(MidiMessage& message, size_t input) {

}

MidiCube::~MidiCube() {

}
