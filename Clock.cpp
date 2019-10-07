#include "Clock.h"
using std::chrono::time_point;
using std::chrono::high_resolution_clock;
using std::chrono::duration;
using std::chrono::duration_cast;
Time Clock::getElapsedTime() const
{
	const time_point<high_resolution_clock> now = high_resolution_clock::now();
	return Time::seconds(
		duration_cast<duration<float>>(now - clockStart).count());
}
Time Clock::restart()
{
	const time_point<high_resolution_clock> clockStartPrev = clockStart;
	const time_point<high_resolution_clock> now = high_resolution_clock::now();
	clockStart = now;
	return Time::seconds(
		duration_cast<duration<float>>(now - clockStartPrev).count());
}