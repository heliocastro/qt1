set(T4_SRCS
    main.cpp
    )

add_executable(t4 ${T4_SRCS})
target_link_libraries(t4 PRIVATE Qt::Qt1)
install(TARGETS t4 RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
