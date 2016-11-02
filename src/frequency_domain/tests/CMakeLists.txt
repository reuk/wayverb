cmake_minimum_required(VERSION 3.0)

include_directories(${CMAKE_BINARY_DIR}/include)

file(GLOB sources "*.cpp")

add_executable(frequency_domain_tests ${sources})
target_link_libraries(frequency_domain_tests frequency_domain audio_file gtest)
add_test(NAME frequency_domain_tests COMMAND frequency_domain_tests)
