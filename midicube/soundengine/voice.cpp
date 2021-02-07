/*
 * voice.cpp
 *
 *  Created on: Feb 6, 2021
 *      Author: jojo
 */

#include "voice.h"


//Arpeggiator
Arpeggiator::Arpeggiator() {

}

void Arpeggiator::apply(SampleInfo& info, std::function<void(SampleInfo&, unsigned int, unsigned int, double)> press, std::function<void(SampleInfo&, unsigned int, unsigned int)> release) {
	//Reset if no keys are pressed
	if (!restart) {
		bool released = true;
		for (size_t i = 0; i < this->note.note.size() && released; ++i) {
			released = !this->note.note[i].note.pressed;
		}
		restart = released;
	}
	if (restart) {
		//Keyboard sync
		metronome.init(info.sample_time);
	}
	//Pattern
	if (metronome.is_beat(info.sample_time, info.sample_rate, preset.value)) {
		unsigned int next_note = 128;
		long int next_index = -1;

		unsigned int lowest_note = 128;
		long int lowest_index = -1;
		int highest_note = -1;
		long int highest_index = -1;

		switch (preset.pattern) {
		case ArpeggiatorPattern::ARP_UP:
			for (size_t i = 0; i < this->note.note.size(); ++i) {
				TriggeredNote& n = note.note[i].note;
				if (n.pressed) {
					if (n.note < lowest_note) {
						lowest_note = n.note;
						lowest_index = i;
					}
					//Find next highest note
					for (unsigned int octave = 0; octave < preset.octaves; ++octave) {
						unsigned int no = n.note + octave * 12;
						if (no < next_note && (no > curr_note || (no == curr_note && note_index > i))) {
							next_note = no;
							next_index = i;
							break;
						}
					}
				}
			}
			//Restart from lowest
			if (restart || next_index < 0) {
				next_note = lowest_note;
				next_index = lowest_index;
			}
			break;
		case ArpeggiatorPattern::ARP_DOWN:
			for (size_t i = 0; i < this->note.note.size(); ++i) {
				TriggeredNote& n = note.note[i].note;
				if (n.pressed) {
					if ((int) (n.note + (preset.octaves - 1) * 12) > highest_note) {
						highest_note = n.note  + (preset.octaves - 1) * 12;
						highest_index = i;
					}
					//Find next lowest note
					for (unsigned int o = 0; o < preset.octaves; ++o) {
						unsigned int octave = preset.octaves - o - 1;
						unsigned int no = n.note + octave * 12;
						if (no > next_note && (no > curr_note || (no == curr_note && note_index > i))) {
							next_note = no;
							next_index = i;
							break;
						}
					}
				}
			}
			//Restart from highest
			if (restart || next_index < 0) {
				next_note = highest_note;
				next_index = highest_index;
			}
			break;
		case ArpeggiatorPattern::ARP_UP_DOWN:
			//TODO
			break;
		case ArpeggiatorPattern::ARP_RANDOM:
			//TODO
			break;
		case ArpeggiatorPattern::ARP_UP_CUSTOM:
			break;
		case ArpeggiatorPattern::ARP_DOWN_CUSTOM:
			break;
		}
		//Press note
		release(info, this->note.note[note_index].note.channel, curr_note);
		if (next_index >= 0) {
			curr_note = next_note;
			this->note_index = next_index;
			press(info, this->note.note[note_index].note.channel, curr_note, this->note.note[note_index].note.velocity);
			restart = false;
		}
	}
}

void Arpeggiator::press_note(SampleInfo& info, unsigned int note, double velocity) {
	this->note.press_note(info, 0 /* FIXME */, note, velocity);
}

void Arpeggiator::release_note(SampleInfo& info, unsigned int note) {
	this->note.release_note(info, note, true);
}
