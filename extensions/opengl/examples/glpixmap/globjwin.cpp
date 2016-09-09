/****************************************************************************
** $Id: globjwin.cpp,v 1.1.2.3 1999/01/28 12:26:11 aavit Exp $
**
** Implementation of GLObjectWindow widget class
**
****************************************************************************/


#include <qpushbutton.h>
#include <qslider.h>
#include <qlayout.h>
#include <qframe.h>
#include <qlabel.h>
#include <qmenubar.h>
#include <qpopupmenu.h>
#include <qapplication.h>
#include <qkeycode.h>
#include <qpixmap.h>
#include <qpainter.h>
#include "globjwin.h"
#include "glbox.h"


GLObjectWindow::GLObjectWindow( QWidget* parent, const char* name )
    : QWidget( parent, name )
{
    // Create top-level layout manager
    QHBoxLayout* hlayout = new QHBoxLayout( this, 20, 20, "hlayout");

    // Create a menu
    file = new QPopupMenu();
    file->setCheckable( TRUE );
    file->insertItem( "Render Pixmap", this, 
		      SLOT(makePixmap()) );
    file->insertItem( "Render Pixmap Manually", this, 
		      SLOT(makePixmapManually()) );
    file->insertItem( "Render Pixmap Hidden", this, 
		      SLOT(makePixmapHidden()) );
    file->insertItem( "Render Pixmap Hidden and Manually", this, 
		      SLOT(makePixmapHiddenManually()) );
    file->insertSeparator();
    fixMenuItemId = file->insertItem( "Use Fixed Pixmap Size", this, 
				      SLOT(useFixedPixmapSize()) );
    file->insertSeparator();
    file->insertItem( "Exit",  qApp, SLOT(quit()), CTRL+Key_Q );

    // Create a menu bar
    QMenuBar *m = new QMenuBar( this );
    m->setSeparator( QMenuBar::InWindowsStyle );
    m->insertItem("&File", file );
    hlayout->setMenuBar( m );

    // Create a layout manager for the sliders
    QVBoxLayout* vlayout = new QVBoxLayout( 20, "vlayout");
    hlayout->addLayout( vlayout );

    // Create a nice frame to put around the OpenGL widget
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

    // Create a label to hold the pixmap
    lb = new QLabel( this, "pixlabel" );
    lb->setMinimumSize( 50, 50 );
    lb->setFrameStyle( QFrame::Sunken | QFrame::Panel );
    lb->setLineWidth( 2 );
    lb->setAlignment( AlignCenter );
    lb->setMargin( 0 );
    hlayout->addWidget( lb, 1 );

    // Create the three sliders; one for each rotation axis
    QSlider* x = new QSlider ( 0, 360, 60, 0, QSlider::Vertical, this, "xsl" );
    x->setTickmarks( QSlider::Left );
    x->setMinimumSize( x->sizeHint() );
    vlayout->addWidget( x );
    QObject::connect( x, SIGNAL(valueChanged(int)),c1,SLOT(setXRotation(int)));

    QSlider* y = new QSlider ( 0, 360, 60, 0, QSlider::Vertical, this, "ysl" );
    y->setTickmarks( QSlider::Left );
    y->setMinimumSize( y->sizeHint() );
    vlayout->addWidget( y );
    QObject::connect( y, SIGNAL(valueChanged(int)),c1,SLOT(setYRotation(int)));

    QSlider* z = new QSlider ( 0, 360, 60, 0, QSlider::Vertical, this, "zsl" );
    z->setTickmarks( QSlider::Left );
    z->setMinimumSize( z->sizeHint() );
    vlayout->addWidget( z );
    QObject::connect( z, SIGNAL(valueChanged(int)),c1,SLOT(setZRotation(int)));

    // Start the geometry management
    hlayout->activate();
}



void GLObjectWindow::makePixmap()
{
    // This is the easiest way to render a pixmap, and sufficient unless one
    // has special needs

    // Make a pixmap to to be rendered by the gl widget
    QPixmap pm;

    // Render the pixmap, with either c1's size or the fixed size pmSz
    if ( pmSz.isValid() ) 
	pm = c1->renderPixmap( pmSz.width(), pmSz.height() );
    else 
	pm = c1->renderPixmap();

    if ( !pm.isNull() ) {
	// Present the pixmap to the user
	drawOnPixmap( &pm );
	lb->setPixmap( pm );
    }
    else {
	lb->setText( "Failed to render Pixmap." );
    }
}


void GLObjectWindow::makePixmapManually()
{
    // Make a pixmap to be rendered on by the QGLWidget
    QPixmap pm( pmSz.isValid() ? pmSz : c1->size() );

    // Store the QGLWidget's current context, for later restoration
    QGLContext* origCx = (QGLContext*)c1->context();

    // Make a gl format suitable for pixmap rendering 
    QGLFormat fmt( QGL::SingleBuffer | QGL::IndirectRendering );

    // Make a gl context to draw on our pixmap
    QGLContext* pcx = new QGLContext( fmt, &pm );

    // Make the QGLwidget use our pixmap-context, 
    // without deleting the old context, which we are storing
    // It will share GL display lists with the old context
    c1->setContext( pcx, 0, FALSE );
    
    if ( c1->isValid() ) {

	// Make the QGLWidget draw itself, i.e. render the pixmap
	c1->updateGL();

	// Present the pixmap to the user
	drawOnPixmap( &pm );
        lb->setPixmap( pm );
    }
    else {
	lb->setText( "Failed to render Pixmap." );
    }
    // Restore the old context to the QGLWidget, 
    // so it will continue to work as before
    c1->setContext( origCx );	// Will delete pcx
}


void GLObjectWindow::makePixmapHidden()
{
    // Make a gl format suitable for pixmap rendering 
    QGLFormat fmt( QGL::SingleBuffer | QGL::IndirectRendering );

    // Make a QGLWidget to draw the pixmap. This widget will not be shown.
    GLBox* w = new GLBox( fmt, this, "temporary glwidget", c1 );

    bool success = FALSE;
    QPixmap pm;

    if ( w->isValid() ) {
	// Set the current rotation
	w->copyRotation( *c1 );

	// Determine wanted pixmap size
	QSize sz = pmSz.isValid() ? pmSz : c1->size();

	// Make our hidden glwidget render the pixmap
	pm = w->renderPixmap( sz.width(), sz.height() );

	if ( !pm.isNull() )
	    success = TRUE;
    }

    if ( success ) {
	// Present the pixmap to the user
	drawOnPixmap( &pm );
	lb->setPixmap( pm );
    }
    else {
	lb->setText( "Failed to render Pixmap." );
    }
    delete w;
}


void GLObjectWindow::makePixmapHiddenManually()
{
    // Make a gl format suitable for pixmap rendering 
    QGLFormat fmt( QGL::SingleBuffer | QGL::IndirectRendering );

    // Make a QGLWidget to draw the pixmap. This widget will not be shown.    
    GLBox* w = new GLBox( this, "temporary glwidget", c1 );

    // Make a pixmap to be rendered
    QPixmap pm( pmSz.isValid() ? pmSz : c1->size() );

    // Make a gl context to draw on this pixmap
    QGLContext* pcx = new QGLContext( fmt, &pm );

    // Make our widget use this context
    w->setContext( pcx );

    if ( w->isValid() ) {
	// Set the current rotation
	w->copyRotation( *c1 );

	// Make the QGLWidget draw itself, i.e. render the pixmap
	w->updateGL();

	// Present the pixmap to the user
	drawOnPixmap( &pm );
	lb->setPixmap( pm );
    }
    else {
	lb->setText( "Failed to render Pixmap." );
    }
    delete w;	// Will delete pcx
}


void GLObjectWindow::drawOnPixmap( QPixmap* pm )
{
    // Draw some text on the pixmap to differentiate it from the GL window

    if ( pm->isNull() ) {
	warning("Cannot draw on null pixmap");
	return;
    }
    else {
	QPainter p( pm );
	p.setFont( QFont( "Helvetica", 18 ) );
	p.setPen( white );
	p.drawText( pm->rect(), AlignCenter, "This is a Pixmap" );
    }
}



void GLObjectWindow::useFixedPixmapSize()
{
    if ( pmSz.isValid() ) {
	pmSz = QSize();
	file->setItemChecked( fixMenuItemId, TRUE );
    }
    else {
	pmSz = QSize( 200, 200 );
	file->setItemChecked( fixMenuItemId, TRUE );
    }
}
