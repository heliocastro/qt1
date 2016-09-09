//
// Qt OpenGL example: Box
//
// A small example showing how a GLWidget can be used just as any Qt widget
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
    w.resize( 400, 350 );
    a.setMainWidget( &w );
    w.show();
    return a.exec();
}
