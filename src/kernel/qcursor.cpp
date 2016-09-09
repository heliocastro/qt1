/****************************************************************************
** $Id: qcursor.cpp,v 2.9 1998/07/03 00:09:31 hanord Exp $
**
** Implementation of QCursor class
**
** Created : 940220
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

#include "qcursor.h"
#include "qbitmap.h"
#include "qdatastream.h"

/*!
  \class QCursor qcursor.h

  \brief The QCursor class provides a mouse cursor with an arbitrary shape.

  \ingroup kernel
  \ingroup shared

  This class is used for mainly two things; to create mouse cursors to be
  associated with widgets and to get and set the position of the mouse
  cursor.

  Qt has a number of standard cursor shapes, but you can also make
  custom cursor shapes based on a \link QBitmap bitmap \endlink, a
  mask and a hotspot.

  To associate a cursor with a widget, use QWidget::setCursor().
  To associate a cursor with all widgets (maybe for a short period of time),
  use QApplication::setOverrideCursor().

  The <a name=cursors>predefined cursor</a> objects are:
  <ul>
  <li> arrowCursor : standard arrow cursor
  <li> upArrowCursor : upwards arrow
  <li> crossCursor : crosshair
  <li> waitCursor : hourglass/watch
  <li> ibeamCursor : ibeam/text entry
  <li> sizeVerCursor : vertical resize
  <li> sizeHorCursor : horizontal resize
  <li> sizeBDiagCursor : diagonal resize (/)
  <li> sizeFDiagCursor : diagonal resize (\)
  <li> sizeAllCursor : all directions resize
  <li> blankCursor : blank/invisible cursor
  </ul>
  
  \sa QWidget
  <a href="guibooks.html#fowler">GUI Design Handbook: Cursors.</a>
*/


/*****************************************************************************
  QCursor stream functions
 *****************************************************************************/

/*!
  \relates QCursor
  Writes a cursor to the stream.
*/

QDataStream &operator<<( QDataStream &s, const QCursor &c )
{
    s << (INT16)c.shape();			// write shape id to stream
    if ( c.shape() == BitmapCursor ) {		// bitmap cursor
	s << *c.bitmap() << *c.mask();
	s << c.hotSpot();
    }
    return s;
}

/*!
  \relates QCursor
  Reads a cursor from the stream.
*/

QDataStream &operator>>( QDataStream &s, QCursor &c )
{
    INT16 shape;
    s >> shape;					// read shape id from stream
    if ( shape == BitmapCursor ) {		// read bitmap cursor
	QBitmap bm, bmm;
	QPoint	hot;
	s >> bm >> bmm >> hot;
	c = QCursor( bm, bmm, hot.x(), hot.y() );
    } else {
	c.setShape( (int)shape );		// create cursor with shape
    }
    return s;
}
