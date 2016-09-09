//
// Qt OpenGL example: Shared Box
//
// A small example showing how to use OpenGL display list sharing
// 
// File: main.cpp
//
// The main() function 
// 

#include "globjwin.h"
#include <qapplication.h>
#include <qgl.h>

/*
  The main program is here. 
*/

int main( int argc, char **argv )
{
    QApplication::setColorSpec( QApplication::CustomColor );
    QApplication a(argc,argv);			

    if ( !QGLFormat::hasOpenGL() ) {
	warning( "This system has no OpenGL support. Exiting." );
	return -1;
    }

    GLObjectWindow w;
    w.resize( 550, 350 );
    a.setMainWidget( &w );
    w.show();
    return a.exec();
}
