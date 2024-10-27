cmake_minimum_required(VERSION 3.10)

project(JIT_COMPILER VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)

set(SRC_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR})

add_compile_options(-std=c++17 -Wall -Wextra -Werror)

add_subdirectory(tests)

add_library(jit_compiler STATIC
    utils/debug.cpp
    ir/graph.cpp
    ir/common.cpp
    ir/ir_builder.cpp
    ir/basic_block.cpp
    ir/instruction.cpp
    ir/analysis.cpp
)

target_include_directories(jit_compiler
    PUBLIC ${SRC_ROOT_DIR}
)

function (filter_items aItems aRegEx)
    # For each item in our list
    foreach (item ${${aItems}})
        # Check if our items matches our regular expression
        if ("${item}" MATCHES ${aRegEx})
            # Remove current item from our list
            list (REMOVE_ITEM ${aItems} ${item})
        endif ("${item}" MATCHES ${aRegEx})
    endforeach(item)
    # Provide output parameter
    set(${aItems} ${${aItems}} PARENT_SCOPE)
endfunction (filter_items)
file(GLOB_RECURSE EMULATOR_FILES *cpp *.h)
filter_items(EMULATOR_FILES "CMakeFiles/*")

find_program(CLANG_FORMAT "clang-format-14" REQUIRED)

add_custom_target(format
    COMMAND clang-format-14
    -i
    -style=file
    ${EMULATOR_FILES}
)
