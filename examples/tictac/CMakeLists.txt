set(TICTAC_SRCS
    main.cpp
    tictac.cpp
    )

set(TICTAC_HEADERS
    tictac.h
    )

qt1_wrap_moc(TICTAC_SRCS SOURCES ${TICTAC_HEADERS})

add_executable(tictac ${TICTAC_SRCS})
target_link_libraries(tictac PRIVATE Qt::Qt1)
install(TARGETS tictac RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
