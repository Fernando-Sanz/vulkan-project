#pragma once

#include <chrono>


using timeStamp = std::chrono::steady_clock::time_point;

class AppTime {
public:

	static float time() { return timeValue; }
	static float deltaTime() { return deltaTimeValue; }
	static float startTime() { return startTimeValue; }

	static void updateDeltaTime();

private:
	static bool initialized;

	static float startTimeValue;
	static float timeValue;
	static float deltaTimeValue;

	static timeStamp startTimeStamp;
};
