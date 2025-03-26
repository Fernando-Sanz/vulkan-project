
cmake_minimum_required(VERSION 3.10)

set(PROJECT_NAME "VulkanProject")
project(${PROJECT_NAME})

set(CMAKE_CXX_STANDARD 17)
set(BUILD_LIBRARY true)

#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
# PROJECT FILES

set(SOURCE_CODE_PATH "${PROJECT_NAME}/src")
include_directories(${SOURCE_CODE_PATH})

if(NOT BUILD_LIBRARY)
    set(TEST_FILES "tests/main.cpp")
endif()

set(SOURCES
    ${TEST_FILES}
    ${SOURCE_CODE_PATH}/CommandManager.cpp
    ${SOURCE_CODE_PATH}/Device.cpp
    ${SOURCE_CODE_PATH}/GraphicsPipeline.cpp
    ${SOURCE_CODE_PATH}/imageUtils.cpp
    ${SOURCE_CODE_PATH}/Model.cpp
    ${SOURCE_CODE_PATH}/SwapChain.cpp
    ${SOURCE_CODE_PATH}/Texture.cpp
    ${SOURCE_CODE_PATH}/UniformManager.cpp
    ${SOURCE_CODE_PATH}/utils.cpp
    ${SOURCE_CODE_PATH}/Window.cpp
)

set(THIRD_PARTY_LIB_PATH "C:/development/cpp")

#------------------------------
# BUILD THE LIBRARY
if(BUILD_LIBRARY)
    add_library(${PROJECT_NAME} STATIC ${SOURCES})

    # Build library outside the project
    set(LIBRARY_OUTPUT_DIRECTORY "${THIRD_PARTY_LIB_PATH}/${PROJECT_NAME}")
    set_target_properties(${PROJECT_NAME} PROPERTIES
        ARCHIVE_OUTPUT_DIRECTORY "${LIBRARY_OUTPUT_DIRECTORY}"
    )
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${LIBRARY_OUTPUT_DIRECTORY}")

    # Copy header files outside the project
    file(GLOB HEADER_FILES "${SOURCE_CODE_PATH}/*.hpp")
    file(COPY ${HEADER_FILES} DESTINATION ${LIBRARY_OUTPUT_DIRECTORY}/include)

#------------------------------
# BUILD THE EXECUTABLE
else()
    add_executable(${PROJECT_NAME} ${SOURCES})
endif()

#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
# EXTERNAL LIBRARIES

find_package(Vulkan REQUIRED)
target_include_directories(${PROJECT_NAME} PRIVATE
    ${THIRD_PARTY_LIB_PATH}/glfw-3.4.bin.WIN64/include
    ${THIRD_PARTY_LIB_PATH}/glm
    ${THIRD_PARTY_LIB_PATH}/stb-master
    ${THIRD_PARTY_LIB_PATH}/tiny_obj_loader/include
)

target_link_libraries(${PROJECT_NAME}
    ${THIRD_PARTY_LIB_PATH}/glfw-3.4.bin.WIN64/lib-vc2022/glfw3.lib
    Vulkan::Vulkan
)
