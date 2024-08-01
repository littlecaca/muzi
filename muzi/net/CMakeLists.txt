set(NET_SRCS
    event_loop.cc
)

add_library(muzi_net ${NET_SRCS})

target_include_directories(muzi_net 
                        INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(muzi_net muzi_compiler_flags)

if (MUZI_BUILD_TEST)
    add_subdirectory(test)
endif()
