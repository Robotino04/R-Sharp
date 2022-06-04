cmake_minimum_required(VERSION 3.0)

project("R Sharp")

enable_testing()

macro(add_valiadtive_test test_name)
    add_test(NAME ${test_name} COMMAND compilerTester $<TARGET_FILE:rsc> "${CMAKE_SOURCE_DIR}/tests/${test_name}.rs" "${CMAKE_BINARY_DIR}/tests")
endmacro()

add_subdirectory("compiler")

add_executable(compilerTester "tests/compilerTester.cpp")

file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/tests")

add_valiadtive_test(return0)
add_valiadtive_test(return42)