#ifndef B79FF893_24F4_4DA0_814F_C2EAD638B6DC
#define B79FF893_24F4_4DA0_814F_C2EAD638B6DC

#include <vector>
#include "../CrossplatformVk.hpp"

namespace Rhea
{
    struct SwapChainSupportDetails {
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> present_modes;
    };

    class SwapChainSupportUtils
    {
    public:
        static SwapChainSupportDetails QuerySwapChainSupport(const VkPhysicalDevice& device, const VkSurfaceKHR& surface);
    };
    
} // namespace Rhea


#endif /* B79FF893_24F4_4DA0_814F_C2EAD638B6DC */
