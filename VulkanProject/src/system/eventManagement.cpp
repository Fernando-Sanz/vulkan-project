#include "eventManagement.hpp"

#include <map>
#include <vector>


std::map<EventType, std::vector<EventSubscriber>> subscribers;

void addEventSubscriber(EventType type, EventSubscriber subscriber) {
	subscribers[type].push_back(subscriber);
}

static void updateSubscribers(SDL_Event event) {
	std::vector<EventSubscriber> callbacks = subscribers[event.type];
	for (auto& update : callbacks)
		update(event);

}

void pollEvents() {

	SDL_Event event;
	while (SDL_PollEvent(&event)) {
		updateSubscribers(event);
	}
}
