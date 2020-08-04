/*
 * envelope.cpp
 *
 *  Created on: 1 Jul 2020
 *      Author: jojo
 */

#include "envelope.h"
#include <cmath>

double ADSREnvelope::amplitude(double time, TriggeredNote& note, KeyboardEnvironment& env) {
	double release_time = fmax(note.release_time, env.sustain_release_time);
	if (note.pressed || (env.sustain && env.sustain_time <= note.release_time)) {
		//Attack
		if (time <= note.start_time + attack) {
			return (time - note.start_time)/attack;
		}
		//Decay
		else if (time <= note.start_time + attack + decay) {
			return 1 - (time - note.start_time - attack)/decay * (1 - sustain);
		}
		//Sustain
		else {
			return sustain;
		}
	}
	else if (time - release_time > release) {
		return 0;
	}
	else {
		return sustain - (time - release_time)/release * sustain;
	}

	return 0;
}

double ADSREnvelope::is_finished(double time, TriggeredNote& note, KeyboardEnvironment& env) {
	double release_time = fmax(note.release_time, env.sustain_release_time);
	return !note.pressed && (!env.sustain || env.sustain_time > note.release_time) && time - release_time > release;
}

