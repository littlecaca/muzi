cmake_minimum_required(VERSION 3.5)

project(muzi_go)

enable_testing()
set(TEST_SCRIPT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/scripts/test)

# Functions to create test quickly
function(do_test lib target arg)
    add_executable(${target} ${target}.cc)
    target_link_libraries(${target} PRIVATE ${lib})
    add_test(NAME ${target} COMMAND ${target} ${arg})
endfunction(do_test)

function(do_bad_test lib target arg)
    add_executable(${target} ${target}.cc)
    target_link_libraries(${target} PRIVATE ${lib})
    add_test(NAME ${target} COMMAND ${CMAKE_COMMAND} -E env $<TARGET_FILE:${target}> ${arg})
    set_property(TEST ${target} PROPERTY WILL_FAIL true)
endfunction(do_bad_test)

option(MUZI_BUILD_TEST "build test program" ON)

# Set compiler flag

# By default, only symbols that are used within the executable itself are included 
# in its symbol table, while other symbols are not.
# The -rdynamic flag is used to ensure that symbols from the main 
# executable are added to the dynamic symbol table, making them available 
# for dynamic symbol resolution at runtime.
add_link_options(-rdynamic)

# Setting c++11 standard with the interface libraries
add_library(muzi_compiler_flags INTERFACE)
target_compile_features(muzi_compiler_flags INTERFACE cxx_std_17)

add_subdirectory(muzi/base)
link_libraries(muzi_base)
add_subdirectory(muzi/net)
link_libraries(muzi_net)


add_executable(${PROJECT_NAME} main.cc)
