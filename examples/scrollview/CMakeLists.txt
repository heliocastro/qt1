set(SCROLLVIEW_SRCS
    scrollview.cpp
    )

qt1_wrap_cpp(SCROLLVIEW_SRCS SOURCES scrollview.cpp)

add_executable(scrollview ${SCROLLVIEW_SRCS})
target_link_libraries(scrollview PRIVATE Qt::Qt1)
install(TARGETS scrollview RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
