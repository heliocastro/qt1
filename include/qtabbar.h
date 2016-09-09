/****************************************************************************
** $Id: qtabbar.h,v 2.12.2.2 1998/09/03 20:14:48 hanord Exp $
**
** Definition of QTabBar class
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

#ifndef QTABBAR_H
#define QTABBAR_H

#ifndef QT_H
#include "qwidget.h"
#include "qpainter.h"
#include "qlist.h"
#endif // QT_H

struct QTabPrivate;


struct Q_EXPORT QTab
{
    QTab(): enabled( TRUE ), id( 0 ) {}
    virtual ~QTab();

    QString label;
    // the bounding rectangle of this - may overlap with others
    QRect r;
    bool enabled;
    int id;
};


class Q_EXPORT QTabBar: public QWidget
{
    Q_OBJECT

public:
    QTabBar( QWidget * parent = 0, const char * name = 0 );
   ~QTabBar();

    enum Shape { RoundedAbove, RoundedBelow,
		 TriangularAbove, TriangularBelow };

    Shape shape() const;
    void setShape( Shape );

    void show();

    virtual int addTab( QTab * );

    void setTabEnabled( int, bool );
    bool isTabEnabled( int ) const;

    QSize sizeHint() const;

    int currentTab() const;
    int keyboardFocusTab() const;

    QTab * tab( int );

public slots:
    void setCurrentTab( int );
    void setCurrentTab( QTab * );

signals:
    void  selected( int );

protected:
    virtual void paint( QPainter *, QTab *, bool ) const;
    virtual QTab * selectTab( const QPoint & p ) const;

    void paintEvent( QPaintEvent * );
    void mousePressEvent ( QMouseEvent * );
    void mouseReleaseEvent ( QMouseEvent * );
    void keyPressEvent( QKeyEvent * );

    QListT<QTab> * tabList();

private:
    QListT<QTab> * l;
    QTabPrivate * d;

    void paintLabel( QPainter*, const QRect&, QTab*, bool ) const;
};


#endif // QTABBAR_H
