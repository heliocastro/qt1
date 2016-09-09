/****************************************************************************
** $Id: main.cpp,v 2.6 1998/06/16 11:39:32 warwick Exp $
**
** Copyright (C) 1992-1998 Troll Tech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#include "hello.h"
#include <qapplication.h>


/*
  The program starts here. It parses the command line and builds a message
  string to be displayed by the Hello widget.
*/

int main( int argc, char **argv )
{
    QApplication a(argc,argv);
    QString s;
    for ( int i=1; i<argc; i++ ) {
	s += argv[i];
	if ( i<argc-1 )
	    s += " ";
    }
    if ( s.isEmpty() )
	s = "Hello, World";
    Hello h( s );
    h.setCaption( "Qt says hello" );
    QObject::connect( &h, SIGNAL(clicked()), &a, SLOT(quit()) );
    h.setFont( QFont("times",32,QFont::Bold) );		// default font
    h.setBackgroundColor( white );			// default bg color
    a.setMainWidget( &h );
    h.show();
    return a.exec();
}
