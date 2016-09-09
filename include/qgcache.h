/****************************************************************************
** $Id: qgcache.h,v 2.4.2.1 1998/08/19 16:02:36 agulbra Exp $
**
** Definition of QGCache and QGCacheIterator classes
**
** Created : 950208
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

#ifndef QGCACHE_H
#define QGCACHE_H

#ifndef QT_H
#include "qcollection.h"
#include "qglist.h"
#include "qgdict.h"
#endif // QT_H


class QCList;					// internal classes
class QCDict;


/*****************************************************************************
  QGCache class
 *****************************************************************************/

class Q_EXPORT QGCache : public QCollection		// LRU cache class
{
friend class QGCacheIterator;
protected:
    QGCache( int maxCost, uint size,bool caseS, bool copyKeys, bool trivial );
    QGCache( const QGCache & );			// not allowed, calls fatal()
   ~QGCache();
    QGCache &operator=( const QGCache & );	// not allowed, calls fatal()

    uint    count()	const	{ return ((QGDict*)dict)->count(); }
    uint    size()	const	{ return ((QGDict*)dict)->size(); }
    int	    maxCost()	const	{ return mCost; }
    int	    totalCost() const	{ return tCost; }
    void    setMaxCost( int maxCost );

    bool    insert( const char *key, GCI, int cost, int priority );
    bool    remove( const char *key );
    GCI	    take( const char *key );
    void    clear();

    GCI	    find( const char *key, bool ref=TRUE ) const;

    void    statistics() const;			// output debug statistics

private:
    bool    makeRoomFor( int cost, int priority = -1 );
    QCList *lruList;
    QCDict *dict;
    int	    mCost;
    int	    tCost;
    bool    copyK;
};


/*****************************************************************************
  QGCacheIterator class
 *****************************************************************************/

class QListIteratorM_QCacheItem;

class Q_EXPORT QGCacheIterator				// QGCache iterator
{
protected:
    QGCacheIterator( const QGCache & );
    QGCacheIterator( const QGCacheIterator & );
   ~QGCacheIterator();
    QGCacheIterator &operator=( const QGCacheIterator & );

    uint  count()   const;			// number of items in cache
    bool  atFirst() const;			// test if at first item
    bool  atLast()  const;			// test if at last item
    GCI	  toFirst();				// move to first item
    GCI	  toLast();				// move to last item

    GCI	  get() const;				// get current item
    const char *getKey() const;			// get current key
    GCI	  operator()();				// get current and move to next
    GCI	  operator++();				// move to next item (prefix)
    GCI	  operator+=( uint );			// move n positions forward
    GCI	  operator--();				// move to prev item (prefix)
    GCI	  operator-=( uint );			// move n positions backward

protected:
    QListIteratorM_QCacheItem *it;		// iterator on cache list
};


#endif // QGCACHE_H
