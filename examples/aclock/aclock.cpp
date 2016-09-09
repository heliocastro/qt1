/****************************************************************************
** $Id: aclock.cpp,v 2.4 1998/05/21 19:24:50 agulbra Exp $
**
** Copyright (C) 1992-1998 Troll Tech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#include "aclock.h"
#include <qtimer.h>
#include <qpainter.h>


//
// Constructs an analog clock widget that uses an internal QTimer.
//

AnalogClock::AnalogClock( QWidget *parent, const char *name )
    : QWidget( parent, name )
{
    time = QTime::currentTime();		// get current time
    QTimer *internalTimer = new QTimer( this );	// create internal timer
    connect( internalTimer, SIGNAL(timeout()), SLOT(timeout()) );
    internalTimer->start( 5000 );		// emit signal every 5 seconds
}


//
// The QTimer::timeout() signal is received by this slot.
//

void AnalogClock::timeout()
{
    QTime new_time = QTime::currentTime();	// get the current time
    if ( new_time.minute() != time.minute() )	// minute has changed
	update();
}


//
// The clock is painted using a 1000x1000 square coordinate system.
//

void AnalogClock::paintEvent( QPaintEvent * )	// paint clock
{
    if ( !isVisible() )				// is is invisible
	return;
    time = QTime::currentTime();		// save current time

    QPointArray pts;
    QPainter paint( this );
    paint.setBrush( foregroundColor() );	// fill with foreground color

    QPoint cp = rect().center();		// widget center point
    int d = QMIN(width(),height());		// we want a circular clock

    QWMatrix matrix;				// setup transformation matrix
    matrix.translate( cp.x(), cp.y() );		// origin at widget center
    matrix.scale( d/1000.0F, d/1000.0F );	// scale coordinate system

    float h_angle = 30*(time.hour()%12-3) + time.minute()/2;
    matrix.rotate( h_angle );			// rotate to draw hour hand
    paint.setWorldMatrix( matrix );
    pts.setPoints( 4, -20,0,  0,-20, 300,0, 0,20 );
    paint.drawPolygon( pts );			// draw hour hand
    matrix.rotate( -h_angle );			// rotate back to zero

    float m_angle = (time.minute()-15)*6;
    matrix.rotate( m_angle );			// rotate to draw minute hand
    paint.setWorldMatrix( matrix );
    pts.setPoints( 4, -10,0, 0,-10, 400,0, 0,10 );
    paint.drawPolygon( pts );			// draw minute hand
    matrix.rotate( -m_angle );			// rotate back to zero

    for ( int i=0; i<12; i++ ) {		// draw hour lines
	paint.setWorldMatrix( matrix );
	paint.drawLine( 450,0, 500,0 );
	matrix.rotate( 30 );
    }
}
