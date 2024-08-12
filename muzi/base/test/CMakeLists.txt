add_executable(mutex_test mutex_test.cc)
target_link_libraries(mutex_test PRIVATE muzi_base)
add_test(NAME mutex_test COMMAND mutex_test)

add_executable(current_thread_test current_thread_test.cc)
target_link_libraries(current_thread_test PRIVATE muzi_base)
add_test(NAME current_thread_test COMMAND current_thread_test)

add_executable(condtion_test condtion_test.cc)
target_link_libraries(condtion_test PRIVATE muzi_base)
add_test(NAME condtion_test COMMAND condtion_test)

add_executable(fiexed_buffer_test fiexed_buffer_test.cc)
target_link_libraries(fiexed_buffer_test PRIVATE muzi_base)
add_test(NAME fiexed_buffer_test COMMAND fiexed_buffer_test)

add_executable(log_stream_test log_stream_test.cc)
target_link_libraries(log_stream_test PRIVATE muzi_base)
add_test(NAME log_stream_test COMMAND log_stream_test)

add_executable(timestamp_test timestamp_test.cc)
target_link_libraries(timestamp_test PRIVATE muzi_base)
add_test(NAME timestamp_test COMMAND timestamp_test)

add_executable(logger_test logger_test.cc)
target_link_libraries(logger_test PRIVATE muzi_base)
add_test(NAME logger_test COMMAND logger_test)

add_executable(async_ouputer_test async_ouputer_test.cc)
target_link_libraries(async_ouputer_test PRIVATE muzi_base)
add_test(NAME async_ouputer_test COMMAND async_ouputer_test)

add_executable(thread_test thread_test.cc)
target_link_libraries(thread_test PRIVATE muzi_base)
add_test(NAME thread_test COMMAND thread_test)

do_test(muzi_base stacktrace_test "")