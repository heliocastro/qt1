/****************************************************************************
** $Id: qpixmapcache.h,v 2.5.2.1 1998/08/19 16:02:32 agulbra Exp $
**
** Definition of QPixmapCache class
**
** Created : 950501
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

#ifndef QPIXMAPCACHE_H
#define QPIXMAPCACHE_H

#ifndef QT_H
#include "qpixmap.h"
#endif // QT_H


class Q_EXPORT QPixmapCache				// global pixmap cache
{
public:
    static  int		cacheLimit();
    static  void	setCacheLimit( int );
    static  QPixmap    *find( const char *key );
    static  bool	find( const char *key, QPixmap& );
    static  bool	insert( const char *key, QPixmap * );
    static  void	insert( const char *key, const QPixmap& );
    static  void	clear();
};


#endif // QPIXMAPCACHE_H
