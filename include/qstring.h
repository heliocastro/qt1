/****************************************************************************
** $Id: qstring.h,v 2.10.2.4 1999/02/17 21:27:16 warwick Exp $
**
** Definition of extended char array operations, and QByteArray and
** QString classes
**
** Created : 920609
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

#ifndef QSTRING_H
#define QSTRING_H

#ifndef QT_H
#include "qarray.h"
#endif // QT_H

#include <string.h>

#if defined(_OS_SUN_) && defined(_CC_GNU_)
#include <strings.h>
#endif


/*****************************************************************************
  Fixes and workarounds for some platforms
 *****************************************************************************/

#if defined(_OS_HPUX_)
// HP-UX has badly defined strstr() etc.
inline char *hack_strstr( const char *s1, const char *s2 )
{ return (char *)strstr(s1, s2); }
inline char *hack_strchr( const char *s, int c )
{ return (char *)strchr(s, c); }
inline char *hack_strrchr( const char *s, int c )
{ return (char *)strrchr(s, c); }
#define strstr	hack_strstr
#define strchr	hack_strchr
#define strrchr hack_strrchr
#endif


/*****************************************************************************
  Safe and portable C string functions; extensions to standard string.h
 *****************************************************************************/

Q_EXPORT void *qmemmove( void *dst, const void *src, uint len );

#if defined(_OS_SUN_) || defined(_CC_OC_)
#define memmove qmemmove
#endif

Q_EXPORT char *qstrdup( const char * );

Q_EXPORT inline uint cstrlen( const char *str )
{ return strlen(str); }

Q_EXPORT inline uint qstrlen( const char *str )
{ return str ? strlen(str) : 0; }

#undef	strlen
#define strlen qstrlen

Q_EXPORT inline char *cstrcpy( char *dst, const char *src )
{ return strcpy(dst,src); }

Q_EXPORT inline char *qstrcpy( char *dst, const char *src )
{ return src ? strcpy(dst, src) : 0; }

#undef	strcpy
#define strcpy qstrcpy

Q_EXPORT char *qstrncpy( char *dst, const char *src, uint len );

Q_EXPORT inline int cstrcmp( const char *str1, const char *str2 )
{ return strcmp(str1,str2); }

Q_EXPORT inline int qstrcmp( const char *str1, const char *str2 )
{ return (str1 && str2) ? strcmp(str1,str2) : (int)((long)str2 - (long)str1); }

#undef	strcmp
#define strcmp qstrcmp

Q_EXPORT inline int cstrncmp( const char *str1, const char *str2, uint len )
{ return strncmp(str1,str2,len); }

Q_EXPORT inline int qstrncmp( const char *str1, const char *str2, uint len )
{ return (str1 && str2) ? strncmp(str1,str2,len) :
			  (int)((long)str2 - (long)str1); }

#undef	strncmp
#define strncmp qstrncmp

Q_EXPORT int qstricmp( const char *, const char * );
Q_EXPORT int qstrnicmp( const char *, const char *, uint len );

#undef	stricmp
#define stricmp	 qstricmp
#undef	strnicmp
#define strnicmp qstrnicmp


// qchecksum: Internet checksum

#if 1	/* OBSOLETE */
#if !defined(QT_CLEAN_NAMESPACE)
Q_EXPORT UINT16 qchecksum( const char *s, uint len );
#endif
#endif
Q_EXPORT Q_UINT16 qChecksum( const char *s, uint len );

/*****************************************************************************
  QByteArray class
 *****************************************************************************/

#if defined(USE_TEMPLATECLASS)
#if defined(Q_TEMPLATEDLL)
template class Q_EXPORT QArray<char>;
typedef QArray<char> QByteArray;
#else
#define QByteArray QArrayT<char>
#endif
#else
Q_DECLARE(QArrayM,char);
#define QByteArray QArrayM(char)
#endif


/*****************************************************************************
  QByteArray stream functions
 *****************************************************************************/

Q_EXPORT QDataStream &operator<<( QDataStream &, const QByteArray & );
Q_EXPORT QDataStream &operator>>( QDataStream &, QByteArray & );


/*****************************************************************************
  QString class
 *****************************************************************************/

class QRegExp;

class Q_EXPORT QString : public QByteArray		// string class
{
public:
    QString() {}				// make null string
    QString( int size );			// allocate size incl. \0
    QString( const QString &s ) : QByteArray( s ) {}
    QString( const char *str );			// deep copy
    QString( const char *str, uint maxlen );	// deep copy, max length

    QString    &operator=( const QString &s );	// shallow copy
    QString    &operator=( const char *str );	// deep copy

    bool	isNull()	const;
    bool	isEmpty()	const;
    uint	length()	const;
    bool	resize( uint newlen );
    bool	truncate( uint pos );
    bool	fill( char c, int len = -1 );

    QString	copy()	const;

    QString    &sprintf( const char *format, ... );

    int		find( char c, int index=0, bool cs=TRUE ) const;
    int		find( const char *str, int index=0, bool cs=TRUE ) const;
    int		find( const QRegExp &, int index=0 ) const;
    int		findRev( char c, int index=-1, bool cs=TRUE) const;
    int		findRev( const char *str, int index=-1, bool cs=TRUE) const;
    int		findRev( const QRegExp &, int index=-1 ) const;
    int		contains( char c, bool cs=TRUE ) const;
    int		contains( const char *str, bool cs=TRUE ) const;
    int		contains( const QRegExp & ) const;

    QString	left( uint len )  const;
    QString	right( uint len ) const;
    QString	mid( uint index, uint len) const;

    QString	leftJustify( uint width, char fill=' ', bool trunc=FALSE)const;
    QString	rightJustify( uint width, char fill=' ',bool trunc=FALSE)const;

    QString	lower() const;
    QString	upper() const;

    QString	stripWhiteSpace()	const;
    QString	simplifyWhiteSpace()	const;

    QString    &insert( uint index, const char * );
    QString    &insert( uint index, char );
    QString    &append( const char * );
    QString    &prepend( const char * );
    QString    &remove( uint index, uint len );
    QString    &replace( uint index, uint len, const char * );
    QString    &replace( const QRegExp &, const char * );

    short	toShort( bool *ok=0 )	const;
    ushort	toUShort( bool *ok=0 )	const;
    int		toInt( bool *ok=0 )	const;
    uint	toUInt( bool *ok=0 )	const;
    long	toLong( bool *ok=0 )	const;
    ulong	toULong( bool *ok=0 )	const;
    float	toFloat( bool *ok=0 )	const;
    double	toDouble( bool *ok=0 )	const;

    QString    &setStr( const char *s );
    QString    &setNum( short );
    QString    &setNum( ushort );
    QString    &setNum( int );
    QString    &setNum( uint );
    QString    &setNum( long );
    QString    &setNum( ulong );
    QString    &setNum( float, char f='g', int prec=6 );
    QString    &setNum( double, char f='g', int prec=6 );

    bool	setExpand( uint index, char c );

		operator const char *() const;
    QString    &operator+=( const char *str );
    QString    &operator+=( char c );
};


/*****************************************************************************
  QString stream functions
 *****************************************************************************/

Q_EXPORT QDataStream &operator<<( QDataStream &, const QString & );
Q_EXPORT QDataStream &operator>>( QDataStream &, QString & );


/*****************************************************************************
  QString inline functions
 *****************************************************************************/

inline QString &QString::operator=( const QString &s )
{ return (QString&)assign( s ); }

inline QString &QString::operator=( const char *str )
{ return (QString&)duplicate( str, strlen(str)+1 ); }

inline bool QString::isNull() const
{ return data() == 0; }

inline bool QString::isEmpty() const
{ return data() == 0 || *data() == '\0'; }

inline uint QString::length() const
{ return strlen( data() ); }

inline bool QString::truncate( uint pos )
{ return resize(pos+1); }

inline QString QString::copy() const
{ return QString( data() ); }

inline QString &QString::prepend( const char *s )
{ return insert(0,s); }

inline QString &QString::append( const char *s )
{ return operator+=(s); }

inline QString &QString::setNum( short n )
{ return setNum((long)n); }

inline QString &QString::setNum( ushort n )
{ return setNum((ulong)n); }

inline QString &QString::setNum( int n )
{ return setNum((long)n); }

inline QString &QString::setNum( uint n )
{ return setNum((ulong)n); }

inline QString &QString::setNum( float n, char f, int prec )
{ return setNum((double)n,f,prec); }

inline QString::operator const char *() const
{ return (const char *)data(); }


/*****************************************************************************
  QString non-member operators
 *****************************************************************************/

Q_EXPORT inline bool operator==( const QString &s1, const QString &s2 )
{ return strcmp(s1.data(),s2.data()) == 0; }

Q_EXPORT inline bool operator==( const QString &s1, const char *s2 )
{ return strcmp(s1.data(),s2) == 0; }

Q_EXPORT inline bool operator==( const char *s1, const QString &s2 )
{ return strcmp(s1,s2.data()) == 0; }

Q_EXPORT inline bool operator!=( const QString &s1, const QString &s2 )
{ return strcmp(s1.data(),s2.data()) != 0; }

Q_EXPORT inline bool operator!=( const QString &s1, const char *s2 )
{ return strcmp(s1.data(),s2) != 0; }

Q_EXPORT inline bool operator!=( const char *s1, const QString &s2 )
{ return strcmp(s1,s2.data()) != 0; }

Q_EXPORT inline bool operator<( const QString &s1, const char *s2 )
{ return strcmp(s1.data(),s2) < 0; }

Q_EXPORT inline bool operator<( const char *s1, const QString &s2 )
{ return strcmp(s1,s2.data()) < 0; }

Q_EXPORT inline bool operator<=( const QString &s1, const char *s2 )
{ return strcmp(s1.data(),s2) <= 0; }

Q_EXPORT inline bool operator<=( const char *s1, const QString &s2 )
{ return strcmp(s1,s2.data()) <= 0; }

Q_EXPORT inline bool operator>( const QString &s1, const char *s2 )
{ return strcmp(s1.data(),s2) > 0; }

Q_EXPORT inline bool operator>( const char *s1, const QString &s2 )
{ return strcmp(s1,s2.data()) > 0; }

Q_EXPORT inline bool operator>=( const QString &s1, const char *s2 )
{ return strcmp(s1.data(),s2) >= 0; }

Q_EXPORT inline bool operator>=( const char *s1, const QString &s2 )
{ return strcmp(s1,s2.data()) >= 0; }

Q_EXPORT inline QString operator+( const QString &s1, const QString &s2 )
{
    QString tmp( s1.data() );
    tmp += s2;
    return tmp;
}

Q_EXPORT inline QString operator+( const QString &s1, const char *s2 )
{
    QString tmp( s1.data() );
    tmp += s2;
    return tmp;
}

Q_EXPORT inline QString operator+( const char *s1, const QString &s2 )
{
    QString tmp( s1 );
    tmp += s2;
    return tmp;
}

Q_EXPORT inline QString operator+( const QString &s1, char c2 )
{
    QString tmp( s1.data() );
    tmp += c2;
    return tmp;
}

Q_EXPORT inline QString operator+( char c1, const QString &s2 )
{
    QString tmp;
    tmp += c1;
    tmp += s2;
    return tmp;
}


#endif // QSTRING_H
