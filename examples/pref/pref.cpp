/****************************************************************************
** $Id: pref.cpp,v 1.21.2.1 1998/09/28 17:38:19 warwick Exp $
**
** Copyright (C) 1992-1998 Troll Tech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#include "pref.h"
#include <qtabdialog.h>
#include <qmultilinedit.h>
#include <qbuttongroup.h>
#include <qradiobutton.h>
#include <qlabel.h>
#include <qslider.h>
#include <qlayout.h>
#include <stdio.h>
#include <qstring.h>

Preferences::Preferences( QWidget * parent , const char * name )
    : QLabel( parent, name )
{
    QTabDialog * tab = new QTabDialog( 0, "top-level dialog" );
    tab->setCaption( "Ugly Tab Dialog" );

    // set up page one of the tab dialog
    QWidget * w = new QWidget( tab, "page one" );

    // stuff the labels and lineedits into a grid layout
    QGridLayout * g = new QGridLayout( w, 2, 2, 5 );

    // two multilineedits in column 1
    ed1 = new QMultiLineEdit( w );
    g->addWidget( ed1, 0, 1 );
    ed1->setText( "" );
    ed1->setMinimumSize( QSize( 100, 10 ) );
    ed2 = new QMultiLineEdit( w );
    g->addWidget( ed2, 1, 1 );
    ed2->setText( "" );
    ed2->setMinimumSize( QSize( 100, 10 ) );

    // let the lineedits stretch
    g->setColStretch( 1, 1 );

    // two labels in column 0
    QLabel * l = new QLabel( w );
    g->addWidget( l, 0, 0 );
    l->setText( "&Name" );
    l->setBuddy( ed1 );
    l->setMinimumSize( l->sizeHint() );
    l = new QLabel( w );
    g->addWidget( l, 1, 0 );
    l->setText( "&Email" );
    l->setBuddy( ed2 );
    l->setMinimumSize( l->sizeHint() );

    // no stretch on the labels unless they have to
    g->setColStretch( 0, 0 );

    // finally insert page one into the tab dialog and start GM
    tab->addTab( w, "&Who" );
    g->activate();

    // that was page one, now for page two, where we use a box layout
    w = new QWidget( tab, "page two" );
    QBoxLayout * b = new QBoxLayout( w, QBoxLayout::LeftToRight, 5 );

    // two vertical boxes in the horizontal one
    QBoxLayout * radioBoxes = new QBoxLayout( QBoxLayout::Down );
    b->addLayout( radioBoxes );

    // fill the leftmost vertical box
    b1 = new QRadioButton( w, "radio button 1" );
    b1->setText( "Male" );
    b1->setMinimumSize( b1->sizeHint() );
    b1->setMaximumSize( 500, b1->minimumSize().height() );
    radioBoxes->addWidget( b1, AlignLeft|AlignTop );
    b2 = new QRadioButton( w, "radio button 2" );
    b2->setText( "Female" );
    b2->setMinimumSize( b2->sizeHint() );
    b2->setMaximumSize( 500, b2->minimumSize().height() );
    radioBoxes->addWidget( b2, AlignLeft|AlignTop );
    b3 = new QRadioButton( w, "radio button 3" );
    b3->setText( "Other" );
    b3->setMinimumSize( b3->sizeHint() );
    b3->setMaximumSize( 500, b3->minimumSize().height() );
    radioBoxes->addWidget( b3, AlignLeft|AlignTop );

    // since none of those will stretch, add some stretch at the bottom
    radioBoxes->addStretch( 1 );

    // insert all of the radio boxes into the button group, so they'll
    // switch each other off
    bg = new QButtonGroup();
    bg->insert( b1 );
    bg->insert( b2 );
    bg->insert( b3 );

    // add some optional spacing between the radio buttons and the slider

    b->addStretch( 1 );

    // make the central slider
    mood = new QSlider( QSlider::Vertical, w, "mood slider" );
    mood->setRange( 0, 127 );
    mood->setMinimumSize( mood->sizeHint() );
    mood->setMaximumSize( mood->minimumSize().width(), 500 );
    b->addWidget( mood, AlignLeft|AlignTop|AlignBottom );

    // make the top and bottom labels for the slider
    QBoxLayout * labels = new QBoxLayout( QBoxLayout::Down );
    b->addLayout( labels );
    l = new QLabel( "Optimistic", w, "optimistic" );
    l->setFixedSize( l->sizeHint() );
    labels->addWidget( l, AlignTop|AlignLeft );

    // spacing in the middle, so the labels are located right
    labels->addStretch( 1 );

    l = new QLabel( "Pessimistic", w, "pessimistic" );
    l->setFixedSize( l->sizeHint() );
    labels->addWidget( l, AlignBottom|AlignLeft );

    b->activate();
    tab->addTab( w, "&How" );

    // we want both Apply and Cancel
    tab->setApplyButton();
    tab->setCancelButton();

    connect( tab, SIGNAL(applyButtonPressed()), SLOT(apply()) );
    connect( tab, SIGNAL(cancelButtonPressed()), SLOT(setup()) );
    connect( tab, SIGNAL(aboutToShow()), SLOT(setup()) );

    tab->resize( 200, 135 );

    setText( "This tab dialog is rather ugly:  The code is clear, though:\n"
	     "There are no hard-to-understand aesthetic tradeoffs\n" );

    show();
    tab->show();
}


Preferences::~Preferences()
{
    delete bg;
    // the others are children of this, so Qt will delete them
}


void Preferences::setup()
{
    ed1->setText( "Inge Rowe" );
    ed2->setText( "inge@troll.no" );

    b1->setChecked( TRUE );

    mood->setValue( 42 );
}


void Preferences::apply()
{
    QString s;
    s.sprintf( "What the dialog decided:\n"
	       "\tLine Edit 1: %s\n\tLineEdit 2: %s\n"
	       "\tMood: %d (0 == down, 127 == up)\n"
	       "\tButtons: %s %s %s\n",
	       (const char *)(ed1->text()), (const char *)(ed2->text()),
	       mood->value(),
	       b1->isChecked() ? "X" : "-",
	       b2->isChecked() ? "X" : "-",
	       b3->isChecked() ? "X" : "-" );
    setText( s );
    QSize sh (sizeHint() );
    bool b = FALSE;

    if ( sh.width() > width() ) {
	sh.setWidth( width() );
	b = TRUE;
    }
    if ( sh.height() > height() ) {
	sh.setHeight( height() );
	b = TRUE;
    }

    if ( b )
	resize( sh );

    repaint();
}
