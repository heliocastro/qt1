set(CURSOR_SRCS
    cursor.cpp
    )

add_executable(cursor ${CURSOR_SRCS})
target_link_libraries(cursor PRIVATE Qt::Qt1)
install(TARGETS cursor RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
