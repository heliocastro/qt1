/****************************************************************************
** $Id: qaccel.h,v 2.8.2.2 1998/08/21 19:13:22 hanord Exp $
**
** Definition of QAccel class
**
** Created : 950419
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

#ifndef QACCEL_H
#define QACCEL_H

#ifndef QT_H
#include "qobject.h"
#include "qkeycode.h"
#endif // QT_H


class QAccelList;				// internal class


class Q_EXPORT QAccel : public QObject			// accelerator class
{
    Q_OBJECT
public:
    QAccel( QWidget *parent, const char *name=0 );
   ~QAccel();

    bool	isEnabled() const;
    void	setEnabled( bool );

    uint	count() const;

    int		insertItem( int key, int id=-1 );
    void	removeItem( int id );
    void	clear();

    int		key( int id );
    int		findKey( int key ) const;

    bool	isItemEnabled( int id )	 const;
    void	setItemEnabled( int id, bool enable );

    bool	connectItem( int id,
			     const QObject *receiver, const char *member );
    bool	disconnectItem( int id,
				const QObject *receiver, const char *member );

    void	repairEventFilter();

signals:
    void	activated( int id );

protected:
    bool	eventFilter( QObject *, QEvent * );

private slots:
    void	tlwDestroyed();

private:
    QAccelList *aitems;
    bool	enabled;
    QWidget    *tlw;

private:	// Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QAccel( const QAccel & );
    QAccel &operator=( const QAccel & );
#endif
};


inline bool QAccel::isEnabled() const
{
    return enabled;
}


#endif // QACCEL_H
