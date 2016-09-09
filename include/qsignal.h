/****************************************************************************
** $Id: qsignal.h,v 2.4.2.2 1998/08/21 19:13:23 hanord Exp $
**
** Definition of QSignal class
**
** Created : 941201
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

#ifndef QSIGNAL_H
#define QSIGNAL_H

#ifndef QT_H
#include "qobject.h"
#endif // QT_H


class Q_EXPORT QSignal : private QObject			// signal class
{
public:
    QSignal( QObject *parent=0, const char *name=0 );

    const char *name() const		{ return QObject::name(); }
    void    setName( const char *name ) { QObject::setName(name); }

    bool    connect( const QObject *receiver, const char *member );
    bool    disconnect( const QObject *receiver, const char *member=0 );

    bool    isBlocked()	 const		{ return QObject::signalsBlocked(); }
    void    block( bool b )		{ QObject::blockSignals( b ); }

    void    activate()			{ activate_signal("x()"); }

private:
    void    dummy();
    Q_OBJECT_FAKE

private:	// Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QSignal( const QSignal & );
    QSignal &operator=( const QSignal & );
#endif
};


#endif // QSIGNAL_H
