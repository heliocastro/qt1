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

add_subdirectory(moc)
add_subdirectory(kernel)
add_subdirectory(dialogs)
add_subdirectory(widgets)
add_subdirectory(tools)

add_library(Qt1
    $<TARGET_OBJECTS:kernel>
    $<TARGET_OBJECTS:dialogs>
    $<TARGET_OBJECTS:widgets>
    $<TARGET_OBJECTS:tools>
)

add_library(Qt::Qt1 ALIAS Qt1)

target_link_libraries(Qt1
    PRIVATE
        ${X11_LIBRARIES}
    )

set_target_properties(Qt1 PROPERTIES
    OUTPUT_NAME qt1
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    LINK_FLAGS "-Wl,--as-needed -Wl,--no-undefined"
    )

target_include_directories(Qt1
    PUBLIC
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/qt1>
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    PRIVATE
        ${X11_INCLUDE_DIRS}
    )

install(TARGETS Qt1 EXPORT Qt1Targets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/Qt1/Qt1ConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY AnyNewerVersion
)

export(EXPORT Qt1Targets
    FILE "${CMAKE_CURRENT_BINARY_DIR}/Qt1/Qt1Targets.cmake"
    NAMESPACE Qt::
)

configure_file(${PROJECT_SOURCE_DIR}/cmake/Qt1Config.cmake
    "${CMAKE_CURRENT_BINARY_DIR}/Qt1/Qt1Config.cmake"
    COPYONLY
)

install(EXPORT Qt1Targets
    FILE Qt1Targets.cmake
    NAMESPACE Qt::
    DESTINATION ${CMAKE_INSTALL_DATADIR}/cmake/Qt1
    )

install(FILES
    ${PROJECT_SOURCE_DIR}/cmake/Qt1Config.cmake
    ${PROJECT_SOURCE_DIR}/cmake/Qt1Utils.cmake
    "${CMAKE_CURRENT_BINARY_DIR}/Qt1/Qt1ConfigVersion.cmake"
    DESTINATION ${CMAKE_INSTALL_DATADIR}/cmake/Qt1
    COMPONENT Devel
)

