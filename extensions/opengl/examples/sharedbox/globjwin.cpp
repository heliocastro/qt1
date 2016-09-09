/****************************************************************************
** $Id: globjwin.cpp,v 1.2.2.3 1999/01/19 14:07:39 aavit Exp $
**
** Implementation of GLObjectWindow widget class
**
****************************************************************************/


#include <qpushbutton.h>
#include <qslider.h>
#include <qlayout.h>
#include <qframe.h>
#include <qmenubar.h>
#include <qpopupmenu.h>
#include <qapplication.h>
#include <qkeycode.h>
#include "globjwin.h"
#include "glbox.h"


GLObjectWindow::GLObjectWindow( QWidget* parent, const char* name )
    : QWidget( parent, name )
{
    // Create top-level layout manager
    QHBoxLayout* hlayout = new QHBoxLayout( this, 20, 20, "hlayout");

    // Create a menu
    QPopupMenu *file = new QPopupMenu();
    file->insertItem( "Delete Left QGLWidget", this, 
		      SLOT(deleteFirstWidget()) );
    file->insertItem( "Exit",  qApp, SLOT(quit()), CTRL+Key_Q );

    // Create a menu bar
    QMenuBar *m = new QMenuBar( this );
    m->setSeparator( QMenuBar::InWindowsStyle );
    m->insertItem("&File", file );
    hlayout->setMenuBar( m );

    // Create a layout manager for the sliders
    QVBoxLayout* vlayout = new QVBoxLayout( 20, "vlayout");
    hlayout->addLayout( vlayout );

    // Create a nice frame to put around the openGL widget
    QFrame* f = new QFrame( this, "frame" );
    f->setFrameStyle( QFrame::Sunken | QFrame::Panel );
    f->setLineWidth( 2 );
    hlayout->addWidget( f, 1 );

    // Create a layout manager for the openGL widget
    QHBoxLayout* flayout = new QHBoxLayout( f, 2, 2, "flayout");


    // Create an openGL widget
    c1 = new GLBox( f, "glbox1");
    c1->setMinimumSize( 50, 50 );
    flayout->addWidget( c1, 1 );
    flayout->activate();


    // Create a nice frame to put around the second openGL widget
    QFrame* f2 = new QFrame( this, "frame2" );
    f2->setFrameStyle( QFrame::Sunken | QFrame::Panel );
    f2->setLineWidth( 2 );
    hlayout->addWidget( f2, 1 );

    // Create a layout manager for the second openGL widget
    QHBoxLayout* flayout2 = new QHBoxLayout( f2, 2, 2, "flayout2");

    // Create another openGL widget which shares display lists with the first
    c2 = new GLBox( f2, "glbox2", c1 );
    c2->setMinimumSize( 50, 50 );
    flayout2->addWidget( c2, 1 );
    flayout2->activate();

    // Create the three sliders; one for each rotation axis
    QSlider* x = new QSlider ( 0, 360, 60, 0, QSlider::Vertical, this, "xsl" );
    x->setTickmarks( QSlider::Left );
    x->setMinimumSize( x->sizeHint() );
    vlayout->addWidget( x );
    QObject::connect( x, SIGNAL(valueChanged(int)),c1,SLOT(setXRotation(int)));
    QObject::connect( x, SIGNAL(valueChanged(int)),c2,SLOT(setZRotation(int)));

    QSlider* y = new QSlider ( 0, 360, 60, 0, QSlider::Vertical, this, "ysl" );
    y->setTickmarks( QSlider::Left );
    y->setMinimumSize( y->sizeHint() );
    vlayout->addWidget( y );
    QObject::connect( y, SIGNAL(valueChanged(int)),c1,SLOT(setYRotation(int)));
    QObject::connect( y, SIGNAL(valueChanged(int)),c2,SLOT(setXRotation(int)));

    QSlider* z = new QSlider ( 0, 360, 60, 0, QSlider::Vertical, this, "zsl" );
    z->setTickmarks( QSlider::Left );
    z->setMinimumSize( z->sizeHint() );
    vlayout->addWidget( z );
    QObject::connect( z, SIGNAL(valueChanged(int)),c1,SLOT(setZRotation(int)));
    QObject::connect( z, SIGNAL(valueChanged(int)),c2,SLOT(setYRotation(int)));

    // Start the geometry management
    hlayout->activate();
}


void GLObjectWindow::deleteFirstWidget()
{
    // Delete only c1; c2 will keep working and use the shared display list
    if ( c1 ) {
	delete c1;
	c1 = 0;
    }
}
