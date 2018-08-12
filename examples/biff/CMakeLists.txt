set(BIFF_SRCS
    biff.cpp
    main.cpp
    )

set(BIFF_HEADERS
    biff.h
    )

qt1_wrap_moc(BIFF_SRCS SOURCES ${BIFF_HEADERS})

add_executable(biff ${BIFF_SRCS})
target_link_libraries(biff PRIVATE Qt::Qt1)
install(TARGETS biff RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
