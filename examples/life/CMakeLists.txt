set(LIFE_SRCS
    life.cpp
    lifedlg.cpp
    main.cpp
    )

set(LIFE_HEADERS
    life.h
    lifedlg.h
    )

qt1_wrap_moc(LIFE_SRCS SOURCES ${LIFE_HEADERS})

add_executable(life ${LIFE_SRCS})
target_link_libraries(life PRIVATE Qt::Qt1)
install(TARGETS life RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
