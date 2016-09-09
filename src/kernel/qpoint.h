/****************************************************************************
** $Id: qpoint.h,v 2.4.2.3 1998/11/02 16:09:37 hanord Exp $
**
** Definition of QPoint class
**
** Created : 931028
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

#ifndef QPOINT_H
#define QPOINT_H

#ifndef QT_H
#include "qwindowdefs.h"
#endif // QT_H

#if (defined(QPOINT_C) || defined(DEBUG)) && !defined(_OS_WIN32_)
#define QPOINT_DEBUG
#endif


class Q_EXPORT QPoint
{
public:
    QPoint()	{}
    QPoint( int xpos, int ypos );

    bool   isNull()	const;

    int	   x()		const;
    int	   y()		const;
    void   setX( int x );
    void   setY( int y );

    QCOORD &rx();
    QCOORD &ry();

    QPoint &operator+=( const QPoint &p );
    QPoint &operator-=( const QPoint &p );
    QPoint &operator*=( int c );
    QPoint &operator*=( double c );
    QPoint &operator/=( int c );
    QPoint &operator/=( double c );

    friend inline bool	 operator==( const QPoint &, const QPoint & );
    friend inline bool	 operator!=( const QPoint &, const QPoint & );
    friend inline QPoint operator+( const QPoint &, const QPoint & );
    friend inline QPoint operator-( const QPoint &, const QPoint & );
    friend inline QPoint operator*( const QPoint &, int );
    friend inline QPoint operator*( int, const QPoint & );
    friend inline QPoint operator*( const QPoint &, double );
    friend inline QPoint operator*( double, const QPoint & );
    friend inline QPoint operator-( const QPoint & );
#if defined(QPOINT_DEBUG)
    friend Q_EXPORT QPoint operator/( const QPoint &, int );
    friend Q_EXPORT QPoint operator/( const QPoint &, double );
#else
    friend inline QPoint operator/( const QPoint &, int );
    friend inline QPoint operator/( const QPoint &, double );
#endif

    static void warningDivByZero();

private:
#if defined(_OS_MAC_)
    QCOORD yp;
    QCOORD xp;
#else
    QCOORD xp;
    QCOORD yp;
#endif
};


/*****************************************************************************
  QPoint stream functions
 *****************************************************************************/

Q_EXPORT QDataStream &operator<<( QDataStream &, const QPoint & );
Q_EXPORT QDataStream &operator>>( QDataStream &, QPoint & );


/*****************************************************************************
  QPoint inline functions
 *****************************************************************************/

inline QPoint::QPoint( int xpos, int ypos )
{ xp=(QCOORD)xpos; yp=(QCOORD)ypos; }

inline bool QPoint::isNull() const
{ return xp == 0 && yp == 0; }

inline int QPoint::x() const
{ return xp; }

inline int QPoint::y() const
{ return yp; }

inline void QPoint::setX( int x )
{ xp = (QCOORD)x; }

inline void QPoint::setY( int y )
{ yp = (QCOORD)y; }

inline QCOORD &QPoint::rx()
{ return xp; }

inline QCOORD &QPoint::ry()
{ return yp; }

inline QPoint &QPoint::operator+=( const QPoint &p )
{ xp+=p.xp; yp+=p.yp; return *this; }

inline QPoint &QPoint::operator-=( const QPoint &p )
{ xp-=p.xp; yp-=p.yp; return *this; }

inline QPoint &QPoint::operator*=( int c )
{ xp*=(QCOORD)c; yp*=(QCOORD)c; return *this; }

inline QPoint &QPoint::operator*=( double c )
{ xp=(QCOORD)(xp*c); yp=(QCOORD)(yp*c); return *this; }

inline bool operator==( const QPoint &p1, const QPoint &p2 )
{ return p1.xp == p2.xp && p1.yp == p2.yp; }

inline bool operator!=( const QPoint &p1, const QPoint &p2 )
{ return p1.xp != p2.xp || p1.yp != p2.yp; }

inline QPoint operator+( const QPoint &p1, const QPoint &p2 )
{ return QPoint(p1.xp+p2.xp, p1.yp+p2.yp); }

inline QPoint operator-( const QPoint &p1, const QPoint &p2 )
{ return QPoint(p1.xp-p2.xp, p1.yp-p2.yp); }

inline QPoint operator*( const QPoint &p, int c )
{ return QPoint(p.xp*c, p.yp*c); }

inline QPoint operator*( int c, const QPoint &p )
{ return QPoint(p.xp*c, p.yp*c); }

inline QPoint operator*( const QPoint &p, double c )
{ return QPoint((QCOORD)(p.xp*c), (QCOORD)(p.yp*c)); }

inline QPoint operator*( double c, const QPoint &p )
{ return QPoint((QCOORD)(p.xp*c), (QCOORD)(p.yp*c)); }

inline QPoint operator-( const QPoint &p )
{ return QPoint(-p.xp, -p.yp); }

//
// The QPoint functions below are inline if DEBUG is not defined.
// The debug implementation in qpoint.cpp checks c and gives a warning
// before dividing by zero.
//

#if !defined(QPOINT_DEBUG)

inline QPoint &QPoint::operator/=( int c )
{
#if defined(CHECK_MATH)
    if ( c == 0 )
	warningDivByZero();
#endif
    xp/=(QCOORD)c;
    yp/=(QCOORD)c;
    return *this;
}

inline QPoint &QPoint::operator/=( double c )
{
#if defined(CHECK_MATH)
    if ( c == 0.0 )
	warningDivByZero();
#endif
    xp=(QCOORD)(xp/c);
    yp=(QCOORD)(yp/c);
    return *this;
}

inline QPoint operator/( const QPoint &p, int c )
{
#if defined(CHECK_MATH)
    if ( c == 0 )
	QPoint::warningDivByZero();
#endif
    return QPoint(p.xp/c, p.yp/c);
}

inline QPoint operator/( const QPoint &p, double c )
{
#if defined(CHECK_MATH)
    if ( c == 0.0 )
	QPoint::warningDivByZero();
#endif
    return QPoint((QCOORD)(p.xp/c), (QCOORD)(p.yp/c));
}

#endif // no-debug inline functions


#endif // QPOINT_H
