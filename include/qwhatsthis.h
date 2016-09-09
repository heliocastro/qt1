/****************************************************************************
** $Id: qwhatsthis.h,v 2.5.2.1 1998/08/19 16:02:44 agulbra Exp $
**
** Definition of QWhatsThis class
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

#ifndef QWHATSTHIS_H
#define QWHATSTHIS_H

#ifndef QT_H
#include "qobject.h"
#endif // QT_H

class QToolButton;
class QPopupMenu;

// it's a class! it's a struct! it's a namespace! IT'S WHATS THIS?!
class Q_EXPORT QWhatsThis
{
public:
    static void add( QWidget *, const char *, bool deepCopy = TRUE );
    static void add( QWidget *, const QPixmap &,
		     const char *, const char *, bool deepCopy = TRUE );
    static void remove( QWidget * );
    static const char * textFor( QWidget * );

    static QToolButton * whatsThisButton( QWidget * parent );
    //static void enterWhatsThisMode();

    //static void say( const char *, QWidget * near );

    //static int addMenuEntry( QPopupMenu *, QWidget *, const char * = 0 );
};

#endif
