add_executable(simple_example simple_example.cpp)
target_link_libraries(simple_example PRIVATE xor_smc)

add_executable(complex_test complex_test.cpp)
target_link_libraries(complex_test PRIVATE xor_smc)

add_executable(more_tests more_tests.cpp)
target_link_libraries(more_tests PRIVATE xor_smc)


add_executable(disaster_plan disaster_plan.cpp)
target_link_libraries(disaster_plan PRIVATE xor_smc)

