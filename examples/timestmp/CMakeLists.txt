set(TIMESTMP_SRCS
    timestmp.cpp
    )

add_executable(timestmp ${TIMESTMP_SRCS})
target_link_libraries(timestmp PRIVATE Qt::Qt1)
install(TARGETS timestmp RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
