/****************************************************************************
** $Id: qbutton.cpp,v 2.64.2.2 1998/11/01 20:37:01 agulbra Exp $
**
** Implementation of QButton widget class
**
** Created : 940206
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

#include "qbutton.h"
#include "qbuttongroup.h"
#include "qbitmap.h"
#include "qpainter.h"
#include "qkeycode.h"
#include "qtimer.h"
#include "qaccel.h"
#include "qpixmapcache.h"
#include <ctype.h>

static const int autoRepeatDelay  = 300;
static const int autoRepeatPeriod = 100;

static const int drawingPixWidth  = 300;
static const int drawingPixHeight = 100;


/*
  Returns a pixmap of dimension (drawingPixWidth x drawingPixHeight). The
  pixmap is used by paintEvent for flicker-free drawing.
 */

static QPixmap *drawpm = 0;
static void cleanupButtonPm()
{
    delete drawpm;
}
static QPixmap *getDrawingPixmap()
{
    if ( !drawpm ) {
	qAddPostRoutine( cleanupButtonPm );
	drawpm = new QPixmap( drawingPixWidth, drawingPixHeight );
	CHECK_PTR( drawpm );
    }
    return drawpm;
}


struct QButtonData
{
    QButtonData() : group(0), a(0) {}
    QButtonGroup *group;
    QTimer timer;
    QAccel *a;
};


void QButton::ensureData()
{
    if ( !d ) {
	d = new QButtonData;
	CHECK_PTR( d );
	connect(&d->timer, SIGNAL(timeout()), this, SLOT(autoRepeatTimeout()));
    }
}


QButtonGroup *QButton::group() const
{
    return d ? d->group : 0;
}


void QButton::setGroup( QButtonGroup* g )
{
    ensureData();
    d->group = g;
}


QTimer *QButton::timer()
{
    ensureData();
    return &d->timer;
}


/*
  Internal function that returns the shortcut character in a string.
  Returns zero if no shortcut character was found.
  Example:
    shortcutChar("E&xit") returns 'x'.
*/

static int shortcutChar( const char *str )
{
    const char *p = str ? strchr(str, '&') : 0;
    while ( p && *p && p[1] == '&' )
	p = strchr( p+2, '&' );
    return (p && *p && p[1] && p[1] != '&') ? p[1] : 0;
}


/*!
  \class QButton qbutton.h

  \brief The QButton class is the abstract base class of button
  widgets, providing functionality common to buttons.

  \ingroup abstractwidgets

  The QButton class implements an abstract button, and lets subclasses
  specify how to reply to user actions and how to draw the button.

  QButton provides both push and toggle buttons.  The QRadioButton and
  QCheckBox classes provide only toggle buttons, QPushButton and
  QToolButton provide both toggle and push buttons.

  Any button can have either a text or pixmap label.  setText() sets
  the button to be a text button and setPixmap() sets it to be a
  pixmap button.  The text/pixmap is manipulated as necessary to
  create "disabled" appearance when the button is \link
  QWidget::setEnabled() disabled\endlink.

  QButton provides most of the states used for buttons:
  <ul>
  <li>isDown() determines whether the button is \e pressed down.
  <li>isOn() determines whether the button is \e on.
  Only toggle buttons can be switched on and off  (see below).
  <li>isEnabled() determines whether the button can be pressed by the
  user.
  <li>setAutoRepeat() determines whether the button will auto-repeat
  if the user holds it down.
  <li>setToggleButton() determines whether the button is a toggle
  button or not.
  </ul>

  The difference between isDown() and isOn() is as follows:
  When the user clicks a toggle button to toggle it on, the button is
  first \e pressed, then released into \e on state.  When the user
  clicks it again (to toggle it off) the button moves first to the \e
  pressed state, then to the \e off state (isOn() and isDown() are
  both FALSE).

  Default buttons (as used in many dialogs) are provided by
  QPushButton::setDefault() and QPushButton::setAutoDefault().

  QButton provides four signals:
  <ul>
  <li>pressed() is emitted when the left mouse button is pressed while
  the mouse cursor is inside the button.
  <li>released() is emitted when the left mouse button is released.
  <li>clicked() is emitted when the button is first pressed then
  released, or when the accelerator key is typed, or when animateClick()
  is called.
  <li>toggled() is emitted when the state of a toggle button changes.
  </ul>

  If the button is a text button with "&" in its text, QButton creates an
  automatic accelerator key.  This code creates a push button labelled "Rock
  & Roll" (where the c is underscored).  The button gets an automatic
  accelerator key, Alt-C:

  \code
    QPushButton *p = new QPushButton( "Ro&ck && Roll", this );
  \endcode

  In this example, when the user presses Alt-C the button will
  \link animateClick() animate a click\endlink.

  You can also set a custom accelerator using the setAccel() function.
  This is useful mostly for pixmap buttons since they have no
  automatic accelerator.

  \code
    QPushButton *p;
    p->setPixmap( QPixmap("print.gif") );
    p->setAccel( ALT+Key_F7 );
  \endcode

  All of the buttons provided by Qt (\l QPushButton, \l QToolButton,
  \l QCheckBox and \l QRadioButton) can display both text and pixmaps.

  To subclass QButton, you have to reimplement at least drawButton()
  (to draw the button's outskirts) and drawButtonLabel() (to draw its
  text or pixmap).  It is generally advisable to reimplement
  sizeHint() as well, and sometimes hitButton() (to determine whether
  a button press is within the button).

  To reduce flickering the QButton::paintEvent() sets up a pixmap that the
  drawButton() function draws in. You should not reimplement paintEvent()
  for a subclass of QButton unless you want to take over all drawing.

  \sa QButtonGroup
  <a href="http://www.microsoft.com/win32dev/uiguide/uigui161.htm">Microsoft Style Guide</a>
*/


/*!
  Constructs a standard button with a parent widget and a name.

  If \a parent is a QButtonGroup, this constructor calls
  QButtonGroup::insert().
*/

QButton::QButton( QWidget *parent, const char *name )
    : QWidget( parent, name )
{
    bpixmap    = 0;
    toggleBt   = FALSE;				// button is not on/off
    buttonDown = FALSE;				// button is up
    buttonOn   = FALSE;				// button is off
    mlbDown    = FALSE;				// mouse left button up
    autoresize = FALSE;				// not auto resizing
    animation  = FALSE;				// no pending animateClick
    repeat     = FALSE;				// not in autorepeat mode
    d	       = 0;
    if ( parent && parent->inherits("QButtonGroup") ) {
	setGroup((QButtonGroup*)parent);
	group()->insert( this );		// insert into button group
    }
    setFocusPolicy( TabFocus );
}

/*!
  Destroys the button and all its child widgets.
*/

QButton::~QButton()
{
    if ( group() )				// remove from button group
	group()->remove( this );
    delete bpixmap;
    delete d;
}


/*!
  \fn void QButton::pressed()
  This signal is emitted when the button is pressed down.

  \sa released(), clicked()
*/

/*!
  \fn void QButton::released()
  This signal is emitted when the button is released.

  \sa pressed(), clicked(), toggled()
*/

/*!
  \fn void QButton::clicked()
  This signal is emitted when the button is activated (i.e. first
  pressed down and then released when the mouse cursor is inside the
  button).

  \sa pressed(), released(), toggled()
*/

/*!
  \fn void QButton::toggled( bool on )
  This signal is emitted whenever a toggle button changes status.
  \e on is TRUE if the button is on, or FALSE if the button is off.

  This may be the result of a user action, toggle() slot activation,
  or because setOn() was called.

  \sa clicked()
*/


/*!
  \fn const char *QButton::text() const
  Returns the button text, or 0 if the button has no text.
  \sa setText()
*/

/*!
  Sets the button to display \e text and repaints

  The button resizes itself if auto-resizing is enabled, changes its
  minimum size if autoMinimumSize() is enabled, and sets the
  appropriate accelerator.

  \sa text(), setPixmap(), setAutoMinimumSize(), setAutoResize(),
  setAccel(), QPixmap::mask()
*/

void QButton::setText( const char *text )
{
    if ( btext == text )
	return;
    int oldAccelChar = shortcutChar(btext);
    int newAccelChar = shortcutChar(text);
    btext = text;
    if ( bpixmap ) {
	delete bpixmap;
	bpixmap = 0;
    }
    if ( autoresize )
	adjustSize();
    if ( oldAccelChar && !newAccelChar && !accel() )
	setAccel( 0 );
    if ( newAccelChar )
	setAccel( ALT+toupper(newAccelChar) );
    repaint( FALSE );
}


/*!
  \fn const QPixmap *QButton::pixmap() const
  Returns the button pixmap, or 0 if the button has no pixmap.
*/

/*!
  Sets the button to display \a pixmap and repaints at once.

  If \a pixmap is monochrome (i.e. it is a QBitmap or its \link
  QPixmap::depth() depth\endlink is 1) and it does not have a mask,
  this function sets the pixmap to be its own mask. The purpose of
  this is to draw transparent bitmaps, which is important for
  e.g. toggle buttons.

  The button resizes itself if auto-resizing is enabled, changes its
  minimum size if autoMinimumSize() is enabled, and always disables
  any accelerator.

  \sa pixmap(), setText(), setAutoMinimumSize(), setAutoResize(),
  setAccel(), QPixmap::mask()
*/

void QButton::setPixmap( const QPixmap &pixmap )
{
    bool newSize;
    if ( bpixmap ) {
	newSize = pixmap.width() != bpixmap->width() ||
		  pixmap.height() != bpixmap->height();
        *bpixmap = pixmap;
    } else {
	newSize = TRUE;
	bpixmap = new QPixmap( pixmap );
	CHECK_PTR( bpixmap );
    }
    if ( bpixmap->depth() == 1 && !bpixmap->mask() )
	bpixmap->setMask( *((QBitmap *)bpixmap) );
    int oldAccelChar = shortcutChar(btext);
    if ( !btext.isNull() )
	btext.resize( 0 );
    if ( autoresize && newSize )
	adjustSize();
    if ( oldAccelChar )
	setAccel( 0 );
    repaint( FALSE );
}


/*!
  Returns the accelerator key currently set for the button, or 0
  if no accelerator key has been set.
  \sa setAccel()
*/

int QButton::accel() const
{
    return d && d->a ? d->a->key(0) : 0;
}

/*!
  Specifies an accelerator \a key for the button, or removes the
  accelerator if \a key is 0.

  Setting a button text containing a shortcut character (for
  example the 'x' in E&xit) automatically defines an ALT+letter
  accelerator for the button.
  You only need to call this function in order to specify a custom
  accelerator.

  Example:
  \code
    QPushButton *b1 = new QPushButton;
    b1->setText( "&OK" );		// sets accel ALT+'O'

    QPushButton *b2 = new QPushButton;
    b2->setPixmap( printIcon );		// pixmap instead of text
    b2->setAccel( CTRL+'P' );		// custom accel
  \endcode

  \sa accel(), setText(), QAccel
*/

void QButton::setAccel( int key )
{
    if ( key == -1 ) {				// guess from the text
	int c = shortcutChar( text() );
	key = c ? ALT+toupper(c) : 0;
    }

    ensureData();
    if ( key == 0 ) {				// delete accel
	delete d->a;
	d->a = 0;
    } else {
	if ( d->a )
	    d->a->clear();
	else
	    d->a = new QAccel( this, "buttonAccel" );
	d->a->connectItem( d->a->insertItem(key,0),
			   this, SLOT(animateClick()) );
    }
}


/*!
  \fn bool QButton::autoResize() const
  Returns TRUE if auto-resizing is enabled, or FALSE if auto-resizing is
  disabled.

  Auto-resizing is disabled by default.

  \sa setAutoResize()
*/


/*!
  Enables auto-resizing if \e enable is TRUE, or disables it if \e enable is
  FALSE.

  When auto-resizing is enabled, the button will resize itself whenever
  the contents change.

  \sa autoResize(), adjustSize()
*/

void QButton::setAutoResize( bool enable )
{
    if ( (bool)autoresize != enable ) {
	autoresize = enable;
	if ( autoresize )
	    adjustSize();			// calls resize which repaints
    }
}


/*!
  \fn bool QButton::autoRepeat() const

  Returns TRUE if the button is auto-repeating, else FALSE.

  The default is FALSE.

  \sa setAutoRepeat()
*/


/*!
  Turns on auto-repeat for the button if \a enable is TRUE, or
  turns it off if \a enable is FALSE.

  When auto-repeat is enabled, the clicked() signal is emitted at
  regular intervals while the buttons \link isDown() is down. \endlink

  setAutoRepeat() has no effect for \link setToggleButton() toggle
  buttons. \endlink

  \sa isDown(), autoRepeat(), clicked()
*/

void QButton::setAutoRepeat( bool enable )
{
    repeat = (uint)enable;
    if ( repeat && mlbDown )
	timer()->start( autoRepeatDelay, TRUE );
}


/*!
  Performs an animated click: The button is pressed and a short while
  later released.

  pressed(), released(), clicked() and toggled() signals are emitted as
  appropriate.

  This function does nothing if the button is \link setEnabled()
  disabled. \endlink

  \sa setAccel()
*/

void QButton::animateClick()
{
    if ( !isEnabled() || animation )
	return;
    animation  = TRUE;
    buttonDown = TRUE;
    repaint( FALSE );
    emit pressed();
    QTimer::singleShot( 100, this, SLOT(animateTimeout()) );
}


/*!
  \fn bool QButton::isDown() const
  Returns TRUE if the button pressed down, or FALSE if it is standing up.
  \sa setDown()
*/

/*!
  Sets the state of the button to pressed down if \e enable is TRUE
  or to standing up if \e enable is FALSE.

  If the button is a toggle button, it is \e not toggled.  Call
  toggle() as well if you need to do that.  The pressed() and
  released() signals are not emitted by this function.

  This method is provided in case you need to override the mouse event
  handlers.

  \sa isDown(), setOn(), toggle(), toggled()
*/

void QButton::setDown( bool enable )
{
    mlbDown = FALSE;				// the safe setting
    if ( (bool)buttonDown != enable ) {
	buttonDown = enable;
	repaint( FALSE );
    }
}


/*!
  \fn bool QButton::isOn() const
  Returns TRUE if this toggle button is switched on, or FALSE if it is
  switched off.
  \sa setOn(), toggleButton()
*/

/*!
  Switches a toggle button on if \e enable is TRUE or off if \e enable is
  FALSE.  This function should be called only for toggle buttons.
  \sa isOn(), toggleButton()
*/

void QButton::setOn( bool enable )
{
    if ( !toggleBt ) {
#if defined(CHECK_STATE)
	warning( "QButton::setOn(): (%s) Only toggle buttons may be switched",
		 name( "unnamed" ) );
#endif
	return;
    }

    if ( (bool)buttonOn != enable ) {		// changed state
	buttonOn = enable;
	repaint( FALSE );
	emit toggled( buttonOn );
    }
}


/*!
  \fn bool QButton::isToggleButton() const
  Returns TRUE if the button is a toggle button.
  \sa setToggleButton()
*/

/*!
  Makes the button a toggle button if \e enable is TRUE, or a normal button
  if \e enable is FALSE.

  Note that this function is protected. It is called from subclasses
  to enable the toggle functionality. QCheckBox and QRadioButton are
  toggle buttons. QPushButton is initially not a toggle button, but
  QPushButton::setToggleButton() can be called to create toggle buttons.

  \sa isToggleButton()
*/

void QButton::setToggleButton( bool enable )
{
    toggleBt = enable;
}


/*!
  Returns TRUE if \e pos is inside the widget rectangle, or FALSE if it
  is outside.

  This virtual function is reimplemented by subclasses.
*/

bool QButton::hitButton( const QPoint &pos ) const
{
    return rect().contains( pos );
}

/*!
  Draws the button.  The default implementation does nothing.

  This virtual function is reimplemented by subclasses to draw real buttons.
*/

void QButton::drawButton( QPainter * )
{
    return;
}

/*!
  Draws the button text or pixmap.  The default implementation does nothing.

  This virtual function is reimplemented by subclasses to draw real buttons.
*/

void QButton::drawButtonLabel( QPainter * )
{
    return;
}


/*!
  Handles keyboard events for the button.

  Space calls animateClick(), the arrow keys cause focus changes.
*/

void QButton::keyPressEvent( QKeyEvent *e )
{
    if ( e->key() == Key_Space )
	animateClick();
    else if ( e->key() == Key_Up || e->key() == Key_Left )
	focusNextPrevChild( FALSE );
    else if ( e->key() == Key_Down || e->key() == Key_Right )
	focusNextPrevChild( TRUE );
    else
	e->ignore();
}


/*!
  Handles mouse press events for the button.
  \sa mouseReleaseEvent()
*/

void QButton::mousePressEvent( QMouseEvent *e )
{
    if ( e->button() != LeftButton )
	return;
    bool hit = hitButton( e->pos() );
    if ( hit ) {				// mouse press on button
	mlbDown = TRUE;				// left mouse button down
	buttonDown = TRUE;
	repaint( FALSE );
	emit pressed();
	if ( repeat )
	    timer()->start( autoRepeatDelay, TRUE );
    }
}

/*!
  Handles mouse release events for the button.
  \sa mousePressEvent()
*/

void QButton::mouseReleaseEvent( QMouseEvent *e)
{
    if ( e->button() != LeftButton || !mlbDown )
	return;
    if ( d )
        timer()->stop();
    mlbDown = FALSE;				// left mouse button up
    buttonDown = FALSE;
    if ( hitButton( e->pos() ) ) {		// mouse release on button
	int t = toggleBt && ( !isOn() || !group() || !group()->isExclusive() );
	if ( t )
	    buttonOn = !buttonOn;
	repaint( FALSE );
	if ( t )
	    emit toggled( buttonOn );
	emit released();
	emit clicked();
    } else {
	repaint( FALSE );
	emit released();
    }
}

/*!
  Handles mouse move events for the button.
  \sa mousePressEvent(), mouseReleaseEvent()
*/

void QButton::mouseMoveEvent( QMouseEvent *e )
{
    if ( !((e->state() & LeftButton) && mlbDown) )
	return;					// left mouse button is up
    if ( hitButton( e->pos() ) ) {		// mouse move in button
	if ( !buttonDown ) {
	    buttonDown = TRUE;
	    repaint( FALSE );
	    emit pressed();
	}
    } else {					// mouse move outside button
	if ( buttonDown ) {
	    buttonDown = FALSE;
	    repaint( FALSE );
	    emit released();
	}
    }
}


/*!
  Handles paint events for the button.

  Opens the painter on the button and calls drawButton().
*/

void QButton::paintEvent( QPaintEvent *event )
{
    QPainter paint;

    if ( event &&
	 width() <= drawingPixWidth &&
	 height() <= drawingPixHeight ) {
	QPixmap *pm = getDrawingPixmap();
	ASSERT( pm );
	
	pm->fill( this, 0, 0 );
	paint.begin( pm, this );
	drawButton( &paint );
	paint.end();
	
	bitBlt( this, event->rect().topLeft(), pm, event->rect() );
    } else {
	paint.begin( this );

	// This optimization is worth it, since we often call repaint()
	// to draw exactly the whole button.
	if ( event && !event->rect().contains(rect()) )
	    paint.setClipRect( event->rect() );

	if ( event )
	    erase( event->rect() );
	else
	    erase();

	drawButton( &paint );
	paint.end();
    }


}

/*!
  Handles focus in events for the button.
  \sa focusOutEvent()
*/

void QButton::focusInEvent( QFocusEvent * )
{
    repaint( FALSE );
}

/*!
  Handles focus out events for the button.
  \sa focusInEvent()
*/

void QButton::focusOutEvent( QFocusEvent * )
{
    repaint( FALSE );
}


/*!
  Internal slot used for auto repeat.
*/

void QButton::autoRepeatTimeout()
{
    if ( mlbDown && isEnabled() && autoRepeat() ) {
	if ( buttonDown ) {
	    emit released();
	    emit clicked();
	    emit pressed();
	}
	timer()->start( autoRepeatPeriod, TRUE );
    }
}


/*!
  Internal slot used for the second stage of animateClick().
*/

void QButton::animateTimeout()
{
    if ( !animation )
	return;
    animation  = FALSE;
    buttonDown = FALSE;
    bool t = isToggleButton() &&
	     (!isOn() || !group() || !group()->isExclusive());
    if ( t )
	buttonOn = !buttonOn;
    repaint( FALSE );
    if ( t )
	emit toggled( buttonOn );
    emit released();
    emit clicked();
}


/*! \reimp */

void QButton::enabledChange( bool e )
{
    if ( !e )
	setDown( FALSE );
    QWidget::enabledChange( e );
}


/*!  if this is a toggle button, toggles it. */

void QButton::toggle()
{
    if ( isToggleButton() )
	 setOn( !isOn() );
}
