/****************************************************************************
** $Id: qwmatrix.cpp,v 2.6.2.1 1998/09/20 19:05:02 aavit Exp $
**
** Implementation of QWMatrix class
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

#include "qwmatrix.h"
#include "qdatastream.h"
#if defined(_WS_X11_)
double qsincos( double, bool calcCos );		// defined in qpainter_x11.cpp
#else
#include <math.h>
#endif

/*!
  \class QWMatrix qwmatrix.h
  \brief The QWMatrix class specifies 2D transformations of a
  coordinate system.

  \ingroup drawing

  The standard coordinate system of a \link QPaintDevice paint
  device\endlink has the origin located at the top left position. X
  values increase to the right, and Y values increase downwards.

  This coordinate system is default for the QPainter, which renders
  graphics in a paint device. A user-defined coordinate system can be
  specified by setting a QWMatrix for the painter.

  Example:
  \code
    MyWidget::paintEvent( QPaintEvent * )
    {
      QPainter p;			// our painter
      QWMatrix m;			// our transformation matrix
      m.rotate( 22.5 );			// rotated coordinate system
      p.begin( this );			// start painting
      p.setWorldMatrix( m );		// use rotated coordinate system
      p.drawText( 30,20, "detator" );	// draw rotated text at 30,20
      p.end();				// painting done
    }
  \endcode

  A matrix specifies how to translate, scale, shear or rotate the
  graphics, and the actual transformation is performed by the drawing
  routines in QPainter and by QPixmap::xForm().

  The QWMatrix class contains a 3*3 matrix of the form:
  <pre>
    m11	 m12  0
    m21	 m22  0
    dx	 dy   1
  </pre>

  A matrix transforms a point in the plane to another point:
  \code
    x' = m11*x + m21*y + dx
    y' = m22*y + m12*x + dy
  \endcode

  The point \e (x,y) is the original point, and \e (x',y') is the
  transformed point.  \e (x',y') can be transformed back to \e (x,y)
  by performing the same operation on the \link QWMatrix::invert()
  inverted matrix\endlink.

  The elements \e dx and \e dy specify horisontal and vertical
  translation.	The elements \e m11 and \e m22 specify horisontal and
  vertical scaling.  The elements \e m12 and \e m21 specify horisontal
  and vertical shearing.

  The identity matrix has \e m11 and \e m22 set to 1, all others set
  to 0.	 This matrix maps a point to itself.

  Translation is the simplest transformation. Setting \e dx and \e dy
  will move the coordinate system \e dx units along the X axis and \e
  dy units along the Y axis.

  Scaling can be done by setting \e m11 and \e m22.  For example,
  setting \e m11 to 2 and \e m22 to 1.5 will double the height and
  increase the width by 50%.

  Shearing is controlled by \e m12 and \e m21. Setting these elements
  to values different from zero will twist the coordinate system.

  Rotation is achieved by carefully setting both the shearing factors
  and the scaling factors.  The QWMatrix has a function that sets
  \link rotate() rotation \endlink directly.

  QWMatrix lets you combine transformations like this:
  \code
    QWMatrix m;					// identity matrix
    m.translate(10, -20);			// first translate (10,-20)
    m.rotate(25);				// then rotate 25 degrees
    m.scale(1.2, 0.7);				// finally scale it
  \endcode

  The same example, but using basic matrix operations:
  \code
    float a    = pi/180 * 25;			// convert 25 to radians
    float sina = sin(a);
    float cosa = cos(a);
    QWMatrix m1(0, 0, 0, 0, 10, -20);		// translation matrix
    QWMatrix m2( cosa, sina,			// rotation matrix
		-sina, cosa, 0, 0 );
    QWMatrix m3(1.2, 0, 0, 0.7, 0, 0);		// scaling matrix
    QWMatrix m;
    m = m3 * m2 * m1;				// combine all transformations
  \endcode

  \link QPainter QPainter\endlink has functions that \link
  QPainter::translate() translate\endlink, \link QPainter::scale()
  scale\endlink, \link QPainter::shear() shear\endlink and \link
  QPainter::rotate() rotate\endlink the coordinate system without using a
  QWMatrix.  These functions are very convenient, however, if you want to
  perform more than a single transform operation, it is more efficient to
  build a QWMatrix and call QPainter::setWorldMatrix().

  \sa QPainter::setWorldMatrix(), QPixmap::xForm()
*/


/*****************************************************************************
  QWMatrix member functions
 *****************************************************************************/

/*!
  Constructs an identity matrix.  All elements are set to zero,
  except \e m11 and \e m22 (scaling) which are set to 1.
*/

QWMatrix::QWMatrix()
{
    _m11 = _m22 = (float)1.0;
    _m12 = _m21 = _dx = _dy = (float)0.0;
}

/*!
  Constructs a matrix with the specified elements.
*/

QWMatrix::QWMatrix( float m11, float m12, float m21, float m22,
		    float dx, float dy )
{
    _m11 = m11;	 _m12 = m12;
    _m21 = m21;	 _m22 = m22;
    _dx	 = dx;	 _dy  = dy;
}


/*!
  Sets the matrix elements to the specified values.
*/

void QWMatrix::setMatrix( float m11, float m12, float m21, float m22,
			  float dx, float dy )
{
    _m11 = m11;	 _m12 = m12;
    _m21 = m21;	 _m22 = m22;
    _dx	 = dx;	 _dy  = dy;
}


/*!
  \fn float QWMatrix::m11() const
  Returns the X scaling factor.
*/

/*!
  \fn float QWMatrix::m12() const
  Returns the vertical shearing factor.
*/

/*!
  \fn float QWMatrix::m21() const
  Returns the horizontal shearing factor.
*/

/*!
  \fn float QWMatrix::m22() const
  Returns the Y scaling factor.
*/

/*!
  \fn float QWMatrix::dx() const
  Returns the horizontal translation.
*/

/*!
  \fn float QWMatrix::dy() const
  Returns the vertical translation.
*/


/*!
  Transforms \e (x,y) to \e (*tx,*ty), using the formulae:

  \code
    *tx = m11*x + m21*y + dx
    *ty = m22*y + m12*x + dy
  \endcode
*/

void QWMatrix::map( float x, float y, float *tx, float *ty ) const
{
    *tx = _m11*x + _m21*y + _dx;
    *ty = _m12*x + _m22*y + _dy;
}

/*!
  Transforms \e (x,y) to \e (*tx,*ty), using the formulae:

  \code
    *tx = m11*x + m21*y + dx  --  (rounded to the nearest integer)
    *ty = m22*y + m12*x + dy  --  (rounded to the nearest integer)
  \endcode
*/

void QWMatrix::map( int x, int y, int *tx, int *ty ) const
{
    double fx = (double)x;
    double fy = (double)y;
    *tx = qRound(_m11*fx + _m21*fy + _dx);
    *ty = qRound(_m12*fx + _m22*fy + _dy);
}

/*!
  Returns the transformed \e p.
*/

QPoint QWMatrix::map( const QPoint &p ) const
{
    double fx = (double)p.x();
    double fy = (double)p.y();
    return QPoint( qRound(_m11*fx + _m21*fy + _dx),
		   qRound(_m12*fx + _m22*fy + _dy) );
}

/*!
  Returns the tranformed rectangle \e r.

  If rotation or shearing has been specified, then the bounding rectangle
  will be returned.
*/

QRect QWMatrix::map( const QRect &r ) const
{
    QRect result;
    if ( _m12 == 0.0F && _m21 == 0.0F ) {
	result = QRect( map(r.topLeft()), map(r.bottomRight()) );
    } else {
	QPointArray a( r );
	a = map( a );
	result = a.boundingRect();
    }
    return result;
}

/*!
  Returns the point array \e a transformed by calling map for each point.
*/

QPointArray QWMatrix::map( const QPointArray &a ) const
{
    QPointArray result = a.copy();
    int x, y;
    for ( int i=0; i<(int)result.size(); i++ ) {
	result.point( i, &x, &y );
	map( x, y, &x, &y );
	result.setPoint( i, x, y );
    }
    return result;
}


/*!
  Resets the matrix to an identity matrix.

  All elements are set to zero, except \e m11 and \e m22 (scaling)
  that are set to 1.
*/

void QWMatrix::reset()
{
    _m11 = _m22 = (float)1.0;
    _m12 = _m21 = _dx = _dy = (float)0.0;
}


/*!
  Moves the coordinate system \e dx along the X-axis and \e dy
  along the Y-axis.

  Returns a reference to the matrix.

  \sa scale(), shear(), rotate()
*/

QWMatrix &QWMatrix::translate( float dx, float dy )
{
    QWMatrix result( 1.0F, 0.0F, 0.0F, 1.0F, dx, dy );
    return bmul( result );
}

/*!
  Scales the coordinate system unit by \e sx horizontally and \e sy
  vertically.

  Returns a reference to the matrix.

  \sa translate(), shear(), rotate()
*/

QWMatrix &QWMatrix::scale( float sx, float sy )
{
    QWMatrix result( sx, 0.0F, 0.0F, sy, 0.0F, 0.0F );
    return bmul( result );
}

/*!
  Shears the coordinate system	by \e sh horizontally and \e sv vertically.

  Returns a reference to the matrix.

  \sa translate(), scale(), rotate()
*/

QWMatrix &QWMatrix::shear( float sh, float sv )
{
    QWMatrix result( 1.0F, sv, sh, 1.0F, 0.0F, 0.0F );
    return bmul( result );
}

const float deg2rad = .017453292519943295769F;	// pi/180

/*!
  Rotates the coordinate system \e a degrees counterclockwise.

  Returns a reference to the matrix.

  \sa translate(), scale(), shear()
*/

QWMatrix &QWMatrix::rotate( float a )
{
    double b = deg2rad*a;			// convert to radians
#if defined(_WS_X11_)
    float sina = qsincos(b,FALSE);		// fast and convenient
    float cosa = qsincos(b,TRUE);
#else
    float sina = (float)sin(b);
    float cosa = (float)cos(b);
#endif
    QWMatrix result( cosa, sina, -sina, cosa, 0.0F, 0.0F );
    return bmul( result );
}


/*!
  Returns the inverted matrix.

  If the matrix is singular (not invertible), then the identity matrix is
  returned.

  If \e *invertible is not null, then the value of \e *invertible will
  be set to TRUE or FALSE to tell if the matrix is invertible or not.
*/

QWMatrix QWMatrix::invert( bool *invertible ) const
{
    double det = _m11*_m22 - _m12*_m21;
    if ( QABS(det) < 0.000001 ) {		// very close to zero
	if ( invertible )			//   (whatever that means...)
	    *invertible = FALSE;		// singular matrix
	QWMatrix defaultMatrix;
	return defaultMatrix;
    }
    else {					// invertible matrix
	if ( invertible )
	    *invertible = TRUE;
	double dinv = 1.0/det;
	QWMatrix imatrix( (float)(_m22*dinv),	(float)(-_m12*dinv),
			  (float)(-_m21*dinv), (float)( _m11*dinv),
			  (float)((_m21*_dy - _m22*_dx)*dinv),
			  (float)((_m12*_dx - _m11*_dy)*dinv) );
	return imatrix;
    }
}


/*!
  Returns TRUE if this matrix is equal to \e m.
*/

bool QWMatrix::operator==( const QWMatrix &m ) const
{
    return _m11 == m._m11 &&
	   _m12 == m._m12 &&
	   _m21 == m._m21 &&
	   _m22 == m._m22 &&
	   _dx == m._dx &&
	   _dy == m._dy;
}

/*!
  Returns TRUE if this matrix is not equal to \e m.
*/

bool QWMatrix::operator!=( const QWMatrix &m ) const
{
    return _m11 != m._m11 ||
	   _m12 != m._m12 ||
	   _m21 != m._m21 ||
	   _m22 != m._m22 ||
	   _dx != m._dx ||
	   _dy != m._dy;
}

/*!
  Returns the result of multiplying this matrix with \e m.
*/

QWMatrix &QWMatrix::operator*=( const QWMatrix &m )
{
    setMatrix( _m11*m._m11 + _m12*m._m21,  _m11*m._m12 + _m12*m._m22,
	       _m21*m._m11 + _m22*m._m21,  _m21*m._m12 + _m22*m._m22,
	       _dx*m._m11  + _dy*m._m21 + m._dx,
	       _dx*m._m12  + _dy*m._m22 + m._dy );
    return *this;
}

QWMatrix &QWMatrix::bmul( const QWMatrix &m )
{
    setMatrix( m._m11*_m11 + m._m12*_m21,  m._m11*_m12 + m._m12*_m22,
	       m._m21*_m11 + m._m22*_m21,  m._m21*_m12 + m._m22*_m22,
	       m._dx*_m11  + m._dy*_m21 + _dx,
	       m._dx*_m12  + m._dy*_m22 + _dy );
    return *this;
}

/*!
  \relates QWMatrix
  Returns the product \e m1 * \e m2.

  Remember that matrix multiplication is not commutative, thus
  a*b != b*a.
*/

QWMatrix operator*( const QWMatrix &m1, const QWMatrix &m2 )
{
    QWMatrix result = m1;
    result *= m2;
    return result;
}


/*****************************************************************************
  QWMatrix stream functions
 *****************************************************************************/

/*!
  \relates QWMatrix
  Writes a matrix to the stream and returns a reference to the stream.

  Serialization format: m11, m12, m21, m22, dx and dy are serialized as
  \c float in the listed order.
*/

QDataStream &operator<<( QDataStream &s, const QWMatrix &m )
{
    return s << m.m11() << m.m12() << m.m21() << m.m22()
	     << m.dx()	<< m.dy();
}

/*!
  \relates QWMatrix
  Reads a matrix from the stream and returns a reference to the stream.
*/

QDataStream &operator>>( QDataStream &s, QWMatrix &m )
{
    float m11, m12, m21, m22, dx, dy;
    s >> m11;  s >> m12;  s >> m21;  s >> m22;
    s >> dx;   s >> dy;
    m.setMatrix( m11, m12, m21, m22, dx, dy );
    return s;
}
