/****************************************************************************
** $Id: qfocusdata.cpp,v 2.2.2.1 1998/08/12 16:55:07 agulbra Exp $
**
** Implementation of QFocusData class
**
** Created : 980622
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

#include "qfocusdata.h"

/*!
  \class QFocusData qfocusdata.h
  \brief Maintains the list of widgets which can take focus.

  This read-only list always contains at least one widget (the
  top-level widget, actually).  It provides a simple cursor, which can
  be reset to the current focus widget using home(), or moved to its
  neighboring widgets using next() and prev(), and a count() of
  widgets in the list.
  
  Note that some widgets in the list may not accept focus.  Widgets
  are added to the list as necessary, but not removed from it.  This
  lets widgets change focus policy dynamically without disrupting the
  focus chain the user sees: When a widget disables and re-enables tab
  focus, its position in the focus chain does not change.

  When reimplementing QWidget::focusNextPrevChild() to provide special
  focus flow, you will usually call QWidget::focusData() to retrieve
  the focus data stored at the top-level widget - the focus data for
  that hierarchy of widgets.
  
  The cursor may change at any time; this class is not thread-safe.
  
  \sa QWidget::focusNextPrevChild() QWidget::setTabOrder()
  QWidget::setFocusPolicy()
*/

/*!
  \fn QWidget* QFocusData::focusWidget() const

  Returns the widgets in the hierarchy which currently has focus.
*/

/*!
  \fn int QFocusData::count() const

  Returns a count of the number of widgets in the hierarchy which accept focus.
*/

/*!
  Moves the cursor to the focusWidget() and returns that widget.
  You must call this before next() or prev() to iterate meaningfully.
*/
QWidget* QFocusData::home()
{
    focusWidgets.find(it.current());
    return focusWidgets.current();
}

/*!
  Moves the cursor to the right.  Note that the focus widgets
  are a \e loop of widgets.  If you keep calling next(), it will
  loop, without ever returning 0.
*/
QWidget* QFocusData::next()
{
    QWidget* r = focusWidgets.next();
    if ( !r )
	r = focusWidgets.first();
    return r;
}

/*!
  Moves the cursor to the left.  Note that the focus widgets
  are a \e loop of widgets.  If you keep calling prev(), it will
  loop, without ever returning 0.
*/
QWidget* QFocusData::prev()
{
    QWidget* r = focusWidgets.prev();
    if ( !r )
	r = focusWidgets.last();
    return r;
}
