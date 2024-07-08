cmake_minimum_required(VERSION 3.5)

project(muzi_go)


include_directories(
    "${PROJECT_SOURCE_DIR}"
    )
add_subdirectory(muzi/base)
link_libraries(muzi_base)

add_executable(${PROJECT_NAME} main.cpp)

