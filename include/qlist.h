/****************************************************************************
** $Id: qlist.h,v 2.5.2.3 1998/08/27 08:14:01 hanord Exp $
**
** Definition of QList template/macro class
**
** Created : 920701
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

#ifndef QLIST_H
#define QLIST_H

#ifndef QT_H
#include "qglist.h"
#endif // QT_H


#if defined(USE_MACROCLASS)

#include "qgeneric.h"

#if !defined(name2)
#define name2(a,b)    name2_xx(a,b)
#define name2_xx(a,b) a##b
#endif

#if defined(DEFAULT_MACROCLASS)
#define QListdeclare QListMdeclare
#define QList QListM
#endif
#define QListM(type) name2(QListM_,type)

#define QListMdeclare(type)						      \
class Q_EXPORTM QListM(type) : public QGList				      \
{									      \
public:									      \
    QListM(type)()			{}				      \
    QListM(type)( const QListM(type) &l ) : QGList(l) {}		      \
   ~QListM(type)()			{ clear(); }			      \
    QListM(type) &operator=(const QListM(type) &l)			      \
			{ return (QListM(type)&)QGList::operator=(l); }	      \
    uint  count()   const		{ return QGList::count(); }	      \
    bool  isEmpty() const		{ return QGList::count() == 0; }      \
    bool  insert( uint i, const type *d){ return QGList::insertAt(i,(GCI)d); }\
    void  inSort( const type *d )	{ QGList::inSort((GCI)d); }	      \
    void  append( const type *d )	{ QGList::append((GCI)d); }	      \
    bool  remove( uint i )		{ return QGList::removeAt(i); }	      \
    bool  remove()			{ return QGList::remove((GCI)0); }    \
    bool  remove( const type *d )	{ return QGList::remove((GCI)d); }    \
    bool  removeRef( const type *d )	{ return QGList::removeRef((GCI)d); } \
    void  removeNode( QLNode *n )	{ QGList::removeNode(n); }	      \
    bool  removeFirst()			{ return QGList::removeFirst(); }     \
    bool  removeLast()			{ return QGList::removeLast(); }      \
    type *take( uint i )		{ return (type *)QGList::takeAt(i); } \
    type *take()			{ return (type *)QGList::take(); }    \
    type *takeNode( QLNode *n )		{ return (type *)QGList::takeNode(n);}\
    void  clear()			{ QGList::clear(); }		      \
    int	  find( const type *d )		{ return QGList::find((GCI)d); }      \
    int	  findNext( const type *d )	{ return QGList::find((GCI)d,FALSE);} \
    int	  findRef( const type *d )	{ return QGList::findRef((GCI)d); }   \
    int	  findNextRef( const type *d ){ return QGList::findRef((GCI)d,FALSE);}\
    uint  contains( const type *d ) const { return QGList::contains((GCI)d); }\
    uint  containsRef( const type *d ) const				      \
					{ return QGList::containsRef((GCI)d);}\
    type *at( uint i )			{ return (type *)QGList::at(i); }     \
    int	  at() const			{ return QGList::at(); }	      \
    type *current()  const		{ return (type *)QGList::get(); }     \
    QLNode *currentNode()  const	{ return QGList::currentNode(); }     \
    type *getFirst() const		{ return (type *)QGList::cfirst(); }  \
    type *getLast()  const		{ return (type *)QGList::clast(); }   \
    type *first()			{ return (type *)QGList::first(); }   \
    type *last()			{ return (type *)QGList::last(); }    \
    type *next()			{ return (type *)QGList::next(); }    \
    type *prev()			{ return (type *)QGList::prev(); }    \
    void  toVector( QGVector *vec )const{ QGList::toVector(vec); }	      \
private:								      \
    void  deleteItem( GCI d ) { if ( del_item ) delete (type *)d; }	      \
}


#if defined(DEFAULT_MACROCLASS)
#define QListIteratordeclare QListIteratorMdeclare
#define QListIterator QListIteratorM
#endif
#define QListIteratorM(type) name2(QListIteratorM_,type)

#define QListIteratorMdeclare(type)					      \
class Q_EXPORTM QListIteratorM(type) : public QGListIterator		      \
{									      \
public:									      \
    QListIteratorM(type)(const QListM(type) &l) :QGListIterator((QGList &)l){}\
   ~QListIteratorM(type)()    {}					      \
    uint  count()   const     { return list->count(); }			      \
    bool  isEmpty() const     { return list->count() == 0; }		      \
    bool  atFirst() const     { return QGListIterator::atFirst(); }	      \
    bool  atLast()  const     { return QGListIterator::atLast(); }	      \
    type *toFirst()	      { return (type *)QGListIterator::toFirst(); }   \
    type *toLast()	      { return (type *)QGListIterator::toLast(); }    \
    operator type *() const   { return (type *)QGListIterator::get(); }	      \
    type *current()   const   { return (type *)QGListIterator::get(); }	      \
    type *operator()()	      { return (type *)QGListIterator::operator()();} \
    type *operator++()	      { return (type *)QGListIterator::operator++(); }\
    type *operator+=(uint j)  { return (type *)QGListIterator::operator+=(j);}\
    type *operator--()	      { return (type *)QGListIterator::operator--(); }\
    type *operator-=(uint j)  { return (type *)QGListIterator::operator-=(j);}\
    QListIteratorM(type)& operator=(const QListIteratorM(type)&it)            \
			      { QGListIterator::operator=(it); return *this; }\
}

#endif // USE_MACROCLASS


#if defined(USE_TEMPLATECLASS)

#if defined(DEFAULT_TEMPLATECLASS)
#undef	QList
#define QList QListT
#endif

template<class type> class Q_EXPORT QListT : public QGList
{
public:
    QListT()				{}
    QListT( const QListT<type> &l ) : QGList(l) {}
   ~QListT()				{ clear(); }
    QListT<type> &operator=(const QListT<type> &l)
			{ return (QListT<type>&)QGList::operator=(l); }
    uint  count()   const		{ return QGList::count(); }
    bool  isEmpty() const		{ return QGList::count() == 0; }
    bool  insert( uint i, const type *d){ return QGList::insertAt(i,(GCI)d); }
    void  inSort( const type *d )	{ QGList::inSort((GCI)d); }
    void  append( const type *d )	{ QGList::append((GCI)d); }
    bool  remove( uint i )		{ return QGList::removeAt(i); }
    bool  remove()			{ return QGList::remove((GCI)0); }
    bool  remove( const type *d )	{ return QGList::remove((GCI)d); }
    bool  removeRef( const type *d )	{ return QGList::removeRef((GCI)d); }
    void  removeNode( QLNode *n )	{ QGList::removeNode(n); }
    bool  removeFirst()			{ return QGList::removeFirst(); }
    bool  removeLast()			{ return QGList::removeLast(); }
    type *take( uint i )		{ return (type *)QGList::takeAt(i); }
    type *take()			{ return (type *)QGList::take(); }
    type *takeNode( QLNode *n )		{ return (type *)QGList::takeNode(n); }
    void  clear()			{ QGList::clear(); }
    int	  find( const type *d )		{ return QGList::find((GCI)d); }
    int	  findNext( const type *d )	{ return QGList::find((GCI)d,FALSE); }
    int	  findRef( const type *d )	{ return QGList::findRef((GCI)d); }
    int	  findNextRef( const type *d ){ return QGList::findRef((GCI)d,FALSE);}
    uint  contains( const type *d ) const { return QGList::contains((GCI)d); }
    uint  containsRef( const type *d ) const
					{ return QGList::containsRef((GCI)d); }
    type *at( uint i )			{ return (type *)QGList::at(i); }
    int	  at() const			{ return QGList::at(); }
    type *current()  const		{ return (type *)QGList::get(); }
    QLNode *currentNode()  const	{ return QGList::currentNode(); }
    type *getFirst() const		{ return (type *)QGList::cfirst(); }
    type *getLast()  const		{ return (type *)QGList::clast(); }
    type *first()			{ return (type *)QGList::first(); }
    type *last()			{ return (type *)QGList::last(); }
    type *next()			{ return (type *)QGList::next(); }
    type *prev()			{ return (type *)QGList::prev(); }
    void  toVector( QGVector *vec )const{ QGList::toVector(vec); }
private:
    void  deleteItem( GCI d ) { if ( del_item ) delete (type *)d; }
};


#if defined(DEFAULT_TEMPLATECLASS)
#undef	QListIterator
#define QListIterator QListIteratorT
#endif

template<class type> class Q_EXPORT QListIteratorT : public QGListIterator
{
public:
    QListIteratorT(const QListT<type> &l) :QGListIterator((QGList &)l) {}
   ~QListIteratorT()	      {}
    uint  count()   const     { return list->count(); }
    bool  isEmpty() const     { return list->count() == 0; }
    bool  atFirst() const     { return QGListIterator::atFirst(); }
    bool  atLast()  const     { return QGListIterator::atLast(); }
    type *toFirst()	      { return (type *)QGListIterator::toFirst(); }
    type *toLast()	      { return (type *)QGListIterator::toLast(); }
    operator type *() const   { return (type *)QGListIterator::get(); }
    type *current()   const   { return (type *)QGListIterator::get(); }
    type *operator()()	      { return (type *)QGListIterator::operator()();}
    type *operator++()	      { return (type *)QGListIterator::operator++(); }
    type *operator+=(uint j)  { return (type *)QGListIterator::operator+=(j);}
    type *operator--()	      { return (type *)QGListIterator::operator--(); }
    type *operator-=(uint j)  { return (type *)QGListIterator::operator-=(j);}
    QListIteratorT<type>& operator=(const QListIteratorT<type>&it)
			      { QGListIterator::operator=(it); return *this; }
};


#endif // USE_TEMPLATECLASS


#endif // QLIST_H
