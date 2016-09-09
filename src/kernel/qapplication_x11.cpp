/****************************************************************************
** $Id: qapplication_x11.cpp,v 2.144.2.17 1999/02/13 18:25:10 ettrich Exp $
**
** Implementation of X11 startup routines and event handling
**
** Created : 931029
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

#define select		_qt_hide_select
#define gettimeofday	_qt_hide_gettimeofday

#include "qglobal.h"
#if defined(_OS_WIN32_)
#include <windows.h>
#define HANDLE QT_HANDLE
#endif
#include "qapplication.h"
#include "qwidget.h"
#include "qobjectlist.h"
#include "qobjectdict.h"
#include "qwidgetlist.h"
#include "qwidgetintdict.h"
#include "qbitarray.h"
#include "qpainter.h"
#include "qpixmapcache.h"
#include "qdatetime.h"
#include "qkeycode.h"
#include <stdlib.h>
#include <ctype.h>
#include <locale.h>
#include <errno.h>
#define	 GC GC_QQQ
#if defined(_OS_WIN32_)
#undef gettimeofday
#endif
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include <X11/Xatom.h>
#if !defined(XlibSpecificationRelease)
#define X11R4
typedef char *XPointer;
#else
#undef  X11R4
#include <X11/Xlocale.h>
#endif

#if defined(_OS_LINUX_) && defined(DEBUG)
#include "qfile.h"
#include <unistd.h>
#endif

#if defined(_OS_IRIX_)
#include <bstring.h>
#endif

#if defined(_OS_AIX_) && defined(_CC_GNU_)
#include <sys/time.h>
#include <sys/select.h>
#include <unistd.h>
#endif

#if defined(_OS_QNX_)
#include <sys/select.h>
#endif

#if defined(_CC_MSVC_)
#pragma warning(disable: 4018)
#undef open
#undef close
#endif

#if defined(_OS_WIN32_) && defined(gettimeofday)
#undef gettimeofday
#include <sys/timeb.h>
inline void gettimeofday( struct timeval *t, struct timezone * )
{
    struct _timeb tb;
    _ftime( &tb );
    t->tv_sec  = tb.time;
    t->tv_usec = tb.millitm * 1000;
}
#else
#undef gettimeofday
extern "C" int gettimeofday( struct timeval *, struct timezone * );
#endif // _OS_WIN32 etc.
#undef select
extern "C" int select( int, void *, void *, void *, struct timeval * );

#if defined(_OS_AIX_)
// for FD_ZERO
static inline void bzero( void *s, int n )
{
    memset( s, 0, n );
}
#endif

/*****************************************************************************
  Internal variables and functions
 *****************************************************************************/

static char    *appName;			// application name
static char    *appFont		= 0;		// application font
static char    *appBGCol	= 0;		// application bg color
static char    *appFGCol	= 0;		// application fg color
static char    *mwGeometry	= 0;		// main widget geometry
static char    *mwTitle		= 0;		// main widget title
static bool	mwIconic	= FALSE;	// main widget iconified
static Display *appDpy		= 0;		// X11 application display
static char    *appDpyName	= 0;		// X11 display name
static bool     appForeignDpy	= FALSE;        // we didn't create display
static bool	appSync		= FALSE;	// X11 synchronization
#if defined(DEBUG)
static bool	appNoGrab	= FALSE;	// X11 grabbing enabled
static bool	appDoGrab	= FALSE;	// X11 grabbing override (gdb)
#endif
static int	appScreen;			// X11 screen number
static Window	appRootWin;			// X11 root window
static bool	app_save_rootinfo = FALSE;	// save root info

static bool	app_do_modal	= FALSE;	// modal mode
static bool	app_exit_loop	= FALSE;	// flag to exit local loop
static int	app_Xfd;			// X network socket
static fd_set	app_readfds;			// fd set for reading
static fd_set	app_writefds;			// fd set for writing
static fd_set	app_exceptfds;			// fd set for exceptions

static GC	app_gc_ro	= 0;		// read-only GC
static GC	app_gc_tmp	= 0;		// temporary GC
static GC	app_gc_ro_m	= 0;		// read-only GC (monochrome)
static GC	app_gc_tmp_m	= 0;		// temporary GC (monochrome)
static QWidget *desktopWidget	= 0;		// root window widget
static Atom	qt_wm_protocols;		// window manager protocols
Atom		qt_wm_delete_window;		// delete window protocol
static Atom	qt_qt_scrolldone;		// scroll synchronization
static Atom	qt_xsetroot_id;
Atom		qt_selection_property;
Atom		qt_wm_state;
static Atom 	qt_resource_manager;   	// X11 Resource manager
Atom 		qt_sizegrip;		// sizegrip

static Window	mouseActWindow	     = 0;	// window where mouse is
static int	mouseButtonPressed   = 0;	// last mouse button pressed
static int	mouseButtonState     = 0;	// mouse button state
static Time	mouseButtonPressTime = 0;	// when was a button pressed
static short	mouseXPos, mouseYPos;		// mouse position in act window
#if defined(DEBUG)
static int	debug_level = 0;
#endif

static QWidgetList *modal_stack  = 0;		// stack of modal widgets
static QWidgetList *popupWidgets = 0;		// list of popup widgets
static QWidget     *popupButtonFocus = 0;
static bool	    popupCloseDownMode = FALSE;
static bool	    popupGrabOk;
static bool	    popupFilter( QWidget * );

typedef void  (*VFPTR)();
typedef Q_DECLARE(QListM,void) QVFuncList;
static QVFuncList *postRList = 0;		// list of post routines

static void	cleanupPostedEvents();

static void	initTimers();
static void	cleanupTimers();
static timeval	watchtime;			// watch if time is turned back

#if defined(X11R4) || (defined(_OS_OSF_) && (XlibSpecificationRelease < 6)) || defined(_OS_AIX_)
#define NO_XIM
#endif

#if !defined(NO_XIM)
static XIM	xim;
#endif

timeval        *qt_wait_timer();
int	        qt_activate_timers();

QObject	       *qt_clipboard = 0;
Time		qt_x_clipboardtime = CurrentTime;

static void	qt_save_rootinfo();
static bool	qt_try_modal( QWidget *, XEvent * );
void		qt_reset_color_avail();		// defined in qcolor_x11.cpp

int		qt_ncols_option  = 216;		// used in qcolor_x11.cpp
int		qt_visual_option = -1;
bool		qt_cmap_option	 = FALSE;
QWidget*	qt_button_down	     = 0;	// the widget getting last button-down

// stuff in qt_xdnd.cpp
// setup
extern void qt_xdnd_setup();
// x event handling
extern void qt_handle_xdnd_enter( QWidget *, const XEvent * );
extern void qt_handle_xdnd_position( QWidget *, const XEvent * );
extern void qt_handle_xdnd_status( QWidget *, const XEvent * );
extern void qt_handle_xdnd_leave( QWidget *, const XEvent * );
extern void qt_handle_xdnd_drop( QWidget *, const XEvent * );
extern void qt_handle_xdnd_finished( QWidget *, const XEvent * );
extern void qt_xdnd_handle_selection_request( const XSelectionRequestEvent * );
extern bool qt_xdnd_handle_badwindow();
// client message atoms
extern Atom qt_xdnd_enter;
extern Atom qt_xdnd_position;
extern Atom qt_xdnd_status;
extern Atom qt_xdnd_leave;
extern Atom qt_xdnd_drop;
extern Atom qt_xdnd_finished;
// xdnd selection atom
extern Atom qt_xdnd_selection;
// thatsall

void qt_x11_intern_atom( const char *, Atom * );


class QETWidget : public QWidget		// event translator widget
{
public:
    void setWFlags( WFlags f )		{ QWidget::setWFlags(f); }
    void clearWFlags( WFlags f )	{ QWidget::clearWFlags(f); }
    bool translateMouseEvent( const XEvent * );
    bool translateKeyEvent( const XEvent *, bool grab );
    bool translatePaintEvent( const XEvent * );
    bool translateConfigEvent( const XEvent *);
    bool translateCloseEvent( const XEvent * );
    bool translateScrollDoneEvent( const XEvent * );
};


/*****************************************************************************
  Default X error handlers
 *****************************************************************************/

#if defined(Q_C_CALLBACKS)
extern "C" {
#endif

static int qt_x_errhandler( Display *dpy, XErrorEvent *err ) {
    if ( err->request_code == 25 && qt_xdnd_handle_badwindow() )
	return 0;
    char errstr[256];
    XGetErrorText( dpy, err->error_code, errstr, 256 );
    fatal( "X Error: %s %d\n  Major opcode:  %d", errstr, err->error_code, err->request_code );
    return 0;
}


static int qt_xio_errhandler( Display * )
{
    warning( "%s: Fatal IO error: client killed", appName );
    exit( 1 );
    return 0;
}

#if defined(Q_C_CALLBACKS)
}
#endif


// memory leak: if the app exits before qt_init_internal(), this dict
// isn't released correctly.
static QDict<Atom> * atoms_to_be_created = 0;
static bool create_atoms_now = 0;

/*****************************************************************************
  qt_x11_intern_atom() - efficiently interns an atom, now or later.

  If the application is being initialized, this function stores the
  adddress of the atom and qt_init_internal will do the actual work
  quickly.  If the application is running, the atom is created here.

  Neither argument may point to temporary variables.
 *****************************************************************************/

void qt_x11_intern_atom( const char * name, Atom * result)
{
    if ( !name || !result || *result )
	return;

    if ( create_atoms_now ) {
	*result = XInternAtom(appDpy, name, FALSE );
    } else {
	if ( !atoms_to_be_created ) {
	    atoms_to_be_created = new QDict<Atom>;
	    atoms_to_be_created->setAutoDelete( FALSE );
	}
	atoms_to_be_created->insert( name, result );
	*result = 0;
    }
}


static void qt_x11_process_intern_atoms()
{
    if ( atoms_to_be_created ) {
#if defined(XlibSpecificationRelease) && (XlibSpecificationRelease >= 6)
	int i = atoms_to_be_created->count();
	Atom * res = (Atom *)malloc( i * sizeof( Atom ) );
	Atom ** resp = (Atom **)malloc( i * sizeof( Atom* ) );
	char ** names = (char **)malloc( i * sizeof(const char*));

	i = 0;
	QDictIterator<Atom> it( *atoms_to_be_created );
	while( it.current() ) {
	    res[i] = 0;
	    resp[i] = it.current();
	    names[i] = (char *)it.currentKey();
	    i++;
	    ++it;
	}
	XInternAtoms( appDpy, names, i, FALSE, res );
	while( i ) {
	    i--;
	    if ( res[i] && resp[i] )
		*(resp[i]) = res[i];
	}
	(void)free( res );
	(void)free( resp );
	(void)free( names );
#else
	QDictIterator<Atom> it( *atoms_to_be_created );
	Atom * result;
	const char * name;
	while( (result = it.current()) != 0 ) {
	    name = it.currentKey();
	    ++it;
	    *result = XInternAtom(appDpy, name, FALSE );
	}
#endif
	delete atoms_to_be_created;
	atoms_to_be_created = 0;
	create_atoms_now = TRUE;
    }
}



/*****************************************************************************
  set_local_font() - tries to set a sensible default font char set
 *****************************************************************************/

/* locale names mostly copied from XFree86 */
static const char * latin2locales[] = {
    "croatian", "cs", "cs_CS", "cs_CZ","cz", "cz_CZ", "czech", "hr",
    "hr_HR", "hu", "hu_HU", "hungarian", "pl", "pl_PL", "polish", "ro",
    "ro_RO", "rumanian", "serbocroatian", "sh", "sh_SP", "sh_YU", "sk",
    "sk_SK", "sl", "sl_CS", "sl_SI", "slovak", "slovene", "sr_SP", 0 };

static const char * latin5locales[] = {
    "bg", "bg_BG", "bulgarian", "mk", "mk_MK", "ru", "ru_RU", "ru_SU",
    "russian", "sp", "sp_YU", 0 };


static const char * latin6locales[] = {
    "ar_AA", "ar_SA", "arabic", 0 };

static const char * latin7locales[] = {
    "el", "el_GR", "greek", 0 };

static const char * latin8locales[] = {
    "hebrew", "iw", "iw_IL", 0 };

static const char * latin9locales[] = {
    "tr", "tr_TR", "turkish", 0 };

static bool try_locale( const char * locale[], const char * lang,
			QFont::CharSet encoding )
{
    int i;
    for( i=0; locale[i] && strcmp(locale[i], lang); i++ )
	;
    if ( locale[i] ) {
	QFont::setDefaultFont( QFont( "Helvetica", 12,
				      QFont::Normal, FALSE, encoding ) );
	return TRUE;
    }
    return FALSE;
}


static struct {
    const char * name;
    QFont::CharSet cs;
} encoding_names[] = {
    { "ISO8859-1", QFont::Latin1 },
    { "ISO8859-2", QFont::Latin2 },
    { "ISO8859-3", QFont::Latin3 },
    { "ISO8859-4", QFont::Latin4 },
    { "ISO8859-5", QFont::Latin5 },
    { "ISO8859-6", QFont::Latin6 },
    { "ISO8859-7", QFont::Latin7 },
    { "ISO8859-8", QFont::Latin8 },
    { "ISO8859-9", QFont::Latin9 },
    { "KOI8-R", QFont::KOI8R },
    { 0, /* anything */ QFont::Latin1 }
};


	
static void set_local_font()
{
    char * lang = qstrdup( getenv( "LANG" ) );
    char * p = lang;
    while( p && *p ) {
	if ( *p == '.' ) {
	    *p++ = 0;
	    int i=0;
	    while( encoding_names[i].name &&
		   qstricmp( p, encoding_names[i].name ) )
		i++;
	    if ( encoding_names[i].name ) {
		QFont::setDefaultFont( QFont( "Helvetica", 12, QFont::Normal,
					      FALSE, encoding_names[i].cs ) );
		return;
	    }
	    p--;
	} else {
	    p++;
	}
    }

    if ( lang &&
	 !try_locale( latin2locales, lang, QFont::Latin2 ) &&
	 !try_locale( latin5locales, lang, QFont::Latin5 ) &&
	 !try_locale( latin6locales, lang, QFont::Latin5 ) &&
	 !try_locale( latin7locales, lang, QFont::Latin7 ) &&
	 !try_locale( latin8locales, lang, QFont::Latin8 ) &&
	 !try_locale( latin9locales, lang, QFont::Latin9 ) )
	QFont::setDefaultFont( QFont( "Helvetica", 12,
				      QFont::Normal, FALSE, QFont::Latin1 ) );
    if ( lang )				// Avoid purify complaint
	delete[] lang;
}


/*****************************************************************************
  qt_init() - initializes Qt for X11
 *****************************************************************************/

static void qt_init_internal( int *argcptr, char **argv, Display *display )
{
    if ( display ) {
      // Qt part of other application	

	appForeignDpy = TRUE;
	appName = "Qt-subapplication";
	appDpy  = display;
	app_Xfd = XConnectionNumber( appDpy );

    } else {
      // Qt controls everything (default)

	char *p;
	int argc = *argcptr;
	int i, j;

      // Install default error handlers

	XSetErrorHandler( qt_x_errhandler );
	XSetIOErrorHandler( qt_xio_errhandler );

      // Set application name

	p = strrchr( argv[0], '/' );
	appName = p ? p + 1 : argv[0];

      // Get command line params

	j = 1;
	for ( i=1; i<argc; i++ ) {
	    if ( argv[i] && *argv[i] != '-' ) {
		argv[j++] = argv[i];
		continue;
	    }
	    QString arg = argv[i];
	    if ( arg == "-display" ) {
		if ( ++i < argc )
		    appDpyName = argv[i];
	    } else if ( arg == "-fn" || arg == "-font" ) {
		if ( ++i < argc )
		    appFont = argv[i];
	    } else if ( arg == "-bg" || arg == "-background" ) {
		if ( ++i < argc )
		    appBGCol = argv[i];
	    } else if ( arg == "-fg" || arg == "-foreground" ) {
		if ( ++i < argc )
		    appFGCol = argv[i];
	    } else if ( arg == "-name" ) {
		if ( ++i < argc )
		    appName = argv[i];
	    } else if ( arg == "-title" ) {
		if ( ++i < argc )
		    mwTitle = argv[i];
	    } else if ( arg == "-geometry" ) {
		if ( ++i < argc )
		    mwGeometry = argv[i];
	    } else if ( arg == "-iconic" ) {
		mwIconic = !mwIconic;
	    } else if ( stricmp(arg, "-style=windows") == 0 ) {
		QApplication::setStyle( WindowsStyle );
	    } else if ( stricmp(arg, "-style=motif") == 0 ) {
		QApplication::setStyle( MotifStyle );
	    } else if ( strcmp(arg,"-style") == 0 && i < argc-1 ) {
		QString s = argv[++i];
		s = s.lower();
		if ( s == "windows" )
		    QApplication::setStyle( WindowsStyle );
		else if ( s == "motif" )
		    QApplication::setStyle( MotifStyle );
#if defined(DEBUG)
	    } else if ( arg == "-qdebug" ) {
		debug_level++;
#endif
	    } else if ( arg == "-ncols" ) {   // xv and netscape use this name
		if ( ++i < argc )
		    qt_ncols_option = QMAX(0,atoi(argv[i]));
	    } else if ( arg == "-visual" ) {  // xv and netscape use this name
		if ( ++i < argc ) {
		    QString s = QString(argv[i]).lower();
		    if ( s == "truecolor" ) {
			qt_visual_option = TrueColor;
		    } else {
			// ### Should we honor any others?
		    }
		}
	    } else if ( arg == "-cmap" ) {    // xv uses this name
		qt_cmap_option = TRUE;
	    }
#if defined(DEBUG)
	    else if ( arg == "-sync" )
		appSync = !appSync;
	    else if ( arg == "-nograb" )
		appNoGrab = !appNoGrab;
	    else if ( arg == "-dograb" )
		appDoGrab = !appDoGrab;
#endif
	    else
		argv[j++] = argv[i];
	}

	*argcptr = j;

#if defined(DEBUG) && defined(_OS_LINUX_)
	if ( !appNoGrab && !appDoGrab ) {
	    QString s;
	    s.sprintf( "/proc/%d/cmdline", getppid() );
	    QFile f( s );
	    if ( f.open( IO_ReadOnly ) ) {
		s.truncate( 0 );
		int c;
		while ( (c = f.getch()) > 0 ) {
		    if ( c == '/' )
			s.truncate( 0 );
		    else
			s += (char)c;
		}
		if ( s == "gdb" ) {
		    appNoGrab = TRUE;
		    debug( "Qt: gdb: -nograb added to command-line options.\n"
			   "\t Use the -dograb option to enforce grabbing." );
		}
		f.close();
	    }
	}
#endif
	// pick default character set

	set_local_font();

      // Connect to X server

	if ( ( appDpy = XOpenDisplay(appDpyName) ) == 0 ) {
	    warning( "%s: cannot connect to X server %s", appName,
		     XDisplayName(appDpyName) );
	    exit( 1 );
	}
	app_Xfd = XConnectionNumber( appDpy );	// set X network socket

	if ( appSync )				// if "-sync" argument
	    XSynchronize( appDpy, TRUE );
    }

  // Common code, regardless of whether display is foreign.

  // Get X parameters

    appScreen  = DefaultScreen(appDpy);
    appRootWin = RootWindow(appDpy,appScreen);

  // Support protocols

    qt_x11_intern_atom( "WM_PROTOCOLS", &qt_wm_protocols );
    qt_x11_intern_atom( "WM_DELETE_WINDOW", &qt_wm_delete_window );
    qt_x11_intern_atom( "_XSETROOT_ID", &qt_xsetroot_id );
    qt_x11_intern_atom( "QT_SCROLL_DONE", &qt_qt_scrolldone );
    qt_x11_intern_atom( "QT_SELECTION", &qt_selection_property );
    qt_x11_intern_atom( "WM_STATE", &qt_wm_state );
    qt_x11_intern_atom( "RESOURCE_MANAGER", &qt_resource_manager );
    qt_x11_intern_atom( "QT_SIZEGRIP", &qt_sizegrip );

    qt_xdnd_setup();

    // Finally create all atoms
    qt_x11_process_intern_atoms();

  // Misc. initialization

    QColor::initialize();
    QFont::initialize();
    QCursor::initialize();
    QPainter::initialize();
    gettimeofday( &watchtime, 0 );

    qApp->setName( appName );
    if ( appFont ) {				// set application font
	QFont font;
	font.setRawMode( TRUE );
	font.setFamily( appFont );
	QApplication::setFont( font );
    }
    if ( appBGCol || appFGCol ) {		// set application colors
	QColor bg;
	QColor fg;
	if ( appBGCol )
	    bg = QColor(appBGCol);
	else
	    bg = lightGray;
	if ( appFGCol )
	    fg = QColor(appFGCol);
	else
	    fg = black;
	QColorGroup cg( fg, bg, bg.light(),
			bg.dark(), bg.dark(150), fg, white );
	QColor disabled( (fg.red()+bg.red())/2,
			 (fg.green()+bg.green())/2,
			 (fg.blue()+bg.blue())/2 );
	QColorGroup dcg( disabled, bg, bg.light( 125 ), bg.dark(), bg.dark(150),
			 disabled, white );
	QPalette pal( cg, dcg, cg );
	QApplication::setPalette( pal );
    }
#if !defined(NO_XIM)
    setlocale( LC_ALL, "" );		// use correct char set mapping
    setlocale( LC_NUMERIC, "C" );	// make sprintf()/scanf() work
    if ( XSupportsLocale() &&
	 ( qstrlen(XSetLocaleModifiers( "" )) ||
	   qstrlen(XSetLocaleModifiers( "@im=none" ) ) ) )
	xim = XOpenIM( appDpy, 0, 0, 0 );
    else
	xim = 0;
#endif
}

void qt_init( int *argcptr, char **argv )
{
    qt_init_internal( argcptr, argv, 0 );
}

void qt_init( Display *display )
{
    qt_init_internal( 0, 0, display );
}


/*****************************************************************************
  qt_cleanup() - cleans up when the application is finished
 *****************************************************************************/

void qt_cleanup()
{
    cleanupPostedEvents();			// remove list of posted events
    if ( postRList ) {
	VFPTR f = (VFPTR)postRList->first();
	while ( f ) {				// call post routines
	    (*f)();
	    postRList->remove();
	    f = (VFPTR)postRList->first();
	}
	delete postRList;
    }

    if ( app_save_rootinfo )			// root window must keep state
	qt_save_rootinfo();
    cleanupTimers();
    QPixmapCache::clear();
    QPainter::cleanup();
    QCursor::cleanup();
    QFont::cleanup();
    QColor::cleanup();

#if !defined(NO_XIM)
    if ( xim ) {
	// Calling XCloseIM gives a Purify FMR error
	// Instead we get a non-critical memory leak
	// XCloseIM( xim );
	xim = 0;
    }
#endif

#define CLEANUP_GC(g) if (g) XFreeGC(appDpy,g)
    CLEANUP_GC(app_gc_ro);
    CLEANUP_GC(app_gc_ro_m);
    CLEANUP_GC(app_gc_tmp);
    CLEANUP_GC(app_gc_tmp_m);

    if ( !appForeignDpy )
	XCloseDisplay( appDpy );		// close X display
    appDpy = 0;
}


/*****************************************************************************
  Platform specific global and internal functions
 *****************************************************************************/

void qt_save_rootinfo()				// save new root info
{
    Atom type;
    int format;
    unsigned long length, after;
    unsigned char *data;

    if ( qt_xsetroot_id ) {			// kill old pixmap
	if ( XGetWindowProperty( appDpy, appRootWin, qt_xsetroot_id, 0, 1,
				 TRUE, AnyPropertyType, &type, &format,
				 &length, &after, &data ) == Success ) {
	    if ( type == XA_PIXMAP && format == 32 && length == 1 &&
		 after == 0 && data ) {
		XKillClient( appDpy, *((Pixmap*)data) );
		XFree( (char *)data );
	    }
	    Pixmap dummy = XCreatePixmap( appDpy, appRootWin, 1, 1, 1 );
	    XChangeProperty( appDpy, appRootWin, qt_xsetroot_id, XA_PIXMAP, 32,
			     PropModeReplace, (uchar *)&dummy, 1 );
	    XSetCloseDownMode( appDpy, RetainPermanent );
	}
    }
}

void qt_updated_rootinfo()
{
    app_save_rootinfo = TRUE;
}


/*!
  \relates QApplication
  Adds a global routine that will be called from the QApplication destructor.
  This function is normally used to add cleanup routines.

  CleanUpFunctions is defined as <code> typedef void
  (*CleanUpFunction)(); </code>, i.e. a pointer to a function that
  takes no arguments and returns nothing.

  Example of use:
  \code
    static int *global_ptr = 0;

    void cleanup_ptr()
    {
	delete [] global_ptr;
    }

    void init_ptr()
    {
	global_ptr = new int[100];		// allocate data
	qAddPostRoutine( cleanup_ptr );		// delete later
    }
  \endcode
*/

void qAddPostRoutine( CleanUpFunction p )
{
    if ( !postRList ) {
	postRList = new QVFuncList;
	CHECK_PTR( postRList );
    }
    postRList->insert( 0, (void *)p );		// store at list head
}


char *qAppName()				// get application name
{
    return appName;
}

Display *qt_xdisplay()				// get current X display
{
    return appDpy;
}

int qt_xscreen()				// get current X screen
{
    return appScreen;
}

WId qt_xrootwin()				// get X root window
{
    return appRootWin;
}

bool qt_nograb()				// application no-grab option
{
#if defined(DEBUG)
    return appNoGrab;
#else
    return FALSE;
#endif
}

static GC create_gc( bool monochrome )
{
    GC gc;
    if ( monochrome ) {
	Pixmap pm = XCreatePixmap( appDpy, appRootWin, 8, 8, 1 );
	gc = XCreateGC( appDpy, pm, 0, 0 );
	XFreePixmap( appDpy, pm );
    } else {
	if ( QPaintDevice::x11DefaultVisual() ) {
	    gc = XCreateGC( appDpy, appRootWin, 0, 0 );
	} else {
	    Window w;
	    XSetWindowAttributes a;
	    a.background_pixel = black.pixel();
	    a.border_pixel = black.pixel();		
	    a.colormap = QPaintDevice::x11Colormap();
	    w = XCreateWindow( appDpy, appRootWin, 0, 0, 100, 100,
			       0, QPaintDevice::x11Depth(), InputOutput,
			       (Visual*)QPaintDevice::x11Visual(),
			       CWBackPixel|CWBorderPixel|CWColormap,
			       &a );
	    gc = XCreateGC( appDpy, w, 0, 0 );
	    XDestroyWindow( appDpy, w );
	}
    }
    XSetGraphicsExposures( appDpy, gc, FALSE );
    return gc;
}

GC qt_xget_readonly_gc( bool monochrome )	// get read-only GC
{
    GC gc;
    if ( monochrome ) {
	if ( !app_gc_ro_m )			// create GC for bitmap
	    app_gc_ro_m = create_gc(TRUE);
	gc = app_gc_ro_m;
    } else {					// create standard GC
	if ( !app_gc_ro )			// create GC for bitmap
	    app_gc_ro = create_gc(FALSE);
	gc = app_gc_ro;
    }
    return gc;
}

GC qt_xget_temp_gc( bool monochrome )		// get temporary GC
{
    GC gc;
    if ( monochrome ) {
	if ( !app_gc_tmp_m )			// create GC for bitmap
	    app_gc_tmp_m = create_gc(TRUE);
	gc = app_gc_tmp_m;
    } else {					// create standard GC
	if ( !app_gc_tmp )			// create GC for bitmap
	    app_gc_tmp = create_gc(FALSE);
	gc = app_gc_tmp;
    }
    return gc;
}


/*****************************************************************************
  Platform specific QApplication members
 *****************************************************************************/

/*!
  \fn QWidget *QApplication::mainWidget() const
  Returns the main application widget, or 0 if there is not a defined
  main widget.
  \sa setMainWidget()
*/

/*!
  Sets the main widget of the application.

  The special thing about the main widget is that destroying the main
  widget (i.e. the program calls QWidget::close() or the user
  double-clicks the window close box) will leave the main event loop and
  \link QApplication::quit() exit the application\endlink.

  For X11, this function also resizes and moves the main widget
  according to the \e -geometry command-line option, so you should
  \link QWidget::setGeometry() set the default geometry\endlink before
  calling setMainWidget().

  \sa mainWidget(), exec(), quit()
*/

void QApplication::setMainWidget( QWidget *mainWidget )
{
    extern int qwidget_tlw_gravity;		// in qwidget_x11.cpp
    main_widget = mainWidget;
    if ( main_widget ) {			// give WM command line
	XSetWMProperties( main_widget->x11Display(), main_widget->winId(),
			  0, 0, app_argv, app_argc, 0, 0, 0 );
	if ( mwTitle )
	    XStoreName( appDpy, main_widget->winId(), mwTitle );
	if ( mwGeometry ) {			// parse geometry
	    int x, y;
	    int w, h;
	    int m = XParseGeometry( mwGeometry, &x, &y, (uint*)&w, (uint*)&h );
	    QSize minSize = main_widget->minimumSize();
	    QSize maxSize = main_widget->maximumSize();
	    if ( (m & XValue) == 0 )
		x = main_widget->geometry().x();
	    if ( (m & YValue) == 0 )
		y = main_widget->geometry().y();
	    if ( (m & WidthValue) == 0 )
		w = main_widget->width();
	    if ( (m & HeightValue) == 0 )
		h = main_widget->height();
	    w = QMIN(w,maxSize.width());
	    h = QMIN(h,maxSize.height());
	    w = QMAX(w,minSize.width());
	    h = QMAX(h,minSize.height());
	    if ( (m & XNegative) ) {
		x = desktop()->width()  + x - w;
		qwidget_tlw_gravity = 3;
	    }
	    if ( (m & YNegative) ) {
		y = desktop()->height() + y - h;
		qwidget_tlw_gravity = (m & XNegative) ? 9 : 7;
	    }
	    main_widget->setGeometry( x, y, w, h );
	}
    }
}


/*!
  \fn QWidget *QApplication::desktop()
  Returns the desktop widget (also called the root window).

  The desktop widget is useful for obtaining the size of the screen.
  It can also be used to draw on the desktop.

  \code
    QWidget *d = QApplication::desktop();
    int w=d->width();			// returns screen width
    int h=d->height();			// returns screen height
    d->setBackgroundColor( red );	// makes desktop red
  \endcode
*/

QWidget *QApplication::desktop()
{
    if ( !desktopWidget ) {			// not created yet
	desktopWidget = new QWidget( 0, "desktop", WType_Desktop );
	CHECK_PTR( desktopWidget );
    }
    return desktopWidget;
}


/*****************************************************************************
  QApplication cursor stack
 *****************************************************************************/

typedef Q_DECLARE(QListM,QCursor) QCursorList;

static QCursorList *cursorStack = 0;

/*!
  \fn QCursor *QApplication::overrideCursor()
  Returns the active application override cursor.

  This function returns 0 if no application cursor has been defined (i.e. the
  internal cursor stack is empty).

  \sa setOverrideCursor(), restoreOverrideCursor()
*/

/*!
  Sets the application override cursor to \e cursor.

  Application override cursor are intended for showing the user that the
  application is in a special state, for example during an operation that
  might take some time.

  This cursor will be displayed in all application widgets until
  restoreOverrideCursor() or another setOverrideCursor() is called.

  Application cursors are stored on an internal stack. setOverrideCursor()
  pushes the cursor onto the stack, and restoreOverrideCursor() pops the
  active cursor off the stack.	Every setOverrideCursor() must have an
  corresponding restoreOverrideCursor(), otherwise the stack will get out
  of sync. overrideCursor() returns 0 if the cursor stack is empty.

  If \e replace is TRUE, the new cursor will replace the last override
  cursor.

  Example:
  \code
    QApplication::setOverrideCursor( waitCursor );
    calculateHugeMandelbrot();			// lunch time...
    QApplication::restoreOverrideCursor();
  \endcode

  \sa overrideCursor(), restoreOverrideCursor(), QWidget::setCursor()
*/

void QApplication::setOverrideCursor( const QCursor &cursor, bool replace )
{
    if ( !cursorStack ) {
	cursorStack = new QCursorList;
	CHECK_PTR( cursorStack );
	cursorStack->setAutoDelete( TRUE );
    }
    app_cursor = new QCursor( cursor );
    CHECK_PTR( app_cursor );
    if ( replace )
	cursorStack->removeLast();
    cursorStack->append( app_cursor );
    QWidgetIntDictIt it( *((QWidgetIntDict*)QWidget::mapper) );
    register QWidget *w;
    while ( (w=it.current()) ) {		// for all widgets that have
	if ( w->testWFlags(WCursorSet) )	//   set a cursor
	    XDefineCursor( w->x11Display(), w->winId(), app_cursor->handle() );
	++it;
    }
    XFlush( appDpy );				// make X execute it NOW
}

/*!
  Restores the effect of setOverrideCursor().

  If setOverrideCursor() has been called twice, calling
  restoreOverrideCursor() will activate the first cursor set.  Calling
  this function a second time restores the original widgets cursors.

  Application cursors are stored on an internal stack. setOverrideCursor()
  pushes the cursor onto the stack, and restoreOverrideCursor() pops the
  active cursor off the stack.	Every setOverrideCursor() must have an
  corresponding restoreOverrideCursor(), otherwise the stack will get out
  of sync. overrideCursor() returns 0 if the cursor stack is empty.

  \sa setOverrideCursor(), overrideCursor().
*/

void QApplication::restoreOverrideCursor()
{
    if ( !cursorStack )				// no cursor stack
	return;
    cursorStack->removeLast();
    app_cursor = cursorStack->last();
    QWidgetIntDictIt it( *((QWidgetIntDict*)QWidget::mapper) );
    register QWidget *w;
    while ( (w=it.current()) ) {		// set back to original cursors
	if ( w->testWFlags(WCursorSet) )
	    XDefineCursor( w->x11Display(), w->winId(),
			   app_cursor ? app_cursor->handle()
			   : w->cursor().handle() );
	++it;
    }
    XFlush( appDpy );
    if ( !app_cursor ) {
	delete cursorStack;
	cursorStack = 0;
    }
}


/*!
  \fn bool QApplication::hasGlobalMouseTracking()
  Returns TRUE if global mouse tracking is enabled, otherwise FALSE.

  \sa setGlobalMouseTracking()
*/

/*!
  Enables global mouse tracking if \a enable is TRUE or disables it
  if \a enable is FALSE.

  Enabling global mouse tracking makes it possible for widget event
  filters or application event filters to get all mouse move events, even
  when no button is depressed.  This is useful for special GUI elements,
  e.g. tool tips.

  Global mouse tracking does not affect widgets and their
  mouseMoveEvent().  For a widget to get mouse move events when no button
  is depressed, it must do QWidget::setMouseTracking(TRUE).

  This function has an internal counter.  Each
  setGlobalMouseTracking(TRUE) must have a corresponding
  setGlobalMouseTracking(FALSE).

  \sa hasGlobalMouseTracking(), QWidget::hasMouseTracking()
*/

void QApplication::setGlobalMouseTracking( bool enable )
{
    bool tellAllWidgets;
    if ( enable ) {
	tellAllWidgets = (++app_tracking == 1);
    } else {
	tellAllWidgets = (--app_tracking == 0);
    }
    if ( tellAllWidgets ) {
	QWidgetIntDictIt it( *((QWidgetIntDict*)QWidget::mapper) );
	register QWidget *w;
	while ( (w=it.current()) ) {
	    if ( app_tracking > 0 ) {		// switch on
		if ( !w->testWFlags(WState_TrackMouse) ) {
		    w->setMouseTracking( TRUE );
		    w->clearWFlags(WState_TrackMouse);
		}
	    } else {				// switch off
		if ( !w->testWFlags(WState_TrackMouse) ) {
		    w->setWFlags(WState_TrackMouse);
		    w->setMouseTracking( FALSE );
		}
	    }
	    ++it;
	}
    }
}


/*****************************************************************************
  Routines to find a Qt widget from a screen position
 *****************************************************************************/

static QWidget *findChildWidget( const QWidget *p, const QPoint &pos )
{
    if ( p->children() ) {
	QWidget *w;
	QObjectListIt it( *p->children() );
	it.toLast();
	while ( it.current() ) {
	    if ( it.current()->isWidgetType() ) {
		w = (QWidget*)it.current();
		if ( w->isVisible() && w->geometry().contains(pos) ) {
		    QWidget *c = findChildWidget( w, w->mapFromParent(pos) );
		    return c ? c : w;
		}
	    }
	    --it;
	}
    }
    return 0;
}

Window qt_x11_findClientWindow( Window win, Atom WM_STATE, bool leaf )
{
    Atom   type = None;
    int	   format, i;
    ulong  nitems, after;
    uchar *data;
    Window root, parent, target=0, *children=0;
    uint   nchildren;
    XGetWindowProperty( appDpy, win, WM_STATE, 0, 0, False, AnyPropertyType,
			&type, &format, &nitems, &after, &data );
    if ( data )
	XFree( (char *)data );
    if ( type )
	return win;
    if ( !XQueryTree(appDpy,win,&root,&parent,&children,&nchildren) ) {
	if ( children )
	    XFree( (char *)children );
	return 0;
    }
    for ( i=nchildren-1; !target && i >= 0; i-- )
	target = qt_x11_findClientWindow( children[i], WM_STATE, leaf );
    if ( children )
	XFree( (char *)children );
    return target;
}


/*!
  Returns a pointer to the widget at global screen position \a (x,y), or a
  null pointer if there is no Qt widget there.

  If \a child is FALSE and there is a child widget at position \a (x,y),
  the top-level widget containing it is returned.  If \a child is TRUE
  the child widget at position \a (x,y) is returned.

  \sa QCursor::pos(), QWidget::grabMouse(), QWidget::grabKeyboard()
*/

QWidget *QApplication::widgetAt( int x, int y, bool child )
{
    int lx, ly;

    Window target;
    if ( !XTranslateCoordinates(appDpy, appRootWin, appRootWin,
				x, y, &lx, &ly, &target) )
	return 0;
    if ( !target || target == appRootWin )
	return 0;
    QWidget *w, *c;
    w = QWidget::find( (WId)target );
    if ( child && w ) {
	c = findChildWidget( w, w->mapFromParent(QPoint(lx,ly)) );
	if ( c )
	    return c;
    }

    if ( !qt_wm_state )
	return w;
    target = qt_x11_findClientWindow( target, qt_wm_state, TRUE );
    c = QWidget::find( (WId)target );
    if ( !c ) {
	if ( !w ) {
	    // Perhaps the widgets at (x,y) is inside a foreign application?
	    // Search all toplevel widgets to see if one is within target
	    QWidgetList *list   = topLevelWidgets();
	    QWidget     *widget = list->first();
	    while ( widget && !w ) {
		Window	ctarget = target;
		if ( widget->isVisible() && !widget->isDesktop() ) {
		    Window wid = widget->winId();
		    while ( ctarget && !w ) {
			XTranslateCoordinates(appDpy, appRootWin, ctarget,
			    x, y, &lx, &ly, &ctarget);
			if ( ctarget == wid ) {
			    // Found
			    w = widget;
			    XTranslateCoordinates(appDpy, appRootWin, ctarget,
				x, y, &lx, &ly, &ctarget);
			}
		    }
		}
		widget = list->next();
	    }
	    delete list;
	}
	c = w;
	if ( !w )
	    return w;
    }
    if ( child ) {
	c = findChildWidget( c, c->mapFromParent(QPoint(lx,ly)) );
	if ( !c )
	    c = w;
    }
    return c;
}

/*!
  \overload QWidget *QApplication::widgetAt( const QPoint &pos, bool child )
*/


/*!
  Flushes the X event queue in the X11 implementation.
  Does nothing on other platforms.
  \sa syncX()
*/

void QApplication::flushX()
{
    if ( appDpy )
	XFlush( appDpy );
}

/*!
  Synchronizes with the X server in the X11 implementation.
  Does nothing on other platforms.
  \sa flushX()
*/

void QApplication::syncX()
{
    if ( appDpy )
	XSync( appDpy, FALSE );			// don't discard events
}


/*!
  Sounds the bell, using the default volume and sound.
*/

void QApplication::beep()
{
    if ( appDpy )
	XBell( appDpy, 0 );
}



/*****************************************************************************
  QApplication management of posted events
 *****************************************************************************/

class QPEObject : public QObject		// trick to set/clear pendEvent
{
public:
    void setPendEventFlag()	{ pendEvent = TRUE; }
};

class QPEvent : public QEvent			// trick to set/clear posted
{
public:
    QPEvent( int type ) : QEvent( type ) {}
    void setPostedFlag()	{ posted = TRUE; }
    void clearPostedFlag()	{ posted = FALSE; }
};

struct QPostEvent {
    QPostEvent( QObject *r, QEvent *e ) { receiver=r; event=e; }
   ~QPostEvent()			{ delete event; }
    QObject  *receiver;
    QEvent   *event;
};

Q_DECLARE(QListM,QPostEvent);
Q_DECLARE(QListIteratorM,QPostEvent);
typedef QListM(QPostEvent)	   QPostEventList;
typedef QListIteratorM(QPostEvent) QPostEventListIt;
static QPostEventList *postedEvents = 0;	// list of posted events


/*!
  Stores the event in a queue and returns immediatly.

  The event must be allocated on the heap, as it is deleted when the event
  has been posted.

  When control returns to the main event loop, all events that are
  stored in the queue will be sent using the notify() function.

  \sa sendEvent()
*/

void QApplication::postEvent( QObject *receiver, QEvent *event )
{
    if ( !postedEvents ) {			// create list
	postedEvents = new QListM(QPostEvent);
	CHECK_PTR( postedEvents );
	postedEvents->setAutoDelete( TRUE );
    }
    if ( receiver == 0 ) {
#if defined(CHECK_NULL)
	warning( "QApplication::postEvent: Unexpeced null receiver" );
#endif
	return;
    }
    ((QPEObject*)receiver)->setPendEventFlag();
    ((QPEvent*)event)->setPostedFlag();
    postedEvents->append( new QPostEvent(receiver,event) );
}

void qt_x11SendPostedEvents()			// transmit posted events
{
    if ( !postedEvents )
	return;
    QPostEventListIt it(*postedEvents);
    QPostEvent *pe;
    while ( (pe=it.current()) ) {
	++it;
	postedEvents->take( postedEvents->findRef( pe ) );
	if ( pe->event ) {
	    if ( pe->event->type() == Event_LayoutHint ) {
		// layout hints are idempotent and can cause quite
		// expensive processing, so make sure to deliver just
		// one per receiver.
		QPostEventListIt it2( *postedEvents );
		it2 = it;
		++it2;
		QPostEvent * pe2;
		while ( (pe2=it2.current()) != 0 ) {
		    ++it2;
		    if ( pe2->event &&
			 pe2->event->type() == Event_LayoutHint &&
			 pe2->receiver == pe->receiver ) {
			((QPEvent*)pe2->event)->clearPostedFlag();
			postedEvents->removeRef( pe2 );
		    }
		}
	    }
	    QApplication::sendEvent( pe->receiver, pe->event );
	    ((QPEvent*)pe->event)->clearPostedFlag();
	}
	delete pe;
    }
}


/*!
  Immediately dispatches all events which have been previously enqueued
  with QApplication::postEvent() and which are for the object \a receiver
  and have the \a event_type.

  Some event compression may occur.  Note that events from the
  window system are \e not dispatched by this function.
*/
void QApplication::sendPostedEvents( QObject *receiver, int event_type )
{
    if ( !postedEvents )
	return;
    QPostEventListIt it(*postedEvents);
    QPostEvent *pe;

    // For accumulating compressed events
    QPoint oldpos, newpos;
    QSize oldsize, newsize;
    bool first=TRUE;

    while ( (pe = it.current()) ) {
	++it;
	
	if ( pe->event
	     && pe->receiver == receiver
	     && pe->event->type() == event_type )
	    {
		postedEvents->take( postedEvents->findRef( pe ) );
		switch ( event_type ) {
		case Event_Move:
		    if ( first ) {
			oldpos = ((QMoveEvent*)pe->event)->oldPos();
			first = FALSE;
		    }
		    newpos = ((QMoveEvent*)pe->event)->pos();
		    break;
		case Event_Resize:
		    if ( first ) {
			oldsize = ((QResizeEvent*)pe->event)->oldSize();
			first = FALSE;
		    }
		    newsize = ((QResizeEvent*)pe->event)->size();
		    break;
		default:
		    sendEvent( receiver, pe->event );
		}
		((QPEvent*)pe->event)->clearPostedFlag();
		delete pe;
	    }
    }
    if ( !first ) {
	// Got one
	switch ( event_type ) {
	case Event_Move:
	    {
		QMoveEvent e(newpos, oldpos);
		sendEvent( receiver, &e );
	    }
	    break;
	case Event_Resize:
	    {
		QResizeEvent e(newsize, oldsize);
		sendEvent( receiver, &e );
	    }
	    break;
	default:
	    ; // Nothing
	}
    }
}


void qRemovePostedEvents( QObject *receiver )	// remove receiver from list
{
    if ( !postedEvents )
	return;
    register QPostEvent *pe = postedEvents->first();
    while ( pe ) {
	if ( pe->receiver == receiver ) {	// remove this receiver
	    ((QPEvent*)pe->event)->clearPostedFlag();
	    postedEvents->remove();
	    pe = postedEvents->current();
	} else {
	    pe = postedEvents->next();
	}
    }
}

void qRemovePostedEvent( QEvent *event )	// remove event in list
{
    if ( !postedEvents )
	return;
    register QPostEvent *pe = postedEvents->first();
    while ( pe ) {
	if ( pe->event == event )		// make this event invalid
	    pe->event = 0;			//   will not be sent!
	pe = postedEvents->next();
    }
}

static void cleanupPostedEvents()		// cleanup list
{
    delete postedEvents;
    postedEvents = 0;
}


/*****************************************************************************
  Special lookup functions for windows that have been recreated recently
 *****************************************************************************/

static QWidgetIntDict *wPRmapper = 0;		// alternative widget mapper

void qPRCreate( const QWidget *widget, Window oldwin )
{						// QWidget::recreate mechanism
    if ( !wPRmapper ) {
	wPRmapper = new QWidgetIntDict;
	CHECK_PTR( wPRmapper );
    }
    wPRmapper->insert( (long)oldwin, widget );	// add old window to mapper
    QETWidget *w = (QETWidget *)widget;
    w->setWFlags( WRecreated );			// set recreated flag
}

void qPRCleanup( QETWidget *widget )
{
    if ( !(wPRmapper && widget->testWFlags(WRecreated)) )
	return;					// not a recreated widget
    QWidgetIntDictIt it(*wPRmapper);
    QWidget *w;
    while ( (w=it.current()) ) {
	if ( w == widget ) {			// found widget
	    widget->clearWFlags( WRecreated );	// clear recreated flag
	    wPRmapper->remove( it.currentKey());// old window no longer needed
	    if ( wPRmapper->count() == 0 ) {	// became empty
		delete wPRmapper;		// then reset alt mapper
		wPRmapper = 0;
	    }
	    return;
	}
	++it;
    }
}

QETWidget *qPRFindWidget( Window oldwin )
{
    return wPRmapper ? (QETWidget*)wPRmapper->find((long)oldwin) : 0;
}


/*****************************************************************************
  Socket notifier (type: 0=read, 1=write, 2=exception)

  The QSocketNotifier class (qsocketnotifier.h) provides installable callbacks
  for select() throught the internal function qt_set_socket_handler().
 *****************************************************************************/

struct QSockNot {
    QObject *obj;
    int	     fd;
    fd_set  *queue;
};

typedef Q_DECLARE(QListM,QSockNot)	   QSNList;
typedef Q_DECLARE(QListIteratorM,QSockNot) QSNListIt;

static int	sn_highest = -1;
static QSNList *sn_read	   = 0;
static QSNList *sn_write   = 0;
static QSNList *sn_except  = 0;

static fd_set	sn_readfds;			// fd set for reading
static fd_set	sn_writefds;			// fd set for writing
static fd_set	sn_exceptfds;			// fd set for exceptions
static fd_set	sn_queued_read;
static fd_set	sn_queued_write;
static fd_set	sn_queued_except;

static struct SN_Type {
    QSNList **list;
    fd_set   *fdspec;
    fd_set   *fdres;
    fd_set   *queue;
} sn_vec[3] = {
    { &sn_read,	  &sn_readfds,	 &app_readfds,   &sn_queued_read },
    { &sn_write,  &sn_writefds,	 &app_writefds,  &sn_queued_write },
    { &sn_except, &sn_exceptfds, &app_exceptfds, &sn_queued_except } };


static QSNList *sn_act_list = 0;


static void sn_cleanup()
{
    delete sn_act_list;
    sn_act_list = 0;
    for ( int i=0; i<3; i++ ) {
	delete *sn_vec[i].list;
	*sn_vec[i].list = 0;
    }	
}


static void sn_init()
{
    if ( !sn_act_list ) {
	sn_act_list = new QSNList;
	CHECK_PTR( sn_act_list );
	qAddPostRoutine( sn_cleanup );
    }
}


bool qt_set_socket_handler( int sockfd, int type, QObject *obj, bool enable )
{
    if ( sockfd < 0 || type < 0 || type > 2 || obj == 0 ) {
#if defined(CHECK_RANGE)
	warning( "QSocketNotifier: Internal error" );
#endif
	return FALSE;
    }

    QSNList  *list = *sn_vec[type].list;
    fd_set   *fds  =  sn_vec[type].fdspec;
    QSockNot *sn;

    if ( enable ) {				// enable notifier
	if ( !list ) {
	    sn_init();
	    list = new QSNList;			// create new list
	    CHECK_PTR( list );
	    list->setAutoDelete( TRUE );
	    *sn_vec[type].list = list;
	    FD_ZERO( fds );
	    FD_ZERO( sn_vec[type].queue );
	}
	sn = new QSockNot;
	CHECK_PTR( sn );
	sn->obj = obj;
	sn->fd	= sockfd;
	sn->queue = sn_vec[type].queue;
	if ( list->isEmpty() ) {
	    list->insert( 0, sn );
	} else {				// sort list by fd, decreasing
	    QSockNot *p = list->first();
	    while ( p && p->fd > sockfd )
		p = list->next();
#if defined(DEBUG)
	    if ( p && p->fd == sockfd ) {
		static const char *t[] = { "read", "write", "exception" };
		warning( "QSocketNotifier: Multiple socket notifiers for "
			 "same socket %d and type %s", sockfd, t[type] );
	    }
#endif	
	    if ( p )
		list->insert( list->at(), sn );
	    else
		list->append( sn );
	}
	FD_SET( sockfd, fds );
	sn_highest = QMAX(sn_highest,sockfd);

    } else {					// disable notifier

	if ( list == 0 )
	    return FALSE;			// no such fd set
	QSockNot *sn = list->first();
	while ( sn && !(sn->obj == obj && sn->fd == sockfd) )
	    sn = list->next();
	if ( !sn )				// not found
	    return FALSE;
	FD_CLR( sockfd, fds );			// clear fd bit
	FD_CLR( sockfd, sn->queue );
	if ( sn_act_list )
	    sn_act_list->removeRef( sn );	// remove from activation list
	list->remove();				// remove notifier found above
	if ( sn_highest == sockfd ) {		// find highest fd
	    sn_highest = -1;
	    for ( int i=0; i<3; i++ ) {
		if ( *sn_vec[i].list && (*sn_vec[i].list)->count() )
		    sn_highest = QMAX(sn_highest,  // list is fd-sorted
				      (*sn_vec[i].list)->getFirst()->fd);
	    }
	}
    }

    return TRUE;
}


//
// We choose a random activation order to be more fair under high load.
// If a constant order is used and a peer early in the list can
// saturate the IO, it might grab our attention completely.
// Also, if we're using a straight list, the callback routines may
// delete other entries from the list before those other entries are
// processed.
//

static int sn_activate()
{
    if ( !sn_act_list )
	sn_init();
    int i, n_act = 0;
    for ( i=0; i<3; i++ ) {			// for each list...
	if ( *sn_vec[i].list ) {		// any entries?
	    QSNList  *list = *sn_vec[i].list;
	    fd_set   *fds  = sn_vec[i].fdres;
	    QSockNot *sn   = list->first();
	    while ( sn ) {
		if ( FD_ISSET( sn->fd, fds ) &&	// store away for activation
		     !FD_ISSET( sn->fd, sn->queue ) ) {
		    sn_act_list->insert( (rand() & 0xff) %
					 (sn_act_list->count()+1),
					 sn );
		    FD_SET( sn->fd, sn->queue );
		}
		sn = list->next();
	    }
	}
    }
    if ( sn_act_list->count() > 0 ) {		// activate entries
	QEvent event( Event_SockAct );
	QSNListIt it( *sn_act_list );
	QSockNot *sn;
	while ( (sn=it.current()) ) {
	    ++it;
	    sn_act_list->removeRef( sn );
	    if ( FD_ISSET(sn->fd, sn->queue) ) {
		FD_CLR( sn->fd, sn->queue );
		QApplication::sendEvent( sn->obj, &event );
		n_act++;
	    }
        }
    }
    return n_act;
}


/*****************************************************************************
  Main event loop
 *****************************************************************************/

/*!
  Enters the main event loop and waits until exit() is called or
  the \link setMainWidget() main widget\endlink is destroyed.
  Returns the value that was specified to exit(), which is 0 if
  exit() is called via quit().

  It is necessary to call this function to start event handling.
  The main event loop receives \link QWidget::event() events\endlink from
  the window system and dispatches these to the application widgets.

  Generally, no user interaction can take place before calling exec().
  As a special case, modal widgets like QMessageBox can be used before
  calling exec(), because modal widget have a local event loop.

  To make your application perform idle processing, i.e. executing a
  special function whenever there are no pending events, use a QTimer
  with 0 timeout. More advanced idle processing schemes can be
  achieved by using processEvents() and processOneEvent().

  \sa quit(), exit(), processEvents(), setMainWidget(), QTimer
*/

int QApplication::exec()
{
    quit_now  = FALSE;
    quit_code = 0;
    enter_loop();
    return quit_code;
}


/*!
  Processes the next event and returns TRUE if there was an event
  (excluding posted events or zero-timer events) to process.

  This function returns immediately if \e canWait is FALSE. It might go
  into a sleep/wait state if \e canWait is TRUE.

  \sa processEvents()
*/

bool QApplication::processNextEvent( bool canWait )
{
    XEvent event;
    int	   nevents = 0;

    if ( postedEvents && postedEvents->count() )
	qt_x11SendPostedEvents();

    while ( XPending(appDpy) ) {		// also flushes output buffer
	if ( quit_now )				// quit between events
	    return FALSE;
	XNextEvent( appDpy, &event );		// get next event
	nevents++;

	if ( x11ProcessEvent( &event ) == 1 )
	    return TRUE;
    }

    if ( quit_now || app_exit_loop )		// break immediatly
	return FALSE;

    static timeval zerotm;
    timeval *tm = qt_wait_timer();		// wait for timer or X event
    if ( !canWait || postedEvents && postedEvents->count() ) {
	if ( !tm )
	    tm = &zerotm;
	tm->tv_sec  = 0;			// no time to wait
	tm->tv_usec = 0;
    }
    if ( sn_highest >= 0 ) {			// has socket notifier(s)
	if ( sn_read )
	    app_readfds = sn_readfds;
	else
	    FD_ZERO( &app_readfds );
	if ( sn_write )
	    app_writefds = sn_writefds;
	if ( sn_except )
	    app_exceptfds = sn_exceptfds;
    } else {
	FD_ZERO( &app_readfds );
    }
    FD_SET( app_Xfd, &app_readfds );

    int nsel;
    nsel = select( QMAX(app_Xfd,sn_highest)+1,
		   (void *) (&app_readfds),
		   (void *) (sn_write  ? &app_writefds  : 0),
		   (void *) (sn_except ? &app_exceptfds : 0),
		   tm );
#undef FDCAST

    if ( nsel == -1 ) {
	if ( errno == EINTR || errno == EAGAIN ) {
	    errno = 0;
	    return (nevents > 0);
	} else {
	    ; // select error
	}
    } else if ( nsel > 0 && sn_highest >= 0 ) {
	nevents += sn_activate();
    }

    nevents += qt_activate_timers();		// activate timers
    qt_reset_color_avail();			// color approx. optimization

    return (nevents > 0);
}


/*!
  Returns
  1 if the event was consumed by special handling,
  0 if the event was consumed by normal handling, and
  -1 if the event was for an unrecognized widget.

  \internal

  This documentation is unclear.
*/
int QApplication::x11ProcessEvent( XEvent* event )
{
    if ( x11EventFilter(event) )		// send through app filter
	return 1;

    QETWidget *widget = (QETWidget*)QWidget::find( (WId)event->xany.window );

    if ( wPRmapper ) {				// just did a widget recreate?
	if ( widget == 0 ) {			// not in std widget mapper
	    switch ( event->type ) {		// only for mouse/key events
	    case ButtonPress:
	    case ButtonRelease:
	    case MotionNotify:
	    case KeyPress:
	    case KeyRelease:
		widget = qPRFindWidget( event->xany.window );
		break;
	    }
	}
	else if ( widget->testWFlags(WRecreated) )
	    qPRCleanup( widget );		// remove from alt mapper
    }

    if ( event->type == MappingNotify ) {	// keyboard mapping changed
	XRefreshKeyboardMapping( &event->xmapping );
	return 0;
    }

    if ( !widget ) {				// don't know this window
	if ( (widget=(QETWidget*)QApplication::activePopupWidget()) )
	    {
		// Danger - make sure we don't lock the server
		switch ( event->type ) {
		case ButtonPress:
		case ButtonRelease:
		case KeyPress:
		case KeyRelease:
		    widget->hide();
		    return 1;
		}
	    } else {
		void qt_np_process_foreign_event(XEvent*); // in qnpsupport.cpp
		qt_np_process_foreign_event( event );
	    }
	return -1;
    }

    if ( app_do_modal )				// modal event handling
	if ( !qt_try_modal(widget, event) )
	    return 1;

    if ( popupWidgets ) {			// in popup mode
	switch ( event->type ) {
	    // Mouse and keyboard events are handled by the
	    // translate routines
	case FocusIn:
	case FocusOut:
	case EnterNotify:
	case LeaveNotify:
	    if ( popupFilter(widget) )
		return 1;
	    break;
	}
    }

    if ( widget->x11Event(event) )		// send through widget filter
	return 1;

    switch ( event->type ) {

    case ButtonPress:			// mouse event
    case ButtonRelease:
    case MotionNotify:
	qt_x_clipboardtime = (event->type == MotionNotify) ?
			     event->xmotion.time : event->xbutton.time;
	if ( widget->isEnabled() &&
	     event->type == ButtonPress &&
	     event->xbutton.button == Button1 &&
	     (widget->focusProxy()
	      ? (widget->focusProxy()->focusPolicy() & QWidget::ClickFocus)
	      : (widget->focusPolicy() & QWidget::ClickFocus) ) )
	    widget->setFocus();
	widget->translateMouseEvent( event );
	break;

    case KeyPress:				// keyboard event
    case KeyRelease: {
	qt_x_clipboardtime = event->xkey.time;
	QWidget *g = QWidget::keyboardGrabber();
	if ( g )
	    widget = (QETWidget*)g;
	else if ( focus_widget )
	    widget = (QETWidget*)focus_widget;
	else
	    widget = (QETWidget*)widget->topLevelWidget();
	if ( widget->isEnabled() )
	    widget->translateKeyEvent( event, g != 0 );
    }
    break;

    case GraphicsExpose:
    case Expose:				// paint event
	if ( widget->testWFlags( WState_DoHide ) ) {
	    widget->setWFlags( WState_Visible );
	    widget->hide();
	} else {
	    widget->translatePaintEvent( event );
	}
	break;

    case ConfigureNotify:			// window move/resize event
	widget->translateConfigEvent( event );
	break;

    case FocusIn: {				// got focus
	QWidget *w = widget->focusWidget();
	if ( !qApp->focus_widget || qApp->focus_widget != w ) { // short-circ
	    if ( qApp->focus_widget ) {
		// Inconsistent messages from X.  Force a focus out.
		QFocusEvent out( Event_FocusOut );
		QApplication::sendEvent( qApp->focus_widget, &out );
	    }
	    if ( w && (w->isFocusEnabled() || w->isTopLevel()) ) {
		qApp->focus_widget = w;
		QFocusEvent in( Event_FocusIn );
		QApplication::sendEvent( w, &in );
	    } else {
		// set focus to some arbitrary widget with WTabToFocus
		widget->topLevelWidget()->focusNextPrevChild( TRUE );
		focus_widget = widget->focusWidget();
	    }
	}
    }
    break;

    case FocusOut:				// lost focus
	if ( focus_widget ) {
	    QFocusEvent out( Event_FocusOut );
	    QWidget *w = focus_widget;
	    focus_widget = 0;
	    QApplication::sendEvent( w, &out );
	}
	break;

    case EnterNotify:			// enter window
    case LeaveNotify: {			// leave window
	QEvent e( event->type == EnterNotify ? Event_Enter : Event_Leave );
	QApplication::sendEvent( widget, &e );
    }
    break;

    case UnmapNotify:			// window hidden
	if ( widget->testWFlags( WState_Visible ) ) {
	    widget->clearWFlags( WState_Visible );
	    QHideEvent e(TRUE);
	    QApplication::sendEvent( widget, &e );
	}
	break;
	
    case MapNotify:				// window shown
	if ( !widget->testWFlags( WState_Visible ) ) {
	    widget->setWFlags( WState_Visible );
	    QShowEvent e(TRUE);
	    QApplication::sendEvent( widget, &e );
	}
	break;

    case ClientMessage:			// client message
	if ( event->xclient.format == 32 && event->xclient.message_type ) {
	    if ( event->xclient.message_type == qt_wm_protocols ) {
		long *l = event->xclient.data.l;
		if ( *l == (long)qt_wm_delete_window )
		    widget->translateCloseEvent(event);
	    } else if ( event->xclient.message_type == qt_qt_scrolldone ) {
		widget->translateScrollDoneEvent(event);
	    } else if ( event->xclient.message_type == qt_xdnd_position ) {
		qt_handle_xdnd_position( widget, event );
	    } else if ( event->xclient.message_type == qt_xdnd_enter ) {
		qt_handle_xdnd_enter( widget, event );
	    } else if ( event->xclient.message_type == qt_xdnd_status ) {
		qt_handle_xdnd_status( widget, event );
	    } else if ( event->xclient.message_type == qt_xdnd_leave ) {
		qt_handle_xdnd_leave( widget, event );
	    } else if ( event->xclient.message_type == qt_xdnd_drop ) {
		qt_handle_xdnd_drop( widget, event );
	    } else if ( event->xclient.message_type == qt_xdnd_finished ) {
		qt_handle_xdnd_finished( widget, event );
	    }
	}
	break;

    case ReparentNotify:			// window manager reparents
	if ( event->xreparent.parent != appRootWin
	     && !QWidget::find((WId)event->xreparent.parent) )
	    {
		XWindowAttributes a1, a2;
		while ( XCheckTypedWindowEvent( widget->x11Display(),
						widget->winId(),
						ReparentNotify,
						event ) )
		    ;				// skip old reparent events
		Window parent = event->xreparent.parent;
		XGetWindowAttributes( widget->x11Display(),
				      widget->winId(), &a1 );
		XGetWindowAttributes( widget->x11Display(), parent,
				      &a2 );
		QRect *r = &widget->crect;
		XWindowAttributes *a;
		if ( a1.x == 0 && a1.y == 0 && (a2.x + a2.y > 0)
		     && a2.x + a2.y < r->left() + r->top() )
		    a = &a2;			// typical for mwm, fvwm
		else
		    a = &a1;			// typical for twm, olwm
		a->x += a2.border_width;
		a->y += a2.border_width;
		widget->frect = QRect(QPoint(r->left()	 - a->x,
					     r->top()	 - a->y),
				      QPoint(r->right()	 + a->x,
					     r->bottom() + a->x) );
	    }
	break;

    case SelectionRequest:
	if ( qt_xdnd_selection ) {
	    XSelectionRequestEvent *req = &event->xselectionrequest;
	    if ( req && req->selection == qt_xdnd_selection ) {
		qt_xdnd_handle_selection_request( req );
		break;
	    }
	}
	// FALL THROUGH
    case SelectionClear:
    case SelectionNotify:
	if ( qt_clipboard ) {
	    QCustomEvent e( Event_Clipboard, event );
	    QApplication::sendEvent( qt_clipboard, &e );
	}
	break;

    default:
	break;
    }

    return 0;
}


/*!
  Processes pending events, for \a maxtime milliseconds or until there
  are no more events to process, then return.

  You can call this function occasionally when you program is busy doing a
  long operation (e.g. copying a file).

  \sa processOneEvent(), exec(), QTimer
*/

void QApplication::processEvents( int maxtime )
{
    QTime start = QTime::currentTime();
    QTime now;
    while ( !quit_now && processNextEvent(FALSE) ) {
	now = QTime::currentTime();
	if ( start.msecsTo(now) > maxtime )
	    break;
    }
}

/*!
  This function enters the main event loop (recursively).
  Do not call it unless you are an expert.
  \sa exit_loop()
*/

int QApplication::enter_loop()
{
    loop_level++;
    quit_now = FALSE;

    bool old_app_exit_loop = app_exit_loop;
    app_exit_loop = FALSE;

    while ( !quit_now && !app_exit_loop )
	processNextEvent( TRUE );

    app_exit_loop = old_app_exit_loop;
    loop_level--;

    return 0;
}


/*!
  This function leaves from a recursive call to the main event loop.
  Do not call it unless you are an expert.
  \sa enter_loop()
*/

void QApplication::exit_loop()
{
    app_exit_loop = TRUE;
}


/*!
  This virtual function is only implemented under X11.

  If you create an application that inherits QApplication and reimplement this
  function, you get direct access to all X events that the are received
  from the X server.

  Return TRUE if you want to stop the event from being dispatched, or return
  FALSE for normal event dispatching.
*/

bool QApplication::x11EventFilter( XEvent * )
{
    return FALSE;
}


/*****************************************************************************
  Modal widgets; Since Xlib has little support for this we roll our own
  modal widget mechanism.
  A modal widget without a parent becomes application-modal.
  A modal widget with a parent becomes modal to its parent and grandparents..

  qt_enter_modal()
	Enters modal state
	Arguments:
	    QWidget *widget	A modal widget

  qt_leave_modal()
	Leaves modal state for a widget
	Arguments:
	    QWidget *widget	A modal widget
 *****************************************************************************/

bool qt_modal_state()
{
    return app_do_modal;
}

void qt_enter_modal( QWidget *widget )
{
    if ( !modal_stack ) {			// create modal stack
	modal_stack = new QWidgetList;
	CHECK_PTR( modal_stack );
    }
    modal_stack->insert( 0, widget );
    app_do_modal = TRUE;
}


void qt_leave_modal( QWidget *widget )
{
    if ( modal_stack && modal_stack->removeRef(widget) ) {
	if ( modal_stack->isEmpty() ) {
	    delete modal_stack;
	    modal_stack = 0;
	}
    }
    app_do_modal = modal_stack != 0;
}


static bool qt_try_modal( QWidget *widget, XEvent *event )
{
    if ( popupWidgets )				// popup widget mode
	return TRUE;
    if ( widget->testWFlags(WStyle_Tool) )	// allow tool windows
	return TRUE;

    QWidget *modal=0, *top=modal_stack->getFirst();

    widget = widget->topLevelWidget();
    if ( widget->testWFlags(WType_Modal) )	// widget is modal
	modal = widget;
    if ( modal == top )				// don't block event
	return TRUE;

#ifdef ALLOW_NON_APPLICATION_MODAL
    if ( top && top->parentWidget() ) {
	// Not application-modal
	// Does widget have a child in modal_stack?
	bool unrelated = TRUE;
	modal = modal_stack->first();
	while (modal && unrelated) {
	    QWidget* p = modal->parentWidget();
	    while ( p && p != widget ) {
		p = p->parentWidget();
	    }
	    modal = modal_stack->next();
	    if ( p ) unrelated = FALSE;
	}
	if ( unrelated ) return TRUE;		// don't block event
    }
#endif

    bool block_event  = FALSE;
    bool expose_event = FALSE;

    switch ( event->type ) {
	case ButtonPress:			// disallow mouse/key events
	case ButtonRelease:
	case MotionNotify:
	case KeyPress:
	case KeyRelease:
	case FocusIn:
	case FocusOut:
	case ClientMessage:
	    block_event	 = TRUE;
	    break;
	case Expose:
	    expose_event = TRUE;
	    break;
    }

    if ( top->parentWidget() == 0 && (block_event || expose_event) )
	XRaiseWindow( appDpy, top->winId() );	// raise app-modal widget

    return !block_event;
}


/*****************************************************************************
  Popup widget mechanism

  qt_open_popup()
	Adds a widget to the list of popup widgets
	Arguments:
	    QWidget *widget	The popup widget to be added

  qt_close_popup()
	Removes a widget from the list of popup widgets
	Arguments:
	    QWidget *widget	The popup widget to be removed
 *****************************************************************************/

bool popupFilter( QWidget *widget )
{
    if ( popupWidgets ) {			// maybe eat the event
	return widget != popupWidgets->last();
    } else {
	return FALSE;				// don't eat the event
    }
}

void qt_open_popup( QWidget *popup )
{
    if ( !popupWidgets ) {			// create list
	popupWidgets = new QWidgetList;
	CHECK_PTR( popupWidgets );
    }
    popupWidgets->append( popup );		// add to end of list
    if ( popupWidgets->count() == 1 && !qt_nograb() ){ // grab mouse/keyboard
	int r;
	r = XGrabKeyboard( popup->x11Display(), popup->winId(), TRUE,
			   GrabModeSync, GrabModeSync, CurrentTime );
	if ( (popupGrabOk = (r == GrabSuccess)) ) {
	    XAllowEvents( popup->x11Display(), SyncKeyboard, CurrentTime );
	    r = XGrabPointer( popup->x11Display(), popup->winId(), TRUE,
			      (uint)(ButtonPressMask | ButtonReleaseMask |
				     ButtonMotionMask | EnterWindowMask |
				     LeaveWindowMask | PointerMotionMask),
			      GrabModeSync, GrabModeAsync,
			      None, None, CurrentTime );
	    if ( (popupGrabOk = (r == GrabSuccess)) ) {
		XAllowEvents( popup->x11Display(), SyncPointer, CurrentTime );
	    } else {
		XUngrabKeyboard( popup->x11Display(), CurrentTime );
	    }
	}
    }
}

void qt_close_popup( QWidget *popup )
{
    if ( !popupWidgets )
	return;
    popupWidgets->removeRef( popup );
    if ( popupWidgets->count() == 0 ) {		// this was the last popup
	popupCloseDownMode = TRUE;		// control mouse events
	delete popupWidgets;
	popupWidgets = 0;
	if ( !qt_nograb() && popupGrabOk ) {	// grabbing not disabled
	    XUngrabKeyboard( popup->x11Display(), CurrentTime );
	    if ( mouseButtonState != 0 ) {	// mouse release event
		XAllowEvents( popup->x11Display(), AsyncPointer,
			      CurrentTime );
	    } else {				// mouse press event
		mouseButtonPressTime -= 10000;	// avoid double click
		XAllowEvents( popup->x11Display(), ReplayPointer,CurrentTime );
	    }
	    XFlush( popup->x11Display() );
	}
    }
}


/*****************************************************************************
  Functions returning the active popup and modal widgets.
 *****************************************************************************/

/*!
  Returns the active popup widget.

  A popup widget is a special top level widget that sets the WType_Popup
  widget flag, e.g. the QPopupMenu widget.  When the application opens a
  popup widget, all events are sent to the popup and normal widgets and
  modal widgets cannot be accessed before the popup widget is closed.

  Only other popup widgets may be opened when a popup widget is shown.
  The popup widgets are organized in a stack.
  This function returns the active popup widget on top of the stack.

  \sa currentModalWidget(), topLevelWidgets()
*/

QWidget *QApplication::activePopupWidget()
{
    return popupWidgets ? popupWidgets->getLast() : 0;
}


/*!
  Returns the active modal widget.

  A modal widget is a special top level widget which is a subclass of
  QDialog that specifies the modal parameter of the constructor to TRUE.
  A modal widget must be finished before the user can continue with other
  parts of the program.

  The modal widgets are organized in a stack.
  This function returns the active modal widget on top of the stack.

  \sa currentPopupWidget(), topLevelWidgets()
*/

QWidget *QApplication::activeModalWidget()
{
    return modal_stack ? modal_stack->getLast() : 0;
}


/*****************************************************************************
  Timer handling; Xlib has no application timer support so we'll have to
  make our own from scratch.

  NOTE: These functions are for internal use. QObject::startTimer() and
	QObject::killTimer() are for public use.
	The QTimer class provides a high-level interface which translates
	timer events into signals.

  qStartTimer( interval, obj )
	Starts a timer which will run until it is killed with qKillTimer()
	Arguments:
	    int interval	timer interval in milliseconds
	    QObject *obj	where to send the timer event
	Returns:
	    int			timer identifier, or zero if not successful

  qKillTimer( timerId )
	Stops a timer specified by a timer identifier.
	Arguments:
	    int timerId		timer identifier
	Returns:
	    bool		TRUE if successful

  qKillTimer( obj )
	Stops all timers that are sent to the specified object.
	Arguments:
	    QObject *obj	object receiving timer events
	Returns:
	    bool		TRUE if successful
 *****************************************************************************/

//
// Internal data structure for timers
//

struct TimerInfo {				// internal timer info
    int	     id;				// - timer identifier
    timeval  interval;				// - timer interval
    timeval  timeout;				// - when to sent event
    QObject *obj;				// - object to receive event
};

typedef Q_DECLARE(QListM,TimerInfo) TimerList;	// list of TimerInfo structs

static QBitArray *timerBitVec;			// timer bit vector
static TimerList *timerList	= 0;		// timer list


//
// Internal operator functions for timevals
//

static inline bool operator<( const timeval &t1, const timeval &t2 )
{
    return t1.tv_sec < t2.tv_sec ||
	  (t1.tv_sec == t2.tv_sec && t1.tv_usec < t2.tv_usec);
}

static inline timeval &operator+=( timeval &t1, const timeval &t2 )
{
    t1.tv_sec += t2.tv_sec;
    if ( (t1.tv_usec += t2.tv_usec) >= 1000000 ) {
	t1.tv_sec++;
	t1.tv_usec -= 1000000;
    }
    return t1;
}

static inline timeval operator+( const timeval &t1, const timeval &t2 )
{
    timeval tmp;
    tmp.tv_sec = t1.tv_sec + t2.tv_sec;
    if ( (tmp.tv_usec = t1.tv_usec + t2.tv_usec) >= 1000000 ) {
	tmp.tv_sec++;
	tmp.tv_usec -= 1000000;
    }
    return tmp;
}

static inline timeval operator-( const timeval &t1, const timeval &t2 )
{
    timeval tmp;
    tmp.tv_sec = t1.tv_sec - t2.tv_sec;
    if ( (tmp.tv_usec = t1.tv_usec - t2.tv_usec) < 0 ) {
	tmp.tv_sec--;
	tmp.tv_usec += 1000000;
    }
    return tmp;
}


//
// Internal functions for manipulating timer data structures.
// The timerBitVec array is used for keeping track of timer identifiers.
//

static int allocTimerId()			// find avail timer identifier
{
    int i = timerBitVec->size()-1;
    while ( i >= 0 && (*timerBitVec)[i] )
	i--;
    if ( i < 0 ) {
	i = timerBitVec->size();
	timerBitVec->resize( 4 * i );
	for( int j=timerBitVec->size()-1; j > i; j-- )
	    timerBitVec->clearBit( j );
    }
    timerBitVec->setBit( i );
    return i+1;
}

static void insertTimer( const TimerInfo *ti )	// insert timer info into list
{
    TimerInfo *t = timerList->first();
    int index = 0;
    while ( t && t->timeout < ti->timeout ) {	// list is sorted by timeout
	t = timerList->next();
	index++;
    }
    timerList->insert( index, ti );		// inserts sorted
}

static inline void getTime( timeval &t )	// get time of day
{
    gettimeofday( &t, 0 );
    while ( t.tv_usec >= 1000000 ) {		// NTP-related fix
	t.tv_usec -= 1000000;
	t.tv_sec++;
    }
    while ( t.tv_usec < 0 ) {
	if ( t.tv_sec > 0 ) {
	    t.tv_usec += 1000000;
	    t.tv_sec--;
	} else {
	    t.tv_usec = 0;
	    break;
	}
    }
}

static void repairTimer( const timeval &time )	// repair broken timer
{
    if ( !timerList )				// not initialized
	return;
    timeval diff = watchtime - time;
    register TimerInfo *t = timerList->first();
    while ( t ) {				// repair all timers
	t->timeout = t->timeout - diff;
	t = timerList->next();
    }
}


//
// Timer activation functions (called from the event loop)
//

/*
  Returns the time to wait for the next timer, or null if no timers are
  waiting.
*/

timeval *qt_wait_timer()
{
    static timeval tm;
    bool first = TRUE;
    timeval currentTime;
    if ( timerList && timerList->count() ) {	// there are waiting timers
	getTime( currentTime );
	if ( first ) {
	    if ( currentTime < watchtime )	// clock was turned back
		repairTimer( currentTime );
	    first = FALSE;
	    watchtime = currentTime;
	}
	TimerInfo *t = timerList->first();	// first waiting timer
	if ( currentTime < t->timeout ) {	// time to wait
	    tm = t->timeout - currentTime;
	} else {
	    tm.tv_sec  = 0;			// no time to wait
	    tm.tv_usec = 0;
	}
	return &tm;
    }
    return 0;					// no timers
}

/*
  Activates the timer events that have expired. Returns the number of timers
  (not 0-timer) that were activated.
*/

int qt_activate_timers()
{
    if ( !timerList || !timerList->count() )	// no timers
	return 0;
    bool first = TRUE;
    timeval currentTime;
    int maxcount = timerList->count();
    int n_act = 0;
    register TimerInfo *t;
    while ( maxcount-- ) {			// avoid starvation
	getTime( currentTime );			// get current time
	if ( first ) {
	    if ( currentTime < watchtime )	// clock was turned back
		repairTimer( currentTime );
	    first = FALSE;
	    watchtime = currentTime;
	}
	t = timerList->first();
	if ( !t || currentTime < t->timeout )	// no timer has expired
	    break;
	timerList->take();			// unlink from list
	t->timeout += t->interval;
	if ( t->timeout < currentTime )
	    t->timeout = currentTime + t->interval;
	insertTimer( t );			// relink timer
	if ( t->interval.tv_usec > 0 || t->interval.tv_sec > 0 )
	    n_act++;
	QTimerEvent e( t->id );
	QApplication::sendEvent( t->obj, &e );	// send event
    }
    return n_act;
}


//
// Timer initialization and cleanup routines
//

static void initTimers()			// initialize timers
{
    timerBitVec = new QBitArray( 128 );
    CHECK_PTR( timerBitVec );
    int i = timerBitVec->size();
    while( i-- > 0 )
	timerBitVec->clearBit( i );
    timerList = new TimerList;
    CHECK_PTR( timerList );
    timerList->setAutoDelete( TRUE );
}

static void cleanupTimers()			// cleanup timer data structure
{
    if ( timerList ) {
	delete timerList;
	timerList = 0;
	delete timerBitVec;
	timerBitVec = 0;
    }
}


//
// Main timer functions for starting and killing timers
//

int qStartTimer( int interval, QObject *obj )
{
    if ( !timerList )				// initialize timer data
	initTimers();
    int id = allocTimerId();			// get free timer id
    if ( id <= 0 ||
	 id > (int)timerBitVec->size() || !obj )// cannot create timer
	return 0;
    timerBitVec->setBit( id-1 );		// set timer active
    TimerInfo *t = new TimerInfo;		// create timer
    CHECK_PTR( t );
    t->id = id;
    t->interval.tv_sec  = interval/1000;
    t->interval.tv_usec = (interval%1000)*1000;
    timeval currentTime;
    getTime( currentTime );
    t->timeout = currentTime + t->interval;
    t->obj = obj;
    insertTimer( t );				// put timer in list
    return id;
}

bool qKillTimer( int id )
{
    register TimerInfo *t;
    if ( !timerList || id <= 0 ||
	 id > (int)timerBitVec->size() || !timerBitVec->testBit( id-1 ) )
	return FALSE;				// not init'd or invalid timer
    t = timerList->first();
    while ( t && t->id != id )			// find timer info in list
	t = timerList->next();
    if ( t ) {					// id found
	timerBitVec->clearBit( id-1 );		// set timer inactive
	return timerList->remove();
    }
    else					// id not found
	return FALSE;
}

bool qKillTimer( QObject *obj )
{
    register TimerInfo *t;
    if ( !timerList )				// not initialized
	return FALSE;
    t = timerList->first();
    while ( t ) {				// check all timers
	if ( t->obj == obj ) {			// object found
	    timerBitVec->clearBit( t->id-1 );
	    timerList->remove();
	    t = timerList->current();
	} else {
	    t = timerList->next();
	}
    }
    return TRUE;
}


/*****************************************************************************
  Event translation; translates X11 events to Qt events
 *****************************************************************************/

//
// Mouse event translation
//
// Xlib doesn't give mouse double click events, so we generate them by
// comparing window, time and position between two mouse press events.
//

int translateButtonState( int s )
{
    int bst = 0;
    if ( s & Button1Mask )
	bst |= LeftButton;
    if ( s & Button2Mask )
	bst |= MidButton;
    if ( s & Button3Mask )
	bst |= RightButton;
    if ( s & ShiftMask )
	bst |= ShiftButton;
    if ( s & ControlMask )
	bst |= ControlButton;
    if ( s & Mod1Mask )
	bst |= AltButton;
    return bst;
}

bool QETWidget::translateMouseEvent( const XEvent *event )
{
    static bool manualGrab = FALSE;
    int	   type;				// event parameters
    QPoint pos;
    QPoint globalPos;
    int	   button = 0;
    int	   state;

    if ( event->type == MotionNotify ) {	// mouse move
	XEvent *xevent = (XEvent *)event;
	while ( XCheckTypedWindowEvent(dpy,winId(),MotionNotify,xevent) )
	    ;					// compress motion events
	type = Event_MouseMove;
	pos.rx() = xevent->xmotion.x;
	pos.ry() = xevent->xmotion.y;
	globalPos.rx() = xevent->xmotion.x_root;
	globalPos.ry() = xevent->xmotion.y_root;
	state = translateButtonState( xevent->xmotion.state );
	if ( !qt_button_down )
	    state &= ~(LeftButton|MidButton|RightButton);
    } else {					// button press or release
	pos.rx() = event->xbutton.x;
	pos.ry() = event->xbutton.y;
	globalPos.rx() = event->xbutton.x_root;
	globalPos.ry() = event->xbutton.y_root;
	switch ( event->xbutton.button ) {
	    case Button1: button = LeftButton;	break;
	    case Button2: button = MidButton;	break;
	    case Button3: button = RightButton; break;
	}
	state = translateButtonState( event->xbutton.state );
	if ( event->type == ButtonPress ) {	// mouse button pressed
	    qt_button_down = this;
	    if ( mouseActWindow == event->xbutton.window &&
		 mouseButtonPressed == button &&
		 (long)event->xbutton.time -(long)mouseButtonPressTime
                       < QApplication::doubleClickInterval() &&
		 QABS(event->xbutton.x - mouseXPos) < 5 &&
		 QABS(event->xbutton.y - mouseYPos) < 5 ) {
		type = Event_MouseButtonDblClick;
		mouseButtonPressTime -= 2000;	// no double-click next time
	    } else {
		type = Event_MouseButtonPress;
		mouseButtonPressTime = event->xbutton.time;
	    }
	    mouseButtonPressed = button; 	// save event params for
	    mouseXPos = pos.x();		// future double click tests
	    mouseYPos = pos.y();
	} else {				// mouse button released
	    if ( manualGrab ) {			// release manual grab
		manualGrab = FALSE;
		XUngrabPointer( dpy, CurrentTime );
		XFlush( dpy );
	    }
	    bool unexpected = FALSE;
	    if ( qt_button_down != this && !popupWidgets )
		unexpected = TRUE;

	    if ( (state &  (~button) & (LeftButton|MidButton|RightButton)) == 0 )
		qt_button_down = 0;

	    if ( unexpected )
		return FALSE;			// unexpected event

	    type = Event_MouseButtonRelease;
	}
    }
    mouseActWindow = winId();			// save some event params
    mouseButtonState = state; 			
    if ( type == 0 )				// don't send event
	return FALSE;

    if ( popupWidgets ) {			// in popup mode
	QWidget *popup = popupWidgets->last();
	if ( popup != this ) {
	    if ( testWFlags(WType_Popup) && rect().contains(pos) )
		popup = this;
	    else				// send to last popup
		pos = popup->mapFromGlobal( mapToGlobal(pos) );
	}

	bool releaseAfter = FALSE;
	QWidget *popupChild  = findChildWidget( popup, pos );
	QWidget *popupTarget = popupChild ? popupChild : popup;

	if ( !popupTarget->isEnabled() )
	    return FALSE;

	switch ( type ) {
	    case Event_MouseButtonPress:
	    case Event_MouseButtonDblClick:
		popupButtonFocus = popupChild;
		break;
	    case Event_MouseButtonRelease:
		releaseAfter = TRUE;
		break;
	    default:
		break;				// nothing for mouse move
	}

	if ( popupButtonFocus ) {
	    QMouseEvent e( type, popupButtonFocus->
			   mapFromGlobal(popup->mapToGlobal(pos)),
			   globalPos,
			   button, state );
	    QApplication::sendEvent( popupButtonFocus, &e );
	    if ( releaseAfter )
		popupButtonFocus = 0;
	} else {
	    QMouseEvent e( type, pos, globalPos, button, state );
	    QApplication::sendEvent( popup, &e );
	}

	if ( popupWidgets ) {			// still in popup mode
	    if ( popupGrabOk )
		XAllowEvents( dpy, SyncPointer, CurrentTime );
	} else {				// left popup mode
	    if ( type != Event_MouseButtonRelease && state != 0 &&
		 QWidget::find((WId)mouseActWindow) ) {
		manualGrab = TRUE;		// need to manually grab
		XGrabPointer( dpy, mouseActWindow, FALSE,
			      (uint)(ButtonPressMask | ButtonReleaseMask |
			      ButtonMotionMask |
			      EnterWindowMask | LeaveWindowMask),
			      GrabModeAsync, GrabModeAsync,
			      None, None, CurrentTime );
	    }
	}
    } else {
	QWidget *widget = this;
	QWidget *mg = QWidget::mouseGrabber();
	if ( mg && mg != this ) {
	    widget = mg;
	    pos = mapToGlobal( pos );
	    pos = mg->mapFromGlobal( pos );
	}

	if ( popupCloseDownMode ) {
	    popupCloseDownMode = FALSE;
	    if ( testWFlags(WType_Popup) )	// ignore replayed event
		return TRUE;
	    QMouseEvent* e = new QMouseEvent( type, pos, globalPos, button, state );
	    QApplication::postEvent( widget, e );
	}
	else {
	    QMouseEvent e( type, pos, globalPos, button, state );
	    QApplication::sendEvent( widget, &e );
	}
    }
    return TRUE;
}


//
// Keyboard event translation
//

#define XK_MISCELLANY
#define XK_LATIN1
#include <X11/keysymdef.h>
#include "qkeycode.h"

#ifndef XK_ISO_Left_Tab
#define	XK_ISO_Left_Tab					0xFE20
#endif
static KeySym KeyTbl[] = {			// keyboard mapping table
    XK_Escape,		Key_Escape,		// misc keys
    XK_Tab,		Key_Tab,
    XK_ISO_Left_Tab,    Key_Backtab,
    XK_BackSpace,	Key_Backspace,
    XK_Return,		Key_Return,
    XK_Insert,		Key_Insert,
    XK_KP_Insert,		Key_Insert,
    XK_Delete,		Key_Delete,
    XK_KP_Delete,		Key_Delete,
    XK_Clear,		Key_Delete,
    XK_Pause,		Key_Pause,
    XK_Print,		Key_Print,
    0x1005FF60,		Key_SysReq,		// hardcoded Sun SysReq
    0x1007ff00,		Key_SysReq,		// hardcoded X386 SysReq
    XK_Home,		Key_Home,		// cursor movement
    XK_End,		Key_End,
    XK_Left,		Key_Left,
    XK_Up,		Key_Up,
    XK_Right,		Key_Right,
    XK_Down,		Key_Down,
    XK_Prior,		Key_Prior,
    XK_Next,		Key_Next,
    XK_KP_Home,		Key_Home,
    XK_KP_End,		Key_End,
    XK_KP_Left,		Key_Left,
    XK_KP_Up,		Key_Up,
    XK_KP_Right,		Key_Right,
    XK_KP_Down,		Key_Down,
    XK_KP_Prior,		Key_Prior,
    XK_KP_Next,		Key_Next,
    XK_Shift_L,		Key_Shift,		// modifiers
    XK_Shift_R,		Key_Shift,
    XK_Shift_Lock,	Key_Shift,
    XK_Control_L,	Key_Control,
    XK_Control_R,	Key_Control,
    XK_Meta_L,		Key_Meta,
    XK_Meta_R,		Key_Meta,
    XK_Alt_L,		Key_Alt,
    XK_Alt_R,		Key_Alt,
    XK_Caps_Lock,	Key_CapsLock,
    XK_Num_Lock,	Key_NumLock,
    XK_Scroll_Lock,	Key_ScrollLock,
    XK_KP_Space,	Key_Space,		// numeric keypad
    XK_KP_Tab,		Key_Tab,
    XK_KP_Enter,	Key_Enter,
    XK_KP_Equal,	Key_Equal,
    XK_KP_Multiply,	Key_Asterisk,
    XK_KP_Add,		Key_Plus,
    XK_KP_Separator,	Key_Comma,
    XK_KP_Subtract,	Key_Minus,
    XK_KP_Decimal,	Key_Period,
    XK_KP_Divide,	Key_Slash,
    XK_Super_L,		Key_Super_L,
    XK_Super_R,		Key_Super_R,
    XK_Menu,		Key_Menu,
    XK_Hyper_L,		Key_Hyper_L,
    XK_Hyper_R,		Key_Hyper_R,
    0,			0
};


static QIntDict<void>    *keyDict   = 0;
static QIntDict<QString> *asciiDict = 0;

static void deleteKeyDicts()
{
    if ( keyDict )
	delete keyDict;
    keyDict = 0;
    if ( asciiDict )
	delete asciiDict;
    asciiDict = 0;
}


bool QETWidget::translateKeyEvent( const XEvent *event, bool grab )
{
    int	   type;
    int	   code = -1;
    char   ascii[16];
    int	   count = 0;
    int	   state;
    KeySym key = 0;

    if ( !keyDict ) {
	keyDict = new QIntDict<void>( 13 );
	keyDict->setAutoDelete( FALSE );
	asciiDict = new QIntDict<QString>( 13 );
	asciiDict->setAutoDelete( TRUE );
	qAddPostRoutine( deleteKeyDicts );
    }

    type = (event->type == KeyPress) ? Event_KeyPress : Event_KeyRelease;

#if defined(NO_XIM)

    count = XLookupString( &((XEvent*)event)->xkey, ascii, 16, &key, 0 );

#else
    // Implementation for X11R5 and newer, using XIM

    static int composingKeycode;
    int	       keycode = event->xkey.keycode;
    Status     status;

    if ( type == Event_KeyPress ) {
	if ( xim ) {
	    QWExtra * xd = extraData();
	    if ( !xd ) {
		createExtra();
		xd = extraData();
	    }
	    if ( xd->xic == 0 )
		xd->xic = (void*)XCreateIC( xim, XNInputStyle,
					    XIMPreeditNothing+XIMStatusNothing,
					    XNClientWindow, winId(),
					    0 );
	    if ( XFilterEvent( (XEvent*)event, winId() ) ) {
		composingKeycode = keycode; // ### not documented in xlib
		return TRUE;
	    }
	    count = XmbLookupString( (XIC)(xd->xic), &((XEvent*)event)->xkey,
				     ascii, 16, &key, &status );
	} else {
	    count = XLookupString( &((XEvent*)event)->xkey,
				   ascii, 16, &key, 0 );
	}
	if ( count && !keycode ) {
	    keycode = composingKeycode;
	    composingKeycode = 0;
	}
	keyDict->replace( keycode, (void*)key );
	if ( count < 15 )
	    ascii[count] = '\0';
	if ( count )
	    asciiDict->replace( keycode, new QString(ascii) );
    } else {
	key = (int)(long)keyDict->find( keycode );
	if ( key )
	    keyDict->take( keycode );
	QString * s = asciiDict->find( keycode );
	if ( s ) {
	    asciiDict->take( keycode );
	    qstrcpy( ascii, *s );
	    count = qstrlen( ascii );
	    delete s;
	}
    }
#endif // !NO_XIM

    state = translateButtonState( event->xkey.state );

    // commentary in X11/keysymdef says that X codes match ASCII, so it
    // is safe to use the locale functions to process X codes
    if ( key < 256 ) {
	code = isprint((int)key) ? toupper((int)key) : 0; // upper-case key, if known
    } else if ( key >= XK_F1 && key <= XK_F35 ) {
	code = Key_F1 + ((int)key - XK_F1);	// function keys
    } else if ( key >= XK_KP_0 && key <= XK_KP_9){
	code = Key_0 + ((int)key - XK_KP_0);	// numeric keypad keys
    } else {
	int i = 0;				// any other keys
	while ( KeyTbl[i] ) {
	    if ( key == KeyTbl[i] ) {
		code = (int)KeyTbl[i+1];
		break;
	    }
	    i += 2;
	}
	if ( code == Key_Tab && (state & ShiftButton) == ShiftButton ) {
	    code = Key_Backtab;
	    ascii[0] = 0;
	}
    }
#if defined(DEBUG)
    if ( debug_level > 0
	 && type==Event_KeyPress
	 && code==Key_D
	 && (state&ControlButton)
	 && (state&AltButton) )
	{
	    QWidgetList *list   = qApp->topLevelWidgets();
	    QWidget     *widget = list->first();
	    while ( widget ) {
		debug("Top-level widget %p", widget);
		widget->dumpObjectTree();
		widget = list->next();
	    }
	    delete list;
	    return TRUE;
	}
#endif
    QKeyEvent e( type, code, count > 0 ? ascii[0] : 0, state );
    if ( popupWidgets ) {			// in popup mode
	QWidget *popup = popupWidgets->last();
	QApplication::sendEvent( popup, &e );	// send event to popup instead
	if ( popupWidgets ) {			// still in popup mode
	    if ( popupGrabOk )
		XAllowEvents( dpy, SyncKeyboard, CurrentTime );
	}
	return TRUE;
    }
    if ( type == Event_KeyPress && !grab ) {	// send accel event to tlw
	QKeyEvent a( Event_Accel, code, count > 0 ? ascii[0] : 0, state );
	a.ignore();
	QApplication::sendEvent( topLevelWidget(), &a );
	if ( a.isAccepted() )
	    return TRUE;
    }
    return QApplication::sendEvent( this, &e );
}


//
// Paint event translation
//
// When receiving many expose events, we compress them (union of all expose
// rectangles) into one event which is sent to the widget.
// Some X servers send expose events before resize (configure) events.
// We try to remedy that, too.
//

struct PaintEventInfo {
    Window window;
    int	   w, h;
    bool   check;
    int	   config;
};

#if defined(Q_C_CALLBACKS)
extern "C" {
#endif

static Bool isPaintOrScrollDoneEvent( Display *, XEvent *ev, XPointer a )
{
    PaintEventInfo *info = (PaintEventInfo *)a;
    if ( ev->type == Expose || ev->type == GraphicsExpose
      ||    ev->type == ClientMessage
	 && ev->xclient.message_type == qt_qt_scrolldone )
    {
	if ( ev->xexpose.window == info->window )
	    return TRUE;
    } else if ( ev->type == ConfigureNotify && info->check ) {
	XConfigureEvent *c = (XConfigureEvent *)ev;
	if ( c->window == info->window &&
	     (c->width != info->w || c->height != info->h) ) {
	    info->w = c->width;
	    info->h = c->height;
	    info->config++;
	    return TRUE;
	}
    }
    return FALSE;
}

#if defined(Q_C_CALLBACKS)
}
#endif

struct QScrollInProgress {
    static long serial;
    QScrollInProgress( QWidget* w, int x, int y ) :
	id( serial++ ), scrolled_widget( w ), dx( x ), dy( y )
    {
    }

    long id;
    QWidget* scrolled_widget;
    int dx, dy;
};

long QScrollInProgress::serial=0;

static QList<QScrollInProgress> *sip_list = 0;

void qt_insert_sip( QWidget* scrolled_widget, int dx, int dy )
{
    if ( !sip_list ) {
	sip_list = new QList<QScrollInProgress>;
	sip_list->setAutoDelete( TRUE );
    }

    QScrollInProgress* sip = new QScrollInProgress( scrolled_widget, dx, dy );
    sip_list->append( sip );

    XClientMessageEvent client_message;
    client_message.type = ClientMessage;
    client_message.window = scrolled_widget->winId();
    client_message.format = 32;
    client_message.message_type = qt_qt_scrolldone;
    client_message.data.l[0] = sip->id;

    XSendEvent( appDpy, scrolled_widget->winId(), FALSE, NoEventMask,
	(XEvent*)&client_message );
}

int qt_sip_count( QWidget* scrolled_widget )
{
    if ( !sip_list )
	return 0;

    int sips=0;

    for (QScrollInProgress* sip = sip_list->first();
	sip; sip=sip_list->next())
    {
	if ( sip->scrolled_widget == scrolled_widget )
	    sips++;
    }

    return sips;
}

static
bool translateBySips( QWidget* that, QRect& paintRect )
{
    if ( sip_list ) {
	int dx=0, dy=0;
	int sips=0;
	for (QScrollInProgress* sip = sip_list->first();
	    sip; sip=sip_list->next())
	{
	    if ( sip->scrolled_widget == that ) {
		if ( sips ) {
		    dx += sip->dx;
		    dy += sip->dy;
		}
		sips++;
	    }
	}
	if ( sips > 1 ) {
	    paintRect.moveBy( dx, dy );
	    return TRUE;
	}
    }
    return FALSE;
}

bool QETWidget::translatePaintEvent( const XEvent *event )
{
    QRect  paintRect( event->xexpose.x,	   event->xexpose.y,
		      event->xexpose.width, event->xexpose.height );
    bool   merging_okay = !testWFlags(WPaintClever);
    XEvent xevent;
    PaintEventInfo info;

    info.window = winId();
    info.w	= width();
    info.h	= height();
    info.check	= testWFlags(WType_TopLevel);
    info.config = 0;
    bool should_clip = translateBySips( this, paintRect );

    if ( merging_okay ) {
	while ( XCheckIfEvent(dpy,&xevent,isPaintOrScrollDoneEvent,(XPointer)&info)
	    && !qApp->x11EventFilter(&xevent) )	// send event through filter
	{
	    if ( !info.config ) {
		if ( xevent.type == Expose || xevent.type == GraphicsExpose ) {
		    QRect exposure(xevent.xexpose.x,
				   xevent.xexpose.y,
				   xevent.xexpose.width,
				   xevent.xexpose.height);
		    if ( translateBySips( this, exposure ) )
			should_clip = TRUE;
		    paintRect = paintRect.unite( exposure );
		} else {
		    translateScrollDoneEvent( &xevent );
		}
	    }
	}
    }

    if ( info.config ) {
	XConfigureEvent *c = (XConfigureEvent *)&xevent;
	c->window  = info.window;
	c->event  = info.window;
	c->width  = info.w;				// send resize event first
	c->height = info.h;	
	translateConfigEvent( (XEvent*)c);	// will clear window
    }

    if ( should_clip ) {
	paintRect = paintRect.intersect( rect() );
	if ( paintRect.isEmpty() )
	    return TRUE;
    }

    QPaintEvent e( paintRect );
    setWFlags( WState_PaintEvent );
    QApplication::sendEvent( this, &e );
    clearWFlags( WState_PaintEvent );
    return TRUE;
}

//
// Scroll-done event translation.
//

bool QETWidget::translateScrollDoneEvent( const XEvent *event )
{
    if ( !sip_list ) return FALSE;

    long id = event->xclient.data.l[0];

    // Remove any scroll-in-progress record for the given id.
    for (QScrollInProgress* sip = sip_list->first(); sip; sip=sip_list->next()) {
	if ( sip->id == id ) {
	    sip_list->remove( sip_list->current() );
	    return TRUE;
	}
    }

    return FALSE;
}


//
// ConfigureNotify (window move and resize) event translation
//
// The problem with ConfigureNotify is that one cannot trust x and y values
// in the xconfigure struct. Top level widgets are reparented by the window
// manager, and (x,y) is sometimes relative to the parent window, but not
// always!  It is safer (but slower) to translate the window coordinates.
//

bool QETWidget::translateConfigEvent( const XEvent *event )
{
    if ( parentWidget() && !testWFlags(WType_Modal) )
	return TRUE;				// child widget

    clearWFlags(WConfigPending);

    QSize  newSize( event->xconfigure.width, event->xconfigure.height );

    {
	XEvent otherEvent;
	while ( XCheckTypedEvent(dpy, ConfigureNotify, &otherEvent) ) {
	    if (otherEvent.type != ConfigureNotify
		|| otherEvent.xconfigure.window != event->xconfigure.window
		|| otherEvent.xconfigure.event != event->xconfigure.event) {
		XPutBackEvent( dpy, &otherEvent );
		break;
	    }
	    if (qApp->x11EventFilter(&otherEvent))
		break;
	    newSize.setWidth( otherEvent.xconfigure.width );
	    newSize.setHeight( otherEvent.xconfigure.height );
	}

    }
    Window child;
    int	   x, y;
    XTranslateCoordinates( dpy, winId(), DefaultRootWindow(dpy),
			   0, 0, &x, &y, &child );
    QPoint newPos( x, y );
    QRect  r = geometry();
    if ( newSize != size() ) {			// size changed
	QSize oldSize = size();
	r.setSize( newSize );
	setCRect( r );
	if ( isVisible() ) {
	    cancelResize();
	    QResizeEvent e( newSize, oldSize );
	    QApplication::sendEvent( this, &e );
	} else {
	    deferResize( oldSize );
	}
 	if ( !testWFlags(WResizeNoErase) ) {
 	    repaint( TRUE );
	}
    }
    if ( newPos != geometry().topLeft() ) {
	QPoint oldPos = pos();
	r.moveTopLeft( newPos );
	setCRect( r );
	if ( isVisible() ) {
	    cancelMove();
	    QMoveEvent e( newPos, oldPos );
	    QApplication::sendEvent( this, &e );
	} else {
	    deferMove( oldPos );
	}
    }
    return TRUE;
}


//
// Close window event translation.
//
// This class is a friend of QApplication because it needs to emit the
// lastWindowClosed() signal when the last top level widget is closed.
//

bool QETWidget::translateCloseEvent( const XEvent * )
{
    return close(FALSE);
}
