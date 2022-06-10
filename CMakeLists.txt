cmake_minimum_required(VERSION 3.0)

project("R Sharp")

enable_testing()

macro(add_valiadtive_test test_name test_file, out_dir)
    add_test(NAME ${test_name} COMMAND compilerTester $<TARGET_FILE:rsc> ${test_file} ${out_dir})
endmacro()

add_subdirectory("compiler")

add_executable(compilerTester "tests/compilerTester.cpp")

file(GLOB_RECURSE RSHARP_TESTS "tests/*.rs")

foreach(test_file ${RSHARP_TESTS})
    file(RELATIVE_PATH test_name "${CMAKE_SOURCE_DIR}/tests" ${test_file})

    get_filename_component(test_dir ${test_name}, DIRECTORY)
    get_filename_component(test_filename ${test_name}, NAME_WE)

    add_valiadtive_test("${test_dir}/${test_filename}" ${test_file} "${CMAKE_BINARY_DIR}/tests/${test_dir}")

    file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/tests/${test_dir}")
endforeach()