cmake_minimum_required(VERSION 3.0)

add_executable(compilerTester "compilerTester.cpp")

macro(add_valiadtive_test test_name test_file, out_dir out_lang)
    add_test(NAME "${test_name}_${out_lang}" COMMAND compilerTester $<TARGET_FILE:rsc> ${test_file} ${out_dir} ${out_lang})
endmacro()


file(GLOB_RECURSE RSHARP_TESTS "*.rs")

foreach(test_file ${RSHARP_TESTS})
    file(RELATIVE_PATH test_name "${CMAKE_SOURCE_DIR}/tests" ${test_file})

    get_filename_component(test_dir ${test_name}, DIRECTORY)
    get_filename_component(test_filename ${test_name}, NAME_WE)

    add_valiadtive_test("${test_dir}/${test_filename}" ${test_file} "${CMAKE_BINARY_DIR}/tests/${test_dir}" "c")
    
    # only add tests for the native architecture
    if(${CMAKE_SYSTEM_PROCESSOR} MATCHES "x86_64")
        add_valiadtive_test("${test_dir}/${test_filename}" ${test_file} "${CMAKE_BINARY_DIR}/tests/${test_dir}" "nasm")
    else(${CMAKE_SYSTEM_PROCESSOR} MATCHES "aarch64")
        add_valiadtive_test("${test_dir}/${test_filename}" ${test_file} "${CMAKE_BINARY_DIR}/tests/${test_dir}" "aarch64")
    endif()

    file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/tests/${test_dir}")
endforeach()

# targets to only test a single generator
add_custom_target(test_nasm COMMAND ${CMAKE_CTEST_COMMAND} -R "\".*\\(nasm\\)\"" DEPENDS rsc compilerTester)
add_custom_target(test_aarch64 COMMAND ${CMAKE_CTEST_COMMAND} -R "\".*\\(aarch64\\)\"" DEPENDS rsc compilerTester)
add_custom_target(test_c COMMAND ${CMAKE_CTEST_COMMAND} -R "\".*\\(c\\)\"" DEPENDS rsc compilerTester)