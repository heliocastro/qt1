#=============================================================================
# Copyright 2018       Helio Chissini de Castro <helio@kde.org>
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#
# 1. Redistributions of source code must retain the copyright
#    notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
# 3. The name of the author may not be used to endorse or promote products
#    derived from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
# IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
# OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
# IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
# INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
# NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
# THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

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
