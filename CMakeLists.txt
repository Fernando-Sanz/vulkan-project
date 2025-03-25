
cmake_minimum_required(VERSION 3.10)

set(PROJECT_NAME "VulkanProject")
project(${PROJECT_NAME})

set(CMAKE_CXX_STANDARD 17)


#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
# PROJECT FILES

include_directories(${PROJECT_NAME}/src)

set(SOURCES
    tests/main.cpp
    ${PROJECT_NAME}/src/CommandManager.cpp
    ${PROJECT_NAME}/src/Device.cpp
    ${PROJECT_NAME}/src/GraphicsPipeline.cpp
    ${PROJECT_NAME}/src/imageUtils.cpp
    ${PROJECT_NAME}/src/Model.cpp
    ${PROJECT_NAME}/src/SwapChain.cpp
    ${PROJECT_NAME}/src/Texture.cpp
    ${PROJECT_NAME}/src/UniformManager.cpp
    ${PROJECT_NAME}/src/utils.cpp
    ${PROJECT_NAME}/src/Window.cpp
)

add_executable(${PROJECT_NAME} ${SOURCES})

#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
# EXTERNAL LIBRARIES

find_package(Vulkan REQUIRED)
target_include_directories(${PROJECT_NAME} PRIVATE
    C:/VulkanSDK/1.3.290.0/Libraries/glfw-3.4.bin.WIN64/include
    C:/VulkanSDK/1.3.290.0/Include/glm
    C:/VulkanSDK/1.3.290.0/Libraries/stb-master
    C:/development/libraries/tiny_obj_loader/include
)

target_link_libraries(${PROJECT_NAME}
    C:/VulkanSDK/1.3.290.0/Libraries/glfw-3.4.bin.WIN64/lib-vc2022/glfw3.lib
    Vulkan::Vulkan)
