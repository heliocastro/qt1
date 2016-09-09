/****************************************************************************
** $Id: main.cpp,v 2.18.2.1 1999/01/27 12:43:10 aavit Exp $
**
** Copyright (C) 1992-1998 Troll Tech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#include "showimg.h"
#include <qapplication.h>
#include <qimage.h>

#ifdef QIMGIO
#include <qimageio.h>
#endif


int main( int argc, char **argv )
{
    if ( argc > 1 && QString(argv[1]) == "-m" ) {
	QApplication::setColorSpec( QApplication::ManyColor );
	argc--;
	argv++;
    }

    QApplication::setFont( QFont("Helvetica", 12) );
    QApplication a( argc, argv );

#ifdef QIMGIO
    qInitImageIO();
#endif

    if ( argc <= 1 ) {
	// Create a window which looks after its own existence.
	ImageViewer *w =
	    new ImageViewer(0, "new window", WDestructiveClose);
	w->show();
    } else {
	for ( int i=1; i<argc; i++ ) {
	    // Create a window which looks after its own existence.
	    ImageViewer *w =
		new ImageViewer(0, argv[i], WDestructiveClose);
	    w->loadImage( argv[i] );
	    w->show();
	}
    }

    QObject::connect(qApp, SIGNAL(lastWindowClosed()), qApp, SLOT(quit()));

    return a.exec();
}
