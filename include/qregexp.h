/****************************************************************************
** $Id: qregexp.h,v 2.3.2.1 1998/08/19 16:02:37 agulbra Exp $
**
** Definition of QRegExp class
**
** Created : 950126
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

#ifndef QREGEXP_H
#define QREGEXP_H

#ifndef QT_H
#include "qstring.h"
#endif // QT_H


class Q_EXPORT QRegExp
{
public:
    QRegExp();
    QRegExp( const char *, bool caseSensitive=TRUE, bool wildcard=FALSE );
    QRegExp( const QRegExp & );
   ~QRegExp();
    QRegExp    &operator=( const QRegExp & );
    QRegExp    &operator=( const char *pattern );

    bool	operator==( const QRegExp & )  const;
    bool	operator!=( const QRegExp &r ) const
					{ return !(this->operator==(r)); }

    bool	isEmpty()	const	{ return rxdata == 0; }
    bool	isValid()	const	{ return error == 0; }

    bool	caseSensitive() const	{ return cs; }
    void	setCaseSensitive( bool );

    bool	wildcard()	const	{ return wc; }
    void	setWildcard( bool );

    const char *pattern()	const	{ return (const char *)rxstring; }

    int		match( const char *str, int index=0, int *len=0 ) const;

protected:
    void	compile();
    char       *matchstr( ushort *, char *, char * ) const;

private:
    QString	rxstring;			// regular expression pattern
    ushort     *rxdata;				// compiled regexp pattern
    int		error;				// error status
    bool	cs;				// case sensitive
    bool	wc;				// wildcard
};


#endif // QREGEXP_H
