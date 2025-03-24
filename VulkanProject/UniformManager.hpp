#pragma once

#include <glm/glm.hpp>
#include <vector>

#include "Device.hpp"


// See alignment requirements in specification
// (https://docs.vulkan.org/spec/latest/chapters/interfaces.html#interfaces-resources-layout)
struct UniformBufferObject {
    alignas(16) glm::mat4 model;
    alignas(16) glm::mat4 view;
    alignas(16) glm::mat4 proj;
};


class UniformManager {
public:

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // GETTERS AND SETTERS

    VkBuffer getBuffer(size_t index) { return buffers[index]; }

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // METHODS

    // Create as many uniform buffers as count
    void createBuffers(Device device, int count);

    // Update uniform values
    void upateBuffer(uint32_t currentImage, uint32_t screenWidth, uint32_t screenHeight);

    // Destroy Vulkan an other objects
    void cleanup();

private:

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // CLASS MEMBERS

    Device device;

    std::vector<VkBuffer> buffers;
    std::vector<VkDeviceMemory> buffersMemory;
    std::vector<void*> buffersMapped;
};
