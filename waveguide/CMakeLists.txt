cmake_minimum_required(VERSION 3.0)

project(WAVEGUIDE VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} " -std=c++14 ")
if(CMAKE_BUILD_TYPE MATCHES DEBUG)
    set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} " -Wall -Werror ")
endif()

file(GLOB_RECURSE sources "src/*.cpp")
file(GLOB_RECURSE public_headers "include/*.h")
file(GLOB_RECURSE private_headers "src/*.h")

add_library(waveguide STATIC ${sources} ${public_headers} ${private_headers})

add_dependencies(waveguide glm_external)

target_include_directories(waveguide
    PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
    PRIVATE
    ${WAYVERB_BINARY_DIR}/include
)

target_link_libraries(waveguide wayverb-data common samplerate)

add_subdirectory(compressed)
add_subdirectory(tests)
