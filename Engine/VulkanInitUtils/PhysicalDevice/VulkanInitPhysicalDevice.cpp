#include "VulkanInitPhysicalDevice.hpp"

bool Rhea::VulkanInitPhysicalDevice::SetupPhysicalDevice(const VkInstance& instance, const VkSurfaceKHR& surface, VkPhysicalDevice& physical_device)
{
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
    if (deviceCount == 0)
    {
        std::cout << "[ERROR]:: GPU not found :|\n";
        return false;
    }
    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());
    bool error = PickBestPhysicalDevice(devices, surface, physical_device);
    return error;
}

bool Rhea::VulkanInitPhysicalDevice::PickBestPhysicalDevice(const std::vector<VkPhysicalDevice> &devices, const VkSurfaceKHR& surface, VkPhysicalDevice& physical_device)
{
    std::multimap<int, VkPhysicalDevice> candidates;

    for (const auto &device : devices)
    {
        int score = RateDevice(device, surface);
        candidates.insert(std::make_pair(score, device));
    }

    if (candidates.rbegin()->first > 0)
    {
        physical_device = candidates.rbegin()->second;
        std::cout << "[SUCCESS]:: Physical device found\n";
        return true;
    }
    else
    {
        std::cout << "[ERROR]:: Your GPU is not suitable for this application\n";
        return false;
    }
}

bool Rhea::VulkanInitPhysicalDevice::CheckDeviceExtensionSupport(const VkPhysicalDevice &device)
{
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

    std::set<std::string> requiredExtensions(device_extensions.begin(), device_extensions.end());

    for (const auto &extension : availableExtensions)
    {
        requiredExtensions.erase(extension.extensionName);
    }

    return requiredExtensions.empty();
}

int Rhea::VulkanInitPhysicalDevice::RateDevice(const VkPhysicalDevice &device, const VkSurfaceKHR& surface)
{
    VkPhysicalDeviceProperties deviceProperties;
    VkPhysicalDeviceFeatures deviceFeatures;
    vkGetPhysicalDeviceProperties(device, &deviceProperties);
    vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

    bool queue_result = VulkanQueueFamilyUtils::GetDeviceQueueFamily(device, surface).SupportedAll();
    bool extensionsSupported = CheckDeviceExtensionSupport(device);
    bool swapChainAdequate = false;
    if (extensionsSupported)
    {
        SwapChainSupportDetails swapChainSupport = SwapChainSupportUtils::QuerySwapChainSupport(device, surface);
        swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.present_modes.empty();
    }
    if (!queue_result || !extensionsSupported || !swapChainAdequate)
        return 0;

    int score = 100;

    if (!deviceFeatures.geometryShader)
        return 0;

    if (deviceProperties.limits.maxImageDimension2D < APP_MIN_IMAGE_2D_DIMENSION_SUPPORT)
        return 0;

    // max possible size of textures
    score += deviceProperties.limits.maxImageDimension2D;

    // discrete GPU
    if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
    {
        score += 1000;
    }

    return score;
}
