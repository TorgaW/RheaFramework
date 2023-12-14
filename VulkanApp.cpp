#include "VulkanApp.hpp"

void VulkanApp::Init()
{
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    GlobalState::mainWindow = glfwCreateWindow(init_window_width, init_window_height, "Rhea", nullptr, nullptr);

    InitVulkan();
}

void VulkanApp::Run()
{
    RunLoop();
}

void VulkanApp::InitVulkan()
{
    // CreateVkAppInstance();
    Rhea::VulkanInitInstance::CreateVulkanInstance(&vkapp_instance);
    CreateVkAppSurfaceKHR();
    SetVkAppPhysicalDevice();
    SetVkAppLogicalDevice();
    CreateSwapChain();
    CreateImageViews();
    CreateRenderPass();
    CreateGraphicsPipeline();
    CreateFramebuffers();
    CreateCommandPool();
    CreateCommandBuffer();
    CreateSyncObjects();
}

void VulkanApp::RunLoop()
{
    double delta;
    while (!glfwWindowShouldClose(GlobalState::mainWindow))
    {
        auto start = glfwGetTime();
        glfwPollEvents();
        Draw();
        auto end = glfwGetTime();
        delta = end - start;
    }
    vkDeviceWaitIdle(vkapp_device);
    std::cout << "\nlast frame time: " << 1.0/delta << "\n";
}

void VulkanApp::CleanUp()
{
    vkDestroySemaphore(vkapp_device, vkapp_image_available_semaphore, nullptr);
    vkDestroySemaphore(vkapp_device, vkapp_render_finished_semaphore, nullptr);
    vkDestroyFence(vkapp_device, vkapp_in_flight_fence, nullptr);
    vkDestroyCommandPool(vkapp_device, vkapp_command_pool, nullptr);
    for (auto framebuffer : vkapp_swap_chain_framebuffers)
    {
        vkDestroyFramebuffer(vkapp_device, framebuffer, nullptr);
    }
    vkDestroyPipelineLayout(vkapp_device, vkapp_pipeline_layout, nullptr);
    vkDestroyRenderPass(vkapp_device, vkapp_render_pass, nullptr);
    for (auto imageView : vkapp_swap_chain_image_views)
    {
        vkDestroyImageView(vkapp_device, imageView, nullptr);
    }
    vkDestroySwapchainKHR(vkapp_device, vkapp_swap_chain, nullptr);
    vkDestroyDevice(vkapp_device, nullptr);
    vkDestroySurfaceKHR(vkapp_instance, vkapp_surface, nullptr);
    vkDestroyInstance(vkapp_instance, nullptr);
    glfwDestroyWindow(GlobalState::mainWindow);
    GlobalState::mainWindow = nullptr;
    glfwTerminate();
}

void VulkanApp::Draw()
{
    vkWaitForFences(vkapp_device, 1, &vkapp_in_flight_fence, VK_TRUE, UINT64_MAX);
    vkResetFences(vkapp_device, 1, &vkapp_in_flight_fence);

    uint32_t imageIndex;
    vkAcquireNextImageKHR(vkapp_device, vkapp_swap_chain, UINT64_MAX, vkapp_image_available_semaphore, VK_NULL_HANDLE, &imageIndex);

    vkResetCommandBuffer(vkapp_command_buffer, 0);
    RecordCommandBuffer(vkapp_command_buffer, imageIndex);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = {vkapp_image_available_semaphore};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores; 
    submitInfo.pWaitDstStageMask = waitStages;

    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &vkapp_command_buffer;

    VkSemaphore signalSemaphores[] = {vkapp_render_finished_semaphore};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    if (vkQueueSubmit(vkapp_graphics_queue, 1, &submitInfo, vkapp_in_flight_fence) != VK_SUCCESS)
    {
        std::cout << "[ERROR]:: Failed to submit draw command\n";
        return;
    }

    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    VkSwapchainKHR swapChains[] = {vkapp_swap_chain};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;

    presentInfo.pImageIndices = &imageIndex;

    vkQueuePresentKHR(vkapp_present_queue, &presentInfo);
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

void VulkanApp::CreateSwapChain()
{
    SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(vkapp_physical_device);

    VkSurfaceFormatKHR surfaceFormat = ChooseSwapSurfaceFormat(swapChainSupport.formats);
    VkPresentModeKHR presentMode = ChooseSwapPresentMode(swapChainSupport.present_modes);
    VkExtent2D extent = ChooseSwapExtent(swapChainSupport.capabilities);

    uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
    if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount)
    {
        imageCount = swapChainSupport.capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = vkapp_surface;

    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    QueueFamilyIndices indices = GetDeviceQueueFamilies(vkapp_physical_device);
    uint32_t queueFamilyIndices[] = {indices.g_family.value(), indices.p_family.value()};

    if (indices.p_family != indices.p_family)
    {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    }
    else
    {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }

    createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;

    createInfo.oldSwapchain = VK_NULL_HANDLE;

    if (vkCreateSwapchainKHR(vkapp_device, &createInfo, nullptr, &vkapp_swap_chain) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create swap chain!");
    }

    vkGetSwapchainImagesKHR(vkapp_device, vkapp_swap_chain, &imageCount, nullptr);
    vkapp_swap_chain_images.resize(imageCount);
    vkGetSwapchainImagesKHR(vkapp_device, vkapp_swap_chain, &imageCount, vkapp_swap_chain_images.data());

    vkapp_swap_chain_image_format = surfaceFormat.format;
    vkapp_swap_chain_extent = extent;

    std::cout << "[SUCCESS]:: Swap is setup\n";
}

void VulkanApp::CreateImageViews()
{
    vkapp_swap_chain_image_views.resize(vkapp_swap_chain_images.size());

    for (size_t i = 0; i < vkapp_swap_chain_images.size(); i++)
    {
        VkImageViewCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.image = vkapp_swap_chain_images[i];
        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.format = vkapp_swap_chain_image_format;
        createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.levelCount = 1;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount = 1;

        if (vkCreateImageView(vkapp_device, &createInfo, nullptr, &vkapp_swap_chain_image_views[i]) != VK_SUCCESS)
        {
            std::cout << "[ERROR]:: Failed to create image views\n";
        }
        else
        {
            std::cout << "[SUCCESS]:: Image views are created\n";
        }
    }
}

void VulkanApp::CreateRenderPass()
{
    VkAttachmentDescription colorAttachment{};
    colorAttachment.format = vkapp_swap_chain_image_format;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;

    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = 1;
    renderPassInfo.pAttachments = &colorAttachment;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;

    if (vkCreateRenderPass(vkapp_device, &renderPassInfo, nullptr, &vkapp_render_pass) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create render pass!");
    }
}

void VulkanApp::CreateGraphicsPipeline()
{
    auto vertShaderCode = Utils::ReadFile("Resources/Shaders/Compiled/vert.spv");
    auto fragShaderCode = Utils::ReadFile("Resources/Shaders/Compiled/frag.spv");

    VkShaderModule vkapp_vert_shader_module = CreateShaderModule(vertShaderCode);
    VkShaderModule vkapp_frag_shader_module = CreateShaderModule(fragShaderCode);

    VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
    vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertShaderStageInfo.module = vkapp_vert_shader_module;
    vertShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
    fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageInfo.module = vkapp_frag_shader_module;
    fragShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = 0;
    vertexInputInfo.vertexAttributeDescriptionCount = 0;

    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    VkPipelineViewportStateCreateInfo viewportState{};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.scissorCount = 1;

    VkPipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;

    VkPipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_FALSE;

    VkPipelineColorBlendStateCreateInfo colorBlending{};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = VK_LOGIC_OP_COPY;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;
    colorBlending.blendConstants[0] = 0.0f;
    colorBlending.blendConstants[1] = 0.0f;
    colorBlending.blendConstants[2] = 0.0f;
    colorBlending.blendConstants[3] = 0.0f;

    std::vector<VkDynamicState> dynamicStates = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR};
    VkPipelineDynamicStateCreateInfo dynamicState{};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
    dynamicState.pDynamicStates = dynamicStates.data();

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 0;
    pipelineLayoutInfo.pushConstantRangeCount = 0;

    if (vkCreatePipelineLayout(vkapp_device, &pipelineLayoutInfo, nullptr, &vkapp_pipeline_layout) != VK_SUCCESS)
    {
        std::cout << "[ERROR]:: Failed to create pipeline layout!\n";
        return;
    }
    else
    {
        std::cout << "[SUCCESS]:: Pipeline layout is created\n";
    }

    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDynamicState = &dynamicState;
    pipelineInfo.layout = vkapp_pipeline_layout;
    pipelineInfo.renderPass = vkapp_render_pass;
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

    if (vkCreateGraphicsPipelines(vkapp_device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &vkapp_graphics_pipeline) != VK_SUCCESS)
    {
        std::cout << "[ERROR]:: Failed to create graphics pipeline\n";
    }
    else
    {
        std::cout << "[SUCCESS]:: Graphics pipeline is created\n";
    }

    vkDestroyShaderModule(vkapp_device, vkapp_frag_shader_module, nullptr);
    vkDestroyShaderModule(vkapp_device, vkapp_vert_shader_module, nullptr);
}

void VulkanApp::CreateFramebuffers()
{
    vkapp_swap_chain_framebuffers.resize(vkapp_swap_chain_image_views.size());

    for (size_t i = 0; i < vkapp_swap_chain_image_views.size(); i++)
    {
        VkImageView attachments[] = {
            vkapp_swap_chain_image_views[i]};

        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = vkapp_render_pass;
        framebufferInfo.attachmentCount = 1;
        framebufferInfo.pAttachments = attachments;
        framebufferInfo.width = vkapp_swap_chain_extent.width;
        framebufferInfo.height = vkapp_swap_chain_extent.height;
        framebufferInfo.layers = 1;

        if (vkCreateFramebuffer(vkapp_device, &framebufferInfo, nullptr, &vkapp_swap_chain_framebuffers[i]) != VK_SUCCESS)
        {
            std::cout << "[ERROR]:: Failed to create framebuffer\n";
            return;
        }
        else
        {
            std::cout << "[SUCCESS]:: Framebuffer is created\n";
        }
    }
}

void VulkanApp::CreateCommandPool()
{
    QueueFamilyIndices queueFamilyIndices = GetDeviceQueueFamilies(vkapp_physical_device);

    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = queueFamilyIndices.g_family.value();

    if (vkCreateCommandPool(vkapp_device, &poolInfo, nullptr, &vkapp_command_pool) != VK_SUCCESS)
    {
        std::cout << "[ERROR]:: Failed to create command pool\n";
        return;
    }
    else
    {
        std::cout << "[SUCCESS]:: Command pool is created\n";
    }
}

void VulkanApp::CreateCommandBuffer()
{
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = vkapp_command_pool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = 1;

    if (vkAllocateCommandBuffers(vkapp_device, &allocInfo, &vkapp_command_buffer) != VK_SUCCESS)
    {
        std::cout << "[ERROR]:: Failed to create command buffer\n";
        return;
    }
    else
    {
        std::cout << "[SUCCESS]:: Command buffer is created\n";
    }
}

void VulkanApp::CreateSyncObjects()
{
    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    if (vkCreateSemaphore(vkapp_device, &semaphoreInfo, nullptr, &vkapp_image_available_semaphore) != VK_SUCCESS ||
        vkCreateSemaphore(vkapp_device, &semaphoreInfo, nullptr, &vkapp_render_finished_semaphore) != VK_SUCCESS ||
        vkCreateFence(vkapp_device, &fenceInfo, nullptr, &vkapp_in_flight_fence) != VK_SUCCESS)
    {
        std::cout << "[ERROR]:: Syncing failed\n";
        return;
    }
    else
    {
        std::cout << "[SUCCESS]:: Syncing...\n";
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
    bool swapChainAdequate = false;
    if (extensionsSupported)
    {
        SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(device);
        swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.present_modes.empty();
    }
    if (!queue_result || !extensionsSupported || !swapChainAdequate)
        return 0;

    int score = 100;

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

VkSurfaceFormatKHR VulkanApp::ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats)
{
    for (const auto &availableFormat : availableFormats)
    {
        if (availableFormat.format == VK_FORMAT_R8G8B8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
        {
            return availableFormat;
        }
    }
    return availableFormats[0];
}

VkPresentModeKHR VulkanApp::ChooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes)
{
    for (const auto &availablePresentMode : availablePresentModes)
    {
        if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
        {
            return availablePresentMode;
        }
    }

    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D VulkanApp::ChooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities)
{
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
    {
        return capabilities.currentExtent;
    }
    else
    {
        int width, height;
        glfwGetFramebufferSize(GlobalState::mainWindow, &width, &height);

        VkExtent2D actualExtent = {
            static_cast<uint32_t>(width),
            static_cast<uint32_t>(height)};

        actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
        actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

        return actualExtent;
    }
}

VkShaderModule VulkanApp::CreateShaderModule(const std::vector<char> &code)
{
    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size();
    createInfo.pCode = reinterpret_cast<const uint32_t *>(code.data());

    VkShaderModule shaderModule;
    if (vkCreateShaderModule(vkapp_device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
    {
        std::cout << "[ERROR]:: Failed to create shader module\n";
    }
    else
    {
        std::cout << "[SUCCESS]:: Shader module is created\n";
    }

    return shaderModule;
}

void VulkanApp::RecordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex)
{
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS)
    {
        std::cout << "[ERROR]:: Failed to start command recording\n";
    }

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = vkapp_render_pass;
    renderPassInfo.framebuffer = vkapp_swap_chain_framebuffers[imageIndex];
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = vkapp_swap_chain_extent;

    VkClearValue clearColor = {{{0.13f, 0.1f, 0.15f, 1.0f}}};
    renderPassInfo.clearValueCount = 1;
    renderPassInfo.pClearValues = &clearColor;

    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vkapp_graphics_pipeline);

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float)vkapp_swap_chain_extent.width;
    viewport.height = (float)vkapp_swap_chain_extent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = vkapp_swap_chain_extent;
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

    vkCmdDraw(commandBuffer, 3, 1, 0, 0);

    vkCmdEndRenderPass(commandBuffer);

    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
    {
        std::cout << "[ERROR]:: Failed to end command recording\n";
    }
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
