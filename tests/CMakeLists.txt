cmake_minimum_required(VERSION 3.0)

set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} " -Wall -Werror -std=c++14 ")

project(tests)

include_directories(
    ${CMAKE_SOURCE_DIR}/combined
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/submodules/cereal/include
    "/usr/local/include/"
)
link_directories(/usr/local/lib)

set(name tests)
file(GLOB sources "*.cpp")

add_definitions(-DSCRATCH_PATH="${CMAKE_BINARY_DIR}")

add_definitions(${test_file_flag})
add_definitions(-DOBJ_PATH="${CMAKE_SOURCE_DIR}/demo/assets/test_models/vault.obj")
add_definitions(-DMAT_PATH="${CMAKE_SOURCE_DIR}/demo/assets/materials/vault.json")
add_definitions(-DOBJ_PATH_TUNNEL="${CMAKE_SOURCE_DIR}/demo/assets/test_models/echo_tunnel.obj")
add_definitions(-DMAT_PATH_TUNNEL="${CMAKE_SOURCE_DIR}/demo/assets/materials/mat.json")
add_definitions(-DOBJ_PATH_BEDROOM="${CMAKE_SOURCE_DIR}/demo/assets/test_models/bedroom.obj")
add_definitions(-DMAT_PATH_BEDROOM="${CMAKE_SOURCE_DIR}/demo/assets/materials/mat.json")
add_definitions(-DOBJ_PATH_BAD_BOX="${CMAKE_SOURCE_DIR}/demo/assets/test_models/small_square.obj")
add_definitions(-DMAT_PATH_BAD_BOX="${CMAKE_SOURCE_DIR}/demo/assets/materials/damped.json")

add_executable(${name} ${sources})

find_library(sndfile_lib sndfile)
find_library(flac_lib flac)
find_library(ogg_lib ogg)
find_library(vorbis_lib vorbis)
find_library(vorbisenc_lib vorbisenc)

target_link_libraries(${name} raytracer waveguide common ${sndfile_lib} ${flac_lib} ${ogg_lib} ${vorbis_lib} ${vorbisenc_lib} glog gtest)

add_test(NAME tests COMMAND ${name})

add_subdirectory(mic_test)
add_subdirectory(boundary_test)
add_subdirectory(hybrid_test)
