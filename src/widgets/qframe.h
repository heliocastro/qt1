/****************************************************************************
** $Id: qframe.h,v 2.7.2.2 1998/08/21 19:13:25 hanord Exp $
**
** Definition of QFrame widget class
**
** Created : 950201
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

#ifndef QFRAME_H
#define QFRAME_H

#ifndef QT_H
#include "qwidget.h"
#endif // QT_H


class Q_EXPORT QFrame : public QWidget			// frame class
{
    Q_OBJECT
public:
    QFrame( QWidget *parent=0, const char *name=0, WFlags f=0,
	    bool allowLines=TRUE );

    enum { NoFrame  = 0,			// no frame
	   Box	    = 0x0001,			// rectangular box
	   Panel    = 0x0002,			// rectangular panel
	   WinPanel = 0x0003,			// rectangular panel (Windows)
	   HLine    = 0x0004,			// horizontal line
	   VLine    = 0x0005,			// vertical line
	   MShape   = 0x000f,
	   Plain    = 0x0010,			// plain line
	   Raised   = 0x0020,			// raised shadow effect
	   Sunken   = 0x0030,			// sunken shadow effect
	   MShadow  = 0x00f0 };

    int		frameStyle()	const;
    int		frameShape()	const;
    int		frameShadow()	const;
    void	setFrameStyle( int );

    bool	lineShapesOk()	const;

    int		lineWidth()	const;
    void	setLineWidth( int );

    int		margin()	const;
    void	setMargin( int );

    int		midLineWidth()	const;
    void	setMidLineWidth( int );

    int		frameWidth()	const;
    QRect	frameRect()	const;
    QRect	contentsRect()	const;

    QSize	sizeHint() const;

protected:
    void	setFrameRect( const QRect & );
    void	paintEvent( QPaintEvent * );
    void	resizeEvent( QResizeEvent * );
    virtual void drawFrame( QPainter * );
    virtual void drawContents( QPainter * );
    virtual void frameChanged();

private:
    void	updateFrameWidth();
    QRect	frect;
    int		fstyle;
    short	lwidth;
    short	mwidth;
    short	fwidth;
    short	lineok;

private:	// Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QFrame( const QFrame & );
    QFrame &operator=( const QFrame & );
#endif
};


inline int QFrame::frameStyle() const
{ return fstyle; }

inline int QFrame::frameShape() const
{ return fstyle & MShape; }

inline int QFrame::frameShadow() const
{ return fstyle & MShadow; }

inline bool QFrame::lineShapesOk() const
{ return lineok; }				// ### Qt 2.0: bool

inline int QFrame::lineWidth() const
{ return lwidth; }

inline int QFrame::midLineWidth() const
{ return mwidth & 0x00ff; }

inline int QFrame::margin() const
{ return ((int)mwidth) >> 8; }

inline int QFrame::frameWidth() const
{ return fwidth; }


#endif // QFRAME_H
