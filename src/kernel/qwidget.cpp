/****************************************************************************
** $Id: qwidget.cpp,v 2.117.2.4 1999/01/27 17:06:00 ettrich Exp $
**
** Implementation of QWidget class
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

#include "qobjectlist.h"
#include "qobjectdict.h"
#include "qwidget.h"
#include "qwidgetlist.h"
#include "qwidgetintdict.h"
#include "qptrdict.h"
#include "qfocusdata.h"
#include "qpixmap.h"
#include "qkeycode.h"
#include "qapplication.h"
#include "qbrush.h"
#if defined(_WS_WIN_)
#if defined(_CC_BOOL_DEF_)
#undef	bool
#include <windows.h>
#define bool int
#else
#include <windows.h>
#endif
#endif

/*!
  \class QWidget qwidget.h
  \brief The QWidget class is the base class of all user interface objects.

  \ingroup abstractwidgets

  The widget is the atom of the user interface: It receives mouse,
  keyboard and other events from the window system, and paints a
  representation of itself on the screen.  Every widget is
  rectangular, and they are sorted in a Z-order.  A widget is clipped
  by its parent and by the widgets in front of it.

  A widget without a parent, called a top-level widget, is a
  window with a frame and a title bar (though it is also possible to
  create top level widgets without such decoration).  A widget with a
  parent is a child window in its parent.  You usually cannot distinguish
  a child widget from its parent visually.

  QWidget has many member functions, but some of them have little
  direct functionality - for example it has a font but never uses it
  itself. There are many subclasses which provide real functionality,
  as diverse as QPushButton, QListBox and QTabDialog.

  <strong>Groups of functions:</strong>
  <ul>

  <li> Window functions:
	show(),
	hide(),
	raise(),
	lower(),
	close().

  <li> Top level windows:
	caption(),
	setCaption(),
	icon(),
	setIcon(),
	iconText(),
	setIconText(),
	isActiveWindow(),
	setActiveWindow(),
	iconify().

  <li> Window contents:
	update(),
	repaint(),
	erase(),
	drawRect(),
	scroll().

  <li> Geometry:
	move(),
	resize(),
	setGeometry(),
	pos(),
	size(),
	rect(),
	x(),
	y(),
	width(),
	height(),
	frameGeometry(),
	geometry(),
	childrenRect(),
	sizeHint(),
	adjustSize(),
	mapFromGlobal(),
	mapFromParent()
	mapToGlobal(),
	mapToParent(),
	maximumSize(),
	minimumSize(),
	sizeIncrement(),
	setMaximumSize(),
	setMinimumSize(),
	setSizeIncrement(),
	setFixedSize()

  <li> Mode:
	isVisible(),
	isVisibleTo(),
	isVisibleToTLW(),
	isDesktop(),
	isEnabled(),
	isEnabledTo(),
	isEnabledToTLW(),
	isModal(),
	isPopup(),
	isTopLevel(),
	setEnabled(),
	hasMouseTracking(),
	setMouseTracking(),
	isUpdatesEnabled(),
	setUpdatesEnabled(),
	setFontPropagation(),
	fontPropagation(),
	setPalettePropagation(),
	palettePropagation().

  <li> Look and feel:
	style(),
	setStyle(),
	cursor(),
	setCursor()
	font(),
	setFont(),
	palette(),
	setPalette(),
	backgroundMode(),
	setBackgroundMode(),
	backgroundPixmap(),
	setBackgroundPixmap(),
	backgroundBrush(),
	colorGroup(),
	fontMetrics(),
	fontInfo().

  <li> Keyboard focus functions:
	isFocusEnabled(),
	setFocusPolicy(),
	focusPolicy(),
	hasFocus(),
	setFocus(),
	clearFocus(),
	setTabOrder(),
	setFocusProxy().

  <li> Mouse and keyboard grabbing:
	grabMouse(),
	releaseMouse(),
	grabKeyboard(),
	releaseKeyboard(),
	mouseGrabber(),
	keyboardGrabber().

  <li> Event handlers:
	event(),
	mousePressEvent(),
	mouseReleaseEvent(),
	mouseDoubleClickEvent(),
	mouseMoveEvent(),
	keyPressEvent(),
	keyReleaseEvent(),
	focusInEvent(),
	focusOutEvent(),
	enterEvent(),
	leaveEvent(),
	paintEvent(),
	moveEvent(),
	resizeEvent(),
	closeEvent().

  <li> Change handlers:
	backgroundColorChange(),
	backgroundPixmapChange(),
	enabledChange(),
	fontChange(),
	paletteChange(),
	styleChange().

  <li> System functions:
	parentWidget(),
	topLevelWidget(),
	recreate(),
	winId(),
	find(),
	metric().

  <li> Internal kernel functions:
	setFRect(),
	setCRect(),
	focusNextPrevChild(),
	wmapper(),
	clearWFlags(),
	getWFlags(),
	setWFlags(),
	testWFlags().
  </ul>

  Every widget's constructor accepts two or three standard arguments:
  <ul>
  <li><code>QWidget *parent = 0</code> is the parent of the new widget.
  If it is 0 (the default), the new widget will be a top-level window.
  If not, it will be a child of \e parent, and be constrained by \e
  parent's geometry.
  <li><code>const char *name = 0</code> is the widget name of the new
  widget.  The widget name is little used at the moment - the
  dumpObjectTree() debugging function uses it, and you can access it using
  name().  It will become more important when our visual GUI builder is
  ready (you can name a a widget in the builder, and connect() to it by
  name in your code).
  <li><code>WFlags f = 0</code> (where available) sets the <a
  href="#widgetflags">widget flags;</a> the default is good for almost
  all widgets, but to get e.g. top-level widgets without a window
  system frame you must use special flags.
  </ul>

  The tictac/tictac.cpp example program is good example of a simple
  widget.  It contains a few event handlers (as all widgets must), a
  few custom routines that are peculiar to it (as all useful widgets
  must), and has a few children and connections.  Everything it does
  is done in response to an event: This is by far the most common way
  to design GUI applications.

  You will need to supply the content for your widgets yourself, but
  here is a brief run-down of the events, starting with the most common
  ones: <ul>

  <li> paintEvent() - called whenever the widget needs to be
  repainted.  Every widget which displays output must implement it,
  and it is sensible to \e never paint on the screen outside
  paintEvent().  You are guaranteed to receive a paint event
  immediately after every resize events, and at once when the widget
  is first shown.

  <li> resizeEvent() - called when the widget has been resized.

  <li> mousePressEvent() - called when a mouse button is pressed.
  There are six mouse-related events, mouse press and mouse release
  events are by far the most important.  A widget receives mouse press
  events when the widget is inside it, or when it has grabbed the
  mouse using grabMouse().

  <li> mouseReleaseEvent() - called when a mouse button is released.
  A widget receives mouse release events when it has received the
  corresponding mouse press event.  This means that if the user
  presses the mouse inside \e your widget, then drags the mouse to
  somewhere else, then releases, \e your widget receives the release
  event.  There is one exception, however: If a popup menu appears
  while the mouse button is held down, that popup steals the mouse
  events at once.

  <li> mouseDoubleClickEvent() - not quite as obvious as it might seem.
  If the user double-clicks, the widget receives a mouse press event
  (perhaps a mouse move event or two if he/she does not hold the mouse
  quite steady), a mouse release event and finally this event.  It is \e
  not \e possible to distinguish a click from a double click until you've
  seen whether the second click arrives.  (This is one reason why most GUI
  books recommend that double clicks be an extension of single clicks,
  rather than a different action.)
  </ul>

  If your widget only contains child widgets, you probably do not need to
  implement any event handlers (except resizeEvent() for custom layout
  management).

  Widgets that accept keyboard input need to reimplement a few more
  event handlers: <ul>

  <li> keyPressEvent() - called whenever a key is pressed, and again
  when a key has been held down long enough for it to auto-repeat.
  Note that the Tab and shift-Tab keys are only passed to the widget
  if they are not used by the focus-change mechanisms.  To force those
  keys to be processed by your widget, you must override QWidget::event().

  <li> focusInEvent() - called when the widget gains keyboard focus
  (assuming you have called setFocusPolicy(), of course). Well
  written widgets indicate that they own the keyboard focus in a clear
  but discreet way.

  <li> focusOutEvent() - called when the widget loses keyboard
  focus.
  </ul>

  Some widgets will need to reimplement some more obscure event
  handlers, too: <ul>

  <li> mouseMoveEvent() - called whenever the mouse moves while a
  button is held down.  This is useful for e.g. dragging.  If you call
  setMouseTracking(TRUE), you get mouse move events even when no
  buttons are held down.  (Note that applications which make use of
  mouse tracking are often not very useful on low-bandwidth X
  connections.)

  <li> keyReleaseEvent() - called whenever a key is released, and also
  while it is held down if the key is auto-repeating.  In that case
  the widget receives a key release event and immediately a key press
  event for every repeat.
  Note that the Tab and shift-Tab keys are only passed to the widget
  if they are not used by the focus-change mechanisms.  To force those
  keys to be processed by your widget, you must override QWidget::event().

  <li> enterEvent() - called when the mouse enters the widget's screen
  space.  (This excludes screen space owned by any children of the
  widget.)

  <li> leaveEvent() - called when the mouse leaves the widget's screen
  space.

  <li> moveEvent() - called when the widget has been moved relative to its
  parent.

  <li> closeEvent() - called when the user closes the widget (or when
  close() is called).
 </ul>

  There are also some \e really obscure events.  They are listed in
  qevent.h and you need to reimplement event() to handle them.  The
  default implementation of event() handles Tab and shift-Tab (to move
  the keyboard focus), and passes on every other event to one of the
  more specialized handlers above.

  When writing a widget, there are a few more things to look out for.
  In the constructor, be sure to set up your member variables early
  on, before there's any chance that you might receive an event.

  It is often a good idea to reimplement sizeHint(), so users of your
  class can set up layout management more easily.  If you do, consider
  offering size management using autoMinimumSize() too.

  If your widget is a top-level window, setCaption() and setIcon() set
  the title bar and icon respectively.

  \sa QEvent, QPainter, QGridLayout, QBoxLayout
*/


/*****************************************************************************
  Internal QWidgetMapper class

  The purpose of this class is to map widget identifiers to QWidget objects.
  All QWidget objects register themselves in the QWidgetMapper when they
  get an identifier. Widgets unregister themselves when they change ident-
  ifier or when they are destroyed. A widget identifier is really a window
  handle.

  The widget mapper is created and destroyed by the main application routines
  in the file qapp_xxx.cpp.
 *****************************************************************************/

static const int WDictSize = 101;

class QWidgetMapper : public QWidgetIntDict
{						// maps ids -> widgets
public:
    QWidgetMapper();
   ~QWidgetMapper();
    QWidget *find( WId id );			// find widget
    void     insert( const QWidget * );		// insert widget
    bool     remove( WId id );			// remove widget
private:
    WId	     cur_id;
    QWidget *cur_widget;
};

QWidgetMapper *QWidget::mapper = 0;		// app global widget mapper


QWidgetMapper::QWidgetMapper() : QWidgetIntDict(WDictSize)
{
    cur_id = 0;
    cur_widget = 0;
}

QWidgetMapper::~QWidgetMapper()
{
    clear();
}

inline QWidget *QWidgetMapper::find( WId id )
{
    if ( id != cur_id ) {			// need to lookup
	cur_widget = QWidgetIntDict::find((long)id);
	if ( cur_widget )
	    cur_id = id;
	else
	    cur_id = 0;
    }
    return cur_widget;
}

inline void QWidgetMapper::insert( const QWidget *widget )
{
    QWidgetIntDict::insert((long)widget->winId(),widget);
}

inline bool QWidgetMapper::remove( WId id )
{
    if ( cur_id == id ) {			// reset current widget
	cur_id = 0;
	cur_widget = 0;
    }
    return QWidgetIntDict::remove((long)id);
}


/*****************************************************************************
  QWidget member functions
 *****************************************************************************/

typedef Q_DECLARE(QPtrDictM,void) QDeferDict;

static QDeferDict *deferredMoves   = 0;
static QDeferDict *deferredResizes = 0;

static void cleanupDeferredDicts()
{
    delete deferredMoves;
    deferredMoves = 0;
    delete deferredResizes;
    deferredResizes = 0;
}

static void initDeferredDicts()
{
    if ( deferredMoves )
	return;
    deferredMoves   = new QDeferDict( 137 );
    CHECK_PTR( deferredMoves );
    deferredResizes = new QDeferDict( 137 );
    CHECK_PTR( deferredResizes );
    qAddPostRoutine( cleanupDeferredDicts );
}

/*
  The compress functions below store two short values in a pointer.
*/

static inline uint compress( int a, int b )
{
    return ((uint)(a-QCOORD_MIN) << 16) | ((b-QCOORD_MIN) & 0xffff);
}

static inline short decompress_a( uint n )
{
    return (short)((int)(n >> 16) + QCOORD_MIN);
}

static inline short decompress_b( uint n )
{
    return (short)((int)(n & 0xffff) + QCOORD_MIN);
}


void QWidget::deferMove( const QPoint &oldPos )
{
    uint n = (uint)(long)deferredMoves->find( this );
    if ( !n ) {
	n = compress(oldPos.x(),oldPos.y());
	deferredMoves->insert( this, (void*)n );
    }
}

void QWidget::deferResize( const QSize &oldSize )
{
    int w = oldSize.width();
    int h = oldSize.height();
    uint n = (uint)(long)deferredResizes->find( this );
    if ( n ) {
	if ( w < 0 && decompress_a(n) > 0 ) {	// did setGeometry
	    deferredResizes->take( this );
	    n = compress(-decompress_a(n),-decompress_b(n));
	    deferredResizes->insert( this, (void*)n );
	}
    } else {
	n = compress(w,h);
	deferredResizes->insert( this, (void*)n );
    }
}


void QWidget::cancelMove()
{
    deferredMoves->take( this );
}

void QWidget::cancelResize()
{
    deferredResizes->take( this );
}


/*!
  Send deferred or enforced move, resize and child events for this widget.
*/

void QWidget::sendDeferredEvents()
{
    QApplication::sendPostedEvents( this, Event_ChildInserted );
    uint m = (uint)(long)deferredMoves->find(this);
    uint r = (uint)(long)deferredResizes->find(this);
    if ( m && r && decompress_a(r) < 0 ) {
	// Hack it to work: the old width is negative to indicate that
	// we wanted to setGeometry and not move + resize.
	internalSetGeometry( x(), y(), width(), height() );
    } else {
	if ( m )
	    internalMove( x(), y() );
	if ( r )
	    internalResize( width(), height() );
    }
    if ( m ) {
	deferredMoves->take( this );
	QMoveEvent e( pos(), QPoint(decompress_a(m), decompress_b(m)) );
	QApplication::sendEvent( this, &e );
    }
    if ( r ) {
	deferredResizes->take( this );
	int w = decompress_a(r);
	int h = decompress_b(r);
	QResizeEvent e( size(), QSize(QABS(w),QABS(h)) );
	QApplication::sendEvent( this, &e );
    }
}

/*!
  Constructs a widget which is a child of \e parent, with the name \e name and
  widget flags set to \e f.

  If \e parent is 0, the new widget becomes a \link isTopLevel() top-level
  window\endlink. If \e parent is another widget, this widget becomes a child
  window inside \e parent.

  The \e name is sent to the QObject constructor.

  <a name="widgetflags"></a>

  The widget flags argument \e f is normally 0, but it can be set to
  customize the window frame of a top-level widget (i.e. \e parent must be
  zero). To customize the frame, set the \c WStyle_Customize flag OR'ed with
  any of these flags:

  <ul>
  <li> \c WStyle_NormalBorder gives the window a normal border. Cannot
    be combined with \c WStyle_DialogBorder or \c WStyle_NoBorder.
  <li> \c WStyle_DialogBorder gives the window a thin dialog border.
    Cannot be combined with \c WStyle_NormalBorder or \c WStyle_NoBorder.
  <li> \c WStyle_NoBorder gives a borderless window. Note that the
    user cannot move or resize a borderless window via the window system.
    Cannot be combined with \c WStyle_NormalBorder or \c WStyle_DialogBorder.
  <li> \c WStyle_Title gives the window a title bar.
  <li> \c WStyle_SysMenu adds a window system menu.
  <li> \c WStyle_Minimize adds a minimize button.
  <li> \c WStyle_Maximize adds a maximize button.
  <li> \c WStyle_MinMax is equal to <code>(WStyle_Minimize | WStyle_Maximize)
  </code>.
  <li> \c WStyle_Tool makes the window a tool window, usually
    combined with \c WStyle_NoBorder. A tool window is a small window that
    lives for a short time and it is typically used for creating popup
    windows.
  </ul>

  Note that X11 does not necessarily support all style flag combinations. X11
  window managers live their own lives and can only take hints. Win32
  supports all style flags.

  Example:
  \code
    QLabel *toolTip = new QLabel( 0, "myToolTip",
				  WStyle_Customize | WStyle_NoBorder |
				  WStyle_Tool );
  \endcode

  The widget flags are defined in qwindowdefs.h (which is included by
  qwidget.h).
*/

QWidget::QWidget( QWidget *parent, const char *name, WFlags f )
    : QObject( parent, name ), QPaintDevice( PDT_WIDGET ),
      pal( parent ? parent->palette()		// use parent's palette
           : *qApp->palette() )			// use application palette
{
    isWidget = TRUE;				// is a widget
    winid = 0;					// default attributes
    flags = f;
    focusChild = 0;
    extra = 0;					// no extra widget info
    if ( !deferredMoves )			// do it only once
	initDeferredDicts();
    create();					// platform-dependent init
    deferMove( frect.topLeft() );
    deferResize( crect.size() );
    if ( isTopLevel() ||			// kludge alert
	 testWFlags(WState_TabToFocus) ) {	// focus was set using WFlags
	QFocusData *fd = focusData( TRUE );
	if ( fd->focusWidgets.findRef(this) < 0 )
 	    fd->focusWidgets.append( this );
	if ( isTopLevel() ) {
	    // kludge details: set focus to this widget, whether it
	    // acceps focus or not.  this makes buggy old qt programs
	    // which assume that the tlw has focus even though it
	    // doesn't accept focus work.
	    if ( fd->it.current() != this ) {
		fd->it.toFirst();
		while ( fd->it.current() != this && !fd->it.atLast() )
		    ++fd->it;
	    }
	}
    }
    if ( parent ) {
	QChildEvent *e = new QChildEvent( Event_ChildInserted, this );
	QApplication::postEvent( parent, e );
    }
}


/*!
  Destroys the widget.

  All children of this widget are deleted first.
  The application exits if this widget is (was) the main widget.
*/

QWidget::~QWidget()
{
    // remove myself from the can-take-focus list
    QFocusData *f = focusData( FALSE );
    if ( f )
	f->focusWidgets.removeRef( this );

    if ( parentObj ) {
	QChildEvent e( Event_ChildRemoved, this );
	QApplication::sendEvent( parentObj, &e );
    }
    if ( deferredMoves ) {
	deferredMoves->take( this );	// clean deferred move/resize
	deferredResizes->take( this );
    }
    if ( QApplication::main_widget == this ) {	// reset main widget
	QApplication::main_widget = 0;
	if (qApp)
	    qApp->quit();
    }
    if ( focusWidget() == this )
	clearFocus();
    if ( testWFlags(WExportFontMetrics) )	// remove references to this
	QFontMetrics::reset( this );
    if ( testWFlags(WExportFontInfo) )		// remove references to this
	QFontInfo::reset( this );

    if ( isTopLevel() && isVisible() && winId() )
	hide();					// emit lastWindowClosed?

    // A parent widget must destroy all its children before destroying itself
    if ( childObjects ) {			// delete children objects
	QObjectListIt it(*childObjects);
	QObject *obj;
	while ( (obj=it.current()) ) {
	    ++it;
	    delete obj;
	    if ( !childObjects )		// removeChild resets it
		break;
	}
	delete childObjects;
	childObjects = 0;
    }

    // if I had focus, clear focus entirely.  this is probably a bad idea -
    // better to give focus to someone else
    if ( qApp->focus_widget == this )
	qApp->focus_widget = 0;

    if ( extra )
	deleteExtra();
    destroy();					// platform-dependent cleanup
}


/*!
  \internal
  Creates the global widget mapper.
  The widget mapper converts window handles to widget pointers.
  \sa destroyMapper()
*/

void QWidget::createMapper()
{
    mapper = new QWidgetMapper;
    CHECK_PTR( mapper );
}

/*!
  \internal
  Destroys the global widget mapper.
  \sa createMapper()
*/

void QWidget::destroyMapper()
{
    if ( !mapper )				// already gone
	return;
    register QWidget *w;
    QWidgetIntDictIt it( *((QWidgetIntDict*)mapper) );
    while ( (w=it.current()) ) {		// remove parents widgets
	++it;
	if ( !w->parentObj )			// widget is a parent
	    delete w;
    }
#if defined(DEBUG)
    ASSERT( it.count() == 0 );
#endif
#if 0
    it.toFirst();				// shouln't be any more widgets
    while ( (w=it.current()) ) {		// delete the rest
	++it;
	delete w;
    }
#endif
    delete mapper;
    mapper = 0;
}


static QWidgetList *wListInternal( QWidgetMapper *mapper, bool onlyTopLevel )
{
    QWidgetList *list = new QWidgetList;
    CHECK_PTR( list );
    if ( mapper ) {
	QWidget *w;
	QWidgetIntDictIt it( *((QWidgetIntDict*)mapper) );
	while ( (w=it.current()) ) {
	    ++it;
	    if ( !onlyTopLevel || w->isTopLevel() )
		list->append( w );
	}
    }
    return list;
}

/*!
  \internal
  Returns a list of all widgets.
  \sa tlwList(), QApplication::allWidgets()
*/

QWidgetList *QWidget::wList()
{
    return wListInternal( mapper, FALSE );
}

/*!
  \internal
  Returns a list of all top level widgets.
  \sa wList(), QApplication::topLevelWidgets()
*/

QWidgetList *QWidget::tlwList()
{
    return wListInternal( mapper, TRUE );
}


void QWidget::setWinId( WId id )		// set widget identifier
{
    if ( !mapper )				// mapper destroyed
	return;
    if ( winid )
	mapper->remove( winid );
    winid = id;
#if defined(_WS_X11_)
    hd = id;					// X11: hd == ident
#endif
    if ( id )
	mapper->insert( this );
}


/*!
  \internal
  Returns a pointer to the block of extra widget data.
*/

QWExtra *QWidget::extraData()
{
    return extra;
}

/*!
  \internal
  Creates the widget extra data.
*/

void QWidget::createExtra()
{
    if ( !extra ) {				// if not exists
	extra = new QWExtra;
	CHECK_PTR( extra );
	extra->guistyle = QApplication::style();// default style
	extra->minw = extra->minh = 0;
	extra->maxw = extra->maxh = QCOORD_MAX;
	extra->incw = extra->inch = 0;
	extra->caption = extra->iconText = 0;
	extra->icon = extra->bg_pix = 0;
	extra->bg_mode = PaletteBackground;
	extra->focusData = 0;
	extra->sizegrip = 0;
	extra->propagateFont = 0;
	extra->propagatePalette = 0;
	createSysExtra();
    }
}

/*!
  \internal
  Deletes the widget extra data.
*/

void QWidget::deleteExtra()
{
    if ( extra ) {				// if exists
	if ( extra->caption )			// Avoid purify complaint.
	    delete [] extra->caption;
	if ( extra->iconText )			// Avoid purify complaint.
	    delete [] extra->iconText;
	delete extra->icon;
	delete extra->bg_pix;
	delete extra->focusData;
	deleteSysExtra();
	delete extra;
	// extra->xic destroyed in QWidget::destroy()
	extra = 0;
    }
}


/*!
  Returns a pointer to the widget with window identifer/handle \e id.

  The window identifier type depends by the underlying window system,
  see qwindowdefs.h for the actual definition.
  If there is no widget with this identifier, a null pointer is returned.

  \sa wmapper(), id()
*/

QWidget *QWidget::find( WId id )
{
    return mapper ? mapper->find( id ) : 0;
}

/*!
  \fn QWidgetMapper *QWidget::wmapper()
  \internal
  Returns a pointer to the widget mapper.

  The widget mapper is an internal dictionary that is used to map from
  window identifiers/handles to widget pointers.
  \sa find(), id()
*/


/*!
  \fn WFlags QWidget::getWFlags() const
  \internal
  Returns the widget flags for this this widget.

  Widget flags are internal, not meant for public use.
  \sa testWFlags(), setWFlags(), clearWFlags()
*/

/*!
  \fn void QWidget::setWFlags( WFlags f )
  \internal
  Sets the widget flags \e f.

  Widget flags are internal, not meant for public use.
  \sa testWFlags(), getWFlags(), clearWFlags()
*/

/*!
  \fn void QWidget::clearWFlags( WFlags f )
  \internal
  Clears the widget flags \e f.

  Widget flags are internal, not meant for public use.
  \sa testWFlags(), getWFlags(), setWFlags()
*/


/*!
  \fn WId QWidget::winId() const
  Returns the window system identifier of the widget.

  Portable in principle, but if you use it you are probably about to do
  something non-portable. Be careful.

  \sa find()
*/


/*!
  Returns the GUI style for this widget.

  \sa setStyle(), QApplication::style()
*/

GUIStyle QWidget::style() const
{
    return extra ? extra->guistyle : QApplication::style();
}

/*!
  Sets the GUI style for this widget.  The valid values are listed
  in qwindowdefs.h.

  \sa style(), styleChange(), QApplication::setStyle()
*/

void QWidget::setStyle( GUIStyle style )
{
    GUIStyle old = this->style();
    createExtra();
    extra->guistyle = style;
    styleChange( old );
}

/*!
  \fn void QWidget::styleChange( GUIStyle oldStyle )

  This virtual function is called from setStyle().  \e oldStyle is the
  previous style; you can get the new style from style().

  Reimplement this function if your widget needs to know when its GUI
  style changes.  You will almost certainly need to update the widget
  using either repaint(TRUE) or update().

  The default implementation calls update().

  \sa setStyle(), style(), repaint(), update()
*/

void QWidget::styleChange( GUIStyle )
{
    update();
}


/*!
  \fn bool QWidget::isTopLevel() const
  Returns TRUE if the widget is a top-level widget, otherwise FALSE.

  A top-level widget is a widget which usually has a frame and a \link
  setCaption() caption\endlink (title bar). \link isPopup() Popup\endlink
  and \link isDesktop() desktop\endlink widgets are also top-level
  widgets. Modal \link QDialog dialog\endlink widgets are the only
  top-level widgets that can have \link parentWidget() parent
  widgets\endlink; all other top-level widgets have null parents.  Child
  widgets are the opposite of top-level widgets.

  \sa topLevelWidget(), isModal(), isPopup(), isDesktop(), parentWidget()
*/

/*!
  \fn bool QWidget::isModal() const
  Returns TRUE if the widget is a modal widget, otherwise FALSE.

  A modal widget is also a top-level widget.

  \sa isTopLevel(), QDialog
*/

/*!
  \fn bool QWidget::isPopup() const
  Returns TRUE if the widget is a popup widget, otherwise FALSE.

  A popup widget is created by specifying the widget flag \c WType_Popup
  to the widget constructor.

  A popup widget is also a top-level widget.

  \sa isTopLevel()
*/


/*!
  \fn bool QWidget::isDesktop() const
  Returns TRUE if the widget is a desktop widget, otherwise FALSE.

  A desktop widget is also a top-level widget.

  \sa isTopLevel(), QApplication::desktop()
*/


/*!
  \fn bool QWidget::isEnabled() const
  Returns TRUE if the widget is enabled, or FALSE if it is disabled.
  \sa setEnabled()
*/


/*!
  Returns TRUE if this widget and every parent up to but excluding
  \a ancestor is enabled, otherwise returns FALSE.

  \sa setEnabled() isEnabled()
*/

bool QWidget::isEnabledTo(QWidget* ancestor) const
{
    const QWidget * w = this;
    while ( w && w->isEnabled()
	    && !w->isTopLevel()
	    && w->parentWidget()
	    && w->parentWidget()!=ancestor
	)
	w = w->parentWidget();
    return w->isEnabled();
}


/*!
  Returns TRUE if this widget and every parent up to the \link
  topLevelWidget() top level widget \endlink is enabled, otherwise
  returns FALSE.

  This is equivalent to isEnabledTo(0).

  \sa setEnabled() isEnabled()
*/

bool QWidget::isEnabledToTLW() const
{
    return isEnabledTo(0);
}


/*!
  Enables widget input events if \e enable is TRUE, otherwise disables
  input events.

  An enabled widget receives keyboard and mouse events; a disabled
  widget does not.  Note that an enabled widget receives keyboard
  events only when it is in focus.

  Some widgets display themselves differently when they are disabled.
  For example a button might draw its label grayed out.

  \sa isEnabled(), QKeyEvent, QMouseEvent
*/

void QWidget::setEnabled( bool enable )
{
    if ( enable ) {
	if ( testWFlags(WState_Disabled) ) {
	    clearWFlags( WState_Disabled );
	    setBackgroundColorFromMode();
	    enabledChange( TRUE );
	}
    } else {
	if ( !testWFlags(WState_Disabled) ) {
	    if ( focusWidget() == this )
		focusNextPrevChild( TRUE );
	    setWFlags( WState_Disabled );
	    setBackgroundColorFromMode();
	    enabledChange( FALSE );
	}
    }
}

/*!
  \fn void QWidget::enabledChange( bool oldEnabled )

  This virtual function is called from setEnabled(). \e oldEnabled is the
  previous setting; you can get the new setting from enabled().

  Reimplement this function if your widget needs to know when it becomes
  enabled or disabled. You will almost certainly need to update the widget
  using either repaint(TRUE) or update().

  The default implementation calls repaint(TRUE).

  \sa setEnabled(), isEnabled(), repaint(), update()
*/

void QWidget::enabledChange( bool )
{
    repaint();
}


/*!
  \fn const QRect &QWidget::frameGeometry() const
  Returns the geometry of the widget, relative to its parent and
  including the window frame.
  \sa geometry(), x(), y(), pos()
*/

/*!
  \fn const QRect &QWidget::geometry() const
  Returns the geometry of the widget, relative to its parent widget
  and excluding the window frame.
  \sa frameGeometry(), size(), rect()
*/

/*!
  \fn int QWidget::x() const
  Returns the x coordinate of the widget, relative to its parent
  widget and including the window frame.
  \sa frameGeometry(), y(), pos()
*/

/*!
  \fn int QWidget::y() const
  Returns the y coordinate of the widget, relative to its parent
  widget and including the window frame.
  \sa frameGeometry(), x(), pos()
*/

/*!
  \fn QPoint QWidget::pos() const
  Returns the postion of the widget in its parent widget, including
  the window frame.
  \sa frameGeometry(), x(), y()
*/

/*!
  \fn QSize QWidget::size() const
  Returns the size of the widget, excluding the window frame.
  \sa geometry(), width(), height()
*/

/*!
  \fn int QWidget::width() const
  Returns the width of the widget, excluding the window frame.
  \sa geometry(), height(), size()
*/

/*!
  \fn int QWidget::height() const
  Returns the height of the widget, excluding the window frame.
  \sa geometry(), width(), size()
*/

/*!
  \fn QRect QWidget::rect() const
  Returns the the internal geometry of the widget, excluding the window frame.
  rect() equals QRect(0,0,width(),height()).
  \sa size()
*/


/*!
  Returns the bounding rectangle of the widget's children.
*/

QRect QWidget::childrenRect() const
{
    QRect r( 0, 0, 0, 0 );
    if ( !children() )
	return r;
    QObjectListIt it( *children() );		// iterate over all children
    QObject *obj;
    while ( (obj=it.current()) ) {
	++it;
	if ( obj->isWidgetType() )
	    r = r.unite( ((QWidget*)obj)->geometry() );
    }
    return r;
}


/*!
  Returns the minimum widget size.

  The widget cannot be resized to a smaller size than the minimum widget
  size.

  \sa setMinimumSize() maximumSize(), sizeIncrement()
*/

QSize QWidget::minimumSize() const
{
    return extra ? QSize(extra->minw,extra->minh) : QSize(0,0);
}

/*!
  Returns the maximum widget size.

  The widget cannot be resized to a larger size than the maximum widget
  size.

  \sa setMaximumSize(), minimumSize(), sizeIncrement()
*/

QSize QWidget::maximumSize() const
{
    return extra ? QSize(extra->maxw,extra->maxh)
		 : QSize(QCOORD_MAX,QCOORD_MAX);
}

/*!
  Returns the widget size increment.

  \sa setSizeIncrement(), minimumSize(), maximumSize()
*/

QSize QWidget::sizeIncrement() const
{
    return extra ? QSize(extra->incw,extra->inch) : QSize(0,0);
}


/*!
  Sets both the minimum and maximum sizes of the widget to \e s,
  thereby preventing it from ever growing or shrinking.

  \sa setMaximumSize() setMinimumSize()
*/

void QWidget::setFixedSize( const QSize & s)
{
    setMinimumSize( s );
    setMaximumSize( s );
    resize( s );
}


/*!
  \overload void QWidget::setFixedSize( int w, int h )
*/

void QWidget::setFixedSize( int w, int h )
{
    setMinimumSize( w, h );
    setMaximumSize( w, h );
    resize( w, h );
}


/*!
  Sets the minimum width of the widget to \a w without changing the
  height.  Provided for convenience.

  \sa sizeHint() minimumSize() maximumSize()
  setFixedSize() and more
*/

void QWidget::setMinimumWidth( int w )
{
    setMinimumSize( w, minimumSize().height() );
}


/*!
  Sets the minimum height of the widget to \a h without changing the
  width.  Provided for convenience.

  \sa sizeHint() minimumSize() maximumSize() setFixedSize() and more
*/

void QWidget::setMinimumHeight( int h )
{
    setMinimumSize( minimumSize().width(), h );
}


/*!
  Sets the maximum width of the widget to \a w without changing the
  height.  Provided for convenience.

  \sa sizeHint() minimumSize() maximumSize() setFixedSize() and more
*/

void QWidget::setMaximumWidth( int w )
{
    setMaximumSize( w, maximumSize().height() );
}


/*!
  Sets the maximum height of the widget to \a h without changing the
  width.  Provided for convenience.

  \sa sizeHint() minimumSize() maximumSize() setFixedSize() and more
*/

void QWidget::setMaximumHeight( int h )
{
    setMaximumSize( maximumSize().width(), h );
}


/*!
  Sets both the minimum and maximum width of the widget to \a w
  without changing the heights.  Provided for convenience.

  \sa sizeHint() minimumSize() maximumSize() setFixedSize() and more
*/

void QWidget::setFixedWidth( int w )
{
    setMinimumSize( w, minimumSize().height() );
    setMaximumSize( w, maximumSize().height() );
}


/*!
  Sets both the minimum and maximum heights of the widget to \a h
  without changing the widths.  Provided for convenience.

  \sa sizeHint() minimumSize() maximumSize() setFixedSize() and more
*/

void QWidget::setFixedHeight( int h )
{
    setMinimumSize( minimumSize().width(), h );
    setMaximumSize( maximumSize().width(), h );
}


/*!
  Translates the widget coordinate \e pos to a coordinate in the parent widget.

  Same as mapToGlobal() if the widget has no parent.
  \sa mapFromParent()
*/

QPoint QWidget::mapToParent( const QPoint &p ) const
{
    return p + crect.topLeft();
}

/*!
  Translates the parent widget coordinate \e pos to widget coordinates.

  Same as mapFromGlobal() if the widget has no parent.

  \sa mapToParent()
*/

QPoint QWidget::mapFromParent( const QPoint &p ) const
{
    return p - crect.topLeft();
}


/*!
  Returns the top-level widget for this widget.

  A top-level widget is an overlapping widget. It usually has no parent.
  Modal \link QDialog dialog widgets\endlink are the only top-level
  widgets that can have parent widgets.

  \sa isTopLevel()
*/

QWidget *QWidget::topLevelWidget() const
{
    QWidget *w = (QWidget *)this;
    QWidget *p = w->parentWidget();
    while ( !w->testWFlags(WType_TopLevel) && p ) {
	w = p;
	p = p->parentWidget();
    }
    return w;
}

void QWidget::setBackgroundColorFromMode()
{
    switch (extra ? (BackgroundMode)extra->bg_mode : PaletteBackground) {
      case FixedColor:
      case FixedPixmap:
      case NoBackground:
	break;
      case PaletteForeground:
	setBackgroundColorDirect( colorGroup().foreground() );
	break;
      case PaletteBackground:
	setBackgroundColorDirect( colorGroup().background() );
	break;
      case PaletteLight:
	setBackgroundColorDirect( colorGroup().light() );
	break;
      case PaletteMidlight:
	setBackgroundColorDirect( colorGroup().midlight() );
	break;
      case PaletteDark:
	setBackgroundColorDirect( colorGroup().dark() );
	break;
      case PaletteMid:
	setBackgroundColorDirect( colorGroup().mid() );
	break;
      case PaletteText:
	setBackgroundColorDirect( colorGroup().text() );
	break;
      case PaletteBase:
	setBackgroundColorDirect( colorGroup().base() );
	break;
    }
}

/*!
  Returns the mode most recently set by setBackgroundMode().
  The default is \link QWidget::BackgroundMode PaletteBackground\endlink.
*/
QWidget::BackgroundMode QWidget::backgroundMode() const
{
    return extra ? (BackgroundMode)extra->bg_mode : PaletteBackground;
}

/*!
  Tells the window system which color to clear this widget to when
  sending a paint event.

  In other words, this color is the color of the widget when
  paintEvent() is called.  To minimize flicker, this should be the
  most common color in the widget.

  The following values are valid:

  <ul>
    <li> \c PaletteForeground
	- use palette() . \link QColorGroup::foreground() foreground()\endlink
    <li> \c PaletteBackground
	- use palette() . \link QColorGroup::background() background()\endlink
    <li> \c PaletteLight
	- use palette() . \link QColorGroup::light() light()\endlink
    <li> \c PaletteMidlight
	- use palette() . \link QColorGroup::midlight() midlight()\endlink
    <li> \c PaletteDark
	- use palette() . \link QColorGroup::dark() dark()\endlink
    <li> \c PaletteMid
	- use palette() . \link QColorGroup::mid() mid()\endlink
    <li> \c PaletteText
	- use palette() . \link QColorGroup::text() text()\endlink
    <li> \c PaletteBase
	- use palette() . \link QColorGroup::base() base()\endlink
    <li> \c NoBackground
	- no color or pixmap is used - the paintEvent() must completely
	    cover the drawing area.  This can help avoid flicker.
  </ul>

  If setBackgroundPixmap() or setBackgroundColor() is called, the
  mode will be one of:
  <ul>
    <li> \c FixedPixmap - the pixmap set by setBackgroundPixmap()
    <li> \c FixedColor - the color set by setBackgroundColor()
  </ul>

  These values may not be used as parameters to setBackgroundMode().

  \define QWidget::BackgroundMode
  For most widgets the default (PaletteBackground, normally
  gray) suffices, but some need to use PaletteBase (the
  background color for text output, normally white) and a few need
  other colors.

  QListBox, which is "sunken" and uses the base color to contrast with
  its environment, does this:

  \code
    setBackgroundMode( PaletteBase );
  \endcode

  If you want to change the color scheme of a widget, the setPalette()
  function is better suited.  Here is how to set \e thatWidget to use a
  light green (RGB value 80, 255, 80) as background color, with shades
  of green used for all the 3D effects:

  \code
    thatWidget->setPalette( QPalette( QColor(80, 255, 80) ) );
  \endcode

  You can also use QApplication::setPalette() if you want to change
  the color scheme of your entire application, or of all new widgets.
*/
void QWidget::setBackgroundMode( BackgroundMode m )
{
    if ( m==NoBackground )
	setBackgroundEmpty();
    else if ( m==FixedColor || m==FixedPixmap ) {
	warning("May not pass FixedColor or FixedPixmap to setBackgroundMode()");
	return;
    }
    setBackgroundModeDirect(m);
}

/*!
  \internal
*/
void QWidget::setBackgroundModeDirect( BackgroundMode m )
{
    if (m==PaletteBackground && !extra) return;

    createExtra();
    if (extra->bg_mode != m) {
	extra->bg_mode = m;
	setBackgroundColorFromMode();
    }
}


/*!
  \fn const QColor &QWidget::backgroundColor() const

  Returns the background color of this widget.

  The background color is independent of the color group.

  Setting a new palette overwrites the background color.

  \sa setBackgroundColor(), foregroundColor(), colorGroup(), palette()
*/

/*!
  Returns the foreground color of this widget.

  The foreground color equals <code>colorGroup().foreground()</code>.

  \sa backgroundColor(), colorGroup()
*/

const QColor &QWidget::foregroundColor() const
{
    return colorGroup().foreground();
}


/*!
  \fn void QWidget::backgroundColorChange( const QColor &oldBackgroundColor )

  This virtual function is called from setBackgroundColor().
  \e oldBackgroundColor is the previous background color; you can get the new
  background color from backgroundColor().

  Reimplement this function if your widget needs to know when its
  background color changes.  You will almost certainly need to update the
  widget using either repaint(TRUE) or update().

  The default implementation calls update().

  \sa setBackgroundColor(), backgroundColor(), setPalette(), repaint(),
  update()
*/

void QWidget::backgroundColorChange( const QColor & )
{
    update();
}


/*!
  Returns the background pixmap, or null if no background pixmap has not
  been set.  If the widget has been made empty, this function will return
  a pixmap which isNull() rather than a null pointer.

  \sa setBackgroundPixmap(), setBackgroundMode()
*/

const QPixmap *QWidget::backgroundPixmap() const
{
    return (extra && extra->bg_pix) ? extra->bg_pix : 0;
}


/*!
  \fn void QWidget::backgroundPixmapChange( const QPixmap & oldBackgroundPixmap )

  This virtual function is called from setBackgroundPixmap().
  \e oldBackgroundPixmap is the previous background pixmap; you can get the
  new background pixmap from backgroundPixmap().

  Reimplement this function if your widget needs to know when its
  background pixmap changes.  You will almost certainly need to update the
  widget using either repaint(TRUE) or update().

  The default implementation calls update().

  \sa setBackgroundPixmap(), backgroundPixmap(), repaint(), update()
*/

void QWidget::backgroundPixmapChange( const QPixmap & )
{
    update();
}


/*!
  Returns the current color group of the widget palette.

  The color group is determined by the state of the widget.

  A disabled widget returns the QPalette::disabled() color group, a
  widget in focus returns the QPalette::active() color group and a
  normal widget returns the QPalette::normal() color group.

  \sa palette(), setPalette()
*/

const QColorGroup &QWidget::colorGroup() const
{
    if ( testWFlags(WState_Disabled) )
	return pal.disabled();
    else if ( qApp->focus_widget == this && focusPolicy() != NoFocus )
	return pal.active();
    else
	return pal.normal();
}

/*!
  \fn const QPalette &QWidget::palette() const
  Returns the widget palette.
  \sa setPalette(), colorGroup(), QApplication::palette()
*/

/*!
  Sets the widget palette to \e p. The widget background color is set to
  <code>colorGroup().background()</code>.

  If \a palettePropagation() is \c AllChildren or \c SamePalette,
  setPalette() calls setPalette() for children of the object, or those
  with whom the object shares the palette, respectively.  The default
  for QWidget is \a NoChildren, so setPalette() will not change the children's
  palettes.

  \sa QApplication::setPalette(), palette(), paletteChange(),
  colorGroup(), setBackgroundColor(), setPalettePropagation()
*/

void QWidget::setPalette( const QPalette &p )
{
    QPalette old = pal;
    pal = p;
    setBackgroundColorFromMode();
    paletteChange( old );
    PropagationMode m = palettePropagation();
    if ( m != NoChildren && children() ) {
	QObjectListIt it( *children() );
	QWidget *w;
	while( (w=(QWidget *)it.current()) != 0 ) {
	    ++it;
	    if ( w->isWidgetType() &&
		 ( m == AllChildren ||
		   old.isCopyOf( w->pal ) ) )
		w->setPalette( pal );
	}
    }
}

/*!
  \fn void QWidget::paletteChange( const QPalette &oldPalette )

  This virtual function is called from setPalette().  \e oldPalette is the
  previous palette; you can get the new palette from palette().

  Reimplement this function if your widget needs to know when its palette
  changes.  You will almost certainly need to update the widget using
  either repaint(TRUE) or update().

  The default implementation calls update().

  \sa setPalette(), palette(), repaint(), update()
*/

void QWidget::paletteChange( const QPalette & )
{
    update();
}


/*!
  \fn const QFont &QWidget::font() const

  Returns the font currently set for the widget.

  fontInfo() tells you what font is actually being used.

  \sa setFont(), fontInfo(), fontMetrics()
*/

/*!
  Sets the font for the widget.

  The fontInfo() function reports the actual font that is being used by the
  widget.

  This code fragment sets a 12 point helvetica bold font:
  \code
    QFont f("Helvetica", 12, QFont::Bold);
    setFont( f );
  \endcode

  If \a fontPropagation() is \c AllChildren or \c SameFont, setFont()
  calls setFont() for children of the object, or those with whom the
  object shares the font, respectively.  The default for QWidget
  is \a NoChildren, so setFont() will not change the children's fonts.

  \sa font(), fontChange(), fontInfo(), fontMetrics(), setFontPropagation()
*/

void QWidget::setFont( const QFont &font )
{
    QFont old = fnt;
    fnt = font;
    fnt.handle();				// force load font
    fontChange( old );
    PropagationMode m = fontPropagation();
    if ( m != NoChildren && children() ) {
	QObjectListIt it( *children() );
	QWidget *w;
	while( (w=(QWidget *)it.current()) != 0 ) {
	    ++it;
	    if ( w->isWidgetType() &&
		 ( m == AllChildren ||
		   old.isCopyOf( w->fnt ) ) )
		 w->setFont( fnt );
	}
    }
}


/*!
  \fn void QWidget::fontChange( const QFont &oldFont )

  This virtual function is called from setFont().  \e oldFont is the
  previous font; you can get the new font from font().

  Reimplement this function if your widget needs to know when its font
  changes.  You will almost certainly need to update the widget using
  either repaint(TRUE) or update().

  The default implementation calls update().

  \sa setFont(), font(), repaint(), update()
*/

void QWidget::fontChange( const QFont & )
{
    update();
}


#if QT_VERSION == 200
#error "Fix doc below. No longer automatic font metrics update"
#endif

/*!
  \fn QFontMetrics QWidget::fontMetrics() const
  Returns the font metrics for the widget.

  The font metrics object is automatically updated if somebody sets a new
  widget font. We have decided to change this policy in Qt 2.0: setting
  a new font for a widget should not affect existing QFontMetrics objects.

  \sa font(), fontInfo(), setFont()
*/

/*!
  \fn QFontInfo QWidget::fontInfo() const
  Returns the font info for the widget.

  The font info object is automatically updated if somebody sets a new
  widget font. We have decided to change this policy in Qt 2.0: setting
  a new font for a widget should not affect existing QFontInfo objects.

  \sa font(), fontMetrics(), setFont()
*/


/*!
  Returns the widget cursor.
  \sa setCursor()
*/

const QCursor &QWidget::cursor() const
{
    return curs;
}


/*!
  Returns the widget caption, or null if no caption has been set.
  \sa setCaption(), icon(), iconText()
*/

const char *QWidget::caption() const
{
    return extra ? extra->caption : 0;
}

/*!
  Returns the widget icon pixmap, or null if no icon has been set.
  \sa setIcon(), iconText(), caption()
*/

const QPixmap *QWidget::icon() const
{
    return extra ? extra->icon : 0;
}

/*!
  Returns the widget icon text, or null if no icon text has been set.
  \sa setIconText(), icon(), caption()
*/

const char *QWidget::iconText() const
{
    return extra ? extra->iconText : 0;
}


/*!
  \fn bool QWidget::hasMouseTracking() const
  Returns TRUE if mouse tracking is enabled for this widget, or FALSE
  if mouse tracking is disabled.
  \sa setMouseTracking()
*/

/*!
  \fn void QWidget::setMouseTracking( bool enable )
  Enables mouse tracking if \e enable is TRUE, or disables it if \e enable
  is FALSE.

  If mouse tracking is disabled (default), this widget only receives
  mouse move events when at least one mouse button is pressed down while
  the mouse is being moved.

  If mouse tracking is enabled, this widget receives mouse move events
  even if no buttons are pressed down.

  \sa hasMouseTracking(), mouseMoveEvent(),
    QApplication::setGlobalMouseTracking()
*/

#if !defined(_WS_X11_)
void QWidget::setMouseTracking( bool enable )
{
    if ( enable )
	setWFlags( WState_TrackMouse );
    else
	clearWFlags( WState_TrackMouse );
    return;
}
#endif // _WS_X11_


/*!  Sets this widget's focus proxy to \a w.

  Some widgets, such as QComboBox, can "have focus," but create a
  child widget to actually handle the focus.  QComboBox, for example,
  creates a QLineEdit.

  setFocusProxy() sets the widget which will actually get focus when
  "this widget" gets it.  If there is a focus proxy, focusPolicy(),
  setFocusPolicy(), setFocus() and hasFocus() all operate on the focus
  proxy.
*/

void QWidget::setFocusProxy( QWidget * w )
{
    if ( focusChild )
	disconnect( focusChild, SIGNAL(destroyed()),
		    this, SLOT(focusProxyDestroyed()) );

    if ( w ) {
	w->setFocusPolicy( focusPolicy() );
	focusChild = 0;
	setFocusPolicy( NoFocus );
	focusChild = w;
	connect( focusChild, SIGNAL(destroyed()),
		 this, SLOT(focusProxyDestroyed()) );
    } else {
	QWidget * pfc = focusChild;
	focusChild = 0;
	if ( pfc )
	    setFocusPolicy( pfc->focusPolicy() );
    }
}


/*!  Returns a pointer to the focus proxy, or 0 if there is no focus
  proxy.
  \sa setFocusProxy()
*/

QWidget * QWidget::focusProxy() const
{
    return focusChild; // ### watch out for deletes
}


/*!  Internal slot used to clean up if the focus proxy is destroyed.
  \sa setFocusProxy()
*/

void QWidget::focusProxyDestroyed()
{
    focusChild = 0;
    setFocusPolicy( NoFocus );
}


/*!
  Returns TRUE if this widget (or its focus proxy) has the keyboard
  input focus, otherwise FALSE.

  Equivalent to <code>qApp->focusWidget() == this</code>.

  \sa setFocus(), clearFocus(), setFocusPolicy(), QApplication::focusWidget()
*/

bool QWidget::hasFocus() const
{
    return qApp->focusWidget() == (focusProxy() ? focusProxy() : this);
}

/*!
  Gives the keyboard input focus to the widget (or its focus proxy).

  First, a \link focusOutEvent() focus out event\endlink is sent to the
  focus widget (if any) to tell it that it is about to loose the
  focus. Then a \link focusInEvent() focus in event\endlink is sent to
  this widget to tell it that it just received the focus.

  setFocus() gives focus to a widget regardless of its focus policy.
  However, QWidget::focusWidget() (which determines where
  Tab/shift-Tab) moves from) is changed only if the widget accepts
  focus.  This can be used to implement "hidden focus"; see
  focusNextPrevChild() for details.

  \warning If you call setFocus() in a function which may itself be
  called from focusOutEvent() or focusInEvent(), you may see infinite
  recursion.

  \sa hasFocus(), clearFocus(), focusInEvent(), focusOutEvent(),
  setFocusPolicy(), QApplication::focusWidget()
*/

void QWidget::setFocus()
{
    if ( !isEnabled() )
	return;

    if ( focusProxy() ) {
	focusProxy()->setFocus();
	return;
    }

    QFocusData * f = focusData(TRUE);
    if ( f->it.current() == this && qApp->focusWidget() == this )
	return;

    if ( isFocusEnabled() ) {
	if ( testWFlags(WState_TabToFocus) ) {
	    // move the tab focus pointer only if this widget can be
	    // tabbed to or from.
	    f->it.toFirst();
	    while ( f->it.current() != this && !f->it.atLast() )
		++f->it;
	    // at this point, the iterator should point to 'this'.  if it
	    // does not, 'this' must not be in the list - an error, but
	    // perhaps possible.  fix it.
	    if ( f->it.current() != this ) {
		f->focusWidgets.append( this );
		f->it.toLast();
	    }
	}
    }

    if ( isActiveWindow() ) {
	if ( qApp->focus_widget && qApp->focus_widget != this ) {
	    QWidget * prev = qApp->focus_widget;
	    qApp->focus_widget = this;
	    QFocusEvent out( Event_FocusOut );
	    QApplication::sendEvent( prev, &out );
	}

	qApp->focus_widget = this;
	QFocusEvent in( Event_FocusIn );
	QApplication::sendEvent( this, &in );
    } else {
	qApp->focus_widget = 0;
    }
}


/*!
  Takes keyboard input focus from the widget.

  If the widget has active focus, a \link focusOutEvent() focus out
  event\endlink is sent to this widget to tell it that it is about to
  loose the focus.

  This widget must enable focus setting in order to get the keyboard input
  focus, i.e. it must call setFocusPolicy().

  \sa hasFocus(), setFocus(), focusInEvent(), focusOutEvent(),
  setFocusPolicy(), QApplication::focusWidget()
*/

void QWidget::clearFocus()
{
    if ( focusProxy() ) {
	focusProxy()->clearFocus();
    } else {
	QWidget * w = qApp->focusWidget();
	if ( w && w->focusWidget() == this ) {
	    // clear active focus
	    qApp->focus_widget = 0;
	    QFocusEvent out( Event_FocusOut );
	    QApplication::sendEvent( w, &out );
	}
    }
}


/*!
  Finds a new widget to give the keyboard focus to, as appropriate for
  Tab/shift-Tab, and returns TRUE if is can find a new widget and
  FALSE if it can't,

  If \a next is true, this function searches "forwards", if \a next is
  FALSE, "backwards".

  Sometimes, you will want to reimplement this function.  For example,
  a web browser might reimplement it to move its "current active link"
  forwards or backwards, and call QWidget::focusNextPrevChild() only
  when it reaches the last/first.

  Child widgets call focusNextPrevChild() on their parent widgets, and
  only the top-level widget will thus make the choice of where to redirect
  focus.  By overriding this method for an object, you thus gain control
  of focus traversal for all child widgets.

  \sa focusData()
*/

bool QWidget::focusNextPrevChild( bool next )
{
    QWidget* p = parentWidget();
    if ( !testWFlags(WType_TopLevel) && p )
	return p->focusNextPrevChild(next);

    QFocusData *f = focusData( TRUE );

    QWidget *startingPoint = f->it.current();
    QWidget *candidate = 0;
    QWidget *w = next ? f->focusWidgets.last() : f->focusWidgets.first();

    do {
	if ( w && w != startingPoint &&
	     w->testWFlags( WState_TabToFocus ) && !w->focusProxy() &&
	     w->isVisibleToTLW() && w->isEnabledToTLW() )
	    candidate = w;
	w = next ? f->focusWidgets.prev() : f->focusWidgets.next();
    } while( w && !(candidate && w==startingPoint) );

    if ( !candidate )
	return FALSE;

    candidate->setFocus();
    return TRUE;
}


/*!
  Returns the focus widget in this widget's window.  This
  is not the same as QApplication::focusWidget(), which returns the
  focus widget in the currently active window.
*/

QWidget *QWidget::focusWidget() const
{
    QWidget *that = (QWidget *)this;		// mutable
    QFocusData *f = that->focusData( FALSE );
    if ( f && f->focusWidgets.count() && f->it.current() == 0 )
	f->it.toFirst();
    return f ? f->it.current() : 0;
}


/*!
  Returns a pointer to the focus data for this widget's top-level
  widget.

  Focus data always belongs to the top-level widget.  The focus data
  list contains all the widgets in this top-level widget that can
  accept focus, in tab order.  An iterator points to the current focus
  widget (focusWidget() returns a pointer to this widget).

  This information is useful for implementing advanced versions
  of focusNextPrevChild().
*/
QFocusData * QWidget::focusData()
{
    return focusData(TRUE);
}

/*!
  Internal function which lets us not create it too.
*/
QFocusData * QWidget::focusData( bool create )
{
    QWidget * tlw = topLevelWidget();
    QWExtra * ed = tlw->extraData();
    if ( create && !ed ) {
	tlw->createExtra();
	ed = tlw->extraData();
    }
    if ( create && !ed->focusData ) {
	ed->focusData = new QFocusData;
    }
    return ed ? ed->focusData : 0;
}


/*!
  \fn void QWidget::setSizeGrip(bool sizegrip)

  Informs the underlying window system that this widget is a size grip
  (if sizegrip is TRUE). An example is the nifty decoration in the
  bottom right corner of a QStatusBar.

  This function does yet nothing under Windows. Under X11, the window
  manager has to support the QT_SIZEGRIP protocol.
*/


/*!
  Moves the \a second widget around the ring of focus widgets
  so that keyboard focus moves from \a first widget to \a second
  widget when Tab is pressed.

  Note that since the tab order of the \e second widget is changed,
  you should order a chain like this:

  \code
    setTabOrder(a, b ); // a to b
    setTabOrder(b, c ); // a to b to c
    setTabOrder(c, d ); // a to b to c to d
  \endcode

  not like this:

  \code
    setTabOrder(c, d); // c to d
    setTabOrder(a, b); // a to b AND c to d
    setTabOrder(b, c); // a to b to c, but not c to d
  \endcode

  If either \a first or \a second has a focus proxy, setTabOrder()
  substitutes its/their proxies.

  \sa setFocusPolicy(), setFocusProxy()
*/
void QWidget::setTabOrder( QWidget* first, QWidget *second )
{
    if ( !first || !second )
	return;

    while ( first->focusProxy() )
	first = first->focusProxy();
    while ( second->focusProxy() )
	second = second->focusProxy();

    QFocusData *f = first->focusData( TRUE );
    bool focusThere = (f->it.current() == second );
    f->focusWidgets.removeRef( second );
    if ( f->focusWidgets.findRef( first ) >= 0 )
	f->focusWidgets.insert( f->focusWidgets.at() + 1, second );
    else
	f->focusWidgets.append( second );
    if ( focusThere ) { // reset iterator so tab will work appropriately
	f->it.toFirst();
	while( f->it.current() && f->it.current() != second )
	    ++f->it;
    }
}

/*!
  Moves the relevant widgets from the this window's tab chain to
  that of \a parent, if there's anything to move and we're really
  moving

  \sa recreate()
*/

void QWidget::reparentFocusWidgets( QWidget * parent )
{
    if ( focusData() &&
	 ( parent == 0 ? parentObj != 0
		       : parent->topLevelWidget() != topLevelWidget() ) )
    {
	QFocusData * from = focusData();
	from->focusWidgets.first();
	QFocusData * to;
	if ( parent ) {
	    to = parent->focusData( TRUE );
	} else {
	    createExtra();
	    to = extra->focusData = new QFocusData;
	}

	do {
	    QWidget * pw = from->focusWidgets.current();
	    while( pw && pw != this )
		pw = pw->parentWidget();
	    if ( pw == this ) {
		QWidget * w = from->focusWidgets.take();
		if ( w == from->it.current() )
		    // probably best to clear keyboard focus, or
		    // the user might become rather confused
		    w->clearFocus();
		to->focusWidgets.append( w );
	    } else {
		from->focusWidgets.next();
	    }
	} while( from->focusWidgets.current() );

	if ( parentObj == 0 ) {
	    // this widget is no longer a top-level widget, so get rid
	    // of old focus data
	    delete extra->focusData;
	    extra->focusData = 0;
	}
    }
}


/*!
  \internal
  Sets the frame rectangle and recomputes the client rectangle.

  The frame rectangle is the geometry of this widget including any
  decorative borders, in its parent's coordinate system.

  The client rectangle is the geometry of just this widget in its
  parent's coordinate system.
*/

void QWidget::setFRect( const QRect &r )
{
    crect.setLeft( crect.left() + r.left() - frect.left() );
    crect.setTop( crect.top() + r.top() - frect.top() );
    crect.setRight( crect.right() + r.right() - frect.right() );
    crect.setBottom( crect.bottom() + r.bottom() - frect.bottom() );
    frect = r;
}

/*!
  \internal
  Sets the client rectangle and recomputes the frame rectangle.

  The client rectangle is the geometry of just this widget in its
  parent's coordinate system.

  The frame rectangle is the geometry of this widget including any
  decorative borders, in its parent's coordinate system.
*/

void QWidget::setCRect( const QRect &r )
{
    frect.setLeft( frect.left() + r.left() - crect.left() );
    frect.setTop( frect.top() + r.top() - crect.top() );
    frect.setRight( frect.right() + r.right() - crect.right() );
    frect.setBottom( frect.bottom() + r.bottom() - crect.bottom() );
    crect = r;
}


/*!
  \fn bool QWidget::isFocusEnabled() const

  Returns TRUE if the widget accepts keyboard focus, or FALSE if it does
  not.

  Keyboard focus is initially disabled (i.e. focusPolicy() ==
  \c QWidget::NoFocus).

  You must enable keyboard focus for a widget if it processes keyboard
  events. This is normally done from the widget's constructor.  For
  instance, the QLineEdit constructor calls
  setFocusPolicy(\c QWidget::StrongFocus).

  \sa setFocusPolicy(), focusInEvent(), focusOutEvent(), keyPressEvent(),
  keyReleaseEvent(), isEnabled()
*/

/*!
  \fn QWidget::FocusPolicy QWidget::focusPolicy() const

  Returns \c QWidget::TabFocus if the widget accepts focus by tabbing, \c
  QWidget::ClickFocus if the widget accepts focus by clicking, \c
  QWidget::StrongFocus if it accepts both and \c QWidget::NoFocus if it
  does not accept focus at all.

  \sa isFocusEnabled(), setFocusPolicy(), focusInEvent(), focusOutEvent(),
  keyPressEvent(), keyReleaseEvent(), isEnabled()
*/

/*!
  Enables or disables the keyboard focus for the widget.

  The keyboard focus is initially disabled (i.e. \e policy ==
  \c QWidget::NoFocus).

  You must enable keyboard focus for a widget if it processes keyboard
  events. This is normally done from the widget's constructor.  For
  instance, the QLineEdit constructor calls
  setFocusPolicy(\c QWidget::StrongFocus).

  The \e policy can be:
  <ul>
  <li> \c QWidget::TabFocus, the widget accepts focus by tabbing.
  <li> \c QWidget::ClickFocus, the widget accepts focus by clicking.
  <li> \c QWidget::StrongFocus, the widget accepts focus by both tabbing
  and clicking.
  <li> \c QWidget::NoFocus, the widget does not accept focus
  </ul>

  As a special case to support applications not utilizing focus,
  \link isTopLevel() Top-level widgets \endlink that have
  NoFocus policy will receive focus events and
  gain keyboard events.

  \sa isFocusEnabled(), focusInEvent(), focusOutEvent(), keyPressEvent(),
  keyReleaseEvent(), isEnabled()
*/

void QWidget::setFocusPolicy( FocusPolicy policy )
{
    if ( focusProxy() )
	focusProxy()->setFocusPolicy( policy );

    if ( policy ) {
	QFocusData * f = focusData( TRUE );
	if ( f->focusWidgets.findRef( this ) < 0 )
 	    f->focusWidgets.append( this );
    }

    if ( policy & TabFocus )
	setWFlags( WState_TabToFocus );
    else
	clearWFlags( WState_TabToFocus );

    if ( policy & ClickFocus )
	setWFlags( WState_ClickToFocus );
    else
	clearWFlags( WState_ClickToFocus );
}


void QWidget::setAcceptFocus( bool enable )
{
    if ( enable ) {
	QFocusData * f = focusData( TRUE );
	if ( f->focusWidgets.findRef( this ) < 0 )
 	    f->focusWidgets.append( this );
	setWFlags( WState_TabToFocus | WState_ClickToFocus );
    } else {
	clearWFlags( WState_TabToFocus | WState_ClickToFocus );
    }
}


/*!
  \fn bool QWidget::isUpdatesEnabled() const
  Returns TRUE if updates are enabled, otherwise FALSE.
  \sa setUpdatesEnabled()
*/

/*!
  Enables widget updates if \e enable is TRUE, or disables widget updates
  if \e enable is FALSE.

  Calling update() and repaint() has no effect if updates are disabled.
  Paint events from the window system are processed as normally even if
  updates are disabled.

  This function is normally used to disable updates for a short period of
  time, for instance to avoid screen flicker during large changes.

  Example:
  \code
    setUpdatesEnabled( FALSE );
    bigVisualChanges();
    setUpdatesEnabled( TRUE );
    repaint();
  \endcode

  \sa isUpdatesEnabled(), update(), repaint(), paintEvent()
*/

void QWidget::setUpdatesEnabled( bool enable )
{
    if ( enable )
	clearWFlags( WState_BlockUpdates );
    else
	setWFlags( WState_BlockUpdates );
}


/*
  Returns TRUE if there's no visible top level window (except the desktop).
  This is an internal function used by QWidget::hide().
*/

static bool noVisibleTLW()
{
    QWidgetList *list   = qApp->topLevelWidgets();
    QWidget     *widget = list->first();
    while ( widget ) {
	if ( widget->isVisible() && !widget->isDesktop() )
	    break;
	widget = list->next();
    }
    delete list;
    return widget == 0;
}


void qt_enter_modal( QWidget * );		// defined in qapp_xxx.cpp
void qt_leave_modal( QWidget * );		// --- "" ---
bool qt_modal_state();				// --- "" ---
void qt_open_popup( QWidget * );		// --- "" ---
void qt_close_popup( QWidget * );		// --- "" ---


/*!
  Shows the widget and its child widgets.

  If its size or position has changed, Qt guarantees that a widget gets
  move and resize events just before the widget is shown.

  \sa hide(), iconify(), isVisible()
*/

void QWidget::show()
{
    if ( testWFlags(WState_Visible) )
	return;
    if ( extra ) {
	int w = crect.width();
	int h = crect.height();
	if ( w < extra->minw || h < extra->minh ||
	     w > extra->maxw || h > extra->maxh ) {
	    w = QMAX( extra->minw, QMIN( w, extra->maxw ));
	    h = QMAX( extra->minh, QMIN( h, extra->maxh ));
	    resize( w, h );			// deferred resize
	}
    }
    sendDeferredEvents();
    if ( children() ) {
	QObjectListIt it(*children());
	register QObject *object;
	QWidget *widget;
	while ( it ) {				// show all widget children
	    object = it.current();		//   (except popups)
	    ++it;
	    if ( object->isWidgetType() ) {
		widget = (QWidget*)object;
		if ( !widget->testWFlags(WState_DoHide) )
		    widget->show();
	    }
	}
    }
    if ( testWFlags(WStyle_Tool) ) {
	raise();
    } else if ( testWFlags(WType_TopLevel) && !testWFlags(WType_Popup) ) {
	while ( QApplication::activePopupWidget() )
	    QApplication::activePopupWidget()->hide();
    }

    if ( testWFlags(WType_Modal) ) {
	// qt_enter_modal *before* show, otherwise the initial
	// stacking might be wrong
	qt_enter_modal( this ); 
	showWindow();
    }
    else {
	showWindow();
	if ( testWFlags(WType_Popup) )
	    qt_open_popup( this );
    }
}


/*!
  Hides the widget.

  The QApplication::lastWindowClosed() signal is emitted when the last
  visible top level widget is hidden,

  \sa show(), iconify(), isVisible()
*/

void QWidget::hide()
{
    setWFlags( WState_DoHide );
    if ( !testWFlags(WState_Visible) )
	return;

    if ( testWFlags(WType_Modal) )
	qt_leave_modal( this );
    else if ( testWFlags(WType_Popup) )
	qt_close_popup( this );

    hideWindow();

    clearWFlags( WState_Visible );

    // next bit tries to move the focus if the focus widget is now
    // hidden.
    if ( qApp && qApp->focusWidget() == this )
	focusNextPrevChild( TRUE );

    // ### Can these ever be useful?
    cancelMove();
    cancelResize();

    if ( isTopLevel() ) {			// last TLW hidden?
	if ( qApp->receivers(SIGNAL(lastWindowClosed())) && noVisibleTLW() )
	    emit qApp->lastWindowClosed();
    }

#if QT_VERSION == 200
#error "Do this inherits-stuff by overriding hide() in QDialog"
#error "Be sure to test for testWFlags(WState_Visible) or anything"
#error " else that makes QWidget::hide() return early."
#endif
    if ( testWFlags(WType_Modal)
      && inherits("QDialog") ) qApp->exit_loop();

    QHideEvent e(FALSE);
    QApplication::sendEvent( this, &e );
}


/*!
  Closes this widget. Returns TRUE if the widget was closed, otherwise
  FALSE.

  First it sends the widget a QCloseEvent. The widget is \link hide()
  hidden\endlink if it \link QCloseEvent::accept() accepts\endlink the
  close event. The default implementation of QWidget::closeEvent()
  accepts the close event.

  If \e forceKill is TRUE, the widget is deleted whether it accepts
  the close event or not.

  The application is \link QApplication::quit() terminated\endlink when
  the \link QApplication::setMainWidget() main widget\endlink is closed.

  The QApplication::lastWindowClosed() signal is emitted when the last
  visible top level widget is closed.

  \sa closeEvent(), QCloseEvent, hide(), QApplication::quit(),
  QApplication::setMainWidget()
*/

bool QWidget::close( bool forceKill )
{
    WId	 id	= winId();
    bool isMain = qApp->mainWidget() == this;
    QCloseEvent e;
    bool accept = QApplication::sendEvent( this, &e );
    if ( !QWidget::find(id) ) {			// widget was deleted
	if ( isMain )
	    qApp->quit();
	return TRUE;
    }
    if ( forceKill )
	accept = TRUE;
    if ( accept ) {
	hide();
	if ( isMain )
	    qApp->quit();
	else if ( forceKill || testWFlags(WDestructiveClose) )
	    delete this;
    }
    return accept;
}


/*!
  \fn bool QWidget::isVisible() const

  Returns TRUE if the widget itself is set to visible status, or else
  FALSE.  Calling show() sets the widget to visible status; calling
  hide() sets it to hidden status. Iconified top-level widgets also
  have hidden status.

  If a widget is set to visible status, but its parent widget is set
  to hidden status, this function returns TRUE.  isVisibleToTLW()
  looks at the visibility status of the parent widgets up to the
  top level widget.

  This function returns TRUE if the widget currently is obscured by
  other windows on the screen, but would be visible if moved.

  \sa show(), hide(), isVisibleToTLW()
*/


/*!
  Returns TRUE if this widget and every parent up to but excluding
  \a ancestor is visible, otherwise returns FALSE.

  This function returns TRUE if the widget it is obscured by other
  windows on the screen, but would be visible if moved.

  \sa show() hide() isVisible()
*/

bool QWidget::isVisibleTo(QWidget* ancestor) const
{
    const QWidget * w = this;
    while ( w
	    && w->isVisible()
	    && !w->isTopLevel()
	    && w->parentWidget()
	    && w->parentWidget()!=ancestor )
	w = w->parentWidget();
    return w->isVisible();
}


/*!
  Returns TRUE if this widget and every parent up to the \link
  topLevelWidget() top level widget \endlink is visible, otherwise
  returns FALSE.

  This function returns TRUE if the widget it is obscured by other
  windows on the screen, but would be visible if moved.

  This is equivalent to isVisibleTo(0).

  \sa show(), hide(), isVisible()
*/

bool QWidget::isVisibleToTLW() const
{
    return isVisibleTo( 0 );
}


/*!
  Adjusts the size of the widget to fit the contents.

  Uses sizeHint() if valid (i.e if the size hint's width and height are
  equal to or greater than 0), otherwise sets the size to the children
  rectangle (the union of all child widget geometries).

  \sa sizeHint(), childrenRect()
*/

void QWidget::adjustSize()
{
    QSize s = sizeHint();
    if ( s.isValid() ) {
	resize( s );
	return;
    }
    QRect r = childrenRect();			// get children rectangle
    if ( r.isNull() )				// probably no widgets
	return;
    resize( r.width()+2*r.x(), r.height()+2*r.y() );
}


/*!
  Returns a recommended size for the widget, or an invalid size if
  no size is recommended.

  The default implementation returns an invalid size.

  \sa QSize::isValid(), resize(), setMinimumSize()
*/

QSize QWidget::sizeHint() const
{
    return QSize( -1, -1 );
}


/*!
  \fn QWidget *QWidget::parentWidget() const
  Returns a pointer to the parent of this widget, or a null pointer if
  it does not have any parent widget.
*/

/*!
  \fn bool QWidget::testWFlags( WFlags n ) const

  Returns non-zero if any of the widget flags in \e n are set. The
  widget flags are listed in qwindowdefs.h, and are strictly for
  internal use.

  \internal

  Widget state flags:
  <dl compact>
  <dt>WState_Created<dd> The widget has a valid winId().
  <dt>WState_Disabled<dd> Disables mouse and keyboard events.
  <dt>WState_Visible<dd> show() has been called.
  <dt>WState_DoHide<dd> hide() has been called before first show().
  <dt>WState_AcceptFocus<dd> The widget can take keyboard focus.
  <dt>WState_TrackMouse<dd> Mouse tracking is enabled.
  <dt>WState_BlockUpdates<dd> Repaints and updates are disabled.
  <dt>WState_PaintEvent<dd> Currently processing a paint event.
  </dl>

  Widget type flags:
  <dl compact>
  <dt>WType_TopLevel<dd> Top-level widget (not a child).
  <dt>WType_Modal<dd> Modal widget, implies \c WType_TopLevel.
  <dt>WType_Popup<dd> Popup widget, implies \c WType_TopLevel.
  <dt>WType_Desktop<dd> Desktop widget (root window), implies
	\c WType_TopLevel.
  </dl>

  Window style flags (for top-level widgets):
  <dl compact>
  <dt>WStyle_Customize<dd> Custom window style.
  <dt>WStyle_NormalBorder<dd> Normal window border.
  <dt>WStyle_DialogBorder<dd> Thin dialog border.
  <dt>WStyle_NoBorder<dd> No window border.
  <dt>WStyle_Title<dd> The window has a title.
  <dt>WStyle_SysMenu<dd> The window has a system menu
  <dt>WStyle_Minimize<dd> The window has a minimize box.
  <dt>WStyle_Maximize<dd> The window has a maximize box.
  <dt>WStyle_MinMax<dd> Equals (\c WStyle_Minimize | \c WStyle_Maximize).
  <dt>WStyle_Tool<dd> The window is a tool window.
  </dl>

  Misc. flags:
  <dl compact>
  <dt>WCursorSet<dd> Flags that a cursor has been set.
  <dt>WDestructiveClose<dd> The widget is deleted when its closed.
  <dt>WPaintDesktop<dd> The widget wants desktop paint events.
  <dt>WPaintUnclipped<dd> Paint without clipping child widgets.
  <dt>WPaintClever<dd> The widget wants every update rectangle.
  <dt>WConfigPending<dd> Config (resize,move) event pending.
  <dt>WResizeNoErase<dd> Widget resizing should not erase the widget.
  <dt>WRecreated<dd> The widet has been recreated.
  <dt>WExportFontMetrics<dd> Somebody refers the font's metrics.
  <dt>WExportFontInfo<dd> Somebody refers the font's info.
  </dl>
*/


/*****************************************************************************
  QWidget event handling
 *****************************************************************************/


/*!
  This is the main event handler. You may reimplement this function
  in a subclass, but we recommend using one of the specialized event
  handlers instead.

  The main event handler first passes an event through all \link
  QObject::installEventFilter() event filters\endlink that have been
  installed.  If none of the filters intercept the event, it calls one
  of the specialized event handlers.

  Key press/release events are treated differently from other events.
  event() checks for Tab and shift-Tab and tries to move the focus
  appropriately.  If there is no widget to move the focus to (or the
  key press is not Tab or shift-Tab), event() calls keyPressEvent().

  This function returns TRUE if it is able to pass the event over to
  someone, or FALSE if nobody wanted the event.

  \sa closeEvent(), focusInEvent(), focusOutEvent(), enterEvent(),
  keyPressEvent(), keyReleaseEvent(), leaveEvent(),
  mouseDoubleClickEvent(), mouseMoveEvent(), mousePressEvent(),
  mouseReleaseEvent(), moveEvent(), paintEvent(),
  resizeEvent(), QObject::event(), QObject::timerEvent()
*/

bool QWidget::event( QEvent *e )
{
    if ( eventFilters ) {			// try filters
	if ( activate_filters(e) )		// stopped by a filter
	    return TRUE;
    }

    switch ( e->type() ) {

	case Event_Timer:
	    timerEvent( (QTimerEvent*)e );
	    break;

	case Event_MouseMove:
	    mouseMoveEvent( (QMouseEvent*)e );
	    break;

	case Event_MouseButtonPress:
	    mousePressEvent( (QMouseEvent*)e );
	    break;

	case Event_MouseButtonRelease:
	    mouseReleaseEvent( (QMouseEvent*)e );
	    break;

	case Event_MouseButtonDblClick:
	    mouseDoubleClickEvent( (QMouseEvent*)e );
	    break;

	case Event_KeyPress: {
	    QKeyEvent *k = (QKeyEvent *)e;
	    bool res = FALSE;
	    if ( k->key() == Key_Backtab ||
		 (k->key() == Key_Tab && (k->state() & ShiftButton)) )
		res = focusNextPrevChild( FALSE );
	    else if ( k->key() == Key_Tab )
		res = focusNextPrevChild( TRUE );
	    if ( res )
		break;
	    QWidget *w = this;
	    while ( w ) {
		k->accept();
		w->keyPressEvent( k );
		if ( k->isAccepted() || w->isTopLevel() )
		    break;
		w = w->parentWidget();
	    }
	    }
	    break;

	case Event_KeyRelease: {
	    QKeyEvent *k = (QKeyEvent *)e;
	    QWidget *w = this;
	    while ( w ) {
		w->keyReleaseEvent( k );
		if ( k->isAccepted() || w->isTopLevel() )
		    break;
		w = w->parentWidget();
	    }
	    }
	    break;

	case Event_FocusIn:
	    focusInEvent( (QFocusEvent*)e );
	    break;

	case Event_FocusOut:
	    focusOutEvent( (QFocusEvent*)e );
	    break;

	case Event_Enter:
	    enterEvent( e );
	    break;

	case Event_Leave:
	     leaveEvent( e );
	    break;

	case Event_Paint:
	    paintEvent( (QPaintEvent*)e );
	    break;

	case Event_Move:
	    moveEvent( (QMoveEvent*)e );
	    break;

	case Event_Resize:
	    resizeEvent( (QResizeEvent*)e );
	    break;

	case Event_Close: {
	    QCloseEvent *c = (QCloseEvent *)e;
	    closeEvent( c );
	    if ( !c->isAccepted() )
		return FALSE;
	    }
	    break;

	default:
	    return FALSE;
    }
    return TRUE;
}


/*!
  This event handler can be reimplemented in a subclass to receive
  mouse move events for the widget.

  If mouse tracking is switched off, mouse move events only occur if a
  mouse button is down while the mouse is being moved.	If mouse
  tracking is switched on, mouse move events occur even if no mouse
  button is down.

  QMouseEvent::pos() reports the position of the mouse cursor, relative to
  this widget.  For press and release events, the position is usually
  the same as the position of the last mouse move event, but it might be
  different if the user moves and clicks the mouse fast.  This is
  a feature of the underlying window system, not Qt.

  The default implementation does nothing.

  \sa setMouseTracking(), mousePressEvent(), mouseReleaseEvent(),
  mouseDoubleClickEvent(), event(), QMouseEvent
*/

void QWidget::mouseMoveEvent( QMouseEvent * )
{
}

/*!
  This event handler can be reimplemented in a subclass to receive
  mouse press events for the widget.

  The default implementation does nothing.

  If you create new widgets in the mousePressEvent() the
  mouseReleaseEvent() may not end up where you expect, depending on the
  underlying window system (or X11 window manager), the widgets'
  location and maybe more.

  \sa mouseReleaseEvent(), mouseDoubleClickEvent(),
  mouseMoveEvent(), event(),  QMouseEvent
*/

void QWidget::mousePressEvent( QMouseEvent * )
{
}

/*!
  This event handler can be reimplemented in a subclass to receive
  mouse release events for the widget.

  The default implementation does nothing.

  \sa mouseReleaseEvent(), mouseDoubleClickEvent(),
  mouseMoveEvent(), event(),  QMouseEvent
*/

void QWidget::mouseReleaseEvent( QMouseEvent * )
{
}

/*!
  This event handler can be reimplemented in a subclass to receive
  mouse double click events for the widget.

  The default implementation generates a normal mouse press event.

  Note that the widgets gets a mousePressEvent() and a mouseReleaseEvent()
  before the mouseDoubleClickEvent().

  \sa mousePressEvent(), mouseReleaseEvent()
  mouseMoveEvent(), event(),  QMouseEvent
*/

void QWidget::mouseDoubleClickEvent( QMouseEvent *e )
{
    mousePressEvent( e );			// try mouse press event
}


/*!
  This event handler can be reimplemented in a subclass to receive
  key press events for the widget.

  A widget must \link setFocusPolicy() accept focus\endlink initially
  and \link hasFocus() have focus\endlink in order to receive a key press
  event.

  If you reimplement this handler, it is very important that you \link
  QKeyEvent ignore()\endlink the press if you do not understand it, so
  that the widget's parent can interpret it.

  The default implementation ignores the event.

  As a special case to support applications not utilizing focus,
  \link isTopLevel() Top-level widgets \endlink that have
  NoFocus policy will receive keyboard events.

  \sa keyReleaseEvent(), QKeyEvent::ignore(), setFocusPolicy(),
  focusInEvent(), focusOutEvent(), event(), QKeyEvent
*/

void QWidget::keyPressEvent( QKeyEvent *e )
{
    e->ignore();
}

/*!
  This event handler can be reimplemented in a subclass to receive
  key release events for the widget.

  A widget must \link setFocusPolicy() accept focus\endlink initially
  and \link hasFocus() have focus\endlink in order to receive a key
  release event.

  If you reimplement this handler, it is very important that you \link
  QKeyEvent ignore()\endlink the release if you do not understand it,
  so that the widget's parent can interpret it.

  The default implementation ignores the event.

  \sa keyPressEvent(), QKeyEvent::ignore(), setFocusPolicy(),
  focusInEvent(), focusOutEvent(), event(), QKeyEvent
*/

void QWidget::keyReleaseEvent( QKeyEvent *e )
{
    e->ignore();
}

/*!
  This event handler can be reimplemented in a subclass to receive
  keyboard focus events (focus received) for the widget.

  A widget must \link setFocusPolicy() accept focus\endlink initially in
  order to receive focus events.

  The default implementation calls repaint() since the widget's \link
  QColorGroup color group\endlink changes from normal to active.  You
  may want to call repaint(FALSE) to reduce flicker in any reimplementation.

  As a special case to support applications not utilizing focus,
  \link isTopLevel() Top-level widgets \endlink that have
  \link focusPolicy() NoFocus policy\endlink will receive focus events and
  gain keyboard events, but the repaint is not done by default.

  \sa focusOutEvent(), setFocusPolicy(),
  keyPressEvent(), keyReleaseEvent(), event(), QFocusEvent
*/

void QWidget::focusInEvent( QFocusEvent * )
{
    if ( focusPolicy() != NoFocus || !isTopLevel() )
	repaint();
}

/*!
  This event handler can be reimplemented in a subclass to receive
  keyboard focus events (focus lost) for the widget.

  A widget must \link setFocusPolicy() accept focus\endlink initially in
  order to receive focus events.

  The default implementation calls repaint() since the widget's \link
  QColorGroup color group\endlink changes from active to normal.  You
  may want to call repaint(FALSE) to reduce flicker in any reimplementation.

  \sa focusInEvent(), setFocusPolicy(),
  keyPressEvent(), keyReleaseEvent(), event(), QFocusEvent
*/

void QWidget::focusOutEvent( QFocusEvent * )
{
    if ( focusPolicy() != NoFocus || !isTopLevel() )
	repaint();
}

/*!
  This event handler can be reimplemented in a subclass to receive
  widget enter events.

  An event is sent to the widget when the mouse cursor enters the widget.

  The default implementation does nothing.

  \sa leaveEvent(), mouseMoveEvent(), event()
*/

void QWidget::enterEvent( QEvent * )
{
}

/*!
  This event handler can be reimplemented in a subclass to receive
  widget leave events.

  A leave event is sent to the widget when the mouse cursor leaves
  the widget.

  The default implementation does nothing.

  \sa enterEvent(), mouseMoveEvent(), event()
*/

void QWidget::leaveEvent( QEvent * )
{
}

/*!
  This event handler can be reimplemented in a subclass to receive
  widget paint events.	Actually, it more or less \e must be
  reimplemented.

  The default implementation does nothing.

  When the paint event occurs, the update rectangle QPaintEvent::rect()
  normally has been cleared to the background color or pixmap. An
  exception is repaint() with erase=FALSE.

  For many widgets it is sufficient to redraw the entire widget each time,
  but some need to consider the update rectangle to avoid flicker or slow
  update.

  Pixmaps can also be used to implement flicker-free update.

  update() and repaint() can be used to force a paint event.

  \sa event(), repaint(), update(), QPainter, QPixmap, QPaintEvent
*/

void QWidget::paintEvent( QPaintEvent * )
{
}


/*!
  This event handler can be reimplemented in a subclass to receive
  widget move events.  When the widget receives this event, it is
  already at the new position.

  The old position is accessible through QMoveEvent::oldPos().

  The default implementation does nothing.

  \sa resizeEvent(), event(), move(), QMoveEvent
*/

void QWidget::moveEvent( QMoveEvent * )
{
}


/*!
  This event handler can be reimplemented in a subclass to receive
  widget resize events.	 When resizeEvent() is called, the widget
  already has its new geometry.

  The old size is accessible through QResizeEvent::oldSize().

  The default implementation does nothing.

  \sa moveEvent(), event(), resize(), QResizeEvent
*/

void QWidget::resizeEvent( QResizeEvent * )
{
}

/*!
  This event handler can be reimplemented in a subclass to receive
  widget close events.

  The default implementation calls e->accept(), which hides this widget.
  See the QCloseEvent documentation for more details.

  \sa event(), hide(), close(), QCloseEvent
*/

void QWidget::closeEvent( QCloseEvent *e )
{
    e->accept();
}


#if defined(_WS_MAC_)

/*!
  This special event handler can be reimplemented in a subclass to receive
  native Macintosh events.

  If the event handler returns FALSE, this native event is passed back to
  Qt, which translates the event into a Qt event and sends it to the
  widget.  If the event handler returns TRUE, the event is stopped.

  \warning This function is not portable.

  QApplication::macEventFilter()
*/

bool QWidget::macEvent( MSG * )
{
    return FALSE;
}

#elif defined(_WS_WIN_)

/*!
  This special event handler can be reimplemented in a subclass to receive
  native Windows events.

  If the event handler returns FALSE, this native event is passed back to
  Qt, which translates the event into a Qt event and sends it to the
  widget.  If the event handler returns TRUE, the event is stopped.

  \warning This function is not portable.

  QApplication::winEventFilter()
*/

bool QWidget::winEvent( MSG * )
{
    return FALSE;
}

#elif defined(_WS_PM_)

/*!
  This special event handler can be reimplemented in a subclass to receive
  native OS/2 Presentation Manager events.

  If the event handler returns FALSE, this native event is passed back to
  Qt, which translates the event into a Qt event and sends it to the
  widget.  If the event handler returns TRUE, the event is stopped.

  \warning This function is not portable.

  QApplication::pmEventFilter()
*/

bool QWidget::pmEvent( QMSG * )
{
    return FALSE;
}

#elif defined(_WS_X11_)

/*!
  This special event handler can be reimplemented in a subclass to receive
  native X11 events.

  If the event handler returns FALSE, this native event is passed back to
  Qt, which translates the event into a Qt event and sends it to the
  widget.  If the event handler returns TRUE, the event is stopped.

  \warning This function is not portable.

  QApplication::x11EventFilter()
*/

bool QWidget::x11Event( XEvent * )
{
    return FALSE;
}

#endif


/*!  Returns the font propagation mode of this widget.  The default
  font propagation mode is \c NoChildren, but you can set it to \a
  SameFont or \a AllChildren.

  \sa setFontPropagation()
*/

QWidget::PropagationMode QWidget::fontPropagation() const
{
    return extra ? (PropagationMode)extra->propagateFont : NoChildren;
}


/*!  Sets the font propagation mode to \a m.

  if \a m is \c NoChildren (the default), setFont() does not change
  any children's fonts.  If it is \c SameFont, setFont() changes the
  font of the children that have the exact same font as this widget
  (see \l QFont::isCopyOf() for details).  If it is \c AllChildren,
  setFont() changes the font of all children.

  \sa fontPropagation() setFont() setPalettePropagation()
*/

void QWidget::setFontPropagation( PropagationMode m )
{
    createExtra();
    extra->propagateFont = (int)m;
}


/*!  Returns the palette propagation mode of this widget.  The default
  palette propagation mode is \c NoChildren, but you can set it to \a
  SamePalette or \a AllChildren.

  \sa setPalettePropagation()
*/

QWidget::PropagationMode QWidget::palettePropagation() const
{
    return extra ? (PropagationMode)extra->propagatePalette : NoChildren;
}


/*!  Sets the palette propagation mode to \a m.

  if \a m is \c NoChildren (the default), setPalette() does not change
  any children's palettes.  If it is \c SamePalette, setPalette()
  changes the palette of the children that have the exact same palette
  as this widget (see \l QPalette::isCopyOf() for details).  If it is
  \c AllChildren, setPalette() changes the palette of all children.

  \sa palettePropagation() setPalette() setFontPropagation()
*/

void QWidget::setPalettePropagation( PropagationMode m )
{
    createExtra();
    extra->propagatePalette = (int)m;
}
