set(T14_SRCS
    cannon.cpp
    gamebrd.cpp
    lcdrange.cpp
    main.cpp
    )

set(T14_HEADERS
    cannon.h
    gamebrd.h
    lcdrange.h
    )

qt1_wrap_moc(T14_SRCS SOURCES ${T14_HEADERS})

add_executable(t14 ${T14_SRCS})
target_link_libraries(t14 PRIVATE Qt::Qt1)
install(TARGETS t14 RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
