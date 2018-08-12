set(FOREVER_SRCS
    forever.cpp
    )

add_executable(forever ${FOREVER_SRCS})
target_link_libraries(forever PRIVATE Qt::Qt1)
install(TARGETS forever RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
