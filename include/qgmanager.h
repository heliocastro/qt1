/****************************************************************************
** $Id: qgmanager.h,v 2.14.2.2 1998/08/21 19:13:22 hanord Exp $
**
** Definition of QGManager class (workhorse for QLayout classes)
**
** Created : 960406
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

#ifndef QGMANAGER_H
#define QGMANAGER_H

#ifndef QT_H
#include "qintdict.h"
#include "qwidget.h"
#endif // QT_H


class QChain;
struct QGManagerData;

class Q_EXPORT QGManager : public QObject
{
    Q_OBJECT
public:
    QGManager( QWidget *parent, const char *name=0 );
    ~QGManager();

    void setBorder( int b ) { border = b; }

    enum Direction { LeftToRight, RightToLeft, Down, Up };
    enum { unlimited = QCOORD_MAX };

    QChain *newSerChain( Direction );
    //    QChain *newSerChain( Direction, int );
    QChain *newParChain( Direction );

    bool add( QChain *destination, QChain *source, int stretch = 0 );
    bool addWidget( QChain *, QWidget *, int stretch = 0 );
    bool addSpacing( QChain *, int minSize, int stretch = 0, int maxSize = unlimited );

    bool addBranch( QChain *destination, QChain *branch, int fromIndex,
		    int toIndex );
    void setStretch( QChain*, int );
    bool activate();

    void freeze( int w = 0, int h = 0 );
    void unFreeze();

    QChain *xChain() {	return xC; }
    QChain *yChain() {	return yC; }

    void  setMenuBar( QWidget *w ) { menuBar = w; }

    QWidget *mainWidget() { return main; }

    void remove( QWidget *w );

    void setName( QChain *, const char * );
        
protected:
    bool  eventFilter( QObject *, QEvent * );

private:
    int border;

    void      resizeHandle( QWidget *, const QSize & );
    void      resizeAll();

    QChain *xC;
    QChain *yC;
    QWidget *main;
    QWidget *menuBar;
    int	    menuBarHeight;
    QGManagerData *extraData;
    bool frozen;

private:	// Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QGManager( const QGManager & );
    QGManager &operator=( const QGManager & );
#endif
};


#endif // QGMANAGER_H
