#pragma once
#include <vulkan/vulkan.h>
#include <optional>
#include <vector>


struct QueueFamilyIndices {
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;
    //std::optional<uint32_t> transferFamily; Implicitly included with graphics

    bool isComplete() {
        return graphicsFamily.has_value() && presentFamily.has_value();
    }
};

struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

const std::vector<const char*> deviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
};


class PhysicalDevice {
public:

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // CONSTRUCTOR

    PhysicalDevice() {
    }


    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // GETTERS AND SETTERS

    VkPhysicalDevice getDevice() { return device; }
    VkSampleCountFlagBits getMsaaSamples() { return msaaSamples; }


    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // METHODS

    void pickDevice();

    QueueFamilyIndices findQueueFamilies() { return findQueueFamilies(device); }
    SwapChainSupportDetails querySwapChainSupport() { return querySwapChainSupport(device); }
    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
    VkPhysicalDeviceProperties getPhysicalDeviceProperties(); // TODO: [REFACTOR] make this class member?
    VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);


private:

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // CLASS MEMBERS

    VkPhysicalDevice device = VK_NULL_HANDLE;	// destroyed with instance
    VkSampleCountFlagBits msaaSamples = VK_SAMPLE_COUNT_1_BIT; // multisampling


    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // METHODS

    bool isDeviceSuitable(VkPhysicalDevice device);

    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);

    bool checkDeviceExtensionSupport(VkPhysicalDevice device);

    SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);


    VkSampleCountFlagBits getMaxUsableSampleCount();

};
