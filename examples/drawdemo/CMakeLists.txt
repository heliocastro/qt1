set(DRAWDEMO_SRCS
    drawdemo.cpp
    )

qt1_wrap_cpp(DRAWDEMO_SRCS SOURCES drawdemo.cpp)

add_executable(drawdemo ${DRAWDEMO_SRCS})
target_link_libraries(drawdemo PRIVATE Qt::Qt1)
install(TARGETS drawdemo RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
