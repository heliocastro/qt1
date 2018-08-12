set(LAYOUTS_SRCS
    qtgrid.cpp
    qthbox.cpp
    qtvbox.cpp
    qtbuttonrow.cpp
    qtlabelled.cpp
    layouts.cpp
    )

set(LAYOUTS_HEADERS
    qtgrid.h
    qthbox.h
    qtvbox.h
    qtbuttonrow.h
    qtlabelled.h
    )

qt1_wrap_moc(LAYOUTS_SRCS SOURCES ${LAYOUTS_HEADERS})

add_executable(layouts ${LAYOUTS_SRCS})
target_link_libraries(layouts PRIVATE Qt::Qt1)
install(TARGETS layouts RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
