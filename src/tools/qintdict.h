/****************************************************************************
** $Id: qintdict.h,v 2.5.2.2 1998/08/27 08:14:01 hanord Exp $
**
** Definition of QIntDict template/macro class
**
** Created : 940624
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

#ifndef QINTDICT_H
#define QINTDICT_H

#ifndef QT_H
#include "qgdict.h"
#endif // QT_H


#if defined(USE_MACROCLASS)

#include "qgeneric.h"

#if !defined(name2)
#define name2(a,b)    name2_xx(a,b)
#define name2_xx(a,b) a##b
#endif

#if defined(DEFAULT_MACROCLASS)
#define QIntDictdeclare QIntDictMdeclare
#define QIntDict QIntDictM
#endif
#define QIntDictM(type) name2(QIntDictM_,type)

#define QIntDictMdeclare(type)						      \
class Q_EXPORTM QIntDictM(type) : public QGDict				      \
{									      \
public:									      \
    QIntDictM(type)(int size=17):QGDict(size,0,0,TRUE) {}		      \
    QIntDictM(type)( const QIntDictM(type) &d ) : QGDict(d) {}		      \
   ~QIntDictM(type)()		{ clear(); }				      \
    QIntDictM(type) &operator=(const QIntDictM(type) &d)		      \
			{ return (QIntDictM(type)&)QGDict::operator=(d); }    \
    uint  count()   const	{ return QGDict::count(); }		      \
    uint  size()    const	{ return QGDict::size(); }		      \
    bool  isEmpty() const	{ return QGDict::count() == 0; }	      \
    void  insert( long k, const type *d )				      \
				{ QGDict::look((const char*)k,(GCI)d,1); }    \
    void  replace( long k, const type *d )				      \
				{ QGDict::look((const char*)k,(GCI)d,2); }    \
    bool  remove( long k )	{ return QGDict::remove((const char*)k); }    \
    type *take( long k )	{ return (type*)QGDict::take((const char*)k);}\
    void  clear()		{ QGDict::clear(); }			      \
    void  resize( uint n )	{ QGDict::resize(n); }			      \
    type *find( long k )	const					      \
	{ return (type *)((QGDict*)this)->QGDict::look((const char*)k,0,0);}  \
    type *operator[]( long k ) const					      \
	{ return (type *)((QGDict*)this)->QGDict::look((const char*)k,0,0);}  \
    void  statistics() const	{ QGDict::statistics(); }		      \
private:								      \
    void  deleteItem( GCI d )	{ if ( del_item ) delete (type *)d; }	      \
}


#if defined(DEFAULT_MACROCLASS)
#define QIntDictIteratordeclare QIntDictIteratorMdeclare
#define QIntDictIterator QIntDictIteratorM
#endif
#define QIntDictIteratorM(type) name2(QIntDictIteratorM_,type)

#define QIntDictIteratorMdeclare(type)					      \
class Q_EXPORTM QIntDictIteratorM(type) : public QGDictIterator		      \
{									      \
public:									      \
    QIntDictIteratorM(type)(const QIntDictM(type) &d) :			      \
	QGDictIterator((QGDict &)d){}					      \
   ~QIntDictIteratorM(type)()	 {}					      \
    uint  count()   const     { return dict->count(); }			      \
    bool  isEmpty() const     { return dict->count() == 0; }		      \
    type *toFirst()	      { return (type *)QGDictIterator::toFirst(); }   \
    operator type *()  const  { return (type *)QGDictIterator::get(); }	      \
    type *current()    const  { return (type *)QGDictIterator::get(); }	      \
    long  currentKey() const  { return (long)QGDictIterator::getKey(); }      \
    type *operator()()	      { return (type *)QGDictIterator::operator()(); }\
    type *operator++()	      { return (type *)QGDictIterator::operator++(); }\
    type *operator+=(uint j)  { return (type *)QGDictIterator::operator+=(j);}\
}

#endif // USE_MACROCLASS


#if defined(USE_TEMPLATECLASS)

#if defined(DEFAULT_TEMPLATECLASS)
#undef	QIntDict
#define QIntDict QIntDictT
#endif

template<class type> class Q_EXPORT QIntDictT : public QGDict
{
public:
    QIntDictT(int size=17) : QGDict(size,0,0,TRUE) {}
    QIntDictT( const QIntDictT<type> &d ) : QGDict(d) {}
   ~QIntDictT()			{ clear(); }
    QIntDictT<type> &operator=(const QIntDictT<type> &d)
			{ return (QIntDictT<type>&)QGDict::operator=(d); }
    uint  count()   const	{ return QGDict::count(); }
    uint  size()    const	{ return QGDict::size(); }
    bool  isEmpty() const	{ return QGDict::count() == 0; }
    void  insert( long k, const type *d )
				{ QGDict::look((const char*)k,(GCI)d,1); }
    void  replace( long k, const type *d )
				{ QGDict::look((const char*)k,(GCI)d,2); }
    bool  remove( long k )	{ return QGDict::remove((const char*)k); }
    type *take( long k )	{ return (type*)QGDict::take((const char*)k); }
    void  clear()		{ QGDict::clear(); }
    void  resize( uint n )	{ QGDict::resize(n); }
    type *find( long k )	const
	{ return (type *)((QGDict*)this)->QGDict::look((const char*)k,0,0); }
    type *operator[]( long k ) const
	{ return (type *)((QGDict*)this)->QGDict::look((const char*)k,0,0); }
    void  statistics() const	{ QGDict::statistics(); }
private:
    void  deleteItem( GCI d )	{ if ( del_item ) delete (type *)d; }
};


#if defined(DEFAULT_TEMPLATECLASS)
#undef	QIntDictIterator
#define QIntDictIterator QIntDictIteratorT
#endif

template<class type> class Q_EXPORT QIntDictIteratorT : public QGDictIterator
{
public:
    QIntDictIteratorT(const QIntDictT<type> &d) :QGDictIterator((QGDict &)d) {}
   ~QIntDictIteratorT()	      {}
    uint  count()   const     { return dict->count(); }
    bool  isEmpty() const     { return dict->count() == 0; }
    type *toFirst()	      { return (type *)QGDictIterator::toFirst(); }
    operator type *()  const  { return (type *)QGDictIterator::get(); }
    type *current()    const  { return (type *)QGDictIterator::get(); }
    long  currentKey() const  { return (long)QGDictIterator::getKey(); }
    type *operator()()	      { return (type *)QGDictIterator::operator()(); }
    type *operator++()	      { return (type *)QGDictIterator::operator++(); }
    type *operator+=(uint j)  { return (type *)QGDictIterator::operator+=(j);}
};

#endif // USE_TEMPLATECLASS


#endif // QINTDICT_H
