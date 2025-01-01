aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR} NET_SRCS)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/poller NET_SRCS)

add_library(muzi_net ${NET_SRCS})

target_include_directories(muzi_net 
                        PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(muzi_net muzi_compiler_flags)

if (MUZI_BUILD_TEST)
    add_subdirectory(test)
endif()

install(TARGETS muzi_net
        LIBRARY DESTINATION ${MUZI_LIB_PATH}
        ARCHIVE DESTINATION ${MUZI_LIB_PATH}
)

install(DIRECTORY ./ DESTINATION ${MUZI_INCLUDE_PATH}
        FILES_MATCHING PATTERN "*.h"    
)
