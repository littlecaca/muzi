set(NET_SRCS
    event_loop.cc
    channel.cc
    poller.cc
    poller/default_poller.cc
    poller/epoll_poller.cc
    timer_queue.cc
    timer.cc
    event_loop_thread.cc
    acceptor.cc
    inet_address.cc
    socket_ops.cc
    socket.cc
)

add_library(muzi_net ${NET_SRCS})

target_include_directories(muzi_net 
                        PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(muzi_net muzi_compiler_flags)

if (MUZI_BUILD_TEST)
    add_subdirectory(test)
endif()
