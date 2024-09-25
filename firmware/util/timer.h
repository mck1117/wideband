#pragma once

#include <cstdint>

/**
 * Helper class with "has X amount of time elapsed since most recent reset" methods
 * Brand new instances have most recent reset time far in the past, i.e. "hasElapsed" is true for any reasonable range
 */
class Timer final {
public:
	Timer();
	// returns timer to the most original-as-constructed state
	void init();

	void reset();

	bool hasElapsedSec(float seconds) const;
	bool hasElapsedMs(float ms) const;
	bool hasElapsedUs(float us) const;

	// Return the elapsed time since the last reset.
	// If the elapsed time is longer than 2^32 timer tick counts,
	// then a time period representing 2^32 counts will be returned.
	float getElapsedSeconds() const;
	float getElapsedUs() const;

	// Perform an atomic update and returning the delta between
	// now and the last reset
	float getElapsedSecondsAndReset();

	static void setMockTime(int64_t stamp);
	static void advanceMockTime(int64_t increment);

private:
	int64_t getTimestamp() const;

	void reset(int64_t stamp);
	float getElapsedSeconds(int64_t stamp) const;
	float getElapsedUs(int64_t stamp) const;

	int64_t m_lastReset;

	static int64_t mockTimeStamp;
};
