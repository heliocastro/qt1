/****************************************************************************
** $Id: qbrush.h,v 2.4.2.2 1998/08/25 09:20:51 hanord Exp $
**
** Definition of QBrush class
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

#ifndef QBRUSH_H
#define QBRUSH_H

#ifndef QT_H
#include "qcolor.h"
#include "qshared.h"
#endif // QT_H


enum BrushStyle					// brush style
      { NoBrush, SolidPattern,
	Dense1Pattern, Dense2Pattern, Dense3Pattern, Dense4Pattern,
	Dense5Pattern, Dense6Pattern, Dense7Pattern,
	HorPattern, VerPattern, CrossPattern,
	BDiagPattern, FDiagPattern, DiagCrossPattern, CustomPattern=24 };


class Q_EXPORT QBrush
{
friend class QPainter;
public:
    QBrush();
    QBrush( BrushStyle );
    QBrush( const QColor &, BrushStyle=SolidPattern );
    QBrush( const QColor &, const QPixmap & );
    QBrush( const QBrush & );
   ~QBrush();
    QBrush &operator=( const QBrush & );

    BrushStyle	style()	 const		{ return data->style; }
    void	setStyle( BrushStyle );
    const QColor &color()const		{ return data->color; }
    void	setColor( const QColor & );
    QPixmap    *pixmap() const		{ return data->pixmap; }
    void	setPixmap( const QPixmap & );

    bool	operator==( const QBrush &p ) const;
    bool	operator!=( const QBrush &b ) const
					{ return !(operator==(b)); }

private:
    QBrush	copy()	const;
    void	detach();
    void	init( const QColor &, BrushStyle );
    struct QBrushData : public QShared {	// brush data
	BrushStyle style;
	QColor	  color;
	QPixmap	 *pixmap;
    } *data;
};


/*****************************************************************************
  QBrush stream functions
 *****************************************************************************/

Q_EXPORT QDataStream &operator<<( QDataStream &, const QBrush & );
Q_EXPORT QDataStream &operator>>( QDataStream &, QBrush & );


#endif // QBRUSH_H
