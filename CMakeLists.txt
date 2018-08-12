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

cmake_minimum_required(VERSION 3.2 FATAL_ERROR)

project(qt1
    VERSION 1.45
    LANGUAGES C CXX)

list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

include(GNUInstallDirs)
include(Qt1Utils)
include(Qt1SystemCheck)
include(Qt1InternalBuild)

set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_CXX_STANDARD 98)
set(BUILD_SHARED_LIBS ON)

option(ENABLE_THREAD_SUPPORT "Compile with Threading Support" ON)
option(ENABLE_OPENGL "Compile OpenGL module" ON)
option(BUILD_QT1_TUTORIAL "Build tutorials." ON)
option(BUILD_QT1_EXAMPLES "Build examples." ON)
option(INSTALL_QT_DOCS "Install Qt Documentation" ON)

find_package(PkgConfig REQUIRED)

if(ENABLE_THREAD_SUPPORT)
    find_package(Threads REQUIRED)
endif()

if(ENABLE_OPENGL)
    find_package(OpenGL REQUIRED)
endif()

if(SYSTEM_ZLIB)
    find_package(ZLIB REQUIRED)
endif()

find_package(X11 REQUIRED)
find_package(BYACC REQUIRED)
find_package(FLEX REQUIRED)

add_subdirectory(src)
add_subdirectory(include)

if(BUILD_QT1_TUTORIAL)
    add_subdirectory(tutorial)
endif()

if(BUILD_QT1_EXAMPLES)
    add_subdirectory(examples)
endif()

if(INSTALL_QT_DOCS)
    install(DIRECTORY html/ DESTINATION ${CMAKE_INSTALL_DOCDIR})
    if(UNIX)
        install(DIRECTORY man/ DESTINATION ${CMAKE_INSTALL_MANDIR})
    endif()

endif()
