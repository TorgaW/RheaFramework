cmake_minimum_required(VERSION 3.21)                              
set(CMAKE_CXX_STANDARD 20) # vulkan supports c++17 and newer           
                                                                                                                                                                                  
project(RheaRender)                                                                                                                                                            
                                                                                                                                                                                  
# find_package(OpenGL REQUIRED)
find_package(Vulkan REQUIRED)                                                                                                                                               
find_package(glfw3 3.3 REQUIRED)       

set(RENDER Engine/Render/Render.cpp)

set(VULKAN_UTILS
Engine/Utils/Vulkan/QueueFamilyUtils.cpp
Engine/Utils/Vulkan/SwapChainSupportUtils.cpp
)

set(VULKAN_INITS
Engine/VulkanInitUtils/Window/WindowInit.cpp
Engine/VulkanInitUtils/Instance/VulkanInitInstance.cpp
Engine/VulkanInitUtils/Surface/VulkanInitSurface.cpp
Engine/VulkanInitUtils/PhysicalDevice/VulkanInitPhysicalDevice.cpp
)

set(ENGINE ${RENDER} ${VULKAN_INITS} ${VULKAN_UTILS})

add_executable(RheaRender main.cpp VulkanApp.cpp Utils.cpp ${ENGINE})
target_link_libraries(RheaRender glfw vulkan)

# command to update resources every build
add_custom_target(copy_resources ALL                                                                                                                                              
        COMMAND ${CMAKE_COMMAND} -E copy_directory                                                                                                                                
        ${PROJECT_SOURCE_DIR}/Resources                                                                                                                                           
        ${PROJECT_BINARY_DIR}/Resources                                                                                                                                           
        COMMENT "Copying resources into binary directory")