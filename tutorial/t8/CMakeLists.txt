set(T8_SRCS
    cannon.cpp
    lcdrange.cpp
    main.cpp
    )

set(T8_HEADERS
    cannon.h
    lcdrange.h
    )

qt1_wrap_moc(T8_SRCS SOURCES ${T8_HEADERS})

add_executable(t8 ${T8_SRCS})
target_link_libraries(t8 PRIVATE Qt::Qt1)
install(TARGETS t8 RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
