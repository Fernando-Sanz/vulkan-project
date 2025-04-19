#include "scene/Light.hpp"

#include "time/AppTime.hpp"


Light::Light() {
	transform.position = glm::vec3(2.0);
	transform.lookAt = glm::vec3(0.0f) - transform.position;
	transform.up = glm::vec3(0.0f); // Not orthogonal space
	transform.right = glm::cross(transform.position, transform.up);
	color = glm::vec3(0.5f, 0.437f, 0.365f);
}

void Light::update() {
	// SIMPLE MOVEMENT FOR TESTING:
	static float angle = 90.0f;
	float speed = 5.0f;
	angle += AppTime::deltaTime() * speed;
	transform.position.x = glm::sin(angle);
	transform.position.y = glm::cos(angle);
}

void Light::keyboardReaction(SDL_Event event) {

}
