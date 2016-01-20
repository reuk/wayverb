cmake_minimum_required(VERSION 3.0)

project(tests)

include_directories(
    ${CMAKE_SOURCE_DIR}/lib
    ${CMAKE_SOURCE_DIR}/common
    ${CMAKE_SOURCE_DIR}/bsp
    ${CMAKE_SOURCE_DIR}/rayverb
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/gtest/include
    "/usr/local/include/"
)
link_directories(/usr/local/lib)

set(name tests)
file(GLOB sources "*.cpp")

add_definitions(${test_file_flag})
add_definitions(-DOBJ_PATH="${CMAKE_SOURCE_DIR}/demo/assets/test_models/vault.obj")
add_definitions(-DMAT_PATH="${CMAKE_SOURCE_DIR}/demo/assets/materials/vault.json")
add_definitions(-DOBJ_PATH_TUNNEL="${CMAKE_SOURCE_DIR}/demo/assets/test_models/echo_tunnel.obj")
add_definitions(-DMAT_PATH_TUNNEL="${CMAKE_SOURCE_DIR}/demo/assets/materials/mat.json")
add_definitions(-DOBJ_PATH_BEDROOM="${CMAKE_SOURCE_DIR}/demo/assets/test_models/bedroom.obj")
add_definitions(-DMAT_PATH_BEDROOM="${CMAKE_SOURCE_DIR}/demo/assets/materials/mat.json")

add_executable(${name} ${sources})

find_library(sndfile_lib sndfile)

target_link_libraries(${name} waveguide rayverb ${sndfile_lib} gtest)

add_test(waveguide ${name})
