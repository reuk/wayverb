cmake_minimum_required(VERSION 3.0)

project(WAYVERB_DATA VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} " -std=c++14 ")

add_executable(data-convert convert.cpp)

find_library(sndfile_lib sndfile)
find_library(flac_lib flac)
find_library(ogg_lib ogg)
find_library(vorbis_lib vorbis)
find_library(vorbisenc_lib vorbisenc)

target_link_libraries(data-convert
    ${sndfile_lib} ${flac_lib} ${ogg_lib} ${vorbis_lib} ${vorbisenc_lib})

target_include_directories(data-convert
    PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:include>
    PRIVATE
    /usr/local/include
)

set(outfile ${CMAKE_CURRENT_BINARY_DIR}/mesh_impulse_response.cpp)

set_source_files_properties(mesh_impulse_response.cpp PROPERTIES GENERATED true)
add_custom_command(OUTPUT mesh_impulse_response.cpp
    COMMAND echo "static const float data[] = {" > ${outfile}
    COMMAND data-convert ${CMAKE_CURRENT_SOURCE_DIR}/mesh_impulse_response.wav >> ${outfile}
    COMMAND echo "};" >> ${outfile}
    VERBATIM
)

add_library(wayverb-data STATIC mesh_impulse_response.cpp data.cpp)

target_include_directories(wayverb-data
    PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:include>
    PRIVATE
    ${CMAKE_CURRENT_BINARY_DIR}
)
