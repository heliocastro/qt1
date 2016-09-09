/****************************************************************************
** $Id: qradiobutton.cpp,v 2.29 1998/07/03 00:09:52 hanord Exp $
**
** Implementation of QRadioButton class
**
** Created : 940222
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

#include "qradiobutton.h"
#include "qbuttongroup.h"
#include "qpainter.h"
#include "qdrawutil.h"
#include "qpixmap.h"
#include "qpixmapcache.h"
#include "qbitmap.h"
#include "qkeycode.h"

/*!
  \class QRadioButton qradiobutton.h
  \brief The QRadioButton widget provides a radio button with a text label.

  \ingroup realwidgets

  QRadioButton and QCheckBox are both toggle buttons, that is, they can be
  switched on (checked) or off (unchecked).  Unlike check boxes, radio
  buttons are normally organized in groups where only one radio button can be
  switched on at a time.

  The QButtonGroup widget is very useful for defining groups of radio buttons.

  <img src=qradiobt-m.gif> <img src=qradiobt-w.gif>

  \sa QPushButton QToolButton
  <a href="guibooks.html#fowler">GUI Design Handbook: Radio Button</a>
*/


static QSize sizeOfBitmap( GUIStyle gs )
{
    switch ( gs ) {
	case WindowsStyle:
	    return QSize(12,12);
	case MotifStyle:
	    return QSize(13,13);
	default:
	    return QSize(10,10);
    }
}

static const int gutter = 6; // between button and text
static const int margin = 2; // to right of text


/*!
  Constructs a radio button with no text.

  The \e parent and \e name arguments are sent to the QWidget constructor.
*/

QRadioButton::QRadioButton( QWidget *parent, const char *name )
	: QButton( parent, name )
{
    init();
}

/*!
  Constructs a radio button with a text.

  The \e parent and \e name arguments are sent to the QWidget constructor.
*/

QRadioButton::QRadioButton( const char *text, QWidget *parent,
			    const char *name )
	: QButton( parent, name )
{
    init();
    setText( text );
}

/*!
  Initializes the radio button.
*/

void QRadioButton::init()
{
    setToggleButton( TRUE );
    noHit = FALSE;
    if ( parentWidget()->inherits("QButtonGroup") ) {
	QButtonGroup *bgrp = (QButtonGroup *)parentWidget();
	bgrp->setExclusive( TRUE );
    }
}


/*!
  \fn bool QRadioButton::isChecked() const
  Returns TRUE if the radio button is checked, or FALSE if it is not checked.
  \sa setChecked()
*/

/*!
  \fn void QRadioButton::setChecked( bool check )
  Checks the radio button if \e check is TRUE, or unchecks it if \e check
  is FALSE.

  Calling this function does not affect other radio buttons unless a radio
  button group has been defined using the QButtonGroup widget.

  \sa isChecked()
*/


/*!
  Returns a size which fits the contents of the radio button.
*/

QSize QRadioButton::sizeHint() const
{
    // Any more complex, and we will use qItemRect()
    // NB: QCheckBox::sizeHint() is similar

    QSize sz;
    if (pixmap()) {
	sz = pixmap()->size();
    } else {
	sz = fontMetrics().size( ShowPrefix, text() );
    }
    GUIStyle gs = style();
    QSize bmsz = sizeOfBitmap( gs );
    if ( sz.height() < bmsz.height() )
	sz.setHeight( bmsz.height() );

    return sz + QSize( bmsz.width()
			+ (text() ? gutter+margin : 0),
			4 );
}


/*!
  Reimplements QButton::hitButton().  This function is implemented to
  prevent a radio button that is \link isOn() on \endlink from being
  switched off.
*/

bool QRadioButton::hitButton( const QPoint &pos ) const
{
    return noHit ? FALSE : rect().contains( pos );
}


/*!
  Draws the radio button, but not the button label.
  \sa drawButtonLabel()
*/

void QRadioButton::drawButton( QPainter *paint )
{
    QPainter	*p = paint;
    GUIStyle	 gs = style();
    QColorGroup	 g  = colorGroup();
    int		 x, y;

    QFontMetrics fm = fontMetrics();
    QSize lsz = fm.size(ShowPrefix, text());
    QSize sz = sizeOfBitmap( gs );
    x = 0;
    y = (height() - lsz.height() + fm.height() - sz.height())/2;

#define SAVE_RADIOBUTTON_PIXMAPS
#if defined(SAVE_RADIOBUTTON_PIXMAPS)
    QString pmkey;				// pixmap key
    int kf = 0;
    if ( isDown() )
	kf |= 1;
    if ( isOn() )
	kf |= 2;
    if ( isEnabled() )
	kf |= 4;
    pmkey.sprintf( "$qt_radio_%d_%d_%d", gs, palette().serialNumber(), kf );
    QPixmap *pm = QPixmapCache::find( pmkey );
    if ( pm ) {					// pixmap exists
	p->drawPixmap( x, y, *pm );
	drawButtonLabel( p );
	return;
    }
    bool use_pm = TRUE;
    QPainter pmpaint;
    int wx, wy;
    if ( use_pm ) {
	pm = new QPixmap( sz );			// create new pixmap
	CHECK_PTR( pm );
	pmpaint.begin( pm );
	p = &pmpaint;				// draw in pixmap
	wx=x;  wy=y;				// save x,y coords
	x = y = 0;
	p->setBackgroundColor( g.background() );
    }
#endif

#define QCOORDARRLEN(x) sizeof(x)/(sizeof(QCOORD)*2)

    if ( gs == WindowsStyle ) {			// Windows radio button
	static QCOORD pts1[] = {		// dark lines
	    1,9, 1,8, 0,7, 0,4, 1,3, 1,2, 2,1, 3,1, 4,0, 7,0, 8,1, 9,1 };
	static QCOORD pts2[] = {		// black lines
	    2,8, 1,7, 1,4, 2,3, 2,2, 3,2, 4,1, 7,1, 8,2, 9,2 };
	static QCOORD pts3[] = {		// background lines
	    2,9, 3,9, 4,10, 7,10, 8,9, 9,9, 9,8, 10,7, 10,4, 9,3 };
	static QCOORD pts4[] = {		// white lines
	    2,10, 3,10, 4,11, 7,11, 8,10, 9,10, 10,9, 10,8, 11,7,
	    11,4, 10,3, 10,2 };
	static QCOORD pts5[] = {		// inner fill
	    4,2, 7,2, 9,4, 9,7, 7,9, 4,9, 2,7, 2,4 };
	p->eraseRect( x, y, sz.width(), sz.height() );
	QPointArray a( QCOORDARRLEN(pts1), pts1 );
	a.translate( x, y );
	p->setPen( g.dark() );
	p->drawPolyline( a );
	a.setPoints( QCOORDARRLEN(pts2), pts2 );
	a.translate( x, y );
	p->setPen( black );
	p->drawPolyline( a );
	a.setPoints( QCOORDARRLEN(pts3), pts3 );
	a.translate( x, y );
	p->setPen( g.midlight() );
	p->drawPolyline( a );
	a.setPoints( QCOORDARRLEN(pts4), pts4 );
	a.translate( x, y );
	p->setPen( g.light() );
	p->drawPolyline( a );
	a.setPoints( QCOORDARRLEN(pts5), pts5 );
	a.translate( x, y );
	QColor fillColor = isDown() ? g.background() : g.base();
	p->setPen( fillColor );
	p->setBrush( fillColor );
	p->drawPolygon( a );
	if ( isOn() ) {
	    p->setPen( NoPen );
	    p->setBrush( g.foreground() );
	    p->drawRect( x+5, y+4, 2, 4 );
	    p->drawRect( x+4, y+5, 4, 2 );
	}
    } else if ( gs == MotifStyle ) {		// Motif radio button
	static QCOORD inner_pts[] =		// used for filling diamond
	    { 2,6, 6,2, 10,6, 6,10 };
	static QCOORD top_pts[] =		// top (^) of diamond
	    { 0,6, 6,0 , 11,5, 10,5, 6,1, 1,6, 2,6, 6,2, 9,5 };
	static QCOORD bottom_pts[] =		// bottom (V) of diamond
	    { 1,7, 6,12, 12,6, 11,6, 6,11, 2,7, 3,7, 6,10, 10,6 };
	bool showUp = !(isDown() ^ isOn());
	QPointArray a( QCOORDARRLEN(inner_pts), inner_pts );
	p->eraseRect( x, y, sz.width(), sz.height() );
	p->setPen( NoPen );
	p->setBrush( showUp ? g.background() : g.mid() );
	a.translate( x, y );
	p->drawPolygon( a );			// clear inner area
	p->setPen( showUp ? g.light() : g.dark() );
	p->setBrush( NoBrush );
	a.setPoints( QCOORDARRLEN(top_pts), top_pts );
	a.translate( x, y );
	p->drawPolyline( a );			// draw top part
	p->setPen( showUp ? g.dark() : g.light() );
	a.setPoints( QCOORDARRLEN(bottom_pts), bottom_pts );
	a.translate( x, y );
	p->drawPolyline( a );			// draw bottom part
    }

#if defined(SAVE_RADIOBUTTON_PIXMAPS)
    if ( use_pm ) {
	pmpaint.end();
	p = paint;				// draw in default device
	p->drawPixmap( wx, wy, *pm );
	if (!QPixmapCache::insert(pmkey, pm) )	// save in cache
	    delete pm;
    }
#endif
    drawButtonLabel( p );
}


/*!
  Draws the radio button label.
  \sa drawButton()
*/

void QRadioButton::drawButtonLabel( QPainter *p )
{
    int x, y, w, h;
    GUIStyle gs = style();
    QSize sz = sizeOfBitmap( gs );
    if ( gs == WindowsStyle )
	sz.setWidth(sz.width()+1);
    y = 0;
    x = sz.width() + gutter;
    w = width() - x;
    h = height();

    qDrawItem( p, gs, x, y, w, h,
	       AlignLeft|AlignVCenter|ShowPrefix,
	       colorGroup(), isEnabled(),
	       pixmap(), text() );

    if ( hasFocus() ) {
	QRect br = qItemRect( p, gs, x, y, w, h,
			      AlignLeft|AlignVCenter|ShowPrefix,
			      isEnabled(),
			      pixmap(), text() );
	br.setLeft( br.left()-3 );
	br.setRight( br.right()+2 );
	br.setTop( br.top()-2 );
	br.setBottom( br.bottom()+2);
	br = br.intersect( QRect(0,0,width(),height()) );

	if ( gs == WindowsStyle ) {
	    p->drawWinFocusRect( br, backgroundColor() );
	} else {
	    p->setPen( black );
	    p->drawRect( br );
	}
    }


}


/*!  Obsolete; to be removed in Qt 2.0. */

void QRadioButton::mouseReleaseEvent( QMouseEvent *e )
{
    QButton::mouseReleaseEvent( e );
}


/*!  Obsolete; to be removed in Qt 2.0. */

void QRadioButton::keyPressEvent( QKeyEvent * e )
{
    QButton::keyPressEvent( e );
}
