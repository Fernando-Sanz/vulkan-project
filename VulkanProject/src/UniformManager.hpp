#pragma once

#include <glm/glm.hpp>
#include <vector>

#include "Device.hpp"
#include "Camera.hpp"
#include "Light.hpp"


// See alignment requirements in specification
// (https://docs.vulkan.org/spec/latest/chapters/interfaces.html#interfaces-resources-layout)
struct UniformBufferObject {
    alignas(16) glm::mat4 modelView;
    alignas(16) glm::mat4 invTrans_modelView;
    alignas(16) glm::mat4 proj;
};

struct LightUBO {
    alignas(16) glm::vec3 pos;
    alignas(16) glm::vec3 color;
    alignas(16) glm::vec3 direction;
};


class UniformManager {
public:

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // GETTERS AND SETTERS

    VkBuffer getBuffer(size_t index) { return buffers[index]; }
    VkBuffer getLightBuffer() { return lightUBOBuffer; }

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // METHODS

    // Create as many uniform buffers as count
    void createBuffers(Device device, int count);

    // Update uniform values
    void upateBuffer(uint32_t currentImage, glm::mat4 model, Camera camera, Light light);

    // Destroy Vulkan an other objects
    void cleanup();

private:

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // CLASS MEMBERS

    Device device;

    std::vector<VkBuffer> buffers;
    std::vector<VkDeviceMemory> buffersMemory;
    std::vector<void*> buffersMapped;

    VkBuffer lightUBOBuffer;
    VkDeviceMemory lightUBOMemory;
    void* lightUBOMapped;

};
