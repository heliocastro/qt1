set(T6_SRCS
    main.cpp
    )

add_executable(t6 ${T6_SRCS})
target_link_libraries(t6 PRIVATE Qt::Qt1)
install(TARGETS t6 RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
