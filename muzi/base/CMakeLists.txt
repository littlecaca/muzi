aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR} BASE_SRCS)

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
        LIBRARY DESTINATION ${MUZI_LIB_PATH}
        ARCHIVE DESTINATION ${MUZI_LIB_PATH}        
)

install(DIRECTORY ./ DESTINATION ${MUZI_INCLUDE_PATH}
        FILES_MATCHING PATTERN "*.h"    
)
