include(GoogleTest)

find_package(GTest REQUIRED)

add_executable(compiler_gtests
    ir_builder_tests.cpp
    dom_tree_tests.cpp
    peephole_tests.cpp
    checks_elemination_tests.cpp
    graph_inlining_tests.cpp
)

target_compile_options(compiler_gtests PUBLIC -g -O0 -Wno-unused-lambda-capture)

target_link_libraries(compiler_gtests
    PUBLIC jit_compiler
    PUBLIC GTest::gtest_main
)

gtest_discover_tests(compiler_gtests)