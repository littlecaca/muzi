function(do_test target arg)
    add_executable(${target} ${target}.cc)
    target_link_libraries(${target} PRIVATE muzi_net)
    add_test(NAME ${target} COMMAND ${target} ${arg})
endfunction(do_test)

do_test(event_loop_test "")

do_test(event_loop_test2 "")
set_tests_properties(event_loop_test2 PROPERTIES WILL_FAIL TRUE)