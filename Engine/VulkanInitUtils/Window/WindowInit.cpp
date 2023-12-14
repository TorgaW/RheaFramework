#include "WindowInit.hpp"

GLFWwindow *Rhea::WindowInit::CreateGLFWWindow(int width, int height, const std::string& title, bool resizable)
{
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, (int)resizable);

    return glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
}