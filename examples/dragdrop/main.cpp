/****************************************************************************
** $Id: main.cpp,v 1.8.2.2 1999/02/13 19:38:21 hanord Exp $
**
** Ritual main() for Qt applications
**
** Copyright (C) 1996 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#include <qapplication.h>
#include "dropsite.h"
#include "secret.h"
#include <qlayout.h>
#include <qcombobox.h>
#include <qlabel.h>
#include <qpixmap.h>

static void addStuff( QWidget * parent, bool image, bool secret = FALSE )
{
    QVBoxLayout * tll = new QVBoxLayout( parent, 10 );
    DropSite * d = new DropSite( parent );
    d->setFrameStyle( QFrame::Sunken + QFrame::WinPanel );
    tll->addWidget( d );
    if ( image ) {
	QPixmap stuff;
	stuff.load( "../widgets/trolltech.bmp" );
	d->setPixmap( stuff );
    } else {
	d->setText("Drag and Drop");
    }
    d->setFont(QFont("Helvetica",18));
    d->setMinimumSize( d->sizeHint() );
    if ( secret ) {
	SecretSource *s = new SecretSource( 42, parent );
	tll->addWidget( s );
    }
    
    QLabel * format = new QLabel( "\nNone\n", parent );
    format->setMinimumSize( format->sizeHint() );
    tll->addWidget( format );
    tll->activate();
    parent->resize( 1, 1 );

    QObject::connect( d, SIGNAL(message(const char *)),
		      format, SLOT(setText(const char *)) );
}


int main( int argc, char ** argv ) {
    QApplication a( argc, argv );

    QWidget mw;
    addStuff( &mw, TRUE );
    mw.setCaption( "Drag and Drop Example" );
    mw.show();

    QWidget mw2;
    addStuff( &mw2, FALSE );
    mw2.setCaption( "Drag and Drop Example" );
    mw2.show();

    QWidget mw3;
    addStuff( &mw3, TRUE, TRUE );
    mw3.setCaption( "Drag and Drop Example" );
    mw3.show();

    QObject::connect(qApp,SIGNAL(lastWindowClosed()),qApp,SLOT(quit()));
    return a.exec();
}
