set(SHOWIMG_SRCS
    main.cpp
    showimg.cpp
    )

set(SHOWIMG_HEADERS
    showimg.h
    )

qt1_wrap_moc(SHOWIMG_SRCS SOURCES ${SHOWIMG_HEADERS})

add_executable(showimg ${SHOWIMG_SRCS})
target_link_libraries(showimg PRIVATE Qt::Qt1)
install(TARGETS showimg RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
