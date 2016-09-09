/****************************************************************************
** $Id: qtoolbar.cpp,v 2.25.2.1 1998/08/12 16:55:16 agulbra Exp $
**
** Implementation of QToolBar class
**
** Created : 980315
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

#include "qtoolbar.h"

#include "qmainwindow.h"
#include "qpushbutton.h"
#include "qtooltip.h"
#include "qlayout.h"
#include "qframe.h"
#include "qobjectlist.h"
#include "qobjectdict.h"
#include "qpainter.h"
#include "qdrawutil.h"

/*! \class QToolBar qtoolbar.h

  \brief The QToolBar class provides a simple tool bar.

  \ingroup realwidgets
  \ingroup application

  To use QToolBar, you simply create a QToolBar as child of a
  QMainWindow, create a number of QToolButton widgets (or other
  widgets) in left to right (or top to bottom) order, call
  addSeparator() when you want a separator, and that's all.

  The application/application.cpp example does precisely this.

  Each QToolBar lives in a \link QMainWindow dock \endlink in a
  QMainWindow, and can optionally start a new line in its dock.  Tool
  bars that start a new line are always positioned at the left end or
  top of the tool bar dock; others are placed next to the previous
  tool bar and word-wrapped as necessary.

  The tool bar is able to arrange its buttons horizontally or
  vertically (see setOrientation() for details) and draws the
  appropriate frames around the tool button in Windows and Motif
  style.  Generally, QMainWindow will set the orientation correctly
  for you.

  \sa QToolButton QMainWindow
  <a href="http://www.iarchitect.com/visual.htm">Parts of Isys on Visual Design,</a>
  <a href="http://www.microsoft.com/win32dev/uiguide/uigui192.htm">Microsoft Style Guide,</a>
  <a href="http://www.microsoft.com/win32dev/uiguide/uigui196.htm">some common buttons (NB: read the license),</a>
  <a href="guibooks.html#fowler">GUI Design Handbook: Tool Bar.</a>
*/


/*!  Constructs an empty tool bar which is a chilf od \a parent and
  managed by \a parent, initially in \a dock, labelled \a and starting
  a new line in the dock if \a newLine is TRUE.  \a name is the object
  name, as usual.
*/

QToolBar::QToolBar( const char * label,
		    QMainWindow * parent, QMainWindow::ToolBarDock dock,
		    bool newLine, const char * name )
    : QWidget( parent, name )
{
    d = 0;
    b = 0;
    mw = parent;
    sw = 0;
    o = (dock == QMainWindow::Left || dock == QMainWindow::Right )
	? Vertical : Horizontal;
    parent->addToolBar( this, label, dock, newLine );
}


/*!  Constructs an empty horizontal tool bar which is a parent of \a
  parent and managed by \a mainWindow.  The \a label and \a newLine
  are passed straight to QMainWindow::addToolBar().  \a name is the
  object name and \a f is the widget flags.

  This is the constructor to use if you want to create torn-off
  toolbars, or toolbars in the status bar.
*/

QToolBar::QToolBar( const char * label, QMainWindow * mainWindow,
		    QWidget * parent, bool newLine, const char * name,
		    WFlags f )
    : QWidget( parent, name, f )
{
    d = 0;
    b = 0;
    mw = mainWindow;
    sw = 0;
    o = Horizontal;
    mainWindow->addToolBar( this, label, QMainWindow::Unmanaged, newLine );
}


/*!  Constructs an empty tool bar in the top dock of its parent,
  without any label and without requiring a newline.  This is mostly
  useless. */

QToolBar::QToolBar( QMainWindow * parent, const char * name )
    : QWidget( parent, name )
{
    d = 0;
    b = 0;
    o = Horizontal;
    sw = 0;
    mw = parent;
    parent->addToolBar( this, 0, QMainWindow::Top );
}


/*! Destroys the object and frees any allocated resources. */

QToolBar::~QToolBar()
{
    delete b;
    b = 0;
    // delete d; as soon as there is a d
}


/*!  Adds a separator in here.  Cool, man. */

void QToolBar::addSeparator()
{
    QFrame * f = new QFrame( this, "tool bar separator" );
    f->setFrameStyle( QFrame::NoFrame ); // old-style whatevers
}


/*!  Sets this toolbar to organize its content vertically if \a
  newOrientation is \c Vertical and horizontally if \a newOrientation
  is \c Horizontal.
*/

void QToolBar::setOrientation( Orientation newOrientation )
{
    if ( o != newOrientation ) {
	o = newOrientation;
	setUpGM();
    }
}


/*! \fn QToolBar::Orientation QToolBar::orientation() const

  Returns the current orientation of the toolbar.

*/

/*!  Reimplemented to set up geometry management. */

void QToolBar::show()
{
    setUpGM();
    QWidget::show();
}


/*!  Sets up geometry management for this toolbar. */

void QToolBar::setUpGM()
{
    delete b;
    b = new QBoxLayout( this, orientation() == Vertical
			? QBoxLayout::Down : QBoxLayout::LeftToRight,
			style() == WindowsStyle ? 2 : 1, 0 );

    b->addSpacing( 9 );

    const QObjectList * c = children();
    QObjectListIt it( *c );
    QObject *obj;
    while( (obj=it.current()) != 0 ) {
	++it;
	if ( obj->isWidgetType() ) {
	    QWidget * w = (QWidget *)obj;
	    if ( !qstrcmp( "tool bar separator", obj->name() ) &&
		 !qstrcmp( "QFrame", obj->className() ) ) {
		QFrame * f = (QFrame *)obj;
		if ( orientation() == Vertical ) {
		    f->setMinimumSize( 0, 6 );
		    f->setMaximumSize( 32767, 6 );
		    if ( style() == WindowsStyle )
			f->setFrameStyle( QFrame::HLine + QFrame::Sunken );
		    else
			f->setFrameStyle( QFrame::NoFrame );
		} else {
		    f->setMinimumSize( 6, 0 );
		    f->setMaximumSize( 6, 32767 );
		    if ( style() == WindowsStyle )
			f->setFrameStyle( QFrame::VLine + QFrame::Sunken );
		    else
			f->setFrameStyle( QFrame::NoFrame );
		}
	    } else if ( w->maximumSize() == QSize(32767,32767)
			&& w->minimumSize() == QSize(0,0) ) {
		QSize s( w->sizeHint() );
		if ( s.width() > 0 && s.height() > 0 )
		    w->setMinimumSize( s );
		else if ( s.width() > 0 )
		    w->setMinimumWidth( s.width() );
		else if ( s.height() > 0 )
		    w->setMinimumHeight( s.width() );
	    }
	    b->addWidget( w, w == sw ? 42 : 0 );
	}
    }
    b->activate();
}


/*!  Paint the handle.  The Motif style is rather close to Netscape
  and even closer to KDE. */

void QToolBar::paintEvent( QPaintEvent * )
{
    QPainter p( this );
    qDrawShadePanel( &p, 0, 0, width(), height(),
		     colorGroup(), FALSE, 1, 0 );
    if ( style() == MotifStyle ) {
	QColor dark( colorGroup().dark() );
	QColor light( colorGroup().light() );
	unsigned int i;
	if ( orientation() == Vertical ) {
	    QPointArray a( 2 * ((width()-6)/3) );
	    int x = 3 + (width()%3)/2;
	    p.setPen( dark );
	    p.drawLine( 1, 8, width()-2, 8 );
	    for( i=0; 2*i < a.size(); i ++ ) {
		a.setPoint( 2*i, x+1+3*i, 6 );
		a.setPoint( 2*i+1, x+2+3*i, 3 );
	    }
	    p.drawPoints( a );
	    p.setPen( light );
	    p.drawLine( 1, 9, width()-2, 9 );
	    for( i=0; 2*i < a.size(); i++ ) {
		a.setPoint( 2*i, x+3*i, 5 );
		a.setPoint( 2*i+1, x+1+3*i, 2 );
	    }
	    p.drawPoints( a );
	} else {
	    QPointArray a( 2 * ((height()-6)/3) );
	    int y = 3 + (height()%3)/2;
	    p.setPen( dark );
	    p.drawLine( 8, 1, 8, height()-2 );
	    for( i=0; 2*i < a.size(); i ++ ) {
		a.setPoint( 2*i, 5, y+1+3*i );
		a.setPoint( 2*i+1, 2, y+2+3*i );
	    }
	    p.drawPoints( a );
	    p.setPen( light );
	    p.drawLine( 9, 1, 9, height()-2 );
	    for( i=0; 2*i < a.size(); i++ ) {
		a.setPoint( 2*i, 4, y+3*i );
		a.setPoint( 2*i+1, 1, y+1+3*i );
	    }
	    p.drawPoints( a );
	}
    } else {
	if ( orientation() == Vertical ) {
	    qDrawShadePanel( &p, 2, 4, width() - 4, 3,
			     colorGroup(), FALSE, 1, 0 );
	    qDrawShadePanel( &p, 2, 7, width() - 4, 3,
			     colorGroup(), FALSE, 1, 0 );
	} else {
	    qDrawShadePanel( &p, 4, 2, 3, height() - 4,
			     colorGroup(), FALSE, 1, 0 );
	    qDrawShadePanel( &p, 7, 2, 3, height() - 4,
			     colorGroup(), FALSE, 1, 0 );
	}
    }
}



/*!  Returns a pointer to the QMainWindow which controls this tool bar.
*/

QMainWindow * QToolBar::mainWindow()
{
    return mw;
}


/*!  Sets \a w to be expanded if this toolbar is requested to stretch
  (because QMainWindow right-justifies the dock it's in).
*/

void QToolBar::setStretchableWidget( QWidget * w )
{
    sw = w;
}


/*! \reimp */

bool QToolBar::event( QEvent * e )
{
    if ( e->type() == Event_LayoutHint )
	setUpGM();
    return QWidget::event( e );
}
