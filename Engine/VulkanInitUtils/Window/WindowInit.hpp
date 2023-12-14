#ifndef B8255B7E_E041_4750_A997_B3D4C08507BF
#define B8255B7E_E041_4750_A997_B3D4C08507BF

#include <string> 
#include "../../Utils/CrossplatformVk.hpp"

namespace Rhea
{
    class WindowInit
    {
    public:
        static GLFWwindow *CreateGLFWWindow(int width, int height, const std::string& title, bool resizable);
    };
    
} // namespace Rhea


#endif /* B8255B7E_E041_4750_A997_B3D4C08507BF */
