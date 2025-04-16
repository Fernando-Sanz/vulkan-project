#pragma once

#include <glm/glm.hpp>
#include <vector>

#include "Device.hpp"
#include "Camera.hpp"
#include "Light.hpp"


struct LightUBO {
    alignas(16) glm::vec3 pos;
    alignas(16) glm::vec3 color;
    alignas(16) glm::vec3 direction;
};


class LightUboManager {
public:

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // GETTERS AND SETTERS

    bool hasLights() { return !buffers.empty() && !buffers[0].buffers.empty(); }
    std::vector<VkBuffer> getBuffers(size_t index) { return buffers[index].buffers; }

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // METHODS

    // Create as many uniform buffers as count
    void createBuffers(Device device, size_t count, size_t lightCount);

    // Update uniform values
    void upateBuffers(uint32_t currentImage, std::vector<Light> lights, Camera camera);

    // Destroy Vulkan an other objects
    void cleanup();

private:

    struct UBOResources {
        // a vector of resources per light
        std::vector<VkBuffer> buffers;
        std::vector<VkDeviceMemory> memory;
        std::vector<void*> mapped;
    };

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // CLASS MEMBERS

    Device device;

    // A vector uniform buffers per frame
    std::vector<UBOResources> buffers;

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // METHODS

    void updateBuffer(void* mapped, Light light, glm::mat4 view);
};
