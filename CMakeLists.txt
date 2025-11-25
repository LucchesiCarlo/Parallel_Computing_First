cmake_minimum_required(VERSION 4.0)
project(First_Assignment)

set(CMAKE_CXX_STANDARD 20)
if (UNIX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fopenmp")
else ()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Xpreprocessor -fopenmp -lomp")
endif ()

add_compile_options("-mavx")
add_compile_options("-mavx2")

#Code copied from the SFML site for CMake configurations
include(FetchContent)
FetchContent_Declare(SFML
        GIT_REPOSITORY https://github.com/SFML/SFML.git
        GIT_TAG 3.0.2
        GIT_SHALLOW ON
        EXCLUDE_FROM_ALL
        SYSTEM)
FetchContent_MakeAvailable(SFML)

#Probably not the best way to do it
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_options("-fsanitize=address,leak,undefined")
    add_link_options("-fsanitize=address,leak,undefined")
    add_compile_options("-g")
elseif (CMAKE_BUILD_TYPE STREQUAL "DebugThreads")
    #To use this sanitizer you must disable ASLR (in LINUX echo 0 | sudo tee /proc/sys/kernel/randomize_va_space)
    add_compile_options("-fsanitize=thread")
    add_link_options("-fsanitize=thread")
    add_compile_options("-g")
    add_compile_options("-O2")
elseif (CMAKE_BUILD_TYPE STREQUAL "Release")
    add_compile_options("-O3")
    add_compile_options("-mfma")
    add_compile_options("-march=native")
    add_compile_options("-fvectorize")
    add_compile_options("-fno-math-errno")
elseif (CMAKE_BUILD_TYPE STREQUAL "Padding")
    add_compile_options("-O3")
    add_compile_options("-mfma")
    add_compile_options("-march=native")
    add_compile_options("-fvectorize")
    add_compile_options("-fno-math-errno")
    add_definitions("-DPADDING=1")
endif ()

add_executable(sequential sequential.cpp)
target_compile_features(sequential PRIVATE cxx_std_17)
target_link_libraries(sequential PRIVATE SFML::Graphics)

add_executable(parallel_AOS parallel_AOS.cpp)
target_compile_features(parallel_AOS PRIVATE cxx_std_17)
target_link_libraries(parallel_AOS PRIVATE SFML::Graphics)

add_executable(parallel_SOA parallel_SOA.cpp)
target_compile_features(parallel_SOA PRIVATE cxx_std_17)
target_link_libraries(parallel_SOA PRIVATE SFML::Graphics)

add_executable(parallel_AOS_simd parallel_AOS_simd.cpp)
target_compile_features(parallel_AOS_simd PRIVATE cxx_std_17)
target_link_libraries(parallel_AOS_simd PRIVATE SFML::Graphics)

add_executable(parallel_SOA_simd parallel_SOA_simd.cpp)
target_compile_features(parallel_SOA_simd PRIVATE cxx_std_17)
target_link_libraries(parallel_SOA_simd PRIVATE SFML::Graphics)

add_executable(parallel_AOS_simd_manual parallel_AOS_simd_manual.cpp)
target_compile_features(parallel_AOS_simd_manual PRIVATE cxx_std_17)
target_link_libraries(parallel_AOS_simd_manual PRIVATE SFML::Graphics)

add_executable(parallel_SOA_simd_manual parallel_SOA_simd_manual.cpp)
target_compile_features(parallel_SOA_simd_manual PRIVATE cxx_std_17)
target_link_libraries(parallel_SOA_simd_manual PRIVATE SFML::Graphics)