cmake_minimum_required(VERSION 3.5)

project(muzi_go)

# Setting c++11 standard with the interface libraries
add_library(muzi_compiler_flags INTERFACE)
target_compile_features(muzi_compiler_flags INTERFACE cxx_std_14)

add_subdirectory(muzi/base)
link_libraries(muzi_base)

add_executable(${PROJECT_NAME} main.cpp)
