set(CONNECT_SRCS
    connect.cpp
    )

add_executable(connect ${CONNECT_SRCS})
target_link_libraries(connect PRIVATE Qt::Qt1)
install(TARGETS connect RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
