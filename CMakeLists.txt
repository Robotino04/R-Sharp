cmake_minimum_required(VERSION 3.0)

cmake_policy(SET CMP0110 NEW)
project("R Sharp")
enable_testing()
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ";${CMAKE_SOURCE_DIR}/cmake/Modules/")


macro(add_valiadtive_test test_name test_file, out_dir out_lang)
    add_test(NAME "${test_name} (${out_lang})" COMMAND compilerTester $<TARGET_FILE:rsc> ${test_file} ${out_dir} ${out_lang})
endmacro()

add_subdirectory("compiler")

add_executable(compilerTester "tests/compilerTester.cpp")

file(GLOB_RECURSE RSHARP_TESTS "tests/*.rs")

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

foreach(test_file ${RSHARP_TESTS})
    file(RELATIVE_PATH test_name "${CMAKE_SOURCE_DIR}/tests" ${test_file})

    get_filename_component(test_dir ${test_name}, DIRECTORY)
    string(REPLACE "invalid/" "" test_dir ${test_dir})
    string(REPLACE "valid/" "" test_dir ${test_dir})

    list(APPEND TEST_GROUPS ${test_dir})
endforeach(test_file)

list(REMOVE_DUPLICATES TEST_GROUPS)


foreach(group ${TEST_GROUPS})
    add_custom_target(test_${group} COMMAND ${CMAKE_CTEST_COMMAND} -R "/${group}/" DEPENDS rsc compilerTester)
    add_custom_target(test_${group}_nasm COMMAND ${CMAKE_CTEST_COMMAND} -R "\"/${group}/.*\\(nasm\\)\"" DEPENDS rsc compilerTester)
    add_custom_target(test_${group}_c COMMAND ${CMAKE_CTEST_COMMAND} -R "\"/${group}/.*\\(c\\)\"" DEPENDS rsc compilerTester)
    add_custom_target(test_${group}_aarch64 COMMAND ${CMAKE_CTEST_COMMAND} -R "\"/${group}/.*\\(aarch64\\)\"" DEPENDS rsc compilerTester)
endforeach(group)

add_custom_target(test_nasm COMMAND ${CMAKE_CTEST_COMMAND} -R "\".*\\(nasm\\)\"" DEPENDS rsc compilerTester)
add_custom_target(test_aarch64 COMMAND ${CMAKE_CTEST_COMMAND} -R "\".*\\(aarch64\\)\"" DEPENDS rsc compilerTester)
add_custom_target(test_c COMMAND ${CMAKE_CTEST_COMMAND} -R "\".*\\(c\\)\"" DEPENDS rsc compilerTester)
