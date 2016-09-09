/****************************************************************************
** $Id: qwidgetstack.cpp,v 2.11.2.3 1998/09/28 17:13:56 warwick Exp $
**
** Implementation of QWidgetStack class
**
** Created : 980128
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

#include "qwidgetstack.h"

#include "qobjectlist.h"
#include "qobjectdict.h"
#include "qlayout.h"

class QWidgetStackPrivate {
};


/*! \class QWidgetStack qwidgetstack.h

  \brief The QWidgetStack class provides a stack of widgets, where the
  user can see only the top widget.

  \ingroup misc

  This is often used e.g. in tab and wizard dialogs.

  The application programmer can move any widget to the top of the
  stack at any time using the slot raiseWidget(), and add or remove
  widgets using addWidget() and removeWidget().

  visibleWidget() is the \e get equivalent of raiseWidget(); it
  returns a pointer ot the widget that is currently on the top of the
  stack.

  QWidgetStack also provides the ability to manipulate widgets through
  application-specfied integer IDs, and to translate from widget
  pointers to IDs using id() and from IDs to widget pointers using
  widget().  These numeric IDs have and unique (per QWidgetStack, not
  globally) and cannot be -1, but apart from that QWidgetStack does
  not attach any meaning to them.

  The default widget stack is frame-less and propagates its font and
  palette to all its children, but you can use the usual QFrame
  functions (like setFrameStyle()) to add a frame, and use
  setFontPropagation() and setPalettePropagation() to change the
  propagation style.

  Finally, QWidgetStack provides a signal, aboutToShow(), which is
  emitted just before a managed widget is shown.

  \sa QTabDialog QTabBar QFrame
*/


/*!  Constructs an empty widget stack. */

QWidgetStack::QWidgetStack( QWidget * parent, const char * name )
    : QFrame( parent, name )
{
    d = 0;
    dict = new QIntDict<QWidget>;
    l = 0;
    topWidget = 0;
    setFontPropagation( AllChildren );
    setPalettePropagation( AllChildren );
}


/*! Destroys the object and frees any allocated resources. */

QWidgetStack::~QWidgetStack()
{
    delete d;
    delete dict;
}


/*!  Adds \a w to this stack of widgets, with id \a id.

  If \a w is not a child of \c this, QWidgetStack moves it using
  recreate().
*/

void QWidgetStack::addWidget( QWidget * w, int id )
{
    dict->insert( id+1, w );
    if ( w->parent() != this )
	w->recreate( this, 0, QPoint(0,0), FALSE );
    setChildGeometries();
}


/*!  Removes \a w from this stack of widgets.  Does not delete \a
  w. If \a w is the currently visible widget, no other widget is
  substituted. \sa visibleWidget() raiseWidget() */

void QWidgetStack::removeWidget( QWidget * w )
{
    if ( !w )
	return;
    int i = id( w );
    if ( i != -1 )
	dict->take( i+1 );
    if ( w == topWidget )
	topWidget = 0;
}


/*!  Raises \a id to the top of the widget stack. \sa visibleWidget() */

void QWidgetStack::raiseWidget( int id )
{
    if ( id == -1 )
	return;
    QWidget * w = dict->find( id+1 );
    if ( w )
	raiseWidget( w );
}


/*!  Raises \a w to the top of the widget stack. */

void QWidgetStack::raiseWidget( QWidget * w )
{
    if ( !w || !isMyChild( w ) )
	return;

    topWidget = w;
    if ( !isVisible() )
	return;

    emit aboutToShow( w );
    if ( receivers( SIGNAL(aboutToShow(int)) ) ) {
	// ### O(n)
	int i = id( w );
	if ( i )
	    emit aboutToShow( i );
    }
    w->show();

    // try to move focus onto the incoming widget if focus
    // was somewhere on the outgoing widget.
    QWidget * f = w->focusWidget();
    while ( f && f->parent() != this )
	f = f->parentWidget();
    if ( f && f->parent() == this ) {
	if ( w->focusPolicy() != QWidget::NoFocus ) {
	    w->setFocus();
	} else {
	    bool done = FALSE;
	    const QObjectList * c = w->children();
	    if ( c ) {
		QObjectListIt it( *c );
		QObject * wc;
		while( !done && (wc=it.current()) != 0 ) {
		    ++it;
		    if ( wc->isWidgetType() ) {
			f = (QWidget *)wc;
			if ( f->focusPolicy() == QWidget::StrongFocus ||
			     f->focusPolicy() == QWidget::TabFocus ) {
			    f->setFocus();
			    done = TRUE;
			}
		    }
		}
	    }
	}
    }

    const QObjectList * c = children();
    QObjectListIt it( *c );
    QObject * o;

    while( (o=it.current()) != 0 ) {
	++it;
	if ( o->isWidgetType() && o != w )
	    ((QWidget *)o)->hide();
    }
}


/*!  Returns TRUE if \a w is a child of this widget, else FALSE. */

bool QWidgetStack::isMyChild( QWidget * w )
{
    const QObjectList * c = children();
    if ( !c )
	return FALSE;
    QObjectListIt it( *c );
    QObject * o;

    while( (o=it.current()) != 0 ) {
	++it;
	if ( o->isWidgetType() && o == w )
	    return TRUE;
    }
    return FALSE;
}


/*! Reimpelemented in order to set the children's geometries
  appropriately. */

void QWidgetStack::frameChanged()
{
    QFrame::frameChanged();
    setChildGeometries();
}


/*!  Fix up the children's geometries. */

void QWidgetStack::setChildGeometries()
{
    delete l;
    l = new QGridLayout( this, 3, 3 );
    if ( frameWidth() ) {
	l->addRowSpacing( 0, frameWidth() );
	l->addRowSpacing( 2, frameWidth() );
	l->addColSpacing( 0, frameWidth() );
	l->addColSpacing( 2, frameWidth() );
    }
    l->setRowStretch( 1, 1 );
    l->setColStretch( 1, 1 );

    const QObjectList * c = children();
    if ( c ) {
	QObjectListIt it( *c );
	QObject * o;
	
	while( (o=it.current()) != 0 ) {
	    ++it;
	    if ( o->isWidgetType() ) {
		l->addWidget( (QWidget *)o, 1, 1 );
	    }
	}
    }
    l->activate();
}


/*!  Reimplemented in order to set the children's geometries
  appropriately. */

void QWidgetStack::show()
{
    if ( !isVisible() && children() ) {
	const QObjectList * c = children();
	QObjectListIt it( *c );
	QObject * o;

	while( (o=it.current()) != 0 ) {
	    ++it;
	    if ( o->isWidgetType() && o != topWidget )
		((QWidget *)o)->hide();
	}
    }

    setChildGeometries();
    QFrame::show();
}


/*!  Returns a pointer to the widget with ID \a id.  If this widget
  stack does not manage a widget with ID \a id, this function return
  0.

  \sa id() addWidget()
*/

QWidget * QWidgetStack::widget( int id ) const
{
    return id != -1 ? dict->find( id+1 ) : 0;
}


/*!  Returns the ID of the \a widget.  If \a widget is 0 or is not
  being managed by this widget stack, this function returns -1.

  \sa widget() addWidget()
*/

int QWidgetStack::id( QWidget * widget ) const
{
    if ( !widget || !dict )
	return -1;

    QIntDictIterator<QWidget> it( *dict );
    while ( it.current() && it.current() != widget )
	++it;
    return it.current() == widget ? it.currentKey()-1 : -1;
}


/*! Returns a pointer to the currently visible widget (the one on the
  top of the stack), of 0 if nothing is currently being shown.

  \sa aboutToShow() id() raiseWidget()
*/

QWidget * QWidgetStack::visibleWidget() const
{
    return topWidget;
}


/*! \fn void QWidgetStack::aboutToShow( int )

  This signal is emitted just before a managed widget is shown, if
  that managed widget has a non-zero ID.  The argument is the numeric
  ID of the widget.
*/


/*! \fn void QWidgetStack::aboutToShow( QWidget * )

  This signal is emitted just before a managed widget is shown.  The
  argument is a pointer to the widget.
*/


/*! \reimp */

bool QWidgetStack::event( QEvent * e )
{
    if ( e->type() == Event_ChildInserted )
	setChildGeometries();
    return QFrame::event( e );
}
