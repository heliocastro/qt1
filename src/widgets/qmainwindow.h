/****************************************************************************
** $Id: qmainwindow.h,v 2.17.2.1 1998/08/19 16:02:40 agulbra Exp $
**
** Definition of QMainWindow class
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

#ifndef QMAINWINDOW_H
#define QMAINWINDOW_H

#ifndef QT_H
#include "qwidget.h"
#endif // QT_H

class QMenuBar;
class QToolBar;
class QStatusBar;
class QToolTipGroup;

class QMainWindowPrivate;


class Q_EXPORT QMainWindow: public QWidget
{
    Q_OBJECT
public:
    QMainWindow( QWidget * parent = 0, const char * name = 0, WFlags f = 0 );
    ~QMainWindow();

    QMenuBar * menuBar() const;
    QStatusBar * statusBar() const;
    QToolTipGroup * toolTipGroup() const;

    virtual void setCentralWidget( QWidget * );
    QWidget * centralWidget() const;

    enum ToolBarDock { Unmanaged, TornOff, Top, Bottom, Right, Left };

    void setDockEnabled( ToolBarDock dock, bool enable );
    bool isDockEnabled( ToolBarDock dock ) const;

    void addToolBar( QToolBar *, const char * label,
		     ToolBarDock = Top, bool newLine = FALSE );
    void removeToolBar( QToolBar * );

    void show();

    bool rightJustification() const;
    bool usesBigPixmaps() const;

    bool eventFilter( QObject*, QEvent* );

public slots:
    void setRightJustification( bool );
    void setUsesBigPixmaps( bool );

signals:
    void pixmapSizeChanged( bool );

protected slots:
    void setUpLayout();

protected:
    void paintEvent( QPaintEvent * );
    bool event( QEvent * );

private:
    QMainWindowPrivate * d;
    void triggerLayout();
    void moveToolBar( QToolBar *, QMouseEvent * );

    virtual void setMenuBar( QMenuBar * );
    virtual void setStatusBar( QStatusBar * );
    virtual void setToolTipGroup( QToolTipGroup * );
};


#endif
