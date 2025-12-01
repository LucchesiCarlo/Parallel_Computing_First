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

set(AOS_SOURCE mainAOS.cpp src/helpersAOS.cpp)
set(SOA_SOURCE mainSOA.cpp src/helpersSOA.cpp)

add_executable(sequentialAOS ${AOS_SOURCE} src/sequentialgenAOS.cpp)
target_compile_features(sequentialAOS PRIVATE cxx_std_17)
target_link_libraries(sequentialAOS PRIVATE SFML::Graphics)

add_executable(parallelAOS ${AOS_SOURCE} src/parallelgenAOS.cpp)
target_compile_features(parallelAOS PRIVATE cxx_std_17)
target_link_libraries(parallelAOS PRIVATE SFML::Graphics)

add_executable(parallelSIMD_AOS ${AOS_SOURCE} src/parallelgenSIMD_AOS.cpp)
target_compile_features(parallelSIMD_AOS PRIVATE cxx_std_17)
target_link_libraries(parallelSIMD_AOS PRIVATE SFML::Graphics)

add_executable(parallelAVX_AOS ${AOS_SOURCE} src/parallelgenAVX_AOS.cpp)
target_compile_features(parallelAVX_AOS PRIVATE cxx_std_17)
target_link_libraries(parallelAVX_AOS PRIVATE SFML::Graphics)

add_executable(parallelSOA ${SOA_SOURCE} src/parallelgenSOA.cpp)
target_compile_features(parallelSOA PRIVATE cxx_std_17)
target_link_libraries(parallelSOA PRIVATE SFML::Graphics)

add_executable(parallelSIMD_SOA ${SOA_SOURCE} src/parallelgenSIMD_SOA.cpp)
target_compile_features(parallelSIMD_SOA PRIVATE cxx_std_17)
target_link_libraries(parallelSIMD_SOA PRIVATE SFML::Graphics)

add_executable(parallelAVX_SOA ${SOA_SOURCE} src/parallelgenAVX_SOA.cpp)
target_compile_features(parallelAVX_SOA PRIVATE cxx_std_17)
target_link_libraries(parallelAVX_SOA PRIVATE SFML::Graphics)