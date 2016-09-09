/****************************************************************************
** $Id: qvector.h,v 2.3.2.2 1998/08/27 08:14:01 hanord Exp $
**
** Definition of QVector template/macro class
**
** Created : 930907
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

#ifndef QVECTOR_H
#define QVECTOR_H

#ifndef QT_H
#include "qgvector.h"
#endif // QT_H


#if defined(USE_MACROCLASS)

#include "qgeneric.h"

#if !defined(name2)
#define name2(a,b)    name2_xx(a,b)
#define name2_xx(a,b) a##b
#endif

#if defined(DEFAULT_MACROCLASS)
#define QVectordeclare QVectorMdeclare
#define QVector QVectorM
#endif
#define QVectorM(type) name2(QVectorM_,type)

#define QVectorMdeclare(type)						      \
class Q_EXPORTM QVectorM(type) : public QGVector			      \
{									      \
public:									      \
    QVectorM(type)()			{}				      \
    QVectorM(type)( uint size ) : QGVector(size) {}			      \
    QVectorM(type)( const QVectorM(type) &v ) : QGVector(v) {}		      \
   ~QVectorM(type)()			{ clear(); }			      \
    QVectorM(type)& operator=(const QVectorM(type) &v)			      \
			{ return (QVectorM(type)&)QGVector::operator=(v); }   \
    type **data()   const		{ return (type **)QGVector::data(); } \
    uint  size()    const		{ return QGVector::size(); }	      \
    uint  count()   const		{ return QGVector::count(); }	      \
    bool  isEmpty() const		{ return QGVector::count() == 0; }    \
    bool  isNull()  const		{ return QGVector::size() == 0; }     \
    bool  resize( uint size )		{ return QGVector::resize(size); }    \
    bool  insert( uint i, const type *d){ return QGVector::insert(i,(GCI)d); }\
    bool  remove( uint i )		{ return QGVector::remove(i); }	      \
    type *take( uint i )		{ return (type *)QGVector::take(i); } \
    void  clear()			{ QGVector::clear(); }		      \
    bool  fill( const type *d, int size=-1 )				      \
					{ return QGVector::fill((GCI)d,size);}\
    void  sort()			{ QGVector::sort(); }		      \
    int	  bsearch( const type *d ) const{ return QGVector::bsearch((GCI)d); } \
    int	  findRef( const type *d, uint i=0 ) const			      \
					{ return QGVector::findRef((GCI)d,i);}\
    int	  find( const type *d, uint i= 0 ) const			      \
					{ return QGVector::find((GCI)d,i); }  \
    uint  containsRef( const type *d ) const				      \
				{ return QGVector::containsRef((GCI)d); }     \
    uint  contains( const type *d ) const				      \
					{ return QGVector::contains((GCI)d); }\
    type *operator[]( int i ) const	{ return (type *)QGVector::at(i); }   \
    type *at( uint i ) const		{ return (type *)QGVector::at(i); }   \
    void  toList( QGList *list ) const	{ QGVector::toList(list); }	      \
private:								      \
    void  deleteItem( GCI d ) { if ( del_item ) delete (type *)d; }	      \
}

#endif // USE_MACROCLASS


#if defined(USE_TEMPLATECLASS)

#if defined(DEFAULT_TEMPLATECLASS)
#undef	QVector
#define QVector QVectorT
#endif

template<class type> class Q_EXPORT QVectorT : public QGVector
{
public:
    QVectorT()				{}
    QVectorT( uint size ) : QGVector(size) {}
    QVectorT( const QVectorT<type> &v ) : QGVector(v) {}
   ~QVectorT()				{ clear(); }
    QVectorT<type> &operator=(const QVectorT<type> &v)
			{ return (QVectorT<type>&)QGVector::operator=(v); }
    type **data()   const		{ return (type **)QGVector::data(); }
    uint  size()    const		{ return QGVector::size(); }
    uint  count()   const		{ return QGVector::count(); }
    bool  isEmpty() const		{ return QGVector::count() == 0; }
    bool  isNull()  const		{ return QGVector::size() == 0; }     \
    bool  resize( uint size )		{ return QGVector::resize(size); }
    bool  insert( uint i, const type *d){ return QGVector::insert(i,(GCI)d); }
    bool  remove( uint i )		{ return QGVector::remove(i); }
    type *take( uint i )		{ return (type *)QGVector::take(i); }
    void  clear()			{ QGVector::clear(); }
    bool  fill( const type *d, int size=-1 )
					{ return QGVector::fill((GCI)d,size);}
    void  sort()			{ QGVector::sort(); }
    int	  bsearch( const type *d ) const{ return QGVector::bsearch((GCI)d); }
    int	  findRef( const type *d, uint i=0 ) const
					{ return QGVector::findRef((GCI)d,i);}
    int	  find( const type *d, uint i= 0 ) const
					{ return QGVector::find((GCI)d,i); }
    uint  containsRef( const type *d ) const
				{ return QGVector::containsRef((GCI)d); }
    uint  contains( const type *d ) const
					{ return QGVector::contains((GCI)d); }
    type *operator[]( int i ) const	{ return (type *)QGVector::at(i); }
    type *at( uint i ) const		{ return (type *)QGVector::at(i); }
    void  toList( QGList *list ) const	{ QGVector::toList(list); }
private:
    void  deleteItem( GCI d ) { if ( del_item ) delete (type *)d; }
};

#endif // USE_TEMPLATECLASS


#endif // QVECTOR_H
