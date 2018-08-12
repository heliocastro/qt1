set(MOVIES_SRCS
    main.cpp
    )

qt1_wrap_cpp(MOVIES_SRCS SOURCES main.cpp)

add_executable(movies ${MOVIES_SRCS})
target_link_libraries(movies PRIVATE Qt::Qt1)
install(TARGETS movies RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
