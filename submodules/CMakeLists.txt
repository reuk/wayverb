cmake_minimum_required(VERSION 3.0)

project(WAYVERB_SUBMODULES VERSION 0.1 LANGUAGES CXX)

add_subdirectory(gflags)
add_subdirectory(googletest)

option(WITH_GFLAGS "" OFF)
add_subdirectory(glog)

option(ASSIMP_BUILD_TESTS "" OFF)
add_subdirectory(assimp)

add_subdirectory(glm)
add_subdirectory(modern_gl_utils)

add_subdirectory(haart-core)

set(JUCE_MODULES "${CMAKE_CURRENT_SOURCE_DIR}/JUCE/modules" CACHE PATH "")

add_subdirectory(ConvolutionComponent)
add_subdirectory(UtilityComponents)
add_subdirectory(OtherComponents)
