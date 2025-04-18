#pragma once

#include <glm/glm.hpp>
#include <vector>

#include "Device.hpp"
#include "Camera.hpp"


// See alignment requirements in specification
// (https://docs.vulkan.org/spec/latest/chapters/interfaces.html#interfaces-resources-layout)
struct ModelUBO {
    alignas(16) glm::mat4 modelView;
    alignas(16) glm::mat4 invTrans_modelView;
    alignas(16) glm::mat4 proj;
};


class ModelUboManager {
public:

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // GETTERS AND SETTERS

    bool hasModel() { return !buffers.empty(); }
    VkBuffer getBuffer(size_t index) { return buffers[index]; }

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // METHODS

    // Create as many uniform buffers as count
    void createBuffers(Device device, int count);

    // Update uniform values
    void upateBuffer(uint32_t index, glm::mat4 model, Camera camera);

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
