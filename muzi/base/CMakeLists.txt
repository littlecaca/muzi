set(BASE_SRCS
    mutex.cc
    thread.cc
    current_thread.cc
    condition.cc
    log_stream.cc
    logger.cc
    timezone.cc
    timestamp.cc
    config.cc
    outputer.cc
    async_outputer.cc
    countdown_latch.cc
    process_info.cc
    stacktrace.cc
    log_file.cc
    thread_pool.cc
)

add_library(muzi_base ${BASE_SRCS})

target_include_directories(muzi_base
                        PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# add the interface compiler flags
target_link_libraries(muzi_base muzi_compiler_flags)
target_link_libraries(muzi_base pthread rt)

# build test
if (MUZI_BUILD_TEST)
    add_subdirectory(test)
endif()

install(TARGETS muzi_base
        ARCHIVE DESTINATION lib        
)

install(DIRECTORY ./ DESTINATION include
        FILES_MATCHING PATTERN "*.h"    
)
