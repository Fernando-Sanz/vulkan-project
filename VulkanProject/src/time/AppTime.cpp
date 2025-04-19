#include "time/AppTime.hpp"


bool AppTime::initialized = false;

float AppTime::startTimeValue;
float AppTime::timeValue;
float AppTime::deltaTimeValue;

timeStamp AppTime::startTimeStamp;
timeStamp AppTime::lastTimeStamp;

namespace {

	float parseChronoValue(std::chrono::steady_clock::duration& duration) {
		return std::chrono::duration_cast<std::chrono::nanoseconds>(duration).count();
	}

	float parseChronoValue(timeStamp time) {
		return parseChronoValue(time.time_since_epoch());
	}
}

void AppTime::updateDeltaTime() {

	if (!initialized) {
		initialized = true;
		startTimeStamp = std::chrono::high_resolution_clock::now();
		startTimeValue = parseChronoValue(startTimeStamp);
		lastTimeStamp = startTimeStamp;
	}

	auto currentTimeStamp = std::chrono::high_resolution_clock::now();

	timeValue = parseChronoValue(currentTimeStamp - startTimeStamp);
	deltaTimeValue = parseChronoValue(currentTimeStamp - lastTimeStamp);

	lastTimeStamp = currentTimeStamp;
}
