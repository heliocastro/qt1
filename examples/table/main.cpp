/****************************************************************************
** $Id: main.cpp,v 1.4 1998/06/16 11:39:34 warwick Exp $
**
** Copyright (C) 1992-1998 Troll Tech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#include "table.h"
#include <qpushbutton.h>
#include <qapplication.h>


/*
  Constants
*/

const int numRows = 20;				// Tablesize: number of rows
const int numCols = 20;				// Tablesize: number of columns

/*
  The program starts here. 
*/

int main( int argc, char **argv )
{
    QApplication a(argc,argv);			

    Table v( numRows, numCols );

    /*
      Fill the table with default content: a coordinate string.
    */
    QString s ;
    for( int i = 0; i < numRows; i++ ) {
	for( int j = 0; j < numCols; j++ ) {
	    s.setNum(j);
	    s += ' ';
	    s += 'A' + ( i % 26 );		// Wrap if necessary
	    v.setCellContent( i, j, s );
	}
    }

    a.setMainWidget( &v );
    v.show();
    return a.exec();
}
