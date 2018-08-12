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

set(TOOLS_HDRS
    qarray.h
    qbitarray.h
    qbuffer.h
    qcache.h
    qcollection.h
    qdatetime.h
    qdict.h
    qdir.h
    qdatastream.h
    qfile.h
    qfiledefs.h
    qfileinfo.h
    qgarray.h
    qgcache.h
    qgdict.h
    qgeneric.h
    qglist.h
    qglobal.h
    qgvector.h
    qintcache.h
    qintdict.h
    qiodevice.h
    qlist.h
    qptrdict.h
    qqueue.h
    qregexp.h
    qshared.h
    qstack.h
    qstring.h
    qstrlist.h
    qstrvec.h
    qtextstream.h
    qvector.h
    )

set(TOOLS_SRCS
    qbitarray.cpp
    qbuffer.cpp
    qcollection.cpp
    qdatetime.cpp
    qdir.cpp
    qdatastream.cpp
    qfile.cpp
    qfileinfo.cpp
    qgarray.cpp
    qgcache.cpp
    qgdict.cpp
    qglist.cpp
    qglobal.cpp
    qgvector.cpp
    qiodevice.cpp
    qregexp.cpp
    qstring.cpp
    qtextstream.cpp
)

qt1_wrap_moc(TOOLS_SRCS
    SOURCES ${TOOLS_HDRS}
    )

add_qt1_object_library(tools
    SOURCES
    ${TOOLS_SRCS}
    )
