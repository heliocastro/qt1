/****************************************************************************
** $Id: qwindow.h,v 2.4.2.2 1998/08/21 19:13:24 hanord Exp $
**
** Definition of QWindow class
**
** Created : 931112
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

#ifndef QWINDOW_H
#define QWINDOW_H

#ifndef QT_H
#include "qwidget.h"
#endif // QT_H


class Q_EXPORT QWindow : public QWidget			// window widget class
{
    Q_OBJECT
public:
    QWindow( QWidget *parent=0, const char *name=0, WFlags f=0 );
   ~QWindow();

private:	// Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QWindow( const QWindow & );
    QWindow &operator=( const QWindow & );
#endif
};


#endif // QWINDOW_H
