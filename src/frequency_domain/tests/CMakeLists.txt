include_directories(${DEPENDENCY_INSTALL_PREFIX}/include)

file(GLOB sources "*.cpp")

add_executable(frequency_domain_tests ${sources})
target_link_libraries(frequency_domain_tests frequency_domain audio_file gtest)
add_test(NAME frequency_domain_tests COMMAND frequency_domain_tests)
