/****************************************************************************
** $Id: forever.cpp,v 2.4 1998/06/16 11:39:32 warwick Exp $
**
** Copyright (C) 1992-1998 Troll Tech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#include <qlabel.h>
#include <qwidget.h>
#include <qpainter.h>
#include <qapplication.h>
#include <stdlib.h>				// defines rand() function


#define COLORS 120				// number of colors to use


//
// Counter - a widget that displays an unsigned int continuously.
//

class Counter : public QLabel
{
public:
    Counter( int *number, QWidget *parent=0, const char *name=0 );
protected:
    void  timerEvent( QTimerEvent * );
private:
    int  *number;				// number to display
};

//
// Constructs a Counter
//

Counter::Counter( int *c, QWidget *parent, const char *name )
    : QLabel( parent, name )
{
    number = c;
    setText( "  0  rectangles/second" );
    setAutoResize( TRUE );			// resize to fit the contents
    startTimer( 1000 );				// one second timeout
}

//
// Timer event is called every second and prints out number
//

void Counter::timerEvent( QTimerEvent * )
{
    if ( number ) {
	QString s;
	s.sprintf( "%d rectangles/second", *number );
	setText( s );
	*number = 0;
    }
    repaint( TRUE );
}


//
// Forever - a widget that draws rectangles forever.
//

class Forever : public QWidget
{
public:
    Forever( QWidget *parent=0, const char *name=0 );
protected:
    void	paintEvent( QPaintEvent * );
    void	timerEvent( QTimerEvent * );
private:
    Counter    *counter;
    int		rectangles;
    QColor	colors[COLORS];
};

//
// Constructs a Forever widget.
//

Forever::Forever( QWidget *parent, const char *name )
    : QWidget( parent, name )
{
    for (int a=0; a<COLORS; a++) {
	colors[a] = QColor( rand()&255,
			    rand()&255,
			    rand()&255 );
    }
    rectangles = 0;
    counter = new Counter( &rectangles );	// counter in its own window
    counter->show();				// show the counter
    startTimer( 0 );				// run continuous timer
}

//
// Handles paint events for the Forever widget.
//

void Forever::paintEvent( QPaintEvent * )
{
    QPainter paint( this );			// painter object
    paint.setWindow( 0, 0, 1024, 1024 );	// define coordinate system
    paint.setPen( NoPen );			// do not draw outline
    paint.setBrush( colors[rand() % COLORS]);	// set random brush color
    QPoint p1( rand()&1023, rand()&1023 );	// p1 = top left
    QPoint p2( rand()&1023, rand()&1023 );	// p2 = bottom right
    QRect r( p1, p2 );
    paint.drawRect( r );			// draw filled rectangle
}

//
// Handles timer events for the Forever widget.
//

void Forever::timerEvent( QTimerEvent * )
{
    repaint( FALSE );				// repaint, don't erase
    rectangles++;
}


//
// Create and display Forever widget.
//

int main( int argc, char **argv )
{
    QApplication a( argc, argv );		// create application object
    Forever always;				// create widget
    a.setMainWidget( &always );			// set as main widget
    always.show();				// show widget
    return a.exec();				// run event loop
}
