set(BASE_SRCS
    mutex.cpp
)

add_library(muzi_base ${BASE_SRCS})
target_link_libraries(muzi_base pthread)

