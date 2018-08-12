set(T5_SRCS
    main.cpp
    )

add_executable(t5 ${T5_SRCS})
target_link_libraries(t5 PRIVATE Qt::Qt1)
install(TARGETS t5 RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
