cmake_minimum_required(VERSION 3.12)
project(code_opt_dev LANGUAGES CXX)
find_package(OpenMP REQUIRED)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB KERNEL_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_SOURCE_DIR}/src/kernel/*.cpp"
)

add_library(kernels STATIC ${KERNEL_SOURCES})

target_include_directories(kernels
    PUBLIC
        "${CMAKE_SOURCE_DIR}/include"
)

target_compile_options(kernels
    PRIVATE
        -O3 -march=native -fno-associative-math -fno-reciprocal-math -fno-trapping-math
)
target_include_directories(kernels PUBLIC include)
target_link_libraries(kernels PUBLIC OpenMP::OpenMP_CXX)

set_source_files_properties(
    ${CMAKE_SOURCE_DIR}/src/kernel/matmul.cpp
    PROPERTIES COMPILE_FLAGS "-O2 -fno-tree-vectorize"
)

# ---- Executables ----

add_executable(single_bench src/main/single_bench.cpp)
target_link_libraries(single_bench PRIVATE kernels)

add_executable(run_all src/main/run_all.cpp)
target_link_libraries(run_all PRIVATE kernels)
