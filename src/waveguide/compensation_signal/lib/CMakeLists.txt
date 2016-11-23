cmake_minimum_required(VERSION 3.3)

file(GLOB_RECURSE sources "src/*.cpp")
file(GLOB_RECURSE public_headers "include/*.h")
file(GLOB_RECURSE private_headers "src/*.h")

add_library(compensation_signal STATIC 
    ${sources} 
    ${public_headers} 
    ${private_headers})

target_include_directories(compensation_signal
    PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
    PRIVATE
    ${DEPENDENCY_INSTALL_PREFIX}/include
)

target_link_libraries(compensation_signal core)
