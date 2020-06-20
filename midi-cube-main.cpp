#include <iostream>
#include "midicube/audio.h"
#include "midicube/synthesis.h"
using namespace std;

double process(unsigned int channel, double time) {
	return square_wave(time, 440.0);
}

int main(int argc, char **argv) {
	cout << "Playing a cool square wave sound!" << endl;

	AudioHandler handler;
	try {
		handler.set_sample_callback(process);
		handler.init();
	}
	catch (AudioException& e) {
		cerr << e.what() << endl;
	}
	cout << "Press any key to exit!" << endl;
	getchar();

	return 0;
}
