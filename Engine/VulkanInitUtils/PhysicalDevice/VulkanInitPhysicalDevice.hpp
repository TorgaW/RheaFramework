#ifndef AA939664_C7E3_41F7_8DC5_9AA3A9D7B3D8
#define AA939664_C7E3_41F7_8DC5_9AA3A9D7B3D8

#include <cstdint>
#include <vector>
#include <iostream>
#include <set>
#include <map>
#include "../../Utils/CrossplatformVk.hpp"
#include "../../Utils/Vulkan/QueueFamilyUtils.hpp"
#include "../../Utils/Vulkan/SwapChainSupportUtils.hpp"


#define APP_MIN_IMAGE_2D_DIMENSION_SUPPORT 2048

namespace Rhea
{
    class VulkanInitPhysicalDevice
    {
    public:
        static bool SetupPhysicalDevice(const VkInstance& instance, const VkSurfaceKHR& surface, VkPhysicalDevice& physical_device);
    private:
        static bool PickBestPhysicalDevice(const std::vector<VkPhysicalDevice> &devices, const VkSurfaceKHR& surface, VkPhysicalDevice& physical_device);
        static bool CheckDeviceExtensionSupport(const VkPhysicalDevice& device);
        static int RateDevice(const VkPhysicalDevice& device, const VkSurfaceKHR& surface);
    private:
        static inline const std::vector<const char*> device_extensions { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
    };
    
} // namespace Rhea


#endif /* AA939664_C7E3_41F7_8DC5_9AA3A9D7B3D8 */
