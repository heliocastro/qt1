/****************************************************************************
** $Id: qdropsite.h,v 2.2.2.2 1998/08/20 16:20:29 paul Exp $
**
** Definitation of Drag and Drop support
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

#ifndef QDROPSITE_H
#define QDROPSITE_H

class QDropSitePrivate;
class QWidget;
class QDragEnterEvent;
class QDragMoveEvent;
class QDragLeaveEvent;
class QDropEvent;

#include <qglobal.h>

class Q_EXPORT QDropSite {
public:
    QDropSite( QWidget* parent );
    virtual ~QDropSite();

    virtual void dragEnterEvent( QDragEnterEvent * );
    virtual void dragMoveEvent( QDragMoveEvent * );
    virtual void dragLeaveEvent( QDragLeaveEvent * );
    virtual void dropEvent( QDropEvent * );

private:
    QDropSitePrivate *d;
};

#endif
