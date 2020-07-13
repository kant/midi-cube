/*
 * synthesis.cpp
 *
 *  Created on: 20 Jun 2020
 *      Author: jojo
 */

#include "synthesis.h"
#include <cmath>


extern double note_to_freq_transpose (double tnote) {
	return pow(2, (tnote)/12.0);
}

extern double note_to_freq (double note) {
	return 440.0 * pow(2, (note - 69)/12.0);
}

extern double freq_to_radians(double freq) {
	return 2 * M_PI * freq;
}

extern double sine_wave(double time, double freq) {
	return sin(freq_to_radians(freq) * time);
}

extern double square_wave(double time, double freq) {
	return copysign(1.0, sin(freq_to_radians(freq) * time));
}

extern double saw_wave(double time, double freq) {
	double interval = 1.0/freq;
	return fmod(time, interval)/interval * 2 - 1;
}

/**
 * The arguments are irrelevant here, they are just there if a function pointer to a generic wave function is needed
 */
extern double noise_wave(double time, double freq) {
	return ((double) rand())/RAND_MAX* 2 - 1;
}


//DelayBuffer
void DelayBuffer::add_sample(double sample, unsigned int delay) {
	if (delay < buffer.size()) {
		buffer[(index + delay) % buffer.size()] += sample;
	}
}

void DelayBuffer::add_sample(double sample, unsigned int delay, unsigned int repetition, unsigned int rep_delay, double factor) {
	for (unsigned int i = 0; i < repetition; ++i) {
		add_sample(sample, delay);
		sample *= factor;
		delay += rep_delay;
	}
}

double DelayBuffer::process() {
	double sample = buffer[index];
	buffer[index] = 0;
	index++;
	index %= buffer.size();
	return sample;
}

//LowPassFilter
LowPassFilter::LowPassFilter(double cutoff) {
	this->cutoff = cutoff;
	this->last = 0;
	this->rc = 1.0/(2 * M_PI * cutoff);
}

double LowPassFilter::apply (double sample, double time_step) {
	double filtered = 0;
	double a = time_step / (rc + time_step);

	filtered = a * sample + (1 - a) * last;

	last = filtered;
	return filtered;
}

void LowPassFilter::set_cutoff(double cutoff) {
	this->cutoff = cutoff;
	this->rc = 1.0/(2 * M_PI * cutoff);
}

double LowPassFilter::get_cutoff() {
	return cutoff;
}

