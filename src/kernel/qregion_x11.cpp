/****************************************************************************
** $Id: qregion_x11.cpp,v 2.17.2.1 1999/01/22 15:45:30 warwick Exp $
**
** Implementation of QRegion class for X11
**
** Created : 940729
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

#include "qregion.h"
#include "qpointarray.h"
#include "qbuffer.h"
#include "qimage.h"
#include "qbitmap.h"
#define	 GC GC_QQQ
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>

static QRegion *empty_region = 0;

static void cleanup_empty_region()
{
    delete empty_region;
    empty_region = 0;
}


/*!
  Constructs an null region.
  \sa isNull()
*/

QRegion::QRegion()
{
    if ( !empty_region ) {			// avoid too many allocs
	qAddPostRoutine( cleanup_empty_region );
	empty_region = new QRegion( TRUE );
	CHECK_PTR( empty_region );
    }
    data = empty_region->data;
    data->ref();
}

/*!
  Internal constructor that creates a null region.
*/

QRegion::QRegion( bool is_null )
{
    data = new QRegionData;
    CHECK_PTR( data );
    data->rgn = XCreateRegion();
    data->is_null = is_null;
}

/*!
  Constructs a rectangular or elliptic region.

  \arg \e r is the region rectangle.
  \arg \e t is the region type: QRegion::Rectangle (default) or
  QRegion::Ellipse.
*/

QRegion::QRegion( const QRect &r, RegionType t )
{
    QRect rr = r.normalize();
    data = new QRegionData;
    CHECK_PTR( data );
    data->is_null = FALSE;
    if ( t == Rectangle ) {			// rectangular region
	data->rgn = XCreateRegion();
	XRectangle xr;
	xr.x = rr.x();
	xr.y = rr.y();
	xr.width  = rr.width();
	xr.height = rr.height();
	XUnionRectWithRegion( &xr, data->rgn, data->rgn );
    } else if ( t == Ellipse ) {		// elliptic region
	QPointArray a;
	a.makeEllipse( rr.x(), rr.y(), rr.width(), rr.height() );
	data->rgn = XPolygonRegion( (XPoint*)a.data(), a.size(), EvenOddRule );
    }
}


/*!
  Constructs a polygon region from the point array \e a.

  If \e winding is TRUE, the polygon region uses the winding
  algorithm, otherwise the alternative (even-odd) algorithm
  will be used.
*/

QRegion::QRegion( const QPointArray &a, bool winding )
{
    data = new QRegionData;
    CHECK_PTR( data );
    data->is_null = FALSE;
    data->rgn = XPolygonRegion( (XPoint*)a.data(), a.size(),
				winding ? WindingRule : EvenOddRule );
}


/*!
  Constructs a region which is a
  \link shclass.html shallow copy\endlink of \e r.
*/

QRegion::QRegion( const QRegion &r )
{
    data = r.data;
    data->ref();
}

Region qt_x11_bitmapToRegion(const QBitmap& bitmap)
{
    Region region = XCreateRegion();
    QImage image = bitmap.convertToImage();

    XRectangle xr;

#define AddSpan \
	{ \
	    xr.x = prev1; \
	    xr.y = y; \
	    xr.width = x-prev1-1; \
	    xr.height = 1; \
	    XUnionRectWithRegion( &xr, region, region ); \
	}

    // deal with 0<->1 problem (not on X11 anymore)
    int zero=0;//(qGray(image.color(0)) < qGray(image.color(1)) ? 0x00 : 0xff);
    bool little = image.bitOrder() == QImage::LittleEndian;

    int x, y;
    for (y=0; y<image.height(); y++) {
	uchar *line = image.scanLine(y);
	int w = image.width();
	uchar all=zero;
	int prev1 = -1;
	for (x=0; x<w; ) {
	    uchar byte = line[x/8];
	    if ( x>w-8 || byte!=all ) {
		if ( little ) {
		    for ( int b=8; b>0 && x<w; b-- ) {
			if ( !(byte&0x01) == !all ) {
			    // More of the same
			} else {
			    // A change.
			    if ( all!=zero ) {
				AddSpan;
				all = zero;
			    } else {
				prev1 = x;
				all = ~zero;
			    }
			}
			byte >>= 1;
			x++;
		    }
		} else {
		    for ( int b=8; b>0 && x<w; b-- ) {
			if ( !(byte&0x80) == !all ) {
			    // More of the same
			} else {
			    // A change.
			    if ( all!=zero ) {
				AddSpan;
				all = zero;
			    } else {
				prev1 = x;
				all = ~zero;
			    }
			}
			byte <<= 1;
			x++;
		    }
		}
	    } else {
		x+=8;
	    }
	}
	if ( all != zero ) {
	    AddSpan;
	}
    }

    return region;
}


/*!
  Constructs a region from a bitmap.

  The pixels in \a bm that are color1 will be part of the
  region as if each was a 1 by 1 rectangle.
*/
QRegion::QRegion( const QBitmap & bm )
{
    data = new QRegionData;
    CHECK_PTR( data );
    data->is_null = FALSE;
    data->rgn = qt_x11_bitmapToRegion(bm);
}

/*!
  Destroys the region.
*/

QRegion::~QRegion()
{
    if ( data->deref() ) {
	XDestroyRegion( data->rgn );
	delete data;
    }
}


/*!
  Assigns a 
  \link shclass.html shallow copy\endlink of \e r to this region and
  returns a reference to the region.
*/

QRegion &QRegion::operator=( const QRegion &r )
{
    r.data->ref();				// beware of r = r
    if ( data->deref() ) {
	XDestroyRegion( data->rgn );
	delete data;
    }
    data = r.data;
    return *this;
}


/*!
  Returns a 
  \link shclass.html deep copy\endlink of the region.
*/

QRegion QRegion::copy() const
{
    QRegion r( data->is_null );
    XUnionRegion( data->rgn, r.data->rgn, r.data->rgn );
    return r;
}


/*!
  Returns TRUE if the region is a null region, otherwise FALSE.

  A null region is a region that has not been initialized. The
  documentation for isEmpty() contains an example that shows how to use
  isNull() and isEmpty().

  \sa isEmpty()
*/

bool QRegion::isNull() const
{
    return data->is_null;
}


/*!
  Returns TRUE if the region is empty, or FALSE if it is non-empty.

  Example:
  \code
    QRegion r1( 10, 10, 20, 20 );
    QRegion r2( 40, 40, 20, 20 );
    QRegion r3;
    r1.isNull();		// FALSE
    r1.isEmpty();		// FALSE
    r3.isNull();		// TRUE
    r3.isEmpty();		// TRUE
    r3 = r1.intersect( r2 );	// r3 = intersection of r1 and r2
    r3.isNull();		// FALSE
    r3.isEmpty();		// TRUE
    r3 = r1.unite( r2 );	// r3 = union of r1 and r2
    r3.isNull();		// FALSE
    r3.isEmpty();		// FALSE
  \endcode

  \sa isNull()
*/

bool QRegion::isEmpty() const
{
    return data->is_null || XEmptyRegion( data->rgn );
}


/*!
  Returns TRUE if the region contains the point \e p, or FALSE if \e p is
  outside the region.
*/

bool QRegion::contains( const QPoint &p ) const
{
    return XPointInRegion( data->rgn, p.x(), p.y() );
}

/*!
  Returns TRUE if the region contains the rectangle \e r, or FALSE if \e r is
  outside the region.
*/

bool QRegion::contains( const QRect &r ) const
{
    return XRectInRegion( data->rgn, r.left(), r.right(),
			  r.width(), r.height() ) != RectangleOut;
}


/*!
  Translates the region \e dx along the X axis and \e dy along the Y axis.
*/

void QRegion::translate( int dx, int dy )
{
    if ( data == empty_region->data )
	return;
    detach();
    XOffsetRegion( data->rgn, dx, dy );
}


/*!
  Returns a region which is the union of this region and \e r.
*/

QRegion QRegion::unite( const QRegion &r ) const
{
    QRegion result( FALSE );
    XUnionRegion( data->rgn, r.data->rgn, result.data->rgn );
    return result;
}

/*!
  Returns a region which is the intersection of this region and \e r.
*/

QRegion QRegion::intersect( const QRegion &r ) const
{
    QRegion result( FALSE );
    XIntersectRegion( data->rgn, r.data->rgn, result.data->rgn );
    return result;
}

/*!
  Returns a region which is \e r subtracted from this region.
*/

QRegion QRegion::subtract( const QRegion &r ) const
{
    QRegion result( FALSE );
    XSubtractRegion( data->rgn, r.data->rgn, result.data->rgn );
    return result;
}

/*!
  Returns a region which is this region XOR \e r.
*/

QRegion QRegion::eor( const QRegion &r ) const
{
    QRegion result( FALSE );
    XXorRegion( data->rgn, r.data->rgn, result.data->rgn );
    return result;
}


/*!
  Returns the bounding rectange of this region.
  An empty region gives a \link QRect::isNull() null\endlink
  rectangle.
*/

QRect QRegion::boundingRect() const
{
    XRectangle r;
    XClipBox( data->rgn, &r );
    return QRect( r.x, r.y, r.width, r.height );
}


/*
  This is how X represents regions internally.
*/

struct BOX {
    short x1, x2, y1, y2;
};

struct _XRegion {
    long size;
    long numRects;
    BOX *rects;
    BOX  extents;
};


/*!
  Returns an array of the rectangles that make up the region.
  The rectangles are non-overlapping. The region is formed by
  the union of all these rectangles.
*/

QArray<QRect> QRegion::rects() const
{
    QArray<QRect> a( (int)data->rgn->numRects );
    BOX *r = data->rgn->rects;
    for ( int i=0; i<(int)a.size(); i++ ) {
	// Note: the -1 are correct - see that setClipRect(r)
	//       gives r back.
	a[i].setCoords( r->x1, r->y1, r->x2-1, r->y2-1);
	r++;
    }
    return a;
}


/*!
  Returns TRUE if the region is equal to \e r, or FALSE if the regions are
  different.
*/

bool QRegion::operator==( const QRegion &r ) const
{
    return data == r.data ?
	TRUE : XEqualRegion( data->rgn, r.data->rgn );
}

/*!
  \fn bool QRegion::operator!=( const QRegion &r ) const
  Returns TRUE if the region is different from \e r, or FALSE if the regions
  are equal.
*/
