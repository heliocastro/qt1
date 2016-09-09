/****************************************************************************
** $Id: qclipboard.h,v 2.6.2.2 1998/08/21 19:13:22 hanord Exp $
**
** Definition of QClipboard class
**
** Created : 960430
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

#ifndef QCLIPBOARD_H
#define QCLIPBOARD_H

#ifndef QT_H
#include "qobject.h"
#endif // QT_H


class Q_EXPORT QClipboard : public QObject
{
    Q_OBJECT
private:
    QClipboard( QObject *parent=0, const char *name=0 );
   ~QClipboard();

public:
    void	clear();

    void       *data( const char *format ) const;
    void	setData( const char *format, void * );

    const char *text()	 const;
    void	setText( const char * );
    QPixmap    *pixmap() const;
    void	setPixmap( const QPixmap & );

signals:
    void	dataChanged();

private slots:
    void	ownerDestroyed();

protected:
    void	connectNotify( const char * );
    bool	event( QEvent * );

    friend class QApplication;
    friend class QDragManager;

private:	// Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QClipboard( const QClipboard & );
    QClipboard &operator=( const QClipboard & );
#endif
};


#endif // QCLIPBOARD_H
