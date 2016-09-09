/****************************************************************************
** $Id: main.cpp,v 1.7 1998/06/16 11:39:33 warwick Exp $
**
** Copyright (C) 1992-1998 Troll Tech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#include <qapplication.h>
#include "pref.h"

int main( int argc, char ** argv ) {
    QApplication a( argc, argv );
    Preferences mw;
    a.setMainWidget( &mw );
    mw.setCaption( "Preferences Output" );
    mw.show();
    return a.exec();
}
