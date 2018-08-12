set(T3_SRCS
    main.cpp
    )

add_executable(t3 ${T3_SRCS})
target_link_libraries(t3 PRIVATE Qt::Qt1)
install(TARGETS t3 RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
