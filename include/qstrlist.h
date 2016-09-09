/****************************************************************************
** $Id: qstrlist.h,v 2.12.2.3 1998/08/25 09:20:55 hanord Exp $
**
** Definition of QStrList, QStrIList and QStrListIterator classes
**
** Created : 920730
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

#ifndef QSTRLIST_H
#define QSTRLIST_H

#ifndef QT_H
#include "qstring.h"
#include "qlist.h"
#include "qdatastream.h"
#endif // QT_H


#if defined(DEFAULT_TEMPLATECLASS)
#if defined(Q_TEMPLATEDLL)
template class Q_EXPORT QList<char>;
template class Q_EXPORT QListIterator<char>;
#endif
typedef QList<char>			QStrListBase;
typedef QListIterator<char>		QStrListIterator;
#else
typedef Q_DECLARE(QListM,char)		QStrListBase;
typedef Q_DECLARE(QListIteratorM,char)	QStrListIterator;
#endif


class Q_EXPORT QStrList : public QStrListBase
{
public:
    QStrList( bool deepCopies=TRUE ) { dc = deepCopies; }
    QStrList( const QStrList & );
   ~QStrList()			{ clear(); }
    QStrList& operator=( const QStrList & );

private:
    GCI	  newItem( GCI d )	{ return dc ? qstrdup( (const char*)d ) : d; }
    void  deleteItem( GCI d )	{ if ( dc ) delete[] (char*)d; }
    int	  compareItems( GCI s1, GCI s2 )
				{ return strcmp((const char*)s1,
						(const char*)s2); }
    QDataStream &read( QDataStream &s, GCI &d )
				{ s >> (char *&)d; return s; }
    QDataStream &write( QDataStream &s, GCI d ) const
				{ return s << (const char *)d; }
    bool  dc;
};


class Q_EXPORT QStrIList : public QStrList	// case insensitive string list
{
public:
    QStrIList( bool deepCopies=TRUE ) : QStrList( deepCopies ) {}
   ~QStrIList()			{ clear(); }
private:
    int	  compareItems( GCI s1, GCI s2 )
				{ return stricmp((const char*)s1,
						 (const char*)s2); }
};


inline QStrList & QStrList::operator=( const QStrList &strList )
{
    clear();
    dc = strList.dc;
    QStrListBase::operator=(strList);
    return *this;
}

inline QStrList::QStrList( const QStrList &strList )
    : QStrListBase( strList )
{
    dc = FALSE;
    operator=(strList);
}


#endif // QSTRLIST_H
