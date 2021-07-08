/*
 * audioloader.h
 *
 *  Created on: Jul 4, 2021
 *      Author: jojo
 */

#ifndef MIDICUBE_AUDIOLOADER_H_
#define MIDICUBE_AUDIOLOADER_H_

#include "util.h"
#include "audiofile.h"
#include "boost/lockfree/queue.hpp"
#include <thread>


struct LoadRequest {
	MultiBuffer<float>* buffer;
	size_t buffer_index;
	StreamedAudioSample* sample;
	size_t block;
};

class StreamedAudioLoader {
private:
	boost::lockfree::queue<LoadRequest> requests;
	std::atomic<bool> running{true};
	std::array<std::thread, 4> worker;

public:

	StreamedAudioLoader() : requests(1024) {

	}

	//Audio Thread
	void queue_request(LoadRequest request);
	void run();
	void stop();
	void start();

	~StreamedAudioLoader() {
		stop();
	}

};

extern StreamedAudioLoader global_audio_loader; //FIXME

#endif /* MIDICUBE_AUDIOLOADER_H_ */
