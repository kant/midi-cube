/*
 * wav.h
 *
 *  Created on: 1 Jul 2020
 *      Author: jojo
 */

#ifndef MIDICUBE_AUDIOFILE_H_
#define MIDICUBE_AUDIOFILE_H_

#include <cstdint>
#include <vector>
#include <string>
#include <cmath>
#include <iostream>
#include <array>
#include <mutex>
#include <chrono>
#include "util.h"
#include "sndfile.h"

#define TIME_NOW() (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch())).count()

struct WAVHeader {
	char chunkID[4];
	std::uint32_t chunkSize;
	char riffType[4];
} __attribute__((packed));

struct WAVFormat {
	char chunkID[4];
	std::uint32_t chunkSize;
	std::uint16_t wFormatTag;
	std::uint16_t wChannels;
	std::uint32_t dwSamplesPerSec;
	std::uint32_t dwAvgBytesPerSec;
	std::uint16_t wBlockAlign;
	std::uint16_t wBitsPerSample;
} __attribute__((packed));

struct AudioSample {
	unsigned int sample_rate;
	unsigned int channels;
	unsigned int loop_start = 0;
	unsigned int loop_end = 0;
	std::vector<float> samples;

	double duration () {
		return sample_rate > 0 ? samples.size()/channels / (double) sample_rate : 0;
	}

	double sample(unsigned int channel, double time,unsigned int sample_rate ) {
		if (channels > 0) {
			channel %= channels;
			std::size_t index = (std::size_t) round(time * sample_rate) * channels + channel;
			return index < samples.size() ? samples[index] : 0;
		}
		else {
			return 0;
		}
	}

	double isample(unsigned int channel, double time, unsigned int sample_rate) {
		if (channels > 0) {
			channel %= channels;
			double index = time * sample_rate;
			std::size_t index1 = (std::size_t) floor(index)  * channels + channel;
			std::size_t index2 = (std::size_t) ceil(index) * channels + channel;
			double sample1 = index1 < samples.size() ? samples[index1] : 0;
			double sample2 = index2 < samples.size() ? samples[index2] : 0;
			double prog = 1 - index + floor(index);
			return sample1 * prog + sample2 * (1 - prog);
		}
		else {
			return 0;
		}
	}

	void clear () {
		sample_rate = 0;
		channels = 0;
		samples.clear();
	}
};

#define STREAM_AUDIO_CHUNK_SIZE 131072

struct StreamedAudioSample {
	unsigned int sample_rate = 1;
	unsigned int channels = 1;
	unsigned int loop_start = 0;
	unsigned int loop_end = 0;
	unsigned int total_size = 0;
	std::string path;
	std::array<float, STREAM_AUDIO_CHUNK_SIZE> head_samples = {};
	SpinLock lock;
	bool loaded = false;
	unsigned int last_used = 0;
	std::vector<float> samples;

	StreamedAudioSample() {

	}

	StreamedAudioSample(StreamedAudioSample& copy) = delete;

	double total_duration () {
		return (double) total_size / sample_rate;
	}

	void clear () {
		sample_rate = 1;
		channels = 1;
		total_size = 0;
		samples = {};
	}

	double isample(unsigned int channel, double time, unsigned int sample_rate) {
		if (channels > 0) {
			channel %= channels;
			double index = time * sample_rate;
			std::size_t index1 = (std::size_t) floor(index)  * channels + channel;
			std::size_t index2 = (std::size_t) ceil(index) * channels + channel;
			double sample1 = 0;
			double sample2 = 0;
			if (index2 >= head_samples.size()) {
				if (lock.try_lock()) {
					last_used = TIME_NOW();
					sample1 = index1 < samples.size() ? samples[index1] : 0;
					sample2 = index2 < samples.size() ? samples[index2] : 0;
					lock.unlock();
				}
			}
			else {
				sample1 = index1 < head_samples.size() ? head_samples[index1] : 0;
				sample2 = index2 < head_samples.size() ? head_samples[index2] : 0;
			}
			double prog = 1 - index + floor(index);
			return sample1 * prog + sample2 * (1 - prog);
		}
		else {
			return 0;
		}
	}
};

bool read_stream_audio_file(StreamedAudioSample& audio, std::string fname);

bool read_audio_file(AudioSample& audio, std::string fname);

bool write_audio_file(AudioSample& audio, std::string fname);


#endif /* MIDICUBE_AUDIOFILE_H_ */
