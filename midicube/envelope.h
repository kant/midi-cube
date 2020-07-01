/*
 * envelope.h
 *
 *  Created on: 1 Jul 2020
 *      Author: jojo
 */

#ifndef MIDICUBE_ENVELOPE_H_
#define MIDICUBE_ENVELOPE_H_

struct TriggeredNote {
	double start_time = 0;
	bool pressed = false;
	double release_time = 0;
	unsigned int note = 0;
	double freq = 0;
	double velocity = 0;
	double aftertouch = 0;
};

struct ADSREnvelope {
	double attack;
	double decay;
	double sustain;
	double release;

	double amplitude(double time, TriggeredNote& note);
};


#endif /* MIDICUBE_ENVELOPE_H_ */