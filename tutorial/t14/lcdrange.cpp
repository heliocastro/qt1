/****************************************************************
**
** Implementation of LCDRange class, Qt tutorial 14
**
****************************************************************/

#include "lcdrange.h"

#include <qscrollbar.h>
#include <qlcdnumber.h>
#include <qlabel.h>

LCDRange::LCDRange( QWidget *parent, const char *name )
        : QWidget( parent, name )
{
    init();
}

LCDRange::LCDRange( const char *s, QWidget *parent, const char *name )
        : QWidget( parent, name )
{
    init();
    setText( s );
}

void LCDRange::init()
{
    lcd  = new QLCDNumber( 2, this, "lcd"  );
    lcd->move( 0, 0 );
    sBar = new QScrollBar( 0, 99,		       	// range
			   1, 10, 			// line/page steps
			   0, 				// inital value
			   QScrollBar::Horizontal, 	// orientation
                           this, "scrollbar" );
    label  = new QLabel( this, "label"  );
    label->setAlignment( AlignCenter );
    connect( sBar, SIGNAL(valueChanged(int)), lcd, SLOT(display(int)) );
    connect( sBar, SIGNAL(valueChanged(int)), SIGNAL(valueChanged(int)) );

}

int LCDRange::value() const
{
    return sBar->value();
}

const char *LCDRange::text() const
{
    return label->text();
}

void LCDRange::setValue( int value )
{
    sBar->setValue( value );
}

void LCDRange::setRange( int minVal, int maxVal )
{
    if ( minVal < 0 || maxVal > 99 || minVal > maxVal ) {
	warning( "LCDRange::setRange(%d,%d)\n"
		 "\tRange must be 0..99\n"
		 "\tand minVal must not be greater than maxVal",
		 minVal, maxVal );
	return;
    }
    sBar->setRange( minVal, maxVal );
}

void LCDRange::setText( const char *s )
{
    label->setText( s );
}

void LCDRange::resizeEvent( QResizeEvent * )
{
    lcd->resize( width(), height() - 41 - 5 );
    sBar->setGeometry( 0, lcd->height() + 5, width(), 16 );
    label->setGeometry( 0, lcd->height() + 21, width(), 20 );
}
