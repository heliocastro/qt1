set(ACLOCK_SRCS aclock.cpp main.cpp)
set(ACLOCK_HEADERS aclock.h)
qt1_wrap_moc(ACLOCK_SRCS SOURCES ${ACLOCK_HEADERS})

add_executable(aclock ${ACLOCK_SRCS})
target_link_libraries(aclock PRIVATE Qt::Qt1)
