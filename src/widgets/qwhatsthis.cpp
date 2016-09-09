/****************************************************************************
** $Id: qwhatsthis.cpp,v 2.16.2.2 1999/02/01 10:44:12 hanord Exp $
**
** Implementation of QWhatsThis class
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

#include "qwhatsthis.h"

#include "qapplication.h"
#include "qpaintdevicemetrics.h"
#include "qpixmap.h"
#include "qpainter.h"
#include "qtimer.h"
#include "qptrdict.h"
#include "qtoolbutton.h"
#include "qshared.h"
#include "qkeycode.h"
#include "qcursor.h"
#include "qbitmap.h"

/*! \class QWhatsThis qwhatsthis.h

  \brief The QWhatsThis class provides a simple description of any
  widget, e.g. answering the question "what's this?"

  \ingroup application

  What's This help lies between tool tips and fully-blown online help
  systems: <ul><li> Tool Tips - flyweight help, extremely brief,
  entirely integrated in the user interface. <li> What's This? - also
  lightweight, but can encompass a three-paragraph explanation.  <li>
  Online Help - can encompass any amount of information, but is
  typically a little slower to call up, a little separated from the
  user's work, and often users feel that using online help is a
  digression from their real task. </ul>

  QWhatsThis, then, offers a single window with a single explanatory
  text, which pops up quickly when the user asks "what's this?", and
  goes away as soon as the user does something else.  There are two
  ways to make QWhatsThis pop up: Click a "What's This?" button and
  then click on some other widget to get help for that other widget,
  or press Shift-F1 to get help for the widget that has keyboard
  focus.

  QWhatsThis provides functions to add() and remove() What's This help
  for a widget, and it provides a function to create a What's This
  button suitable for typical tool bars.

  <img src="whatsthis.gif" width="376" height="239">

  More functionality will be provided in the coming releases of Qt.

  \sa QToolTip
*/

class QWhatsThisPrivate: public QObject
{
public:
    // a special button
    struct Button: public QToolButton
    {
	Button( QWidget * parent, const char * name );
	~Button();

	// reimplemented because of QButton's lack of virtuals
	void mouseReleaseEvent( QMouseEvent * );
	
	// reimplemented because, well, because I'm evil.
	const char *className() const;
    };

    // an item for storing texts
    struct Item: public QShared
    {
	Item(): QShared(), dc(FALSE), s(0), t(0) {}
	~Item();
	bool dc;
	char * s;
	char * t;
	QPixmap pm;
    };

    // the state machine
    enum State { Inactive, Waiting, Displaying, FinalPress };

    QWhatsThisPrivate();
    ~QWhatsThisPrivate();

    bool eventFilter( QObject *, QEvent * );

    // say it.
    void say( QWidget *, const char * );

    // setup and teardown
    static void tearDownWhatsThis();
    static void setUpWhatsThis();

    // variables
    QWidget * whatsThat;
    QPtrDict<Item> * dict;
    QPtrDict<QWidget> * tlw;
    QPtrDict<Button> * buttons;
    State state;

    QCursor * cursor;
};


// static, but static the less-typing way
static QWhatsThisPrivate * wt = 0;


// the item
QWhatsThisPrivate::Item::~Item()
{
    if ( count )
	fatal( "Internal error #10%d in What's This", count );
    if ( dc && s )
	delete[] (char*)s;
}


static const char * button_image[] = {
"16 16 3 1",
" 	c None",
"o	c #000000",
"a	c #000080",
"o        aaaaa  ",
"oo      aaa aaa ",
"ooo    aaa   aaa",
"oooo   aa     aa",
"ooooo  aa     aa",
"oooooo  a    aaa",
"ooooooo     aaa ",
"oooooooo   aaa  ",
"ooooooooo aaa   ",
"ooooo     aaa   ",
"oo ooo          ",
"o  ooo    aaa   ",
"    ooo   aaa   ",
"    ooo         ",
"     ooo        ",
"     ooo        "};

#define cursor_bits_width 32
#define cursor_bits_height 32
static unsigned char cursor_bits_bits[] = {
  0x01, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x05, 0xf0, 0x07, 0x00,
  0x09, 0x18, 0x0e, 0x00, 0x11, 0x1c, 0x0e, 0x00, 0x21, 0x1c, 0x0e, 0x00,
  0x41, 0x1c, 0x0e, 0x00, 0x81, 0x1c, 0x0e, 0x00, 0x01, 0x01, 0x07, 0x00,
  0x01, 0x82, 0x03, 0x00, 0xc1, 0xc7, 0x01, 0x00, 0x49, 0xc0, 0x01, 0x00,
  0x95, 0xc0, 0x01, 0x00, 0x93, 0xc0, 0x01, 0x00, 0x21, 0x01, 0x00, 0x00,
  0x20, 0xc1, 0x01, 0x00, 0x40, 0xc2, 0x01, 0x00, 0x40, 0x02, 0x00, 0x00,
  0x80, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, };

#define cursor_mask_width 32
#define cursor_mask_height 32
static unsigned char cursor_mask_bits[] = {
  0x01, 0x00, 0x00, 0x00, 0x03, 0xf0, 0x07, 0x00, 0x07, 0xf8, 0x0f, 0x00,
  0x0f, 0xfc, 0x1f, 0x00, 0x1f, 0x3e, 0x1f, 0x00, 0x3f, 0x3e, 0x1f, 0x00,
  0x7f, 0x3e, 0x1f, 0x00, 0xff, 0x3e, 0x1f, 0x00, 0xff, 0x9d, 0x0f, 0x00,
  0xff, 0xc3, 0x07, 0x00, 0xff, 0xe7, 0x03, 0x00, 0x7f, 0xe0, 0x03, 0x00,
  0xf7, 0xe0, 0x03, 0x00, 0xf3, 0xe0, 0x03, 0x00, 0xe1, 0xe1, 0x03, 0x00,
  0xe0, 0xe1, 0x03, 0x00, 0xc0, 0xe3, 0x03, 0x00, 0xc0, 0xe3, 0x03, 0x00,
  0x80, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, };



// the button class
QWhatsThisPrivate::Button::Button( QWidget * parent, const char * name )
    : QToolButton( parent, name )
{
    QPixmap p( button_image );
    setPixmap( p );
    setToggleButton( TRUE );
    wt->buttons->insert( (void *)this, this );
}


QWhatsThisPrivate::Button::~Button()
{
    if ( wt && wt->buttons )
	wt->buttons->take( (void *)this );
}


void QWhatsThisPrivate::Button::mouseReleaseEvent( QMouseEvent * e )
{
    QToolButton::mouseReleaseEvent( e );
    if ( isOn() ) {
	setUpWhatsThis();
	QApplication::setOverrideCursor( *wt->cursor, FALSE );
	wt->state = Waiting;
	qApp->installEventFilter( wt );
    }
}


const char *QWhatsThisPrivate::Button::className() const
{
    return "QWhatsThisPrivate::Button";
}


// the what's this manager class
QWhatsThisPrivate::QWhatsThisPrivate()
    : QObject( 0, "global what's this object" )
{
    qAddPostRoutine( tearDownWhatsThis );
    whatsThat = 0;
    dict = new QPtrDict<QWhatsThisPrivate::Item>;
    tlw = new QPtrDict<QWidget>;
    wt = this;
    buttons = new QPtrDict<Button>;
    state = Inactive;
    cursor = new QCursor( QBitmap( cursor_bits_width, cursor_bits_height,
				   cursor_bits_bits, TRUE ),
			  QBitmap( cursor_mask_width, cursor_mask_height,
				   cursor_mask_bits, TRUE ),
			  1, 1 );
}

QWhatsThisPrivate::~QWhatsThisPrivate()
{
    if ( state == Waiting )
	QApplication::restoreOverrideCursor();

    // the two straight-and-simple dicts
    delete tlw;
    delete buttons;

    // then delete the complex one.
    QPtrDictIterator<Item> it( *dict );
    Item * i;
    QWidget * w;
    while( (i=it.current()) != 0 ) {
	w = (QWidget *)it.currentKey();
	++it;
	dict->take( w );
	i->deref();
	if ( !i->dc || !i->count )
	    delete i;
    }
    delete dict;
    delete cursor;
    // and finally lose wt
    wt = 0;
}

bool QWhatsThisPrivate::eventFilter( QObject * o, QEvent * e )
{
    if ( !o || !e )
	return FALSE;

    switch( state ) {
    case FinalPress:
	if( e->type() == Event_MouseButtonRelease ) {
	    state = Inactive;
	    qApp->removeEventFilter( this );
	    if ( whatsThat )
		whatsThat->hide();
	    return TRUE;
	} else if ( e->type() == Event_MouseMove ) {
	    return TRUE;
	}
	break;
    case Displaying:
	if ( e->type() == Event_MouseButtonPress ) {
	    if ( !qstrcmp( "QWhatsThisPrivate::Button", o->className() ) ) {
		state = Inactive;
		qApp->removeEventFilter( this );
	    } else {
		state = FinalPress;
	    }
	    if ( whatsThat )
		whatsThat->hide();
	    return TRUE;
	} else if ( e->type() == Event_MouseButtonRelease ||
		    e->type() == Event_MouseMove ) {
	    return TRUE;
	} else if ( e->type() == Event_Accel ) {
	    if ( whatsThat )
		whatsThat->hide();
	    ((QKeyEvent *)e)->accept();
	    state = Inactive;
	    qApp->removeEventFilter( this );
	} else if ( e->type() == Event_FocusOut ||
		    e->type() == Event_FocusIn ) {
	    if ( whatsThat )
		whatsThat->hide();
	    state = Inactive;
	    qApp->removeEventFilter( this );
	}
	break;
    case Waiting:
	if ( e->type() == Event_MouseButtonPress && o->isWidgetType() ) {
	    QWidget * w = (QWidget *) o;
	    QWhatsThisPrivate::Item * i = 0;
	    while( w && !i ) {
		i = dict->find( w );
		if ( !i )
		    w = w->parentWidget();
	    }
	    QPtrDictIterator<Button> it( *(wt->buttons) );
	    Button * b;
	    while( (b=it.current()) != 0 ) {
		++it;
		b->setOn( FALSE );
	    }
	    if ( i ) {
		state = Displaying;
		say( w, i->s );
	    } else {
		state = FinalPress;
	    }
	    QApplication::restoreOverrideCursor();
	    return TRUE;
	} else if ( e->type() == Event_MouseButtonPress ||
		    e->type() == Event_MouseMove ) {
	    return TRUE;
	} else if (e->type() == Event_Accel ||
		   e->type() == Event_KeyPress ) {
	    QPtrDictIterator<Button> it( *(wt->buttons) );
	    Button * b;
	    while( (b=it.current()) != 0 ) {
		++it;
		b->setOn( FALSE );
	    }
	    QApplication::restoreOverrideCursor();
	    state = Inactive;
	    qApp->removeEventFilter( this );
	}
	break;
    case Inactive:
	if ( e->type() == Event_Accel &&
	     ((QKeyEvent *)e)->key() == Key_F1 &&
	     !o->parent() &&
	     o->isWidgetType() &&
	     ((QKeyEvent *)e)->state() == ShiftButton ) {
	    QWidget * w = ((QWidget *)o)->focusWidget();
	    QWhatsThisPrivate::Item * i = 0;
	    if ( w && (i=dict->find( w )) != 0 && i->s ) {
		say( w, i->s );
		state = Displaying;
		qApp->installEventFilter( this );
		((QKeyEvent *)e)->accept();
		return TRUE;
	    }
	}
	break;
    }
    return FALSE;
}



void QWhatsThisPrivate::setUpWhatsThis()
{
    if ( !wt )
	wt = new QWhatsThisPrivate();
}


void QWhatsThisPrivate::tearDownWhatsThis()
{
    delete wt;
    wt = 0;
}


void QWhatsThisPrivate::say( QWidget * widget, const char * text )
{
    const int shadowWidth = 6;   // also used as '5' and '6' and even '8' below
    const int normalMargin = 12; // *2
    const int leftMargin = 18;   // *3

    QWidget * desktop = QApplication::desktop();

    int w = desktop->width() / 3;
    if ( w < 200 )
	w = 200;
    else if ( w > 300 )
	w = 300;

    QPixmap pm( 1,1 );
    QPainter p( &pm );
    QRect r = p.boundingRect( 0, 0, w, 1000,
			      AlignLeft + AlignTop + WordBreak + ExpandTabs,
			      text );
    p.end();
    int h = r.height() + normalMargin + normalMargin;
    w = w + leftMargin + normalMargin;

    // okay, now to find a suitable location

    QPoint pos = widget->mapToGlobal( QPoint( 0,0 ) );
    int x;

    // first try locating the widget immediately above/below,
    // with nice alignment if possible.
    if ( w > widget->width() + 16 )
	x = pos.x() + widget->width()/2 - w/2;
    else
	x = pos.x();

    // squeeze it in if that would result in part of what's this
    // being only partially visible
    if ( x + w > QApplication::desktop()->width() )
	x = QMIN(QApplication::desktop()->width(),
		 pos.x() + widget->width())
	    - w;

    if ( x < 0 )
	x = 0;

    int y =pos.y() + widget->height() + 2; // below, two pixels spacing
    // what's this is above or below, wherever there's most space
    if ( y + h + 10 > QApplication::desktop()->height() )
	y = pos.y() + 2 - shadowWidth - h; // above, overlap

#if 0
    // should try to fit the whats this widget onto the same
    // top-level widget here, if possible.

    // if there wasn't enough space either above or below, try the sides
    if ( y + height() > m.height() || y < 0 ) {
	//
	if ( pos.x() > width() + 2 )
	    x = pos.x() + widget->width() + 8;
	else
	    x = pos.x() - width() - 2;

	if ( pos.y() + height() < m.height() )
	    y = pos.y();
	else if ( pos.y() + widget->height() > height() )
	    y = pos.y() + widget->height() - height();
	else
	    y = m.height() - height();
    }
#endif

    // make the widget, and set it up
    if ( !whatsThat ) {
	whatsThat = new QWidget( 0, "automatic what's this? widget",
				 WStyle_Customize +
				 WStyle_NoBorder + WStyle_Tool );
	whatsThat->setBackgroundMode( QWidget::NoBackground );
    }
    whatsThat->setGeometry( x, y, w + shadowWidth, h + shadowWidth );
    whatsThat->show();

    // now for super-clever shadow stuff.  super-clever mostly in
    // how many window system problems it skirts around.
    p.begin( whatsThat );

    p.setPen( QApplication::palette()->normal().foreground() );
    p.drawRect( 0, 0, w, h );
    p.setPen( QApplication::palette()->normal().mid() );
    p.setBrush( QColor( 255, 255, 240 ) );
    p.drawRect( 1, 1, w-2, h-2 );
    p.setPen( QApplication::palette()->normal().text() );
    p.drawText( leftMargin, normalMargin, r.width(), r.height(),
		AlignLeft + AlignTop + WordBreak + ExpandTabs,
		text );
    p.setPen( black );
    p.drawPoint( w + 5, 6 );
    p.drawLine( w + 3, 6,
		w + 5, 8 );
    p.drawLine( w + 1, 6,
		w + 5, 10 );
    int i;
    for( i=7; i < h; i += 2 )
	p.drawLine( w, i,
		    w + 5, i + 5 );
    for( i = w - i + h; i > 6; i -= 2 )
	p.drawLine( i, h,
		    i + 5, h + 5 );
    for( ; i > 0 ; i -= 2 )
	p.drawLine( 6, h + 6 - i,
		    i + 5, h + 5 );
    p.end();
}



// and finally the What's This class itself

/*!  Adds \a text as What's This help for \a widget.  If \a deepCopy
  is TRUE, QWhatsThis makes a deep copy of the string; if it is FALSE
  QWhatsThis just copies the pointer \a text.
*/

void QWhatsThis::add( QWidget * widget, const char * text, bool deepCopy )
{
    QPixmap tmp;
    add( widget, tmp, 0, text, deepCopy );
}



/*!  Adds \a text as What's This help for \a widget, with title line
  \a title and icon \a icon.  If \a deepCopy is TRUE, QWhatsThis makes
  a deep copy of \a title and \a text; if it is FALSE QWhatsThis just
  copies the pointers.
*/

void QWhatsThis::add( QWidget * widget, const QPixmap & icon,
		      const char * title, const char * text,
		      bool deepCopy )
{
    QWhatsThisPrivate::setUpWhatsThis();
    QWhatsThisPrivate::Item * i = wt->dict->find( (void *)widget );
    if ( i )
	remove( widget );

    i = new QWhatsThisPrivate::Item;
    i->dc = deepCopy;
    if ( deepCopy ) {
	i->s = new char[ qstrlen(text) + 1 ];
	qstrcpy( i->s, text );
	if ( title ) {
	    i->t = new char[ qstrlen(text) + 1 ];
	    qstrcpy( i->t, text );
	}
    } else {
	i->s = (char*)text;
	i->t = (char*)title;
    }
    i->pm = icon;
    wt->dict->insert( (void *)widget, i );
    QWidget * tlw = widget->topLevelWidget();
    if ( !wt->tlw->find( (void *)tlw ) ) {
	wt->tlw->insert( (void *)tlw, tlw );
	tlw->installEventFilter( wt );
    }
}


/*!  Removes the What's This help for \a widget.  \sa add() */

void QWhatsThis::remove( QWidget * widget )
{
    QWhatsThisPrivate::setUpWhatsThis();
    QWhatsThisPrivate::Item * i = wt->dict->find( (void *)widget );
    if ( !i )
	return;

    wt->dict->take( (void *)widget );

    i->deref();
    if ( !i->dc || !i->count )
	delete i;
}


/*!  Returns the text (not the title) for \a widget, or 0 if there
  isn't any What's This help for \a widget.

  \sa add() */

const char * QWhatsThis::textFor( QWidget * widget )
{
    QWhatsThisPrivate::setUpWhatsThis();
    QWhatsThisPrivate::Item * i = wt->dict->find( widget );
    return i ? i->s : 0;
}


/*!  Returns a pointer to a specially configured QToolButton, suitable
  for use to enter What's This mode.

  \sa QToolButton
*/

QToolButton * QWhatsThis::whatsThisButton( QWidget * parent )
{
    QWhatsThisPrivate::setUpWhatsThis();
    return new QWhatsThisPrivate::Button( parent,
					  "automatic what's this? button" );
}


/*! \base64 whatsthis.gif

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
GyLb533h/K3hGL7he9HhKfLho9XeI+459z1CChHisz3e+o3eLO7f6xbkLs4XMG4bMn7gxV3j
aXPj6bQk5D3fQm7h+p3eQz4ZwX3kWpvlhLjlWcvlW+vlWr7ZhMLk0JHjtM1RSYTb0aPeGb7m
Vc7hV57dM87db642MKHjdZ7nRB7n/vJN43r+OSWO3w6uE3ge4X/u3Xxu4IVeM0peM2SuTYce
6YJR5DRy5IueE40u5sf96Nhi5g9+5pI+T4kO4n4u6ZzeLaGe6l9B6RBi6aUe6afuERB+6c2t
6m/O6szh6nRu6oGO406e5vXc23zF5rZuPaM+5wguGJmeK7HeEbMO6inx5CpO5V9G7MXOEMeO
5L697GPe601+4j0+7bzt5s/D3q+15rktaz/u4rhu4HKu7a7N7UvS7Gzl6YTOUeHuaWw+a9I9
bcvW7/4O8Hne7tr97rROKPK+IfQeTfk94RVe29Hd4gJf7uZe8S0+8Nl+8LGk7KG+8J1h7zmx
6Che6/W8lO/CTvEo7+8WT+1DTvDCreivfugeXzHg3uI+vu/9/e8Sv/N/7vK6ftwJLx8z/06D
HvL4buj6rt/8juH93vTmPvG3nvEx3/PeXua/HkeedvK6Te5uvvTo3vUpj/HXbfAcHxhBfx1D
rzs7fu9vvu5V7vNkv+uwXvWQfu0flepw3+dyL/N03+lFf/Rsny9u3/JSv/cqERAAOw==
*/
