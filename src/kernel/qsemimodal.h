/****************************************************************************
** $Id: qsemimodal.h,v 2.4.2.2 1998/08/21 19:13:23 hanord Exp $
**
** Definition of QSemiModal class
**
** Created : 970627
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

#ifndef QSEMIMODAL_H
#define QSEMIMODAL_H

#ifndef QT_H
#include "qwidget.h"
#endif // QT_H


class Q_EXPORT QSemiModal : public QWidget
{
    Q_OBJECT
public:
    QSemiModal( QWidget *parent=0, const char *name=0, bool modal=FALSE, WFlags f=0 );
   ~QSemiModal();

    void	show();

    void	move( int x, int y );
    void	move( const QPoint &p );
    void	resize( int w, int h );
    void	resize( const QSize & );
    void	setGeometry( int x, int y, int w, int h );
    void	setGeometry( const QRect & );

private:
    uint	did_move   : 1;
    uint	did_resize : 1;

private:	// Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QSemiModal( const QSemiModal & );
    QSemiModal &operator=( const QSemiModal & );
#endif
};


#endif // QSEMIMODAL_H
