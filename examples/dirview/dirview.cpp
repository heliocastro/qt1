/****************************************************************************
** $Id: dirview.cpp,v 1.9 1998/06/16 11:39:32 warwick Exp $
**
** Copyright (C) 1992-1998 Troll Tech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#include "dirview.h"
#include <qdir.h>
#include <qfile.h>
#include <qfileinfo.h>

Directory::Directory( Directory * parent, const char * filename )
    : QListViewItem( parent ), f(filename )
{
    p = parent;
    readable = TRUE;
}


Directory::Directory( QListView * parent )
    : QListViewItem( parent ), f("/")
{
    p = 0;
    readable = TRUE;
}


void Directory::setOpen( bool o )
{
    if ( o && !childCount() ) {
	QString s( fullName() );
	QDir thisDir( s );
	if ( !thisDir.isReadable() ) {
	    readable = FALSE;
	    return;
	}

	const QFileInfoList * files = thisDir.entryInfoList();
	if ( files ) {
	    QFileInfoListIterator it( *files );
	    QFileInfo * f;
	    while( (f=it.current()) != 0 ) {
		++it;
		if ( f->fileName() == "." || f->fileName() == ".." )
		    ; // nothing
		else if ( f->isSymLink() )
		    new QListViewItem( this, (const char *)f->fileName(),
				       "Symbolic Link", 0 );
		else if ( f->isDir() )
		    new Directory( this, f->fileName() );
		else
		    new QListViewItem( this, (const char *)f->fileName(),
				       f->isFile() ? "File" : "Special", 0 );
	    }
	}
    }
    QListViewItem::setOpen( o );
}


void Directory::setup()
{
    setExpandable( TRUE );
    QListViewItem::setup();
}


QString Directory::fullName()
{
    QString s;
    if ( p ) {
	s = p->fullName();
	s.append( f.name() );
	s.append( "/" );
    } else {
	s = "/";
    }
    return s;
}


const char * Directory::text( int column ) const
{
    if ( column == 0 )
	return f.name();
    else if ( readable )
	return "Directory";
    else
	return "Unreadable Directory";
}
