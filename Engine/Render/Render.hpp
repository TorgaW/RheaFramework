#ifndef FE300185_DB9F_4572_BE56_42C8EAB747EB
#define FE300185_DB9F_4572_BE56_42C8EAB747EB

#include "../Utils/Definitions.hpp"
#include "../State/State.hpp"
#include "../VulkanInitUtils/VulkanInits.hpp"

namespace Rhea
{

    class Render
    {
    public:
        int default_window_width {800};
        int default_window_height {600};
    
    public:
        /**
         * Init Render object
        */
        bool Init();
        /**
         * Run Render object. 
         * **Returns error code or 0 (not yet implemeted)**.
        */
        int Run();


    private:
        VulkanAppVariables vk_variables;
    };

} // namespace Rhea


#endif /* FE300185_DB9F_4572_BE56_42C8EAB747EB */
