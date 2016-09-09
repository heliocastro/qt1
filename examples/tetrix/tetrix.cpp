/****************************************************************************
** $Id: tetrix.cpp,v 2.5.2.1 1998/08/27 09:02:14 hanord Exp $
**
** Copyright (C) 1992-1998 Troll Tech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#include "qtetrix.h"
#include "qdragapp.h"
#include "qfont.h"

int main( int argc, char **argv )
{
    QApplication::setColorSpec( QApplication::CustomColor );
    QDragApplication a(argc,argv);
#if defined(_WS_X11_)
    // Great font on X, not on Windows
    QApplication::setFont( QFont( "helvetica", 12 ) );
#endif
    QTetrix *tetrix = new QTetrix;
    a.setMainWidget(tetrix);
    tetrix->show();
    return a.exec();
}
