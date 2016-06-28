cmake_minimum_required(VERSION 3.0)

project(OTHER_COMPONENTS VERSION 0.1 LANGUAGES CXX)

set(JUCE_MODULES "" CACHE PATH "")

set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} " -Wall -Werror -std=c++14 ")
set(CMAKE_CXX_FLAGS_DEBUG ${CMAKE_CXX_FLAGS_DEBUG} " -DDEBUG ")
set(CMAKE_CXX_FLAGS_RELEASE ${CMAKE_CXX_FLAGS_RELEASE} " -DNDEBUG ")

add_definitions(-DJUCE_GLOBAL_MODULE_SETTINGS_INCLUDED)

file(GLOB_RECURSE sources "src/*.cpp")
file(GLOB_RECURSE headers "include/*.hpp")

set(name other-components)

add_library(${name} STATIC ${sources} ${headers})

target_include_directories(${name}
    PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
    PRIVATE
    ${JUCE_MODULES}
)

target_link_libraries(${name} glm modern_gl_utils)
