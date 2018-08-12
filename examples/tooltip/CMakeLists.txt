set(TOOLTIP_SRCS
    main.cpp
    tooltip.cpp
    )

set(TOOLTIP_HEADERS
    tooltip.h
    )

qt1_wrap_moc(TOOLTIP_SRCS SOURCES ${TOOLTIP_HEADERS})

add_executable(tooltip ${TOOLTIP_SRCS})
target_link_libraries(tooltip PRIVATE Qt::Qt1)
install(TARGETS tooltip RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
