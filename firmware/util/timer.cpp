#include "ch.hpp"

#include "timer.h"

#define US_PER_SECOND_F 1000000.0

Timer::Timer() {
	init();
}

static systimestamp_t getTimestamp() {
	chibios_rt::CriticalSectionLocker csl;
	return chVTGetTimeStampI();
}

void Timer::reset() {
	reset(getTimestamp());
}

void Timer::reset(systimestamp_t stamp) {
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

bool Timer::hasElapsedUs(float microseconds) const {
	auto delta = getTimestamp() - m_lastReset;

	// If larger than 32 bits, timer has certainly expired
	if (delta >= UINT32_MAX) {
		return true;
	}

	auto delta32 = (uint32_t)delta;

	return delta32 > TIME_US2I(microseconds);
}


float Timer::getElapsedSeconds() const {
	return getElapsedSeconds(getTimestamp());
}

float Timer::getElapsedSeconds(systimestamp_t stamp) const {
	return 1 / US_PER_SECOND_F * getElapsedUs(stamp);
}

float Timer::getElapsedUs() const {
	return getElapsedUs(getTimestamp());
}

float Timer::getElapsedUs(systimestamp_t stamp) const {
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

	return TIME_I2US(delta32);
}

float Timer::getElapsedSecondsAndReset() {
	auto stamp = getTimestamp();

	float result = getElapsedSeconds(stamp);

	reset(stamp);

	return result;
}
