cmake_minimum_required(VERSION 3.0)

project(WAVEGUIDE VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} " -std=c++14 ")
if(CMAKE_BUILD_TYPE MATCHES DEBUG)
    set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} " -Wall -Werror ")
endif()

add_subdirectory(tests)
add_subdirectory(compensation_signal)

set(outfile ${CMAKE_CURRENT_BINARY_DIR}/mesh_impulse_response.h)
add_custom_command(OUTPUT ${outfile}
    COMMAND write_compensation_signal 512 > ${outfile}
)

file(GLOB_RECURSE sources "src/*.cpp")
file(GLOB_RECURSE public_headers "include/*.h")
file(GLOB_RECURSE private_headers "src/*.h")

add_library(waveguide STATIC ${sources} ${public_headers} ${private_headers} ${outfile})

add_dependencies(waveguide glm_external)

target_include_directories(waveguide
    PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
    PRIVATE
    ${CMAKE_BINARY_DIR}/include
    ${CMAKE_CURRENT_BINARY_DIR}
)

target_link_libraries(waveguide common samplerate)

