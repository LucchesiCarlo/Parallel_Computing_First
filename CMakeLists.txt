cmake_minimum_required(VERSION 4.0)
project(First_Assignment)

set(CMAKE_CXX_STANDARD 20)

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
elseif (CMAKE_BUILD_TYPE STREQUAL "Release")
    add_compile_options("-O3")
endif ()
add_executable(First_Assignment main.cpp)
target_compile_features(First_Assignment PRIVATE cxx_std_17)
target_link_libraries(First_Assignment PRIVATE SFML::Graphics)