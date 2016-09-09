/****************************************************************************
** $Id: dirview.h,v 1.5 1998/05/21 19:24:51 agulbra Exp $
**
** Copyright (C) 1992-1998 Troll Tech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#ifndef DIRVIEW_H
#define DIRVIEW_H

#include <qlistview.h>
#include <qstring.h>
#include <qfile.h>

class Directory: public QListViewItem
{
public:
    Directory( QListView * parent );
    Directory( Directory * parent, const char * filename );

    const char * text( int column ) const;

    QString fullName();

    void setOpen( bool );
    void setup();

private:
    QFile f;
    Directory * p;
    bool readable;
};


#endif
