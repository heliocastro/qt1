/****************************************************************************
** $Id: secret.cpp,v 1.2.2.1 1998/08/21 18:08:14 warwick Exp $
**
** Custom MIME type implementation example
**
** Created : 979899
**
** Copyright (C) 1997 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#include "secret.h"
#include <qevent.h>

RCSTAG("$Id: secret.cpp,v 1.2.2.1 1998/08/21 18:08:14 warwick Exp $");


//create the object withe the secret byte
SecretDrag::SecretDrag( uchar secret, QWidget * parent, const char * name )
    : QStoredDrag( "secret/magic", parent, name )
{
    QByteArray data(1);
    data[0]= secret;
    setEncodedData( data );
}


bool SecretDrag::canDecode( QDragMoveEvent* e )
{
    return e->provides( "secret/magic" );
}

//decode it into a string
bool SecretDrag::decode( QDropEvent* e, QString& str )
{
    QByteArray payload = e->data( "secret/magic" );
    if ( payload.size() ) {
	e->accept();
	QString msg;
	msg.sprintf("The secret number is %d", payload[0] );
	str = msg;
	return TRUE;
    }
    return FALSE;
}


SecretSource::SecretSource( int secret, QWidget *parent, const char * name )
    : QLabel( "Secret", parent, name )
{
    setBackgroundColor( blue.light() );
    setFrameStyle( Box | Sunken );
    setMinimumHeight( sizeHint().height()*2 );
    setAlignment( AlignCenter );
    mySecret = secret;
}

SecretSource::~SecretSource()
{
}

/* XPM */
static const char * picture_xpm[] = {
"16 16 3 1",
" 	c None",
".	c #000000",
"X	c #FFFF00",
"     .....      ",
"   ..XXXXX..    ",
"  .XXXXXXXXX.   ",
" .XXXXXXXXXXX.  ",
" .XX..XXX..XX.  ",
".XXXXXXXXXXXXX. ",
".XX...XXX...XX. ",
".XXX..XXX..XXX. ",
".XXXXXXXXXXXXX. ",
".XXXXXX.XXXXXX. ",
" .XX.XX.XX.XX.  ",
" .XXX..X..XXX.  ",
"  .XXXXXXXXX.   ",
"   ..XXXXX..    ",
"     .....      ",
"                "};

void SecretSource::mousePressEvent( QMouseEvent * /*e*/ )
{
    SecretDrag *sd = new SecretDrag( mySecret, this );
    sd->setPixmap(QPixmap(picture_xpm),QPoint(8,8));
    sd->dragCopy();
    mySecret++;
}
