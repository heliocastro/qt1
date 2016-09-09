/****************************************************************************
** $Id: main.cpp,v 2.3 1998/06/16 11:39:35 warwick Exp $
**
** Copyright (C) 1992-1998 Troll Tech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#include <qapplication.h>
#include <stdlib.h>
#include "tictac.h"


int main( int argc, char **argv )
{
    QApplication a( argc, argv );
    int n = 3;
    if ( argc == 2 )				// get board size n
	n = atoi(argv[1]);
    if ( n < 3 || n > 10 ) {			// out of range
	warning( "%s: Board size must be from 3x3 to 10x10", argv[0] );
	return 1;
    }
    TicTacToe ttt( n );				// create game
    a.setMainWidget( &ttt );
    ttt.show();					// show widget
    return a.exec();				// go
}
