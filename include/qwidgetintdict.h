/****************************************************************************
** $Id: qwidgetintdict.h,v 2.2.2.3 1998/08/25 09:20:53 hanord Exp $
**
** Definition of QWidgetIntDict
**
** Created : 950116
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

#ifndef QWIDINTDICT_H
#define QWIDINTDICT_H

#ifndef QT_H
#include "qwidget.h"
#include "qintdict.h"
#endif // QT_H


#if defined(Q_TEMPLATEDLL)

template class Q_EXPORT QIntDict<QWidget>;
template class Q_EXPORT QIntDictIterator<QWidget>;

class Q_EXPORT QWidgetIntDict : public QIntDict<QWidget>
{
public:
    QWidgetIntDict(int size=17) : QIntDict<QWidget>(size) {}
    QWidgetIntDict( const QWidgetIntDict &dict ) : QIntDict<QWidget>(dict) {}
   ~QWidgetIntDict() { clear(); }
    QWidgetIntDict &operator=(const QWidgetIntDict &dict)
	{ return (QWidgetIntDict&)QIntDict<QWidget>::operator=(dict); }
};

class Q_EXPORT QWidgetIntDictIt : public QIntDictIterator<QWidget>
{
public:
    QWidgetIntDictIt( const QWidgetIntDict &dict ) : QIntDictIterator<QWidget>(dict) {}
    QWidgetIntDictIt &operator=(const QWidgetIntDictIt &dict)
	{ return (QWidgetIntDictIt&)QIntDictIterator<QWidget>::operator=(dict); }
};

#else /* Q_TEMPLATEDLL */

typedef Q_DECLARE(QIntDictM,QWidget)		QWidgetIntDict;
typedef Q_DECLARE(QIntDictIteratorM,QWidget)	QWidgetIntDictIt;

#endif


#endif // QWIDINTDICT_H
