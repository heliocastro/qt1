/**********************************************************************
** $Id: qgroupbox.cpp,v 2.13 1998/07/03 00:09:48 hanord Exp $
**
** Implementation of QGroupBox widget class
**
** Created : 950203
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

#include "qgroupbox.h"
#include "qpainter.h"

/*!
  \class QGroupBox qgroupbox.h
  \brief The QGroupBox widget provides a group box frame with a title.

  \ingroup realwidgets

  The intended use of a group box is to show that a number of widgets
  (i.e. child widgets) are logically related.  QPrintDialog is a good
  example; each of its three panes is one group box.

  By default, the group's setFont() and setPalette() functions do not
  change the appearance of the widgets it contaisn, but you can use
  setFontPropagation() and setPalettePropagation() to change that.

  Qt also provides a more specialized group box, QButtonGroup, that is
  very useful for organizing buttons in a group.

  <img src=qgrpbox-m.gif> <img src=qgrpbox-w.gif>
*/


/*!
  Constructs a group box widget with no title.

  The \e parent and \e name arguments are passed to the QWidget constructor.
*/

QGroupBox::QGroupBox( QWidget *parent, const char *name )
    : QFrame( parent, name )
{
    init();
}

/*!
  Constructs a group box with a title.

  The \e parent and \e name arguments are passed to the QWidget constructor.
*/

QGroupBox::QGroupBox( const char *title, QWidget *parent, const char *name )
    : QFrame( parent, name )
{
    init();
    setTitle( title );
}

void QGroupBox::init()
{
    int fs;
    switch ( style() ) {
	case WindowsStyle:
	case MotifStyle:
	    align = AlignLeft;
	    fs = QFrame::Box | QFrame::Sunken;
	    break;
	default:
	    align = AlignHCenter;
	    fs = QFrame::Box | QFrame::Plain;
    }
    setFrameStyle( fs );
}


/*!
  Sets the group box title text to \e title.
*/

void QGroupBox::setTitle( const char *title )
{
    if ( str == title )				// no change
	return;
    str = title;
    repaint();
}

/*!
  \fn const char*  QGroupBox::title() const
  Returns the group box title text.
*/

/*!
  \fn int QGroupBox::alignment() const
  Returns the alignment of the group box title.

  The default alignment is \c AlignHCenter.

  \sa setAlignment()
*/

/*!
  Sets the alignment of the group box title.

  The title is always placed on the upper frame line, however,
  the horizontal alignment can be specified by the \e alignment parameter.

  The \e alignment is the bitwise OR of the following flags:
  <ul>
  <li> \c AlignLeft aligns the title text to the left.
  <li> \c AlignRight aligns the title text to the right.
  <li> \c AlignHCenter aligns the title text centered.
  </ul>

  \sa alignment()
*/

void QGroupBox::setAlignment( int alignment )
{
    align = alignment;
    repaint();
}


/*!
  Handles paint events for the group box.
  \internal
  overrides QFrame::paintEvent
*/

void QGroupBox::paintEvent( QPaintEvent *event )
{
    int		tw  = 0;
    QRect	cr  = rect();
    QRect	r   = cr;
    int		len = str.length();
    QColorGroup g = colorGroup();
    QPainter	paint( this );

    if ( event )
	paint.setClipRect( event->rect() );

    if ( len == 0 )				// no title
	setFrameRect( QRect(0,0,0,0) );		//  then use client rect
    else {					// set up region for title
	QFontMetrics fm = paint.fontMetrics();
	int h = fm.height();
	while ( len ) {
	    tw = fm.width( str, len ) + 2*fm.width( ' ' );
	    if ( tw < cr.width() )
		break;
	    len--;
	}
	if ( len ) {
	    r.setTop( h/2 );			// frame rect should be
	    setFrameRect( r );			//   smaller than client rect
	    int x;
	    if ( align & AlignHCenter )		// center alignment
		x = r.width()/2 - tw/2;
	    else if ( align & AlignRight )	// right alignment
		x = r.width() - tw - 8;
	    else				// left alignment
		x = 8;
	    r.setRect( x, 0, tw, h );
	    QRegion rgn_all( cr );
	    QRegion rgn_title( r );
	    rgn_all = rgn_all.subtract( rgn_title );
	    paint.setClipRegion( rgn_all );	// clip everything but title
	}
    }
    drawFrame( &paint );			// draw the frame
    if ( tw ) {					// draw the title
	paint.setClipping( FALSE );
	paint.setPen( g.text() );
	paint.drawText( r, AlignCenter, str, len );
    }
    drawContents( &paint );
}
