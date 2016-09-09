/****************************************************************************
** $Id: qtimer.h,v 2.4.2.2 1998/08/21 19:13:24 hanord Exp $
**
** Definition of QTimer class
**
** Created : 931111
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

#ifndef QTIMER_H
#define QTIMER_H

#ifndef QT_H
#include "qobject.h"
#endif // QT_H


class Q_EXPORT QTimer : public QObject
{
    Q_OBJECT
public:
    QTimer( QObject *parent=0, const char *name=0 );
   ~QTimer();

    bool	isActive() const;

    int		start( int msec, bool sshot = FALSE );
    void	changeInterval( int msec );
    void	stop();

    static void singleShot( int msec, QObject *receiver, const char *member );

signals:
    void	timeout();

protected:
    bool	event( QEvent * );

private:
    int		id;
    bool	single;

private:	// Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QTimer( const QTimer & );
    QTimer &operator=( const QTimer & );
#endif
};


inline bool QTimer::isActive() const
{
    return id >= 0;
}


#endif // QTIMER_H
