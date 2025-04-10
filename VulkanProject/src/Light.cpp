#include "Light.hpp"


Light::Light() {
	transform.position = glm::vec3(2.0);
	transform.lookAt = glm::vec3(0.0f) - transform.position;
	transform.up = glm::vec3(0.0f); // Not orthogonal space
	color = glm::vec3(0.5f, 0.437f, 0.365f);
}

void Light::update() {

}
