#ifndef B76176A1_AC13_461B_9298_99966606FAE4
#define B76176A1_AC13_461B_9298_99966606FAE4

#include <iostream>

#include "../../Utils/CrossplatformVk.hpp"

namespace Rhea
{
    class VulkanInitInstance
    {
    public:
        static bool CreateVulkanInstance(VkInstance *vk_instance);
    };
    
} // namespace Rhea


#endif /* B76176A1_AC13_461B_9298_99966606FAE4 */
