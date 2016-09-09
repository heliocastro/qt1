/****************************************************************************
** $Id: qtooltip.cpp,v 2.51.2.1 1998/08/31 15:18:11 hanord Exp $
**
** Tool Tips (or Balloon Help) for any widget or rectangle
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

#include "qtooltip.h"
#include "qlabel.h"
#include "qptrdict.h"
#include "qapplication.h"

// Magic value meaning an entire widget - if someone tries to insert a
// tool tip on this part of a widget it will be interpreted as the
// entire widget.

static inline QRect entireWidget()
{
    return QRect( QCOORD_MIN, QCOORD_MIN,
		  QCOORD_MAX-QCOORD_MIN, QCOORD_MAX-QCOORD_MIN );
}


// Internal class - don't touch

class QTipManager : public QObject
{
    Q_OBJECT	/* tmake ignore Q_OBJECT */
public:
    QTipManager();
   ~QTipManager();

    struct Tip
    {
	QRect		rect;
	QString		text;
	QString	        groupText;
	QToolTipGroup  *group;
	QToolTip       *tip;
	bool	        autoDelete;
	Tip	       *next;
    };

    bool    eventFilter( QObject * o, QEvent * e );
    void    add( QWidget *, const QRect &, const char *,
		 QToolTipGroup *, const char *, QToolTip *, bool );
    void    remove( QWidget *, const QRect & );
    void    remove( QWidget * );

    void    removeFromGroup( QToolTipGroup * );

private slots:
    void    labelDestroyed();
    void    clientWidgetDestroyed();
    void    showTip();
    void    hideTip();

protected:
    void    maybeTip( const QPoint & );

private:
    QTimer  wakeUp;
    QTimer  fallAsleep;
    QTimer  leaveWindow;

    QPtrDict<Tip>    *tips;
    QLabel	     *label;
    QPoint	      pos;
    QWidget	     *widget;
    Tip *currentTip;
    Tip *previousTip;
    bool isApplicationFilter;
};


// moc stuff - included by hand.

/****************************************************************************
** QTipManager meta object code from reading C++ file 'qtooltip.cpp'
**
** Created: Mon Aug 31 17:16:31 1998
**      by: The Qt Meta Object Compiler ($Revision: 2.51.2.1 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#if !defined(Q_MOC_OUTPUT_REVISION)
#define Q_MOC_OUTPUT_REVISION 2
#elif Q_MOC_OUTPUT_REVISION != 2
#error "Moc format conflict - please regenerate all moc files"
#endif

#include <qmetaobject.h>


const char *QTipManager::className() const
{
    return "QTipManager";
}

QMetaObject *QTipManager::metaObj = 0;

void QTipManager::initMetaObject()
{
    if ( metaObj )
	return;
    if ( strcmp(QObject::className(), "QObject") != 0 )
	badSuperclassWarning("QTipManager","QObject");
    if ( !QObject::metaObject() )
	QObject::initMetaObject();
    typedef void(QTipManager::*m1_t0)();
    typedef void(QTipManager::*m1_t1)();
    typedef void(QTipManager::*m1_t2)();
    typedef void(QTipManager::*m1_t3)();
    m1_t0 v1_0 = &QTipManager::labelDestroyed;
    m1_t1 v1_1 = &QTipManager::clientWidgetDestroyed;
    m1_t2 v1_2 = &QTipManager::showTip;
    m1_t3 v1_3 = &QTipManager::hideTip;
    QMetaData *slot_tbl = new QMetaData[4];
    slot_tbl[0].name = "labelDestroyed()";
    slot_tbl[1].name = "clientWidgetDestroyed()";
    slot_tbl[2].name = "showTip()";
    slot_tbl[3].name = "hideTip()";
    slot_tbl[0].ptr = *((QMember*)&v1_0);
    slot_tbl[1].ptr = *((QMember*)&v1_1);
    slot_tbl[2].ptr = *((QMember*)&v1_2);
    slot_tbl[3].ptr = *((QMember*)&v1_3);
    metaObj = new QMetaObject( "QTipManager", "QObject",
	slot_tbl, 4,
	0, 0 );
}

// moc code ends here


// We have a global, internal QTipManager object

static QTipManager *tipManager	  = 0;
static bool	    initializedTM = FALSE;

static void cleanupTipManager()
{
    delete tipManager;
    tipManager = 0;
    initializedTM = FALSE;
}

static void initTipManager()
{
    if ( !tipManager ) {
	tipManager = new QTipManager;
	CHECK_PTR( tipManager );
    }
    if ( !initializedTM ) {
	initializedTM = TRUE;
	qAddPostRoutine( cleanupTipManager );
    }
}


QTipManager::QTipManager()
    : QObject( 0, "toolTipManager" )
{
    tips = new QPtrDict<QTipManager::Tip>( 313 );
    currentTip = 0;
    previousTip = 0;
    label = 0;
    isApplicationFilter = FALSE;
    connect( &wakeUp, SIGNAL(timeout()), SLOT(showTip()) );
    connect( &fallAsleep, SIGNAL(timeout()), SLOT(hideTip()) );
    connect( &leaveWindow, SIGNAL(timeout()), SLOT(hideTip()) );
}


QTipManager::~QTipManager()
{
    if ( isApplicationFilter && !qApp->closingDown() ) {
	qApp->setGlobalMouseTracking( FALSE );
	qApp->removeEventFilter( tipManager );
    }

    if ( tips ) {
	QPtrDictIterator<QTipManager::Tip> i( *tips );
	QTipManager::Tip *t, *n;
	void *k;
	while( (t = i.current()) != 0 ) {
	    k = i.currentKey();
	    ++i;
	    tips->take( k );
	    while ( t ) {
		n = t->next;
		delete t;
		t = n;
	    }
	}
	delete tips;
    }

    delete label;
}

void QTipManager::add( QWidget *w, const QRect &r, const char *s,
		       QToolTipGroup *g, const char *gs,
		       QToolTip *tt, bool a )
{
    QTipManager::Tip *h = (*tips)[ w ];
    QTipManager::Tip *t = new QTipManager::Tip;
    if ( h )
	tips->take( w );
    t->next = h;

    t->tip = tt;
    t->autoDelete = a;
    t->text = s;
    t->rect = r;
    t->groupText = gs;
    t->group = g;

    tips->insert( w, t );
    connect( w, SIGNAL(destroyed()), this, SLOT(clientWidgetDestroyed()) );
    if ( a ) {
	if ( t->rect.contains( pos ) )
	    showTip();
	tips->take( w );
	if( t->next )
	    tips->insert( w, t->next );
	t->next = 0;
    }
    if ( !isApplicationFilter && qApp ) {
	isApplicationFilter = TRUE;
	qApp->installEventFilter( tipManager );
	qApp->setGlobalMouseTracking( TRUE );
    }
}


void QTipManager::remove( QWidget *w, const QRect & r )
{
    QTipManager::Tip *t = (*tips)[ w ];
    if ( t == 0 )
	return;

    if ( t == currentTip )
	hideTip();

    if ( t->rect == r ) {
	tips->take( w );
	if ( t->next ) {
	    tips->insert( w, t->next );
	} else {
	    // ### w->setMouseTracking( FALSE );
	    // ### need to disable sometimes
	    // w->removeEventFilter( tipManager );
	}
	delete t;
    } else {
	while( t->next && t->next->rect != r )
	    t = t->next;
	if ( t->next ) {
	    QTipManager::Tip *d = t->next;
	    t->next = t->next->next;
	    delete d;
	}
    }

    if ( tips->isEmpty() ) {
	// the manager will be recreated if needed
	delete tipManager;
	tipManager = 0;
    }
}


/*!
  The label was destroyed in the program cleanup phase.
*/

void QTipManager::labelDestroyed()
{
    label = 0;
}


/*!
  Remove sender() from the tool tip data structures.
*/

void QTipManager::clientWidgetDestroyed()
{
    const QObject *s = sender();
    if ( s )
	remove( (QWidget*) s );
}


void QTipManager::remove( QWidget *w )
{
    QTipManager::Tip *t = (*tips)[ w ];
    if ( t == 0 )
	return;

    tips->take( w );
    QTipManager::Tip * d;
    while ( t ) {
	if ( t == currentTip )
	    hideTip();
	d = t->next;
	delete t;
	t = d;
    }

    if ( tips->isEmpty() ) {
	delete tipManager;
	tipManager = 0;
    }
}


void QTipManager::removeFromGroup( QToolTipGroup *g )
{
    QPtrDictIterator<QTipManager::Tip> i( *tips );
    QTipManager::Tip *t;
    while( (t = i.current()) != 0 ) {
	++i;
	while ( t ) {
	    if ( t->group == g )
		t->group = 0;
	    t = t->next;
	}
    }
}



bool QTipManager::eventFilter( QObject *obj, QEvent *e )
{
    // avoid dumping core in case of application madness, and return
    // quickly for some common but irrelevant events
    if ( !tips || !e || !obj || !obj->isWidgetType() ||
	 e->type() == Event_Timer || e->type() == Event_Paint ||
	 e->type() == Event_SockAct )
	return FALSE;
    QWidget *w = (QWidget *)obj;

    QTipManager::Tip *t = 0;
    while( w && !t ) {
	t = (*tips)[ w ];
	if ( !t )
	    w = w->testWFlags(WType_TopLevel) ? 0 : w->parentWidget();
    }

    if ( !t ) {
	if ( e->type() >= Event_MouseButtonPress &&
	     e->type() <= Event_Leave )
	    wakeUp.stop();
	return FALSE;
    }

    // with that out of the way, let's get down to action

    switch( e->type() ) {
    case Event_MouseButtonPress:
    case Event_MouseButtonRelease:
    case Event_MouseButtonDblClick:
    case Event_KeyPress:
    case Event_KeyRelease:
	// input - turn off tool tip mode
	hideTip();
	fallAsleep.stop();
	leaveWindow.stop();
	break;
    case Event_MouseMove:
	{ // a whole scope just for one variable
	    QMouseEvent * m = (QMouseEvent *)e;

	    QPoint mousePos( m->pos() );
	    mousePos = ((QWidget*)obj)->mapToGlobal( mousePos );
	    mousePos = w->mapFromGlobal( mousePos );

	    if ( currentTip && !currentTip->rect.contains( mousePos ) ) {
		hideTip();
		if ( m->state() == 0 )
		    return FALSE;
	    }

	    wakeUp.stop();
	    if ( m->state() == 0 ) {
		if ( (label && label->isVisible()) )
		    return TRUE;
		else if ( fallAsleep.isActive() )
		    wakeUp.start( 100, TRUE );
		else {
		    previousTip = 0;
		    wakeUp.start( 700, TRUE );
		}
		widget = w;
		pos = mousePos;
		return FALSE;
	    } else {
		hideTip();
	    }
	}
	break;
    case Event_Enter:
	if ( label && label->isVisible() && w == widget )
	    leaveWindow.stop();
	else if ( w ) // test in event_leave below should always hit first
	    wakeUp.stop();
	break;
    case Event_Leave:
	if ( label && label->isVisible() )
	    leaveWindow.start( 50, TRUE );
	else if ( widget != w )
	    wakeUp.stop();
	break;
    default:
	hideTip();
	break;
    }
    return FALSE;
}



void QTipManager::showTip()
{
    if ( widget == 0 ) {
	widget = 0;
	return;
    }

    QTipManager::Tip *t = (*tips)[ widget ];
    while ( t && !t->rect.contains( pos ) )
	t = t->next;
    if ( t == 0 || t == previousTip )
	return;

    if ( t->tip ) {
	t->tip->maybeTip( pos );
	return;
    }

    if ( label ) {
	label->setText( t->text );
    } else {
	label = new QLabel( 0, "toolTipTip",
			    WStyle_Customize | WStyle_NoBorder | WStyle_Tool );
	CHECK_PTR( label );
	connect( label, SIGNAL(destroyed()), SLOT(labelDestroyed()) );
	label->setFont( QToolTip::font() );
	label->setPalette( QToolTip::palette() );
	label->setText( t->text );
	if ( QApplication::style() == MotifStyle )
	    label->setFrameStyle( QFrame::Plain | QFrame::Box );
	else
	    label->setFrameStyle( QFrame::Raised | QFrame::Panel );
	label->setLineWidth( 1 );
	label->setMargin( 3 );
	label->setAlignment( AlignLeft | AlignTop );
	label->setAutoResize( TRUE );
    }
    QPoint p = widget->mapToGlobal( pos ) + QPoint( 2, 16 );
    if ( p.x() + label->width() > QApplication::desktop()->width() )
	p.setX( QApplication::desktop()->width() - label->width() );
    if ( p.y() + label->height() > QApplication::desktop()->height() )
	p.setY( p.y() - 20 - label->height() );
    label->move( p );
    label->show();
    label->raise();

    fallAsleep.start( 5000, TRUE );
    leaveWindow.stop();

    if ( t->group && !t->groupText.isEmpty() )
	emit t->group->showTip( t->groupText );
    currentTip = t;
    previousTip = 0;
}


void QTipManager::hideTip()
{
    if ( label && label->isVisible() ) {
	label->hide();
	fallAsleep.start( 10000, TRUE );
	if ( currentTip && currentTip->group )
	    emit currentTip->group->removeTip();
    } else if ( wakeUp.isActive() ) {
	wakeUp.stop();
    }

    if ( currentTip && currentTip->autoDelete )
	delete currentTip;
    else
	previousTip = currentTip;

    currentTip = 0;
    widget = 0;
}


/*!
  \class QToolTip qtooltip.h

  \brief The QToolTip class provides tool tips (sometimes called
  balloon help) for any widget or rectangular part of a widget.

  \ingroup application

  The tip is a short, one-line text reminding the user of the widget's
  or rectangle's function.  It is drawn immediately below the region,
  in a distinctive black on yellow combination.  In Motif style, Qt's
  tool tips look much like Motif's but feel more like Windows 95 tool
  tips.

  QToolTipGroup provides a way for tool tips to display another text
  elsewhere (most often in a status bar).

  At any point in time, QToolTip is either dormant or active.  In
  dormant mode the tips are not shown, and in active mode they are.
  The mode is global, not particular to any one widget.

  QToolTip switches from dormant to active mode when the user lets the
  mouse rest on a tip-equipped region for a second or so, and remains
  in active mode until the user either clicks a mouse button, presses
  a key, lets the mouse rest for five seconds, or moves the mouse
  outside \e all tip-equpped regions for at least a second.

  The QToolTip class can be used in three different ways: <ol> <li>
  Adding a tip to an entire widget. <li> Adding a tip to a fixed
  rectangle within a widget. <li> Adding a tip to a dynamic rectangle
  within a widget. </ol>

  To add a tip to a widget, call the static function QToolTip::add()
  with the widget and tip as arguments:

  \code
    QToolTip::add( quitButton, "Leave the application" );
  \endcode

  This is the simplest and most common use of QToolTip.  The tip will
  be deleted automatically when \e quitButton is deleted, but you can
  remove it yourself, too:

  \code
    QToolTip::remove( quitButton );
  \endcode

  You can also display another text (typically in a \link QStatusBar
  status bar),\endlink courtesy of QToolTipGroup.  This example
  assumes that \e g is a <code>QToolTipGroup *</code> and already
  connected to the appropriate status bar:

  \code
    QToolTip::add( quitButton, "Leave the application", g,
                   "Leave the application, without asking for confirmation" );
    QToolTip::add( closeButton, "Close this window", g,
                   "Close this window, without asking for confirmation" );
  \endcode

  To add a tip to a fixed rectangle within a widget, call the static
  function QToolTip::add() with the widget, rectangle and tip as
  arguments.  (See the tooltip/tooltip.cpp example.)  Again, you can supply a
  QToolTipGroup * and another text if you want.

  Both of the above are one-liners and cover the vast majority of
  cases.  The third and most general way to use QToolTip uses a pure
  virtual function to decide whether to pop up a tool tip.  The
  tooltip/tooltip.cpp example demonstrates this too.  This mode can be
  used to implement e.g. tips for text that can move as the user
  scrolls.

  To use QToolTip like this, you need to subclass QToolTip and
  reimplement maybeTip().  maybeTip() will be called when there's a
  chance that a tip should pop up.  It must decide whether to show a
  tip, and possibly call add() with the rectangle the tip applies to,
  the tip's text and optionally the QToolTipGroup details.  The tip
  will disappear once the mouse moves outside the rectangle you
  supply, and \e not \e reappear - maybeTip() will be called again if
  the user lets the mouse rest within the same rectangle again.  You
  can forcibly remove the tip by calling remove() with no arguments.
  This is handy if the widget scrolls.

  \sa QStatusBar QWhatsThis QToolTipGroup
  <a href="guibooks.html#fowler">GUI Design Handbook: Tool Tip</a>
*/


/*
  Global settings for tool tips.
*/

QFont	    *QToolTip::ttFont    = 0;
QPalette    *QToolTip::ttPalette = 0;


void QToolTip::initialize()
{
    if ( ttFont )				// already initialized
	return;
    qAddPostRoutine( cleanup );
    ttFont = new QFont;
    CHECK_PTR( ttFont );
    QColorGroup cg( black, QColor(255,255,220),
		    QColor(96,96,96), black, black,
		    black, QColor(255,255,220) );
    ttPalette = new QPalette( cg, cg, cg );
    CHECK_PTR( ttPalette );
}


void QToolTip::cleanup()
{
    delete ttFont;
    ttFont = 0;
    delete ttPalette;
    ttPalette = 0;
}


/*!
  Returns the font common to all tool tips.
  \sa setFont()
*/

QFont QToolTip::font()
{
    if ( !ttFont )
	initialize();
    return *ttFont;
}


/*!
  Sets the font for all tool tips to \a font.
  \sa font()
*/

void QToolTip::setFont( const QFont &font )
{
    if ( !ttFont )
	initialize();
    *ttFont = font;
}


/*!
  Returns the palette common to all tool tips.
  \sa setPalette()
*/

QPalette QToolTip::palette()
{
    if ( !ttPalette )
	initialize();
    return *ttPalette;
}


/*!
  Sets the palette for all tool tips to \a palette.
  \sa palette()
*/

void QToolTip::setPalette( const QPalette &palette )
{
    if ( !ttPalette )
	initialize();
    *ttPalette = palette;
}


/*!
  Creates a tool tip object.  This is necessary only if you need tool
  tips on regions that can move within the widget (most often because
  the widget's contents can scroll).

  \a parent is widget you want to add dynamic tool tips to and \a
  group (optional) is the tool tip group they should belong to.

  \sa maybeTip().
*/

QToolTip::QToolTip( QWidget * parent, QToolTipGroup * group )
{
    p = parent;
    g = group;
    initTipManager();
    tipManager->add( p, entireWidget(), 0, g, 0, this, FALSE );
}


/*!
  Adds a tool tip to \e widget.  \e text is the text to be shown in
  the tool tip.  QToolTip makes a deep copy of this string.

  This is the most common entry point to the QToolTip class; it is
  suitable for adding tool tips to buttons, check boxes, combo boxes
  and so on.
*/

void QToolTip::add( QWidget *widget, const char *text )
{
    initTipManager();
    tipManager->add( widget, entireWidget(), text, 0, 0, 0, FALSE );
}


/*!
  Adds a tool tip to \a widget, and to tool tip group \a group.

  \e text is the text shown in the tool tip and \a longText is the
  text emitted from \a group.  QToolTip makes deep copies of both
  strings.

  Normally, \a longText is shown in a status bar or similar.
*/

void QToolTip::add( QWidget *widget, const char *text,
		    QToolTipGroup *group, const char *longText )
{
    initTipManager();
    tipManager->add( widget, entireWidget(), text, group, longText, 0, FALSE );
}


/*!
  Remove the tool tip from \e widget.

  If there are more than one tool tip on \a widget, only the one
  covering the entire widget is removed.
*/

void QToolTip::remove( QWidget * widget )
{
    if ( tipManager )
	tipManager->remove( widget, entireWidget() );
}

/*!
  Adds a tool tip to a fixed rectangle within \a widget.  \a text is
  the text shown in the tool tip.  QToolTip makes a deep copy of this
  string.
*/

void QToolTip::add( QWidget * widget, const QRect & rect, const char * text )
{
    initTipManager();
    tipManager->add( widget, rect, text, 0, 0, 0, FALSE );
}


/*!
  Adds a tool tip to an entire \a widget, and to tool tip group \a
  group.

  \e text is the text shown in the tool tip and \a longText is the
  text emitted from \a group.  QToolTip makes deep copies of both
  strings.

  Normally, \a longText is shown in a status bar or similar.
*/

void QToolTip::add( QWidget *widget, const QRect &rect,
		    const char *text,
		    QToolTipGroup *group, const char *groupText )
{
    initTipManager();
    tipManager->add( widget, rect, text, group, groupText, 0, FALSE );
}


/*!
  Remove the tool tip for \e rect from \e widget.

  If there are more than one tool tip on \a widget, only the one
  covering rectangle \e rect is removed.
*/

void QToolTip::remove( QWidget * widget, const QRect & rect )
{
    if ( tipManager )
	tipManager->remove( widget, rect );
}


/*!
  \fn virtual void QToolTip::maybeTip( const QPoint & p);

  This pure virtual function is half of the most versatile interface
  QToolTip offers.

  It is called when there is a chance that a tool tip should be shown,
  and must decide whether there is a tool tip for the point \a p in
  the widget this QToolTip object relates to.

  \p is given in that widget's local coordinates.  Most maybeTip()
  implementation will be of the form:
  
  \code
    if ( \<something\> ) {
	tip( \<something\>, \<something\> );
    }
  \endcode

  The first argument to tip() (a rectangle) should include the \a p,
  or QToolTip, the user or both can be confused.

  \sa tip()
*/


/*!
  Pop up a tip saying \a text right now, and remove that tip once the
  cursor moves out of rectangle \a rect (which is given in the
  coordinate system of the widget this QToolTip relates to).

  The tip will not come back if the cursor moves back; your maybeTip()
  has to reinstate it each time.
*/

void QToolTip::tip( const QRect & rect, const char * text )
{
    initTipManager();
    tipManager->add( parentWidget(), rect, text, 0, 0, 0, TRUE );
}


/*!
  Pop up a tip saying \a text right now, and remove that tip once
  the cursor moves out of rectangle \a rect.

  The tip will not come back if the cursor moves back; your maybeTip()
  has to reinstate it each time.
*/

void QToolTip::tip( const QRect & rect, const char * text,
		    const char * groupText )
{
    initTipManager();
    tipManager->add( parentWidget(), rect, text, group(), groupText, 0, TRUE );
}


/*!
  Remove all tool tips for this widget immediately.
*/

void QToolTip::clear()
{
    if ( tipManager )
	tipManager->remove( parentWidget() );
}


/*!
  \fn QWidget * QToolTip::parentWidget() const

  Returns the widget this QToolTip applies to.

  The tool tip is destroyed automatically when the parent widget is
  destroyed.

  \sa group()
*/


/*!
  \fn QToolTipGroup * QToolTip::group() const

  Returns the tool tip group this QToolTip is a member of, of 0 if it
  isn't a member of any group.

  The tool tip group is the object responsible for relaying contact
  betweeen tool tips and a status bar or something else which can show
  a longer help text.

  \sa parentWidget(), QToolTipGroup
*/


/*!
  \class QToolTipGroup qtooltip.h

  \brief The QToolTipGroup class collects tool tips into natural groups.

  Tool tips can display \e two texts, the one in the tip and
  optionally another one, typically in a status bar.  QToolTipGroup
  provides a way to link tool tips to this status bar.

  QToolTipGroup has practically no API, it is only used as an argument
  to QToolTip's member functions, for example like this:

  \code
    QToolTipGroup * g = new QToolTipGroup( this, "tool tip relay" );
    connect( g, SIGNAL(showTip(const char *)),
             myLabel, SLOT(setText(const char *)) );
    connect( g, SIGNAL(removeTip()),
             myLabel, SLOT(clear()) );
    QToolTip::add( giraffeButton, "feed giraffe",
                   g, "Give the giraffe a meal" );
    QToolTip::add( gorillaButton, "feed gorilla",
                   g, "Give the gorilla a meal" );
  \endcode

  This example makes the object myLabel (which you have to supply)
  display (one assumes, though you can make myLabel do anything, of
  course) the strings "Give the giraffe a meal" and "Give the gorilla
  a meal" while the relevant tool tips are being displayed.

  Deleting a tool tip group removes the tool tips in it.
*/

/*! \fn void QToolTipGroup::showTip (const char* longText)

  This signal is emitted when one of the tool tips in the group is
  displayed.  \a longText is the supplementary text for the displayed
  tool tip.

  \sa removeTip()
*/

/*! \fn void QToolTipGroup::removeTip ()

  This signal is emitted when a tool tip in this group is hidden.  See
  the QToolTipGroup documentation for an example of use.

  \sa showTip()
*/


/*!
  Constructs a tool tip group.
*/

QToolTipGroup::QToolTipGroup( QObject *parent, const char *name )
    : QObject( parent, name )
{
    // nothing
}


/*!
  Destroy this tool tip groups and all tool tips in it.
*/

QToolTipGroup::~QToolTipGroup()
{
    if ( tipManager )
	tipManager->removeFromGroup( this );
}
