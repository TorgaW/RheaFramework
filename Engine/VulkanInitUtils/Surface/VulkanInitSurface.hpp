#ifndef CDDC97B9_9410_4F2D_9A9F_2898C882972D
#define CDDC97B9_9410_4F2D_9A9F_2898C882972D

#include <iostream>
#include "../../Utils/CrossplatformVk.hpp"

namespace Rhea
{
    class VulkanInitSurface
    {
    public:
        static bool CreateVkSurfaceKHR(const VkInstance& instance, GLFWwindow *window, VkSurfaceKHR *surface); 
    };
    
} // namespace Rhea


#endif /* CDDC97B9_9410_4F2D_9A9F_2898C882972D */
