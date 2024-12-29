
add_executable(pingpong_client client.cc)
target_link_libraries(pingpong_client muzi_net muzi_base)

add_executable(pingpong_server server.cc)
target_link_libraries(pingpong_server muzi_net muzi_base)

add_executable(pingpong_bench bench.cc)
target_link_libraries(pingpong_bench muzi_net muzi_base)

install (TARGETS pingpong_client pingpong_server pingpong_bench
    DESTINATION ${MUZI_EXAMPLE_PATH}
)