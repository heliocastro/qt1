/****************************************************************************
** $Id: qcache.h,v 2.3.2.2 1998/08/27 08:14:00 hanord Exp $
**
** Definition of QCache template/macro class
**
** Created : 950209
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

#ifndef QCACHE_H
#define QCACHE_H

#ifndef QT_H
#include "qgcache.h"
#endif // QT_H


#if defined(USE_MACROCLASS)

#include "qgeneric.h"

#if !defined(name2)
#define name2(a,b)    name2_xx(a,b)
#define name2_xx(a,b) a##b
#endif

#if defined(DEFAULT_MACROCLASS)
#define QCachedeclare QCacheMdeclare
#define QCache QCacheM
#endif
#define QCacheM(type) name2(QCacheM_,type)

#define QCacheMdeclare(type)						      \
class Q_EXPORTM QCacheM(type) : public QGCache				      \
{									      \
public:									      \
    QCacheM(type)( const QCacheM(type) &c ) : QGCache(c) {}		      \
    QCacheM(type)( int maxCost=100, int size=17, bool cs=TRUE,bool ck=TRUE )  \
	: QGCache( maxCost, size, cs, ck, FALSE ) {}			      \
   ~QCacheM(type)()		      { clear(); }			      \
    QCacheM(type) &operator=( const QCacheM(type) &c )			      \
			{ return (QCacheM(type)&)QGCache::operator=(c); }     \
    int	  maxCost()   const	      { return QGCache::maxCost(); }	      \
    int	  totalCost() const	      { return QGCache::totalCost(); }	      \
    void  setMaxCost( int m )	      { QGCache::setMaxCost(m); }	      \
    uint  count()     const	      { return QGCache::count(); }	      \
    uint  size()      const	      { return QGCache::size(); }	      \
    bool  isEmpty() const	      { return QGCache::count() == 0; }	      \
    bool  insert( const char *k, const type *d, int c=1, int p=0 )	      \
				      { return QGCache::insert(k,(GCI)d,c,p);}\
    bool  remove( const char *k )     { return QGCache::remove(k); }	      \
    type *take( const char *k )	      { return (type *)QGCache::take(k); }    \
    void  clear()		      { QGCache::clear(); }		      \
    type *find( const char *k, bool ref=TRUE ) const			      \
				       { return (type *)QGCache::find(k,ref);}\
    type *operator[]( const char *k ) const				      \
				      { return (type *)QGCache::find(k); }    \
    void  statistics() const	      { QGCache::statistics(); }	      \
private:								      \
    void  deleteItem( GCI d )	      { if ( del_item ) delete (type *)d; }   \
}


#if defined(DEFAULT_MACROCLASS)
#define QCacheIteratordeclare QCacheIteratorMdeclare
#define QCacheIterator QCacheIteratorM
#endif
#define QCacheIteratorM(type) name2(QCacheIteratorM_,type)

#define QCacheIteratorMdeclare(type)					      \
class Q_EXPORTM QCacheIteratorM(type) : public QGCacheIterator		      \
{									      \
public:									      \
    QCacheIteratorM(type)( const QCacheM(type) &c )			      \
			      : QGCacheIterator( (QGCache &)c ) {}	      \
    QCacheIteratorM(type)( const QCacheIteratorM(type) &ci )		      \
			      : QGCacheIterator( (QGCacheIterator &)ci ) {}   \
    QCacheIteratorM(type) &operator=(const QCacheIteratorM(type)&ci)	      \
	{ return (QCacheIteratorM(type)&)QGCacheIterator::operator=( ci ); }  \
    uint  count()   const    { return QGCacheIterator::count(); }	      \
    bool  isEmpty() const    { return QGCacheIterator::count() == 0; }	      \
    bool  atFirst() const    { return QGCacheIterator::atFirst(); }	      \
    bool  atLast()  const    { return QGCacheIterator::atLast(); }	      \
    type *toFirst()	     { return (type *)QGCacheIterator::toFirst(); }   \
    type *toLast()	     { return (type *)QGCacheIterator::toLast(); }    \
    operator type *() const  { return (type *)QGCacheIterator::get(); }	      \
    type *current()   const  { return (type *)QGCacheIterator::get(); }	      \
    const char *currentKey() const					      \
			     { return QGCacheIterator::getKey();}	      \
    type *operator()()	     { return (type *)QGCacheIterator::operator()();} \
    type *operator++()	     { return (type *)QGCacheIterator::operator++();} \
    type *operator+=(uint j) { return (type *)QGCacheIterator::operator+=(j);}\
    type *operator--()	     { return (type *)QGCacheIterator::operator--();} \
    type *operator-=(uint j) { return (type *)QGCacheIterator::operator-=(j);}\
}

#endif // USE_MACROCLASS


#if defined(USE_TEMPLATECLASS)

#if defined(DEFAULT_TEMPLATECLASS)
#undef	QCache
#define QCache QCacheT
#endif

template<class type> class Q_EXPORT QCacheT : public QGCache
{
public:
    QCacheT( const QCacheT<type> &c ) : QGCache(c) {}
    QCacheT( int maxCost=100, int size=17, bool cs=TRUE, bool ck=TRUE )
	: QGCache( maxCost, size, cs, ck, FALSE ) {}
   ~QCacheT()			      { clear(); }
    QCacheT<type> &operator=( const QCacheT<type> &c )
			{ return (QCacheT<type>&)QGCache::operator=(c); }
    int	  maxCost()   const	      { return QGCache::maxCost(); }
    int	  totalCost() const	      { return QGCache::totalCost(); }
    void  setMaxCost( int m )	      { QGCache::setMaxCost(m); }
    uint  count()     const	      { return QGCache::count(); }
    uint  size()      const	      { return QGCache::size(); }
    bool  isEmpty()   const	      { return QGCache::count() == 0; }
    bool  insert( const char *k, const type *d, int c=1, int p=0 )
				      { return QGCache::insert(k,(GCI)d,c,p);}
    bool  remove( const char *k )     { return QGCache::remove(k); }
    type *take( const char *k )	      { return (type *)QGCache::take(k); }
    void  clear()		      { QGCache::clear(); }
    type *find( const char *k, bool ref=TRUE ) const
				       { return (type *)QGCache::find(k,ref);}
    type *operator[]( const char *k ) const
				      { return (type *)QGCache::find(k);}
    void  statistics() const	      { QGCache::statistics(); }
private:
    void  deleteItem( GCI d )	      { if ( del_item ) delete (type *)d; }
};


#if defined(DEFAULT_TEMPLATECLASS)
#undef	QCacheIterator
#define QCacheIterator QCacheIteratorT
#endif

template<class type> class Q_EXPORT QCacheIteratorT : public QGCacheIterator
{
public:
    QCacheIteratorT( const QCacheT<type> &c ):QGCacheIterator((QGCache &)c) {}
    QCacheIteratorT( const QCacheIteratorT<type> &ci)
				: QGCacheIterator( (QGCacheIterator &)ci ) {}
    QCacheIteratorT<type> &operator=(const QCacheIteratorT<type>&ci)
	{ return ( QCacheIteratorT<type>&)QGCacheIterator::operator=( ci ); }
    uint  count()   const     { return QGCacheIterator::count(); }
    bool  isEmpty() const     { return QGCacheIterator::count() == 0; }
    bool  atFirst() const     { return QGCacheIterator::atFirst(); }
    bool  atLast()  const     { return QGCacheIterator::atLast(); }
    type *toFirst()	      { return (type *)QGCacheIterator::toFirst(); }
    type *toLast()	      { return (type *)QGCacheIterator::toLast(); }
    operator type *() const   { return (type *)QGCacheIterator::get(); }
    type *current()   const   { return (type *)QGCacheIterator::get(); }
    const char *currentKey() const
			      { return QGCacheIterator::getKey();}
    type *operator()()	      { return (type *)QGCacheIterator::operator()();}
    type *operator++()	      { return (type *)QGCacheIterator::operator++(); }
    type *operator+=(uint j)  { return (type *)QGCacheIterator::operator+=(j);}
    type *operator--()	      { return (type *)QGCacheIterator::operator--(); }
    type *operator-=(uint j)  { return (type *)QGCacheIterator::operator-=(j);}
};

#endif // USE_TEMPLATECLASS


#endif // QCACHE_H
