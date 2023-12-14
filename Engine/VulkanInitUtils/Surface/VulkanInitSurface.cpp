#include "VulkanInitSurface.hpp"

bool Rhea::VulkanInitSurface::CreateVkSurfaceKHR(const VkInstance &instance, GLFWwindow *window, VkSurfaceKHR *surface)
{
    if (glfwCreateWindowSurface(instance, window, nullptr, surface) != VK_SUCCESS)
    {
        std::cout << "[ERROR]:: Failed to create surface\n";
        return false;
    }
    else
    {
        std::cout << "[SUCCESS]:: Surface is created\n";
        return true;
    }
}