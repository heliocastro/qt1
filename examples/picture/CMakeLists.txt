set(PICTURE_SRCS
    picture.cpp
    )

add_executable(picture ${PICTURE_SRCS})
target_link_libraries(picture PRIVATE Qt::Qt1)
install(TARGETS picture RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
