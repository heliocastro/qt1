/**********************************************************************
** $Id: qgroupbox.h,v 2.5.2.2 1998/08/21 19:13:25 hanord Exp $
**
** Definition of QGroupBox widget class
**
** Created : 950203
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

#ifndef QGROUPBOX_H
#define QGROUPBOX_H

#ifndef QT_H
#include "qframe.h"
#endif // QT_H


class Q_EXPORT QGroupBox : public QFrame
{
    Q_OBJECT
public:
    QGroupBox( QWidget *parent=0, const char *name=0 );
    QGroupBox( const char *title, QWidget *parent=0, const char *name=0 );

    const char *title()		const	{ return str; }

    void	setTitle( const char * );

    int		alignment()	const	{ return align; }
    void	setAlignment( int );

protected:
    void	paintEvent( QPaintEvent * );

private:
    void	init();
    QString	str;
    int		align;

private:	// Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QGroupBox( const QGroupBox & );
    QGroupBox &operator=( const QGroupBox & );
#endif
};


#endif // QGROUPBOX_H
