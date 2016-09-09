/****************************************************************************
** $Id: qclipboard.cpp,v 2.10 1998/07/03 00:09:30 hanord Exp $
**
** Implementation of QClipboard class
**
** Created : 960430
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

#include "qclipboard.h"
#include "qapplication.h"
#include "qpixmap.h"

/*!
  \class QClipboard qclipboard.h
  \brief The QClipboard class provides access to the window system clipboard.

  \ingroup kernel

  The clipboard offers a simple mechanism to copy and paste data between
  applications.

  QClipboard supports these formats (a format is identified by a string):
  <ul>
  <li>"TEXT", zero-terminated char *.
  <li>"PIXMAP" as provided by QPixmap.
  </ul>

  The "PIXMAP" format is not implemented in this version of Qt.

  Only a single QClipboard object may exist in an application. This is
  because QClipboard is a shared window system resource. It is not
  possible to create a QClipboard object the standard C++ way (the
  constructor and destructor are private member functions, but accessible
  to QApplication since it is a friend class).	Call
  QApplication::clipboard() to access the clipboard.

  Example:
  \code
    QClipboard *cb = QApplication::clipboard();
    const char *text;

    // Copy text from the clipboard (paste)
    text = cb->text();
    if ( text )
	debug( "The clipboard contains: %s", text );

    // Copy text into the clipboard
    cb->setText( "This text can be pasted by other programs" );
  \endcode
*/


/*!
  Constructs a clipboard object.

  Note that only QApplication is allowed to do this. Call
  QApplication::clipboard() to get a pointer to the application global
  clipboard object.
*/

QClipboard::QClipboard( QObject *parent, const char *name )
    : QObject( parent, name )
{
    // nothing
}

/*!
  Destroys the clipboard.

  You should never delete the clipboard. QApplication will do this when
  the application terminates.
*/

QClipboard::~QClipboard()
{
}


/*!
  \fn void QClipboard::dataChanged()
  This signal is emitted when the clipboard data is changed.
*/


/*!
  Returns the clipboard text, or null if the clipboard does not contains
  any text.
  \sa setText()
*/

const char *QClipboard::text() const
{
    return (const char *)data("TEXT");
}

/*!
  Copies \e text into the clipboard.
  \sa text()
*/

void QClipboard::setText( const char *text )
{
    setData( "TEXT", (void *)text );
}


/*!
  Returns the clipboard pixmap, or null if the clipboard does not contains
  any pixmap.
  \sa setText()
*/

QPixmap *QClipboard::pixmap() const
{
    return (QPixmap *)data("PIXMAP");
}

/*!
  Copies \e pixmap into the clipboard.
  \sa pixmap()
*/

void QClipboard::setPixmap( const QPixmap &pixmap )
{
    setData( "PIXMAP", (void *)&pixmap );
}


/*****************************************************************************
  QApplication member functions related to QClipboard.
 *****************************************************************************/

extern QObject *qt_clipboard;			// defined in qapp_xyz.cpp

static void cleanupClipboard()
{
    delete qt_clipboard;
    qt_clipboard = 0;
}

/*!
  Returns a pointer to the application global clipboard.
*/

QClipboard *QApplication::clipboard()
{
    if ( qt_clipboard == 0 ) {
	qt_clipboard = new QClipboard;
	CHECK_PTR( qt_clipboard );
	qAddPostRoutine( cleanupClipboard );
    }
    return (QClipboard *)qt_clipboard;
}
