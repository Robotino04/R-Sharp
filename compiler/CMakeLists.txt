cmake_minimum_required(VERSION 3.27)
file(GLOB_RECURSE RSHARP_SRC "src/*.cpp" "src/*.c")
file(GLOB_RECURSE RSHARP_HEADERS "include/R-Sharp/*.hpp" "include/R-Sharp/*.h")

add_executable(rsc ${RSHARP_SRC} ${RSHARP_HEADERS})
target_include_directories(rsc PUBLIC "include/")

target_link_libraries(rsc PUBLIC ANSI)

target_compile_features(rsc PUBLIC cxx_std_17)