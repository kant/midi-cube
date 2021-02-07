/*
 * voice.h
 *
 *  Created on: Feb 6, 2021
 *      Author: jojo
 */

#ifndef MIDICUBE_SOUNDENGINE_VOICE_H_
#define MIDICUBE_SOUNDENGINE_VOICE_H_

#include "../envelope.h"
#include "../synthesis.h"
#include "../metronome.h"
#include <functional>

struct SimpleVoice {
	TriggeredNote note;
};

template<typename V, size_t N>
class VoiceManager {
private:
	inline size_t next_freq_slot(SampleInfo& info);

public:
	std::array<V, N> note;

	VoiceManager();

	void press_note(SampleInfo& info, unsigned int channel, unsigned int note, double velocity);
	void release_note(SampleInfo& info, unsigned int channel, unsigned int note, bool invalidate = false);

};

template<typename V, size_t N>
VoiceManager<V, N>::VoiceManager () {
	//Init notes
	for (size_t i = 0; i < N; ++i) {
		note[i].note.start_time = -1024;
		note[i].note.release_time = -1024;
	}
}

template<typename V, size_t N>
size_t VoiceManager<V, N>::next_freq_slot(SampleInfo& info) {
	for (size_t i = 0; i < N; ++i) {
		if (!note[i].note.valid) {
			return i;
		}
	}
	//TODO return longest used slot
	return 0;
}

template<typename V, size_t N>
void VoiceManager<V, N>::press_note(SampleInfo& info, unsigned int channel, unsigned int note, double velocity) {
	size_t slot = next_freq_slot(info);
	TriggeredNote& n = this->note[slot].note;
	n.channel = channel;
	n.freq = note_to_freq(note);
	n.velocity = velocity;
	n.note = note;
	n.pressed = true;
	n.start_time = info.time;
	n.release_time = 0;
	n.phase_shift = 0;
	n.valid = true;
}

template<typename V, size_t N>
void VoiceManager<V, N>::release_note(SampleInfo& info, unsigned int channel, unsigned int note, bool invalidate) {
	for (size_t i = 0; i < N; ++i) {
		TriggeredNote& n = this->note[i].note;
		if (n.channel == channel && n.note == note && n.pressed) {
			n.pressed = false;
			n.release_time = info.time;
			if (invalidate) {
				n.valid = false;
			}
		}
	}
}

enum ArpeggiatorPattern {
	ARP_UP, ARP_DOWN, ARP_RANDOM, ARP_UP_DOWN, ARP_UP_CUSTOM, ARP_DOWN_CUSTOM
};

struct ArpeggiatorPreset {
	ArpeggiatorPattern pattern;
	std::vector<unsigned int> data;
	unsigned int octaves = 1;
	int value = 1;
	bool hold = false;
};

#define ARPEGGIATOR_POLYPHONY 30

class Arpeggiator {

private:
	unsigned int curr_note = 0;
	std::size_t data_index = 0;
	std::size_t note_index = 0;
	bool restart = true;

public:
	bool on = false;
	ArpeggiatorPreset preset;
	VoiceManager<SimpleVoice, ARPEGGIATOR_POLYPHONY> note;
	Metronome metronome;

	Arpeggiator();

	void apply(SampleInfo& info, std::function<void(SampleInfo&, unsigned int, unsigned int, double)> press, std::function<void(SampleInfo&, unsigned int, unsigned int)> release);

	void press_note(SampleInfo& info, unsigned int note, double velocity);

	void release_note(SampleInfo& info, unsigned int note);

};


#endif /* MIDICUBE_SOUNDENGINE_VOICE_H_ */
