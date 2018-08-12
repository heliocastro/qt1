set(XFORM_SRCS
    xform.cpp
    )

qt1_wrap_cpp(XFORM_SRCS SOURCES xform.cpp)

add_executable(xform ${XFORM_SRCS})
target_link_libraries(xform PRIVATE Qt::Qt1)
install(TARGETS xform RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
