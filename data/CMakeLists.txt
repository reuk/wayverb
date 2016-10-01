cmake_minimum_required(VERSION 3.0)

project(WAYVERB_DATA VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} " -std=c++14 ")
if(CMAKE_BUILD_TYPE MATCHES DEBUG)
    set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} " -Wall -Werror ")
endif()

add_executable(data-convert convert.cpp)

target_link_libraries(data-convert sndfile)

target_include_directories(data-convert
    PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:include>
    PRIVATE
    ${CMAKE_BINARY_DIR}/include
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
