set(TETRIX_SRCS
    gtetrix.cpp
    qdragapp.cpp
    qtetrix.cpp
    qtetrixb.cpp
    tetrix.cpp
    tpiece.cpp
    )

set(TETRIX_HEADERS
    gtetrix.h
    qdragapp.h
    qtetrix.h
    qtetrixb.h
    tpiece.h
    )

qt1_wrap_moc(TETRIX_SRCS SOURCES ${TETRIX_HEADERS})

qt1_wrap_cpp(TETRIX_SRCS SOURCES qdragapp.cpp)

add_executable(tetrix ${TETRIX_SRCS})
target_link_libraries(tetrix PRIVATE Qt::Qt1)
install(TARGETS tetrix RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
