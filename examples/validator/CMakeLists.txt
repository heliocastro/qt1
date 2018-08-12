set(VALIDATOR_SRCS
    main.cpp
    motor.cpp
    vw.cpp
    )

set(VALIDATOR_HEADERS
    motor.h
    vw.h
    )

qt1_wrap_moc(VALIDATOR_SRCS SOURCES ${VALIDATOR_HEADERS})

add_executable(validator ${VALIDATOR_SRCS})
target_link_libraries(validator PRIVATE Qt::Qt1)
install(TARGETS validator RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
