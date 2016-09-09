/****************************************************************************
** $Id: qxt.cpp,v 1.6 1998/07/03 00:09:27 hanord Exp $
**
** Implementation of Qt extension classes for Xt/Motif support.
**
** Created : 980107
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

#include <qapplication.h>
#include <qwidget.h>
#include <qobjcoll.h>
#include <qwidcoll.h>

#include "qxt.h"

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <limits.h>

#include <X11/Xlib.h>
#include <X11/Intrinsic.h>
#include <X11/IntrinsicP.h> // for XtCreateWindow
#include <X11/Shell.h>
#include <X11/StringDefs.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>

#define HAVE_MOTIF
#ifdef HAVE_MOTIF
#include <Xm/Xm.h>
#endif

typedef void (*SameAsXtTimerCallbackProc)(void*,void*);
typedef void (*IntervalSetter)(int);
typedef void (*ForeignEventProc)(XEvent*);

extern XtEventDispatchProc
 qt_np_cascade_event_handler[LASTEvent];      // defined in qnpsupport.cpp
void            qt_reset_color_avail();       // defined in qcolor_x11.cpp
void            qt_activate_timers();         // defined in qapplication_x11.cpp
timeval        *qt_wait_timer();              // defined in qapplication_x11.cpp
void		qt_x11SendPostedEvents();     // defined in qapplication_x11.cpp
Boolean  qt_event_handler( XEvent* event );   // defined in qnpsupport.cpp
extern int      qt_np_count;                  // defined in qnpsupport.cpp
void qt_np_timeout( void* p, void* id );      // defined in qnpsupport.cpp
void qt_np_add_timeoutcb(
	SameAsXtTimerCallbackProc cb );       // defined in qnpsupport.cpp
void qt_np_remove_timeoutcb(
	SameAsXtTimerCallbackProc cb );       // defined in qnpsupport.cpp
void qt_np_add_timer_setter(
	IntervalSetter is );                  // defined in qnpsupport.cpp
void qt_np_remove_timer_setter(
	IntervalSetter is );                  // defined in qnpsupport.cpp
extern XtIntervalId qt_np_timerid;            // defined in qnpsupport.cpp
extern void (*qt_np_leave_cb)
              (XLeaveWindowEvent*);           // defined in qnpsupport.cpp
void qt_np_add_event_proc(
	    ForeignEventProc fep );           // defined in qnpsupport.cpp
void qt_np_remove_event_proc(
	    ForeignEventProc fep );           // defined in qnpsupport.cpp




typedef struct {
    int empty;
} QWidgetClassPart;

typedef struct _QWidgetClassRec {
    CoreClassPart	core_class;
    QWidgetClassPart	qwidget_class;
} QWidgetClassRec;

//static QWidgetClassRec qwidgetClassRec;

typedef struct {
    /* resources */
    /* (none) */
    /* private state */
    QXtWidget* qxtwidget;
} QWidgetPart;

typedef struct _QWidgetRec {
    CorePart	core;
    QWidgetPart	qwidget;
} QWidgetRec;

class QFixableWidget : public QWidget {
public:
    void fix()
    {
	QRect g = geometry();
	QColor bg = backgroundColor();
	bool mt = hasMouseTracking();
	bool hascurs = testWFlags( WCursorSet );
	QCursor curs = cursor();
	clearWFlags( WState_Created );
	clearWFlags( WState_Visible );
	create( 0, FALSE, FALSE );
	setGeometry(g);
	setBackgroundColor( bg );
	setMouseTracking( mt );
	if ( hascurs ) {
	    setCursor( curs );
	}
    }
};

static
void createNewWindowsForAllChildren(QWidget* parent)
{
    QObjectList* list = parent->queryList("QWidget", 0, FALSE, FALSE);

    if ( list ) {
	QObjectListIt it( *list );
	QFixableWidget* c;
	while ( (c = (QFixableWidget*)it.current()) ) {
	    bool vis = c->isVisible();
	    c->fix();
	    createNewWindowsForAllChildren(c);
	    if ( vis ) c->show(); // Now that all children are valid.
	    ++it;
	}
	delete list;
    }
}

void qwidget_realize(
	Widget                widget,
	XtValueMask*          mask,
	XSetWindowAttributes* attributes
    )
{
    widgetClassRec.core_class.realize(widget, mask, attributes);
    QXtWidget* qxtw = ((QWidgetRec*)widget)->qwidget.qxtwidget;
    if (XtWindow(widget) != qxtw->winId()) {
	qxtw->create(XtWindow(widget), FALSE, FALSE);
	createNewWindowsForAllChildren(qxtw);
    }
    qxtw->show();
}

static
QWidgetClassRec qwidgetClassRec = {
  { /* core fields */
    /* superclass		*/	(WidgetClass) &widgetClassRec,
    /* class_name		*/	"QWidget",
    /* widget_size		*/	sizeof(QWidgetRec),
    /* class_initialize		*/	0,
    /* class_part_initialize	*/	0,
    /* class_inited		*/	FALSE,
    /* initialize		*/	0,
    /* initialize_hook		*/	0,
    /* realize			*/	qwidget_realize,
    /* actions			*/	0,
    /* num_actions		*/	0,
    /* resources		*/	0,
    /* num_resources		*/	0,
    /* xrm_class		*/	NULLQUARK,
    /* compress_motion		*/	TRUE,
    /* compress_exposure	*/	TRUE,
    /* compress_enterleave	*/	TRUE,
    /* visible_interest		*/	FALSE,
    /* destroy			*/	0,
    /* resize			*/	XtInheritResize,
    /* expose			*/	XtInheritExpose,
    /* set_values		*/	0,
    /* set_values_hook		*/	0,
    /* set_values_almost	*/	XtInheritSetValuesAlmost,
    /* get_values_hook		*/	0,
    /* accept_focus		*/	XtInheritAcceptFocus,
    /* version			*/	XtVersion,
    /* callback_private		*/	0,
    /* tm_table			*/	XtInheritTranslations,
    /* query_geometry		*/	XtInheritQueryGeometry,
    /* display_accelerator	*/	XtInheritDisplayAccelerator,
    /* extension		*/	0
  },
  { /* qwidget fields */
    /* empty			*/	0
  }
};
static WidgetClass qWidgetClass = (WidgetClass)&qwidgetClassRec;

static bool filters_installed = FALSE;
static QXtApplication* qxtapp = 0;
static XtAppContext appcon;

static
void installXtEventFilters()
{
    if (filters_installed) return;
    // Get Xt out of our face - install filter on every event type
    for (int et=2; et < LASTEvent; et++) {
	qt_np_cascade_event_handler[et] = XtSetEventDispatcher(
	    qt_xdisplay(), et, qt_event_handler );
    }
    filters_installed = TRUE;
}

static
void removeXtEventFilters()
{
    if (!filters_installed) return;
    // We aren't needed any more... slink back into the shadows.
    for (int et=2; et < LASTEvent; et++) {
	XtSetEventDispatcher(
	    qt_xdisplay(), et, qt_np_cascade_event_handler[et] );
    }
    filters_installed = FALSE;
}

// When we are in an event loop of QApplication rather than the browser's
// event loop (eg. for a modal dialog), we still send events to Xt.
static
void np_event_proc( XEvent* e )
{
    Widget xtw = XtWindowToWidget( e->xany.display, e->xany.window );
    if ( xtw ) {
	// Allow Xt to process the event
	//qt_np_cascade_event_handler[e->type]( e );
    }
}

static void np_set_timer( int interval )
{
    // Ensure we only have one timeout in progress - QApplication is
    // computing the one amount of time we need to wait.
    if ( qt_np_timerid ) {
	XtRemoveTimeOut( qt_np_timerid );
    }
    qt_np_timerid = XtAppAddTimeOut(appcon, interval,
	(XtTimerCallbackProc)qt_np_timeout, 0);
}

static void np_do_timers( void*, void* )
{
    qt_np_timerid = 0; // It's us, and we just expired, that's why we are here.

    qt_activate_timers();

    timeval *tm = qt_wait_timer();

    if (tm) {
	int interval = QMIN(tm->tv_sec,INT_MAX/1000)*1000 + tm->tv_usec/1000;
	np_set_timer( interval );
    }
}

/*!
  \class QXtApplication qxt.h
  \brief Allows mixing of Xt/Motif and Qt widgets.

  \extension Xt/Motif

  The QXtApplication and QXtWidget classes allow old Xt or Motif widgets
  to be used in new Qt applications.  They also allow Qt widgets to
  be used in primarily Xt/Motif applications.  The facility is intended
  to aid migration from Xt/Motif to the more comfortable Qt system.
*/

static bool my_xt;

/*!
  Constructs a QApplication and initializes the Xt toolkit.
  The \a appclass, \a options, \a num_options, and \a resources
  arguments are passed on to XtAppSetFallbackResources and
  XtDisplayInitialize.

  Use this constructor when writing a new Qt application which
  needs to use some existing Xt/Motif widgets.
*/
QXtApplication::QXtApplication(int& argc, char** argv,
	const char* appclass, XrmOptionDescRec *options,
	int num_options,
	const char** resources) :
    QApplication(argc, argv)
{
    my_xt = TRUE;
    XtToolkitInitialize();
    appcon = XtCreateApplicationContext();
    if (resources)
	XtAppSetFallbackResources(appcon, (char**)resources);
    XtDisplayInitialize(appcon, qt_xdisplay(), name(),
	appclass, options, num_options, &argc, argv);
    init();
}

/*!
  Constructs a QApplication from the \a display of an already-initialized
  Xt application.

  Use this constructor when introducing Qt widgets into an existing
  Xt/Motif application.
*/
QXtApplication::QXtApplication(Display *display) :
    QApplication(display)
{
    my_xt = FALSE;
    init();
    appcon = XtDisplayToApplicationContext(display);
}

/*!
  Destructs the application.  Does not close the Xt toolkit.
*/
QXtApplication::~QXtApplication()
{
    ASSERT(qxtapp==this);
    removeXtEventFilters();
    qxtapp = 0;
    if (my_xt) {
	XtDestroyApplicationContext(appcon);
    }
}

void QXtApplication::init()
{
    ASSERT(qxtapp==0);
    qxtapp = this;
    installXtEventFilters();
    qt_np_add_timeoutcb(np_do_timers);
    qt_np_add_timer_setter(np_set_timer);
    qt_np_add_event_proc(np_event_proc);
    qt_np_count++;
}

/*!
  Reimplemented to pass client messages to Xt.
*/
bool QXtApplication::x11EventFilter(XEvent* ev)
{
    if ( ev->type == ClientMessage ) {
	// #### needed?
	//qt_np_cascade_event_handler[ev->type](ev);
    }
    return QApplication::x11EventFilter(ev);
}


/*!
  \class QXtWidget qxt.h
  \brief Allows mixing of Xt/Motif and Qt widgets.

  \extension Xt/Motif

  QXtWidget acts as a bridge between Xt and Qt. For utilizing old
  Xt widgets, it can be a QWidget
  based on a Xt widget class. For including Qt widgets in an existing
  Xt/Motif application, it can be a special Xt widget class that is
  a QWidget.  See the constructors for the different behaviors.
*/

void QXtWidget::init(const char* name, WidgetClass widget_class,
		    Widget parent, ArgList args, Cardinal num_args,
		    bool managed)
{
    if (parent) {
	xtw = XtCreateWidget(name, widget_class, parent, args, num_args);
	if (managed)
	    XtManageChild(xtw);
    } else {
	ASSERT(!managed);
	String n, c;
	XtGetApplicationNameAndClass(qt_xdisplay(), &n, &c);
	xtw = XtAppCreateShell(n, c, widget_class, qt_xdisplay(),
	    args, num_args);
    }

    Arg reqargs[20];
    Cardinal nargs=0;
    XtSetArg(reqargs[nargs], XtNx, x());	nargs++;
    XtSetArg(reqargs[nargs], XtNy, y());	nargs++;
    XtSetArg(reqargs[nargs], XtNwidth, width());	nargs++;
    XtSetArg(reqargs[nargs], XtNheight, height());	nargs++;
    //XtSetArg(reqargs[nargs], "mappedWhenManaged", False);	nargs++;
    XtSetValues(xtw, reqargs, nargs);

    //#### destroy();   MLK

    if (!parent || XtIsRealized(parent))
	XtRealizeWidget(xtw);
}

/*!
  Constructs a QXtWidget of the special Xt widget class known as
  "QWidget" to the resource manager.

  Use this constructor to utilize Qt widgets in an Xt/Motif
  application.  The QXtWidget is a QWidget, so you can create
  subwidgets, layouts, etc. using Qt functionality.
*/
QXtWidget::QXtWidget(const char* name, Widget parent, bool managed) :
    QWidget(0, name)
{
    init(name, qWidgetClass, parent, 0, 0, managed);
    ((QWidgetRec*)xtw)->qwidget.qxtwidget = this;
    Arg reqargs[20];
    Cardinal nargs=0;
    XtSetArg(reqargs[nargs], XtNborderWidth, 0);            nargs++;
    XtSetValues(xtw, reqargs, nargs);
}

/*!
  Constructs a QXtWidget of the given \a widget_class.

  Use this constructor to utilize Xt or Motif widgets in a Qt
  application.  The QXtWidget looks and behaves
  like the Xt class, but can be used like any QWidget.

  Note that the parent must be a QXtWidget (possibly NULL).
  This is necessary since all Xt widgets must have Xt widgets
  as ancestors up to the top-level widget.
  <em>WWA: This restriction may be avoidable by reimplementing
  the low-level Qt window creation/destruction functions.</em>
*/
QXtWidget::QXtWidget(const char* name, WidgetClass widget_class,
		     QXtWidget *parent, ArgList args, Cardinal num_args,
		     bool managed) :
    QWidget(parent, name)
{
    init(name, widget_class, parent ? parent->xtw : 0, args, num_args, managed);
    create(XtWindow(xtw), FALSE, FALSE);
}

/*!
  Destructs the QXtWidget.
*/
QXtWidget::~QXtWidget()
{
    // Delete children first, as Xt will destroy their windows
    //
    QObjectList* list = queryList("QWidget", 0, FALSE, FALSE);
    if ( list ) {
	QWidget* c;
        QObjectListIt it( *list );
        while ( (c = (QWidget*)it.current()) ) {
            delete c;
            ++it;
        }
        delete list;
    }

    XtDestroyWidget(xtw);
    destroy( FALSE, FALSE );
}

/*!
  \fn Widget QXtWidget::xtWidget() const
  Returns the Xt widget equivalent for the Qt widget.
*/

/*!
  Reimplemented to pass the new geometry to Xt via XtSetValues().
*/
void QXtWidget::setGeometry( int x, int y, int w, int h )
{
    QWidget::setGeometry(x,y,w,h);

    Arg args[20];
    Cardinal nargs=0;
    XtSetArg(args[nargs], XtNx, x);       nargs++;
    XtSetArg(args[nargs], XtNy, y);       nargs++;
    XtSetArg(args[nargs], XtNwidth, w);   nargs++;
    XtSetArg(args[nargs], XtNheight, h);  nargs++;
    XtSetValues(xtw, args, nargs);
}

/*!
  Reimplemented to pass the new geometry to Xt via XtSetValues().
*/
void QXtWidget::setGeometry( const QRect & r )
{
    QWidget::setGeometry(r);
}

/*!
  Reimplemented to pass events to Xt.
*/
bool QXtWidget::x11Event( XEvent* ev )
{
    qt_np_cascade_event_handler[ev->type]( ev );
    return QWidget::x11Event(ev); // ### Should we always do it?
}

/*!
  Reimplemented to produce the Xt effect of losing focus when the
  mouse goes out of the widget. <em>This may be changed.</em>
*/
void QXtWidget::leaveEvent( QEvent* ev )
{
    // Xt-style:  focus-follows-mouse-out-of-widget
    QWidget * fw = qApp->focusWidget();
    if (fw) fw->clearFocus();

    QWidget::leaveEvent(ev);
}
