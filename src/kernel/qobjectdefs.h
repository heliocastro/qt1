/****************************************************************************
** $Id: qobjectdefs.h,v 2.5.2.2 1998/08/25 09:20:52 hanord Exp $
**
** Macros and definitions related to QObject
**
** Created : 930419
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

#ifndef QOBJECTDEFS_H
#define QOBJECTDEFS_H

#ifndef QT_H
#include "qglobal.h"
#endif // QT_H


// The following macros are our "extensions" to C++
// They are used, strictly speaking, only by the moc.

#define slots					// slots:   in class
#define signals protected			// signals: in class
#define emit					// emit signal

/* tmake ignore Q_OBJECT */
#define Q_OBJECT							      \
public:									      \
    QMetaObject *metaObject() const { return metaObj; }			      \
    const char  *className()  const;					      \
protected:								      \
    void	 initMetaObject();					      \
private:								      \
    static QMetaObject *metaObj;

/* tmake ignore Q_OBJECT */
#define Q_OBJECT_FAKE Q_OBJECT
						// macro for naming members
#if defined(_OLD_CPP_)
#define METHOD(a)	"0""a"
#define SLOT(a)		"1""a"
#define SIGNAL(a)	"2""a"
#else
#define METHOD(a)	"0"#a
#define SLOT(a)		"1"#a
#define SIGNAL(a)	"2"#a
#endif

#define METHOD_CODE	0			// member type codes
#define SLOT_CODE	1
#define SIGNAL_CODE	2


// Forward declarations so you don't have to include files you don't need

class QObject;
class QMetaObject;
class QSignal;
class QConnection;
class QEvent;


#if defined(Q_TEMPLATEDLL)

class QConnectionList;
class QConnectionListIt;
class QSignalDict;
class QSignalDictIt;
class QObjectList;
class QObjectListIt;
class QMemberDict;

#else

class QListM_QConnection;			// connection list
#define QConnectionList	  QListM_QConnection
#define QConnectionListIt QListIteratorM_QConnection

class QDictM_QListM_QConnection;		// signal dictionary
#define QSignalDict   QDictM_QListM_QConnection
#define QSignalDictIt QDictIteratorM_QListM_QConnection

class QListM_QObject;				// object list
#define QObjectList QListM_QObject

class QDictM_QMetaData;				// meta object dictionaries
#define QMemberDict QDictM_QMetaData

#endif


#endif // QOBJECTDEFS_H
