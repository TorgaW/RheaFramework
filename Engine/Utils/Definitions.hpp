#ifndef EC64783D_2C30_405A_B5F3_8FD84A9243CA
#define EC64783D_2C30_405A_B5F3_8FD84A9243CA

#include <vector>

#include "CrossplatformVk.hpp"

namespace Rhea
{
    // compact all necessary varibales for vulkan in one struct
    struct VulkanAppVariables
    {
        VkInstance instance {nullptr};
        VkPhysicalDevice physical_device {nullptr};
        VkDevice device {nullptr};
        VkQueue graphics_queue {nullptr};
        VkQueue present_queue {nullptr};
        VkSurfaceKHR surface {nullptr};
        VkSwapchainKHR swap_chain {nullptr};
        std::vector<VkImage> swap_chain_images;
        VkFormat swap_chain_image_format;
        VkExtent2D swap_chain_extent;
        std::vector<VkImageView> swap_chain_image_views;
        VkRenderPass render_pass {nullptr};
        VkPipelineLayout pipeline_layout {nullptr};
        VkPipeline graphics_pipeline {nullptr};
        std::vector<VkFramebuffer> swap_chain_framebuffers;
        VkCommandPool command_pool {nullptr};
        VkCommandBuffer command_buffer {nullptr};
        VkSemaphore image_available_semaphore {nullptr};
        VkSemaphore render_finished_semaphore {nullptr};
        VkFence in_flight_fence {nullptr};
        float device_queue_priority {1.0f};
    };

} // namespace Rhea


#endif /* EC64783D_2C30_405A_B5F3_8FD84A9243CA */
