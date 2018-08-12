set(DESKTOP_SRCS
    desktop.cpp
    )

add_executable(desktop ${DESKTOP_SRCS})
target_link_libraries(desktop PRIVATE Qt::Qt1)
install(TARGETS desktop RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
