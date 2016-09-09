/****************************************************************************
** $Id: qsignalslotimp.h,v 1.1.2.7 1998/09/01 11:19:02 hanord Exp $
**
** Definition of signal/slot collections etc.
**
** Created : 980821
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

#ifndef QSIGNALSLOTIMP_H
#define QSIGNALSLOTIMP_H

#ifndef QT_H
#include "qconnection.h"
#include "qlist.h"
#include "qdict.h"
#endif // QT_H


#if defined(Q_TEMPLATEDLL)

template class Q_EXPORT QList<QConnection>;
template class Q_EXPORT QListIterator<QConnection>;

class Q_EXPORT QConnectionList : public QList<QConnection>
{
public:
    QConnectionList() : QList<QConnection>() {}
    QConnectionList( const QConnectionList &list ) : QList<QConnection>(list) {}
   ~QConnectionList() { clear(); }
    QConnectionList &operator=(const QConnectionList &list)
	{ return (QConnectionList&)QList<QConnection>::operator=(list); }
};

class Q_EXPORT QConnectionListIt : public QListIterator<QConnection>
{
public:
    QConnectionListIt( const QConnectionList &list ) : QListIterator<QConnection>(list) {}
    QConnectionListIt &operator=(const QConnectionListIt &list)
	{ return (QConnectionListIt&)QListIterator<QConnection>::operator=(list); }
};


template class Q_EXPORT QDict<QConnectionList>;
template class Q_EXPORT QDictIterator<QConnectionList>;

class Q_EXPORT QSignalDict : public QDict<QConnectionList>
{
public:
    QSignalDict(int size=17,bool cs=TRUE,bool ck=TRUE) :
	QDict<QConnectionList>(size,cs,ck) {}
    QSignalDict( const QSignalDict &dict ) : QDict<QConnectionList>(dict) {}
   ~QSignalDict() { clear(); }
    QSignalDict &operator=(const QSignalDict &dict)
	{ return (QSignalDict&)QDict<QConnectionList>::operator=(dict); }
};

class Q_EXPORT QSignalDictIt : public QDictIterator<QConnectionList>
{
public:
    QSignalDictIt( const QSignalDict &dict ) : QDictIterator<QConnectionList>(dict) {}
    QSignalDictIt &operator=(const QSignalDictIt &dict)
	{ return (QSignalDictIt&)QDictIterator<QConnectionList>::operator=(dict); }
};


#else
#include <qlist.h>
Q_DECLARE(QListM,QConnection);
Q_DECLARE(QListIteratorM,QConnection);
#endif

#endif // QSIGNALSLOTIMP_H
