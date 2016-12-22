include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${DEPENDENCY_INSTALL_PREFIX}/include
)

file(GLOB sources "*.cpp")

add_definitions(-DSCRATCH_PATH="${CMAKE_BINARY_DIR}")

add_definitions(${test_file_flag})
add_definitions(-DOBJ_PATH="${CMAKE_SOURCE_DIR}/demo/assets/test_models/vault.obj")
add_definitions(-DMAT_PATH="${CMAKE_SOURCE_DIR}/demo/assets/materials/vault.json")
add_definitions(-DOBJ_PATH_TUNNEL="${CMAKE_SOURCE_DIR}/demo/assets/test_models/echo_tunnel.obj")
add_definitions(-DMAT_PATH_TUNNEL="${CMAKE_SOURCE_DIR}/demo/assets/materials/mat.json")
add_definitions(-DOBJ_PATH_BEDROOM="${CMAKE_SOURCE_DIR}/demo/assets/test_models/bedroom.obj")
add_definitions(-DMAT_PATH_BEDROOM="${CMAKE_SOURCE_DIR}/demo/assets/materials/mat.json")
add_definitions(-DOBJ_PATH_BAD_BOX="${CMAKE_SOURCE_DIR}/demo/assets/test_models/small_square.obj")
add_definitions(-DMAT_PATH_BAD_BOX="${CMAKE_SOURCE_DIR}/demo/assets/materials/damped.json")

add_executable(raytracer_tests ${sources})

target_link_libraries(raytracer_tests raytracer gtest)

add_test(NAME raytracer_tests COMMAND raytracer_tests)
