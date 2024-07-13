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