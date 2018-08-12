set(DIRVIEW_SRCS
    dirview.cpp
    main.cpp
    )

set(DIRVIEW_HEADERS
    dirview.h
    )

qt1_wrap_moc(DIRVIEW_SRCS SOURCES ${DIRVIEW_HEADERS})

add_executable(dirview ${DIRVIEW_SRCS})
target_link_libraries(dirview PRIVATE Qt::Qt1)
install(TARGETS dirview RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
