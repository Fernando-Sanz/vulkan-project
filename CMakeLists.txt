
cmake_minimum_required(VERSION 3.16)

set(PROJECT_NAME "VulkanProject")
project(${PROJECT_NAME})

set(CMAKE_CXX_STANDARD 17)
set(BUILD_LIBRARY false)

#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
# PROJECT FILES

set(SOURCE_CODE_PATH "${PROJECT_NAME}/src")
include_directories(${SOURCE_CODE_PATH})

if(NOT BUILD_LIBRARY)
    set(TEST_FILES "tests/main.cpp")
endif()

set(SOURCES
    ${TEST_FILES}
    ${SOURCE_CODE_PATH}/AppTime.cpp
    ${SOURCE_CODE_PATH}/CommandManager.cpp
    ${SOURCE_CODE_PATH}/Device.cpp
    ${SOURCE_CODE_PATH}/eventManagement.cpp
    ${SOURCE_CODE_PATH}/FirstPassPipeline.cpp
    ${SOURCE_CODE_PATH}/imageUtils.cpp
    ${SOURCE_CODE_PATH}/Model.cpp
    ${SOURCE_CODE_PATH}/pipelineUtils.cpp
    ${SOURCE_CODE_PATH}/SecondPassPipeline.cpp
    ${SOURCE_CODE_PATH}/StandardPipeline.cpp
    ${SOURCE_CODE_PATH}/SwapChain.cpp
    ${SOURCE_CODE_PATH}/Texture.cpp
    ${SOURCE_CODE_PATH}/UniformManager.cpp
    ${SOURCE_CODE_PATH}/utils.cpp
    ${SOURCE_CODE_PATH}/Window.cpp
)

#------------------------------
# BUILD THE LIBRARY
if(BUILD_LIBRARY)
    add_library(${PROJECT_NAME} STATIC ${SOURCES})

    # Build library outside the project
    set(LIBRARY_OUTPUT_DIRECTORY "C:/development/cpp/${PROJECT_NAME}")
    set_target_properties(${PROJECT_NAME} PROPERTIES
        ARCHIVE_OUTPUT_DIRECTORY "${LIBRARY_OUTPUT_DIRECTORY}"
    )
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${LIBRARY_OUTPUT_DIRECTORY}")

    # Copy header files outside the project
    file(GLOB HEADER_FILES "${SOURCE_CODE_PATH}/*.hpp")
    file(COPY ${HEADER_FILES} DESTINATION ${LIBRARY_OUTPUT_DIRECTORY}/include)

    # Copy resource files outside the project
    file(GLOB RESOURCE_FILES "${PROJECT_NAME}/assets")
    file(COPY ${RESOURCE_FILES} DESTINATION "${LIBRARY_OUTPUT_DIRECTORY}")

#------------------------------
# BUILD THE EXECUTABLE
else()
    # Change the output to have the SDL .dll automatically inside the .exe directory
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")
    add_executable(${PROJECT_NAME} ${SOURCES})
endif()

#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
# EXTERNAL LIBRARIES

set(THIRD_PARTY_LIB_PATH "${CMAKE_SOURCE_DIR}/third-party")

add_subdirectory(third-party/SDL EXCLUDE_FROM_ALL)

find_package(Vulkan REQUIRED)
target_include_directories(${PROJECT_NAME} PRIVATE
    ${THIRD_PARTY_LIB_PATH}/glm
    ${THIRD_PARTY_LIB_PATH}/stb
    ${THIRD_PARTY_LIB_PATH}/tiny_obj_loader/include
)

target_link_libraries(${PROJECT_NAME}
    Vulkan::Vulkan
    SDL3::SDL3
)
