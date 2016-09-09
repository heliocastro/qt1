/****************************************************************************
** $Id: qapplication.cpp,v 2.58.2.7 1999/01/25 20:23:14 warwick Exp $
**
** Implementation of QApplication class
**
** Created : 931107
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

#include "qapplication.h"
#include "qobjectlist.h"
#include "qobjectdict.h"
#include "qwidget.h"
#include "qwidgetlist.h"
#include "qwidgetintdict.h"


/*!
  \class QApplication qapplication.h
  \brief The QApplication class manages the application event queue.

  \ingroup kernel

  The QApplication class is central to Qt.  It receives events from
  the underlying window system and sends them to the destination widgets.
  An application object must be created before any widgets can be created!

  Only one single QApplication object should be created.  In fact Qt
  complains if you create more than one, and this is normally done
  in the main() function.  Once a QApplication object has been
  created, \c qApp (defined as <code>extern QApplication *qApp</code>)
  refers to this object.

  Example (a complete Qt application):
  \code
    #include <qapplication.h>				// defines QApplication
    #include <qpushbutton.h>			// defines QPushButton

    int main( int argc, char **argv )
    {
	QApplication app( argc, argv );		// create application object
	QPushButton  hello( "Hello, world!" );	// create a push button
	app.setMainWidget( &hello );		// define as main widget
	connect( &hello, SIGNAL(clicked()),	// clicking the button
		 &app, SLOT(quit()) );		//   quits the application
	hello.show();				// show button
	return app.exec();			// run main event loop
    }
  \endcode

  <strong>Important</strong><br> Notice that the QApplication object must
  be created before any window-system functionality of Qt is used, this
  includes widgets, colors, fonts etc.

  Note also that for X11, setMainWidget() may change the main widget
  according to the \e -geometry option.	 To preserve this functionality,
  you must set your defaults before setMainWidget() and any overrides
  after.

  While Qt is not optimized or designed for writing non-GUI programs,
  it's possible to use <a href="tools.html">some of its classes</a>
  without creating a QApplication.  This can be very useful if you
  wish to share code between a non-GUI server and a GUI client.

  \header qkeycode.h
  \header qwindowdefs.h
  \header qglobal.h
*/


/*
  The qt_init() and qt_cleanup() functions are implemented in the
  qapplication_xyz.cpp file.
*/

void qt_init( int *, char ** );
void qt_cleanup();
#if defined(_WS_X11_)
void qt_init( Display* dpy );
#endif

QApplication *qApp = 0;				// global application object
QPalette *QApplication::app_pal	       = 0;	// default application palette
QFont	 *QApplication::app_font       = 0;	// default application font
QCursor	 *QApplication::app_cursor     = 0;	// default application cursor
int	  QApplication::app_tracking   = 0;	// global mouse tracking
bool	  QApplication::is_app_running = FALSE;	// app starting up if FALSE
bool	  QApplication::is_app_closing = FALSE;	// app closing down if TRUE
int	  QApplication::loop_level     = 0;	// event loop level
QWidget	 *QApplication::main_widget    = 0;	// main application widget
QWidget	 *QApplication::focus_widget   = 0;	// has keyboard input focus


#if defined(_WS_WIN_)
GUIStyle QApplication::app_style = WindowsStyle;// default style for Windows
#elif defined(_WS_X11_)
GUIStyle QApplication::app_style = MotifStyle;	// default style for X Windows
#endif

int	 QApplication::app_cspec = QApplication::NormalColor;


static QPalette *stdPalette = 0;
static QColor * winHighlightColor = 0;
static int mouseDoubleClickInterval = 400;


static void create_palettes()			// creates default palettes
{
    QColor standardLightGray( 192, 192, 192 );
    QColor light( 255, 255, 255 );
    QColor dark( standardLightGray.dark( 150 ) );
    QColorGroup std_nor( black, standardLightGray,
			 light, dark, gray,
			 black, white );
    QColorGroup std_dis( darkGray, standardLightGray,
			 light, dark, gray,
			 darkGray, std_nor.background() );
    QColorGroup std_act( black, standardLightGray,
			 light, dark, gray,
			 black, white );
    stdPalette = new QPalette( std_nor, std_dis, std_act );
}

static void destroy_palettes()
{
    delete stdPalette;
}


/*!
  Initializes the window system and onstructs an application object
  with the command line arguments \e argc and \e argv.

  The global \c qApp pointer refers to this application object. Only
  one application object should be created.

  This application object must be constructed before any \link
  QPaintDevice paint devices\endlink (includes widgets, pixmaps, bitmaps
  etc.)

  Notice that \e argc and \e argv might be changed.  Qt removes
  command line arguments that it recognizes.  \e argc and \e argv are
  can be accessed later by \c qApp->argc() and \c qApp->argv().	 The
  documentation for argv() contains a detailed description of how to
  process command line arguments.

  Qt debugging options:
  <ul>
  <li> \c -nograb, tells Qt to never grab the mouse or the keyboard.
  <li> \c -sync (only under X11), switches to synchronous mode for
	debugging.
  </ul>

  See <a href="debug.html">Debugging Techniques</a> for a more
  detailed explanation.

  The X11 version of Qt supports a few more command line options:
  <ul>
  <li> \c -display \e display, sets the X display (default is $DISPLAY).
  <li> \c -geometry \e geometry, sets the client geometry of the
	\link setMainWidget() main widget\endlink.
  <li> \c -fn or \c -font \e font, defines the application font.
  <li> \c -bg or \c -background \e color, sets the default background color
	and an application palette (light and dark shades are calculated).
  <li> \c -fg or \c -foreground \e color, sets the default foreground color.
  <li> \c -name \e name, sets the application name.
  <li> \c -title \e title, sets the application title (caption).
  <li> \c -style= \e style, sets the application GUI style. Possible values
       are \c motif and \c windows
  <li> \c -visual \c TrueColor, forces the application to use a TrueColor visual
       on an 8-bit display.
  <li> \c -ncols \e count, limits the number of colors allocated in the
       color cube on a 8-bit display, if the application is using the
       \c QApplication::ManyColor color specification.  If \e count is
       216 then a 6x6x6 color cube is used (ie. 6 levels of red, 6 of green,
       and 6 of blue); for other values, a cube
       approximately proportional to a 2x3x1 cube is used.
  <li> \c -cmap, causes the application to install a private color map
       on an 8-bit display.
  </ul>

  \sa argc(), argv()
*/

QApplication::QApplication( int &argc, char **argv )
{
#if defined(CHECK_STATE)
    if ( qApp )
	warning( "QApplication: There should be only one application object" );
#endif
    qApp = this;
    static char *empty = "";
    if ( argc == 0 || argv == 0 ) {
	argc = 0;
	argv = &empty;
    }
    qt_init( &argc, argv );
    initialize( argc, argv );
}


#if defined(_WS_X11_)

/*!
  Create an application, given an already open display.  This is
  available only on X11.
*/

QApplication::QApplication( Display* dpy )
{
#if defined(CHECK_STATE)
    if ( qApp )
	warning( "QApplication: There should be only one application object" );
#endif
    qApp = this;
    qt_init( dpy );
    initialize( 0, 0 );
}

#endif // _WS_X11_


/*!
  Initializes the QApplication object, called from the constructors.
*/

void QApplication::initialize( int argc, char **argv )
{
    app_argc = argc;
    app_argv = argv;
    quit_now = FALSE;
    quit_code = 0;
    if ( !app_pal ) {				// palette not already set
	create_palettes();
	app_pal = new QPalette( *stdPalette );
	CHECK_PTR( app_pal );
    }
    if ( !app_font ) {				// font not already set
	app_font = new QFont;
	app_font->setCharSet( QFont::defaultFont().charSet() );
	CHECK_PTR( app_font );
    }
    QWidget::createMapper();			// create widget mapper
    is_app_running = TRUE;			// no longer starting up
}


/*!
  Deletes all remaining widgets and cleans up any window system
  resources that were allocated by this application.  Sets the global
  variable \c qApp to null.
*/

QApplication::~QApplication()
{
    is_app_closing = TRUE;
    QWidget::destroyMapper();			// destroy widget mapper
    destroy_palettes();
    delete app_pal;
    app_pal = 0;
    delete app_font;
    app_font = 0;
    delete app_cursor;
    app_cursor = 0;
    qt_cleanup();
    delete winHighlightColor;
    winHighlightColor = 0;
    delete objectDict;
    qApp = 0;
}


/*!
  \fn int QApplication::argc() const
  Returns the number of command line arguments.

  The documentation for argv() contains a detailed description of how to
  process command line arguments.

  \sa argv(), QApplication::QApplication()
*/

/*!
  \fn char **QApplication::argv() const
  Returns the command line argument vector.

  \c argv()[0] is the program name, \c argv()[1] is the first argument and
  \c argv()[argc()-1] is the last argument.

  A QApplication object is constructed by passing \e argc and \e argv from
  the \c main() function.  Some of the arguments may be recognized as Qt
  options removed from the argument vector.  For example, the X11
  version of Qt knows about \c -display, \c -font and a few more options.

  Example:
  \code
    // showargs.cpp - displays program arguments in a list box

    #include <qapplication.h>
    #include <qlistbox.h>

    int main( int argc, char **argv )
    {
	QApplication a( argc, argv );
	QListBox b;
	a.setMainWidget( &b );
	for ( int i=0; i<a.argc(); i++ )	// a.argc() == argc
	    b.insertItem( a.argv()[i] );	// a.argv()[i] == argv[i]
	b.show();
	return a.exec();
    }
  \endcode

  If you run <tt>showargs -display unix:0 -font 9x15bold hello
  world</tt> under X11, the list box contains the three strings
  "showargs", "hello" and "world".

  \sa argc(), QApplication::QApplication()
*/


/*!
  \fn GUIStyle QApplication::style()
  Returns the GUI style of the application.
  \sa setStyle()
*/

/*!
  Sets the application GUI style to \e style.

  The style parameter can be \c WindowsStyle or \c MotifStyle.

  \sa style(), QWidget::setStyle()
*/

void QApplication::setStyle( GUIStyle style )
{
    app_style = style;
}


#if 1  /* OBSOLETE */

QApplication::ColorMode QApplication::colorMode()
{
    return (QApplication::ColorMode)app_cspec;
}

void QApplication::setColorMode( QApplication::ColorMode mode )
{
    app_cspec = mode;
}
#endif


/*!
  Returns the color specification.
  \sa QApplication::setColorSpec()
 */

int QApplication::colorSpec()
{
    return app_cspec;
}

/*!
  Sets the color specification for the application to \a spec.

  The color specification controls how your application allocates
  colors. You must set the color specification before you create the
  QApplication object.

  The choices are:
  <ul>
  <li> \c QApplication::NormalColor.
    This is the default color allocation strategy.
    Use this choice if your application uses buttons, menus,
    texts and pixmaps with few colors.
    With this choice, the application allocates system global colors.
    This work fine for most applications under X11, but Windows dithers to
    the 20 standard colors unless the display has true color support (more
    than 256 colors).

  <li> \c QApplication::CustomColor.
    Use this choice if your application needs a small number of
    custom colors.  This choice only makes a difference on Windows
    - the application gets more colors when it is active, but the
    background windows look less good.
    Under X11 this is the same as \c
    NormalColor. Under Windows, Qt creates a Windows palette if the display
    supports 256 colors.

  <li> \c QApplication::ManyColor.
    Use this choice if your application is very color hungry
    (e.g. it wants thousands of colors).
    Under Windows, this is equal to \c CustomColor.
    Under X11 the effect is:
    <ul>
      <li> For 256-color displays which have at best a 256 color true color
	    visual, the default visual is used, and a colors are allocated
	    from a color cube.
	    The color cube is the 6x6x6 (216 color) "Web palette", but the
	    number of colors can be changed by the \e -ncols option.
	    The user can force the application to use the true color visual by
	    the \link QApplication::QApplication() -visual \endlink
	    option.
      <li> For 256-color displays which have a true color visual with more
	    than 256 colors, use that visual.  Silicon Graphics X servers
	    have this feature. They provide an 8 bit visual as default but
	    can deliver true color when asked.
    </ul>
  </ul>

  Example:
  \code
  int main( int argc, char **argv )
  {
      QApplication::setColorSpec( QApplication::ManyColor );
      QApplication a( argc, argv );
      ...
  }
  \endcode

  QColor provides more functionality for controlling color allocation and
  freeing up certains colors. See QColor::enterAllocContext() for more
  information.

  To see what mode you end up with, you can call QColor::numBitPlanes()
  once the QApplication object exists.  A value greater than 8 (typically
  16, 24 or 32) means true color.

  The color cube used by Qt are all those colors with red, green, and blue
  components of either 0x00, 0x33, 0x66, 0x99, 0xCC, or 0xFF.

  \sa colorSpec(), QColor::numBitPlanes(), QColor::enterAllocContext()
*/

void QApplication::setColorSpec( int spec )
{
#if defined(CHECK_STATE)
    if ( qApp ) {
	warning( "QApplication::setColorSpec: This function must be "
		 "called before the QApplication object is created" );
    }
#endif
    app_cspec = spec;
}


/*!
  Returns a pointer to the default application palette.	 There is
  always an application palette, i.e. the returned pointer is
  guaranteed to be non-null.
  \sa setPalette(), QWidget::palette()
*/

QPalette *QApplication::palette()
{
#if defined(CHECK_STATE)
    if ( !qApp ) {
	warning( "QApplication::palette: This function can only be "
		 "called after the QApplication object has been created" );
    }
#endif
    return app_pal;
}

/*!
  Changes the default application palette to \e palette.

  If \e updateAllWidgets is TRUE, then the palette of all existing
  widgets is set to \e palette.

  Widgets created after this call get \e palette as their \link
  QWidget::palette() palette\endlink.

  \sa QWidget::setPalette(), palette()
*/

void QApplication::setPalette( const QPalette &palette, bool updateAllWidgets )
{
    delete app_pal;
    app_pal = new QPalette( palette.copy() );
    CHECK_PTR( app_pal );
    if ( updateAllWidgets && is_app_running && !is_app_closing ) {
	QWidgetIntDictIt it( *((QWidgetIntDict*)QWidget::mapper) );
	register QWidget *w;
	while ( (w=it.current()) ) {		// for all widgets...
	    ++it;
	    if ( !w->testWFlags(WType_Desktop) )// (except desktop)
		w->setPalette( *app_pal );
	}
    }
}


/*!
  \fn QFont *QApplication::font()
  Returns the default application font.	 There is always an application
  font, i.e. the returned pointer is guaranteed to be non-null.
  \sa setFont(), fontMetrics(), QWidget::font()
*/

/*!
  Changes the default application font to \e font.

  The default font depends on the X server in use.

  If \e updateAllWidgets is TRUE, then the font of all existing
  widgets is set to \e font.

  Widgets created after this call get \e font as their \link
  QWidget::font() font\endlink.

  \sa font(), fontMetrics(), QWidget::setFont()
*/

void QApplication::setFont( const QFont &font,	bool updateAllWidgets )
{
    if ( app_font )
	delete app_font;
    app_font = new QFont( font );
    CHECK_PTR( app_font );
    QFont::setDefaultFont( *app_font );
    if ( updateAllWidgets && is_app_running && !is_app_closing) {		// set for all widgets now
	QWidgetIntDictIt it( *((QWidgetIntDict*)QWidget::mapper) );
	register QWidget *w;
	while ( (w=it.current()) ) {		// for all widgets...
	    ++it;
	    if ( !w->testWFlags(WType_Desktop) )// (except desktop)
		w->setFont( *app_font );
	}
    }
}


/*!
  Returns a list of the top level widgets in the application.

  The list is created using new and must be deleted by the caller.

  The list is \link QList::isEmpty() empty \endlink if there are no
  top level widgets.

  Note that some of the top level widgets may be hidden.

  Example:
  \code
    //
    // Shows all hidden top level widgets.
    //
    QWidgetList	 *list = QApplication::topLevelWidgets();
    QWidgetListIt it( *list );		// iterate over the widgets
    while ( it.current() ) {		// for each top level widget...
	if ( !it.current()->isVisible() )
	    it.current()->show();
	++it;
    }
    delete list;			// delete the list, not the widgets
  \endcode

  The QWidgetList class is defined in the qwidcoll.h header file.

  \warning
  Delete the list away as soon you have finished using it.
  You can get in serious trouble if you for instance try to access
  a widget that has been deleted.

  \sa allWidgets(), QWidget::isTopLevel(), QWidget::isVisible(),
      QList::isEmpty()
*/

QWidgetList *QApplication::topLevelWidgets()
{
    return QWidget::tlwList();
}

/*!
  Returns a list of all the widgets in the application.

  The list is created using new and must be deleted by the caller.

  The list is \link QList::isEmpty() empty \endlink if there are no
  widgets.

  Note that some of the widgets may be hidden.

  Example:
  \code
    //
    // Updates all widgets.
    //
    QWidgetList	 *list = QApplication::allWidgets();
    QWidgetListIt it( *list );		// iterate over the widgets
    while ( it.current() ) {		// for each top level widget...
        it.current()->update();
	++it;
    }
    delete list;			// delete the list, not the widgets
  \endcode

  The QWidgetList class is defined in the qwidcoll.h header file.

  \warning
  Delete the list away as soon you have finished using it.
  You can get in serious trouble if you for instance try to access
  a widget that has been deleted.

  \sa topLevelWidgets(), QWidget::isVisible(), QList::isEmpty(),
*/

QWidgetList *QApplication::allWidgets()
{
    return QWidget::wList();
}


/*!
  \fn QWidget *QApplication::focusWidget() const
  Returns the application widget that has the keyboard input focus, or null
  if no application widget has the focus.
  \sa QWidget::setFocus(), QWidget::hasFocus()
*/


/*!
  Returns display (screen) font metrics for the application font.
  \sa font(), setFont(), QWidget::fontMetrics(), QPainter::fontMetrics()
*/

QFontMetrics QApplication::fontMetrics()
{
    return desktop()->fontMetrics();
}


/*!
  Tells the application to exit with a return code.

  After this function has been called, the application leaves the main
  event loop and returns from the call to exec(). The exec() function
  returns \e retcode.

  By convention, \e retcode 0 means success, any non-zero value indicates
  an error.

  Note that unlike the C library exit function, this function \e does
  returns to the caller - it is event processing that stops.

  \sa quit(), exec()
*/

void QApplication::exit( int retcode )
{
    if ( !qApp )				// no global app object
	return;
    if ( qApp->quit_now )			// don't overwrite quit code
	return;
    qApp->quit_now  = TRUE;
    qApp->quit_code = retcode;
}


/*!
  Tells the application to exit with return code 0 (success).
  Equivalent to calling QApplication::exit( 0 ).

  This function is a \link metaobjects.html slot\endlink, i.e. you
  may connect any signal to activate quit().

  Example:
  \code
    QPushButton *quitButton = new QPushButton( "Quit" );
    connect( quitButton, SIGNAL(clicked()), qApp, SLOT(quit()) );
  \endcode

  \sa exit()
*/

void QApplication::quit()
{
    QApplication::exit( 0 );
}


/*!
  \fn void QApplication::lastWindowClosed()

  This signal is emitted when the user has closed a top level widget
  and there are no more visible top level widgets left.

  The signal is very useful when your application has many top level
  widgets but no main widget. You can then connect it to the quit() slot.

  \sa mainWidget(), topLevelWidgets(), QWidget::isTopLevel()
*/


/*!
  \fn bool QApplication::sendEvent( QObject *receiver, QEvent *event )

  Sends an event directly to a receiver, using the notify() function.
  Returns the value that was returned from the event handler.

  \sa postEvent(), notify()
*/

/*!
  Sends \e event to \e receiver: <code>receiver->event( event )</code>
  Returns the value that is returned from the receiver's event handler.

  Reimplementing this virtual function is one of five ways to process
  an event: <ol> <li> Reimplementing this function.  Very powerful,
  you get \e complete control, but of course only one subclass can be
  qApp.

  <li> Installing an event filter on qApp.  Such an event filter gets
  to process all events for all widgets, so it's just as powerful as
  reimplementing notify(), and in this way it's possible to have more
  than one application-global event filter.  Global event filter get
  to see even mouse events for \link QWidget::isEnabled() disabled
  widgets, \endlink and if \link setGlobalMouseTracking() global mouse
  tracking \endlink is enabled, mouse move events for all widgets.

  <li> Reimplementing QObject::event() (as QWidget does).  If you do
  this you get tab key-presses, and you get to see the events before
  any widget-specific event filters.

  <li> Installing an event filter on the object.  Such an even filter
  gets all the events except Tab and Shift-Tab key presses.

  <li> Finally, reimplementing paintEvent(), mousePressEvent() and so
  on.  This is the normal, easist and least powerful way. </ol>

  \sa QObject::event(), installEventFilter()
*/

bool QApplication::notify( QObject *receiver, QEvent *event )
{
    // no events are delivered after ~QApplication has started
    if ( is_app_closing )
	return FALSE;
    
    if ( receiver == 0 ) {			// serious error
#if defined(CHECK_NULL)
	warning( "QApplication::notify: Unexpected null receiver" );
#endif
	return FALSE;
    }

    if ( eventFilters ) {
	QObjectListIt it( *eventFilters );
	register QObject *obj;
	while ( (obj=it.current()) != 0 ) {	// send to all filters
	    ++it;				//   until one returns TRUE
	    if ( obj->eventFilter(receiver,event) )
		return TRUE;
	}
    }

    // throw away mouse events to disabled widgets
    if ( event->type() <= Event_MouseMove &&
	 event->type() >= Event_MouseButtonPress &&
	 ( receiver->isWidgetType() &&
	   !((QWidget *)receiver)->isEnabled() ) )
	 return FALSE;

    // throw away any mouse-tracking-only mouse events
    if ( event->type() == Event_MouseMove &&
	 (((QMouseEvent*)event)->state() & MouseButtonMask) == 0 &&
	 ( receiver->isWidgetType() &&
	   !((QWidget *)receiver)->hasMouseTracking() ) )
	return TRUE;

    return receiver->event( event );
}


/*!
  Returns TRUE if an application object has not been created yet.
  \sa closingDown()
*/

bool QApplication::startingUp()
{
    return !is_app_running;
}

/*!
  Returns TRUE if the application objects are being destroyed.
  \sa startingUp()
*/

bool QApplication::closingDown()
{
    return is_app_closing;
}

/*!
  Processes pending events, for 3 seconds or until there
  are no more events to process, then return.

  You can call this function occasionally when your program is busy doing a
  long operation (e.g. copying a file).

  \sa processEvents(), exec(), QTimer
*/

void QApplication::processEvents()
{
    processEvents( 3000 );
}

/*!
  Waits for an event to occur, processes it, then returns.

  This function is useful for adapting Qt to situations where the event
  processing must be grafted into existing program loops.  Beware
  that using this function in new applications may be an indication
  of design problems.

  \sa processEvents(), exec(), QTimer
*/

void QApplication::processOneEvent()
{
    processNextEvent(TRUE);
}


#if !defined(_WS_X11_)

// The doc and X implementation of these functions is in qapplication_x11.cpp

void QApplication::flushX()	{}		// do nothing

void QApplication::syncX()	{}		// do nothing

#endif



/*!
  Sets the color used to mark selections in windows style for all widgets
  in the application. Will repaint all widgets if the color is changed.

  The default color is \c darkBlue.
  \sa winStyleHighlightColor()
*/

void QApplication::setWinStyleHighlightColor( const QColor &c )
{
    if ( !winHighlightColor )
	winHighlightColor = new QColor( darkBlue );

    if ( *winHighlightColor == c )
	return;

    *winHighlightColor = c;

    if ( is_app_running && !is_app_closing ) {
	QWidgetIntDictIt it( *((QWidgetIntDict*)QWidget::mapper) );
	register QWidget *w;
	while ( (w=it.current()) ) {		// for all widgets...
	    ++it;
	    if ( w->style() == WindowsStyle &&
		 !w->testWFlags(WType_Desktop) )// (except desktop)
		w->repaint( FALSE );
	}
    }
}


/*!
  Returns the color used to mark selections in windows style.
  \sa setWinStyleHighlightColor()
*/
const QColor& QApplication::winStyleHighlightColor()
{
    if ( !winHighlightColor )
	winHighlightColor = new QColor( darkBlue );

    return *winHighlightColor;
}


/*!
  Sets the time limit that distinguishes a double click from two
  consecutive mouse clicks to \a ms milliseconds. This value is
  ignored under Windows (the control panel value is used.)

  The default value is 400 milliseconds.

  \sa doubleClickInterval()
*/

void QApplication::setDoubleClickInterval( int ms )
{
    mouseDoubleClickInterval = ms;
}


/*!
  Returns the maximum duration for a double click.

  \sa setDoubleClickInterval()
*/

int QApplication::doubleClickInterval()
{
    return mouseDoubleClickInterval;
}


/*!
  \fn WindowsVersion QApplication::winVersion()

  Returns the version of the Windows operating system running:

  <ul>
  <li> \c WV_NT Windows NT.
  <li> \c WV_95 Windows 95.
  <li> \c WV_32s Win32s.
  </ul>

  Note that this function is implemented for the Windows version
  of Qt only.
*/
