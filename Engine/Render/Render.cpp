#include "Render.hpp"

bool Rhea::Render::Init()
{
    auto window = WindowInit::CreateGLFWWindow(default_window_width, default_window_height, "Rhea", false);
    if(!window) return false;

    State::main_window = window;

    bool error = VulkanInitInstance::CreateVulkanInstance(&vk_variables.instance);
    if(error) return false;

    error = VulkanInitSurface::CreateVkSurfaceKHR(vk_variables.instance, window, &vk_variables.surface);
    if(error) return false;

    
    return true;
}

int Rhea::Render::Run()
{

    return 0;
}
