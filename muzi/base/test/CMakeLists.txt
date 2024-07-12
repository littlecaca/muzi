add_executable(mutex_test mutex_test.cc)
target_link_libraries(mutex_test PRIVATE muzi_base)
add_test(NAME mutex_test COMMAND mutex_test)