#ifndef CLOCK_HPP
#define CLOCK_HPP

#include <chrono>


// User libs
#include "misc/data_types.hpp"

class Clock {
private:
	std::chrono::high_resolution_clock::time_point start_time;
	std::chrono::high_resolution_clock::time_point end_time;
	std::chrono::nanoseconds duration;
public:
	Clock();
	~Clock();

	void StartTimer();
	void EndTimer();
	long long GetDuration(const char& type = *"s");
};




#endif // !CLOCK_H
