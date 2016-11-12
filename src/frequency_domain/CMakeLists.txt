cmake_minimum_required(VERSION 3.0)

file(GLOB_RECURSE sources "src/*.cpp")
file(GLOB_RECURSE public_headers "include/*.h")
file(GLOB_RECURSE private_headers "src/*.h")

add_library(frequency_domain STATIC ${sources} ${public_headers} ${private_headers})

target_include_directories(frequency_domain
    PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
    PRIVATE
    ${CMAKE_CURRENT_SRC_DIR}/src
    ${DEPENDENCY_INSTALL_PREFIX}/include
)

target_link_libraries(frequency_domain fftwf utilities)

add_subdirectory(tests)
