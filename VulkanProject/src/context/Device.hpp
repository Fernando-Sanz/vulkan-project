#pragma once
#include <vulkan/vulkan.h>
#include <optional>
#include <vector>


// All the queue families needed by the program
struct QueueFamilyIndices {
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;
    //std::optional<uint32_t> transferFamily; Implicitly included with graphics

    // Check if all the queue family indices have a value
    bool isComplete() {
        return graphicsFamily.has_value() && presentFamily.has_value();
    }
};

// Details for the swap chain supported by the GPU
struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

// Device extensions needed by the program
const std::vector<const char*> deviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
};


class Device {
public:

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // GETTERS AND SETTERS

    VkPhysicalDevice getPhysicalDevice() { return physicalDevice; }
    VkSampleCountFlagBits getMsaaSamples() { return msaaSamples; }

    VkDevice get() { return logicalDevice; }
    VkQueue getGraphicsQueue() { return graphicsQueue; }
    VkQueue getPresentQueue() { return presentQueue; }


    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // PHYSICAL DEVICE METHODS
    
    // Select a device depending on the requirements of 'isDeviceSuitable()' and creates a logical device
    // TODO: Check if it could receive the surface that will be used for rendering
    void pickDevice(VkInstance instance, VkSurfaceKHR surface);

    // This function returns a QueueFamilyIndices struct with the supported queue families
    QueueFamilyIndices getQueueFamilies() const { return queueFamilyIndices; }

    // Check for CAPABILITIES, FORMATS and PRESENT MODES
    SwapChainSupportDetails getSwapChainSupport() { return swapChainSupportDetails; }

    // Search for a memory type that has the properties specified
    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

    // Get GPU properties
    VkPhysicalDeviceProperties getPhysicalDeviceProperties(); // TODO: [REFACTOR] make this class member?

    // Find supported format depending on desired tiling and features
    VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);

    // Get format properties of the device
    void getPhysicalDeviceFormatProperties(VkFormat imageFormat, VkFormatProperties* formatProperties);


    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // LOGICAL DEVICE METHODS
    
    // Create a buffer with the memory type that matches with the specified properties
    void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage,
        VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);

    // Destroy Vulkan and other objects
    void cleanup();


private:

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // CLASS MEMBERS

    // Physical device objects
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;	// destroyed with instance
    VkSampleCountFlagBits msaaSamples = VK_SAMPLE_COUNT_1_BIT; // multisampling
    QueueFamilyIndices queueFamilyIndices; // queue family indices of the physical device
    SwapChainSupportDetails swapChainSupportDetails; // swap chain support details of the physical device

    // Logical device objects
    VkDevice logicalDevice;
    VkQueue graphicsQueue;
    VkQueue presentQueue;



    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // PHYSICAL DEVICE METHODS

    // Get the maximun sample count supported by the GPU depending on color buffer AND depth buffer
    VkSampleCountFlagBits getMaxUsableSampleCount();


    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // LOGICAL DEVICE METHODS

    void createLogicalDevice();
};
