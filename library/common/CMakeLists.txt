cmake_minimum_required(VERSION 3.0)

project(COMMON VERSION 0.1 LANGUAGES CXX)

set(outfile ${CMAKE_CURRENT_BINARY_DIR}/hrtf_entries.h)
add_custom_command(OUTPUT ${outfile}
    COMMAND write_hrtf ${CMAKE_CURRENT_SOURCE_DIR}/data/IRC_1002_C > ${outfile}
)

file(GLOB_RECURSE sources "src/*.cpp")
file(GLOB_RECURSE public_headers "include/*.h")
file(GLOB_RECURSE private_headers "src/*.h")

add_library(common STATIC ${sources} ${public_headers} ${private_headers} ${outfile})

add_dependencies(common glm_external opencl_cpp_external cereal_external)

target_include_directories(common
    PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
    ${WAYVERB_SOURCE_DIR}/include
    PRIVATE
    src
    ${CMAKE_BINARY_DIR}/include
)

target_link_libraries(common
    ${opencl_lib}
    assimp
    zlibstatic 
    fftwf 
    audio_file 
    frequency_domain
    utilities
    hrtf
)

add_subdirectory(tests)
