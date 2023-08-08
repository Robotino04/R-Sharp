cmake_minimum_required(VERSION 3.5...3.27)

add_executable(compilerTester "compilerTester.cpp")
set(RSHARP_STDLIB "${CMAKE_SOURCE_DIR}/stdlib/")
set(RSHARP_TEST_LIB_SRC "${RSHARP_STDLIB}/test/pointers.cpp")
get_filename_component(RSHARP_TEST_LIB_SRC "${RSHARP_TEST_LIB_SRC}" REALPATH)


function(add_compilation_test test_name test_file, out_dir out_lang)
    set (extra_args ${ARGN})

    # Did we get any optional args?
    list(LENGTH extra_args extra_count)
    if (${extra_count} GREATER 1)
        list(GET extra_args 0 proxy)

        if (${extra_count} GREATER_EQUAL 2)
            list(GET extra_args 1 compiler)
        else()
            message(FATAL_ERROR "if a proxy is given then so must a compiler be")
        endif()
    endif()

    add_test(NAME "${test_name}_${out_lang}" COMMAND compilerTester $<TARGET_FILE:rsc> ${test_file} ${out_dir} ${out_lang} ${RSHARP_TEST_LIB_SRC} ${RSHARP_STDLIB} ${proxy} ${compiler})
endfunction()


file(GLOB_RECURSE RSHARP_TESTS "*.rs")

foreach(test_file ${RSHARP_TESTS})
    file(RELATIVE_PATH test_name "${CMAKE_SOURCE_DIR}/tests" ${test_file})

    get_filename_component(test_dir ${test_name}, DIRECTORY)
    get_filename_component(test_filename ${test_name}, NAME_WE)

    add_compilation_test("${test_dir}/${test_filename}" ${test_file} "${CMAKE_BINARY_DIR}/tests/${test_dir}" "c")
    
    # only add tests for the native architecture
    # run the other architectures through qemu
    if(${CMAKE_SYSTEM_PROCESSOR} MATCHES "x86_64")
        add_compilation_test("${test_dir}/${test_filename}" ${test_file} "${CMAKE_BINARY_DIR}/tests/${test_dir}" "nasm")
        add_compilation_test("${test_dir}/${test_filename}" ${test_file} "${CMAKE_BINARY_DIR}/tests/${test_dir}" "aarch64" "qemu-aarch64 -L /usr/aarch64-linux-gnu" "aarch64-linux-gnu-gcc")
    elseif(${CMAKE_SYSTEM_PROCESSOR} MATCHES "aarch64")
        add_compilation_test("${test_dir}/${test_filename}" ${test_file} "${CMAKE_BINARY_DIR}/tests/${test_dir}" "aarch64")
        add_compilation_test("${test_dir}/${test_filename}" ${test_file} "${CMAKE_BINARY_DIR}/tests/${test_dir}" "nasm" "qemu-x86_64 -L /usr/x86_64-linux-gnu" "x86_64-linux-gnu-gcc")
    else()
        message(FATAL_ERROR "Unsupported architecture for tests")
    endif()

    file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/tests/${test_dir}")
endforeach()

# targets to only test a single generator
add_custom_target(test_nasm COMMAND ${CMAKE_CTEST_COMMAND} -R "\".*\\(nasm\\)\"" DEPENDS rsc compilerTester)
add_custom_target(test_aarch64 COMMAND ${CMAKE_CTEST_COMMAND} -R "\".*\\(aarch64\\)\"" DEPENDS rsc compilerTester)
add_custom_target(test_c COMMAND ${CMAKE_CTEST_COMMAND} -R "\".*\\(c\\)\"" DEPENDS rsc compilerTester)