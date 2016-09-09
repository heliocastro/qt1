/****************************************************************************
** $Id: qdropsite.cpp,v 2.3 1998/07/03 00:09:32 hanord Exp $
**
** Implementation of Drag and Drop support
**
** Copyright (C) 1992-1999 Troll Tech AS.  All rights reserved.
**
** This file is part of Qt Free Edition, version 1.45.
**
** See the file LICENSE included in the distribution for the usage
** and distribution terms, or http://www.troll.no/free-license.html.
**
** IMPORTANT NOTE: You may NOT copy this file or any part of it into
** your own programs or libraries.
**
** Please see http://www.troll.no/pricing.html for information about 
** Qt Professional Edition, which is this same library but with a
** license which allows creation of commercial/proprietary software.
**
*****************************************************************************/

#include "qdropsite.h"
#include "qwidget.h"

class QDropSitePrivate : public QObject {
    QDropSite* s;

public:
    QDropSitePrivate( QWidget* parent, QDropSite* site ) :
	QObject(parent),
	s(site)
    {
	parent->installEventFilter(this);
    }

    bool eventFilter( QObject*, QEvent* );
};

bool QDropSitePrivate::eventFilter( QObject *, QEvent * e )
{
    if ( e->type() == Event_Drop ) {
	s->dropEvent( (QDropEvent *)e );
	return TRUE;
    } else if ( e->type() == Event_DragEnter ) {
	s->dragEnterEvent( (QDragEnterEvent *)e );
	return TRUE;
    } else if ( e->type() == Event_DragMove ) {
	s->dragMoveEvent( (QDragMoveEvent *)e );
	return TRUE;
    } else if ( e->type() == Event_DragLeave ) {
	s->dragLeaveEvent( (QDragLeaveEvent *)e );
	return TRUE;
    } else {
	return FALSE;
    }
}


QDropSite::QDropSite( QWidget* parent )
{
    d = new QDropSitePrivate(parent,this);
    parent->setAcceptDrops( TRUE );
}

QDropSite::~QDropSite()
{
    delete d; // not really needed
}

void QDropSite::dragEnterEvent( QDragEnterEvent * )
{
}

void QDropSite::dragMoveEvent( QDragMoveEvent * )
{
}

void QDropSite::dragLeaveEvent( QDragLeaveEvent * )
{
}

void QDropSite::dropEvent( QDropEvent * )
{
}
