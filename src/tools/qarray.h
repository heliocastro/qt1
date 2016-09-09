/****************************************************************************
** $Id: qarray.h,v 2.3.2.3 1998/08/27 08:14:00 hanord Exp $
**
** Definition of QArray template/macro class
**
** Created : 930906
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

#ifndef QARRAY_H
#define QARRAY_H

#ifndef QT_H
#include "qgarray.h"
#endif // QT_H


#if defined(USE_MACROCLASS)

#include "qgeneric.h"

#if !defined(name2)
#define name2(a,b)    name2_xx(a,b)
#define name2_xx(a,b) a##b
#endif

#if defined(DEFAULT_MACROCLASS)
#define QArraydeclare QArrayMdeclare
#define QArray QArrayM
#endif
#define QArrayM(type) name2(QArrayM_,type)

#define QArrayMdeclare(type)						      \
class Q_EXPORTM QArrayM(type) : public QGArray				      \
{									      \
protected:								      \
    QArrayM(type)( int, int ) : QGArray( 0, 0 ) {}			      \
public:									      \
    QArrayM(type)()		{}					      \
    QArrayM(type)( int size ) : QGArray(size*sizeof(type)) {}		      \
    QArrayM(type)( const QArrayM(type) &a ) : QGArray(a) {}		      \
   ~QArrayM(type)()		{}					      \
    QArrayM(type)& operator=(const QArrayM(type) &a)			      \
				{ return (QArrayM(type)&)QGArray::assign(a); }\
    type *data()    const	{ return (type *)QGArray::data(); }	      \
    uint  nrefs()   const	{ return QGArray::nrefs(); }		      \
    uint  size()    const	{ return QGArray::size()/sizeof(type); }      \
    bool  isEmpty() const	{ return QGArray::size() == 0; }	      \
    bool  isNull()  const	{ return QGArray::data() == 0; }	      \
    bool  resize( uint size )	{ return QGArray::resize(size*sizeof(type)); }\
    bool  truncate( uint pos )	{ return QGArray::resize(pos*sizeof(type)); } \
    bool  fill( const type &d, int size=-1 )				      \
	{ return QGArray::fill((char*)&d,size,sizeof(type) ); }		      \
    void  detach()		{ QGArray::detach(); }			      \
    QArrayM(type)   copy() const					      \
	{ QArrayM(type) tmp; return tmp.duplicate(*this); }		      \
    QArrayM(type)& assign( const QArrayM(type)& a )			      \
	{ return (QArrayM(type)&)QGArray::assign(a); }			      \
    QArrayM(type)& assign( const type *a, uint n )			      \
	{ return (QArrayM(type)&)QGArray::assign((char*)a,n*sizeof(type));}   \
    QArrayM(type)& duplicate( const QArrayM(type)& a )			      \
	{ return (QArrayM(type)&)QGArray::duplicate(a); }		      \
    QArrayM(type)& duplicate( const type *a, uint n )			      \
	{ return (QArrayM(type)&)QGArray::duplicate((char*)a,n*sizeof(type));}\
    QArrayM(type)& setRawData( const type *a, uint n )			      \
	{ return (QArrayM(type)&)QGArray::setRawData((char*)a,		      \
						     n*sizeof(type)); }	      \
    void resetRawData( const type *a, uint n )				      \
	{ QGArray::resetRawData((char*)a,n*sizeof(type)); }		      \
    int	 find( const type &d, uint i=0 ) const				      \
	{ return QGArray::find((char*)&d,i,sizeof(type)); }		      \
    int	 contains( const type &d ) const				      \
	{ return QGArray::contains((char*)&d,sizeof(type)); }		      \
    type& operator[]( int i ) const					      \
	{ return (type &)(*(type *)QGArray::at(i*sizeof(type))); }	      \
    type& at( uint i ) const						      \
	{ return (type &)(*(type *)QGArray::at(i*sizeof(type))); }	      \
	 operator const type*() const { return (const type *)QGArray::data();}\
    bool operator==( const QArrayM(type) &a ) const { return isEqual(a); }    \
    bool operator!=( const QArrayM(type) &a ) const { return !isEqual(a); }   \
}

#endif // USE_MACROCLASS


#if defined(USE_TEMPLATECLASS)

#if defined(DEFAULT_TEMPLATECLASS)
#undef	QArray
#define QArray QArrayT
#endif

template<class type> class Q_EXPORT QArrayT : public QGArray
{
protected:
    QArrayT( int, int ) : QGArray( 0, 0 ) {}
public:
    QArrayT()			{}
    QArrayT( int size ) : QGArray(size*sizeof(type)) {}
    QArrayT( const QArrayT<type> &a ) : QGArray(a) {}
   ~QArrayT()			{}
    QArrayT<type> &operator=(const QArrayT<type> &a)
				{ return (QArrayT<type>&)QGArray::assign(a); }
    type *data()    const	{ return (type *)QGArray::data(); }
    uint  nrefs()   const	{ return QGArray::nrefs(); }
    uint  size()    const	{ return QGArray::size()/sizeof(type); }
    bool  isEmpty() const	{ return QGArray::size() == 0; }
    bool  isNull()  const	{ return QGArray::data() == 0; }
    bool  resize( uint size )	{ return QGArray::resize(size*sizeof(type)); }
    bool  truncate( uint pos )	{ return QGArray::resize(pos*sizeof(type)); }
    bool  fill( const type &d, int size=-1 )
	{ return QGArray::fill((char*)&d,size,sizeof(type) ); }
    void  detach()		{ QGArray::detach(); }
    QArrayT<type>   copy() const
	{ QArrayT<type> tmp; return tmp.duplicate(*this); }
    QArrayT<type>& assign( const QArrayT<type>& a )
	{ return (QArrayT<type>&)QGArray::assign(a); }
    QArrayT<type>& assign( const type *a, uint n )
	{ return (QArrayT<type>&)QGArray::assign((char*)a,n*sizeof(type)); }
    QArrayT<type>& duplicate( const QArrayT<type>& a )
	{ return (QArrayT<type>&)QGArray::duplicate(a); }
    QArrayT<type>& duplicate( const type *a, uint n )
	{ return (QArrayT<type>&)QGArray::duplicate((char*)a,n*sizeof(type)); }
    QArrayT<type>& setRawData( const type *a, uint n )
	{ return (QArrayT<type>&)QGArray::setRawData((char*)a,
						     n*sizeof(type)); }
    void resetRawData( const type *a, uint n )
	{ QGArray::resetRawData((char*)a,n*sizeof(type)); }
    int	 find( const type &d, uint i=0 ) const
	{ return QGArray::find((char*)&d,i,sizeof(type)); }
    int	 contains( const type &d ) const
	{ return QGArray::contains((char*)&d,sizeof(type)); }
    type& operator[]( int i ) const
	{ return (type &)(*(type *)QGArray::at(i*sizeof(type))); }
    type& at( uint i ) const
	{ return (type &)(*(type *)QGArray::at(i*sizeof(type))); }
	 operator const type*() const { return (const type *)QGArray::data(); }
    bool operator==( const QArrayT<type> &a ) const { return isEqual(a); }
    bool operator!=( const QArrayT<type> &a ) const { return !isEqual(a); }
};


#endif // USE_TEMPLATECLASS


#endif // QARRAY_H
