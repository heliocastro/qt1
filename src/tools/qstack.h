/****************************************************************************
** $Id: qstack.h,v 2.3.2.2 1998/08/27 08:14:01 hanord Exp $
**
** Definition of QStack template/macro class
**
** Created : 920917
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

#ifndef QSTACK_H
#define QSTACK_H

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
#define QStackdeclare QStackMdeclare
#define QStack QStackM
#endif
#define QStackM(type) name2(QStackM_,type)

#define QStackMdeclare(type)						      \
class Q_EXPORTM QStackM(type) : private QGList				      \
{									      \
public:									      \
    QStackM(type)()			{}				      \
    QStackM(type)( const QStackM(type) &s ) : QGList(s) {}		      \
   ~QStackM(type)()			{ clear(); }			      \
    QStackM(type) &operator=(const QStackM(type) &s)			      \
			{ return (QStackM(type)&)QGList::operator=(s); }      \
    bool  autoDelete() const		{ return QCollection::autoDelete(); } \
    void  setAutoDelete( bool del )	{ QCollection::setAutoDelete(del); }  \
    uint  count()   const		{ return QGList::count(); }	      \
    bool  isEmpty() const		{ return QGList::count() == 0; }      \
    void  push( const type *d )		{ QGList::insertAt(0,GCI(d)); }	      \
    type *pop()				{ return (type *)QGList::takeFirst();}\
    bool  remove()			{ return QGList::removeFirst(); }     \
    void  clear()			{ QGList::clear(); }		      \
    type *top()	    const		{ return (type *)QGList::cfirst(); }  \
	  operator type *() const	{ return (type *)QGList::cfirst(); }  \
    type *current() const		{ return (type *)QGList::cfirst(); }  \
private:								      \
    void  deleteItem( GCI d ) { if ( del_item ) delete (type *)d; }	      \
}

#endif // USE_MACROCLASS


#if defined(USE_TEMPLATECLASS)

#if defined(DEFAULT_TEMPLATECLASS)
#undef	QStack
#define QStack QStackT
#endif

template<class type> class Q_EXPORT QStackT : private QGList
{
public:
    QStackT()				{}
    QStackT( const QStackT<type> &s ) : QGList(s) {}
   ~QStackT()				{ clear(); }
    QStackT<type> &operator=(const QStackT<type> &s)
			{ return (QStackT<type>&)QGList::operator=(s); }
    bool  autoDelete() const		{ return QCollection::autoDelete(); }
    void  setAutoDelete( bool del )	{ QCollection::setAutoDelete(del); }
    uint  count()   const		{ return QGList::count(); }
    bool  isEmpty() const		{ return QGList::count() == 0; }
    void  push( const type *d )		{ QGList::insertAt(0,GCI(d)); }
    type *pop()				{ return (type *)QGList::takeFirst(); }
    bool  remove()			{ return QGList::removeFirst(); }
    void  clear()			{ QGList::clear(); }
    type *top()	    const		{ return (type *)QGList::cfirst(); }
	  operator type *() const	{ return (type *)QGList::cfirst(); }
    type *current() const		{ return (type *)QGList::cfirst(); }
private:
    void  deleteItem( GCI d ) { if ( del_item ) delete (type *)d; }
};

#endif // USE_TEMPLATECLASS


#endif // QSTACK_H
