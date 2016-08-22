cmake_minimum_required(VERSION 3.0)

add_definitions(-DOBJ_PATH="${CMAKE_SOURCE_DIR}/demo/assets/test_models/vault.obj")
add_definitions(-DOBJ_PATH_TUNNEL="${CMAKE_SOURCE_DIR}/demo/assets/test_models/echo_tunnel.obj")
add_definitions(-DOBJ_PATH_BEDROOM="${CMAKE_SOURCE_DIR}/demo/assets/test_models/bedroom.obj")
add_definitions(-DOBJ_PATH_BAD_BOX="${CMAKE_SOURCE_DIR}/demo/assets/test_models/small_square.obj")

set(name image_source_comparison)
add_executable(${name} ${name}.cpp)

target_link_libraries(${name} raytracer gflags)

