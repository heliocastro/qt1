set(LAYOUT_SRCS
    layout.cpp
    )

add_executable(layout ${LAYOUT_SRCS})
target_link_libraries(layout PRIVATE Qt::Qt1)
install(TARGETS layout RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
