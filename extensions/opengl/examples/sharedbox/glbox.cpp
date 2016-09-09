/****************************************************************************
** $Id: glbox.cpp,v 1.1 1998/03/23 11:44:44 aavit Exp $
**
** Implementation of GLBox
** This is a simple QGLWidget displaying a box
**
** The OpenGL code is mostly borrowed from Brian Pauls "spin" example
** in the Mesa distribution
**
****************************************************************************/

#include "glbox.h"

// Shared display list id:
GLuint GLBox::object = 0;

// Counter keeping track of number of GLBox instances sharing 
// the display list, so that the last instance can delete it:
int GLBox::listUsers = 0;

/*!
  Create a GLBox widget
*/

GLBox::GLBox( QWidget* parent, const char* name, const QGLWidget* shareWidget )
    : QGLWidget( parent, name, shareWidget )
{
    xRot = yRot = zRot = 0.0;		// default object rotation
    scale = 1.0;			// default object scale
    listUsers++;
}


/*!
  Release allocated resources
*/

GLBox::~GLBox()
{
    listUsers--;
    if ( listUsers == 0 ) { // I.e. this was the last object using display list
	glDeleteLists( object, 1 );
	object = 0;
    }
}


/*!
  Paint the box. The actual openGL commands for drawing the box are
  performed here.
*/

void GLBox::paintGL()
{
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();
    glTranslatef( 0.0, 0.0, -3.0 );
    glScalef( scale, scale, scale );

    glRotatef( xRot, 1.0, 0.0, 0.0 ); 
    glRotatef( yRot, 0.0, 1.0, 0.0 ); 
    glRotatef( zRot, 0.0, 0.0, 1.0 );

    glCallList( object );
}


/*!
  Set up the OpenGL rendering state, and define display list
*/

void GLBox::initializeGL()
{
    glClearColor( 0.0, 0.0, 0.0, 0.0 ); // Let OpenGL clear to black
    if ( object == 0 )	      		// If the display list is not made yet
	object = makeObject();		// ...make it
    glEnable(GL_DEPTH_TEST);
}



/*!
  Set up the OpenGL view port, matrix mode, etc.
*/

void GLBox::resizeGL( int w, int h )
{
    glViewport( 0, 0, (GLint)w, (GLint)h );
    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();
    glFrustum(-1.0, 1.0, -1.0, 1.0, 1.0, 10.0);
}


/*!
  Generate an OpenGL display list for the object to be shown, i.e. the box
*/

GLuint GLBox::makeObject()
{	
    GLuint list;

    list = glGenLists( 1 );

    glNewList( list, GL_COMPILE );

    glBegin(GL_QUADS);
    /* Front face */
    glColor3f((GLfloat)0.0, (GLfloat)0.7, (GLfloat)0.1);  /* Green */
    glVertex3f(-1.0, 1.0, 1.0);
    glVertex3f(1.0, 1.0, 1.0);
    glVertex3f(1.0, -1.0, 1.0);
    glVertex3f(-1.0, -1.0, 1.0);
    /* Back face */
    glColor3f((GLfloat)0.9, (GLfloat)1.0, (GLfloat)0.0);  /* Yellow */
    glVertex3f(-1.0, 1.0, -1.0);
    glVertex3f(1.0, 1.0, -1.0);
    glVertex3f(1.0, -1.0, -1.0);
    glVertex3f(-1.0, -1.0, -1.0);
    /* Top side face */
    glColor3f((GLfloat)0.2, (GLfloat)0.2, (GLfloat)1.0);  /* Blue */
    glVertex3f(-1.0, 1.0, 1.0);
    glVertex3f(1.0, 1.0, 1.0);
    glVertex3f(1.0, 1.0, -1.0);
    glVertex3f(-1.0, 1.0, -1.0);
    /* Bottom side face */
    glColor3f((GLfloat)0.7, (GLfloat)0.0, (GLfloat)0.1);  /* Red */
    glVertex3f(-1.0, -1.0, 1.0);
    glVertex3f(1.0, -1.0, 1.0);
    glVertex3f(1.0, -1.0, -1.0);
    glVertex3f(-1.0, -1.0, -1.0);
    glEnd();
    glEndList();

    return list;
}


/*!
  Set the rotation angle of the object to \e degrees around the X axis.
*/

void GLBox::setXRotation( int degrees )
{
    xRot = (GLfloat)(degrees % 360);
    updateGL();
}


/*!
  Set the rotation angle of the object to \e degrees around the Y axis.
*/

void GLBox::setYRotation( int degrees )
{
    yRot = (GLfloat)(degrees % 360);
    updateGL();
}


/*!
  Set the rotation angle of the object to \e degrees around the Z axis.
*/

void GLBox::setZRotation( int degrees )
{
    zRot = (GLfloat)(degrees % 360);
    updateGL();
}
