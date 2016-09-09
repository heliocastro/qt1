/****************************************************************************
** $Id: qtoolbutton.cpp,v 2.30.2.1 1999/02/01 10:44:11 hanord Exp $
**
** Implementation of QToolButton class
**
** Created : 980320
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

#include "qtoolbutton.h"

#include "qdrawutil.h"
#include "qpainter.h"
#include "qpixmap.h"
#include "qwmatrix.h"
#include "qapplication.h"
#include "qtooltip.h"
#include "qtoolbar.h"
#include "qimage.h"
#include "qiconset.h"

static QToolButton * threeDeeButton = 0;


class QToolButtonPrivate
{
};


/*! \class QToolButton qtoolbutton.h

  \brief The QToolButton class provides a push button whose appearance
  has been tailored for use in a QToolBar.

  \ingroup realwidgets

  This means that it implements the ridiculous Microsoft auto-raise
  feature using QIconSet.  Apart from that, it's pretty much like a
  QPushButton.  The two classes may at some point be merged.
  
  \sa QPushButton QToolButton
  <a href="guibooks.html#fowler">GUI Design Handbook: Push Button</a>
*/


/*!  Constructs an empty tool button. */

QToolButton::QToolButton( QWidget * parent, const char * name )
    : QButton( parent, name )
{
    init();
    setUsesBigPixmap( FALSE );
}


/*!  Set-up code common to all the constructors */

void QToolButton::init()
{
    d = 0;
    bpID = bp.serialNumber();
    spID = sp.serialNumber();

    utl = FALSE;
    ubp = TRUE;

    s = 0;
}


/*!  Creates a tool button that is a child of \a parent (which must be
  a QToolBar) and named \a name.

  The tool button will display \a pm, with text label or tool tip \a
  textLabel and status-bar message \a grouptext, connected to \a slot
  in object \a receiver, and returns the button.
*/

QToolButton::QToolButton( const QPixmap & pm, const char * textLabel,
			  const char * grouptext,
			  QObject * receiver, const char * slot,
			  QToolBar * parent, const char * name )
    : QButton( parent, name )
{
    init();
    setPixmap( pm );
    setTextLabel( textLabel );
    if ( receiver && slot )
	connect( this, SIGNAL(clicked()), receiver, slot );
    if ( parent->mainWindow() ) {
	connect( parent->mainWindow(), SIGNAL(pixmapSizeChanged(bool)),
		 this, SLOT(setUsesBigPixmap(bool)) );
	setUsesBigPixmap( parent->mainWindow()->usesBigPixmaps() );
    } else {
	setUsesBigPixmap( FALSE );
    }
    if ( textLabel && *textLabel ) {
	if ( grouptext && *grouptext )
	    QToolTip::add( this, textLabel,
			   parent->mainWindow()->toolTipGroup(), grouptext );
	else
	    QToolTip::add( this, textLabel );
    }
}


/*!  Creates a tool button that is a child of \a parent (which must be
  a QToolBar) and named \a name.

  The tool button will display \a iconSet, with text label or tool tip \a
  textLabel and status-bar message \a grouptext, connected to \a slot
  in object \a receiver, and returns the button.
*/

QToolButton::QToolButton( QIconSet iconSet, const char * textLabel,
			  const char * grouptext,
			  QObject * receiver, const char * slot,
			  QToolBar * parent, const char * name )
    : QButton( parent, name )
{
    init();
    setIconSet( iconSet );
    setTextLabel( textLabel );
    if ( receiver && slot )
	connect( this, SIGNAL(clicked()), receiver, slot );
    if ( parent->mainWindow() ) {
	connect( parent->mainWindow(), SIGNAL(pixmapSizeChanged(bool)),
		 this, SLOT(setUsesBigPixmap(bool)) );
	setUsesBigPixmap( parent->mainWindow()->usesBigPixmaps() );
    } else {
	setUsesBigPixmap( FALSE );
    }
    if ( textLabel && *textLabel ) {
	if ( grouptext && *grouptext )
	    QToolTip::add( this, textLabel,
			   parent->mainWindow()->toolTipGroup(), grouptext );
	else
	    QToolTip::add( this, textLabel );
    }
}


/*! Destroys the object and frees any allocated resources. */

QToolButton::~QToolButton()
{
    delete d;
    delete s;
    threeDeeButton = 0;
}


/*!
  Makes the tool button a toggle button if \e enable is TRUE, or a normal
  tool button if \e enable is FALSE.

  Toggle buttons have an on/off state similar to \link QCheckBox check
  boxes. \endlink A tool button is initially not a toggle button.

  \sa setOn(), toggle(), toggleButton() toggled()
*/

void QToolButton::setToggleButton( bool enable )
{
    QButton::setToggleButton( enable );
}


/*!  Returns a size suitable for this tool button.  This depends on
  \link style() GUI style,\endlink usesBigPixmap(), textLabel() and
  usesTextLabel().
*/

QSize QToolButton::sizeHint() const
{
    int w, h;

    if ( text() ) {
	w = fontMetrics().width( text() );
	h = fontMetrics().height(); // boundingRect()?
    } else if ( usesBigPixmap() ) {
	w = h = 32;
    } else {
	w = h = 16;
    }

    if ( usesTextLabel() ) {
	h += 4 + fontMetrics().height();
	int tw = fontMetrics().width( textLabel() );
	if ( tw > w )
	    w = tw;
    }
    return QSize( w + 7, h + 6 );
}



/* \fn bool QToolButton::usesBigPixmap() const

  Returns TRUE or FALSE.

*/


/* \fn bool QToolButton::usesTextLabel() const

  Returns TRUE or FALSE.

*/


/*! \fn const char * QToolButton::textLabel() const

  Returns the text label in use by this tool button, or 0.

  \sa setTextLabel() usesTextLabel() setUsesTextLabel() setText()
*/



/*!  Sets this button to use the big pixmaps provided by its QIconSet
  if \a enable is TRUE, and to use the small ones else.

  QToolButton automatically connects this slot to the relevant signal
  in the QMainWindow in which is resides.
*/

void QToolButton::setUsesBigPixmap( bool enable )
{
    if ( (bool)ubp == enable )
	return;

    ubp = enable;

    if ( parent() )
	QApplication::postEvent( parent(), new QEvent( Event_LayoutHint ) );
}


/*!  \fn bool QToolButton::usesBigPixmap() const

  Returns TRUE if this tool button uses the big (32-pixel) pixmaps,
  and FALSE if it does not.  \sa setUsesBigPixmap(), setPixmap(),
  usesTextLabel
*/


/*!  Sets this button to draw a text label below the icon if \a enable
  is TRUE, and to not draw it if \a enable is FALSE.

  QToolButton automatically connects this slot to the relevant signal
  in the QMainWindow in which is resides.
*/

void QToolButton::setUsesTextLabel( bool enable )
{
    if ( (bool)utl == enable )
	return;

    utl = enable;

    if ( parent() )
	QApplication::postEvent( parent(), new QEvent( Event_LayoutHint ) );
}


/*! \fn bool QToolButton::usesTextLabel() const

  Returns TRUE if this tool button puts a text label below the button
  pixmap, and FALSE if it does not. \sa setUsesTextLabel()
  setTextLabel() usesBigPixmap()
*/


/*!  Sets this tool button to be on if \a enable is TRUE, and off it
  \a enable is FALSE.

  This function has no effect on \link isToggleButton() non-toggling
  buttons. \endlink

  \sa isToggleButton() toggle()
*/

void QToolButton::setOn( bool enable )
{
    if ( !isToggleButton() )
	return;
    QButton::setOn( enable );
}


/*!  Toggles the state of this tool button.

  This function has no effect on \link isToggleButton() non-toggling
  buttons. \endlink

  \sa isToggleButton() toggle()
*/

void QToolButton::toggle()
{
    if ( !isToggleButton() )
	return;
    QButton::setOn( !isOn() );
}


/*!  Draws the edges and decoration of the button (pretty much
  nothing) and calls drawButtonLabel().

  \sa drawButtonLabel() QButton::paintEvent() */

void QToolButton::drawButton( QPainter * p )
{
    if ( uses3D() || isOn() ) {
	QPointArray a;
	a.setPoints( 3, 0, height()-1, 0, 0, width()-1, 0 );
	if ( isOn() && !isDown() && !uses3D() ) {
	    if ( style() == WindowsStyle ) {
		p->setBrush( QBrush(white,Dense4Pattern) );
		p->setPen( NoPen );
		p->setBackgroundMode( OpaqueMode );
		p->drawRect( 0,0, width(),height() );
		p->setBackgroundMode( TransparentMode );
	    } else {
		p->setBrush( colorGroup().mid() );
		p->setPen( NoPen );
		p->drawRect( 0,0, width(),height() );
	    }
	}
	p->setPen( isDown() || isOn()
		   ? colorGroup().dark()
		   : colorGroup().light() );
	p->drawPolyline( a );
	a[1] = QPoint( width()-1, height()-1 );
	p->setPen( isDown() || isOn()
		   ? colorGroup().light()
		   : colorGroup().dark() );
	p->drawPolyline( a );
    }
    drawButtonLabel( p );

    if ( hasFocus() ) {
        if ( style() == WindowsStyle ) {
            p->drawWinFocusRect( 3, 3, width()-6, height()-6,
                                 colorGroup().background() );
        } else {
            p->setPen( black );
            p->drawRect( 3, 3, width()-6, height()-6 );
        }
    }
}


/*!  Draws the contents of the button (pixmap and optionally text).

  \sa drawButton() QButton::paintEvent() */

void QToolButton::drawButtonLabel( QPainter * p )
{
    if ( text() ) {
	qDrawItem( p, style(), 1, 1, width()-2, height()-2,
		   AlignCenter + ShowPrefix,
		   colorGroup(), isEnabled(),
		   0, text() );
    } else {

	QPixmap pm;
	if ( usesBigPixmap() ) {
	    if ( !isEnabled() )
		pm = iconSet().pixmap( QIconSet::Large, QIconSet::Disabled );
	    else if ( uses3D() )
		pm = iconSet().pixmap( QIconSet::Large, QIconSet::Active );
	    else
		pm = iconSet().pixmap( QIconSet::Large, QIconSet::Normal );
	} else {
	    if ( !isEnabled() )
		pm = iconSet().pixmap( QIconSet::Small, QIconSet::Disabled );
	    else if ( uses3D() )
		pm = iconSet().pixmap( QIconSet::Small, QIconSet::Active );
	    else
		pm = iconSet().pixmap( QIconSet::Small, QIconSet::Normal );
	}

	if ( usesTextLabel() ) {
	    int fh = fontMetrics().height();
	    qDrawItem( p, style(), 1, 1, width()-2, height() - 2 - fh - 6,
		       AlignCenter, colorGroup(), TRUE, &pm, 0 );
	    p->setFont( font() );
	    qDrawItem( p, style(), 1, height() - 4 - fh, width()-2, fh,
		       AlignCenter + ShowPrefix,
		       colorGroup(), isEnabled(),
		       0, textLabel() );
	} else {
	    qDrawItem( p, style(), 1, 1, width()-2, height() - 2,
		       AlignCenter, colorGroup(), TRUE, &pm, 0 );

	}
    }
}


/*! Reimplemented to handle the automatic 3D effects in Windows style. */

void QToolButton::enterEvent( QEvent * e )
{
    threeDeeButton = this;
    if ( isEnabled() )
	repaint();
    QButton::enterEvent( e );
}


/*! Reimplemented to handle the automatic 3D effects in Windows style. */

void QToolButton::leaveEvent( QEvent * e )
{
    QToolButton * o = threeDeeButton;
    threeDeeButton = 0;
    if ( o && o->isEnabled() )
	o->repaint();
    QButton::leaveEvent( e );
}



/*!  Returns TRUE if this button should be drawn using raised edges.
  \sa drawButton() */

bool QToolButton::uses3D() const
{
    return threeDeeButton == this && isEnabled();
}


/*!  Sets the label of this button to \a newLabel, and automatically
  sets it as tool tip too if \a tipToo is TRUE.
*/

void QToolButton::setTextLabel( const char * newLabel , bool tipToo )
{
    tl = newLabel;
    if ( !tipToo )
	return;

    if ( usesTextLabel() )
	QToolTip::remove( this );
    else
	QToolTip::add( this, newLabel );
}





/*!  Sets this tool button to display the icons in \a set.
  (setPixmap() is effectively a wrapper for this function.)

  QToolButton makes a copy of \a set, so you must delete \a set
  yourself.

  \sa iconSet() QIconSet
*/

void QToolButton::setIconSet( const QIconSet & set )
{
    if ( s )
	delete s;
    s = new QIconSet( set );
}


/*!  Returns a copy of the icon set in use.  If no icon set has been
  set, iconSeT() creates one from the pixmap().

  If the button doesn't have a pixmap either, iconSet()'s return value
  is meaningless.

  \sa setIconSet() QIconSet
*/

QIconSet QToolButton::iconSet() const
{
    QToolButton * that = (QToolButton *)this;

    if ( pixmap() && (!that->s || (that->s->pixmap().serialNumber() !=
				   pixmap()->serialNumber())) )
	that->setIconSet( *pixmap() );

    if ( that->s )
	return *that->s;

    QPixmap tmp1;
    QIconSet tmp2( tmp1, QIconSet::Small );
    return tmp2;
}
