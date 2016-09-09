/****************************************************************************
** $Id: qobjectlist.h,v 2.2.2.2 1998/08/25 09:20:52 hanord Exp $
**
** Definition of QObjectList
**
** Created : 940807
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

#ifndef QOBJECTLIST_H
#define QOBJECTLIST_H

#ifndef QT_H
#include "qobject.h"
#include "qlist.h"
#endif // QT_H


// QObject collections

#if defined(Q_TEMPLATEDLL)

template class Q_EXPORT QList<QObject>;
template class Q_EXPORT QListIterator<QObject>;

class Q_EXPORT QObjectList : public QList<QObject>
{
public:
    QObjectList() : QList<QObject>() {}
    QObjectList( const QObjectList &list ) : QList<QObject>(list) {}
   ~QObjectList() { clear(); }
    QObjectList &operator=(const QObjectList &list)
	{ return (QObjectList&)QList<QObject>::operator=(list); }
};

class Q_EXPORT QObjectListIt : public QListIterator<QObject>
{
public:
    QObjectListIt( const QObjectList &list ) : QListIterator<QObject>(list) {}
    QObjectListIt &operator=(const QObjectListIt &list)
	{ return (QObjectListIt&)QListIterator<QObject>::operator=(list); }
};

#else /* Q_TEMPLATEDLL */

typedef Q_DECLARE(QListM,QObject)	    QObjectList;
typedef Q_DECLARE(QListIteratorM,QObject)   QObjectListIt;

#endif


#endif // QOBJECTLIST_H
