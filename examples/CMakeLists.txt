add_executable(simple_example simple_example.cpp)
target_link_libraries(simple_example PRIVATE xor_smc)

add_executable(complex_test complex_test.cpp)
target_link_libraries(complex_test PRIVATE xor_smc)

add_executable(smc_test smc_test.cpp)
target_link_libraries(smc_test PRIVATE xor_smc)