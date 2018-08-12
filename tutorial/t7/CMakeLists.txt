set(T7_SRCS
    lcdrange.cpp
    main.cpp
    )

set(T7_HEADERS
    lcdrange.h
    )

qt1_wrap_moc(T7_SRCS SOURCES ${T7_HEADERS})

add_executable(t7 ${T7_SRCS})
target_link_libraries(t7 PRIVATE Qt::Qt1)
install(TARGETS t7 RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
