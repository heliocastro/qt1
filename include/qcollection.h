/****************************************************************************
** $Id: qcollection.h,v 2.4.2.3 1998/08/25 09:20:54 hanord Exp $
**
** Definition of base class for all collection classes
**
** Created : 920629
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

#ifndef QCOLLECTION_H
#define QCOLLECTION_H

#ifndef QT_H
#include "qglobal.h"
#endif // QT_H


typedef void *GCI;				// generic collection item
typedef int (*GCF)(GCI,void*);			// generic collection function

class QGVector;
class QGList;
class QGDict;


class Q_EXPORT QCollection			// inherited by all collections
{
public:
    bool autoDelete()	const	       { return del_item; }
    void setAutoDelete( bool enable )  { del_item = enable; }

    virtual uint  count() const = 0;
    virtual void  clear() = 0;			// delete all objects

protected:
    QCollection() { del_item = FALSE; }		// no deletion of objects
    QCollection(const QCollection &) { del_item = FALSE; }
    virtual ~QCollection() {}

    bool del_item;				// default FALSE

    virtual GCI	     newItem( GCI );		// create object
    virtual void     deleteItem( GCI );		// delete object
};


#endif // QCOLLECTION_H
