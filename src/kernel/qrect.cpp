/****************************************************************************
** $Id: qrect.cpp,v 2.9 1998/07/03 00:09:40 hanord Exp $
**
** Implementation of QRect class
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

#define	 QRECT_C
#include "qrect.h"
#include "qdatastream.h"

/*!
  \class QRect qrect.h
  \brief The QRect class defines a rectangle in the plane.

  \ingroup drawing

  A rectangle is internally represented as an upper left corner and a
  bottom right corner, but it is normally expressed as an upper left
  corner and a size.

  The coordinate type is QCOORD (defined in qwindowdefs.h as \c short).
  The minimum value of QCOORD is QCOORD_MIN (-32768) and the maximum
  value is  QCOORD_MAX (32767).

  Note that the size (width and height) of a rectange might be
  different from what you are used to. If the top left corner and the
  bottom right corner are the same, then the height and the width of
  the rectangle will both be 1.

  Generally, <em>width = right - left + 1</em> and <em>height = bottom
  - top + 1.</em> We designed it this way to make it correspond to
  rectangular spaces used by drawing functions, where the width and
  height denote a number of pixels. For example, drawing a rectangle
  with width and height 1 draws a single pixel.

  The default coordinate system has origin (0,0) in the top left
  corner, the positive direction of the y axis is downwards and the
  positive x axis is from the left to the right.

  \sa QPoint, QSize
*/


/*****************************************************************************
  QRect member functions
 *****************************************************************************/

/*!
  \fn QRect::QRect()
  Constructs an empty rectangle.
*/

/*!
  Constructs a rectangle with \e topLeft as the top left corner and
  \e bottomRight as the bottom right corner.
*/

QRect::QRect( const QPoint &topLeft, const QPoint &bottomRight )
{
    x1 = (QCOORD)topLeft.x();
    y1 = (QCOORD)topLeft.y();
    x2 = (QCOORD)bottomRight.x();
    y2 = (QCOORD)bottomRight.y();
}

/*!
  Constructs a rectangle with \e topLeft as the top left corner and
  \e size as the rectangle size.
*/

QRect::QRect( const QPoint &topLeft, const QSize &size )
{
    x1 = (QCOORD)topLeft.x();
    y1 = (QCOORD)topLeft.y();
    x2 = (QCOORD)(x1+size.width()-1);
    y2 = (QCOORD)(y1+size.height()-1);
}

/*!
  \fn QRect::QRect( int left, int top, int width, int height )

  Constructs a rectangle with the \e top, \e left corner and \e
  width and \e height.

  Example (creates three identical rectangles):
  \code
    QRect r1( QPoint(100,200), QPoint(110,215) );
    QRect r2( QPoint(100,200), QSize(11,16) );
    QRect r3( 100, 200, 11, 16 );
  \endcode
*/


/*!
  \fn bool QRect::isNull() const

  Returns TRUE if the rectangle is a null rectangle, otherwise FALSE.

  A null rectangle has both the width and the height set to 0, that is
  right() == left() - 1 and bottom() == top() - 1.

  Remember that if right() == left() and bottom() == top(), then the
  rectangle has width 1 and height 1.

  A null rectangle is also empty.

  A null rectangle is not valid.

  \sa isEmpty(), isValid()
*/

/*!
  \fn bool QRect::isEmpty() const

  Returns TRUE if the rectangle is empty, otherwise FALSE.

  An empty rectangle has a left() \> right() or top() \> bottom().

  An empty rectangle is not valid. \code isEmpty() == !isValid()\endcode

  \sa isNull(), isValid()
*/

/*!
  \fn bool QRect::isValid() const

  Returns TRUE if the rectangle is valid, or FALSE if it is invalid (empty).

  A valid rectangle has a left() \<= right() and top() \<= bottom().

  \code isValid() == !isEmpty()\endcode

  \sa isNull(), isEmpty(), normalize()
*/


/*!
  Returns a normalized rectangle, i.e. one that has a non-negative width
  and height.

  It swaps left and right if left() \> right(), and swaps top and bottom if
  top() \> bottom().

  \sa isValid()
*/

QRect QRect::normalize() const
{
    QRect r;
    if ( x2 < x1 ) {				// swap bad x values
	r.x1 = x2;
	r.x2 = x1;
    } else {
	r.x1 = x1;
	r.x2 = x2;
    }
    if ( y2 < y1 ) {				// swap bad y values
	r.y1 = y2;
	r.y2 = y1;
    } else {
	r.y1 = y1;
	r.y2 = y2;
    }
    return r;
}


/*!
  \fn int QRect::left() const

  Returns the left coordinate of the rectangle. Identical to x().

  \sa x(), top(), right(), setLeft(), topLeft(), bottomLeft()
*/

/*!
  \fn int QRect::top() const

  Returns the top coordinate of the rectangle. Identical to y().

  \sa y(), left(), bottom(), setTop(), topLeft(), topRight()
*/

/*!
  \fn int QRect::right() const

  Returns the right coordinate of the rectangle.

  \sa left(), setRight(), topRight(), bottomRight()
*/

/*!
  \fn int QRect::bottom() const

  Returns the bottom coordinate of the rectangle.

  \sa top(), setBottom(), bottomLeft(), bottomRight()
*/

/*!
  \fn int QRect::x() const

  Returns the left coordinate of the rectangle.	 Identical to left().

  \sa left(), y(), setX()
*/

/*!
  \fn int QRect::y() const

  Returns the top coordinate of the rectangle.	Identical to top().

  \sa top(), x(), setY()
*/

/*!
  \fn void QRect::setLeft( int pos )

  Sets the left edge of the rectangle.	May change the width, but
  will never change the right edge of the rectangle.

  Identical to setX().

  \sa left(), setTop(), setWidth()
*/

/*!
  \fn void QRect::setTop( int pos )

  Sets the top edge of the rectangle.  May change the height, but
  will never change the bottom edge of the rectangle.

  Identical to setY().

  \sa top(), setBottom(), setHeight()
*/

/*!
  \fn void QRect::setRight( int pos )

  Sets the right edge of the rectangle.	 May change the width, but
  will never change the left edge of the rectangle.

  \sa right(), setLeft(), setWidth()
*/

/*!
  \fn void QRect::setBottom( int pos )

  Sets the bottom edge of the rectangle.  May change the height,
  but will never change the top edge of the rectangle.

  \sa bottom(), setTop(), setHeight()
*/

/*!
  \fn void QRect::setX( int x )

  Sets the x position of the rectangle (its left end).	May change
  the width, but will never change the right edge of the rectangle.

  Identical to setLeft().

  \sa x(), setY()
*/

/*!
  \fn void QRect::setY( int y )

  Sets the y position of the rectangle (its top).  May change the
  height, but will never change the bottom edge of the rectangle.

  Identical to setTop().

  \sa y(), setX()
*/

/*!
  \fn QPoint QRect::topLeft() const
  Returns the top left position of the rectangle.
  \sa moveTopLeft(), topRight(), bottomLeft(), bottomRight(), left(), top()
*/

/*!
  \fn QPoint QRect::bottomRight() const
  Returns the bottom right position of the rectangle.
  \sa moveBottomRight(), bottomLeft(), topLeft(), topRight(), bottom(), right()
*/

/*!
  \fn QPoint QRect::topRight() const
  Returns the top right position of the rectangle.
  \sa moveTopRight(), topLeft(), bottomLeft(), bottomRight(), top(), right()
*/

/*!
  \fn QPoint QRect::bottomLeft() const
  Returns the bottom left position of the rectangle.
  \sa moveBottomLeft(), bottomRight(), topLeft(), topRight(), bottom(), left()
*/

/*!
  \fn QPoint QRect::center() const
  Returns the center point of the rectangle.
  \sa moveCenter(), topLeft(), topRight(), bottomLeft(), bottomRight()
*/


/*!
  Extracts the rectangle parameters as the position and the size.
  \sa setRect(), coords()
*/

void QRect::rect( int *x, int *y, int *w, int *h ) const
{
    *x = x1;
    *y = y1;
    *w = x2-x1+1;
    *h = y2-y1+1;
}

/*!
  Extracts the rectangle parameters as the top left point and the
  bottom right point.
  \sa setCoords(), rect()
*/

void QRect::coords( int *xp1, int *yp1, int *xp2, int *yp2 ) const
{
    *xp1 = x1;
    *yp1 = y1;
    *xp2 = x2;
    *yp2 = y2;
}

/*!
  Sets the top left position of the rectangle to \e p, leaving the
  size unchanged.
  \sa topLeft(), moveTopRight(), moveBottomLeft(), moveBottomRight(),
  setTop(), setLeft()
*/

void QRect::moveTopLeft( const QPoint &p )
{
    x2 += (QCOORD)(p.x() - x1);
    y2 += (QCOORD)(p.y() - y1);
    x1 = (QCOORD)p.x();
    y1 = (QCOORD)p.y();
}

/*!
  Sets the bottom right position of the rectangle to \e p, leaving the size
  unchanged.
  \sa bottomRight(), moveBottomLeft(), moveTopLeft(), moveTopRight(),
  setBottom(), setRight()
*/

void QRect::moveBottomRight( const QPoint &p )
{
    x1 += (QCOORD)(p.x() - x2);
    y1 += (QCOORD)(p.y() - y2);
    x2 = (QCOORD)p.x();
    y2 = (QCOORD)p.y();
}

/*!
  Sets the top right position of the rectangle to \e p, leaving the
  size unchanged.
  \sa topRight(), moveTopLeft(), moveBottomLeft(), moveBottomRight(),
  setTop(), setRight()
*/

void QRect::moveTopRight( const QPoint &p )
{
    x1 += (QCOORD)(p.x() - x2);
    y2 += (QCOORD)(p.y() - y1);
    x2 = (QCOORD)p.x();
    y1 = (QCOORD)p.y();
}

/*!
  Sets the bottom left position of the rectangle to \e p, leaving
  the size unchanged.
  \sa bottomLeft(), moveBottomRight(), moveTopLeft(), moveTopRight(),
  setBottom(), setLeft()
*/

void QRect::moveBottomLeft( const QPoint &p )
{
    x2 += (QCOORD)(p.x() - x1);
    y1 += (QCOORD)(p.y() - y2);
    x1 = (QCOORD)p.x();
    y2 = (QCOORD)p.y();
}


/*!
  Sets the center point of the rectangle to \e p, leaving the size
  unchanged.
  \sa center(), moveTopLeft(), moveTopRight(), moveBottomLeft(), moveBottomRight()
*/

void QRect::moveCenter( const QPoint &p )
{
    QCOORD w = x2 - x1;
    QCOORD h = y2 - y1;
    x1 = (QCOORD)(p.x() - w/2);
    y1 = (QCOORD)(p.y() - h/2);
    x2 = x1 + w;
    y2 = y1 + h;
}


/*!
  Moves the rectangle \e dx along the X axis and \e dy along the Y
  axis, relative to the current position. (Positive values moves the
  rectangle rightwards and/or downwards.)
*/


void QRect::moveBy( int dx, int dy )
{
    x1 += (QCOORD)dx;
    y1 += (QCOORD)dy;
    x2 += (QCOORD)dx;
    y2 += (QCOORD)dy;
}

/*!
  Sets the coordinates of the rectangle's top left corner to \e
  (x,y), and its size to (w,h).
  \sa rect(), setCoords()
*/

void QRect::setRect( int x, int y, int w, int h )
{
    x1 = (QCOORD)x;
    y1 = (QCOORD)y;
    x2 = (QCOORD)(x+w-1);
    y2 = (QCOORD)(y+h-1);
}

/*!
  Sets the coordinates of the rectangle's top left corner to \e (xp1,yp1),
  and the coordinates of its bottom right corner to \e (xp2,yp2).
  \sa coords(), setRect()
*/

void QRect::setCoords( int xp1, int yp1, int xp2, int yp2 )
{
    x1 = (QCOORD)xp1;
    y1 = (QCOORD)yp1;
    x2 = (QCOORD)xp2;
    y2 = (QCOORD)yp2;
}


/*!
  \fn QSize QRect::size() const
  Returns the size of the rectangle.
  \sa width(), height()
*/

/*!
  \fn int QRect::width() const
  Returns the width of the rectangle.  The width includes both the
  left and right edges, ie. width = right - left + 1.
  \sa height(), size(), setHeight()
*/

/*!
  \fn int QRect::height() const
  Returns the height of the rectangle.	The height includes both the
  top and bottom edges, ie. height = bottom - top + 1.
  \sa width(), size(), setHeight()
*/

/*!
  Sets the width of the rectangle to \e w. The right edge is
  changed, but not the left edge.
  \sa width(), setLeft(), setRight(), setSize()
*/

void QRect::setWidth( int w )
{
    x2 = (QCOORD)(x1 + w - 1);
}

/*!
  Sets the height of the rectangle to \e h. The top edge is not
  moved, but the bottom edge may be moved.
  \sa height(), setTop(), setBottom(), setSize()
*/

void QRect::setHeight( int h )
{
    y2 = (QCOORD)(y1 + h - 1);
}

/*!
  Sets the size of the rectangle to \e s. The top left corner is not moved.
  \sa size(), setWidth(), setHeight()
*/

void QRect::setSize( const QSize &s )
{
    x2 = (QCOORD)(s.width() +x1-1);
    y2 = (QCOORD)(s.height()+y1-1);
}

/*!
  Returns TRUE if the point \e p is inside or on the edge of the
  rectangle.

  If \e proper is TRUE, this function returns TRUE only if \e p is
  inside (not on the edge).
*/

bool QRect::contains( const QPoint &p, bool proper ) const
{
    if ( proper )
	return p.x() > x1 && p.x() < x2 &&
	       p.y() > y1 && p.y() < y2;
    else
	return p.x() >= x1 && p.x() <= x2 &&
	       p.y() >= y1 && p.y() <= y2;
}

/*!
  Returns TRUE if the rectangle \e r is inside this rectangle.

  If \e proper is TRUE, this function returns TRUE only if \e r is
  entirely inside (not on the edge).

  \sa unite(), intersect(), intersects()
*/

bool QRect::contains( const QRect &r, bool proper ) const
{
    if ( proper )
	return r.x1 > x1 && r.x2 < x2 && r.y1 > y1 && r.y2 < y2;
    else
	return r.x1 >= x1 && r.x2 <= x2 && r.y1 >= y1 && r.y2 <= y2;
}

/*!
  Returns the union rectangle of this rectangle and \e r.  The union
  rectangle of a nonempty rectangle and an empty or invalid rectangle
  is defined to be the nonempty rectangle.

  \sa intersect(), intersects(), contains()
*/

QRect QRect::unite( const QRect &r ) const
{
    if ( isValid() ) {
	if ( r.isValid() ) {
	    QRect tmp;
	    tmp.setLeft(   QMIN( x1, r.x1 ) );
	    tmp.setRight(  QMAX( x2, r.x2 ) );
	    tmp.setTop(	   QMIN( y1, r.y1 ) );
	    tmp.setBottom( QMAX( y2, r.y2 ) );
	    return tmp;
	} else {
	    return *this;
	}
    } else {
	return r;
    }
}


/*!
  Returns the intersection rectangle of this rectangle and \e r.

  Returns an empty rectangle if there is no intersection.

  \sa isEmpty(), intersects(), unite(), contains()
*/

QRect QRect::intersect( const QRect &r ) const
{
    QRect tmp;
    tmp.x1 = QMAX( x1, r.x1 );
    tmp.x2 = QMIN( x2, r.x2 );
    tmp.y1 = QMAX( y1, r.y1 );
    tmp.y2 = QMIN( y2, r.y2 );
    return tmp;
}

/*!
  Returns TRUE if this rectangle intersects with \e r (there is at
  least one pixel which is within both rectangles).
  \sa intersect(), contains()
*/

bool QRect::intersects( const QRect &r ) const
{
    return ( QMAX( x1, r.x1 ) <= QMIN( x2, r.x2 ) &&
	     QMAX( y1, r.y1 ) <= QMIN( y2, r.y2 ) );
}


/*!
  \relates QRect
  Returns TRUE if \e r1 and \e r2 are equal, or FALSE if they are different.
*/

bool operator==( const QRect &r1, const QRect &r2 )
{
    return r1.x1==r2.x1 && r1.x2==r2.x2 && r1.y1==r2.y1 && r1.y2==r2.y2;
}

/*!
  \relates QRect
  Returns TRUE if \e r1 and \e r2 are different, or FALSE if they are equal.
*/

bool operator!=( const QRect &r1, const QRect &r2 )
{
    return r1.x1!=r2.x1 || r1.x2!=r2.x2 || r1.y1!=r2.y1 || r1.y2!=r2.y2;
}


/*****************************************************************************
  QRect stream functions
 *****************************************************************************/

/*!
  \relates QRect

  Writes a QRect to the stream and returns a reference to the stream.

  Serialization format: [left (INT16), top (INT16), right (INT16),
  bottom (INT16)].
*/

QDataStream &operator<<( QDataStream &s, const QRect &r )
{
    return s << (INT16)r.left() << (INT16)r.top()
	     << (INT16)r.right() << (INT16)r.bottom();
}

/*!
  \relates QRect

  Reads a QRect from the stream and returns a reference to the stream.
*/

QDataStream &operator>>( QDataStream &s, QRect &r )
{
    INT16 x1, y1, x2, y2;
    s >> x1; s >> y1; s >> x2; s >> y2;
    r.setCoords( x1, y1, x2, y2 );
    return s;
}
