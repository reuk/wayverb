cmake_minimum_required(VERSION 3.2)

file(GLOB_RECURSE sources "src/*.cpp")
file(GLOB_RECURSE public_headers "include/*.h")

add_library(audio_file STATIC ${sources} ${public_headers})

target_include_directories(audio_file
    PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
    PRIVATE
    ${DEPENDENCY_INSTALL_PREFIX}/include
)

target_link_libraries(audio_file sndfile)
