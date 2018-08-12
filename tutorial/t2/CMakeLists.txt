set(T2_SRCS
    main.cpp
    )

add_executable(t2 ${T2_SRCS})
target_link_libraries(t2 PRIVATE Qt::Qt1)
install(TARGETS t2 RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
