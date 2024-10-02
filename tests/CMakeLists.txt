include(GoogleTest)

find_package(GTest REQUIRED)

add_executable(ir_builder_gtest
    ir_builder_tests.cpp
)

target_link_libraries(ir_builder_gtest
    PUBLIC jit_compiler
    PUBLIC GTest::gtest_main
)

gtest_discover_tests(ir_builder_gtest)