/****************************************************************************
** $Id: qglist.h,v 2.4.2.2 1998/08/25 09:20:54 hanord Exp $
**
** Definition of QGList and QGListIterator classes
**
** Created : 920624
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

#ifndef QGLIST_H
#define QGLIST_H

#ifndef QT_H
#include "qcollection.h"
#endif // QT_H


/*****************************************************************************
  QLNode class (internal doubly linked list node)
 *****************************************************************************/

class Q_EXPORT QLNode
{
friend class QGList;
friend class QGListIterator;
public:
    GCI	    getData()	{ return data; }
private:
    GCI	    data;
    QLNode *prev;
    QLNode *next;
    QLNode( GCI d )	{ data = d; }
};


/*****************************************************************************
  QGList class
 *****************************************************************************/

class Q_EXPORT QGList : public QCollection	// doubly linked generic list
{
friend class QGListIterator;
friend class QGVector;				// needed by QGVector::toList
public:
    uint  count() const;			// return number of nodes

    QDataStream &read( QDataStream & );		// read list from stream
    QDataStream &write( QDataStream & ) const;	// write list to stream

protected:
    QGList();					// create empty list
    QGList( const QGList & );			// make copy of other list
   ~QGList();

    QGList &operator=( const QGList & );	// assign from other list

    void  inSort( GCI );			// add item sorted in list
    void  append( GCI );			// add item at end of list
    bool  insertAt( uint index, GCI );		// add item at i'th position
    void  relinkNode( QLNode * );		// relink as first item
    bool  removeNode( QLNode * );		// remove node
    bool  remove( GCI = 0 );			// remove item (0=current)
    bool  removeRef( GCI = 0 );			// remove item (0=current)
    bool  removeFirst();			// remove first item
    bool  removeLast();				// remove last item
    bool  removeAt( uint index );		// remove item at i'th position
    GCI	  takeNode( QLNode * );			// take out node
    GCI	  take();				// take out current item
    GCI	  takeAt( uint index );			// take out item at i'th pos
    GCI	  takeFirst();				// take out first item
    GCI	  takeLast();				// take out last item

    void  clear();				// remove all items

    int	  findRef( GCI, bool = TRUE );		// find exact item in list
    int	  find( GCI, bool = TRUE );		// find equal item in list

    uint  containsRef( GCI )	const;		// get number of exact matches
    uint  contains( GCI )	const;		// get number of equal matches

    GCI	  at( uint index );			// access item at i'th pos
    int	  at() const;				// get current index
    QLNode *currentNode() const;		// get current node

    GCI	  get() const;				// get current item

    GCI	  cfirst() const;			// get ptr to first list item
    GCI	  clast()  const;			// get ptr to last list item
    GCI	  first();				// set first item in list curr
    GCI	  last();				// set last item in list curr
    GCI	  next();				// set next item in list curr
    GCI	  prev();				// set prev item in list curr

    void  toVector( QGVector * ) const;		// put items in vector

    virtual int compareItems( GCI, GCI );

    virtual QDataStream &read( QDataStream &, GCI & );
    virtual QDataStream &write( QDataStream &, GCI ) const;

private:
    void  prepend( GCI );			// add item at start of list

    QLNode *firstNode;				// first node
    QLNode *lastNode;				// last node
    QLNode *curNode;				// current node
    int	    curIndex;				// current index
    uint    numNodes;				// number of nodes
    QGList *iterators;				// list of iterators

    QLNode *locate( uint );			// get node at i'th pos
    QLNode *unlink();				// unlink node
};


inline uint QGList::count() const
{
    return numNodes;
}

inline bool QGList::removeFirst()
{
    first();
    return remove();
}

inline bool QGList::removeLast()
{
    last();
    return remove();
}

inline int QGList::at() const
{
    return curIndex;
}

inline GCI QGList::at( uint index )
{
    QLNode *n = locate( index );
    return n ? n->data : 0;
}

inline QLNode *QGList::currentNode() const
{
    return curNode;
}

inline GCI QGList::get() const
{
    return curNode ? curNode->data : 0;
}

inline GCI QGList::cfirst() const
{
    return firstNode ? firstNode->data : 0;
}

inline GCI QGList::clast() const
{
    return lastNode ? lastNode->data : 0;
}


/*****************************************************************************
  QGList stream functions
 *****************************************************************************/

Q_EXPORT QDataStream &operator>>( QDataStream &, QGList & );
Q_EXPORT QDataStream &operator<<( QDataStream &, const QGList & );


/*****************************************************************************
  QGListIterator class
 *****************************************************************************/

class Q_EXPORT QGListIterator			// QGList iterator
{
friend class QGList;
protected:
    QGListIterator( const QGList & );
    QGListIterator( const QGListIterator & );
    QGListIterator &operator=( const QGListIterator & );
   ~QGListIterator();

    bool  atFirst() const;			// test if at first item
    bool  atLast()  const;			// test if at last item
    GCI	  toFirst();				// move to first item
    GCI	  toLast();				// move to last item

    GCI	  get() const;				// get current item
    GCI	  operator()();				// get current and move to next
    GCI	  operator++();				// move to next item (prefix)
    GCI	  operator+=(uint);			// move n positions forward
    GCI	  operator--();				// move to prev item (prefix)
    GCI	  operator-=(uint);			// move n positions backward

protected:
    QGList *list;				// reference to list

private:
    QLNode  *curNode;				// current node in list
};


inline bool QGListIterator::atFirst() const
{
    return curNode == list->firstNode;
}

inline bool QGListIterator::atLast() const
{
    return curNode == list->lastNode;
}

inline GCI QGListIterator::get() const
{
    return curNode ? curNode->data : 0;
}


#endif	// QGLIST_H
