/****************************************************************
**
** Implementation CannonField class, Qt tutorial 9
**
****************************************************************/

#include "cannon.h"
#include <qpainter.h>

CannonField::CannonField( QWidget *parent, const char *name )
        : QWidget( parent, name )
{
    ang = 45;
}

void CannonField::setAngle( int degrees )
{
    if ( degrees < 5 )
	degrees = 5;
    if ( degrees > 70 )
	degrees = 70;
    if ( ang == degrees )
	return;
    ang = degrees;
    repaint();
    emit angleChanged( ang );
}

void CannonField::paintEvent( QPaintEvent * )
{
    QPainter p;
    QBrush   brush( blue );
    QPen     pen( NoPen );

    p.begin( this );
    p.setBrush( brush );
    p.setPen( pen );

    p.translate( 0, rect().bottom() );
    p.drawPie( QRect(-35, -35, 70, 70), 0, 90*16 );
    p.rotate( -ang );
    p.drawRect( QRect(33, -4, 15, 8) );

    p.end();
}
