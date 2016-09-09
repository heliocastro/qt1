/****************************************************************************
** $Id: qcheckbox.cpp,v 2.24 1998/07/03 00:09:47 hanord Exp $
**
** Implementation of QCheckBox class
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

#include "qcheckbox.h"
#include "qpainter.h"
#include "qdrawutil.h"
#include "qpixmap.h"
#include "qpixmapcache.h"

/*!
  \class QCheckBox qcheckbox.h
  \brief The QCheckBox widget provides a check box with a text label.

  \ingroup realwidgets

  QCheckBox and QRadioButton are both toggle buttons, but a check box
  represents an independent switch that can be on (checked) or off
  (unchecked).
  
  <img src=qchkbox-m.gif> <img src=qchkbox-w.gif>

  \sa QButton QRadioButton
  <a href="guibooks.html#fowler">Fowler: Check Box.</a>
*/


static QSize sizeOfBitmap( GUIStyle gs )
{
    switch ( gs ) {				// calculate coords
	case WindowsStyle:
	    return QSize( 13, 13 );
	case MotifStyle:
	    return QSize( 10, 10 );
	default:
	    return QSize( 10, 10 );
    }
}

/*!
  Constructs a check box with no text.

  The \e parent and \e name arguments are sent to the QWidget constructor.
*/

QCheckBox::QCheckBox( QWidget *parent, const char *name )
	: QButton( parent, name )
{
    setToggleButton( TRUE );
}

/*!
  Constructs a check box with a text.

  The \e parent and \e name arguments are sent to the QWidget constructor.
*/

QCheckBox::QCheckBox( const char *text, QWidget *parent, const char *name )
	: QButton( parent, name )
{
    setText( text );
    setToggleButton( TRUE );
}


/*!
  \fn bool QCheckBox::isChecked() const
  Returns TRUE if the check box is checked, or FALSE if it is not checked.
  \sa setChecked()
*/

/*!
  \fn void QCheckBox::setChecked( bool check )
  Checks the check box if \e check is TRUE, or unchecks it if \e check
  is FALSE.
  \sa isChecked()
*/


static int extraWidth( int gs )
{
    if ( gs == MotifStyle )
	return 8;
    else
	return 6;
}


/*!
  Returns a size which fits the contents of the check box.
*/

QSize QCheckBox::sizeHint() const
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

    return sz + QSize( bmsz.width() + (style()==MotifStyle ? 1 : 0)
			+ (text() ? 4 + extraWidth(gs) : 0),
			4 );
}


/*!
  Draws the check box.  Calls drawButtonLabel() to draw the content.
  \sa drawButtonLabel()
*/

void QCheckBox::drawButton( QPainter *paint )
{
    QPainter	*p = paint;
    GUIStyle	 gs = style();
    QColorGroup	 g  = colorGroup();
    int		 x, y;

    QFontMetrics fm = fontMetrics();
    QSize lsz = fm.size(ShowPrefix, text());
    QSize sz = sizeOfBitmap( gs );
    x = gs == MotifStyle ? 1 : 0;
    y = (height() - lsz.height() + fm.height() - sz.height())/2;

#define SAVE_CHECKBOX_PIXMAPS
#if defined(SAVE_CHECKBOX_PIXMAPS)
    QString pmkey;				// pixmap key
    int kf = 0;
    if ( isDown() )
	kf |= 1;
    if ( isOn() )
	kf |= 2;
    if ( isEnabled() )
	kf |= 4;
    pmkey.sprintf( "$qt_check_%d_%d_%d", gs, palette().serialNumber(), kf );
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

    if ( gs == WindowsStyle ) {			// Windows check box
	QColor fillColor;
	if ( isDown() )
	    fillColor = g.background();
	else
	    fillColor = g.base();
	QBrush fill( fillColor );
	qDrawWinPanel( p, x, y, sz.width(), sz.height(), g, TRUE, &fill );
	if ( isOn() ) {
	    QPointArray a( 7*2 );
	    int i, xx, yy;
	    xx = x+3;
	    yy = y+5;
	    for ( i=0; i<3; i++ ) {
		a.setPoint( 2*i,   xx, yy );
		a.setPoint( 2*i+1, xx, yy+2 );
		xx++; yy++;
	    }
	    yy -= 2;
	    for ( i=3; i<7; i++ ) {
		a.setPoint( 2*i,   xx, yy );
		a.setPoint( 2*i+1, xx, yy+2 );
		xx++; yy--;
	    }
	    p->setPen( black );
	    p->drawLineSegments( a );
	}
    }
    if ( gs == MotifStyle ) {			// Motif check box
	bool showUp = !(isDown() ^ isOn());
	QBrush fill( showUp ? g.background() : g.mid() );
	qDrawShadePanel( p, x, y, sz.width(), sz.height(), g, !showUp, 2, &fill );
    }

#if defined(SAVE_CHECKBOX_PIXMAPS)
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
  Draws the check box label.
  \sa drawButton()
*/

void QCheckBox::drawButtonLabel( QPainter *p )
{
    int x, y, w, h;
    GUIStyle gs = style();
    QSize sz = sizeOfBitmap( gs );
    y = 0;
    x = sz.width() + extraWidth( gs );
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
