/****************************************************************************
** $Id: qwindow.cpp,v 2.4 1998/07/03 00:09:42 hanord Exp $
**
** Implementation of QWindow class
**
** Created : 931211
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

#include "qwindow.h"
#include "qpixmap.h"

/*!
  \class QWindow qwindow.h
  \brief The QWindow class is reserved for future extensions.

  This class is not yet finished.  It will contain intelligent handling of
  menus etc. in a future Qt release.
*/


/*!
  Constructs a window named \e name, which will be a child widget of
  \e parent.

  The widget flags \e f should normally be set to zero unless you know what you
  are doing.

  These arguments are sent to the QWidget constructor.
*/

QWindow::QWindow( QWidget *parent, const char *name, WFlags f )
    : QWidget( parent, name, f )
{
    // nothing
}

/*!
  Destroys the window.
*/

QWindow::~QWindow()
{
}
