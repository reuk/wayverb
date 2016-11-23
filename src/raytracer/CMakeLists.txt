cmake_minimum_required(VERSION 3.2)

file(GLOB_RECURSE sources "src/*.cpp")
file(GLOB_RECURSE public_headers "include/*.h")
file(GLOB_RECURSE private_headers "src/*.h")

add_library(raytracer STATIC ${sources} ${public_headers} ${private_headers})

add_dependencies(raytracer glm_external)

target_include_directories(raytracer
    PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
    PRIVATE
    src
    ${DEPENDENCY_INSTALL_PREFIX}/include
)

target_link_libraries(raytracer core)

add_subdirectory(tests)
