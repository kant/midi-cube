/*
 * voice.cpp
 *
 *  Created on: Feb 10, 2021
 *      Author: jojo
 */

#include "voice.h"

//Arpeggiator
Arpeggiator::Arpeggiator() {

}

void Arpeggiator::apply(SampleInfo& info, Metronome& master, std::function<void(SampleInfo&, unsigned int, double)> press, std::function<void(SampleInfo&, unsigned int)> release, bool sustain) {
	//Clean sustained notes
	if (!preset.hold && preset.sustain && !sustain) {
		for (size_t i = 0; i < this->note.note.size(); ++i) {
			if (!this->note.note[i].pressed) {
				this->note.note[i].valid = false;
			}
		}
	}
	//Reset if no keys are pressed
	if (!restart) {
		bool released = true;
		for (size_t i = 0; i < this->note.note.size() && released; ++i) {
			released = !this->note.note[i].valid;
		}
		restart = released;
	}
	if (restart) {
		second = false;
		//Keyboard sync
		if (preset.kb_sync) {
			metronome.init(info.sample_time);
		}
	}
	//Pattern
	if (preset.master_sync ? master.is_beat(info.sample_time, info.sample_rate, preset.value) : metronome.is_beat(info.sample_time, info.sample_rate, preset.value)) {
		int next_note = 128;
		long int next_index = -1;

		unsigned int lowest_note = 128;
		long int lowest_index = -1;
		int highest_note = -1;
		long int highest_index = -1;

		switch (preset.pattern) {
		case ArpeggiatorPattern::ARP_UP:
			for (size_t i = 0; i < this->note.note.size(); ++i) {
				if (this->note.note[i].valid) {
					if (this->note.note[i].note < lowest_note + preset.play_duplicates) {
						lowest_note = this->note.note[i].note;
						lowest_index = i;
					}
					//Find next highest note
					for (unsigned int octave = 0; octave < preset.octaves; ++octave) {
						int n = this->note.note[i].note + octave * 12;
						if ((n < next_note || (n == next_note && i < (size_t) next_index)) && (n > (int) curr_note || (n == (int) curr_note && preset.play_duplicates && i > note_index))) {
							next_note = n;
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
			next_note = -1;
			for (size_t i = 0; i < this->note.note.size(); ++i) {
				if (this->note.note[i].valid) {
					if ((int) (this->note.note[i].note + (preset.octaves - 1) * 12) > highest_note - preset.play_duplicates) {
						highest_note = this->note.note[i].note  + (preset.octaves - 1) * 12;
						highest_index = i;
					}
					//Find next lowest note
					for (unsigned int o = 0; o < preset.octaves; ++o) {
						int octave = preset.octaves - o - 1;
						int n = this->note.note[i].note + octave * 12;
						if ((n > next_note || (n == next_note && i < (size_t) next_index)) && (n < (int) curr_note  || (n == (int) curr_note && preset.play_duplicates && i > note_index))) {
							next_note = n;
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
			if (second) {
				//Down
				next_note = -1;
				for (size_t i = 0; i < this->note.note.size(); ++i) {
					if (this->note.note[i].valid) {
						if (this->note.note[i].note < lowest_note + preset.play_duplicates) {
							lowest_note = this->note.note[i].note;
							lowest_index = i;
						}
						//Find next lowest note
						for (unsigned int o = 0; o < preset.octaves; ++o) {
							int octave = preset.octaves - o - 1;
							int n = this->note.note[i].note + octave * 12;
							if ((n > next_note || (n == next_note && i < (size_t) next_index)) && (n < (int) curr_note  || (n == (int) curr_note && preset.play_duplicates && i > note_index))) {
								next_note = n;
								next_index = i;
								break;
							}
						}
					}
				}
				//Restart from lowest
				if (next_index < 0 || (!preset.repeat_edges && next_index == lowest_index && next_note == (int) lowest_note)) {
					next_note = lowest_note;
					next_index = lowest_index;
					second = !second;
				}
			}
			else {
				//Up
				for (size_t i = 0; i < this->note.note.size(); ++i) {
					if (this->note.note[i].valid) {
						if ((int) (this->note.note[i].note + (preset.octaves - 1) * 12) > highest_note - preset.play_duplicates) {
							highest_note = this->note.note[i].note  + (preset.octaves - 1) * 12;
							highest_index = i;
						}
						if (this->note.note[i].note < lowest_note + preset.play_duplicates) {
							lowest_note = this->note.note[i].note;
							lowest_index = i;
						}
						//Find next highest note
						for (unsigned int octave = 0; octave < preset.octaves; ++octave) {
							int n = this->note.note[i].note + octave * 12;
							if ((n < next_note || (n == next_note && i < (size_t) next_index)) && (n > (int) curr_note || (n == (int) curr_note && preset.play_duplicates && i > note_index))) {
								next_note = n;
								next_index = i;
								break;
							}
						}
					}
				}
				//Restart from highest
				if (next_index < 0 || (!preset.repeat_edges && next_index == highest_index && next_note == highest_note)) {
					next_note = highest_note;
					next_index = highest_index;
					second = !second;
				}
			}
			//Restart from lowest
			if (restart) {
				next_note = lowest_note;
				next_index = lowest_index;
				second = false;
			}
			break;
		case ArpeggiatorPattern::ARP_DOWN_UP:
			if (second) {
				//Up
				for (size_t i = 0; i < this->note.note.size(); ++i) {
					if (this->note.note[i].valid) {
						if ((int) (this->note.note[i].note + (preset.octaves - 1) * 12) > highest_note - preset.play_duplicates) {
							highest_note = this->note.note[i].note  + (preset.octaves - 1) * 12;
							highest_index = i;
						}
						//Find next highest note
						for (unsigned int octave = 0; octave < preset.octaves; ++octave) {
							int n = this->note.note[i].note + octave * 12;
							if ((n < next_note || (n == next_note && i < (size_t) next_index)) && (n > (int) curr_note || (n == (int) curr_note && preset.play_duplicates && i > note_index))) {
								next_note = n;
								next_index = i;
								break;
							}
						}
					}
				}
				//Restart from highest
				if (next_index < 0 || (!preset.repeat_edges && next_index == highest_index && next_note == highest_note)) {
					next_note = highest_note;
					next_index = highest_index;
					second = !second;
				}
			}
			else {
				//Down
				next_note = -1;
				for (size_t i = 0; i < this->note.note.size(); ++i) {
					if (this->note.note[i].valid) {
						if (this->note.note[i].note < lowest_note + preset.play_duplicates) {
							lowest_note = this->note.note[i].note;
							lowest_index = i;
						}
						if ((int) (this->note.note[i].note + (preset.octaves - 1) * 12) > highest_note - preset.play_duplicates) {
							highest_note = this->note.note[i].note  + (preset.octaves - 1) * 12;
							highest_index = i;
						}
						//Find next lowest note
						for (unsigned int o = 0; o < preset.octaves; ++o) {
							int octave = preset.octaves - o - 1;
							int n = this->note.note[i].note + octave * 12;
							if ((n > next_note || (n == next_note && i < (size_t) next_index)) && (n < (int) curr_note || (n == (int) curr_note && preset.play_duplicates && i > note_index))) {
								next_note = n;
								next_index = i;
								break;
							}
						}
					}
				}
				//Restart from lowest
				if (next_index < 0 || (!preset.repeat_edges && next_index == lowest_index && next_note == (int) lowest_note)) {
					next_note = lowest_note;
					next_index = lowest_index;
					second = !second;
				}
			}
			//Restart from highest
			if (restart) {
				next_note = highest_note;
				next_index = highest_index;
				second = false;
			}
			break;
		case ArpeggiatorPattern::ARP_RANDOM:
			//Count valid notes
			size_t count = 0;
			for (size_t i = 0; i < this->note.note.size(); ++i) {
				count += this->note.note[i].valid;
			}
			if (count > 0) {
				size_t next = rand() % count;
				size_t octave = rand() % preset.octaves;
				//Find next note
				count = 0;
				for (size_t i = 0; i < this->note.note.size() && count <= next; ++i) {
					if (this->note.note[i].valid) {
						next_note = this->note.note[i].note + octave * 12;
						next_index = i;
						++count;
					}
				}
			}

			break;
		}
		//Press note
		release(info, curr_note);
		if (next_index >= 0) {
			curr_note = next_note;
			this->note_index = next_index;
			press(info, curr_note, this->note.note[note_index].velocity);
			restart = false;

			//Clean held notes
			if (!preset.hold && !(sustain || preset.sustain)) {
				bool released = true;
				for (size_t i = 0; i < this->note.note.size() && released; ++i) {
					released = !this->note.note[i].pressed;
				}
				if (released) {
					for (size_t i = 0; i < this->note.note.size(); ++i) {
						this->note.note[i].valid = false;
					}
				}
			}
		}
	}
}

void Arpeggiator::press_note(SampleInfo& info, unsigned int note, double velocity, bool sustain) {
	//Clean held notes
	if (preset.hold) {
		bool released = true;
		for (size_t i = 0; i < this->note.note.size() && released; ++i) {
			released = !this->note.note[i].pressed;
		}
		if (released) {
			for (size_t i = 0; i < this->note.note.size(); ++i) {
				this->note.note[i].valid = false;
			}
		}
	}
	this->note.press_note(info, note, note, velocity);
}

void Arpeggiator::release_note(SampleInfo& info, unsigned int note, bool sustain) {
	this->note.release_note(info, note, !preset.hold && (!sustain || !preset.sustain));
}


