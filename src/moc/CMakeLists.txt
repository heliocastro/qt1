set(MOC_SRCS
    ${PROJECT_SOURCE_DIR}/src/tools/qbuffer.cpp
    ${PROJECT_SOURCE_DIR}/src/tools/qcollection.cpp
    ${PROJECT_SOURCE_DIR}/src/tools/qdatetime.cpp
    ${PROJECT_SOURCE_DIR}/src/tools/qdatastream.cpp
    ${PROJECT_SOURCE_DIR}/src/tools/qgarray.cpp
    ${PROJECT_SOURCE_DIR}/src/tools/qgdict.cpp
    ${PROJECT_SOURCE_DIR}/src/tools/qglist.cpp
    ${PROJECT_SOURCE_DIR}/src/tools/qglobal.cpp
    ${PROJECT_SOURCE_DIR}/src/tools/qgvector.cpp
    ${PROJECT_SOURCE_DIR}/src/tools/qiodevice.cpp
    ${PROJECT_SOURCE_DIR}/src/tools/qstring.cpp
    )

byacc_target(MyParser moc.y ${CMAKE_CURRENT_BINARY_DIR}/parser.cpp)
flex_target(MyScanner moc.l ${CMAKE_CURRENT_BINARY_DIR}/lexer.cpp)
add_flex_byacc_dependency(MyScanner MyParser)

add_executable(moc-qt1 ${MOC_SRCS} ${BYACC_MyParser_OUTPUTS})
target_compile_options(moc-qt1 PRIVATE -fno-strength-reduce)
target_include_directories(moc-qt1 PRIVATE ${PROJECT_SOURCE_DIR}/include)
install(TARGETS moc-qt1 RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
