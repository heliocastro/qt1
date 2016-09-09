/****************************************************************************
** $Id: qpen.h,v 2.4.2.2 1998/08/25 09:20:52 hanord Exp $
**
** Definition of QPen class
**
** Created : 940112
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

#ifndef QPEN_H
#define QPEN_H

#ifndef QT_H
#include "qcolor.h"
#include "qshared.h"
#endif // QT_H


enum PenStyle { NoPen, SolidLine, DashLine,	// pen style
		DotLine, DashDotLine, DashDotDotLine };


class Q_EXPORT QPen
{
friend class QPainter;
public:
    QPen();
    QPen( PenStyle );
    QPen( const QColor &color, uint width=0, PenStyle style=SolidLine );
    QPen( const QPen & );
   ~QPen();
    QPen &operator=( const QPen & );

    PenStyle	style() const		{ return data->style; }
    void	setStyle( PenStyle );
    uint	width() const		{ return data->width; }
    void	setWidth( uint );
    const QColor &color() const		{ return data->color; }
    void	setColor( const QColor & );

    bool	operator==( const QPen &p ) const;
    bool	operator!=( const QPen &p ) const
					{ return !(operator==(p)); }

private:
    QPen	copy()	const;
    void	detach();
    void	init( const QColor &, uint, PenStyle );
    struct QPenData : public QShared {		// pen data
	PenStyle  style;
	uint	  width;
	QColor	  color;
    } *data;
};


/*****************************************************************************
  QPen stream functions
 *****************************************************************************/

Q_EXPORT QDataStream &operator<<( QDataStream &, const QPen & );
Q_EXPORT QDataStream &operator>>( QDataStream &, QPen & );


#endif // QPEN_H
