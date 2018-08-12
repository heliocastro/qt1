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

set(KERNEL_HDRS
    qaccel.h
    qapplication.h
    qasyncimageio.h
    qasyncio.h
    qbitmap.h
    qbrush.h
    qclipboard.h
    qcolor.h
    qconnection.h
    qcursor.h
    qdialog.h
    qdragobject.h
    qdrawutil.h
    qdropsite.h
    qevent.h
    qfocusdata.h
    qfont.h
    qfontdata.h
    qfontinfo.h
    qfontmetrics.h
    qgmanager.h
    qiconset.h
    qimage.h
    qkeycode.h
    qlayout.h
    qmetaobject.h
    qmovie.h
    qobject.h
    qobjectdefs.h
    qobjectdict.h
    qobjectlist.h
    qpaintdevice.h
    qpaintdevicedefs.h
    qpainter.h
    qpalette.h
    qpaintdevicemetrics.h
    qpen.h
    qpicture.h
    qpixmap.h
    qpixmapcache.h
    qpointarray.h
    qpoint.h
    qprinter.h
    qrect.h
    qregion.h
    qsemimodal.h
    qsignal.h
    qsignalmapper.h
    qsignalslotimp.h
    qsize.h
    qsocketnotifier.h
    qtimer.h
    qwidget.h
    qwidgetintdict.h
    qwidgetlist.h
    qwindowdefs.h
    qwindow.h
    qwmatrix.h
)

set(KERNEL_SRCS
    qaccel.cpp
    qapplication.cpp
    qasyncimageio.cpp
    qasyncio.cpp
    qbitmap.cpp
    qclipboard.cpp
    qcolor.cpp
    qconnection.cpp
    qcursor.cpp
    qdialog.cpp
    qdragobject.cpp
    qdrawutil.cpp
    qdropsite.cpp
    qevent.cpp
    qfocusdata.cpp
    qfont.cpp
    qgmanager.cpp
    qiconset.cpp
    qimage.cpp
    qlayout.cpp
    qmetaobject.cpp
    qmovie.cpp
    qobject.cpp
    qpainter.cpp
    qpalette.cpp
    qpaintdevicemetrics.cpp
    qpicture.cpp
    qpixmap.cpp
    qpixmapcache.cpp
    qpointarray.cpp
    qpoint.cpp
    qprinter.cpp
    qrect.cpp
    qregion.cpp
    qsemimodal.cpp
    qsignal.cpp
    qsignalmapper.cpp
    qsize.cpp
    qsocketnotifier.cpp
    qtimer.cpp
    qwidget.cpp
    qwindow.cpp
    qwmatrix.cpp
)

if(UNIX)
    list(APPEND KERNEL_SRCS
        qapplication_x11.cpp
        qclipboard_x11.cpp
        qcolor_x11.cpp
        qcursor_x11.cpp
        qdnd_x11.cpp
        qfont_x11.cpp
        qpicture_x11.cpp
        qpixmap_x11.cpp
        qprinter_x11.cpp
        qpaintdevice_x11.cpp
        qpainter_x11.cpp
        qregion_x11.cpp
        qwidget_x11.cpp
        qpsprinter.cpp
        qnpsupport.cpp
        qwidgetcreate_x11.cpp
    )
endif()

qt1_wrap_moc(KERNEL_SRCS
    SOURCES ${KERNEL_HDRS}
    )

add_qt1_object_library(kernel
    SOURCES
    ${KERNEL_SRCS}
    )
