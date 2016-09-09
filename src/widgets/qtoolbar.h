/****************************************************************************
** $Id: qtoolbar.h,v 2.10.2.2 1998/08/19 16:02:44 agulbra Exp $
**
** Definition of QToolBar class
**
** Created : 980306
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

#ifndef QTOOLBAR_H
#define QTOOLBAR_H

#ifndef QT_H
#include "qwidget.h"
#include "qmainwindow.h"
#endif // QT_H

class QButton;
class QBoxLayout;
class QToolBarPrivate;


class Q_EXPORT QToolBar: public QWidget
{
    Q_OBJECT
public:
    QToolBar( const char * label,
	      QMainWindow *, QMainWindow::ToolBarDock = QMainWindow::Top,
	      bool newLine = FALSE, const char * name = 0 );
    QToolBar( const char * label, QMainWindow *, QWidget *,
	      bool newLine = FALSE, const char * name = 0, WFlags f = 0 );
    QToolBar( QMainWindow * parent = 0, const char * name = 0 );
    ~QToolBar();

    void addSeparator();

    enum Orientation { Horizontal, Vertical };
    virtual void setOrientation( Orientation );
    Orientation orientation() const { return o; }

    void show();

    QMainWindow * mainWindow();

    void setStretchableWidget( QWidget * );
    
    bool event( QEvent * e );

protected:
    void paintEvent( QPaintEvent * );

private:
    void setUpGM();

    QBoxLayout * b;
    QToolBarPrivate * d;
    Orientation o;
    QMainWindow * mw;
    QWidget * sw;
};


#endif
