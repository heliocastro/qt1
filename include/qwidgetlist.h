/****************************************************************************
** $Id: qwidgetlist.h,v 2.2.2.2 1998/08/25 09:20:53 hanord Exp $
**
** Definition of QWidgetList
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

#ifndef QWIDGETLIST_H
#define QWIDGETLIST_H

#ifndef QT_H
#include "qwidget.h"
#include "qlist.h"
#endif // QT_H


#if defined(Q_TEMPLATEDLL)

template class Q_EXPORT QList<QWidget>;
template class Q_EXPORT QListIterator<QWidget>;

class Q_EXPORT QWidgetList : public QList<QWidget>
{
public:
    QWidgetList() : QList<QWidget>() {}
    QWidgetList( const QWidgetList &list ) : QList<QWidget>(list) {}
   ~QWidgetList() { clear(); }
    QWidgetList &operator=(const QWidgetList &list)
	{ return (QWidgetList&)QList<QWidget>::operator=(list); }
};

class Q_EXPORT QWidgetListIt : public QListIterator<QWidget>
{
public:
    QWidgetListIt( const QWidgetList &list ) : QListIterator<QWidget>(list) {}
    QWidgetListIt &operator=(const QWidgetListIt &list)
	{ return (QWidgetListIt&)QListIterator<QWidget>::operator=(list); }
};

#else /* Q_TEMPLATEDLL */

typedef Q_DECLARE(QListM,QWidget)		QWidgetList;
typedef Q_DECLARE(QListIteratorM,QWidget)	QWidgetListIt;

#endif


#endif // QWIDGETLIST_H
