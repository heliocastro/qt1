/****************************************************************************
** $Id: qobjectdict.h,v 2.2.2.2 1998/08/25 09:20:52 hanord Exp $
**
** Definition of QObjectDictionary
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

#ifndef QOBJECTDICT_H
#define QOBJECTDICT_H

#ifndef QT_H
#include "qmetaobject.h"
#include "qdict.h"
#endif // QT_H


// QMetaObject collections

#if defined(Q_TEMPLATEDLL)

template class Q_EXPORT QDict<QMetaObject>;

class Q_EXPORT QObjectDictionary : public QDict<QMetaObject>
{
public:
    QObjectDictionary(int size=17,bool cs=TRUE,bool ck=TRUE) :
	QDict<QMetaObject>(size,cs,ck) {}
    QObjectDictionary( const QObjectDictionary &dict ) : QDict<QMetaObject>(dict) {}
   ~QObjectDictionary() { clear(); }
    QObjectDictionary &operator=(const QObjectDictionary &dict)
	{ return (QObjectDictionary&)QDict<QMetaObject>::operator=(dict); }
};

#else

typedef Q_DECLARE(QDictM,QMetaObject) QObjectDictionary;

#endif



// Global object dictionary defined in qmetaobject.cpp

extern Q_EXPORT QObjectDictionary *objectDict;


#endif // QOBJECTDICT_H
