// #define GLFW_INCLUDE_VULKAN
// #include <GLFW/glfw3.h>

// #define GLM_FORCE_RADIANS
// #define GLM_FORCE_DEPTH_ZERO_TO_ONE
// #include "glm/vec4.hpp"
// #include "glm/mat4x4.hpp"

// #include <iostream>

// #include <vulkan/vulkan.h>

#include "VulkanApp.hpp"

int main() {
    VulkanApp app;
    app.Init();
    app.Run();
    return 0;
}