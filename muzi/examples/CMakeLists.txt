add_executable(echo_server echo_server.cc)
target_link_libraries(echo_server PRIVATE muzi_net)
install(TARGETS echo_server
        DESTINATION ${MUZI_TEST_PATH})

add_executable(echo_client echo_client.cc)
target_link_libraries(echo_client PRIVATE muzi_net)
install(TARGETS echo_client
        DESTINATION ${MUZI_TEST_PATH})