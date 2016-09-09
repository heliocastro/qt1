/****************************************************************************
** $Id: qdragapp.h,v 2.3 1998/06/16 11:39:35 warwick Exp $
**
** Copyright (C) 1992-1998 Troll Tech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#ifndef QDRAGAPP_H
#define QDRAGAPP_H

#include "qapplication.h"

class QDragger;

class QDragApplication : public QApplication
{
    Q_OBJECT
public:
    QDragApplication( int &argc, char **argv );
    virtual ~QDragApplication();

    virtual bool notify( QObject *, QEvent * ); // event filter

private:
    QDragger *dragger;
};


#endif // QDRAGAPP_H
