set(QDIR_SRCS
    qdir.cpp
    )

add_executable(qdir ${QDIR_SRCS})
target_link_libraries(qdir PRIVATE Qt::Qt1)
install(TARGETS qdir RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
