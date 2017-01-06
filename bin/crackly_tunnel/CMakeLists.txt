add_definitions(-DOBJ_PATH_TUNNEL="${CMAKE_SOURCE_DIR}/demo/assets/test_models/echo_tunnel.obj")

set(name crackly_tunnel)
add_executable(${name} ${name}.cpp)

target_link_libraries(${name} combined)
