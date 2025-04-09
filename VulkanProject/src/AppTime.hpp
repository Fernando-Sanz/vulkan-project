#pragma once

#include <chrono>


using timeStamp = std::chrono::steady_clock::time_point;

class AppTime {
public:

	static float time() { return timeValue; }
	static float deltaTime() { return parseToSeconds(deltaTimeValue); }
	static float startTime() { return parseToSeconds(startTimeValue); }

	static void updateDeltaTime();

private:
	static bool initialized;

	static float startTimeValue;
	static float timeValue;
	static float deltaTimeValue;

	static timeStamp startTimeStamp;
	static timeStamp lastTimeStamp;

	inline static float parseToSeconds(float time) {
		return time / 1000000000.0f;
	}
};
