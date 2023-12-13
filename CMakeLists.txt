cmake_minimum_required(VERSION 3.21)                              
set(CMAKE_CXX_STANDARD 20) # vulkan supports c++17 and newer           
                                                                                                                                                                                  
project(RheaRender)                                                                                                                                                            
                                                                                                                                                                                  
# find_package(OpenGL REQUIRED)
find_package(Vulkan REQUIRED)                                                                                                                                               
find_package(glfw3 3.3 REQUIRED)                                                                                                                                     

add_executable(RheaRender main.cpp VulkanApp.cpp)
target_link_libraries(RheaRender glfw vulkan)

# command to update resources every build
add_custom_target(copy_resources ALL                                                                                                                                              
        COMMAND ${CMAKE_COMMAND} -E copy_directory                                                                                                                                
        ${PROJECT_SOURCE_DIR}/Resources                                                                                                                                           
        ${PROJECT_BINARY_DIR}/Resources                                                                                                                                           
        COMMENT "Copying resources into binary directory")