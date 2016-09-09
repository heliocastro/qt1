/****************************************************************************
** $Id: qsocketnotifier.h,v 2.5.2.2 1998/08/21 19:13:23 hanord Exp $
**
** Definition of QSocketNotifier class
**
** Created : 951114
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

#ifndef QSOCKETNOTIFIER_H
#define QSOCKETNOTIFIER_H

#ifndef QT_H
#include "qobject.h"
#endif // QT_H


class Q_EXPORT QSocketNotifier : public QObject
{
    Q_OBJECT
public:
    enum Type { Read, Write, Exception };

    QSocketNotifier( int socket, Type, QObject *parent=0, const char *name=0 );
   ~QSocketNotifier();

    int		socket()	const;
    Type	type()		const;

    bool	isEnabled()	const;
    void	setEnabled( bool );

signals:
    void	activated( int socket );

protected:
    bool	event( QEvent * );

private:
    int		sockfd;
    Type	sntype;
    bool	snenabled;

private:	// Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QSocketNotifier( const QSocketNotifier & );
    QSocketNotifier &operator=( const QSocketNotifier & );
#endif
};


inline int QSocketNotifier::socket() const
{ return sockfd; }

inline QSocketNotifier::Type QSocketNotifier::type() const
{ return sntype; }

inline bool QSocketNotifier::isEnabled() const
{ return snenabled; }


#endif // QSOCKETNOTIFIER_H
