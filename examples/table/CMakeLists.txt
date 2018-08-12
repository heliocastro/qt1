set(TABLE_SRCS
    main.cpp
    table.cpp
    )

set(TABLE_HEADERS
    table.h
    )

qt1_wrap_moc(TABLE_SRCS SOURCES ${TABLE_HEADERS})

add_executable(table ${TABLE_SRCS})
target_link_libraries(table PRIVATE Qt::Qt1)
install(TARGETS table RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
