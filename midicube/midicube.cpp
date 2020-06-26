/*
 * midicube.cpp
 *
 *  Created on: 17 Jun 2020
 *      Author: jojo
 */

#include "midicube.h"
#include <iostream>


static double process_func(unsigned int channel, double time, void* user_data) {
	return ((MidiCube*) user_data)->process(channel, time);
}

static void midi_callback(double deltatime, MidiMessage& msg, void* arg) {
	std::cout << "Callback" << std::endl;
	std::cout << msg.to_string() << std::endl;
}

MidiCube::MidiCube() {
	audio_handler.set_sample_callback(&process_func, this);
};

void MidiCube::init() {
	create_default_devices();
	audio_handler.init();
};

double MidiCube::process(unsigned int channel, double time) {
	double sig = 0;
	for (std::pair<std::string, AudioDevice*> device : devices) {
		sig += device.second->process_sample(channel, time);
	}
	return sig;
};

void MidiCube::create_default_devices() {
	using namespace std::string_literals;
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
			std::cerr << "Error opening MIDI Input: " << exc.what() << std::endl;
		}
		if (input != nullptr) {
			PortInputDevice* device = new PortInputDevice(input, "[MIDIIN] "s + name);
			device->set_midi_callback(&midi_callback, nullptr);
			add_device(device);
		}
	}
	//Sound Engine
	SoundEngineDevice* device = new SoundEngineDevice("Sound Engine");
	add_device(device);
	std::cout << devices.size() << std::endl;
};

/**
 * Device must be allocated on the heap
 */
void MidiCube::add_device(AudioDevice* device) {
	if (devices.find(device->get_identifier()) == devices.end()) {
		devices[device->get_identifier()] = device;
	}
};

MidiCube::~MidiCube() {
	for (std::pair<std::string, AudioDevice*> device : devices) {
		delete device.second;
	}
	devices.clear();
};

