#include "VulkanApp.hpp"

void VulkanApp::Init()
{
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    GlobalState::mainWindow = glfwCreateWindow(init_window_width, init_window_height, "Rhea", nullptr, nullptr);

    InitVulkan();
}

void VulkanApp::Run()
{
}

void VulkanApp::InitVulkan()
{
    CreateVkAppInstance();
    CreateVkAppSurfaceKHR();
    SetVkAppPhysicalDevice();
    SetVkAppLogicalDevice();
}

void VulkanApp::RunLoop()
{
    while (!glfwWindowShouldClose(GlobalState::mainWindow))
    {
        glfwPollEvents();
    }
    vkDeviceWaitIdle(vkapp_device);
}

void VulkanApp::CleanUp()
{
    vkDestroyDevice(vkapp_device, nullptr);
    vkDestroySurfaceKHR(vkapp_instance, vkapp_surface, nullptr);
    vkDestroyInstance(vkapp_instance, nullptr);
    glfwDestroyWindow(GlobalState::mainWindow);
    GlobalState::mainWindow = nullptr;
    glfwTerminate();
}

void VulkanApp::CreateVkAppInstance()
{
    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Rhea";
    appInfo.applicationVersion = VK_MAKE_API_VERSION(0, 0, 1, 0);
    appInfo.pEngineName = "Rhea";
    appInfo.engineVersion = VK_MAKE_API_VERSION(0, 0, 1, 0);
    appInfo.apiVersion = VK_MAKE_API_VERSION(0, 0, 1, 0);

    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

    uint32_t glfwExtensionCount = 0;
    const char **glfwExtensions;

    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

#ifdef __APPLE__
    std::vector<const char *> requiredExtensions;

    for (uint32_t i = 0; i < glfwExtensionCount; i++)
    {
        requiredExtensions.emplace_back(glfwExtensions[i]);
    }

    requiredExtensions.emplace_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);

    createInfo.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;

    createInfo.enabledExtensionCount = (uint32_t)requiredExtensions.size();
    createInfo.ppEnabledExtensionNames = requiredExtensions.data();
#else
    createInfo.enabledExtensionCount = glfwExtensionCount;
    createInfo.ppEnabledExtensionNames = glfwExtensions;
#endif

    createInfo.enabledLayerCount = 0;

    if (vkCreateInstance(&createInfo, nullptr, &vkapp_instance) != VK_SUCCESS)
        std::cout << "[ERROR]:: Failed to create VulkanInstance\n";
    else
        std::cout << "[SUCCESS]:: Created VulkanInstance\n";
}

void VulkanApp::SetVkAppPhysicalDevice()
{
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(vkapp_instance, &deviceCount, nullptr);
    if (deviceCount == 0)
    {
        std::cout << "[ERROR]:: GPU not found :|\n";
        return;
    }
    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(vkapp_instance, &deviceCount, devices.data());

    PickBestDevice(devices);
}

void VulkanApp::SetVkAppLogicalDevice()
{
    QueueFamilyIndices indices = GetDeviceQueueFamilies(vkapp_physical_device);

    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    std::set<uint32_t> uniqueQueueFamilies = {indices.g_family.value(), indices.p_family.value()};

    for (uint32_t queueFamily : uniqueQueueFamilies)
    {
        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueFamily;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &vkapp_device_queue_priority;
        queueCreateInfos.push_back(queueCreateInfo);
    }

    VkPhysicalDeviceFeatures deviceFeatures{};

    VkDeviceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    createInfo.pQueueCreateInfos = queueCreateInfos.data();
    createInfo.pEnabledFeatures = &deviceFeatures;
    createInfo.enabledLayerCount = 0;
    createInfo.enabledExtensionCount = static_cast<uint32_t>(vkapp_device_extensions.size());
    createInfo.ppEnabledExtensionNames = vkapp_device_extensions.data();

    if (vkCreateDevice(vkapp_physical_device, &createInfo, nullptr, &vkapp_device) != VK_SUCCESS)
    {
        std::cout << "[ERROR]:: Failed to create logical device\n";
    }

    vkGetDeviceQueue(vkapp_device, indices.g_family.value(), 0, &vkapp_graphics_queue);
    vkGetDeviceQueue(vkapp_device, indices.p_family.value(), 0, &vkapp_present_queue);

    if (vkapp_graphics_queue != nullptr && vkapp_present_queue != nullptr)
        std::cout << "[SUCCESS]:: Graphics and present queue has been created\n";
    else
        std::cout << "[ERROR]:: Failed to create queue\n";
}

void VulkanApp::CreateVkAppSurfaceKHR()
{
    if (glfwCreateWindowSurface(vkapp_instance, GlobalState::mainWindow, nullptr, &vkapp_surface) != VK_SUCCESS)
    {
        std::cout << "[ERROR]:: Failed to create surface\n";
    }
    else
    {
        std::cout << "[SUCCESS]:: Surface setup\n";
    }
}

bool VulkanApp::PhysicalDeviceCheck(VkPhysicalDevice device)
{
    VkPhysicalDeviceProperties deviceProperties;
    VkPhysicalDeviceFeatures deviceFeatures;
    vkGetPhysicalDeviceProperties(device, &deviceProperties);
    vkGetPhysicalDeviceFeatures(device, &deviceFeatures);
    bool queue_result = GetDeviceQueueFamilies(device).isComplete();
    bool extensionsSupported = CheckDeviceExtensionSupport(device);

#ifdef APP_DEVICE_MUST_SUPPORT_GEOMETRY_SHADER
    return deviceProperties.limits.maxImageDimension2D >= APP_MIN_IMAGE_2D_DIMENSION_SUPPORT && deviceFeatures.geometryShader && queue_result && extensionsSupported;
#else
    return deviceProperties.limits.maxImageDimension2D >= APP_MIN_IMAGE_2D_DIMENSION_SUPPORT && queue_result && extensionsSupported;
#endif // !APP_DEVICE_MUST_SUPPORT_GEOMETRY_SHADER
}

int VulkanApp::RatePhysicalDevice(VkPhysicalDevice device)
{
    VkPhysicalDeviceProperties deviceProperties;
    VkPhysicalDeviceFeatures deviceFeatures;
    vkGetPhysicalDeviceProperties(device, &deviceProperties);
    vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

    bool queue_result = GetDeviceQueueFamilies(device).isComplete();
    bool extensionsSupported = CheckDeviceExtensionSupport(device);
    if (!queue_result || !extensionsSupported)
        return 0;

    int score = 0;

#ifdef APP_DEVICE_MUST_SUPPORT_GEOMETRY_SHADER
    if (!deviceFeatures.geometryShader)
        return 0;
#endif // APP_DEVICE_MUST_SUPPORT_GEOMETRY_SHADER

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

QueueFamilyIndices VulkanApp::GetDeviceQueueFamilies(VkPhysicalDevice device)
{
    // using optional because all values are suitable for queue families
    QueueFamilyIndices indices;

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

    int i = 0;
    for (const auto &queueFamily : queueFamilies)
    {
        // TODO add compute operations support
        if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            indices.g_family = i;
        }

        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, vkapp_surface, &presentSupport);

        if (presentSupport)
            indices.p_family = i;

        if (indices.isComplete())
            break;

        i++;
    }

    return indices;
}

bool VulkanApp::CheckDeviceExtensionSupport(VkPhysicalDevice device)
{
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

    std::set<std::string> requiredExtensions(vkapp_device_extensions.begin(), vkapp_device_extensions.end());

    for (const auto &extension : availableExtensions)
    {
        requiredExtensions.erase(extension.extensionName);
    }

    return requiredExtensions.empty();
}

SwapChainSupportDetails VulkanApp::QuerySwapChainSupport(VkPhysicalDevice device)
{
    SwapChainSupportDetails details;

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, vkapp_surface, &details.capabilities);

    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, vkapp_surface, &formatCount, nullptr);

    if (formatCount != 0)
    {
        details.formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, vkapp_surface, &formatCount, details.formats.data());
    }

    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, vkapp_surface, &presentModeCount, nullptr);

    if (presentModeCount != 0)
    {
        details.present_modes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, vkapp_surface, &presentModeCount, details.present_modes.data());
    }

    return details;
}

void VulkanApp::PickBestDevice(const std::vector<VkPhysicalDevice> &devices)
{
#ifdef APP_ENABLE_BEST_DEVICE_CHECK
    std::multimap<int, VkPhysicalDevice> candidates;

    for (const auto &device : devices)
    {
        int score = RatePhysicalDevice(device);
        candidates.insert(std::make_pair(score, device));
    }

    if (candidates.rbegin()->first > 0)
    {
        vkapp_physical_device = candidates.rbegin()->second;
        std::cout << "[SUCCESS]:: Physical device found\n";
    }
    else
        std::cout << "[ERROR]:: Your GPU is not suitable for this application\n";
#else
    for (const auto &device : devices)
    {
        if (PhysicalDeviceCheck(device))
        {
            vkapp_physical_device = device;
            std::cout << "[SUCCESS]:: Physical device found\n";
            break;
        }
    }
    if (vkapp_physical_device == nullptr)
        std::cout << "[ERROR]:: Your GPU does not suitable for this application\n";
#endif
}
