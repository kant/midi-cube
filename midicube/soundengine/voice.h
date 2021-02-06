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

	void press_note(SampleInfo& info, unsigned int note, double velocity);
	void release_note(SampleInfo& info, unsigned int note, bool invalidate = false);

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
void VoiceManager<V, N>::press_note(SampleInfo& info, unsigned int note, double velocity) {
	size_t slot = next_freq_slot(info);
	TriggeredNote& n = this->note[slot].note;
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
void VoiceManager<V, N>::release_note(SampleInfo& info, unsigned int note, bool invalidate) {
	for (size_t i = 0; i < N; ++i) {
		TriggeredNote& n = this->note[i].note;
		if (n.note == note && n.pressed) {
			n.pressed = false;
			n.release_time = info.time;
			if (invalidate) {
				n.valid = false;
			}
		}
	}
}




#endif /* MIDICUBE_SOUNDENGINE_VOICE_H_ */
