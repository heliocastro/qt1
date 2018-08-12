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

set(WIDGETS_HDRS
    qbuttongroup.h
    qbutton.h
    qcheckbox.h
    qcombobox.h
    qframe.h
    qgroupbox.h
    qheader.h
    qlabel.h
    qlcdnumber.h
    qlineedit.h
    qlistbox.h
    qlistview.h
    qmainwindow.h
    qmenubar.h
    qmenudata.h
    qmultilinedit.h
    qpopupmenu.h
    qprogressbar.h
    qpushbutton.h
    qradiobutton.h
    qrangecontrol.h
    qscrollbar.h
    qscrollview.h
    qslider.h
    qspinbox.h
    qsplitter.h
    qstatusbar.h
    qtabbar.h
    qtableview.h
    qtoolbar.h
    qtoolbutton.h
    qtooltip.h
    qvalidator.h
    qwellarray.h
    qwhatsthis.h
    qwidgetstack.h
)

set(WIDGETS_SRCS
    qbuttongroup.cpp
    qbutton.cpp
    qcheckbox.cpp
    qcombobox.cpp
    qframe.cpp
    qgroupbox.cpp
    qheader.cpp
    qlabel.cpp
    qlcdnumber.cpp
    qlineedit.cpp
    qlistbox.cpp
    qlistview.cpp
    qmainwindow.cpp
    qmenubar.cpp
    qmenudata.cpp
    qmultilinedit.cpp
    qpopupmenu.cpp
    qprogressbar.cpp
    qpushbutton.cpp
    qradiobutton.cpp
    qrangecontrol.cpp
    qscrollbar.cpp
    qscrollview.cpp
    qslider.cpp
    qspinbox.cpp
    qsplitter.cpp
    qstatusbar.cpp
    qtabbar.cpp
    qtableview.cpp
    qtoolbar.cpp
    qtoolbutton.cpp
    qtooltip.cpp
    qvalidator.cpp
    qwellarray.cpp
    qwhatsthis.cpp
    qwidgetstack.cpp
)

qt1_wrap_moc(WIDGETS_SRCS
    SOURCES ${WIDGETS_HDRS}
    )

add_qt1_object_library(widgets
    SOURCES
    ${WIDGETS_SRCS}
    )
