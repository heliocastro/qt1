/****************************************************************************
** $Id: qiconset.h,v 2.4.2.2 1998/10/21 14:00:14 agulbra Exp $
**
** Definition of QIconSet class
**
** Created : 980318
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

#ifndef QICONSET_H
#define QICONSET_H

#include "qpixmap.h"


struct QIconSetPrivate;


class Q_EXPORT QIconSet
{
public:
    enum Size { Automatic, Small, Large };

    enum Mode { Normal, Disabled, Active };

    QIconSet( const QPixmap &, Size = Automatic );
    QIconSet( const QIconSet & );
    virtual ~QIconSet();

    void reset( const QPixmap &, Size );

    void setPixmap( const QPixmap &, Size, Mode = Normal );
    void setPixmap( const char *, Size, Mode = Normal );
    QPixmap pixmap( Size, Mode ) const;
    QPixmap pixmap() const;
    bool isGenerated( Size, Mode ) const;

    void detach();

    QIconSet &operator=( const QIconSet & );

private:
    QIconSetPrivate * d;
};


#endif
