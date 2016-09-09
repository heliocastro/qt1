/****************************************************************************
** $Id: qstrvec.h,v 2.5.2.3 1998/08/25 09:20:55 hanord Exp $
**
** Definition of QStrVec and QStrIVec classes
**
** Created : 931203
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

#ifndef QSTRVEC_H
#define QSTRVEC_H

#ifndef QT_H
#include "qstring.h"
#include "qvector.h"
#include "qdatastream.h"
#endif // QT_H


#if defined(DEFAULT_TEMPLATECLASS)
#if defined(Q_TEMPLATEDLL)
template class Q_EXPORT QVector<char>
#endif
typedef QVector<char> QStrVecBase;
#else
typedef Q_DECLARE(QVectorM,char) QStrVecBase;
#endif


class Q_EXPORT QStrVec : public QStrVecBase
{
public:
    QStrVec()  { dc = TRUE; }
    QStrVec( uint size, bool deepc = TRUE ) : QStrVecBase(size) {dc=deepc;}
   ~QStrVec()  { clear(); }
private:
    GCI	 newItem( GCI d )	{ return dc ? qstrdup( (const char*)d ) : d; }
    void deleteItem( GCI d )	{ if ( dc ) delete[] (char*)d; }
    int	 compareItems( GCI s1, GCI s2 )
				{ return strcmp((const char*)s1,
						(const char*)s2); }
    QDataStream &read( QDataStream &s, GCI &d )
				{ s >> (char *&)d; return s; }
    QDataStream &write( QDataStream &s, GCI d ) const
				{ return s << (const char *)d; }
    bool dc;
};


class Q_EXPORT QStrIVec : public QStrVec	// case insensitive string vec
{
public:
    QStrIVec() {}
    QStrIVec( uint size, bool dc = TRUE ) : QStrVec( size, dc ) {}
   ~QStrIVec() { clear(); }
private:
    int	 compareItems( GCI s1, GCI s2 )
				{ return stricmp((const char*)s1,
						 (const char*)s2); }
};


#endif // QSTRVEC_H
