#pragma once

#include <glm/glm.hpp>
#include <vector>

#include "context/Device.hpp"
#include "scene/Camera.hpp"
#include "scene/Light.hpp"


struct LightUBO {
    alignas(16) glm::vec3 pos;
    alignas(16) glm::vec3 color;
    alignas(16) glm::vec3 direction;
};


// Manage a buffer with all lights for each FRAME_IN_FLIGHT
class LightUboManager {
public:

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // GETTERS AND SETTERS

    size_t getLightCount() const { return lightCount; }

    // Return a buffer that references the memory with all the lights
    VkBuffer getBuffer(size_t index) const { return buffers[index]; }

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // METHODS

    // Create as many uniform buffers as count
    void createBuffers(Device device, size_t count, size_t lightCount);

    // Update uniform values
    void upateBuffer(uint32_t index, std::vector<Light> lights, Camera camera);

    // Destroy Vulkan an other objects
    void cleanup();

private:

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // CLASS MEMBERS

    Device device;

    size_t lightCount = 0;

    // a vector per frame
    std::vector<VkBuffer> buffers;
    std::vector<VkDeviceMemory> buffersMemory;
    std::vector<void*> buffersMapped;
};
