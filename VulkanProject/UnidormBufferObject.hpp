#include <glm/glm.hpp>

// See alignment requirements in specification
// (https://docs.vulkan.org/spec/latest/chapters/interfaces.html#interfaces-resources-layout)
struct UniformBufferObject {
    alignas(16) glm::mat4 model;
    alignas(16) glm::mat4 view;
    alignas(16) glm::mat4 proj;
};
