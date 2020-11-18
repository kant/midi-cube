/*
 * filter.h
 *
 *  Created on: 26 Aug 2020
 *      Author: jojo
 */

#ifndef MIDICUBE_FILTER_H_
#define MIDICUBE_FILTER_H_
#include <array>
#include <type_traits>
#include "audio.h"

enum class FilterType {
	LP_12, LP_24, HP_12, HP_24
};

struct FilterData {
	FilterType type;
	double cutoff = 21000;
};

struct FilterCache {
	double last_filtered = 0;
	double last = 0;
	bool started = false;
};

class Filter {
private:
	std::array<FilterCache, 4> cache;
public:
	double apply (FilterData& data, double sample, double time_step) = 0;

};

#endif /* MIDICUBE_FILTER_H_ */
