/****************************************************************************
** $Id: qbuttongroup.cpp,v 2.15 1998/07/03 00:09:47 hanord Exp $
**
** Implementation of QButtonGroup class
**
** Created : 950130
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

#define	 QButtonList QListM_QButtonItem
#include "qbuttongroup.h"
#include "qbutton.h"
#include "qlist.h"

/*!
  \class QButtonGroup qbuttongroup.h
  \brief The QButtonGroup widget organizes QButton widgets in a group.

  \ingroup realwidgets

  A button group widget makes it easier to deal with groups of
  buttons.  A button in a button group is associated with a unique
  identifer. The button group emits a clicked() signal with this
  identifier when the button is clicked. Thus, a button group is an
  ideal solution when you have several similar buttons and want to
  connect all their clicked() signals, for example, to one slot.

  An \link setExclusive() exclusive\endlink button group switches off all
  toggle buttons except the one that was clicked. A button group is by
  default non-exclusive, however, it automatically becomes an exclusive
  group when a QRadioButton is \link insert() inserted\endlink.

  There are two ways of using a button group:
  <ol>
  <li>The button group is a parent widget of a number of buttons,
  i.e. the button group is the parent argument in the button contructor.
  The buttons are assigned identifiers 0, 1, 2 etc. in the order they are
  created. A QButtonGroup can display a frame and a title because it inherits
  QGroupBox.
  <li>The button group is an invisible widget and the contained buttons
  have some other parent widget.  A button must then be manually inserted
  using the insert() function with an identifer.
  </ol>

  By default, the button group's setFont() and setPalette() functions
  do not change the appearance of the buttons, but you can use
  setFontPropagation() and setPalettePropagation() to change that.

  <img src=qbttngrp-m.gif> <img src=qbttngrp-w.gif>

  \sa QButton, QPushButton, QCheckBox, QRadioButton
*/


struct QButtonItem
{
    QButton *button;
    int	     id;
};

Q_DECLARE(QListM,QButtonItem);


/*!
  Constructs a button group with no title.

  The \e parent and \e name arguments are passed to the QWidget constructor.
*/

QButtonGroup::QButtonGroup( QWidget *parent, const char *name )
    : QGroupBox( parent, name )
{
    init();
}

/*!
  Constructs a button group with a title.

  The \e parent and \e name arguments are passed to the QWidget constructor.
*/

QButtonGroup::QButtonGroup( const char *title, QWidget *parent,
			    const char *name )
    : QGroupBox( title, parent, name )
{
    init();
}

/*!
  Initializes the button group.
*/

void QButtonGroup::init()
{
    buttons = new QButtonList;
    CHECK_PTR( buttons );
    buttons->setAutoDelete( TRUE );
    excl_grp = FALSE;
}

/*!
  Destroys the button group and its child widgets.
*/

QButtonGroup::~QButtonGroup()
{
    for ( register QButtonItem *bi=buttons->first(); bi; bi=buttons->next() )
	bi->button->setGroup(0);
    delete buttons;
}


/*!
  Returns TRUE if the button group is exclusive, otherwise FALSE.
  \sa setExclusive()
*/

bool QButtonGroup::isExclusive() const
{
    return excl_grp;
}

/*!
  Sets the button group to be exclusive if \e enable is TRUE,
  or to be non-exclusive if \e enable is FALSE.

  An exclusive button group switches off all other toggle buttons when
  one is switched on. This is ideal for groups of \link QRadioButton
  radio buttons\endlink A non-exclusive group allow many buttons to be
  swithed on at the same time.

  The default setting is FALSE. A button group automatically becomes an
  exclusive group when a QRadioButton is \link insert() inserted\endlink.

  \sa isExclusive()
*/

void QButtonGroup::setExclusive( bool enable )
{
    excl_grp = enable;
}


/*!
  Inserts a button with the identifier \e id into the button group.
  Returns the button identifier.

  It is not necessary to manually insert buttons that have this button
  group as their parent widget. An exception is when you want custom
  identifiers instead of the default 0, 1, 2 etc.

  The button is assigned the identifier \e id or an automatically
  generated identifier.	 It works as follows: If \e id >= 0, this
  identifier is assigned.  If \e id == -1 (default), the identifier is
  equal to the number of buttons in the group.	If \e id is any other
  negative integer, for instance -2, a unique identifier (negative
  integer \<= -2) is generated.

  Inserting several buttons with \e id = -1 assigns the identifers 0,
  1, 2, etc.

  This function calls setExclusive(TRUE) if \e button is a
  QRadioButton.

  \sa find(), remove(), setExclusive()
*/

int QButtonGroup::insert( QButton *button, int id )
{
    if ( button->group() )
	button->group()->remove( button );
    static int seq_no = -2;
    register QButtonItem *bi = new QButtonItem;
    CHECK_PTR( bi );
    if ( id < -1 )
	bi->id = seq_no--;
    else if ( id == -1 )
	bi->id = buttons->count();
    else
	bi->id = id;
    bi->button = button;
    button->setGroup(this);
    buttons->append( bi );
    connect( button, SIGNAL(pressed()) , SLOT(buttonPressed()) );
    connect( button, SIGNAL(released()), SLOT(buttonReleased()) );
    connect( button, SIGNAL(clicked()) , SLOT(buttonClicked()) );
    connect( button, SIGNAL(toggled(bool)) , SLOT(buttonToggled(bool)) );
    if ( button->inherits("QRadioButton") )
	setExclusive( TRUE );
    return bi->id;
}


/*!
  Removes a button from the button group.
  \sa insert()
*/

void QButtonGroup::remove( QButton *button )
{
    for ( QButtonItem *i=buttons->first(); i; i=buttons->next() ) {
	if ( i->button == button ) {
	    buttons->remove();
	    button->setGroup(0);
	    button->disconnect( this );
	    break;
	}
    }
}


/*!
  Finds and returns a pointer to the button with the specified identifier
  \e id.

  Returns null if the button was not found.
*/

QButton *QButtonGroup::find( int id ) const
{
    for ( QButtonItem *i=buttons->first(); i; i=buttons->next() ) {
	if ( i->id == id )
	    return i->button;
    }
    return 0;
}


/*!
  \fn void QButtonGroup::pressed( int id )
  This signal is emitted when a button in the group is
  \link QButton::pressed() pressed\endlink.
  The \e id argument is the button's identifier.
*/

/*!
  \fn void QButtonGroup::released( int id )
  This signal is emitted when a button in the group is
  \link QButton::released() released\endlink.
  The \e id argument is the button's identifier.
*/

/*!
  \fn void QButtonGroup::clicked( int id )
  This signal is emitted when a button in the group is
  \link QButton::clicked() clicked\endlink.
  The \e id argument is the button's identifier.
*/


/*!
  \internal
  This slot is activated when one of the buttons in the group emits the
  QButton::pressed() signal.
*/

void QButtonGroup::buttonPressed()
{
    int id = -1;
    QButton *bt = (QButton *)sender();		// object that sent the signal
    for ( register QButtonItem *i=buttons->first(); i; i=buttons->next() )
	if ( bt == i->button ) {		// button was clicked
	    id = i->id;
	    break;
	}
    if ( id != -1 )
	emit pressed( id );
}

/*!
  \internal
  This slot is activated when one of the buttons in the group emits the
  QButton::released() signal.
*/

void QButtonGroup::buttonReleased()
{
    int id = -1;
    QButton *bt = (QButton *)sender();		// object that sent the signal
    for ( register QButtonItem *i=buttons->first(); i; i=buttons->next() )
	if ( bt == i->button ) {		// button was clicked
	    id = i->id;
	    break;
	}
    if ( id != -1 )
	emit released( id );
}

/*!
  \internal
  This slot is activated when one of the buttons in the group emits the
  QButton::clicked() signal.
*/

void QButtonGroup::buttonClicked()
{
    int id = -1;
    QButton *bt = (QButton *)sender();		// object that sent the signal
#if defined(CHECK_NULL)
    ASSERT( bt->inherits("QButton") );
#endif
    for ( register QButtonItem *i=buttons->first(); i; i=buttons->next() ) {
	if ( bt == i->button ) {			// button was clicked
	    id = i->id;
	    break;
	}
    }
    if ( id != -1 )
	emit clicked( id );
}


/*!
  \internal
  This slot is activated when one of the buttons in the group emits the
  QButton::toggled() signal.
*/

void QButtonGroup::buttonToggled( bool on )
{
    if ( !on || !excl_grp )
	return;
    QButton *bt = (QButton *)sender();		// object that sent the signal
#if defined(CHECK_NULL)
    ASSERT( bt->inherits("QButton") );
    ASSERT( bt->isToggleButton() );
#endif
    for ( register QButtonItem *i=buttons->first(); i; i=buttons->next() ) {
	if ( !(bt == i->button) && i->button->isToggleButton() )
	    i->button->setOn( FALSE );		// turn other radio buttons off
    }
}



/*!  Sets the button with id \a id to be on, and if this is an
  exclusive group, all other button in the group to be off.
*/

void QButtonGroup::setButton( int id )
{
    QButton * b = find( id );
    if ( b )
	b->setOn( TRUE );
}
