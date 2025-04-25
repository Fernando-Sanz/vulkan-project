#include "scene/Light.hpp"

#include "time/AppTime.hpp"


Light::Light() {
	color = glm::vec3(0.5f, 0.437f, 0.365f);
}

// TODO: remove this to a Behaviour type class
void Light::update() {
	// SIMPLE MOVEMENT FOR TESTING:
	static float angle = 90.0f;
	float speed = 5.0f;
	angle += AppTime::deltaTime() * speed;
	transform->position.x = glm::sin(angle);
	transform->position.y = glm::cos(angle);
}

void Light::keyboardReaction(SDL_Event event) {

}
