#ifndef BE973C21_2839_4A74_A1CB_BDC0AD109733
#define BE973C21_2839_4A74_A1CB_BDC0AD109733

// #define VK_USE_PLATFORM_X11_KHR
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#ifdef __APPLE__
#define GLFW_EXPOSE_NATIVE_COCOA
#elif __linux__
#define GLFW_EXPOSE_NATIVE_X11
#endif
#include <GLFW/glfw3native.h>

#include <vulkan/vulkan.h>

#include <iostream>
#include <vector>
#include <map>
#include <optional>
#include <set>

#include "GlobalState.hpp"

#define APP_ENABLE_BEST_DEVICE_CHECK
#define APP_DEVICE_MUST_SUPPORT_GEOMETRY_SHADER
#define APP_MIN_IMAGE_2D_DIMENSION_SUPPORT 2048

// currently not available
#define APP_DEVICE_MUST_SUPPORT_COMPUTE_QUEUE

struct QueueFamilyIndices {
    std::optional<uint32_t> g_family;
    std::optional<uint32_t> p_family;

    inline bool isComplete() {
        return g_family.has_value() && p_family.has_value();
    }
};

struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> present_modes;
};

class VulkanApp
{
public:
    int init_window_width {800};
    int init_window_height {600};
public:
    //* api *//
    void Init();
    void Run();
private:
    //* main functions *//
    void InitVulkan();
    void RunLoop();
    void CleanUp();

    //* vk functions *//
    void CreateVkAppInstance();
    void SetVkAppPhysicalDevice();
    void SetVkAppLogicalDevice();
    void CreateVkAppSurfaceKHR();

    // simple device check (geom shader + discrete)
    bool PhysicalDeviceCheck(VkPhysicalDevice device);

    // extended device check
    int RatePhysicalDevice(VkPhysicalDevice device);
    QueueFamilyIndices GetDeviceQueueFamilies(VkPhysicalDevice device);
    bool CheckDeviceExtensionSupport(VkPhysicalDevice device);
    SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device);

    void PickBestDevice(const std::vector<VkPhysicalDevice>& devices);
private:
    VkInstance vkapp_instance {nullptr};
    VkPhysicalDevice vkapp_physical_device {nullptr};
    VkDevice vkapp_device {nullptr};
    VkQueue vkapp_graphics_queue {nullptr};
    VkQueue vkapp_present_queue {nullptr};
    VkSurfaceKHR vkapp_surface {nullptr};
    float vkapp_device_queue_priority {1.0f};
    const std::vector<const char*> vkapp_device_extensions { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
};


#endif /* BE973C21_2839_4A74_A1CB_BDC0AD109733 */