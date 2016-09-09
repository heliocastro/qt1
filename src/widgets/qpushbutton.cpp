/****************************************************************************
** $Id: qpushbutton.cpp,v 2.44.2.1 1998/08/12 16:55:15 agulbra Exp $
**
** Implementation of QPushButton class
**
** Created : 940221
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

#include "qpushbutton.h"
#include "qdialog.h"
#include "qfontmetrics.h"
#include "qpainter.h"
#include "qdrawutil.h"
#include "qpixmap.h"
#include "qbitmap.h"

/*!
  \class QPushButton qpushbutton.h
  \brief The QPushButton widget provides a push button with a text
	    or pixmap label.

  \ingroup realwidgets

  A default push button in a dialog emits the clicked signal if the user
  presses the Enter key.

  A push button has \c TabFocus as a default focusPolicy(), i.e. it can
  get keyboard focus by tabbing but not by clicking.

  <img src="qpushbt-m.gif"> <img src="qpushbt-w.gif">

  \sa QRadioButton QToolButton
  <a href="guibooks.html#fowler">GUI Design Handbook: Push Button</a>
*/


/*!
  Constructs a push button with no text.

  The \e parent and \e name arguments are sent to the QWidget constructor.
*/

QPushButton::QPushButton( QWidget *parent, const char *name )
	: QButton( parent, name )
{
    init();
}

/*!
  Constructs a push button with a text.

  The \e parent and \e name arguments are sent to the QWidget constructor.
*/

QPushButton::QPushButton( const char *text, QWidget *parent,
			  const char *name )
	: QButton( parent, name )
{
    init();
    setText( text );
}

void QPushButton::init()
{
    autoDefButton = defButton = lastDown = lastDef = lastEnabled
		  = hasMenuArrow = FALSE;
}


/*!
  Makes the push button a toggle button if \e enable is TRUE, or a normal
  push button if \e enable is FALSE.

  Toggle buttons have an on/off state similar to \link QCheckBox check
  boxes. \endlink A push button is initially not a toggle button.

  \sa setOn(), toggle(), isToggleButton() toggled()
*/

void QPushButton::setToggleButton( bool enable )
{
    QButton::setToggleButton( enable );
}


/*!
  Switches a toggle button on if \e enable is TRUE or off if \e enable is
  FALSE.
  \sa isOn(), toggle(), toggled(), isToggleButton()
*/

void QPushButton::setOn( bool enable )
{
    if ( !isToggleButton() )
	return;
    QButton::setOn( enable );
}


/*!
  Toggles the state of a toggle button.
  \sa isOn(), setOn(), toggled(), isToggleButton()
*/

void QPushButton::toggle()
{
    if ( !isToggleButton() )
	return;
    QButton::setOn( !isOn() );
}


/*!
  \fn bool QPushButton::autoDefault() const
  Returns TRUE if the button is an auto-default button.

  \sa setAutoDefault()
*/

/*!
  Sets the push buttons to an auto-default button if \e enable is TRUE,
  or to a normal button if \e enable is FALSE.

  An auto-default button becomes the default push button automatically
  when it receives the keyboard input focus.

  \sa autoDefault(), setDefault()
*/

void QPushButton::setAutoDefault( bool enable )
{
    autoDefButton = enable;
}


/*!
  \fn bool QPushButton::isDefault() const
  Returns TRUE if the button is default.

  \sa setDefault()
*/

/*!
  Sets the button to be the default button if \e enable is TRUE, or
  to be a normal button if \e enable is FALSE.

  A default push button in a \link QDialog dialog\endlink emits the
  QButton::clicked() signal if the user presses the Enter key.	Only
  one push button in the dialog can be default.

  Default push buttons are only allowed in dialogs.

  \sa isDefault(), setAutoDefault(), QDialog
*/

void QPushButton::setDefault( bool enable )
{
    if ( (defButton && enable) || !(defButton || enable) )
	return;					// no change
    QWidget *p = topLevelWidget();
    if ( !p->inherits("QDialog") )		// not a dialog
	return;
    defButton = enable;
    if ( defButton )
	((QDialog*)p)->setDefault( this );
    if ( isVisible() )
	repaint( FALSE );
}


/*!
  Returns a size which fits the contents of the push button.
*/

QSize QPushButton::sizeHint() const
{
    int w, h;
    if ( pixmap() ) {
	QPixmap *pm = (QPixmap *)pixmap();
	w = pm->width()	 + 6;
	h = pm->height() + 6;
    } else {
	QString s( text() );
	if ( s.isEmpty() )
	    s = "XXXX";
	QFontMetrics fm = fontMetrics();
	QSize sz = fm.size( ShowPrefix, s );
	w = sz.width()	+ 6;
	h = sz.height() + sz.height()/8 + 10;
	w += h;
    }
    if ( style() == WindowsStyle ) {
	// in windows style, try a little harder to conform to
	// microsoft's size specifications
	if ( h <= 25 )
	    h = 22;
	if ( w < 85 &&
	     topLevelWidget() &&
	     topLevelWidget()->inherits( "QDialog" ) )
	    w = 80;
    }

    return QSize( w, h );
}


/*!
  Reimplements QWidget::move() for internal purposes.
*/

void QPushButton::move( int x, int y )
{
    QWidget::move( x, y );
}

/*!
  Reimplements QWidget::move() for internal purposes.
*/

void QPushButton::move( const QPoint &p )
{
    move( p.x(), p.y() );
}

/*!
  Reimplements QWidget::resize() for internal purposes.
*/

void QPushButton::resize( int w, int h )
{
    QWidget::resize( w, h );
}

/*!
  Reimplements QWidget::resize() for internal purposes.
*/

void QPushButton::resize( const QSize &s )
{
    resize( s.width(), s.height() );
}

/*!
  Reimplements QWidget::setGeometry() for internal purposes.
*/

void QPushButton::setGeometry( int x, int y, int w, int h )
{
    QWidget::setGeometry( x, y, w, h );
}

/*!
  Reimplements QWidget::setGeometry() for internal purposes.
*/

void QPushButton::setGeometry( const QRect &r )
{
    QWidget::setGeometry( r );
}


/*!
  Draws the push button, except its label.
  \sa drawButtonLabel()
*/

void QPushButton::drawButton( QPainter *paint )
{
    register QPainter *p = paint;
    GUIStyle gs   = style();
    QColorGroup g = colorGroup();
    int x1, y1, x2, y2;

    rect().coords( &x1, &y1, &x2, &y2 );	// get coordinates

    int w = x2 + 1;
    int h = y2 + 1;
    int dx = 0;
    int dy = 0;

    p->setPen( g.foreground() );
    p->setBrush( QBrush(g.background(),NoBrush) );

    if ( gs == WindowsStyle ) {		// Windows push button
	bool clearBackground = TRUE;
	if ( isDown() ) {
	    if ( defButton ) {
		p->setPen( black );
		p->drawRect( x1, y1, x2-x1+1, y2-y1+1 );
		p->setPen( g.dark() );
		p->drawRect( x1+1, y1+1, x2-x1-1, y2-y1-1 );
	    } else {
		qDrawWinButton( p, x1, y1, w, h, g, TRUE );
	    }
	} else {
	    if ( defButton ) {
		p->setPen( black );
		p->drawRect( x1, y1, w, h );
		x1++; y1++;
		x2--; y2--;
	    }
	    if ( isToggleButton() && isOn() && isEnabled() ) {
		QBrush fill(white, Dense4Pattern );
		qDrawWinButton( p, x1, y1, x2-x1+1, y2-y1+1, g, TRUE, &fill );
		clearBackground = FALSE;
	    } else {
		qDrawWinButton( p, x1, y1, x2-x1+1, y2-y1+1, g, isOn() );
	    }
	}
	if ( clearBackground )
	    p->fillRect( x1+2, y1+2, x2-x1-3, y2-y1-3, g.background() );
	if ( hasMenuArrow ) {
	    dx = (y2-y1) / 3;
	    qDrawArrow( p, DownArrow, style(), FALSE,
			x2 - dx, y1, dx, y2 - y1,
			g, isEnabled() );
	}
    } else if ( gs == MotifStyle ) {		// Motif push button
	QBrush fill;
	if ( isDown() )
	    fill = QBrush( g.mid() );
	else if ( isOn() )
	    fill = QBrush( g.mid(), Dense4Pattern );
	else
	    fill = QBrush( g.background() );

	if ( defButton ) {
	    QPointArray a;
	    a.setPoints( 9,
			 x1, y1, x2, y1, x2, y2, x1, y2, x1, y1+1,
			 x2-1, y1+1, x2-1, y2-1, x1+1, y2-1, x1+1, y1+1 );
	    p->setPen( black );
	    p->drawPolyline( a );
	    x1 += 2;
	    y1 += 2;
	    x2 -= 2;
	    y2 -= 2;
	}
	
	qDrawShadePanel( p, x1, y1, x2-x1+1, y2-y1+1, g, isOn() || isDown(),
			 2, &fill );
	

	if ( hasMenuArrow ) {
	    dx = (y1-y2-4)/3;
	    qDrawArrow( p, DownArrow, style(), FALSE,
			x2 - dx, dx, y1, y2 - y1,
			g, isEnabled() );
	}
    }

    if ( p->brush().style() != NoBrush )
	p->setBrush( NoBrush );

    if ( dx || dy )
	p->translate( dx, dy );
    drawButtonLabel( p );
    if ( dx || dy )
	p->translate( -dx, -dy );

    if ( hasFocus() ) {
	if ( style() == WindowsStyle ) {
	    p->drawWinFocusRect( x1+3, y1+3, x2-x1-5, y2-y1-5,
				 g.background() );
	} else {
	    p->setPen( black );
	    p->drawRect( x1+3, y1+3, x2-x1-5, y2-y1-5 );
	}
    }

    lastDown = isDown();
    lastDef = defButton;
    lastEnabled = isEnabled();
}


/*!
  Draws the push button label.
  \sa drawButton()
*/

void QPushButton::drawButtonLabel( QPainter *paint )
{
    register QPainter *p = paint;

    QRect r = rect();
    int x, y, w, h;
    r.rect( &x, &y, &w, &h );
    if ( (isDown() || isOn()) && style() == WindowsStyle ) {
        // shift pixmap/text
	x++;
	y++;
    }
    x += 2;  y += 2;  w -= 4;  h -= 4;
    qDrawItem( p, style(), x, y, w, h,
	       AlignCenter|ShowPrefix,
	       colorGroup(), isEnabled(),
	       pixmap(), text() );
}


/*!
  Handles focus in events for the push button.
*/

void QPushButton::focusInEvent( QFocusEvent *e )
{
    if ( autoDefButton )
	setDefault( TRUE );
    QButton::focusInEvent( e );
}



/*!  Tells this button to draw a menu indication triangle if \a enable
  is TRUE,  and to not draw one if \a enable is FALSE (the default).

  setIsMenuButton() does not cause the button to do anything other
  than draw the menu indication.

  \sa isMenuButton()
*/

void QPushButton::setIsMenuButton( bool enable )
{
    if ( (bool)hasMenuArrow == enable )
	return;
    hasMenuArrow = enable ? 1 : 0;
    repaint( FALSE );
}


/*!  Returns TRUE if this button indicates to the user that pressing
  it will pop up a menu, and FALSE otherwise.  The default is FALSE.

  \sa setIsMenuButton()
*/

bool QPushButton::isMenuButton() const
{
    return hasMenuArrow;
}
