/****************************************************************************
** $Id: qpaintdevice_x11.cpp,v 2.16 1998/07/03 00:09:35 hanord Exp $
**
** Implementation of QPaintDevice class for X11
**
** Created : 940721
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

#include "qpaintdevice.h"
#include "qpaintdevicedefs.h"
#include "qwidget.h"
#include "qbitmap.h"
#include "qapplication.h"
#define	 GC GC_QQQ
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>

/*!
  \class QPaintDevice qpaintdevice.h
  \brief The base class of objects that can be painted.

  \ingroup drawing

  A paint device is an abstraction of a two-dimensional space that can be
  drawn using a QPainter.
  The drawing capabilities are implemented by the subclasses: QWidget,
  QPixmap, QPicture and QPrinter.

  The default coordinate system of a paint device has its origin
  located at the top left position. X increases to the right and Y
  increases downwards. The unit is one pixel.  There are several ways
  to set up a user-defined coordinate system using the painter, for
  example by QPainter::setWorldMatrix().

  Example (draw on a paint device):
  \code
    void MyWidget::paintEvent( QPaintEvent * )
    {
	QPainter p;				// our painter
	p.begin( this );			// start painting widget
	p.setPen( red );			// blue outline
	p.setBrush( yellow );			// yellow fill
	p.drawEllipse( 10,20, 100,100 );	// 100x100 ellipse at 10,20
	p.end();				// painting done
    }
  \endcode

  The bit block transfer is an extremely useful operation for copying pixels
  from one paint device to another (or to itself).
  It is implemented as the global function bitBlt().

  Example (scroll widget contents 10 pixels to the right):
  \code
    bitBlt( myWidget, 10,0, myWidget );
  \endcode

  \warning Qt requires that a QApplication object must exist before any paint
  devices can be created.  Paint devices access window system resources, and
  these resources are not initialized before an application object is created.
*/


//
// Some global variables - these are initialized by QColor::initialize()
//

Display *QPaintDevice::dpy = 0;
Display *QPaintDevice::x_display = 0;
int	 QPaintDevice::x_screen;
int	 QPaintDevice::x_depth;
int	 QPaintDevice::x_cells;
HANDLE	 QPaintDevice::x_colormap;
bool	 QPaintDevice::x_defcmap;
void	*QPaintDevice::x_visual;
bool	 QPaintDevice::x_defvisual;


/*!
  Constructs a paint device with internal flags \e devflags.
  This constructor can only be invoked from subclasses of QPaintDevice.
*/

QPaintDevice::QPaintDevice( uint devflags )
{
    if ( !qApp ) {				// global constructor
#if defined(CHECK_STATE)
	fatal( "QPaintDevice: Must construct a QApplication before a "
	       "QPaintDevice" );
#endif
	return;
    }
    devFlags = devflags;
    if ( dpy == 0 )
	dpy = qt_xdisplay();
    hd	= 0;
}

/*!
  Destroys the paint device and frees window system resources.
*/

QPaintDevice::~QPaintDevice()
{
#if defined(CHECK_STATE)
    if ( paintingActive() )
	warning( "QPaintDevice: Cannot destroy paint device that is being "
		 "painted" );
#endif
}


/*!
  \fn int QPaintDevice::devType() const
  Returns the device type identifier: \c PDT_WIDGET, \c PDT_PIXMAP,
  \c PDT_PRINTER, \c PDT_PICTURE or \c PDT_UNDEF.
*/

/*!
  \fn bool QPaintDevice::isExtDev() const
  Returns TRUE if the device is a so-called external paint device.

  External paint devices cannot be bitBlt()'ed from.
  QPicture and QPrinter are external paint devices.
*/

/*!
  \fn HANDLE QPaintDevice::handle() const

  Returns the window system handle of the paint device, for low-level
  access.  <em>Using this function is not portable.</em>

  The HANDLE type varies with platform; see qpaintdevice.h and qwindowdefs.h
  for details.

  \sa x11Display()
*/

/*!
  \fn Display *QPaintDevice::x11Display() const

  Returns a pointer to the X display (X11 only).
  <em>Using this function is not portable.</em>

  \sa handle()
*/

/*!
  \fn bool QPaintDevice::paintingActive() const
  Returns TRUE if the device is being painted, i.e. someone has called
  QPainter::begin() and not yet QPainter::end() for this device.
  \sa QPainter::isActive()
*/

/*!
  Internal virtual function that interprets drawing commands from
  the painter.

  Implemented by subclasses that have no direct support for drawing
  graphics (external paint devices, for example QPicture).
*/

bool QPaintDevice::cmd( int, QPainter *, QPDevCmdParam * )
{
#if defined(CHECK_STATE)
    warning( "QPaintDevice::cmd: Device has no command interface" );
#endif
    return FALSE;
}

/*!
  Internal virtual function that returns paint device metrics.

  Please use the QPaintDeviceMetrics class instead.
*/

int QPaintDevice::metric( int ) const
{
#if defined(CHECK_STATE)
    warning( "QPaintDevice::metrics: Device has no metric information" );
#endif
    return 0;
}

/*!
  Internal virtual function. Reserved for future use.

  \internal
  Please use the QFontMetrics class instead.
*/

int QPaintDevice::fontMet( QFont *, int, const char *, int ) const
{
    return 0;
}

/*!
  Internal virtual function. Reserved for future use.

  \internal
  Please use the QFontInfo class instead.
*/

int QPaintDevice::fontInf( QFont *, int ) const
{
    return 0;
}


//
// Internal functions for simple GC caching for blt'ing masked pixmaps.
// This cache is used when the pixmap optimization is set to Normal
// and the pixmap size doesn't exceed 128x128.
//

static bool      init_mask_gc = FALSE;
static const int max_mask_gcs = 11;		// suitable for hashing

struct mask_gc {
    GC	gc;
    int mask_no;
};

static mask_gc gc_vec[max_mask_gcs];


static void cleanup_mask_gc()
{
    Display *dpy = qt_xdisplay();
    init_mask_gc = FALSE;
    for ( int i=0; i<max_mask_gcs; i++ ) {
	if ( gc_vec[i].gc )
	    XFreeGC( dpy, gc_vec[i].gc );
    }
}

static GC cache_mask_gc( Display *dpy, Drawable hd, int mask_no, Pixmap mask )
{
    if ( !init_mask_gc ) {			// first time initialization
	init_mask_gc = TRUE;
	qAddPostRoutine( cleanup_mask_gc );
	for ( int i=0; i<max_mask_gcs; i++ )
	    gc_vec[i].gc = 0;
    }
    mask_gc *p = &gc_vec[mask_no % max_mask_gcs];
    if ( !p->gc || p->mask_no != mask_no ) {	// not a perfect match
	if ( !p->gc ) {				// no GC
	    p->gc = XCreateGC( dpy, hd, 0, 0 );
	    XSetGraphicsExposures( dpy, p->gc, FALSE );
	}
	XSetClipMask( dpy, p->gc, mask );
	p->mask_no = mask_no;
    }
    return p->gc;
}


/*!
  \relates QPaintDevice
  This function copies a block of pixels from one paint device to another
  (bitBlt means bit block transfer).

  \arg \e dst is the paint device to copy to.
  \arg \e dx and \e dy is the position to copy to.
  \arg \e src is the paint device to copy from.
  \arg \e sx and \e sy is the position to copy from.
  \arg \e sw and \e sh is the width and height of the block to be copied.
  \arg \e rop defines the raster operation to be used when copying.

  If \e sw is 0 or \e sh is 0, then bitBlt will do nothing.

  If \e sw is negative, then bitBlt calculates <code>sw = src->width -
  sx.</code> If \e sh is negative, then bitBlt calculates <code>sh =
  src->height - sy.</code>

  The \e rop argument can be one of:
  <ul>
  <li> \c CopyROP:     dst = src.
  <li> \c OrROP:       dst = dst OR src.
  <li> \c XorROP:      dst = dst XOR src.
  <li> \c EraseROP:    dst = (NOT src) AND dst
  <li> \c NotCopyROP:  dst = NOT src
  <li> \c NotOrROP:    dst = (NOT src) OR dst
  <li> \c NotXorROP:   dst = (NOT src) XOR dst
  <li> \c NotEraseROP: dst = src AND dst
  <li> \c NotROP:      dst = NOT dst
  </ul>

  The \e ignoreMask argument (default FALSE) applies where \e src is
  a QPixmap with a \link QPixmap::setMask() mask\endlink.
  If \e ignoreMask is TRUE, bitBlt ignores the pixmap's mask.

  BitBlt has two restrictions:
  <ol>
  <li> The \e src device must be QWidget or QPixmap.  You cannot copy pixels
  from a picture or a printer (external device).
  <li> The \e src device may not have pixel depth greater than \e dst.
  You cannot copy from an 8 bit pixmap to a 1 bit pixmap.
  </ol>
*/

void bitBlt( QPaintDevice *dst, int dx, int dy,
	     const QPaintDevice *src, int sx, int sy, int sw, int sh,
	     RasterOp rop, bool ignoreMask )
{
    if ( !src || !dst ) {
#if defined(CHECK_NULL)
	ASSERT( src != 0 );
	ASSERT( dst != 0 );
#endif
	return;
    }
    if ( !src->handle() || src->isExtDev() )
	return;

    int ts = src->devType();			// from device type
    int td = dst->devType();			// to device type
    Display *dpy = src->x11Display();

    if ( sw <= 0 ) {				// special width
	if ( sw < 0 )
	    sw = src->metric( PDM_WIDTH ) - sx;
	else
	    return;
    }
    if ( sh <= 0 ) {				// special height
	if ( sh < 0 )
	    sh = src->metric( PDM_HEIGHT ) - sy;
	else
	    return;
    }

    if ( dst->paintingActive() && dst->isExtDev() ) {
	QPixmap *pm;				// output to picture/printer
	bool	 tmp_pm = TRUE;
	if ( ts == PDT_PIXMAP ) {
	    pm = (QPixmap*)src;
	    if ( sx != 0 || sy != 0 ||
		 sw != pm->width() || sh != pm->height() || ignoreMask ) {
		QPixmap *tmp = new QPixmap( sw, sh, pm->depth() );
		bitBlt( tmp, 0, 0, pm, sx, sy, sw, sh, CopyROP, TRUE );
		if ( pm->mask() && !ignoreMask ) {
		    QBitmap mask( sw, sh );
		    bitBlt( &mask, 0, 0, pm->mask(), sx, sy, sw, sh,
			    CopyROP, TRUE );
		    tmp->setMask( mask );
		}
		pm = tmp;
	    } else {
		tmp_pm = FALSE;
	    }
	} else if ( ts == PDT_WIDGET ) {	// bitBlt to temp pixmap
	    pm = new QPixmap( sw, sh );
	    CHECK_PTR( pm );
	    bitBlt( pm, 0, 0, src, sx, sy, sw, sh );
	} else {
#if defined(CHECK_RANGE)
	    warning( "bitBlt: Cannot bitBlt from device" );
#endif
	    return;
	}
	QPDevCmdParam param[3];
	QPoint p(dx,dy);
	param[0].point	= &p;
	param[1].pixmap = pm;
	dst->cmd( PDC_DRAWPIXMAP, 0, param );
	if ( tmp_pm )
	    delete pm;
	return;
    }

    if ( !(ts <= PDT_PIXMAP && td <= PDT_PIXMAP) ) {
#if defined(CHECK_RANGE)
	warning( "bitBlt: Cannot bitBlt to or from device" );
#endif
	return;
    }
    static short ropCodes[] =			// ROP translation table
	{ GXcopy, GXor, GXxor, GXandInverted,
	  GXcopyInverted, GXorInverted, GXequiv, GXand, GXinvert };
    if ( rop > NotROP ) {
#if defined(CHECK_RANGE)
	warning( "bitBlt: Invalid ROP code" );
#endif
	return;
    }

    if ( dst->handle() == 0 ) {
#if defined(CHECK_NULL)
	warning( "bitBlt: Cannot bitBlt to device" );
#endif
	return;
    }

    bool mono_src;
    bool mono_dst;
    bool include_inferiors = FALSE;
    bool graphics_exposure = FALSE;
    QPixmap *src_pm;
    QBitmap *mask;

    if ( ts == PDT_PIXMAP ) {
	src_pm = (QPixmap*)src;
	mono_src = src_pm->depth() == 1;
	mask = ignoreMask ? 0 : src_pm->data->mask;
    } else {
	src_pm = 0;
	mono_src = FALSE;
	mask = 0;
	include_inferiors = ((QWidget*)src)->testWFlags(WPaintUnclipped);
	graphics_exposure = td == PDT_WIDGET;
    }
    if ( td == PDT_PIXMAP ) {
	mono_dst = ((QPixmap*)dst)->depth() == 1;
	((QPixmap*)dst)->detach();		// changes shared pixmap
    } else {
	mono_dst = FALSE;
	include_inferiors = include_inferiors ||
	    ((QWidget*)dst)->testWFlags(WPaintUnclipped);
    }

    if ( mono_dst && !mono_src ) {	// dest is 1-bit pixmap, source is not
#if defined(CHECK_RANGE)
	warning( "bitBlt: Incompatible destination pixmap" );
#endif
	return;
    }

    GC gc;

    if ( mask && !mono_src ) {			// fast masked blt
	bool temp_gc = FALSE;
	if ( mask->data->maskgc ) {
	    gc = (GC)mask->data->maskgc;	// we have a premade mask GC
	} else {
	    if ( src_pm->optimization() == QPixmap::NormalOptim ) {
		// Compete for the global cache
		gc = cache_mask_gc( dpy, dst->handle(),
				    mask->data->ser_no,
				    mask->handle() );
	    } else {
		// Create a new mask GC. If BestOptim, we store the mask GC
		// with the mask (not at the pixmap). This way, many pixmaps
		// which have a common mask will be optimized at no extra cost.
		gc = XCreateGC( dpy, dst->handle(), 0, 0 );
		XSetGraphicsExposures( dpy, gc, FALSE );
		XSetClipMask( dpy, gc, mask->handle() );
		if ( src_pm->optimization() == QPixmap::BestOptim ) {
		    mask->data->maskgc = gc;
		} else {
		    temp_gc = TRUE;
		}
	    }
	}
	XSetClipOrigin( dpy, gc, dx-sx, dy-sy );
	if ( rop != CopyROP )			// use non-default ROP code
	    XSetFunction( dpy, gc, ropCodes[rop] );
	if ( include_inferiors ) {
	    XSetSubwindowMode( dpy, gc, IncludeInferiors );
	    XCopyArea( dpy, src->handle(), dst->handle(), gc, sx, sy, sw, sh,
		       dx, dy );
	    XSetSubwindowMode( dpy, gc, ClipByChildren );
	} else {
	    XCopyArea( dpy, src->handle(), dst->handle(), gc, sx, sy, sw, sh,
		       dx, dy );
	}

	if ( temp_gc )				// delete temporary GC
	    XFreeGC( dpy, gc );
	else if ( rop != CopyROP )		// restore ROP
	    XSetFunction( dpy, gc, GXcopy );
	return;
    }

    gc = qt_xget_temp_gc( mono_dst );		// get a reusable GC

    if ( rop != CopyROP )			// use non-default ROP code
	XSetFunction( dpy, gc, ropCodes[rop] );

    if ( mono_src ) {				// src is bitmap
	XGCValues gcvals;
	ulong	  valmask = GCBackground | GCForeground | GCFillStyle |
			    GCStipple | GCTileStipXOrigin | GCTileStipYOrigin;
	if ( td == PDT_WIDGET ) {		// set GC colors
	    QWidget *w = (QWidget *)dst;
	    gcvals.background = w->backgroundColor().pixel();
	    gcvals.foreground = w->foregroundColor().pixel();
	    if ( include_inferiors ) {
		valmask |= GCSubwindowMode;
		gcvals.subwindow_mode = IncludeInferiors;
	    }
	} else if ( mono_dst ) {
	    gcvals.background = 0;
	    gcvals.foreground = 1;
	} else {
	    gcvals.background = white.pixel();
	    gcvals.foreground = black.pixel();
	}

	gcvals.fill_style  = FillOpaqueStippled;
	gcvals.stipple	   = src->handle();
	gcvals.ts_x_origin = dx - sx;
	gcvals.ts_y_origin = dy - sy;

	bool clipmask = FALSE;
	if ( mask ) {
	    if ( ((QPixmap*)src)->data->selfmask ) {
		gcvals.fill_style = FillStippled;
	    } else {
		XSetClipMask( dpy, gc, mask->handle() );
		XSetClipOrigin( dpy, gc, dx-sx, dy-sy );
		clipmask = TRUE;
	    }
	}

	XChangeGC( dpy, gc, valmask, &gcvals );
	XFillRectangle( dpy,dst->handle(), gc, dx, dy, sw, sh );

	valmask = GCFillStyle | GCTileStipXOrigin | GCTileStipYOrigin;
	gcvals.fill_style  = FillSolid;
	gcvals.ts_x_origin = 0;
	gcvals.ts_y_origin = 0;
	if ( include_inferiors ) {
	    valmask |= GCSubwindowMode;
	    gcvals.subwindow_mode = ClipByChildren;
	}
	XChangeGC( dpy, gc, valmask, &gcvals );

	if ( clipmask ) {
	    XSetClipOrigin( dpy, gc, 0, 0 );
	    XSetClipMask( dpy, gc, None );
	}

    } else {					// src is pixmap/widget

	if ( graphics_exposure )		// widget to widget
	    XSetGraphicsExposures( dpy, gc, TRUE );
	if ( include_inferiors ) {
	    XSetSubwindowMode( dpy, gc, IncludeInferiors );
	    XCopyArea( dpy, src->handle(), dst->handle(), gc, sx, sy, sw, sh,
		       dx, dy );
	    XSetSubwindowMode( dpy, gc, ClipByChildren );
	} else {
	    XCopyArea( dpy, src->handle(), dst->handle(), gc, sx, sy, sw, sh,
		       dx, dy );
	}
	if ( graphics_exposure )		// reset graphics exposure
	    XSetGraphicsExposures( dpy, gc, FALSE );
    }

    if ( rop != CopyROP )			// restore ROP
	XSetFunction( dpy, gc, GXcopy );
}


/*!
  \fn void bitBlt( QPaintDevice *dst, const QPoint &dp, const QPaintDevice *src, const QRect &sr, RasterOp rop )

  Overloaded bitBlt() with the destination point \e dp and source rectangle
  \e sr.

  \relates QPaintDevice
*/
