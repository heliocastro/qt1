/****************************************************************************
** $Id: qconnection.h,v 2.6.2.3 1998/08/21 19:13:22 hanord Exp $
**
** Definition of QConnection class
**
** Created : 930417
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

#ifndef QCONNECTION_H
#define QCONNECTION_H

#ifndef QT_H
#include "qobject.h"
#endif // QT_H


typedef void (QObject::*QMember)();		// pointer to member function


class Q_EXPORT QConnection
{
public:
    QConnection( const QObject *, QMember, const char *memberName );
   ~QConnection() {}

    bool     isConnected() const { return obj != 0; }

    QObject *object() const  { return obj; }	// get object/member pointer
    QMember *member() const  { return (QMember*)&mbr; }
    const char *memberName() const { return mbr_name; }
    int	     numArgs() const { return nargs; }

private:
    QObject *obj;				// object connected to
    QMember  mbr;				// member connected to
    const char *mbr_name;
    int	     nargs;

private:	// Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QConnection( const QConnection & );
    QConnection &operator=( const QConnection & );
#endif
};


#endif // QCONNECTION_H
