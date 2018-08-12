set(T9_SRCS
    cannon.cpp
    lcdrange.cpp
    main.cpp
    )

set(T9_HEADERS
    cannon.h
    lcdrange.h
    )

qt1_wrap_moc(T9_SRCS SOURCES ${T9_HEADERS})

add_executable(t9 ${T9_SRCS})
target_link_libraries(t9 PRIVATE Qt::Qt1)
install(TARGETS t9 RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
