cmake_minimum_required(VERSION 3.0)

#file(GLOB_RECURSE sources "src/*.cpp")
file(GLOB_RECURSE public_headers "include/*.h")

add_library(hrtf INTERFACE 
    #    ${sources} 
    #    ${public_headers} 
)

target_include_directories(hrtf
    INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)
