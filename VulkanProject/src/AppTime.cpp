#include "AppTime.hpp"


bool AppTime::initialized = false;

float AppTime::startTimeValue;
float AppTime::timeValue;
float AppTime::deltaTimeValue;

timeStamp AppTime::startTimeStamp;

void AppTime::updateDeltaTime() {

	if (!initialized) {
		initialized = true;
		startTimeStamp = std::chrono::high_resolution_clock::now();
		startTimeValue = startTimeStamp.time_since_epoch().count();
	}

	auto currentTime = std::chrono::high_resolution_clock::now();
	timeValue = currentTime.time_since_epoch().count();
	deltaTimeValue = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTimeStamp).count();
}
