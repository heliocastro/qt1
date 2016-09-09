/****************************************************************************
** $Id: qpointarray.h,v 2.8.2.3 1998/12/08 13:35:07 warwick Exp $
**
** Definition of QPointArray class
**
** Created : 940213
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

#ifndef QPOINTARRAY_H
#define QPOINTARRAY_H

#ifndef QT_H
#include "qarray.h"
#include "qpoint.h"
#endif // QT_H


/*****************************************************************************
  QPointData struct; platform dependent element i QPointArray
 *****************************************************************************/

#if defined(_WS_WIN32_) || defined(_WS_PM_)
typedef long Qpnta_t;
#else
typedef short Qpnta_t;
#endif

struct QPointData {				// platform dependent point
    QPointData() {}
    QPointData( int xp, int yp )  { x=(Qpnta_t)xp; y=(Qpnta_t)yp; }
    QPointData( const QPoint &p ) { x=(Qpnta_t)p.x(); y=(Qpnta_t)p.y(); }
#if defined(_WS_MAC_)
    Qpnta_t y;
    Qpnta_t x;
#else
    Qpnta_t x;
    Qpnta_t y;
#endif
};


/*****************************************************************************
  QPointVal class; a context class for QPointArray::operator[]
 *****************************************************************************/

class QPointArray;

class Q_EXPORT QPointVal
{
public:
    QPointVal( QPointData *ptr ) : p(ptr) {}
    bool operator==( const QPointVal &point ) const;
    bool operator!=( const QPointVal &point ) const;
    QPointVal &operator=( const QPointVal &point );
    QPointVal &operator=( const QPoint &point );
    QPointVal &operator+=( const QPoint &point );
    QPointVal &operator-=( const QPoint &point );
	       operator QPoint() const	{ return QPoint(p->x,p->y); }
    int	       x() const		{ return (int)p->x; }
    int	       y() const		{ return (int)p->y; }
private:
    QPointData *p;
};


/*****************************************************************************
  QPointArray class
 *****************************************************************************/

Q_DECLARE(QArrayM,QPointData);

class Q_EXPORT QPointArray : public QArrayM(QPointData)
{
public:
    QPointArray() {}
    QPointArray( int size ) : QArrayM(QPointData)( size ) {}
    QPointArray( const QPointArray &a ) : QArrayM(QPointData)( a ) {}
    QPointArray( const QRect &r, bool closed=FALSE );
    QPointArray( int nPoints, const QCOORD *points );

    QPointArray	 &operator=( const QPointArray &a )
	{ return (QPointArray&)assign( a ); }

    bool    fill( const QPoint &p, int size = -1 );

    QPointArray copy() const
	{ QPointArray tmp; return *((QPointArray*)&tmp.duplicate(*this)); }

    void    translate( int dx, int dy );

    void    point( uint i, int *x, int *y ) const;
    QPoint  point( uint i ) const;
    void    setPoint( uint i, int x, int y );
    void    setPoint( uint i, const QPoint &p );
    bool    setPoints( int nPoints, const QCOORD *points );
    bool    setPoints( int nPoints, int firstx, int firsty, ... );
    bool    putPoints( int index, int nPoints, const QCOORD *points );
    bool    putPoints( int index, int nPoints, int firstx, int firsty, ... );

    QPoint  at( uint i ) const;
    QPointVal operator[]( int i )
		{ return QPointVal( data()+i ); }
    QPointVal operator[]( uint i )
		{ return QPointVal( data()+i ); }
    QPoint operator[]( int i ) const
		{ return (QPoint)QPointVal( data()+i ); }
    QPoint operator[]( uint i ) const
		{ return (QPoint)QPointVal( data()+i ); }

    QRect   boundingRect() const;

    void    makeArc( int x, int y, int w, int h, int a1, int a2 );
    void    makeEllipse( int x, int y, int w, int h );

    QPointArray quadBezier() const;
};


/*****************************************************************************
  QPointArray stream functions
 *****************************************************************************/

Q_EXPORT QDataStream &operator<<( QDataStream &, const QPointArray & );
Q_EXPORT QDataStream &operator>>( QDataStream &, QPointArray & );


/*****************************************************************************
  Misc. QPointArray functions
 *****************************************************************************/

inline void QPointArray::setPoint( uint i, const QPoint &p )
{
    setPoint( i, p.x(), p.y() );
}

inline bool QPointVal::operator==( const QPointVal &pointval ) const
{
    return p->x == pointval.p->x && p->y == pointval.p->y;
}

inline bool QPointVal::operator!=( const QPointVal &pointval ) const
{
    return p->x != pointval.p->x && p->y != pointval.p->y;
}

inline QPointVal &QPointVal::operator=( const QPointVal &pointval )
{
    p->x = pointval.p->x;
    p->y = pointval.p->y;
    return *this;
}

inline QPointVal &QPointVal::operator=( const QPoint &point )
{
    p->x = (Qpnta_t)point.x();
    p->y = (Qpnta_t)point.y();
    return *this;
}

inline QPointVal &QPointVal::operator+=( const QPoint &point )
{
    p->x += (Qpnta_t)point.x();
    p->y += (Qpnta_t)point.y();
    return *this;
}

inline QPointVal &QPointVal::operator-=( const QPoint &point )
{
    p->x -= (Qpnta_t)point.x();
    p->y -= (Qpnta_t)point.y();
    return *this;
}


#endif // QPOINTARRAY_H
