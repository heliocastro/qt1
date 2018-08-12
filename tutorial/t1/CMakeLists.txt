set(T1_SRCS
    main.cpp
    )

add_executable(t1 ${T1_SRCS})
target_link_libraries(t1 PRIVATE Qt::Qt1)
install(TARGETS t1 RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
