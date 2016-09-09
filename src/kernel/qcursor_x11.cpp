/****************************************************************************
** $Id: qcursor_x11.cpp,v 2.12 1998/07/03 00:09:31 hanord Exp $
**
** Implementation of QCursor class for X11
**
** Created : 940219
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

#include "qcursor.h"
#include "qbitmap.h"
#include "qapplication.h"
#include "qdatastream.h"
#define	 GC GC_QQQ
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include <X11/cursorfont.h>

/*****************************************************************************
  Internal QCursorData class
 *****************************************************************************/

struct QCursorData : public QShared
{
    QCursorData();
   ~QCursorData();
    int	      cshape;
    QBitmap  *bm, *bmm;
    short     hx, hy;
    Cursor    hcurs;
    Pixmap    pm, pmm;
};

QCursorData::QCursorData()
{
    bm = bmm = 0;
    hx = hy  = 0;
    pm = pmm = 0;
}

QCursorData::~QCursorData()
{
    Display *dpy = qt_xdisplay();
    if ( hcurs )
	XFreeCursor( dpy, hcurs );
    if ( pm )
	XFreePixmap( dpy, pm );
    if ( pmm )
	XFreePixmap( dpy, pmm );
    if ( bm )
	delete bm;
    if ( bmm )
	delete bmm;
}


/*****************************************************************************
  Global cursors
 *****************************************************************************/

const QCursor arrowCursor;
const QCursor upArrowCursor;
const QCursor crossCursor;
const QCursor waitCursor;
const QCursor ibeamCursor;
const QCursor sizeVerCursor;
const QCursor sizeHorCursor;
const QCursor sizeBDiagCursor;
const QCursor sizeFDiagCursor;
const QCursor sizeAllCursor;
const QCursor blankCursor;


/*****************************************************************************
  QCursor member functions
 *****************************************************************************/

static QCursor *cursorTable[] = {		// the order is important!!
    (QCursor*)&arrowCursor,
    (QCursor*)&upArrowCursor,
    (QCursor*)&crossCursor,
    (QCursor*)&waitCursor,
    (QCursor*)&ibeamCursor,
    (QCursor*)&sizeVerCursor,
    (QCursor*)&sizeHorCursor,
    (QCursor*)&sizeBDiagCursor,
    (QCursor*)&sizeFDiagCursor,
    (QCursor*)&sizeAllCursor,
    (QCursor*)&blankCursor,
    0
};

static QCursor *find_cur( int shape )		// find predefined cursor
{
    return (uint)shape <= LastCursor ? cursorTable[shape] : 0;
}


/*!
  Internal function that initializes the predefined cursors.
  This function is called from the QApplication constructor.
  \sa cleanup()
*/

void QCursor::initialize()
{
    int shape = ArrowCursor;
    while ( cursorTable[shape] ) {
	if ( !cursorTable[shape]->data ) {	// workaround for gcc-2.6.3 bug
	    cursorTable[shape]->data = new QCursorData;
	    CHECK_PTR( cursorTable[shape]->data );
	    cursorTable[shape]->data->hcurs = 0;
	}
	cursorTable[shape]->data->cshape = shape;
	shape++;
    }
}

/*!
  Internal function that cleans up the predefined cursors.
  This function is called from the QApplication destructor.
  \sa initialize()
*/

void QCursor::cleanup()
{
    int shape = ArrowCursor;
    while ( cursorTable[shape] ) {
	delete cursorTable[shape]->data;
	cursorTable[shape]->data = 0;
	shape++;
    }
}


/*!
  Constructs a cursor with the default arrow shape.
*/

QCursor::QCursor()
{
    if ( QApplication::startingUp() ) {		// this is a global cursor
	data = new QCursorData;
	CHECK_PTR( data );
	data->cshape = 0;
	data->hcurs = 0;
    } else {					// default arrow cursor
	data = arrowCursor.data;
	data->ref();
    }
}

/*!
  Constructs a cursor with the specified \e shape.
*/

QCursor::QCursor( int shape )			// cursor with shape
{
    QCursor *c = find_cur( shape );
    if ( !c )					// not found
	c = (QCursor *)&arrowCursor;		//   then use arrowCursor
    c->data->ref();
    data = c->data;
}


/*!
  Constructs a custom bitmap cursor.

  \arg \e bitmap and
  \arg \e mask make up the bitmap.
  \arg \e hotX and
  \arg \e hotY define the hot spot of this cursor.

  If \e hotX is negative, it is set to the bitmap().width()/2.
  If \e hotY is negative, it is set to the bitmap().height()/2.

  The cursor \e bitmap (B) and \e mask (M) bits are combined this way:
  <ol>
  <li> B=1 and M=1 gives black.
  <li> B=0 and M=1 gives white.
  <li> B=0 and M=0 gives transparency.
  <li> B=1 and M=0 gives an undefined result.
  </ol>

  Use the global color \c color0 to draw 0-pixels and \c color1 to draw
  1-pixels in the bitmaps.

  Allowed cursor sizes depend on the display hardware (or the underlying
  window system). We recommend using 32x32 cursors, because this size
  is supported on all platforms. Some platforms also support 16x16, 48x48
  and 64x64 cursors.
*/

QCursor::QCursor( const QBitmap &bitmap, const QBitmap &mask,
		  int hotX, int hotY )
{
    if ( bitmap.depth() != 1 || mask.depth() != 1 ||
	 bitmap.size() != mask.size() ) {
#if defined(CHECK_NULL)
	warning( "QCursor: Cannot create bitmap cursor; invalid bitmap(s)" );
#endif
	QCursor *c = (QCursor *)&arrowCursor;
	c->data->ref();
	data = c->data;
	return;
    }
    data = new QCursorData;
    CHECK_PTR( data );
    data->bm  = new QBitmap( bitmap );
    data->bmm = new QBitmap( mask );
    data->hcurs = 0;
    data->cshape = BitmapCursor;
    data->hx = hotX >= 0 ? hotX : bitmap.width()/2;
    data->hy = hotY >= 0 ? hotY : bitmap.height()/2;
}

/*!
  Constructs a copy of the cursor \e c.
*/

QCursor::QCursor( const QCursor &c )
{
    data = c.data;				// shallow copy
    data->ref();
}

/*!
  Destroys the cursor.
*/

QCursor::~QCursor()
{
    if ( data && data->deref() )
	delete data;
}


/*!
  Assigns \e c to this cursor and returns a reference to this cursor.
*/

QCursor &QCursor::operator=( const QCursor &c )
{
    c.data->ref();				// avoid c = c
    if ( data->deref() )
	delete data;
    data = c.data;
    return *this;
}


/*!
  Returns the cursor shape identifer. If a custom bitmap has been set,
  \c BitmapCursor is returned.

  \sa setShape()
*/

int QCursor::shape() const			// get cursor shape
{
    return data->cshape;
}

/*!
  Sets the cursor to the shape identified by \e shape.

  The allowed shapes are: \c ArrowCursor, \c UpArrowCursor, \c
  CrossCursor, \c WaitCursor, \c IbeamCursor, \c SizeVerCursor, \c
  SizeHorCursor, \c SizeBDiagCursor, \c SizeFDiagCursor, \c
  SizeAllCursor, \c BlankCursor. These correspond to the <a
  href="#cursors">predefined</a> global QCursor objects.

  \sa shape()
*/

void QCursor::setShape( int shape )
{
    QCursor *c = find_cur( shape );		// find one of the global ones
    if ( !c )					// not found
	c = (QCursor *)&arrowCursor;		//   then use arrowCursor
    c->data->ref();
    if ( data->deref() )			// make shallow copy
	delete data;
    data = c->data;
}


/*!
  Returns the cursor bitmap, or 0 if it is one of the standard cursors.
*/

const QBitmap *QCursor::bitmap() const
{
    return data->bm;
}

/*!
  Returns the cursor bitmap mask, or 0 if it is one of the standard cursors.
*/

const QBitmap *QCursor::mask() const
{
    return data->bmm;
}

/*!
  Returns the cursor hot spot, or (0,0) if it is one of the standard cursors.
*/

QPoint QCursor::hotSpot() const
{
    return QPoint( data->hx, data->hy );
}


/*!
  Returns the window system cursor handle.

  \warning
  Portable in principle, but if you use it you are probably about to do
  something non-portable. Be careful.
*/

HANDLE QCursor::handle() const
{
    if ( !data->hcurs )
	update();
    return data->hcurs;
}


/*!
  Returns the position of the cursor (hot spot) in global screen
  coordinates.

  You can call QWidget::mapFromGlobal() to translate it to widget
  coordinates.

  \sa setPos(), QWidget::mapFromGlobal(), QWidget::mapToGlobal()
*/

QPoint QCursor::pos()
{
    Window root;
    Window child;
    int root_x, root_y, win_x, win_y;
    uint buttons;
    XQueryPointer( qt_xdisplay(), qt_xrootwin(), &root, &child,
		   &root_x, &root_y, &win_x, &win_y, &buttons );
    return QPoint( root_x, root_y );
}

/*!
  Moves the cursor (hot spot) to the global screen position \e (x,y).

  You can call QWidget::mapToGlobal() to translate widget coordinates
  to global screen coordinates.

  \sa pos(), QWidget::mapFromGlobal(), QWidget::mapToGlobal()
*/

void QCursor::setPos( int x, int y )
{
    // Need to check, since some X servers generate null mouse move
    // events, causing looping in applications which call setPos() on
    // every mouse move event.
    //
    if (pos() == QPoint(x,y))
	return;

    XWarpPointer( qt_xdisplay(), None, qt_xrootwin(), 0, 0, 0, 0, x, y );
}


/*!
  \overload void QCursor::setPos ( const QPoint & )
*/

/*!
  \internal Creates the cursor.
*/

void QCursor::update() const
{
    register QCursorData *d = data;		// cheat const!
    if ( d->hcurs )				// already loaded
	return;

  // Non-standard X11 cursors are created from bitmaps

    static uchar cur_ver_bits[] = {
	0x00, 0x00, 0x00, 0x00, 0x80, 0x01, 0xc0, 0x03, 0xe0, 0x07, 0xf0, 0x0f,
	0x80, 0x01, 0x80, 0x01, 0x80, 0x01, 0x80, 0x01, 0x80, 0x01, 0xf0, 0x0f,
	0xe0, 0x07, 0xc0, 0x03, 0x80, 0x01, 0x00, 0x00 };
    static uchar mcur_ver_bits[] = {
	0x00, 0x00, 0x80, 0x03, 0xc0, 0x07, 0xe0, 0x0f, 0xf0, 0x1f, 0xf8, 0x3f,
	0xfc, 0x7f, 0xc0, 0x07, 0xc0, 0x07, 0xc0, 0x07, 0xfc, 0x7f, 0xf8, 0x3f,
	0xf0, 0x1f, 0xe0, 0x0f, 0xc0, 0x07, 0x80, 0x03 };
    static uchar cur_hor_bits[] = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x08, 0x30, 0x18,
	0x38, 0x38, 0xfc, 0x7f, 0xfc, 0x7f, 0x38, 0x38, 0x30, 0x18, 0x20, 0x08,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
    static uchar mcur_hor_bits[] = {
	0x00, 0x00, 0x00, 0x00, 0x40, 0x04, 0x60, 0x0c, 0x70, 0x1c, 0x78, 0x3c,
	0xfc, 0x7f, 0xfe, 0xff, 0xfe, 0xff, 0xfe, 0xff, 0xfc, 0x7f, 0x78, 0x3c,
	0x70, 0x1c, 0x60, 0x0c, 0x40, 0x04, 0x00, 0x00 };
    static uchar cur_bdiag_bits[] = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0x00, 0x3e, 0x00, 0x3c, 0x00, 0x3e,
	0x00, 0x37, 0x88, 0x23, 0xd8, 0x01, 0xf8, 0x00, 0x78, 0x00, 0xf8, 0x00,
	0xf8, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
    static uchar mcur_bdiag_bits[] = {
	0x00, 0x00, 0xc0, 0x7f, 0x80, 0x7f, 0x00, 0x7f, 0x00, 0x7e, 0x04, 0x7f,
	0x8c, 0x7f, 0xdc, 0x77, 0xfc, 0x63, 0xfc, 0x41, 0xfc, 0x00, 0xfc, 0x01,
	0xfc, 0x03, 0xfc, 0x07, 0x00, 0x00, 0x00, 0x00 };
    static uchar cur_fdiag_bits[] = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf8, 0x01, 0xf8, 0x00, 0x78, 0x00,
	0xf8, 0x00, 0xd8, 0x01, 0x88, 0x23, 0x00, 0x37, 0x00, 0x3e, 0x00, 0x3c,
	0x00, 0x3e, 0x00, 0x3f, 0x00, 0x00, 0x00, 0x00 };
    static uchar mcur_fdiag_bits[] = {
	0x00, 0x00, 0x00, 0x00, 0xfc, 0x07, 0xfc, 0x03, 0xfc, 0x01, 0xfc, 0x00,
	0xfc, 0x41, 0xfc, 0x63, 0xdc, 0x77, 0x8c, 0x7f, 0x04, 0x7f, 0x00, 0x7e,
	0x00, 0x7f, 0x80, 0x7f, 0xc0, 0x7f, 0x00, 0x00 };
    static uchar cur_blank_bits[] = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

    static uchar *cursor_bits[] = {
	cur_ver_bits, mcur_ver_bits, cur_hor_bits, mcur_hor_bits,
	cur_bdiag_bits, mcur_bdiag_bits, cur_fdiag_bits, mcur_fdiag_bits,
	0, 0, cur_blank_bits, cur_blank_bits };

    Display *dpy = qt_xdisplay();
    if ( d->cshape == BitmapCursor ) {
	XColor bg, fg;				// ignore stupid CFront message
	bg.red   = 255 << 8;
	bg.green = 255 << 8;
	bg.blue  = 255 << 8;
	fg.red   = 0;
	fg.green = 0;
	fg.blue  = 0;
	d->hcurs = XCreatePixmapCursor( dpy, d->bm->handle(), d->bmm->handle(),
					&fg, &bg, d->hx, d->hy );
	return;
    }
    if ( d->cshape >= SizeVerCursor && d->cshape < SizeAllCursor ||
	 d->cshape == BlankCursor ) {
	XColor bg, fg;				// ignore stupid CFront message
	bg.red   = 255 << 8;
	bg.green = 255 << 8;
	bg.blue  = 255 << 8;
	fg.red   = 0;
	fg.green = 0;
	fg.blue  = 0;
	int i = (d->cshape - SizeVerCursor)*2;
	Window rootwin = qt_xrootwin();
	d->pm  = XCreateBitmapFromData( dpy, rootwin, (char *)cursor_bits[i],
					16, 16 );
	d->pmm = XCreateBitmapFromData( dpy, rootwin, (char *)cursor_bits[i+1],
					16,16);
	d->hcurs = XCreatePixmapCursor( dpy, d->pm, d->pmm, &fg, &bg, 8, 8 );
	return;
    }
    uint sh;
    switch ( d->cshape ) {			// map Q cursor to X cursor
	case ArrowCursor:
	    sh = XC_left_ptr;
	    break;
	case UpArrowCursor:
	    sh = XC_center_ptr;
	    break;
	case CrossCursor:
	    sh = XC_crosshair;
	    break;
	case WaitCursor:
	    sh = XC_watch;
	    break;
	case IbeamCursor:
	    sh = XC_xterm;
	    break;
	case SizeAllCursor:
	    sh = XC_fleur;
	    break;
	default:
#if defined(CHECK_RANGE)
	    warning( "QCursor::update: Invalid cursor shape %d", d->cshape );
#endif
	    return;
    }
    d->hcurs = XCreateFontCursor( dpy, sh );
}
