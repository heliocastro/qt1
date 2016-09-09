/****************************************************************************
** $Id: dropsite.cpp,v 1.14.2.1 1998/08/21 18:08:14 warwick Exp $
**
** Drop site example implementation
**
** Created : 979899
**
** Copyright (C) 1997 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#include "dropsite.h"
#include "secret.h"
#include <qevent.h>
#include <qpixmap.h>
#include <qdragobject.h>
#include <qimage.h>

RCSTAG("$Id: dropsite.cpp,v 1.14.2.1 1998/08/21 18:08:14 warwick Exp $");


DropSite::DropSite( QWidget * parent, const char * name )
    : QLabel( parent, name ),
      QDropSite( this )
{
}


DropSite::~DropSite()
{
    // nothing necessary
}


void DropSite::dragMoveEvent( QDragMoveEvent * /*e*/ )
{
    // Check if you want the drag at e->pos()...
    // Give the user some feedback...
}


void DropSite::dragEnterEvent( QDragEnterEvent *e )
{
    // Check if you want the drag...
    if ( SecretDrag::canDecode( e )
      || QTextDrag::canDecode( e )
      || QImageDrag::canDecode( e ) )
    {
	e->accept();
    }


    // Give the user some feedback...
    QString t;
    for( int i=0; e->format( i ); i++ ) {
	if ( *(e->format( i )) ) {
	    if ( !t.isEmpty() )
		t += "\n";
	    t += e->format( i );
	}
    }
    emit message( t );
    setBackgroundColor(white);
}

void DropSite::dragLeaveEvent( QDragLeaveEvent * )
{
    // Give the user some feedback...
    emit message("");
    setBackgroundColor(lightGray);
}


void DropSite::dropEvent( QDropEvent * e )
{
    // Try to decode to the data you understand...

    QString str;
    if ( QTextDrag::decode( e, str ) ) {
	setText( str );
	setMinimumSize( minimumSize().expandedTo( sizeHint() ) );
	return;
    }

    QPixmap pm;
    if ( QImageDrag::decode( e, pm ) ) {
	setPixmap( pm );
	setMinimumSize( minimumSize().expandedTo( sizeHint() ) );
	return;
    }

    if ( SecretDrag::decode( e, str ) ) {
        setText( str );
	setMinimumSize( minimumSize().expandedTo( sizeHint() ) );
	return;
    }
}


void DropSite::mousePressEvent( QMouseEvent * /*e*/ )
{
    QDragObject *d;
    if ( pixmap() ) {
	d = new QImageDrag( pixmap()->convertToImage(), this );
	QPixmap pm;
	pm.convertFromImage(pixmap()->convertToImage().smoothScale(
	   pixmap()->width()/3,pixmap()->height()/3));
	d->setPixmap(pm,QPoint(-5,-7));
    } else {
	d = new QTextDrag( text(), this );
    }
    d->dragCopy();
}


