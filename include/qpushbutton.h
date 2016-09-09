/****************************************************************************
** $Id: qpushbutton.h,v 2.9.2.2 1998/08/21 19:13:26 hanord Exp $
**
** Definition of QPushButton class
**
** Created : 940221
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

#ifndef QPUSHBUTTON_H
#define QPUSHBUTTON_H

#ifndef QT_H
#include "qbutton.h"
#endif // QT_H


class Q_EXPORT QPushButton : public QButton
{
friend class QDialog;
    Q_OBJECT
public:
    QPushButton( QWidget *parent=0, const char *name=0 );
    QPushButton( const char *text, QWidget *parent=0, const char *name=0 );

    void	setToggleButton( bool );

    bool	autoDefault()	const	{ return autoDefButton; }
    void	setAutoDefault( bool autoDef );
    bool	isDefault()	const	{ return defButton; }
    void	setDefault( bool def );

    void	setIsMenuButton( bool );
    bool	isMenuButton() const;
    
    QSize	sizeHint() const;

    void	move( int x, int y );
    void	move( const QPoint &p );
    void	resize( int w, int h );
    void	resize( const QSize & );
    void	setGeometry( int x, int y, int w, int h );
    void	setGeometry( const QRect & );

public slots:
    void	setOn( bool );
    void	toggle();

protected:
    void	drawButton( QPainter * );
    void	drawButtonLabel( QPainter * );
    void	focusInEvent( QFocusEvent * );

private:
    void	init();

    uint	autoDefButton	: 1;
    uint	defButton	: 1;
    uint	lastDown	: 1;
    uint	lastDef		: 1;
    uint	lastEnabled	: 1;
    uint	hasMenuArrow	: 1;

private:	// Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QPushButton( const QPushButton & );
    QPushButton &operator=( const QPushButton & );
#endif
};


#endif // QPUSHBUTTON_H
