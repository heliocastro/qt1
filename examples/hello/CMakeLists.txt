set(HELLO_SRCS
    hello.cpp
    main.cpp
    )

set(HELLO_HEADERS
    hello.h
    )

qt1_wrap_moc(HELLO_SRCS SOURCES ${HELLO_HEADERS})

add_executable(hello ${HELLO_SRCS})
target_link_libraries(hello PRIVATE Qt::Qt1)
install(TARGETS hello RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
