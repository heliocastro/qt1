/****************************************************************************
** $Id: qgdict.h,v 2.6.2.2 1998/11/26 11:14:53 aavit Exp $
**
** Definition of QGDict and QGDictIterator classes
**
** Created : 920529
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

#ifndef QGDICT_H
#define QGDICT_H

#ifndef QT_H
#include "qcollection.h"
#endif // QT_H


class QBucket;					// internal classes
class QListM_QGDictIterator;
#define QGDItList QListM_QGDictIterator


class Q_EXPORT QGDict : public QCollection		// generic dictionary class
{
friend class QGDictIterator;
public:
    uint	count() const	{ return numItems; }
    uint	size()	const	{ return vlen; }
    GCI		look( const char *key, GCI, int );

    QDataStream &read( QDataStream & );
    QDataStream &write( QDataStream & ) const;

protected:
    QGDict( uint len, bool cs, bool ck, bool th );
    QGDict( const QGDict & );
   ~QGDict();

    QGDict     &operator=( const QGDict & );

    bool	remove( const char *key );
    bool	removeItem( const char *key, GCI item );
    GCI		take( const char *key );
    void	clear();
    void	resize( uint );

    virtual int hashKey( const char * );

    void	statistics() const;

    virtual QDataStream &read( QDataStream &, GCI & );
    virtual QDataStream &write( QDataStream &, GCI ) const;

private:
    QBucket   **vec;
    uint	vlen;
    uint	numItems;
    uint	cases	: 1;
    uint	copyk	: 1;
    uint	triv	: 1;
    QGDItList  *iterators;
    QBucket    *unlink( const char *, GCI item = 0 );
    void        init( uint );
};


class Q_EXPORT QGDictIterator				// generic dictionary iterator
{
friend class QGDict;
public:
    QGDictIterator( const QGDict & );
    QGDictIterator( const QGDictIterator & );
    QGDictIterator &operator=( const QGDictIterator & );
   ~QGDictIterator();

    GCI		toFirst();

    GCI		get()	 const;
    const char *getKey() const;

    GCI		operator()();
    GCI		operator++();
    GCI		operator+=(uint);

protected:
    QGDict     *dict;

private:
    QBucket    *curNode;
    uint	curIndex;
};


#endif // QGDICT_H
