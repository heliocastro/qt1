/****************************************************************************
** $Id: qbitmap.h,v 2.3.2.2 1998/08/25 09:20:51 hanord Exp $
**
** Definition of QBitmap class
**
** Created : 941020
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

#ifndef QBITMAP_H
#define QBITMAP_H

#ifndef QT_H
#include "qpixmap.h"
#endif // QT_H


class Q_EXPORT QBitmap : public QPixmap
{
public:
    QBitmap();
    QBitmap( int w, int h,  bool clear = FALSE );
    QBitmap( const QSize &, bool clear = FALSE	);
    QBitmap( int w, int h,  const uchar *bits, bool isXbitmap=FALSE );
    QBitmap( const QSize &, const uchar *bits, bool isXbitmap=FALSE );
    QBitmap( const QBitmap & );
    QBitmap( const char *fileName, const char *format=0 );

    QBitmap &operator=( const QBitmap & );
    QBitmap &operator=( const QPixmap & );
    QBitmap &operator=( const QImage  & );

    QBitmap  xForm( const QWMatrix & ) const;
};


#endif // QBITMAP_H
