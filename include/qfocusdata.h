/****************************************************************************
** $Id: qfocusdata.h,v 2.5.2.3 1998/08/25 09:20:52 hanord Exp $
**
** Definition of internal QFocusData class
**
** Created : 980405
**
** Copyright (C) 1992-1999 Troll Tech AS.  All rights reserved.
**
** This file is part of Qt Free Edition, version 1.45.
**
** See the file LICENSE included in the distribution for the usage
** and distribution terms, or http://www.troll.no/free-license.html.
**
** IMPORTANT NOTE: You may NOT copy this file or any part of it into
** your own programs or libraries.
**
** Please see http://www.troll.no/pricing.html for information about 
** Qt Professional Edition, which is this same library but with a
** license which allows creation of commercial/proprietary software.
**
*****************************************************************************/

#ifndef QFOCUSDATA_H
#define QFOCUSDATA_H

#ifndef QT_H
#include "qglobal.h"
#if defined(Q_TEMPLATEDLL)
#include "qwidgetlist.h"
#else
#include "qlist.h"
#include "qwidget.h"
#endif
#endif // QT_H


class Q_EXPORT QFocusData {
public:
    QWidget* focusWidget() const { return it.current(); }

    // List-iteration
    QWidget* home();
    QWidget* next();
    QWidget* prev();
    int count() const { return focusWidgets.count(); }

private:
    friend class QWidget;
    QFocusData()
	: it(focusWidgets) {}
#if defined(Q_TEMPLATEDLL)
    QWidgetList   focusWidgets;
    QWidgetListIt it;
#else
    QList<QWidget> focusWidgets;
    QListIterator<QWidget> it;
#endif
};


#endif // QFOCUSDATA_H
