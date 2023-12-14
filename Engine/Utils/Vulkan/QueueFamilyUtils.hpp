#ifndef DE950800_DF37_4DDB_A2CC_DBEF74412948
#define DE950800_DF37_4DDB_A2CC_DBEF74412948

#include <optional>
#include <cstdint>
#include <vector>

#include "../CrossplatformVk.hpp"

namespace Rhea
{
    struct QueueFamilyIndices {
        std::optional<uint32_t> g_family;
        std::optional<uint32_t> p_family;

        inline bool SupportedAll() {
            return g_family.has_value() && p_family.has_value();
        }
    };

    class VulkanQueueFamilyUtils
    {
    public:
        static QueueFamilyIndices GetDeviceQueueFamily(const VkPhysicalDevice& device, const VkSurfaceKHR& surface);
    };
    
} // namespace Rhea


#endif /* DE950800_DF37_4DDB_A2CC_DBEF74412948 */
