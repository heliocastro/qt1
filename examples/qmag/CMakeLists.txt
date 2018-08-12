set(QMAG_SRCS
    qmag.cpp
    )

qt1_wrap_cpp(QMAG_SRCS SOURCES qmag.cpp)

add_executable(qmag ${QMAG_SRCS})
target_link_libraries(qmag PRIVATE Qt::Qt1)
install(TARGETS qmag RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
