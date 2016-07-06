cmake_minimum_required(VERSION 3.0)

project(WAYVERB_DATA VERSION 0.1 LANGUAGES CXX)

set_source_files_properties(mesh_impulse_response.cpp PROPERTIES GENERATED true)
add_custom_command(OUTPUT mesh_impulse_response.cpp
    COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/gen.sh ${CMAKE_CURRENT_SOURCE_DIR}/mesh_impulse_response.wav ${CMAKE_CURRENT_BINARY_DIR}/mesh_impulse_response.cpp
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
