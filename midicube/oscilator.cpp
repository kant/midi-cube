/*
 * oscilator.cpp
 *
 *  Created on: 15 Jul 2020
 *      Author: jojo
 */

#include "oscilator.h"
#include <cmath>
#include <iostream>

//AnalogOscilator
AnalogOscilator::AnalogOscilator() {

}

static double polyblep(double phase, double step) {
	if (phase < step) {
		phase /= step;
		return - phase * phase + 2 * phase - 1;
	}
	else if (phase > (1 - step)) {
		phase = (phase - 1)/step;
		return phase * phase + 2 * phase + 1;
	}
	return 0;
}

static double integrated_polyblep(double phase, double step) {
	if (phase < step) {
		phase = phase / step - 1;
		return - 1 / 3.0 * phase * phase * phase;
	}
	else if (phase > (1 - step)) {
		phase = (phase - 1) / step + 1;
		return 1 / 3.0 * phase * phase * phase;
	}
	return 0;
}

AnalogOscilatorSignal AnalogOscilator::signal(double freq, double time_step, AnalogOscilatorData& data) {
	//Move
	double last_rotation = rotation;
	double step = freq * time_step;
	rotation += step;

	double phase = rotation - (long int) rotation;
	//Update parameters
	if (phase < step) {
		pulse_width = data.pulse_width;
	}
	//Sync
	double sync_step = data.sync_mul * freq * time_step;
	double sync_phase = 0;
	if (data.sync) {
		sync_rotation += sync_step;
		sync_phase = sync_rotation - (long int) sync_rotation;
		//Sync now
		if (sync_phase < sync_step) {
			//Reset phase
			rotation += 1 - phase + sync_step;
			phase = rotation - (long int) rotation;
		}
		//Sync next sample
		else if (sync_phase + sync_step >= 1) {
			last_phase = phase;
		}
	}
	//Compute wave
	AnalogOscilatorSignal signal = {0, 0};
	switch(data.waveform) {
	case AnalogWaveForm::SINE:
		//Modulator
		signal.modulator = (-cosine_wave(rotation, 1) + cosine_wave(last_rotation, 1))/(time_step * 2 * M_PI);
		signal.carrier = sine_wave(rotation, 1);
		if (data.analog && data.sync && data.sync_mul != 1) {
			signal.carrier -= polyblep(sync_phase, sync_step) * (sine_wave(last_phase, 1) + 1) / 2;
		}
		break;
	case AnalogWaveForm::SAW_DOWN:
		signal.carrier = saw_wave_down(rotation, 1);
		if (data.analog) {
			signal.carrier += polyblep(phase, step);
			if (data.sync && data.sync_mul != 1) {
				signal.carrier += polyblep(sync_phase, sync_step) * (1 - (saw_wave_down(last_phase, 1) + 1) / 2);
			}
		}
		signal.modulator = 0;
		break;
	case AnalogWaveForm::SAW_UP:
		signal.carrier = saw_wave_up(rotation, 1);
		if (data.analog) {
			signal.carrier -= polyblep(phase, step);
			if (data.sync && data.sync_mul != 1) {
				signal.carrier += polyblep(sync_phase, sync_step) * (saw_wave_up(last_phase, 1) + 1) / 2;
			}
		}
		signal.modulator = 0;
		break;
	case AnalogWaveForm::SQUARE:
		signal.carrier = square_wave(rotation, 1, pulse_width);
		if (data.analog) {
			signal.carrier += polyblep(phase, step);
			double protation = rotation + pulse_width;
			signal.carrier -= polyblep(protation - (long int) protation, step);

			if (data.sync && data.sync_mul != 1) {
				signal.carrier += polyblep(sync_phase, sync_step) *  (1 - (square_wave(rotation, 1, pulse_width) + 1) / 2);
			}
		}
		signal.modulator = 0;
		break;
	case AnalogWaveForm::TRIANGLE_WAVE:
		signal.carrier = triangle_wave(rotation, 1, pulse_width); //TODO PWM
		if (data.analog) {
			double mul1;
			double mul2;
			if (phase < pulse_width) {
				mul1 = 1/pulse_width;
				mul2 = 1/(1 - pulse_width);
			}
			else {
				mul1 = 1/(1 - pulse_width);
				mul2 = 1/pulse_width;
			}
			//TODO
			signal.carrier += integrated_polyblep(phase, step) * 2 * mul1 * step;
			double protation = rotation + pulse_width;
			double pphase = protation - (long int) protation;
			signal.carrier -= integrated_polyblep(pphase, step) * 2 * mul2 * step;
		}
		//TODO sync
		signal.modulator = 0;
		break;
	case AnalogWaveForm::NOISE:
		signal.carrier = noise_wave(rotation, 1);
		signal.modulator = 0;
		break;
	}
	return signal;
}

void AnalogOscilator::reset() {
	rotation = 0;
}

void AnalogOscilator::randomize() {
	rotation = ((double) rand())/RAND_MAX;
}


AnalogOscilator::~AnalogOscilator() {

}

