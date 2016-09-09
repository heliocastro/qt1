/****************************************************************************
** $Id: qclipboard_x11.cpp,v 2.17.2.2 1998/12/07 16:25:07 warwick Exp $
**
** Implementation of QClipboard class for X11
**
** Created : 960430
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

#include "qclipboard.h"
#include "qapplication.h"
#include "qpixmap.h"
#include "qdatetime.h"
#define	 GC GC_QQQ
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include <X11/Xatom.h>

/*****************************************************************************
  Internal QClipboard functions for X11.
 *****************************************************************************/

extern Time qt_x_clipboardtime;			// def. in qapplication_x11.cpp
extern Atom qt_selection_property;


static QWidget * owner = 0;
static QByteArray * buf = 0;

static void cleanup() {
    // ### when qapp stops deleting no-parent widgets, we must delete owner
    owner = 0;
    delete buf;
    buf = 0;
}

static
void setupOwner()
{
    if ( owner )
	return;
    owner = new QWidget( 0, "internal clibpoard owner" );
    buf = new QByteArray;
    qAddPostRoutine( cleanup );
}


enum ClipboardFormat { CFNothing, CFText, CFPixmap };

static ClipboardFormat getFormat( const char *format )
{
    if ( strcmp(format,"TEXT") == 0 )
	 return CFText;
    else if ( strcmp(format,"PIXMAP") == 0 )
	return CFPixmap;
    return CFNothing;
}

class QClipboardData
{
public:
    QClipboardData();
   ~QClipboardData();

    ClipboardFormat	format() const;

    void	       *data( const char *format ) const;
    void		setData( const char *format, void * );

    char	       *text() const;
    void		setText( const char * );
    QPixmap	       *pixmap() const;
    void		setPixmap( QPixmap * );

    void		clear();

private:
    ClipboardFormat	f;
    QString		t;
    QPixmap	       *p;

};

QClipboardData::QClipboardData()
{
    f = CFNothing;
    p = 0;
}

QClipboardData::~QClipboardData()
{
    delete p;
}

inline ClipboardFormat QClipboardData::format() const
{
    return f;
}

inline char *QClipboardData::text() const
{
    return t.data();
}

inline void QClipboardData::setText( const char *text )
{
    t = text;
    f = CFText;
}

inline QPixmap *QClipboardData::pixmap() const
{
    return p;
}

inline void QClipboardData::setPixmap( QPixmap *pixmap )
{
    if ( p )
	delete p;
    p = new QPixmap( *pixmap );
    f = CFPixmap;
}

void *QClipboardData::data( const char *format ) const
{
    switch ( getFormat(format) ) {
	case CFText:
	    return text();
	case CFPixmap:
	    return pixmap();
	default:
	    return 0;
    }
}

void QClipboardData::setData( const char *format, void *data )
{
    switch ( getFormat(format) ) {
	case CFText:
	    setText( (const char *)data );
	    break;
	case CFPixmap:
	    setPixmap( (QPixmap *)data );
	    break;
	default:
	    break;
    }
}

void QClipboardData::clear()
{
    t.resize( 0 );
    delete p;
    p = 0;
    f = CFNothing;
}


static QClipboardData *internalCbData;

static void cleanupClipboardData()
{
    delete internalCbData;
}

static QClipboardData *clipboardData()
{
    if ( internalCbData == 0 ) {
	internalCbData = new QClipboardData;
	CHECK_PTR( internalCbData );
	qAddPostRoutine( cleanupClipboardData );
    }
    return internalCbData;
}


/*****************************************************************************
  QClipboard member functions for X11.
 *****************************************************************************/

/*!
  Clears the clipboard contents.
*/

void QClipboard::clear()
{
    setData( "TEXT", 0 );
}


bool qt_xclb_wait_for_event( Display *dpy, Window win, int type, XEvent *event,
			     int timeout )
{
    QTime started = QTime::currentTime();
    QTime now = started;
    do {
	if ( XCheckTypedWindowEvent(dpy,win,type,event) )
	    return TRUE;
	now = QTime::currentTime();
	if ( started > now )			// crossed midnight
	    started = now;
	XSync( dpy, FALSE );			// toss a ball while we wait
    } while ( started.msecsTo(now) < timeout );
    return FALSE;
}


static inline int maxSelectionIncr( Display *dpy )
{
    return XMaxRequestSize(dpy) > 65536 ?
	4*65536 : XMaxRequestSize(dpy)*4 - 100;
}


// uglehack: externed into qt_xdnd.cpp.  qt is really not designed for
// single-platform, multi-purpose blocks of code...
bool qt_xclb_read_property( Display *dpy, Window win, Atom property,
			   bool deleteProperty,
			   QByteArray *buffer, int *size, Atom *type,
			   int *format, bool nullterm )
{
    int	   maxsize = maxSelectionIncr(dpy);
    ulong  bytes_left;
    ulong  length;
    uchar *data;
    Atom   dummy_type;
    int    dummy_format;
    int    r;

    if ( !type )				// allow null args
	type = &dummy_type;
    if ( !format )
	format = &dummy_format;

    // Don't read anything, just get the size of the property data
    r = XGetWindowProperty( dpy, win, property, 0, 0, FALSE,
			    AnyPropertyType, type, format,
			    &length, &bytes_left, &data );
    if ( r != Success ) {
	buffer->resize( 0 );
	return FALSE;
    }
    XFree( (char*)data );

    int  offset = 0;
    bool ok = buffer->resize( (int)bytes_left+ (nullterm?1:0) );

    if ( ok ) {					// could allocate buffer
	while ( bytes_left ) {			// more to read...
	    r = XGetWindowProperty( dpy, win, property, offset/4, maxsize/4,
				    FALSE, AnyPropertyType, type, format,
				    &length, &bytes_left, &data );
	    if ( r != Success )
		break;
	    length *= *format/8;		// length in bytes
	    // Here we check if we get a buffer overflow and tries to
	    // recover -- this shouldn't normally happen, but it doesn't
	    // hurt to be defensive
	    if ( offset+length > buffer->size() ) {
		length = buffer->size() - offset;
		bytes_left = 0;			// escape loop
	    }
	    memcpy( buffer->data()+offset, data, (unsigned int)length );
	    offset += (unsigned int)length;
	    XFree( (char*)data );
	}
	if (nullterm)
	    buffer->at(offset) = '\0';		// zero-terminate (for text)
    }
    if ( size )
	*size = offset;				// correct size, not 0-term.
    XFlush( dpy );
    if ( deleteProperty ) {
	XDeleteProperty( dpy, win, property );
	XFlush( dpy );
    }
    return ok;
}


// this is externed into qt_xdnd.cpp too.
QByteArray qt_xclb_read_incremental_property( Display *dpy, Window win,
					      Atom property, int nbytes,
					      bool nullterm )
{
    XEvent event;

    QByteArray buf;
    QByteArray tmp_buf;
    bool alloc_error = FALSE;
    int  length;
    int  offset = 0;

    XWindowAttributes wa;
    XGetWindowAttributes( dpy, win, &wa );
    // Change the event mask for the window, it will be restored before
    // this function ends
    XSelectInput( dpy, win, PropertyChangeMask);

    if ( nbytes > 0 ) {
	// Reserve buffer + zero-terminator (for text data)
	// We want to complete the INCR transfer even if we cannot
	// allocate more memory
	alloc_error = !buf.resize(nbytes+1);
    }

    while ( TRUE ) {
	if ( !qt_xclb_wait_for_event(dpy,win,PropertyNotify,
				     (XEvent*)&event,5000) )
	    break;
	XFlush( dpy );
	if ( event.xproperty.atom != property ||
	     event.xproperty.state != PropertyNewValue )
	    continue;
	if ( qt_xclb_read_property(dpy, win, property, TRUE, &tmp_buf,
					&length,0, 0, nullterm) ) {
	    if ( length == 0 ) {		// no more data, we're done
		buf.at( offset ) = '\0';
		buf.resize( offset+1 );
		break;
	    } else if ( !alloc_error ) {
		if ( offset+length > (int)buf.size() ) {
		    if ( !buf.resize(offset+length+65535) ) {
			alloc_error = TRUE;
			length = buf.size() - offset;
		    }
		}
		memcpy( buf.data()+offset, tmp_buf.data(), length );
		tmp_buf.resize( 0 );
		offset += length;
	    }
	} else {
	    break;
	}
    }
    // Restore the event mask
    XSelectInput( dpy, win, wa.your_event_mask & ~PropertyChangeMask );
    return buf;
}


/*!
  Returns a pointer to the clipboard data, where \e format is the clipboard
  format.

  We recommend that you use text() or pixmap() instead.
*/

void *QClipboard::data( const char *format ) const
{
    ClipboardFormat f = getFormat( format );
    switch ( f ) {
	case CFText:
	    break;				// text is ok
	case CFPixmap:
#if defined(CHECK_RANGE)
	    warning( "QClipboard::data: PIXMAP format not supported" );
#endif
	    return 0;
	default:
#if defined(CHECK_RANGE)
	    warning( "QClipboard::data: Unknown format: %s", format );
#endif
	    return 0;
    }

    QClipboardData *d = clipboardData();
    setupOwner();
    Window   win   = owner->winId();
    Display *dpy   = owner->x11Display();

    if ( d->format() != CFNothing ) {		// we own the clipboard
	ASSERT( XGetSelectionOwner(dpy,XA_PRIMARY) == win );
	return d->data(format);
    }

    if ( XGetSelectionOwner(dpy,XA_PRIMARY) == None )
	return 0;

    XConvertSelection( dpy, XA_PRIMARY, XA_STRING, qt_selection_property, win,
		       CurrentTime );
    XFlush( dpy );

    XEvent xevent;
    if ( !qt_xclb_wait_for_event(dpy,win,SelectionNotify,&xevent,5000) )
	return 0;

    Atom   type;

    if ( qt_xclb_read_property(dpy,win,qt_selection_property,TRUE,
			       buf,0,&type,0,TRUE) ) {
	if ( type == XInternAtom(dpy,"INCR",FALSE) ) {
	    int nbytes = buf->size() >= 4 ? *((int*)buf->data()) : 0;
	    *buf = qt_xclb_read_incremental_property( dpy, win,
						      qt_selection_property,
						      nbytes, TRUE );
	} else if ( type != XA_STRING ) {
#if 0
	    // For debugging
	    char *n = XGetAtomName( dpy, type );
	    debug( "Qt clipboard: unknown atom = %s",n);
	    XFree( n );
#endif
	}
    }
    return buf->data();
}


/*!
  Copies text into the clipboard, where \e format is the clipboard format
  string and \e data is the data to be copied.

  We recommend that you use setText() or setPixmap() instead.
*/

void QClipboard::setData( const char *format, void *data )
{
    ClipboardFormat f = getFormat( format );
    switch ( f ) {
	case CFText:
	    break;
	case CFPixmap:
#if defined(CHECK_RANGE)
	    warning( "QClipboard::setData: PIXMAP format not supported" );
#endif
	    return;
	default:
#if defined(CHECK_RANGE)
	    warning( "QClipboard::setData: Unknown format: %s", format );
#endif
	    return;
    }

    QClipboardData *d = clipboardData();
    setupOwner();
    Window   win   = owner->winId();
    Display *dpy   = owner->x11Display();

    if ( d->format() != CFNothing ) {		// we own the clipboard
#if defined(DEBUG)
	ASSERT( XGetSelectionOwner(dpy,XA_PRIMARY) == win );
#endif
	d->setData( format, data );
	emit dataChanged();
	return;
    }

    d->clear();
    d->setData( format, data );
    emit dataChanged();

    XSetSelectionOwner( dpy, XA_PRIMARY, win, qt_x_clipboardtime );
    if ( XGetSelectionOwner(dpy,XA_PRIMARY) != win ) {
#if defined(DEBUG)
	warning( "QClipboard::setData: Cannot set X11 selection owner" );
#endif
	return;
    }
}


/*!
  \internal
  Internal cleanup for Windows.
*/

void QClipboard::ownerDestroyed()
{
}


/*!
  \internal
  Internal optimization for Windows.
*/

void QClipboard::connectNotify( const char * )
{
}


/*!
  Handles clipboard events (very platform-specific).
*/

bool QClipboard::event( QEvent *e )
{
    if ( e->type() != Event_Clipboard )
	return QObject::event( e );

    XEvent *xevent = (XEvent *)Q_CUSTOM_EVENT(e)->data();
    Display *dpy = qt_xdisplay();
    QClipboardData *d = clipboardData();

    switch ( xevent->type ) {

	case SelectionClear:			// new selection owner
	    clipboardData()->clear();
	    emit dataChanged();
	    break;

	case SelectionNotify:
	    clipboardData()->clear();
	    break;

	case SelectionRequest: {		// someone wants our data
	    XSelectionRequestEvent *req = &xevent->xselectionrequest;
	    XEvent evt;
	    evt.xselection.type = SelectionNotify;
	    evt.xselection.display	= req->display;
	    evt.xselection.requestor	= req->requestor;
	    evt.xselection.selection	= req->selection;
	    evt.xselection.target	= req->target;
	    evt.xselection.property	= None;
	    evt.xselection.time = req->time;
	    if ( req->target == XA_STRING ) {
		XChangeProperty ( dpy, req->requestor, req->property,
				  XA_STRING, 8,
				  PropModeReplace,
				  (uchar *)d->text(), strlen(d->text()) );
		evt.xselection.property = req->property;
	    }
	    XSendEvent( dpy, req->requestor, False, 0, &evt );
	    }
	    break;
    }

    return TRUE;
}
