set(PROGRESS_SRCS
    progress.cpp
    )

qt1_wrap_cpp(PROGRESS_SRCS SOURCES progress.cpp)

add_executable(progress ${PROGRESS_SRCS})
target_link_libraries(progress PRIVATE Qt::Qt1)
install(TARGETS progress RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
