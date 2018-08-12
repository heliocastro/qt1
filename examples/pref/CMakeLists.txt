set(PREF_SRCS
    main.cpp
    pref.cpp
    )

set(PREF_HEADERS
    pref.h
    )

qt1_wrap_moc(PREF_SRCS SOURCES ${PREF_HEADERS})

add_executable(pref ${PREF_SRCS})
target_link_libraries(pref PRIVATE Qt::Qt1)
install(TARGETS pref RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
