/****************************************************************************
** $Id: qdict.h,v 2.5.2.3 1998/08/27 08:14:00 hanord Exp $
**
** Definition of QDict template/macro class
**
** Created : 920821
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

#ifndef QDICT_H
#define QDICT_H

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
#define QDictdeclare QDictMdeclare
#define QDict QDictM
#endif
#define QDictM(type) name2(QDictM_,type)

#define QDictMdeclare(type)						      \
class Q_EXPORTM QDictM(type) : public QGDict				      \
{									      \
public:									      \
    QDictM(type)(int size=17,bool cs=TRUE,bool ck=TRUE):QGDict(size,cs,ck,0){}\
    QDictM(type)( const QDictM(type) &d ) : QGDict(d) {}		      \
   ~QDictM(type)()			{ clear(); }			      \
    QDictM(type) &operator=(const QDictM(type) &d)			      \
			{ return (QDictM(type)&)QGDict::operator=(d); }	      \
    uint  count()   const		{ return QGDict::count(); }	      \
    uint  size()    const		{ return QGDict::size(); }	      \
    bool  isEmpty() const		{ return QGDict::count() == 0; }      \
    void  insert( const char *k, const type *d )			      \
					{ QGDict::look(k,(GCI)d,1); }	      \
    void  replace( const char *k, const type *d )			      \
					{ QGDict::look(k,(GCI)d,2); }	      \
    bool  remove( const char *k )	{ return QGDict::remove(k); }	      \
    type *take( const char *k )		{ return (type *)QGDict::take(k); }   \
    void  clear()			{ QGDict::clear(); }		      \
    void  resize( uint n )		{ QGDict::resize(n); }		      \
    type *find( const char *k ) const					      \
		    { return (type *)((QGDict*)this)->QGDict::look(k,0,0);}   \
    type *operator[]( const char *k ) const				      \
		    { return (type *)((QGDict*)this)->QGDict::look(k,0,0);}   \
    void  statistics() const		{ QGDict::statistics(); }	      \
private:								      \
    void  deleteItem( GCI d )	{ if ( del_item ) delete (type *)d; }	      \
}


#if defined(DEFAULT_MACROCLASS)
#define QDictIteratordeclare QDictIteratorMdeclare
#define QDictIterator QDictIteratorM
#endif
#define QDictIteratorM(type) name2(QDictIteratorM_,type)

#define QDictIteratorMdeclare(type)					      \
class Q_EXPORTM QDictIteratorM(type) : public QGDictIterator		      \
{									      \
public:									      \
    QDictIteratorM(type)(const QDictM(type) &d) :QGDictIterator((QGDict &)d){}\
   ~QDictIteratorM(type)()    {}					      \
    uint  count()   const     { return dict->count(); }			      \
    bool  isEmpty() const     { return dict->count() == 0; }		      \
    type *toFirst()	      { return (type *)QGDictIterator::toFirst(); }   \
    operator type *() const   { return (type *)QGDictIterator::get(); }	      \
    type *current()   const   { return (type *)QGDictIterator::get(); }	      \
    const char *currentKey() const					      \
			      { return QGDictIterator::getKey(); }	      \
    type *operator()()	      { return (type *)QGDictIterator::operator()(); }\
    type *operator++()	      { return (type *)QGDictIterator::operator++(); }\
    type *operator+=(uint j)  { return (type *)QGDictIterator::operator+=(j);}\
}

#endif // USE_MACROCLASS


#if defined(USE_TEMPLATECLASS)

#if defined(DEFAULT_TEMPLATECLASS)
#undef	QDict
#define QDict QDictT
#endif

template<class type> class Q_EXPORT QDictT : public QGDict
{
public:
    QDictT(int size=17,bool cs=TRUE,bool ck=TRUE) : QGDict(size,cs,ck,0) {}
    QDictT( const QDictT<type> &d ) : QGDict(d) {}
   ~QDictT()				{ clear(); }
    QDictT<type> &operator=(const QDictT<type> &d)
			{ return (QDictT<type>&)QGDict::operator=(d); }
    uint  count()   const		{ return QGDict::count(); }
    uint  size()    const		{ return QGDict::size(); }
    bool  isEmpty() const		{ return QGDict::count() == 0; }
    void  insert( const char *k, const type *d )
					{ QGDict::look(k,(GCI)d,1); }
    void  replace( const char *k, const type *d )
					{ QGDict::look(k,(GCI)d,2); }
    bool  remove( const char *k )	{ return QGDict::remove(k); }
    type *take( const char *k )		{ return (type *)QGDict::take(k); }
    void  clear()			{ QGDict::clear(); }
    void  resize( uint n )		{ QGDict::resize(n); }
    type *find( const char *k ) const
		    { return (type *)((QGDict*)this)->QGDict::look(k,0,0); }
    type *operator[]( const char *k ) const
		    { return (type *)((QGDict*)this)->QGDict::look(k,0,0); }
    void  statistics() const		{ QGDict::statistics(); }
private:
    void  deleteItem( GCI d )	{ if ( del_item ) delete (type *)d; }
};


#if defined(DEFAULT_TEMPLATECLASS)
#undef	QDictIterator
#define QDictIterator QDictIteratorT
#endif

template<class type> class Q_EXPORT QDictIteratorT : public QGDictIterator
{
public:
    QDictIteratorT(const QDictT<type> &d) :QGDictIterator((QGDict &)d) {}
   ~QDictIteratorT()	      {}
    uint  count()   const     { return dict->count(); }
    bool  isEmpty() const     { return dict->count() == 0; }
    type *toFirst()	      { return (type *)QGDictIterator::toFirst(); }
    operator type *() const   { return (type *)QGDictIterator::get(); }
    type *current()   const   { return (type *)QGDictIterator::get(); }
    const char *currentKey() const
			      { return QGDictIterator::getKey(); }
    type *operator()()	      { return (type *)QGDictIterator::operator()(); }
    type *operator++()	      { return (type *)QGDictIterator::operator++(); }
    type *operator+=(uint j)  { return (type *)QGDictIterator::operator+=(j);}
};

#endif // USE_TEMPLATECLASS


#endif // QDICT_H
