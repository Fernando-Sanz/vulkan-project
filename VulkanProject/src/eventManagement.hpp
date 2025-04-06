#pragma once

#include <SDL3/SDL_events.h>

#include <functional>


using EventSubscriber = std::function<void(SDL_Event)>;
using EventType = Uint32;

void addEventSubscriber(EventType type, EventSubscriber subscriber);

void pollEvents();
