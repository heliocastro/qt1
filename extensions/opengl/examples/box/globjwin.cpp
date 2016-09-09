/****************************************************************************
** $Id: globjwin.cpp,v 1.4.2.2 1999/01/19 14:07:34 aavit Exp $
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
    file->insertItem( "Exit",  qApp, SLOT(quit()), CTRL+Key_Q );

    // Create a menu bar
    QMenuBar *m = new QMenuBar( this );
    m->setSeparator( QMenuBar::InWindowsStyle );
    m->insertItem("&File", file );
    hlayout->setMenuBar( m );

    // Create a layout manager for the sliders
    QVBoxLayout* vlayout = new QVBoxLayout( 20, "vlayout");
    hlayout->addLayout( vlayout );

    // Create a nice frame tp put around the openGL widget
    QFrame* f = new QFrame( this, "frame" );
    f->setFrameStyle( QFrame::Sunken | QFrame::Panel );
    f->setLineWidth( 2 );
    hlayout->addWidget( f, 1 );

    // Create a layout manager for the openGL widget
    QHBoxLayout* flayout = new QHBoxLayout( f, 2, 2, "flayout");

    // Create an openGL widget
    GLBox* c = new GLBox( f, "glbox");
    c->setMinimumSize( 50, 50 );
    flayout->addWidget( c, 1 );
    flayout->activate();

    // Create the three sliders; one for each rotation axis
    QSlider* x = new QSlider ( 0, 360, 60, 0, QSlider::Vertical, this, "xsl" );
    x->setTickmarks( QSlider::Left );
    x->setMinimumSize( x->sizeHint() );
    vlayout->addWidget( x );
    QObject::connect( x, SIGNAL(valueChanged(int)),c,SLOT(setXRotation(int)) );

    QSlider* y = new QSlider ( 0, 360, 60, 0, QSlider::Vertical, this, "ysl" );
    y->setTickmarks( QSlider::Left );
    y->setMinimumSize( y->sizeHint() );
    vlayout->addWidget( y );
    QObject::connect( y, SIGNAL(valueChanged(int)),c,SLOT(setYRotation(int)) );

    QSlider* z = new QSlider ( 0, 360, 60, 0, QSlider::Vertical, this, "zsl" );
    z->setTickmarks( QSlider::Left );
    z->setMinimumSize( z->sizeHint() );
    vlayout->addWidget( z );
    QObject::connect( z, SIGNAL(valueChanged(int)),c,SLOT(setZRotation(int)) );

    // Start the geometry management
    hlayout->activate();
}
