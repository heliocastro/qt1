/****************************************************************************
** $Id: qpicture.h,v 2.6.2.2 1998/08/21 19:13:23 hanord Exp $
**
** Definition of QPicture class
**
** Created : 940729
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

#ifndef QPICTURE_H
#define QPICTURE_H

#ifndef QT_H
#include "qpaintdevice.h"
#include "qbuffer.h"
#endif // QT_H


class Q_EXPORT QPicture : public QPaintDevice		// picture class
{
public:
    QPicture();
   ~QPicture();

    bool	isNull() const;

    uint	size() const;
    const char *data() const;
    void	setData( const char *data, uint size );

    bool	play( QPainter * );

    bool	load( const char *fileName );
    bool	save( const char *fileName );

protected:
    bool	cmd( int, QPainter *, QPDevCmdParam * );
    int		metric( int ) const;

private:
    bool	exec( QPainter *, QDataStream &, int );
    QBuffer	pictb;
    int		trecs;
    bool	formatOk;

private:	// Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QPicture( const QPicture & );
    QPicture &operator=( const QPicture & );
#endif
};


inline bool QPicture::isNull() const
{
    return pictb.buffer().isNull();
}

inline uint QPicture::size() const
{
    return pictb.buffer().size();
}

inline const char *QPicture::data() const
{
    return pictb.buffer().data();
}


#endif // QPICTURE_H
