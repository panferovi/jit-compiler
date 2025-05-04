include(GoogleTest)

find_package(GTest REQUIRED)

add_executable(compiler_gtests
    ir_builder_tests.cpp
    dom_tree_tests.cpp
    peephole_tests.cpp
)

target_link_libraries(compiler_gtests
    PUBLIC jit_compiler
    PUBLIC GTest::gtest_main
)

gtest_discover_tests(compiler_gtests)