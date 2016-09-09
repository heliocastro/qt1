/****************************************************************************
** $Id: qfile.h,v 2.6.2.2 1998/08/21 19:13:24 hanord Exp $
**
** Definition of QFile class
**
** Created : 930831
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

#ifndef QFILE_H
#define QFILE_H

#ifndef QT_H
#include "qiodevice.h"
#include "qstring.h"
#include <stdio.h>
#endif // QT_H

class QDir;


class Q_EXPORT QFile : public QIODevice			// file I/O device class
{
public:
    QFile();
    QFile( const char *name );
   ~QFile();

    const char *name()	const;
    void	setName( const char *name );

    bool	exists()   const;
    static bool exists( const char *fileName );

    bool	remove();
    static bool remove( const char *fileName );

    bool	open( int );
    bool	open( int, FILE * );
    bool	open( int, int );
    void	close();
    void	flush();

    uint	size()	const;
    int		at()	const;
    bool	at( int );
    bool	atEnd() const;

    int		readBlock( char *data, uint len );
    int		writeBlock( const char *data, uint len );
    int		readLine( char *data, uint maxlen );

    int		getch();
    int		putch( int );
    int		ungetch( int );

    int		handle() const;

protected:
    QString	fn;
    FILE       *fh;
    int		fd;
    int		length;
    bool	ext_f;

private:
    void	init();

private:	// Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QFile( const QFile & );
    QFile &operator=( const QFile & );
#endif
};


inline const char *QFile::name() const
{ return fn; }

inline int QFile::at() const
{ return index; }


#endif // QFILE_H
