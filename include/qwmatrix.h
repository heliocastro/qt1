/****************************************************************************
** $Id: qwmatrix.h,v 2.4.2.2 1998/08/25 09:20:53 hanord Exp $
**
** Definition of QWMatrix class
**
** Created : 941020
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

#ifndef QWMATRIX_H
#define QWMATRIX_H

#ifndef QT_H
#include "qwindowdefs.h"
#include "qpointarray.h"
#include "qrect.h"
#endif // QT_H


class Q_EXPORT QWMatrix					// 2D transform matrix
{
public:
    QWMatrix();
    QWMatrix( float m11, float m12, float m21, float m22,
	      float dx,	 float dy );

    void	setMatrix( float m11, float m12, float m21, float m22,
			   float dx,  float dy );

    float	m11() const { return _m11; }
    float	m12() const { return _m12; }
    float	m21() const { return _m21; }
    float	m22() const { return _m22; }
    float	dx()  const { return _dx; }
    float	dy()  const { return _dy; }

    void	map( int x, int y, int *tx, int *ty )	      const;
    void	map( float x, float y, float *tx, float *ty ) const;
    QPoint	map( const QPoint & )	const;
    QRect	map( const QRect & )	const;
    QPointArray map( const QPointArray & ) const;

    void	reset();

    QWMatrix   &translate( float dx, float dy );
    QWMatrix   &scale( float sx, float sy );
    QWMatrix   &shear( float sh, float sv );
    QWMatrix   &rotate( float a );

    QWMatrix	invert( bool * = 0 ) const;

    bool	operator==( const QWMatrix & ) const;
    bool	operator!=( const QWMatrix & ) const;
    QWMatrix   &operator*=( const QWMatrix & );

private:
    QWMatrix   &bmul( const QWMatrix & );
    float	_m11, _m12;
    float	_m21, _m22;
    float	_dx,  _dy;
};


Q_EXPORT QWMatrix operator*( const QWMatrix &, const QWMatrix & );


/*****************************************************************************
  QWMatrix stream functions
 *****************************************************************************/

Q_EXPORT QDataStream &operator<<( QDataStream &, const QWMatrix & );
Q_EXPORT QDataStream &operator>>( QDataStream &, QWMatrix & );


#endif // QWMATRIX_H
