/****************************************************************************
** $Id: drawdemo.cpp,v 2.7.2.3 1998/12/22 13:21:16 hanord Exp $
**
** Copyright (C) 1992-1998 Troll Tech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#include <qwindow.h>
#include <qpainter.h>
#include <qprinter.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qbuttongroup.h>
#include <qapplication.h>
#include <math.h>


//
// This function draws a color wheel.
// The coordinate system x=(0..500), y=(0..500) spans the paint device.
//

void drawColorWheel( QPainter *p )
{
    QFont f( "times", 18, QFont::Bold );
    p->setFont( f );
    p->setPen( black );				// black pen outline
    p->setWindow( 0, 0, 500, 500 );		// defines coordinate system

    for ( int i=0; i<36; i++ ) {		// draws 36 rotated rectangles

	QWMatrix matrix;
	matrix.translate( 250.0F, 250.0F );	// move to center
	matrix.shear( 0.0F, 0.3F );		// twist it
	matrix.rotate( (float)i*10 );		// rotate 0,10,20,.. degrees
	p->setWorldMatrix( matrix );		// use this world matrix

	QColor c;
	c.setHsv( i*10, 255, 255 );		// rainbow effect
	p->setBrush( c );			// solid fill with color c
	p->drawRect( 70, -10, 80, 10 );		// draw the rectangle

	QString n;
	n.sprintf( "H=%d", i*10 );
	p->drawText( 80+70+5, 0, n );		// draw the hue number
    }
}


//
// This function draws a few lines of text using different fonts.
//

void drawFonts( QPainter *p )
{
    static char *fonts[] = { "Helvetica", "Courier", "Times", 0 };
    static int	 sizes[] = { 10, 12, 18, 24, 0 };
    int f = 0;
    int s = 0;
    int y = 0;
    while ( fonts[f] ) {
	s = 0;
	while ( sizes[s] ) {
	    QFont font( fonts[f], sizes[s] );
	    p->setFont( font );
	    QFontMetrics fm = p->fontMetrics();
	    y += fm.ascent();
	    p->drawText( 10, y, "Quartz Glyph Job Vex'd Cwm Finks" );
	    y += fm.descent();
	    s++;
	}
	f++;
    }
}


//
// This function draws some shapes
//

void drawShapes( QPainter *p )
{
    QBrush b1( blue );				// solid blue brush
    QBrush b2( green, Dense6Pattern );		// green 12% fill
    QBrush b3( NoBrush );			// void brush
    QBrush b4( CrossPattern );			// black cross pattern

    p->setPen( red );
    p->setBrush( b1 );
    p->drawRect( 10, 10, 200, 100 );		// draw some shapes
    p->setBrush( b2 );
    p->drawRoundRect( 10, 150, 200, 100, 20, 20 );
    p->setBrush( b3 );
    p->drawEllipse( 250, 10, 200, 100 );
    p->setBrush( b4 );
    p->drawPie( 250, 150, 200, 100, 45*16, 90*16 );
}


//
// This function draws a text that follows a (Bezier) curve.
// Notice that this function does not support the general case.
// It should be rewritten to calculate the real dx/dy.
//

void drawPathText( QPainter *p )
{
    QPointArray a( 4 );
    a.setPoint( 0, 100,200 );
    a.setPoint( 1, 150,75 );
    a.setPoint( 2, 250,75 );
    a.setPoint( 3, 300,200 );
    a = a.quadBezier();				// calculate Bezier curve

    p->setPen( lightGray );			// set light gray pen
    p->drawPolyline( a );			// draw Bezier point array

    p->setFont( QFont("Times",24) );		// set fast font
    p->setPen( black );				// set black pen

    const char *text = "Troll Tech AS";

    int len = strlen(text);
    if ( len == 0 )
	return;
    int ipos = a.size()/len;
    int cpos = ipos;

    for ( int i=0; i<len; i++ ) {		// for each char in text...
	QPoint p1 = a.point( cpos-1 );
	QPoint p2 = a.point( cpos+1 );
	QPoint pt = a.point(cpos);
	float dx = (float)(p2.x() - p1.x());
	float dy = (float)(p2.y() - p1.y());
	float angle = (float)atan(dy/dx);	// way too simple
	angle *= 180.0F/3.14F;
	QWMatrix m;				// setup world matrix
	m.translate( (float)pt.x(), (float)pt.y() );
	m.rotate( angle );
	p->setWorldMatrix( m );
	p->drawText( 0,0, &text[i], 1 );
	cpos += ipos;
    }

}


typedef void (*draw_func)(QPainter*);

struct DrawThing {
    draw_func	 f;
    char	*name;
};

//
// You can add your draw function here.
// Leave the zeros at the end of the array!
//

DrawThing ourDrawFunctions[] = {
    { drawColorWheel,	"Draw color wheel" },
    { drawFonts,	"Draw fonts" },
    { drawShapes,	"Draw shapes" },
    { drawPathText,	"Draw path text" },
    { 0,		0 } };


//
// DrawView has installable draw routines, just add a function pointer
// and a text in the table above.
//

class DrawView : public QWindow
{
    Q_OBJECT
public:
    DrawView();
public slots:
    void   updateIt( int );
    void   printIt();
protected:
    void   drawIt( QPainter * );
    void   paintEvent( QPaintEvent * );
    void   resizeEvent( QResizeEvent * );
private:
    QPrinter	 *printer;
    QButtonGroup *bgroup;
    QPushButton	 *print;
    int		  drawindex;
    int		  maxindex;
};


//
// Construct the DrawView with buttons.
//

DrawView::DrawView()
{
    setCaption( "Qt Draw Demo Application" );
    setBackgroundColor( white );

    printer = new QPrinter;

  // Create a button group to contain all buttons
    bgroup = new QButtonGroup( this );
    bgroup->resize( 200, 200 );
    connect( bgroup, SIGNAL(clicked(int)), SLOT(updateIt(int)) );

  // Calculate the size for the radio buttons
    int maxwidth = 80;
    int i;
    char *n;
    QFontMetrics fm = bgroup->fontMetrics();
    for ( i=0; (n=ourDrawFunctions[i].name) != 0; i++ ) {
	int w = fm.width( n );
	maxwidth = QMAX(w,maxwidth);
    }
    maxwidth = maxwidth + 20;			// add 20 pixels

    for ( i=0; (n=ourDrawFunctions[i].name) != 0; i++ ) {
	QRadioButton *rb = new QRadioButton( n, bgroup );
	rb->setGeometry( 10, i*30+10, maxwidth, 30 );
	if ( i == 0 )
	    rb->setChecked( TRUE );
    }

    drawindex = 0;				// draw first thing
    maxindex  = i;

    maxwidth += 40;				// now size of bgroup

  // Create and setup the print button
    print = new QPushButton( "Print", bgroup );
    print->resize( 80, 30 );
    print->move( maxwidth/2 - print->width()/2, maxindex*30+20 );
    connect( print, SIGNAL(clicked()), SLOT(printIt()) );

    bgroup->resize( maxwidth, print->y()+print->height()+10 );

    resize( 640,300 );
}


//
// Called when a radio button is clicked.
//

void DrawView::updateIt( int index )
{
    if ( index < maxindex ) {
	drawindex = index;
	update();
    }
}

//
// Calls the drawing function as specified by the radio buttons.
//

void DrawView::drawIt( QPainter *p )
{
    (*ourDrawFunctions[drawindex].f)(p);	// call draw function
}

//
// Called when the print button is clicked.
//

void DrawView::printIt()
{
    if ( printer->setup(this) ) {
	QPainter paint( printer );	
	drawIt( &paint );
    }
}

//
// Called when the widget needs to be updated.
//

void DrawView::paintEvent( QPaintEvent * )
{
    QPainter paint( this );
    drawIt( &paint );				// draw color wheel
}

//
// Called when the widget has been resized.
// Moves the button group to the upper right corner
// of the widget.

void DrawView::resizeEvent( QResizeEvent * )
{
    bgroup->move( width()-bgroup->width(), 0 );
}


//
// Create and display our widget.
//

#include "drawdemo.moc"

int main( int argc, char **argv )
{
    QApplication app( argc, argv );
    DrawView   draw;
    app.setMainWidget( &draw );
    draw.show();
    return app.exec();
}
