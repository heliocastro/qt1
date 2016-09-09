/****************************************************************************
** $Id: secret.h,v 1.2 1998/07/08 15:26:48 agulbra Exp $
**
** Custom MIME type implementation example
**
** Created : 979899
**
** Copyright (C) 1997 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#ifndef SECRETDRAG_H
#define SECRETDRAG_H

#include <qdragobject.h>
#include <qlabel.h>

class SecretDrag: public QStoredDrag {
public:
    SecretDrag( uchar, QWidget * parent = 0, const char * name = 0 );
    ~SecretDrag() {};

    static bool canDecode( QDragMoveEvent* e );
    static bool decode( QDropEvent* e, QString& s );
};


class SecretSource: public QLabel
{
public:
    SecretSource( int secret, QWidget *parent = 0, const char * name = 0 );
    ~SecretSource();

protected:
    void mousePressEvent( QMouseEvent * );
private:
    int mySecret;
};

#endif
