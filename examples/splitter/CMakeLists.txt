set(SPLITTER_SRCS
    splitter.cpp
    )

add_executable(splitter ${SPLITTER_SRCS})
target_link_libraries(splitter PRIVATE Qt::Qt1)
install(TARGETS splitter RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
