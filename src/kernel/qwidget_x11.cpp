/****************************************************************************
** $Id: qwidget_x11.cpp,v 2.100.2.9 1999/01/18 11:22:40 aavit Exp $
**
** Implementation of QWidget and QWindow classes for X11
**
** Created : 931031
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

#include "qwindow.h"
#include "qapplication.h"
#include "qpaintdevicedefs.h"
#include "qpainter.h"
#include "qbitmap.h"
#include "qwidgetlist.h"
#include "qwidgetintdict.h"
#include "qobjectlist.h"
#include "qobjectdict.h"
#include "qaccel.h"
#include "qdragobject.h"
#include "qfocusdata.h"
#define	 GC GC_QQQ
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include <X11/Xatom.h>

//#define NO_SHAPE
#ifdef NO_SHAPE
#define XShapeCombineRegion(a,b,c,d,e,f,g)
#define XShapeCombineMask(a,b,c,d,e,f,g)
#else
#include <X11/extensions/shape.h>
#endif

#if !defined(XlibSpecificationRelease)
#define X11R4
typedef char *XPointer;
#else
#undef  X11R4
#endif

void qt_enter_modal( QWidget * );		// defined in qapplication_x11.cpp
void qt_leave_modal( QWidget * );		// --- "" ---
bool qt_modal_state();				// --- "" ---
void qt_open_popup( QWidget * );		// --- "" ---
void qt_close_popup( QWidget * );		// --- "" ---
void qt_insert_sip( QWidget*, int, int );	// --- "" ---
int  qt_sip_count( QWidget* );			// --- "" ---
void qt_updated_rootinfo();


extern bool qt_nograb();
extern QWidget *qt_button_down;

static QWidget *mouseGrb    = 0;
static QWidget *keyboardGrb = 0;


/*****************************************************************************
  QWidget member functions
 *****************************************************************************/

#if QT_VERSION == 200
#error "Make create and destroy virtual, remove the old functions."
#endif

extern Atom qt_wm_delete_window;		// defined in qapplication_x11.cpp
extern Atom qt_sizegrip;			// defined in qapplication_x11.cpp

const uint stdWidgetEventMask =			// X event mask
	(uint)(
	    KeyPressMask | KeyReleaseMask |
	    ButtonPressMask | ButtonReleaseMask |
	    KeymapStateMask |
	    ButtonMotionMask |
	    EnterWindowMask | LeaveWindowMask |
	    FocusChangeMask |
	    ExposureMask |
	    StructureNotifyMask | SubstructureRedirectMask
	);

const uint stdDesktopEventMask =			// X event mask
	(uint)(
	    KeyPressMask | KeyReleaseMask |
	    KeymapStateMask |
	    EnterWindowMask | LeaveWindowMask |
	    FocusChangeMask | PropertyChangeMask
	);


/*
  The qt_ functions below are implemented in qwidgetcreate_x11.cpp.
*/

Window qt_XCreateWindow( const QWidget *creator,
			 Display *display, Window parent,
			 int x, int y, uint w, uint h,
			 int borderwidth, int depth,
			 uint windowclass, Visual *visual,
			 ulong valuemask, XSetWindowAttributes *attributes );
Window qt_XCreateSimpleWindow( const QWidget *creator,
			       Display *display, Window parent,
			       int x, int y, uint w, uint h, int borderwidth,
			       ulong border, ulong background );
void qt_XDestroyWindow( const QWidget *destroyer,
			Display *display, Window window );



/*!
  Creates a new widget window if \a window is null, otherwise sets the
  widget's window to \a window.

  Initializes the window (sets the geometry etc.) if \a initializeWindow
  is TRUE.  If \a initializeWindow is FALSE, no initialization is
  performed.  This parameter makes only sense if \a window is a valid
  window.

  Destroys the old window if \a destroyOldWindow is TRUE.  If \a
  destroyOldWindow is FALSE, you are responsible for destroying
  the window yourself (using platform native code).

  The QWidget constructor calls create(0,TRUE,TRUE) to create a window for
  this widget.
*/

void QWidget::create( WId window, bool initializeWindow, bool destroyOldWindow)
{
    if ( testWFlags(WState_Created) && window == 0 )
	return;
    setWFlags( WState_Created );		// set created flag

    if ( !parentWidget() )
	setWFlags( WType_TopLevel );		// top-level widget

    static int sw = -1, sh = -1;		// screen size

    int	   scr	    = qt_xscreen();
    bool   topLevel = testWFlags(WType_TopLevel);
    bool   popup    = testWFlags(WType_Popup);
    bool   modal    = testWFlags(WType_Modal);
    bool   desktop  = testWFlags(WType_Desktop);
    Window root_win = RootWindow(dpy,scr);
    Window parentw, destroyw = 0;
    WId	   id;

    if ( !window )				// always initialize
	initializeWindow = TRUE;

    if ( popup )				// a popup is a tool window
	setWFlags(WStyle_Tool);

    if ( sw < 0 ) {				// get the screen size
	sw = DisplayWidth(dpy,scr);
	sh = DisplayHeight(dpy,scr);
    }

    bg_col = pal.normal().background();		// default background color

    if ( modal || popup || desktop ) {		// these are top-level, too
	topLevel = TRUE;
	setWFlags( WType_TopLevel );
    }

    if ( desktop ) {				// desktop widget
	modal = popup = FALSE;			// force these flags off
	frect.setRect( 0, 0, sw, sh );
    } else if ( topLevel ) {			// calc pos/size from screen
	frect.setRect( sw/4, 3*sh/10, sw/2, 4*sh/10 );
    } else {					// child widget
	frect.setRect( 0, 0, 100, 30 );
    }
    crect = frect;				// default client rect

    parentw = topLevel ? root_win : parentWidget()->winId();

    XSetWindowAttributes wsa;

    if ( window ) {				// override the old window
	if ( destroyOldWindow )
	    destroyw = winid;
	id = window;
	setWinId( window );
    } else if ( desktop ) {			// desktop widget
	id = (WId)parentw;			// id = root window
	QWidget *otherDesktop = find( id );	// is there another desktop?
	if ( otherDesktop && otherDesktop->testWFlags(WPaintDesktop) ) {
	    otherDesktop->setWinId( 0 );	// remove id from widget mapper
	    setWinId( id );			// make sure otherDesktop is
	    otherDesktop->setWinId( id );	//   found first
	} else {
	    setWinId( id );
	}
    } else {
	if ( x11DefaultVisual() && x11DefaultColormap() ) {
	    id = (WId)qt_XCreateSimpleWindow( this, dpy, parentw,
					 frect.left(), frect.top(),
					 frect.width(), frect.height(),
					 0,
					 black.pixel(),
					 bg_col.pixel() );
	} else {
	    wsa.background_pixel = bg_col.pixel();
	    wsa.border_pixel = black.pixel();		
	    wsa.colormap = (Colormap)x11Colormap();
	    id = (WId)qt_XCreateWindow( this, dpy, parentw,
				   frect.left(), frect.top(),
				   frect.width(), frect.height(),
				   0, x11Depth(), InputOutput,
				   (Visual*)x11Visual(),
				   CWBackPixel|CWBorderPixel|CWColormap,
				   &wsa );
	}
	setWinId( id );				// set widget id/handle + hd
    }

    if ( topLevel && !(desktop || popup || modal) ) {
	if ( testWFlags(WStyle_Customize) ) {	// customize top-level widget
	    ulong wsa_mask = 0;
	    if ( testWFlags(WStyle_NormalBorder) ) {
		;				// ok, we already have it
	    } else {
		if ( testWFlags(WStyle_DialogBorder) && initializeWindow ) {
		    XSetTransientForHint( dpy, id, root_win );
		} else {			// no border
		    wsa.override_redirect = TRUE;
		    wsa_mask |= CWOverrideRedirect;
		}
	    }
	    if ( testWFlags(WStyle_Tool) ) {
		wsa.save_under = TRUE;
		wsa_mask |= CWSaveUnder;
	    }
	    if ( wsa_mask && initializeWindow )
		XChangeWindowAttributes( dpy, id, wsa_mask, &wsa );
	} else {				// normal top-level widget
	    setWFlags( WStyle_NormalBorder | WStyle_Title | WStyle_SysMenu |
		       WStyle_MinMax );
	}
    }

    if ( !initializeWindow ) {
	// do no initialization
    } else if ( popup ) {			// popup widget
	XSetTransientForHint( dpy, id, parentw );
	wsa.override_redirect = TRUE;
	wsa.save_under = TRUE;
	XChangeWindowAttributes( dpy, id, CWOverrideRedirect | CWSaveUnder,
				 &wsa );
    } else if ( topLevel && !desktop ) {	// top-level widget
	if ( modal ) {
	    QWidget *p = parentWidget();	// real parent
	    QWidget *pp = p ? p->parentWidget() : 0;
	    while ( pp && !pp->testWFlags(WType_Modal) ) {
		p = pp;				// find real parent
		pp = pp->parentWidget();
	    }
	    if ( p && p->isVisible() )		// modal to one widget
		XSetTransientForHint( dpy, id, p->winId() );
	    else				// application-modal
		XSetTransientForHint( dpy, id, root_win );
	}
	XSizeHints size_hints;
	size_hints.flags = PPosition | PSize | PWinGravity;
	size_hints.x = crect.left();
	size_hints.y = crect.top();
	size_hints.width = crect.width();
	size_hints.height = crect.height();
	size_hints.win_gravity = 1;		// NortWest
	char *title = qAppName();
	XWMHints wm_hints;			// window manager hints
	wm_hints.input = True;
	wm_hints.initial_state = NormalState;
	wm_hints.flags = InputHint | StateHint;
	XClassHint class_hint;
	class_hint.res_name = title;		// app name and widget name
	class_hint.res_class = name() ? (char *)name() : title;
	XSetWMProperties( dpy, id, 0, 0, 0, 0, &size_hints, &wm_hints,
			  &class_hint );
	XResizeWindow( dpy, id, crect.width(), crect.height() );
	XStoreName( dpy, id, title );
	Atom protocols[1];
	protocols[0] = qt_wm_delete_window;	// support del window protocol
	XSetWMProtocols( dpy, id, protocols, 1 );
    }

    if ( initializeWindow ) {
//     if ( testWFlags(WResizeNoErase) && initializeWindow ) {
	wsa.bit_gravity = NorthWestGravity;	// don't erase when resizing
	XChangeWindowAttributes( dpy, id, CWBitGravity, &wsa );
    }

    setWFlags( WState_TrackMouse );
    setMouseTracking( FALSE );			// also sets event mask
    if ( desktop ) {
	setWFlags( WState_Visible );
    } else if ( topLevel ) {			// set X cursor
	QCursor *oc = QApplication::overrideCursor();
	if ( initializeWindow )
	    XDefineCursor( dpy, winid, oc ? oc->handle() : curs.handle() );
	setWFlags( WCursorSet );
    }

    if ( window ) {				// got window from outside
	XWindowAttributes a;
	XGetWindowAttributes( dpy, window, &a );
	frect.setRect( a.x, a.y, a.width, a.height );
	crect = frect;
	if ( a.map_state == IsUnmapped )
	    clearWFlags( WState_Visible );
	else
	    setWFlags( WState_Visible );
	if ( a.depth != x11Depth() )		// multi-depth system
	     devFlags |= PDF_OWNDEPTH;
    }
    else {
	devFlags &= ~PDF_OWNDEPTH;		// clear (if recreating)
    }

    if ( destroyw )
	qt_XDestroyWindow( this, dpy, destroyw );
}


void QWidget::setSizeGrip(bool sizegrip){
    createExtra();
    WId	   id = winId();
    if (extra->sizegrip != sizegrip) {
	XChangeProperty(qt_xdisplay(), topLevelWidget()->winId(),
			qt_sizegrip, XA_WINDOW, 32, PropModeReplace,
			sizegrip?((unsigned char *)&id):(unsigned char *)None,
			1);
    }
}

#if QT_VERSION == 200
#error "Cleanup here"
//
// Remove create(window) and create().  Return void.
// Default arguments: (window=0, destroyW=TRUE)
#endif


/*!
  \internal
  Creates the widget's window. Equivalent with create(window,TRUE).
  This function is usually called from the QWidget constructor.
*/

void QWidget::create( WId window )
{
    create( window, TRUE, TRUE );
}


/*!
  \internal
  Creates the widget's window. Equivalent with create(0,TRUE).
  This function is usually called from the QWidget constructor.
*/

bool QWidget::create()
{
    create( 0, TRUE, TRUE );
    return TRUE;
}


/*!
  Frees up window system resources.
  Destroys the widget window if \a destroyWindow is TRUE.

  destroy() calls itself recursively for all the child widgets,
  passing \a destroySubWindows for the \a destroyWindow parameter.
  To have more control over destruction of subwidgets,
  destroy subwidgets selectively first.

  This function is usually called from the QWidget destructor.
*/

void QWidget::destroy( bool destroyWindow, bool destroySubWindows )
{
    if ( qt_button_down == this )
	qt_button_down = 0;

    if ( testWFlags(WState_Created) ) {
	clearWFlags( WState_Created );
	if ( children() ) {
	    QObjectListIt it(*children());
	    register QObject *obj;
	    while ( (obj=it.current()) ) {	// destroy all widget children
		++it;
		if ( obj->isWidgetType() )
		    ((QWidget*)obj)->destroy(destroySubWindows,
					     destroySubWindows);
	    }
	}
	if ( mouseGrb == this )
	    releaseMouse();
	if ( keyboardGrb == this )
	    releaseKeyboard();
	if ( extra && extra->xic ) {
#if !defined(X11R4)
	    XDestroyIC( (XIC)extra->xic );
	    extra->xic = 0;
#endif
	}
	if ( testWFlags(WType_Modal) )		// just be sure we leave modal
	    qt_leave_modal( this );
	else if ( testWFlags(WType_Popup) )
	    qt_close_popup( this );
	if ( destroyWindow && !testWFlags(WType_Desktop) )
	    qt_XDestroyWindow( this, dpy, winid );
	setWinId( 0 );
    }
}


/*!
  \internal
  Destroys the widget's window and frees up window system resources.
  Equivalent with destroy(TRUE).
  This function is usually called from the QWidget destructor.
*/

bool QWidget::destroy()
{
    destroy( TRUE, TRUE );
    return TRUE;
}


#if QT_VERSION == 200
#error "Rename recreate as reparent"
#endif

/*!
  Reparents the widget.  The widget gets a new \a parent, new widget
  flags (\a f, but as usual, use 0) at a new position in its new
  parent (\a p).

  If \a showIt is TRUE, show() is called once the widget has been
  recreated.

  If the new parent widget is in a different top-level widget, the
  reparented widget and its children are appended to the end of the
  \link setFocusPolicy() TAB chain \endlink of the new parent widget,
  in the same internal order as before.  If one of the moved widgets
  had keyboard focus, recreate() calls clearFocus() for that widget.

  If the new parent widget is in the same top-level widget as the old
  parent, recreate doesn't change the TAB order or keyboard focus.

  \sa getWFlags()
*/

void QWidget::recreate( QWidget *parent, WFlags f, const QPoint &p,
			bool showIt )
{
    extern void qPRCreate( const QWidget *, Window );
    WId old_winid = winid;
    if ( testWFlags(WType_Desktop) )
	old_winid = 0;
    setWinId( 0 );

    reparentFocusWidgets( parent );		// fix focus chains

    if ( parentObj ) {				// remove from parent
	QChildEvent e( Event_ChildRemoved, this );
	QApplication::sendEvent( parentObj, &e );
	parentObj->removeChild( this );
    }
    if ( parent ) {				// insert into new parent
	parentObj = parent;			// avoid insertChild warning
	parent->insertChild( this );
    }
    bool     enable = isEnabled();		// remember status
    QSize    s	    = size();
    QPixmap *bgp    = (QPixmap *)backgroundPixmap();
    QColor   bgc    = bg_col;			// save colors
    const char* capt= caption();
    flags = f;
    clearWFlags( WState_Created | WState_Visible );
    create();
    const QObjectList *chlist = children();
    if ( chlist ) {				// reparent children
	QObjectListIt it( *chlist );
	QObject *obj;
	while ( (obj=it.current()) ) {
	    if ( obj->isWidgetType() ) {
		QWidget *w = (QWidget *)obj;
		XReparentWindow( dpy, w->winId(), winId(), w->geometry().x(),
				 w->geometry().y() );
	    }
	    ++it;
	}
    }
    qPRCreate( this, old_winid );
    if ( bgp )
	XSetWindowBackgroundPixmap( dpy, winid, bgp->handle() );
    else
	XSetWindowBackground( dpy, winid, bgc.pixel() );
    setGeometry( p.x(), p.y(), s.width(), s.height() );
    setEnabled( enable );
    if ( capt ) {
	extra->caption = 0;
	setCaption( capt );
    }
    if ( showIt )
	show();
    if ( old_winid )
	qt_XDestroyWindow( this, dpy, old_winid );

    QObjectList	*accelerators = queryList( "QAccel" );
    QObjectListIt it( *accelerators );
    QObject *obj;
    while ( (obj=it.current()) != 0 ) {
	++it;
	((QAccel*)obj)->repairEventFilter();
    }
    delete accelerators;
    if ( parent ) {
	QChildEvent *e = new QChildEvent( Event_ChildInserted, this );
	QApplication::postEvent( parent, e );
    } else {
	QFocusData *fd = focusData( TRUE );
	if ( fd->focusWidgets.findRef(this) < 0 )
 	    fd->focusWidgets.append( this );
    }
}


/*!
  Translates the widget coordinate \e pos to global screen coordinates.
  For example, \code mapToGlobal(QPoint(0,0))\endcode would give the
  global coordinates of the top-left pixel of the widget.
  \sa mapFromGlobal()
*/

QPoint QWidget::mapToGlobal( const QPoint &pos ) const
{
    int	   x, y;
    Window child;
    XTranslateCoordinates( dpy, winid, QApplication::desktop()->winId(),
			   pos.x(), pos.y(), &x, &y, &child );
    return QPoint( x, y );
}

/*!
  Translates the global screen coordinate \e pos to widget coordinates.
  \sa mapToGlobal()
*/

QPoint QWidget::mapFromGlobal( const QPoint &pos ) const
{
    int	   x, y;
    Window child;
    XTranslateCoordinates( dpy, QApplication::desktop()->winId(), winid,
			   pos.x(), pos.y(), &x, &y, &child );
    return QPoint( x, y );
}


// Please do NOT remove the FAQ answer from this doc again.  It's a
// FAQ, it remains a FAQ, and people apparently will not follow three
// links to find the right answer.

/*!
  This function is deprecated.  Use setBackgroundMode() or setPalette(),
  as they ensure the appropriate clearing color is used when the widget
  is in the Active, Normal, or Disabled state.

  If you want to change the color scheme of a widget, the setPalette()
  function is better suited.  Here is how to set \e thatWidget to use a
  light green (RGB value 80, 255, 80) as background color, with shades
  of green used for all the 3D effects:

  \code
    thatWidget->setPalette( QPalette( QColor(80, 255, 80) ) );
  \endcode

  \sa setPalette(), QApplication::setPalette(), backgroundColor(),
      setBackgroundPixmap(), setBackgroundMode()
*/

void QWidget::setBackgroundColor( const QColor &color )
{
    setBackgroundModeDirect( FixedColor );
    setBackgroundColorDirect( color );
}

void QWidget::setBackgroundColorDirect( const QColor &color )
{
    QColor old = bg_col;
    bg_col = color;
    XSetWindowBackground( dpy, winid, bg_col.pixel() );
    if ( extra && extra->bg_pix ) {		// kill the background pixmap
	delete extra->bg_pix;
	extra->bg_pix = 0;
    }
    backgroundColorChange( old );
}

static int allow_null_pixmaps = 0;

/*!
  Sets the background pixmap of the widget to \e pixmap.

  The background pixmap is tiled.  Some widgets (e.g. QLineEdit) do
  not work well with a background pixmap.

  \sa backgroundPixmap(), backgroundPixmapChange(), setBackgroundColor()

  \internal
  This function is call with a null pixmap by setBackgroundEmpty().
*/

void QWidget::setBackgroundPixmap( const QPixmap &pixmap )
{
    QPixmap old;
    if ( extra && extra->bg_pix )
	old = *extra->bg_pix;
    if ( !allow_null_pixmaps && pixmap.isNull() ) {
	XSetWindowBackground( dpy, winid, bg_col.pixel() );
	if ( extra && extra->bg_pix ) {
	    delete extra->bg_pix;
	    extra->bg_pix = 0;
	}
    } else {
	QPixmap pm = pixmap;
	if (!pm.isNull()) {
	    if ( pm.depth() == 1 && QPixmap::defaultDepth() > 1 ) {
		pm = QPixmap( pixmap.size() );
		bitBlt( &pm, 0, 0, &pixmap, 0, 0, pm.width(), pm.height() );
	    }
	}
	if ( extra && extra->bg_pix )
	    delete extra->bg_pix;
	else
	    createExtra();
	extra->bg_pix = new QPixmap( pm );
	XSetWindowBackgroundPixmap( dpy, winid, pm.handle() );
	if ( testWFlags(WType_Desktop) )	// save rootinfo later
	    qt_updated_rootinfo();
    }
    if ( !allow_null_pixmaps ) {
	setBackgroundModeDirect( FixedPixmap );
	backgroundPixmapChange( old );
    }
}


/*!
  Sets the window-system background of the widget to nothing.

  Note that `nothing' is actually a pixmap that isNull(), thus you
  can check for an empty background by checking backgroundPixmap().

  \sa setBackgroundPixmap(), setBackgroundColor()

  This class should \e NOT be made virtual - it is an alternate usage
  of setBackgroundPixmap().
*/
void QWidget::setBackgroundEmpty()
{
    allow_null_pixmaps++;
    setBackgroundPixmap(QPixmap());
    allow_null_pixmaps--;
}


/*!
  Sets the widget cursor shape to \e cursor.

  The mouse cursor will assume this shape when it's over this widget.
  See a list of predefined cursor objects with a range of useful
  shapes in the QCursor documentation.

  An editor widget would for example use an I-beam cursor:
  \code
    setCursor( ibeamCursor );
  \endcode

  \sa cursor(), QApplication::setOverrideCursor()
*/

void QWidget::setCursor( const QCursor &cursor )
{
    curs = cursor;
    QCursor *oc = QApplication::overrideCursor();
    XDefineCursor( dpy, winid, oc ? oc->handle() : curs.handle() );
    setWFlags( WCursorSet );
    XFlush( dpy );
}


/*!
  Sets the window caption (title).
  \sa caption(), setIcon(), setIconText()
*/

void QWidget::setCaption( const char *caption )
{
    if ( caption && extra && extra->caption &&
	 !strcmp( extra->caption, caption ) )
	return; // for less flicker
    if ( extra && extra->caption )
	delete [] extra->caption;
    else
	createExtra();
    extra->caption = qstrdup( caption );
    XStoreName( dpy, winId(), extra->caption );
}

/*!
  Sets the window icon pixmap.
  \sa icon(), setIconText(), setCaption()
*/

void QWidget::setIcon( const QPixmap &pixmap )
{
    if ( extra ) {
	delete extra->icon;
	extra->icon = 0;
    } else {
	createExtra();
    }
    Pixmap icon_pixmap;
    Pixmap mask_pixmap;
    QBitmap mask;
    if ( pixmap.isNull() ) {
	icon_pixmap = 0;
	mask_pixmap = 0;
    } else {
	extra->icon = new QPixmap( pixmap );
	icon_pixmap = pixmap.handle();
	mask = pixmap.mask() ? *pixmap.mask() : pixmap.createHeuristicMask();
	mask_pixmap = mask.handle();
    }
    XWMHints *h = XGetWMHints( dpy, winId() );
    XWMHints  wm_hints;
    bool got_hints = h != 0;
    if ( !got_hints ) {
	h = &wm_hints;
	h->flags = 0;
    }
    h->icon_pixmap = icon_pixmap;
    h->icon_mask   = mask_pixmap;
    h->flags |= IconPixmapHint | IconMaskHint;
    XSetWMHints( dpy, winId(), h );
    if ( got_hints )
	XFree( (char *)h );
}


/*!
  Sets the text of the window's icon to \e iconText.
  \sa iconText(), setIcon(), setCaption()
*/

void QWidget::setIconText( const char *iconText )
{
    if ( extra && extra->iconText )
	delete [] extra->iconText;
    else
	createExtra();
    extra->iconText = qstrdup( iconText );
    XSetIconName( dpy, winId(), extra->iconText );
}


void QWidget::setMouseTracking( bool enable )
{
    bool gmt = QApplication::hasGlobalMouseTracking();
    if ( enable == testWFlags(WState_TrackMouse) && !gmt )
	return;
    uint m = (enable || gmt) ? (uint)PointerMotionMask : 0;
    if ( enable )
	setWFlags( WState_TrackMouse );
    else
	clearWFlags( WState_TrackMouse );
    if ( testWFlags(WType_Desktop) ) {		// desktop widget?
	if ( testWFlags(WPaintDesktop) )	// get desktop paint events
	    XSelectInput( dpy, winid, stdDesktopEventMask|ExposureMask );
	else
	    XSelectInput( dpy, winid, stdDesktopEventMask );
    } else {
	XSelectInput( dpy, winid,		// specify events
		      m | stdWidgetEventMask );
    }
}


/*!
  Grabs the mouse input.

  This widget will be the only one to receive mouse events until
  releaseMouse() is called.

  \warning Grabbing the mouse might lock the terminal.

  It is almost never necessary to grab the mouse when using Qt since
  Qt grabs and releases it sensibly.  In particular, Qt grabs the
  mouse when a button is pressed and keeps it until the last button is
  released.

  \sa releaseMouse(), grabKeyboard(), releaseKeyboard()
*/

void QWidget::grabMouse()
{
    if ( !qt_nograb() ) {
	if ( mouseGrb )
	    mouseGrb->releaseMouse();
	XGrabPointer( dpy, winid, TRUE,
		      (uint)(ButtonPressMask | ButtonReleaseMask |
		             PointerMotionMask | EnterWindowMask | LeaveWindowMask),
		      GrabModeAsync, GrabModeAsync,
		      None, None, CurrentTime );
	mouseGrb = this;
    }
}

/*!
  Grabs the mouse intput and changes the cursor shape.

  The cursor will assume shape \e cursor (for as long as the mouse focus is
  grabbed) and this widget will be the only one to receive mouse events
  until releaseMouse() is called().

  \warning Grabbing the mouse might lock the terminal.

  \sa releaseMouse(), grabKeyboard(), releaseKeyboard(), setCursor()
*/

void QWidget::grabMouse( const QCursor &cursor )
{
    if ( !qt_nograb() ) {
	if ( mouseGrb )
	    mouseGrb->releaseMouse();
	XGrabPointer( dpy, winid, TRUE,
		      (uint)(ButtonPressMask | ButtonReleaseMask |
			     PointerMotionMask | EnterWindowMask | LeaveWindowMask),
		      GrabModeAsync, GrabModeAsync,
		      None, cursor.handle(), CurrentTime );
	mouseGrb = this;
    }
}

/*!
  Releases the mouse grab.

  \sa grabMouse(), grabKeyboard(), releaseKeyboard()
*/

void QWidget::releaseMouse()
{
    if ( !qt_nograb() && mouseGrb == this ) {
	XUngrabPointer( dpy, CurrentTime );
	XFlush( dpy );
	mouseGrb = 0;
    }
}

/*!
  Grabs all keyboard input.

  This widget will receive all keyboard events, independent of the active
  window.

  \warning Grabbing the keyboard might lock the terminal.

  \sa releaseKeyboard(), grabMouse(), releaseMouse()
*/

void QWidget::grabKeyboard()
{
    if ( !qt_nograb() ) {
	if ( keyboardGrb )
	    keyboardGrb->releaseKeyboard();
	XGrabKeyboard( dpy, winid, TRUE, GrabModeAsync, GrabModeAsync,
		       CurrentTime );
	keyboardGrb = this;
    }
}

/*!
  Releases the keyboard grab.

  \sa grabKeyboard(), grabMouse(), releaseMouse()
*/

void QWidget::releaseKeyboard()
{
    if ( !qt_nograb() && keyboardGrb == this ) {
	XUngrabKeyboard( dpy, CurrentTime );
	keyboardGrb = 0;
    }
}


/*!
  Returns a pointer to the widget that is currently grabbing the
  mouse input.

  If no widget in this application is currently grabbing the mouse, 0 is
  returned.

  \sa grabMouse(), keyboardGrabber()
*/

QWidget *QWidget::mouseGrabber()
{
    return mouseGrb;
}

/*!
  Returns a pointer to the widget that is currently grabbing the
  keyboard input.

  If no widget in this application is currently grabbing the keyboard, 0
  is returned.

  \sa grabMouse(), mouseGrabber()
*/

QWidget *QWidget::keyboardGrabber()
{
    return keyboardGrb;
}


/*!
  Returns TRUE if the top-level widget containing this widget is the
  active window.

  \sa setActiveWindow(), topLevelWidget()
*/

bool QWidget::isActiveWindow() const
{
    Window win;
    int revert;
    XGetInputFocus( dpy, &win, &revert );

    if ( win == None) return FALSE;

    QWidget *w = find( (WId)win );
    if ( w ) {
	// We know that window
	return w->topLevelWidget() == topLevelWidget();
    } else {
	// Window still may be a parent (if top-level is foreign window)
	Window root, parent;
	Window cursor = winId();
	Window *ch;
	unsigned int nch;
	while ( XQueryTree(dpy, cursor, &root, &parent, &ch, &nch) ) {
	    if (ch) XFree( (char*)ch);
	    if ( parent == win ) return TRUE;
	    if ( parent == root ) return FALSE;
	    cursor = parent;
	}
	return FALSE;
    }
}


/*!
  Sets the top-level widget containing this widget to be the active
  window.

  An active window is a top-level window that has the keyboard input
  focus.

  This function performs the same operation as clicking the mouse on
  the title bar of a top-level window.

  \sa isActiveWindow(), topLevelWidget()
*/

void QWidget::setActiveWindow()
{
    QWidget *tlw = topLevelWidget();
    if ( tlw->isVisible() )
	XSetInputFocus( dpy, tlw->winId(), RevertToNone, CurrentTime);
}


/*!
  Updates the widget unless updates are disabled or the widget is hidden.

  Updating the widget will erase the widget contents and generate a paint
  event from the window system. The paint event is processed after the
  program has returned to the main event loop.

  \sa repaint(), paintEvent(), setUpdatesEnabled(), erase()
*/

void QWidget::update()
{
     if ( (flags & (WState_Visible|WState_BlockUpdates)) == WState_Visible )
 	XClearArea( dpy, winid, 0, 0, 0, 0, TRUE );
}

/*!
  Updates a rectangle (\e x, \e y, \e w, \e h) inside the widget
  unless updates are disabled or the widget is hidden.

  Updating the widget erases the widget area \e (x,y,w,h), which in turn
  generates a paint event from the window system. The paint event is
  processed after the program has returned to the main event loop.

  If \e w is negative, it is replaced with <code>width() - x</code>.
  If \e h is negative, it is replaced width <code>height() - y</code>.

  \sa repaint(), paintEvent(), setUpdatesEnabled(), erase()
*/

void QWidget::update( int x, int y, int w, int h )
{
    if ( w && h &&
	 (flags & (WState_Visible|WState_BlockUpdates)) == WState_Visible ) {
	if ( w < 0 )
	    w = crect.width()  - x;
	if ( h < 0 )
	    h = crect.height() - y;
	if ( w != 0 && h != 0 )
	    XClearArea( dpy, winid, x, y, w, h, TRUE );
    }
}

/*!
  \overload void QWidget::update( const QRect &r )
*/

/*!
  \overload void QWidget::repaint( bool erase )

  This version repaints the entire widget.
*/

/*!
  Repaints the widget directly by calling paintEvent() directly,
  unless updates are disabled or the widget is hidden.

  Erases the widget area  \e (x,y,w,h) if \e erase is TRUE.

  If \e w is negative, it is replaced with <code>width() - x</code>.
  If \e h is negative, it is replaced width <code>height() - y</code>.

  Doing a repaint() usually is faster than doing an update(), but
  calling update() many times in a row will generate a single paint
  event.

  \warning If you call repaint() in a function which may itself be called
  from paintEvent(), you may see infinite recursion. The update() function
  never generates recursion.

  \sa update(), paintEvent(), setUpdatesEnabled(), erase()
*/

void QWidget::repaint( int x, int y, int w, int h, bool erase )
{
    if ( (flags & (WState_Visible|WState_BlockUpdates)) == WState_Visible ) {
	if ( w < 0 )
	    w = crect.width()  - x;
	if ( h < 0 )
	    h = crect.height() - y;
	QPaintEvent e( QRect(x,y,w,h) );
	if ( erase && w != 0 && h != 0 )
	    XClearArea( dpy, winid, x, y, w, h, FALSE );
	QApplication::sendEvent( this, &e );
    }
}

/*!
  \overload void QWidget::repaint( const QRect &r, bool erase )
*/


/*!
  \internal
  Platform-specific part of QWidget::show().
*/

void QWidget::showWindow()
{
    setWFlags( WState_Visible );
    clearWFlags( WState_DoHide );

    QShowEvent e(FALSE);
    QApplication::sendEvent( this, &e );

    XMapWindow( dpy, winId() );
}


/*!
  \internal
  Platform-specific part of QWidget::hide().
*/

void QWidget::hideWindow()
{
    if ( qt_button_down == this )
	qt_button_down = 0;
    XUnmapWindow( dpy, winId() );
    if ( isPopup() ) XFlush( dpy );
}


/*!
  Iconifies the widget.

  Calling this function has no effect for other than \link isTopLevel()
  top-level widgets\endlink.

  \sa show(), hide(), isVisible()
*/

void QWidget::iconify()
{
    if ( testWFlags(WType_TopLevel) )
	XIconifyWindow( dpy, winid, qt_xscreen() );
}


/*!
  Raises this widget to the top of the parent widget's stack.

  If there are any siblings of this widget that overlap it on the screen,
  this widget will be visually in front of its siblings afterwards.

  \sa lower()
*/

void QWidget::raise()
{
    QWidget *p = parentWidget();
    if ( p && p->childObjects && p->childObjects->findRef(this) >= 0 )
	p->childObjects->append( p->childObjects->take() );
    XRaiseWindow( dpy, winid );
}

/*!
  Lowers the widget to the bottom of the parent widget's stack.

  If there are siblings of this widget that overlap it on the screen, this
  widget will be obscured by its siblings afterwards.

  \sa raise()
*/

void QWidget::lower()
{
    QWidget *p = parentWidget();
    if ( p && p->childObjects && p->childObjects->findRef(this) >= 0 )
	p->childObjects->insert( 0, p->childObjects->take() );
    XLowerWindow( dpy, winid );
}


/*
  The global variable qwidget_tlw_gravity defines the window gravity of
  the next top level window to be created. We do this when setting the
  main widget's geometry and the "-geometry" command line option contains
  a negative position.
*/

int qwidget_tlw_gravity = 1;

static void do_size_hints( Display *dpy, WId winid, QWExtra *x, XSizeHints *s )
{
    if ( x ) {
	if ( x->minw > 0 || x->minh > 0 ) {	// add minimum size hints
	    s->flags |= PMinSize;
	    s->min_width  = x->minw;
	    s->min_height = x->minh;
	}
	if ( x->maxw < QCOORD_MAX || x->maxh < QCOORD_MAX ) {
	    s->flags |= PMaxSize;		// add maximum size hints
	    s->max_width  = x->maxw;
	    s->max_height = x->maxh;
	}
	if ( x->incw > 0 || x->inch > 0 ) {	// add resize increment hints
	    s->flags |= PResizeInc | PBaseSize;
	    s->width_inc = x->incw;
	    s->height_inc = x->inch;
	    s->base_width = 0;
	    s->base_height = 0;
	}
    }
    s->flags |= PWinGravity;
    s->win_gravity = qwidget_tlw_gravity;	// usually NorthWest (1)
    qwidget_tlw_gravity = 1;			// reset in case it was set
    XSetWMNormalHints( dpy, winid, s );
}


/*!
  \overload void QWidget::move( const QPoint & )
*/

/*!
  Moves the widget to the position \e (x,y) relative to the parent widget.

  A \link moveEvent() move event\endlink is generated immediately if
  the widget is visible. If the widget is invisible, the move event
  is generated when show() is called.

  This function is virtual, and all other overloaded move()
  implementations call it.

  \warning If you call move() or setGeometry() from moveEvent(), you
  may see infinite recursion.

  \sa pos(), resize(), setGeometry(), moveEvent()
*/

void QWidget::move( int x, int y )
{
    if ( testWFlags(WType_Desktop) )
	return;
    QPoint p(x,y);
    QPoint oldp = pos();
    if ( oldp == p )
	return;
    QRect  r = frect;
    r.moveTopLeft( p );
    setFRect( r );
    internalMove( x, y );
    if ( !isVisible() ) {
	deferMove( oldp );
    } else {
	cancelMove();
	QMoveEvent e( r.topLeft(), oldp );
	QApplication::sendEvent( this, &e );	// send move event immediately
    }
}


void QWidget::internalMove( int x, int y )
{
    if ( testWFlags(WType_TopLevel) ) {
	setWFlags(WConfigPending);
	XSizeHints size_hints;			// tell window manager
	size_hints.flags = PPosition;
	size_hints.x = x;
	size_hints.y = y;
	do_size_hints( dpy, winid, extra, &size_hints );
    }
    XMoveWindow( dpy, winid, x, y );
}


/*!
  \overload void QWidget::resize( const QSize & )
*/

/*!
  Resizes the widget to size \e w by \e h pixels.

  A \link resizeEvent() resize event\endlink is generated immediately if
  the widget is visible. If the widget is invisible, the resize event
  is generated when show() is called.

  The size is adjusted if it is outside the \link setMinimumSize()
  minimum\endlink or \link setMaximumSize() maximum\endlink widget size.

  This function is virtual, and all other overloaded resize()
  implementations call it.

  \warning If you call resize() or setGeometry() from resizeEvent(),
  you may see infinite recursion.

  \sa size(), move(), setGeometry(), resizeEvent(),
  minimumSize(),  maximumSize()
*/

void QWidget::resize( int w, int h )
{
    if ( w == width() && h == height() )
	return;
    if ( testWFlags(WType_Desktop) )
	return;
    if ( extra ) {				// any size restrictions?
	w = QMIN(w,extra->maxw);
	h = QMIN(h,extra->maxh);
	w = QMAX(w,extra->minw);
	h = QMAX(h,extra->minh);
    }
    if ( w < 1 )				// invalid size
	w = 1;
    if ( h < 1 )
	h = 1;
    QRect r = crect;
    QSize s(w,h);
    QSize olds = size();
    r.setSize( s );
    setCRect( r );
    internalResize( w, h );
    if ( !isVisible() ) {
	deferResize( olds );
    } else {
	cancelResize();
	QResizeEvent e( s, olds );
	QApplication::sendEvent( this, &e );       // send resize event immediately
	if ( !testWFlags(WResizeNoErase) ) {
	    repaint( TRUE );
	}
    }
}


void QWidget::internalResize( int w, int h )
{
    if ( testWFlags(WType_TopLevel) ) {
	setWFlags(WConfigPending);
	XSizeHints size_hints;			// tell window manager
	size_hints.flags = USSize;
	size_hints.width = w;
	size_hints.height = h;
	do_size_hints( dpy, winid, extra, &size_hints );
    }
    XResizeWindow( dpy, winid, w, h );
}


/*!
  \overload void QWidget::setGeometry( const QRect & )
*/

/*!
  Sets the widget geometry to \e w by \e h, positioned at \e x,y in its
  parent widget.

  A \link resizeEvent() resize event\endlink and a \link moveEvent() move
  event\endlink are generated immediately if the widget is visible. If the
  widget is invisible, the events are generated when show() is called.

  The size is adjusted if it is outside the \link setMinimumSize()
  minimum\endlink or \link setMaximumSize() maximum\endlink widget size.

  This function is virtual, and all other overloaded setGeometry()
  implementations call it.

  \warning If you call setGeometry() from resizeEvent() or moveEvent(),
  you may see infinite recursion.

  \sa geometry(), move(), resize(), moveEvent(), resizeEvent(),
  minimumSize(), maximumSize()
*/

void QWidget::setGeometry( int x, int y, int w, int h )
{
    if ( testWFlags(WType_Desktop) )
	return;
    if ( extra ) {				// any size restrictions?
	w = QMIN(w,extra->maxw);
	h = QMIN(h,extra->maxh);
	w = QMAX(w,extra->minw);
	h = QMAX(h,extra->minh);
    }
    if ( w < 1 )				// invalid size
	w = 1;
    if ( h < 1 )
	h = 1;
    QPoint oldp = pos();
    QSize  olds = size();
    QRect  r( x, y, w, h );
    if ( r.topLeft() == oldp && r.size() == olds )
	return;
    setCRect( r );
    internalSetGeometry( x, y, w, h );
    if ( !isVisible() ) {
	deferMove( oldp );
	if ( isTopLevel() )			// force internalSetGeometry
	    deferResize( QSize(-olds.width(), -olds.height()) );
	else
	    deferResize( olds );
    } else {
	cancelMove();
	cancelResize();
	QResizeEvent e1( r.size(), olds );
	QApplication::sendEvent( this, &e1 );	// send resize event
	QMoveEvent e2( r.topLeft(), oldp );
	QApplication::sendEvent( this, &e2 );	// send move event
	if ( !testWFlags(WResizeNoErase) ) {
	    repaint( TRUE );
	}
    }
}


void QWidget::internalSetGeometry( int x, int y, int w, int h )
{
    if ( testWFlags(WType_TopLevel) ) {
	setWFlags(WConfigPending);
	XSizeHints size_hints;			// tell window manager
	size_hints.flags = USPosition | USSize;
	size_hints.x = x;
	size_hints.y = y;
	size_hints.width = w;
	size_hints.height = h;
	do_size_hints( dpy, winid, extra, &size_hints );
    }
    XMoveResizeWindow( dpy, winid, x, y, w, h );
}


/*!
  \overload void QWidget::setMinimumSize( const QSize &size )
*/

/*!
  Sets the minimum size of the widget to \e w by \e h pixels.

  The widget cannot be resized to a smaller size than the minimum widget
  size. The widget's size is forced to the minimum size if the current
  size is smaller.

  \sa minimumSize(), setMaximumSize(), setSizeIncrement(), resize(), size()
*/

void QWidget::setMinimumSize( int minw, int minh )
{
#if defined(CHECK_RANGE)
    if ( minw < 0 || minh < 0 )
	warning("QWidget::setMinimumSize: The smallest allowed size is (0,0)");
#endif
    createExtra();
    if ( extra->minw == minw && extra->minh == minh )
	return;
    extra->minw = minw;
    extra->minh = minh;
    if ( minw > width() || minh > height() )
	resize( QMAX(minw,width()), QMAX(minh,height()) );
    if ( testWFlags(WType_TopLevel) ) {
	XSizeHints size_hints;
	size_hints.flags = 0;
	do_size_hints( dpy, winid, extra, &size_hints );
    }
    if ( parentWidget() ) {
	QEvent *e = new QEvent( Event_LayoutHint );
	QApplication::postEvent( parentWidget(), e );
    }
}

/*!
  \overload void QWidget::setMaximumSize( const QSize &size )
*/

/*!
  Sets the maximum size of the widget to \e w by \e h pixels.

  The widget cannot be resized to a larger size than the maximum widget
  size. The widget's size is forced to the maximum size if the current
  size is greater.

  \sa maximumSize(), setMinimumSize(), setSizeIncrement(), resize(), size()
*/

void QWidget::setMaximumSize( int maxw, int maxh )
{
#if defined(CHECK_RANGE)
    if ( maxw > QCOORD_MAX || maxh > QCOORD_MAX )
	warning("QWidget::setMaximumSize: (%s/%s) "
		"The largest allowed size is (%d,%d)",
		 name( "unnamed" ), className(), QCOORD_MAX, QCOORD_MAX );
    if ( maxw < 0 || maxh < 0 )
	warning("QWidget::setMaximumSize: (%s/%s) Negative sizes (%d,%d) "
		"are not possible",
		name( "unnamed" ), className(), maxw, maxh );
#endif
    createExtra();
    if ( extra->maxw == maxw && extra->maxh == maxh )
	return;
    extra->maxw = maxw;
    extra->maxh = maxh;
    if ( maxw < width() || maxh < height() )
	resize( QMIN(maxw,width()), QMIN(maxh,height()) );
    if ( testWFlags(WType_TopLevel) ) {
	XSizeHints size_hints;
	size_hints.flags = 0;
	do_size_hints( dpy, winid, extra, &size_hints );
    }
    if ( parentWidget() ) {
	QEvent *e = new QEvent( Event_LayoutHint );
	QApplication::postEvent( parentWidget(), e );
    }
}

/*!
  Sets the size increment of the widget.  When the user resizes the
  window, the size will move in steps of \e w pixels horizontally and
  \e h pixels vertically.

  Note that while you can set the size increment for all widgets, it
  has no effect except for top-level widgets.

  \warning The size increment has no effect under Windows, and may be
  disregarded by the window manager on X.

  \sa sizeIncrement(), setMinimumSize(), setMaximumSize(), resize(), size()
*/

void QWidget::setSizeIncrement( int w, int h )
{
    createExtra();
    if ( extra->incw == w && extra->inch == h )
	return;
    extra->incw = w;
    extra->inch = h;
    if ( testWFlags(WType_TopLevel) ) {
	XSizeHints size_hints;
	size_hints.flags = 0;
	do_size_hints( dpy, winid, extra, &size_hints );
    }
}
/*!
  \overload void QWidget::setSizeIncrement( const QSize& )
*/

/*!
  \overload void QWidget::erase()
  This version erases the entire widget.
*/

/*!
  \overload void QWidget::erase( const QRect &r )
*/

/*!
  Erases the specified area \e (x,y,w,h) in the widget without generating
  a \link paintEvent() paint event\endlink.

  If \e w is negative, it is replaced with <code>width() - x</code>.
  If \e h is negative, it is replaced width <code>height() - y</code>.

  Child widgets are not affected.

  \sa repaint()
*/

void QWidget::erase( int x, int y, int w, int h )
{
    if ( w < 0 )
	w = crect.width()  - x;
    if ( h < 0 )
	h = crect.height() - y;
    if ( w != 0 && h != 0 )
	XClearArea( dpy, winid, x, y, w, h, FALSE );
}

/*!
  Scrolls the contents of the widget \e dx pixels rightwards and \e dy
  pixels downwards.  If \e dx/dy is negative, the scroll direction is
  leftwards/upwards.  Child widgets are moved accordingly.

  The areas of the widget that are exposed will be erased and
  \link paintEvent() paint events\endlink may be generated immediately,
  or after some further event processing.

  \warning If you call scroll() in a function which may itself be
  called from the moveEvent() or paintEvent() of a direct child of the
  widget being scrolled, you may see infinite recursion.

  \sa erase(), bitBlt()
*/

void QWidget::scroll( int dx, int dy )
{
    int x1, y1, x2, y2, w=crect.width(), h=crect.height();
    if ( dx > 0 ) {
	x1 = 0;
	x2 = dx;
	w -= dx;
    } else {
	x1 = -dx;
	x2 = 0;
	w += dx;
    }
    if ( dy > 0 ) {
	y1 = 0;
	y2 = dy;
	h -= dy;
    } else {
	y1 = -dy;
	y2 = 0;
	h += dy;
    }

    if ( dx == 0 && dy == 0 )
	return;

    GC gc = qt_xget_readonly_gc();
    XSetGraphicsExposures( dpy, gc, TRUE );	// want expose events
    XCopyArea( dpy, winid, winid, gc, x1, y1, w, h, x2, y2);
    XSetGraphicsExposures( dpy, gc, FALSE );

    if ( children() ) {				// scroll children
	QPoint pd( dx, dy );
	QObjectListIt it(*children());
	register QObject *object;
	while ( it ) {				// move all children
	    object = it.current();
	    if ( object->isWidgetType() ) {
		QWidget *w = (QWidget *)object;
		w->move( w->pos() + pd );
	    }
	    ++it;
	}
    }

    // Don't let the server be bogged-down with repaint events
    bool repaint_immediately = qt_sip_count( this ) < 3;

    if ( dx ) {
	x1 = x2 == 0 ? w : 0;
	if ( repaint_immediately )
	    repaint( x1, 0, crect.width()-w, crect.height(), TRUE );
	else
	    XClearArea( dpy, winid, x1, 0, crect.width()-w, crect.height(),
			TRUE);
    }
    if ( dy ) {
	y1 = y2 == 0 ? h : 0;
	if ( repaint_immediately )
	    repaint( 0, y1, crect.width(), crect.height()-h, TRUE );
	else
	    XClearArea( dpy, winid, 0, y1, crect.width(), crect.height()-h,
			TRUE );
    }

    qt_insert_sip( this, dx, dy );
}


/*!
  \overload void QWidget::drawText( const QPoint &pos, const char *str )
*/

/*!
  Writes \e str at position \e x,y.

  The \e y position is the base line position of the text.  The text is
  drawn using the default font and the default foreground color.

  This function is provided for convenience.  You will generally get
  more flexible results and often higher speed by using a a \link
  QPainter painter\endlink instead.

  \sa setFont(), foregroundColor(), QPainter::drawText()
*/

void QWidget::drawText( int x, int y, const char *str )
{
    if ( testWFlags(WState_Visible) ) {
	QPainter paint;
	paint.begin( this );
	paint.drawText( x, y, str );
	paint.end();
    }
}


/*!
  Internal implementation of the virtual QPaintDevice::metric() function.

  Use the QPaintDeviceMetrics class instead.
*/

int QWidget::metric( int m ) const
{
    int val;
    if ( m == PDM_WIDTH || m == PDM_HEIGHT ) {
	if ( m == PDM_WIDTH )
	    val = crect.width();
	else
	    val = crect.height();
    } else {
	int scr = qt_xscreen();
	switch ( m ) {
	    case PDM_WIDTHMM:
		val = (DisplayWidthMM(dpy,scr)*crect.width())/
		      DisplayWidth(dpy,scr);
		break;
	    case PDM_HEIGHTMM:
		val = (DisplayHeightMM(dpy,scr)*crect.height())/
		      DisplayHeight(dpy,scr);
		break;
	    case PDM_NUMCOLORS:
		val = DisplayCells(dpy,scr);
		break;
	    case PDM_DEPTH:
		val = DisplayPlanes(dpy,scr);
		break;
	    default:
		val = 0;
#if defined(CHECK_RANGE)
		warning( "QWidget::metric: Invalid metric command" );
#endif
	}
    }
    return val;
}

void QWidget::createSysExtra()
{
    extra->xic = 0;
    extra->dnd = FALSE;
}

void QWidget::deleteSysExtra()
{
}

/*!
  Announces to the system that this widget \e may be able to
  accept drop events.

  In Qt 1.x, drop event handlers are in QDropSite.

  \sa acceptDrops()
*/

void QWidget::setAcceptDrops( bool on )
{
    createExtra();

    if ( extra->dnd != on ) {
	extra->dnd = on;

	if ( on ) {
	    QWidget * tlw = topLevelWidget();

	    extern Atom qt_xdnd_aware;
	    Atom qt_xdnd_version = (Atom)2;
	    XChangeProperty ( dpy, tlw->winId(), qt_xdnd_aware,
			      XA_ATOM, 32, PropModeReplace,
			      (unsigned char *)&qt_xdnd_version, 1 );
	}
    }
}

/*!
  Returns TRUE if drop events are enabled for this widget.

  \sa setAcceptDrops()
*/
bool QWidget::acceptDrops() const
{
    return ( extra && extra->dnd );
}

/*!
  Causes only the parts of the widget which overlap \a region
  to be visible.  If the region includes pixels outside the
  rect() of the widget, window system controls in that area
  may or may not be visible, depending on the platform.

  Note that this effect can be slow if the region is particularly
  complex.

  \sa setMask(QBitmap), clearMask()
*/
void QWidget::setMask(const QRegion& region)
{
    XShapeCombineRegion( dpy, winId(), ShapeBounding, 0, 0,
	region.handle(), ShapeSet);
}

/*!
  Causes only the pixels of the widget for which \a bitmap
  has a corresponding 1 bit
  to be visible.  If the region includes pixels outside the
  rect() of the widget, window system controls in that area
  may or may not be visible, depending on the platform.

  Note that this effect can be slow if the region is particularly
  complex.

  \sa setMask(const QRegion&), clearMask()
*/
void QWidget::setMask(QBitmap bitmap)
{
    XShapeCombineMask( dpy, winId(), ShapeBounding, 0, 0,
	bitmap.handle(), ShapeSet);
}

/*!
  Removes any mask set by setMask().

  \sa setMask()
*/
void QWidget::clearMask()
{
    XShapeCombineMask( dpy, winId(), ShapeBounding, 0, 0,
	None, ShapeSet);
}
