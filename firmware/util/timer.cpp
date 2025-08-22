#include <cstdint>
#include "timer.h"

#define US_PER_SECOND_F 1000000.0

Timer::Timer() {
	init();
}

#ifdef MOCK_TIMER

#define CH_CFG_ST_FREQUENCY 1000000

// in mock land, ticks == microseconds
#define TIME_US2I(us) (us)
#define TIME_I2US(ticks) (ticks)

/*static*/ int64_t Timer::mockTimeStamp = 0;
int64_t Timer::getTimestamp() const {
	return Timer::mockTimeStamp;
}

/*static*/ void Timer::setMockTime(int64_t stamp) {
	Timer::mockTimeStamp = stamp;
}

/*static*/ void Timer::advanceMockTime(int64_t increment) {
	Timer::mockTimeStamp += increment;
}

#else
#include "ch.hpp"

int64_t Timer::getTimestamp() const {
	// Ensure that our timestamp type is compatible with the one ChibiOS returns
	static_assert(sizeof(int64_t) == sizeof(systimestamp_t));

	return chVTGetTimeStamp();
}
#endif // MOCK_TIMER

void Timer::reset() {
	reset(getTimestamp());
}

void Timer::reset(int64_t stamp) {
	m_lastReset = stamp;
}

void Timer::init() {
	// Use not-quite-minimum value to avoid overflow
	m_lastReset = INT64_MIN / 8;
}

bool Timer::hasElapsedSec(float seconds) const {
	return hasElapsedMs(seconds * 1000);
}

bool Timer::hasElapsedMs(float milliseconds) const {
	return hasElapsedUs(milliseconds * 1000);
}

static const float usPerTick = 1000000.0 / CH_CFG_ST_FREQUENCY;

bool Timer::hasElapsedUs(float microseconds) const {
	auto delta = getTimestamp() - m_lastReset;

	// If larger than 32 bits, timer has certainly expired
	if (delta >= UINT32_MAX) {
		return true;
	}

	auto delta32 = (uint32_t)delta;

	return delta32 > (microseconds / usPerTick);
}

float Timer::getElapsedSeconds() const {
	return getElapsedSeconds(getTimestamp());
}

float Timer::getElapsedSeconds(int64_t stamp) const {
	return 1 / US_PER_SECOND_F * getElapsedUs(stamp);
}

float Timer::getElapsedUs() const {
	return getElapsedUs(getTimestamp());
}

float Timer::getElapsedUs(int64_t stamp) const {
	auto deltaNt = stamp - m_lastReset;

	// Yes, things can happen slightly in the future if we get a lucky interrupt between
	// the timestamp and this subtraction, that updates m_lastReset to what's now "the future",
	// resulting in a negative delta.
	if (deltaNt < 0) {
		return 0;
	}

	if (deltaNt > UINT32_MAX - 1) {
		deltaNt = UINT32_MAX - 1;
	}

	auto delta32 = (uint32_t)deltaNt;

	return delta32 * usPerTick;
}

float Timer::getElapsedSecondsAndReset() {
	auto stamp = getTimestamp();

	float result = getElapsedSeconds(stamp);

	reset(stamp);

	return result;
}
