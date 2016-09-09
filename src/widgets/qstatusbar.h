/****************************************************************************
** $Id: qstatusbar.h,v 2.4.2.1 1998/08/19 16:02:43 agulbra Exp $
**
** Definition of QStatusBar class
**
** Created : 980316
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

#ifndef QSTATUSBAR_H
#define QSTATUSBAR_H

#ifndef QT_H
#include "qwidget.h"
#endif // QT_H


class QStatusBarPrivate;


class Q_EXPORT QStatusBar: public QWidget
{
    Q_OBJECT
public:
    QStatusBar( QWidget * parent = 0, const char * name = 0 );
    ~QStatusBar();

    void addWidget( QWidget *, int, bool = FALSE );
    void removeWidget( QWidget * );

public slots:
    void message( const char * );
    void message( const char *, int );
    void clear();

protected:
    void paintEvent( QPaintEvent * );

    void reformat();
    void hideOrShow();

private:
    QStatusBarPrivate * d;
};


#endif
