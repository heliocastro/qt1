set(WIDGETS_HEADERS
    ${PROJECT_SOURCE_DIR}/examples/aclock/aclock.h
    ${PROJECT_SOURCE_DIR}/examples/dclock/dclock.h
)
set(WIDGETS_SOURCES
    widgets.cpp
    ${PROJECT_SOURCE_DIR}/examples/aclock/aclock.cpp
    ${PROJECT_SOURCE_DIR}/examples/dclock/dclock.cpp
    )

qt1_wrap_moc(WIDGETS_SOURCES SOURCES ${WIDGETS_HEADERS})
qt1_wrap_cpp(WIDGETS_SOURCES SOURCES widgets.cpp)

add_executable(widgets_example ${WIDGETS_SOURCES})
set_target_properties(widgets_example PROPERTIES OUTPUT_NAME widgets)
target_link_libraries(widgets_example PRIVATE Qt::Qt1)
