/****************************************************************************
** $Id: qstring.cpp,v 2.22.2.4 1998/09/15 09:36:50 warwick Exp $
**
** Implementation of extended char array operations, and QByteArray and
** QString classes
**
** Created : 920722
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

#define	 QSTRING_C
#include "qstring.h"
#include "qdatastream.h"
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <ctype.h>

/*****************************************************************************
  Safe and portable C string functions; extensions to standard string.h
 *****************************************************************************/

/*!
  \fn void *memmove( void *dst, const void *src, uint len )
  \relates QString

  This function is normally part of the C library. Qt implements
  memmove() for platforms that do not have it.

  memmove() copies \e len bytes from \e src into \e dst.  The data is
  copied correctly even if \e src and \e dst overlap.
*/

void *qmemmove( void *dst, const void *src, uint len )
{
    register char *d;
    register char *s;
    if ( dst > src ) {
	d = (char *)dst + len - 1;
	s = (char *)src + len - 1;
	while ( len-- )
	    *d-- = *s--;
    } else if ( dst < src ) {
	d = (char *)dst;
	s = (char *)src;
	while ( len-- )
	    *d++ = *s++;
    }
    return dst;
}

/*!
  \relates QString

  Returns a duplicate string.

  Allocates space for a copy of \e str (using \c new), copies it, and returns
  a pointer to the copy.
  If \e src is null, it immediately returns 0.
*/

char *qstrdup( const char *str )
{
    if ( !str )
	return 0;
    char *dst = new char[strlen(str)+1];
    CHECK_PTR( dst );
    return cstrcpy( dst, str );
}

/*!
  \fn uint strlen( const char *str )
  \relates QString

  A safe strlen() function that overrides the one defined by the C library.
  The original strlen() function has been renamed cstrlen().

  Returns the number of characters in \e str, or 0 if \e str is null.
*/

/*!
  \fn char *strcpy( char *dst, const char *str )
  \relates QString

  A safe strcpy() function that overrides the one defined by the C library.
  The original strcpy() function has been renamed cstrcpy().

  Copies all characters (including \0) from \e str into \e dst and returns
  a pointer to \e dst.
  If \e src is null, it immediately returns 0.

  \sa qstrncpy()
*/

/*!
  \relates QString

  A safe strncpy() function.

  Copies all characters up to \e len bytes from \e str into \e dst and returns
  a pointer to \e dst.	Guarantees that \e dst is \0-terminated.
  If \e src is null, it immediately returns 0.

  \sa strcpy()
*/

char *qstrncpy( char *dst, const char *src, uint len )
{
    if ( !src )
	return 0;
    strncpy( dst, src, len );
    if ( len > 0 )
	dst[len-1] = '\0';
    return dst;
}

/*!
  \fn int strcmp( const char *str1, const char *str2 )
  \relates QString

  A safe strcmp() function that overrides the one defined by the C library.
  The original strcmp() function has been renamed cstrcmp().

  Compares \e str1 and \e str2.	 Returns a negative value if \e str1
  is less than \e str2, 0 if \e str1 is equal to \e str2 or a positive
  value if \e str1 is greater than \e str2.

  Special case I: Returns 0 if \e str1 and \e str2 are both null.

  Special case II: Returns a random nonzero value if \e str1 is null
  or \e str2 is null (but not both).

  \sa strncmp(), stricmp(), strnicmp()
*/

/*!
  \fn int strncmp( const char *str1, const char *str2, uint len )
  \relates QString

  A safe strncmp() function that overrides the one defined by the C library.
  The original strncmp() function has been renamed cstrncmp().

  Compares \e str1 and \e str2 up to \e len bytes.

  Returns a negative value if \e str1 is less than \e str2, 0 if \e str1
  is equal to \e str2 or a positive value if \e str1 is greater than \e
  str2.

  Special case I: Returns 0 if \e str1 and \e str2 are both null.

  Special case II: Returns a random nonzero value if \e str1 is null
  or \e str2 is null (but not both).

  \sa strcmp(), stricmp(), strnicmp()
*/

/*!
  \fn int stricmp( const char *str1, const char *str2 )
  \relates QString

  A safe stricmp() function that overrides the one defined by the C library,
  if the C library has one.

  Compares \e str1 and \e str2 ignoring the case.

  Returns a negative value if \e str1 is less than \e str2, 0 if \e str1
  is equal to \e str2 or a positive value if \e str1 is greater than \e
  str2.

  Special case I: Returns 0 if \e str1 and \e str2 are both null.

  Special case II: Returns a random nonzero value if \e str1 is null
  or \e str2 is null (but not both).

  \sa strcmp(), strncmp(), strnicmp()
*/

int qstricmp( const char *str1, const char *str2 )
{
    register const uchar *s1 = (const uchar *)str1;
    register const uchar *s2 = (const uchar *)str2;
    int res;
    uchar c;
    if ( !s1 || !s2 )
	return s1 == s1 ? 0 : (int)((long)s2 - (long)s1);
    for ( ; !(res = (c=tolower(*s1)) - tolower(*s2)); s1++, s2++ )
	if ( !c )				// strings are equal
	    break;
    return res;
}

/*!
  \fn int strnicmp( const char *str1, const char *str2, uint len )
  \relates QString

  A safe strnicmp() function that overrides the one defined by the C library,
  if the C library has one.

  Compares \e str1 and \e str2 up to \e len bytes ignoring the case.

  Returns a negative value if \e str1 is less than \e str2, 0 if \e str1
  is equal to \e str2 or a positive value if \e str1 is greater than \e
  str2.

  Special case I: Returns 0 if \e str1 and \e str2 are both null.

  Special case II: Returns a random nonzero value if \e str1 is null
  or \e str2 is null (but not both).

  \sa strcmp(), strncmp() stricmp()
*/

int qstrnicmp( const char *str1, const char *str2, uint len )
{
    register const uchar *s1 = (const uchar *)str1;
    register const uchar *s2 = (const uchar *)str2;
    int res;
    uchar c;
    if ( !s1 || !s2 )
	return (int)((long)s2 - (long)s1);
    for ( ; len--; s1++, s2++ ) {
	if ( (res = (c=tolower(*s1)) - tolower(*s2)) )
	    return res;
	if ( !c )				// strings are equal
	    break;
    }
    return 0;
}


static Q_UINT16 crc_tbl[16];
static bool   crc_tbl_init = FALSE;

static void createCRC16Table()			// build CRC16 lookup table
{
    register uint i;
    register uint j;
    uint v0, v1, v2, v3;
    for ( i=0; i<16; i++ ) {
	v0 = i & 1;
	v1 = (i >> 1) & 1;
	v2 = (i >> 2) & 1;
	v3 = (i >> 3) & 1;
	j = 0;
#undef	SET_BIT
#define SET_BIT(x,b,v)	x |= v << b
	SET_BIT(j, 0,v0);
	SET_BIT(j, 7,v0);
	SET_BIT(j,12,v0);
	SET_BIT(j, 1,v1);
	SET_BIT(j, 8,v1);
	SET_BIT(j,13,v1);
	SET_BIT(j, 2,v2);
	SET_BIT(j, 9,v2);
	SET_BIT(j,14,v2);
	SET_BIT(j, 3,v3);
	SET_BIT(j,10,v3);
	SET_BIT(j,15,v3);
	crc_tbl[i] = j;
    }
}

/*!
  \relates QByteArray
  Returns the CRC-16 checksum of \e len bytes starting at \e data.

  The checksum is independent of the byte order (endianness).
*/

Q_UINT16 qChecksum( const char *data, uint len )
{
    if ( !crc_tbl_init ) {			// create lookup table
	createCRC16Table();
	crc_tbl_init = TRUE;
    }
    register Q_UINT16 crc = 0xffff;
    uchar c;
    uchar *p = (uchar *)data;
    while ( len-- ) {
	c = *p++;
	crc = ((crc >> 4) & 0x0fff) ^ crc_tbl[((crc ^ c) & 15)];
	c >>= 4;
	crc = ((crc >> 4) & 0x0fff) ^ crc_tbl[((crc ^ c) & 15)];
    }
    return (~crc & 0xffff);
}

/*!
  \relates QByteArray

  Please use qChecksum; this function is provided only for
  upward compatibility.
*/

UINT16 qchecksum( const char *data, uint len )
{
    if ( !crc_tbl_init ) {			// create lookup table
	createCRC16Table();
	crc_tbl_init = TRUE;
    }
    register Q_UINT16 crc = 0xffff;
    uchar c;
    uchar *p = (uchar *)data;
    while ( len-- ) {
	c = *p++;
	crc = ((crc >> 4) & 0x0fff) ^ crc_tbl[((crc ^ c) & 15)];
	c >>= 4;
	crc = ((crc >> 4) & 0x0fff) ^ crc_tbl[((crc ^ c) & 15)];
    }
    return (~crc & 0xffff);
}

// Qt 1.41 ships with this set to 201, as we have only ever found *bugs*
// that relied on explicitly shared QStrings.
#ifndef QSTRING_COMPATIBILITY
#define QSTRING_COMPATIBILITY 201
#endif

#ifndef QSTRING_COMPATIBILITY
#define QSTRING_COMPATIBILITY QT_VERSION
#endif

#if QSTRING_COMPATIBILITY < 200
//
// Explicitly-shared QString
//
# define QSTRING_DETACH(s)
#else
# if QSTRING_COMPATIBILITY == 200
//
// Implicitly-shared QString, with warnings on potentially buggy old calls.
//
static bool warned1nce=FALSE;
#include "qapplication.h"
#  define QSTRING_DETACH(s) { if ( !warned1nce && (s)->nrefs()>1) \
    {	warned1nce=TRUE;\
	warning("%s: appears to rely on old behavior of QString",\
	qApp->argv()[0] ? qApp->argv()[0] : ""); }\
	(s)->detach(); }
# else
//
// Implicitly-shared QString, with warnings turned off, once it is safe
// to rely on the new QString behavior.
//
#  define QSTRING_DETACH(s) (s)->detach();
# endif
#endif


/*****************************************************************************
  QByteArray member functions
 *****************************************************************************/

/*!
  \class QByteArray qstring.h
  \brief The QByteArray class provides an array of bytes.

  \inherit QArray
  \ingroup tools
  \ingroup shared

  QByteArray is defined as QArray\<char\>.
*/


/*****************************************************************************
  QByteArray stream functions
 *****************************************************************************/

/*!
  \relates QByteArray
  Writes a byte array to a stream and returns a reference to the stream.

  The serialization format is the byte array size (\c Q_UINT32) followed by
  this number of bytes.
*/

QDataStream &operator<<( QDataStream &s, const QByteArray &a )
{
    return s.writeBytes( a.data(), a.size() );
}

/*!
  \relates QByteArray
  Reads a byte array from a stream and returns a reference to the stream.
*/

QDataStream &operator>>( QDataStream &s, QByteArray &a )
{
    Q_UINT32 len;
    s >> len;					// read size of array
    if ( len == 0 || s.eof() ) {		// end of file reached
	a.resize( 0 );
	return s;
    }
    if ( !a.resize( (uint)len ) ) {		// resize array
#if defined(CHECK_NULL)
	warning( "QDataStream: Not enough memory to read QByteArray" );
#endif
	len = 0;
    }
    if ( len > 0 )				// not null array
	s.readRawBytes( a.data(), (uint)len );
    return s;
}


/*****************************************************************************
  QString member functions
 *****************************************************************************/

/*!
  \class QString qstring.h

  \brief The QString class provides an abstraction of the classic C
  zero-terminated char array (<var>char*</var>).

  \ingroup tools
  \ingroup shared

  QString inherits QByteArray, which is defined as QArray\<char\>.

  Since QString is a QArray, it uses explicit
  \link shclass.html sharing\endlink with a reference count.

  Note that for the QString methods that take a <var>const char *</var>
  parameter the results are undefined if the QString is not
  zero-terminated.  It is legal for the <var>const char *</var> parameter
  to be 0.

  A QString that has not been assigned to anything is \e null, i.e. both
  the length and data pointer is 0. A QString that references the empty
  string ("", a single '\0' char) is \e empty.	Both null and empty
  QStrings are legal parameters to the methods. Assigning <var>const char
  * 0</var> to QString gives a null QString.

  \sa \link shclass.html Shared classes\endlink
*/


/*!
  \fn QString::QString()
  Constructs a null string.
  \sa isNull()
*/

/*!
  \fn QString::QString( const QString &s )
  Constructs a shallow copy \e s.
  \sa assign()
*/

/*!
  Constructs a string with room for \e size characters, including the
  '\0'-terminator.  Makes a null string if \e size == 0.

  If \e size \> 0, then the first and last characters in the string are
  initialized to '\0'.	All other characters are uninitialized.

  \sa resize(), isNull()
*/

QString::QString( int size )
    : QByteArray( size )
{
    if ( size > 0 ) {
	*data() = '\0';				// set terminator
	*(data()+(size-1)) = '\0';
    }
}

/*!
  Constructs a string that is a deep copy of \e str.

  If \a str is 0 a null string is created.

  \sa isNull()
*/

QString::QString( const char *str )
{
    duplicate( str, strlen(str)+1 );
}


/*!
  Constructs a string that is a deep copy of \e str, that is no more
  than \a maxlen bytes long including the '\0'-terminator.

  Example:
  \code
    QString str("helloworld",6); // Assigns "hello" to str.
  \endcode

  If \a str contains a 0 byte within the first \a maxlen bytes, the
  resulting QString will be terminated by the 0.  If \a str is 0 a
  null string is created.

  \sa isNull()
*/

QString::QString( const char *str, uint maxlen )
{
    uint len; // index of last nul character
    // cannot use memchr - we need the *index*, not void*
    for (len=0; len<maxlen-1; len++) {
	if (!str[len]) break;
    }
    duplicate( str, len+1 );
    data()[len] = 0;
}

/*!
  \fn QString &QString::operator=( const QString &s )
  Assigns a shallow copy of \e s to this string and returns a reference to
  this string.
*/

/*!
  \fn QString &QString::operator=( const char *str )
  Assigns a deep copy of \a str to this string and returns a reference to
  this string.

  If \a str is 0 a null string is created.

  \sa isNull()
*/

/*!
  \fn bool QString::isNull() const
  Returns TRUE if the string is null, i.e. if data() == 0.
  A null string is also an empty string.

  Example:
  \code
    QString a;		// a.data() == 0,  a.size() == 0, a.length() == 0
    QString b == "";	// b.data() == "", b.size() == 1, b.length() == 0
    a.isNull();		// TRUE, because a.data() == 0
    a.isEmpty();	// TRUE, because a.length() == 0
    b.isNull();		// FALSE, because b.data() == ""
    b.isEmpty();	// TRUE, because b.length() == 0
  \endcode

  \sa isEmpty(), length(), size()
*/

/*!
  \fn bool QString::isEmpty() const

  Returns TRUE if the string is empty, i.e. if length() == 0.
  An empty string is not always a null string.

  See example in isNull().

  \sa isNull(), length(), size()
*/

/*!
  \fn uint QString::length() const
  Returns the length of the string, excluding the '\0'-terminator.
  Equivalent to calling \c strlen(data()).

  Null strings and empty strings have zero length.

  \sa size(), isNull(), isEmpty()
*/

/*!
  \fn bool QString::truncate( uint pos )
  Truncates the string at position \e pos.

  Equivalent to calling \c resize(pos+1).

  Example:
  \code
    QString s = "truncate this string";
    s.truncate( 5 );				// s == "trunc"
  \endcode

  \sa resize()
*/

/*!
  Extends or shrinks the string to \e len bytes, including the
  '\0'-terminator.

  A \0-terminator is set at position <code>len - 1</code> unless
  <code>len == 0</code>.

  Example:
  \code
    QString s = "resize this string";
    s.resize( 7 );				// s == "resize"
  \endcode

  \sa truncate()
*/

bool QString::resize( uint len )
{
    QSTRING_DETACH(this)
    if ( !QByteArray::resize(len) )
	return FALSE;
    if ( len )
	*(data()+len-1) = '\0';
    return TRUE;
}


/*!
  Implemented as a call to the native vsprintf() (see your C-library
  manual).

  If your string is shorter than 256 characters, this sprintf() calls
  resize(256) to decrease the chance of memory corruption.  The string is
  resized back to its natural length before sprintf() returns.

  Example:
  \code
    QString s;
    s.sprintf( "%d - %s", 1, "first" );		// result < 256 chars

    QString big( 25000 );			// very long string
    big.sprintf( "%d - %s", 2, longString );	// result < 25000 chars
  \endcode

  \warning All vsprintf() implementations will write past the end of
  the target string (*this) if the format specification and arguments
  happen to be longer than the target string, and some will also fail
  if the target string is longer than some arbitrary implementation
  limit.

  Giving user-supplied arguments to sprintf() is begging for trouble.
  Sooner or later someone \e will paste a 3000-character line into
  your application.
*/

QString &QString::sprintf( const char *format, ... )
{
    QSTRING_DETACH(this)
    va_list ap;
    va_start( ap, format );
    if ( size() < 256 )
	QByteArray::resize( 256 );		// make string big enough
    vsprintf( data(), format, ap );
    resize( strlen(data()) + 1 );		// truncate
    va_end( ap );
    return *this;
}


/*!
  Fills the string with \e len bytes of value \e c, followed by a
  '\0'-terminator.

  If \e len is negative, then the current string length is used.

  Returns FALSE is \e len is nonnegative and there is no memory to
  resize the string, otherwise TRUE is returned.
*/

bool QString::fill( char c, int len )
{
    QSTRING_DETACH(this)
    if ( len < 0 )
	len = length();
    if ( !QByteArray::fill(c,len+1) )
	return FALSE;
    *(data()+len) = '\0';
    return TRUE;
}


/*!
  \fn QString QString::copy() const
  Returns a deep copy of this string.
  \sa detach()
*/


/*!
  Finds the first occurrence of the character \e c, starting at
  position \e index.

  The search is case sensitive if \e cs is TRUE, or case insensitive if \e
  cs is FALSE.

  Returns the position of \e c, or -1 if \e c could not be found.
*/

int QString::find( char c, int index, bool cs ) const
{
    if ( (uint)index >= size() )		// index outside string
	return -1;
    register const char *d;
    if ( cs ) {					// case sensitive
	d = strchr( data()+index, c );
    } else {
	d = data()+index;
	c = tolower( c );
	while ( *d && tolower(*d) != c )
	    d++;
	if ( !*d && c )				// not found
	    d = 0;
    }
    return d ? (int)(d - data()) : -1;
}

/*!
  Finds the first occurrence of the string \e str, starting at position
  \e index.

  The search is case sensitive if \e cs is TRUE, or case insensitive if \e
  cs is FALSE.

  Returns the position of \e str, or -1 if \e str could not be found.
*/

int QString::find( const char *str, int index, bool cs ) const
{
    if ( (uint)index >= size() )		// index outside string
	return -1;
    register const char *d;
    if ( cs ) {					// case sensitive
	d = strstr( data()+index, str );
    } else {					// case insensitive
	d = data()+index;
	int len = strlen( str );
	while ( *d ) {
	    if ( strnicmp(d, str, len) == 0 )
		break;
	    d++;
	}
	if ( !*d )				// not found
	    d = 0;
    }
    return d ? (int)(d - data()) : -1;
}

/*!
  Finds the first occurrence of the character \e c, starting at
  position \e index and searching backwards.

  The search is case sensitive if \e cs is TRUE, or case insensitive if \e
  cs is FALSE.

  Returns the position of \e c, or -1 if \e c could not be found.
*/

int QString::findRev( char c, int index, bool cs ) const
{
    const char *b = data();
    const char *d;
    if ( index < 0 ) {				// neg index ==> start from end
	if ( size() == 0 )
	    return -1;
	if ( cs ) {
	    d = strrchr( b, c );
	    return d ? (int)(d - b) : -1;
	}
	index = length();
    } else if ( (uint)index >= size() ) {	// bad index
	return -1;
    }
    d = b+index;
    if ( cs ) {					// case sensitive
	while ( d >= b && *d != c )
	    d--;
    } else {					// case insensitive
	c = tolower( c );
	while ( d >= b && tolower(*d) != c )
	    d--;
    }
    return d >= b ? (int)(d - b) : -1;
}

/*!
  Finds the first occurrence of the string \e str, starting at
  position \e index and searching backwards.

  The search is case sensitive if \e cs is TRUE, or case insensitive if \e
  cs is FALSE.

  Returns the position of \e str, or -1 if \e str could not be found.
*/

int QString::findRev( const char *str, int index, bool cs ) const
{
    int slen = strlen(str);
    if ( index < 0 )				// neg index ==> start from end
	index = length()-slen;
    else if ( (uint)index >= size() )		// bad index
	return -1;
    else if ( (uint)(index + slen) > length() ) // str would be too long
	index = length() - slen;
    if ( index < 0 )
	return -1;

    register char *d = data() + index;
    if ( cs ) {					// case sensitive
	for ( int i=index; i>=0; i-- )
	    if ( strncmp(d--,str,slen)==0 )
		return i;
    } else {					// case insensitive
	for ( int i=index; i>=0; i-- )
	    if ( strnicmp(d--,str,slen)==0 )
		return i;
    }
    return -1;
}


/*!
  Returns the number of times the character \e c occurs in the string.

  The match is case sensitive if \e cs is TRUE, or case insensitive if \e cs
  if FALSE.
*/

int QString::contains( char c, bool cs ) const
{
    int count = 0;
    char *d = data();
    if ( !d )
	return 0;
    if ( cs ) {					// case sensitive
	while ( *d )
	    if ( *d++ == c )
		count++;
    } else {					// case insensitive
	c = tolower( c );
	while ( *d ) {
	    if ( tolower(*d) == c )
		count++;
	    d++;
	}
    }
    return count;
}

/*!
  Returns the number of times \e str occurs in the string.

  The match is case sensitive if \e cs is TRUE, or case insensitive if \e
  cs if FALSE.

  This function counts overlapping substrings, for example, "banana"
  contains two occurrences of "ana".

  \sa findRev()
*/

int QString::contains( const char *str, bool cs ) const
{
    int count = 0;
    char *d = data();
    if ( !d )
	return 0;
    int len = strlen( str );
    while ( *d ) {				// counts overlapping strings
	if ( cs ) {
	    if ( strncmp( d, str, len ) == 0 )
		count++;
	} else {
	    if ( strnicmp(d, str, len) == 0 )
		count++;
	}
	d++;
    }
    return count;
}

/*!
  Returns a substring that contains the \e len leftmost characters
  of the string.

  The whole string is returned if \e len exceeds the length of the string.

  Example:
  \code
    QString s = "Pineapple";
    QString t = s.left( 4 );			// t == "Pine"
  \endcode

  \sa right(), mid()
*/

QString QString::left( uint len ) const
{
    if ( isEmpty() ) {
	QString empty;
	return empty;
    } else if ( len >= size() ) {
	QString same( data() );
	return same;
    } else {
	QString s( len+1 );
	strncpy( s.data(), data(), len );
	*(s.data()+len) = '\0';
	return s;
    }
}

/*!
  Returns a substring that contains the \e len rightmost characters
  of the string.

  The whole string is returned if \e len exceeds the length of the string.

  Example:
  \code
    QString s = "Pineapple";
    QString t = s.right( 5 );			// t == "apple"
  \endcode

  \sa left(), mid()
*/

QString QString::right( uint len ) const
{
    if ( isEmpty() ) {
	QString empty;
	return empty;
    } else {
	uint l = length();
	if ( len > l )
	    len = l;
	char *p = data() + (l - len);
	return QString( p );
    }
}

/*!
  Returns a substring that contains the \e len characters of this
  string, starting at position \e index.

  Returns a null string if the string is empty or \e index is out
  of range.  Returns the whole string from \e index if \e index+len exceeds
  the length of the string.

  Example:
  \code
    QString s = "Two pineapples";
    QString t = s.mid( 4, 4 );			// t == "pine"
  \endcode

  \sa left(), right()
*/

QString QString::mid( uint index, uint len ) const
{
    uint slen = strlen( data() );
    if ( isEmpty() || index >= slen ) {
	QString empty;
	return empty;
    } else {
	register char *p = data()+index;
	QString s( len+1 );
	strncpy( s.data(), p, len );
	*(s.data()+len) = '\0';
	return s;
    }
}

/*!
  Returns a string of length \e width (plus '\0') that contains this
  string and padded by the \e fill character.

  If the length of the string exceeds \e width and \e truncate is FALSE,
  then the returned string is a copy of the string.
  If the length of the string exceeds \e width and \e truncate is TRUE,
  then the returned string is a left(\e width).

  Example:
  \code
    QString s("apple");
    QString t = s.leftJustify(8, '.');		// t == "apple..."
  \endcode

  \sa rightJustify()
*/

QString QString::leftJustify( uint width, char fill, bool truncate ) const
{
    QString result;
    int len = strlen(data());
    int padlen = width - len;
    if ( padlen > 0 ) {
	result.QByteArray::resize( len+padlen+1 );
	memcpy( result.data(), data(), len );
	memset( result.data()+len, fill, padlen );
	result[len+padlen] = '\0';
    } else {
	if ( truncate )
	    result = left( width );
	else
	    result = copy();
    }
    return result;
}

/*!
  Returns a string of length \e width (plus '\0') that contains pad
  characters followed by the string.

  If the length of the string exceeds \e width and \e truncate is FALSE,
  then the returned string is a copy of the string.
  If the length of the string exceeds \e width and \e truncate is TRUE,
  then the returned string is a right(\e width).

  Example:
  \code
    QString s("pie");
    QString t = s.rightJustify(8, '.');		// t == ".....pie"
  \endcode

  \sa leftJustify()
*/

QString QString::rightJustify( uint width, char fill, bool truncate ) const
{
    QString result;
    int len = strlen(data());
    int padlen = width - len;
    if ( padlen > 0 ) {
	result.QByteArray::resize( len+padlen+1 );
	memset( result.data(), fill, padlen );
	memcpy( result.data()+padlen, data(), len );
	result[len+padlen] = '\0';
    } else {
	if ( truncate )
	    result = left( width );
	else
	    result = copy();
    }
    return result;
}

/*!
  Returns a new string that is the string converted to lower case.

  Presently it only handles 7-bit ASCII, or whatever tolower()
  handles (if $LC_CTYPE is set, most UNIX systems do the Right Thing).

  Example:
  \code
    QString s("TeX");
    QString t = s.lower();			// t == "tex"
  \endcode

  \sa upper()
*/

QString QString::lower() const
{
    QString s( data() );
    register char *p = s.data();
    if ( p ) {
	while ( *p ) {
	    *p = tolower(*p);
	    p++;
	}
    }
    return s;
}

/*!
  Returns a new string that is the string converted to upper case.

  Presently it only handles 7-bit ASCII, or whatever toupper()
  handles (if $LC_CTYPE is set, most UNIX systems do the Right Thing).

  Example:
  \code
    QString s("TeX");
    QString t = s.upper();			// t == "TEX"
  \endcode

  \sa lower()
*/

QString QString::upper() const
{
    QString s( data() );
    register char *p = s.data();
    if ( p ) {
	while ( *p ) {
	    *p = toupper(*p);
	    p++;
	}
    }
    return s;
}


/*!
  Returns a new string that has white space removed from the start and the end.

  White space means any ASCII code 9, 10, 11, 12, 13 or 32.

  Example:
  \code
    QString s = " space ";
    QString t = s.stripWhiteSpace();		// t == "space"
  \endcode

  \sa simplifyWhiteSpace()
*/

QString QString::stripWhiteSpace() const
{
    if ( isEmpty() )				// nothing to do
	return copy();

    register char *s = data();
    QString result = s;
    int reslen = result.length();
    if ( !isspace(s[0]) && !isspace(s[reslen-1]) )
	return result;				// returns a copy

    s = result.data();
    int start = 0;
    int end = reslen - 1;
    while ( isspace(s[start]) )			// skip white space from start
	start++;
    if ( s[start] == '\0' ) {			// only white space
	result.resize( 1 );
	return result;
    }
    while ( end && isspace(s[end]) )		// skip white space from end
	end--;
    end -= start - 1;
    memmove( result.data(), &s[start], end );
    result.resize( end + 1 );
    return result;
}


/*!
  Returns a new string that has white space removed from the start and the end,
  plus any sequence of internal white space replaced with a single space
  (ASCII 32).

  White space means any ASCII code 9, 10, 11, 12, 13 or 32.

  \code
    QString s = "  lots\t of\nwhite    space ";
    QString t = s.simplifyWhiteSpace();		// t == "lots of white space"
  \endcode

  \sa stripWhiteSpace()
*/

QString QString::simplifyWhiteSpace() const
{
    if ( isEmpty() )				// nothing to do
	return copy();
    QString result( size() );
    char *from	= data();
    char *to	= result.data();
    char *first = to;
    while ( TRUE ) {
	while ( *from && isspace(*from) )
	    from++;
	while ( *from && !isspace(*from) )
	    *to++ = *from++;
	if ( *from )
	    *to++ = 0x20;			// ' '
	else
	    break;
    }
    if ( to > first && *(to-1) == 0x20 )
	to--;
    *to = '\0';
    result.resize( (int)((long)to - (long)result.data()) + 1 );
    return result;
}


/*!
  Insert \e s into the string before position \e index.

  If \e index is beyond the end of the string, the string is extended with
  spaces (ASCII 32) to length \e index and \e s is then appended.

  \code
    QString s = "I like fish";
    s.insert( 2, "don't ");			// s == "I don't like fish"
    s = "x";
    s.insert( 3, "yz" );			// s == "x  yz"
  \endcode
*/

QString &QString::insert( uint index, const char *s )
{
    int len = strlen(s);
    if ( len == 0 )
	return *this;
    uint olen = length();
    int nlen = olen + len;
    if ( index >= olen ) {			// insert after end of string
	QSTRING_DETACH(this)
	if ( QByteArray::resize(nlen+index-olen+1) ) {
	    memset( data()+olen, ' ', index-olen );
	    memcpy( data()+index, s, len+1 );
	}
    } else if ( QByteArray::resize(nlen+1) ) {	// normal insert
	QSTRING_DETACH(this)
	memmove( data()+index+len, data()+index, olen-index+1 );
	memcpy( data()+index, s, len );
    }
    return *this;
}

/*!
  Insert \e c into the string at (before) position \e index and returns
  a reference to the string.

  If \e index is beyond the end of the string, the string is extended with
  spaces (ASCII 32) to length \e index and \e c is then appended.

  Example:
  \code
    QString s = "Yes";
    s.insert( 3, '!');				// s == "Yes!"
  \endcode

  \sa remove(), replace()
*/

QString &QString::insert( uint index, char c )	// insert char
{
    char buf[2];
    buf[0] = c;
    buf[1] = '\0';
    return insert( index, buf );
}

/*!
  \fn QString &QString::prepend( const char *s )

  Prepend \s to the string. Equivalent to insert(0,s).

  \sa insert()
*/

/*!
  Removes \e len characters starting at position \e index from the
  string and returns a reference to the string.

  If \e index is too big, nothing happens.  If \e index is valid, but
  \e len is too large, the rest of the string is removed.

  \code
    QString s = "Montreal";
    s.remove( 1, 4 );
    // s == "Meal"
  \endcode

  \sa insert(), replace()
*/

QString &QString::remove( uint index, uint len )
{
    uint olen = length();
    if ( index + len >= olen ) {		// range problems
	if ( index < olen ) {			// index ok
	    QSTRING_DETACH(this)
	    resize( index+1 );
	}
    } else if ( len != 0 ) {
	QSTRING_DETACH(this)
	memmove( data()+index, data()+index+len, olen-index-len+1 );
	QByteArray::resize(olen-len+1);
    }
    return *this;
}

/*!
  Replaces \e len characters starting at position \e index from the
  string with \e s, and returns a reference to the string.

  If \e index is too big, nothing is deleted and \e s is inserted at the
  end of the string.  If \e index is valid, but \e len is too large, \e
  str replaces the rest of the string.

  \code
    QString s = "Say yes!";
    s.replace( 4, 3, "NO" );			// s == "Say NO!"
  \endcode

  \sa insert(), remove()
*/

QString &QString::replace( uint index, uint len, const char *s )
{
    remove( index, len );
    insert( index, s );
    return *this;
}


/*!
  Returns the string converted to a <code>long</code> value.

  If \e ok is non-null, \e *ok is set to TRUE if there are no
  conceivable errors, and FALSE if the string is not a number at all, or if
  it has trailing garbage.
*/

long QString::toLong( bool *ok ) const
{
    char *p = data();
    long val=0;
    const long max_mult = 214748364;
    bool is_ok = FALSE;
    int neg = 0;
    if ( !p )
	goto bye;
    while ( isspace(*p) )			// skip leading space
	p++;
    if ( *p == '-' ) {
	p++;
	neg = 1;
    } else if ( *p == '+' ) {
	p++;
    }
    if ( !isdigit(*p) )
	goto bye;
    while ( isdigit(*p) ) {
	if ( val > max_mult || (val == max_mult && (*p-'0') > 7+neg) )
	    goto bye;
	val = 10*val + (*p++ - '0');
    }
    if ( neg )
	val = -val;
    while ( isspace(*p) )			// skip trailing space
	p++;
    if ( *p == '\0' )
	is_ok = TRUE;
bye:
    if ( ok )
	*ok = is_ok;
    return is_ok ? val : 0;
}

/*!
  Returns the string converted to an <code>unsigned long</code>
  value.

  If \e ok is non-null, \e *ok is set to TRUE if there are no
  conceivable errors, and FALSE if the string is not a number at all,
  or if it has trailing garbage.
*/

ulong QString::toULong( bool *ok ) const
{
    char *p = data();
    ulong val=0;
    const ulong max_mult = 429496729;
    bool is_ok = FALSE;
    if ( !p )
	goto bye;
    while ( isspace(*p) )			// skip leading space
	p++;
    if ( *p == '+' )
	p++;
    if ( !isdigit(*p) )
	goto bye;
    while ( isdigit(*p) ) {
	if ( val > max_mult || (val == max_mult && (*p-'0') > 5) )
	    goto bye;
	val = 10*val + (*p++ - '0');
    }
    while ( isspace(*p) )			// skip trailing space
	p++;
    if ( *p == '\0' )
	is_ok = TRUE;
bye:
    if ( ok )
	*ok = is_ok;
    return is_ok ? val : 0;
}

/*!
  Returns the string converted to a <code>short</code> value.

  If \e ok is non-null, \e *ok is set to TRUE if there are no
  conceivable errors, and FALSE if the string is not a number at all, or if
  it has trailing garbage.
*/

short QString::toShort( bool *ok ) const
{
    long v = toLong( ok );
    if ( ok && *ok && (v < -32768 || v > 32767) )
	*ok = FALSE;
    return (short)v;
}

/*!
  Returns the string converted to an <code>unsigned short</code> value.

  If \e ok is non-null, \e *ok is set to TRUE if there are no
  conceivable errors, and FALSE if the string is not a number at all, or if
  it has trailing garbage.
*/

ushort QString::toUShort( bool *ok ) const
{
    ulong v = toULong( ok );
    if ( ok && *ok && (v > 65535) )
	*ok = FALSE;
    return (ushort)v;
}


/*!
  Returns the string converted to a <code>int</code> value.

  If \e ok is non-null, \e *ok is set to TRUE if there are no
  conceivable errors, and FALSE if the string is not a number at all,
  or if it has trailing garbage.
*/

int QString::toInt( bool *ok ) const
{
    return (int)toLong( ok );
}

/*!
  Returns the string converted to an <code>unsigned int</code> value.

  If \e ok is non-null, \e *ok is set to TRUE if there are no
  conceivable errors, and FALSE if the string is not a number at all,
  or if it has trailing garbage.
*/

uint QString::toUInt( bool *ok ) const
{
    return (uint)toLong( ok );
}

/*!
  Returns the string converted to a <code>double</code> value.

  If \e ok is non-null, \e *ok is set to TRUE if there are no conceivable
  errors, and FALSE if the string is not a number at all, or if it has
  trailing garbage.
*/

double QString::toDouble( bool *ok ) const
{
    char *end;
    double val = strtod( data() ? data() : "", &end );
    if ( ok )
	*ok = ( data() && *data() && ( end == 0 || *end == '\0' ) );
    return val;
}

/*!
  Returns the string converted to a <code>float</code> value.

  If \e ok is non-null, \e *ok is set to TRUE if there are no
  conceivable errors, and FALSE if the string is not a number at all,
  or if it has trailing garbage.
*/

float QString::toFloat( bool *ok ) const
{
    return (float)toDouble( ok );
}


/*!
  Makes a deep copy of \e str.
  Returns a reference to the string.
*/

QString &QString::setStr( const char *str )
{
    QSTRING_DETACH(this)
    if ( str )					// valid string
	store( str, strlen(str)+1 );
    else					// empty
	resize( 0 );
    return *this;
}

/*!
  Sets the string to the printed value of \e n and returns a
  reference to the string.
*/

QString &QString::setNum( long n )
{
    QSTRING_DETACH(this)
    char buf[20];
    register char *p = &buf[19];
    bool neg;
    if ( n < 0 ) {
	neg = TRUE;
	n = -n;
    } else {
	neg = FALSE;
    }
    *p = '\0';
    do {
	*--p = ((int)(n%10)) + '0';
	n /= 10;
    } while ( n );
    if ( neg )
	*--p = '-';
    store( p, strlen(p)+1 );
    return *this;
}

/*!
  Sets the string to the printed unsigned value of \e n and
  returns a reference to the string.
*/

QString &QString::setNum( ulong n )
{
    QSTRING_DETACH(this)
    char buf[20];
    register char *p = &buf[19];
    *p = '\0';
    do {
	*--p = ((int)(n%10)) + '0';
	n /= 10;
    } while ( n );
    store( p, strlen(p)+1 );
    return *this;
}

/*!
  \fn QString &QString::setNum( int n )
  Sets the string to the printed value of \e n and returns a reference
  to the string.
*/

/*!
  \fn QString &QString::setNum( uint n )
  Sets the string to the printed unsigned value of \e n and returns a
  reference to the string.
*/

/*!
  \fn QString &QString::setNum( short n )
  Sets the string to the printed value of \e n and returns a reference
  to the string.
*/

/*!
  \fn QString &QString::setNum( ushort n )
  Sets the string to the printed unsigned value of \e n and returns a
  reference to the string.
*/

/*!
  Sets the string to the printed value of \e n.

  \arg \e f is the format specifier: 'f', 'F', 'e', 'E', 'g', 'G' (same
  as sprintf()).
  \arg \e prec is the precision.

  Returns a reference to the string.
*/

QString &QString::setNum( double n, char f, int prec )
{
#if defined(CHECK_RANGE)
    if ( !(f=='f' || f=='F' || f=='e' || f=='E' || f=='g' || f=='G') )
	warning( "QString::setNum: Invalid format char '%c'", f );
#endif
    char format[20];
    register char *fs = format;			// generate format string
    *fs++ = '%';				//   "%.<prec>l<f>"
    if ( prec > 99 )
	prec = 99;
    *fs++ = '.';
    if ( prec >= 10 ) {
	*fs++ = prec / 10 + '0';
	*fs++ = prec % 10 + '0';
    } else {
	*fs++ = prec + '0';
    }
    *fs++ = 'l';
    *fs++ = f;
    *fs = '\0';
    return sprintf( format, n );
}

/*!
  \fn QString &QString::setNum( float n, char f, int prec )
  Sets the string to the printed value of \e n.

  \arg \e f is the format specifier: 'f', 'F', 'e', 'E', 'g', 'G' (same
  as sprintf()).
  \arg \e prec is the precision.

  Returns a reference to the string.
*/


/*!
  Sets the character at position \e index to \e c and expands the
  string if necessary, filling with spaces.

  Returns FALSE if this \e index was out of range and the string could
  not be expanded, otherwise TRUE.
*/

bool QString::setExpand( uint index, char c )
{
    QSTRING_DETACH(this)
    uint oldlen = length();
    if ( index >= oldlen ) {
	if ( !QByteArray::resize( index+2 ) )	// no memory
	    return FALSE;
	if ( index > oldlen )
	    memset( data() + oldlen, ' ', index - oldlen );
	*(data() + index+1) = '\0';		// terminate padded string
    }
    *(data() + index) = c;
    return TRUE;
}


/*!
  \fn QString::operator const char *() const
  Returns the string data.
*/

/*!
  \fn bool QString::operator!() const
  Returns TRUE if it is a null string, otherwise FALSE.
*/


/*!
  \fn QString& QString::append( const char *str )
  Appends \e str to the string and returns a reference to the string.
  Equivalent to operator+=().
 */

/*!
  Appends \e str to the string and returns a reference to the string.
*/

QString& QString::operator+=( const char *str )
{
    if ( !str )
	return *this;				// nothing to append
    QSTRING_DETACH(this)
    uint len1 = length();
    uint len2 = strlen(str);
    if ( !QByteArray::resize( len1 + len2 + 1 ) )
	return *this;				// no memory
    memcpy( data() + len1, str, len2 + 1 );
    return *this;
}

/*!
  Appends \e c to the string and returns a reference to the string.
*/

QString &QString::operator+=( char c )
{
    QSTRING_DETACH(this)
    uint len = length();
    if ( !QByteArray::resize( len + 2 ) )
	return *this;				// no memory
    *(data() + len) = c;
    *(data() + len+1) = '\0';
    return *this;
}


/*****************************************************************************
  QString stream functions
 *****************************************************************************/

/*!
  \relates QString
  Writes a string to the stream.

  Output format: [length (Q_UINT32) data...]
*/

QDataStream &operator<<( QDataStream &s, const QString &str )
{
    return s.writeBytes( str.data(), str.size() );
}

/*!
  \relates QString
  Reads a string from the stream.
*/

QDataStream &operator>>( QDataStream &s, QString &str )
{
    QSTRING_DETACH(&str)
    Q_UINT32 len;
    s >> len;					// read size of string
    if ( len == 0 || s.eof() ) {		// end of file reached
	str.resize( 0 );
	return s;
    }
    if ( !str.QByteArray::resize( (uint)len )) {// resize string
#if defined(CHECK_NULL)
	warning( "QDataStream: Not enough memory to read QString" );
#endif
	len = 0;
    }
    if ( len > 0 )				// not null array
	s.readRawBytes( str.data(), (uint)len );
    return s;
}


/*****************************************************************************
  Documentation for related functions
 *****************************************************************************/

/*!
  \fn bool operator==( const QString &s1, const QString &s2 )
  \relates QString
  Returns TRUE if the two strings are equal, or FALSE if they are different.

  Equivalent to <code>strcmp(s1,s2) == 0</code>.
*/

/*!
  \fn bool operator==( const QString &s1, const char *s2 )
  \relates QString
  Returns TRUE if the two strings are equal, or FALSE if they are different.

  Equivalent to <code>strcmp(s1,s2) == 0</code>.
*/

/*!
  \fn bool operator==( const char *s1, const QString &s2 )
  \relates QString
  Returns TRUE if the two strings are equal, or FALSE if they are different.

  Equivalent to <code>strcmp(s1,s2) == 0</code>.
*/

/*!
  \fn bool operator!=( const QString &s1, const QString &s2 )
  \relates QString
  Returns TRUE if the two strings are different, or FALSE if they are equal.

  Equivalent to <code>strcmp(s1,s2) != 0</code>.
*/

/*!
  \fn bool operator!=( const QString &s1, const char *s2 )
  \relates QString
  Returns TRUE if the two strings are different, or FALSE if they are equal.

  Equivalent to <code>strcmp(s1,s2) != 0</code>.
*/

/*!
  \fn bool operator!=( const char *s1, const QString &s2 )
  \relates QString
  Returns TRUE if the two strings are different, or FALSE if they are equal.

  Equivalent to <code>strcmp(s1,s2) != 0</code>.
*/

/*!
  \fn bool operator<( const QString &s1, const char *s2 )
  \relates QString
  Returns TRUE if \e s1 is alphabetically less than \e s2, otherwise FALSE.

  Equivalent to <code>strcmp(s1,s2) \< 0</code>.
*/

/*!
  \fn bool operator<( const char *s1, const QString &s2 )
  \relates QString
  Returns TRUE if \e s1 is alphabetically less than \e s2, otherwise FALSE.

  Equivalent to <code>strcmp(s1,s2) \< 0</code>.
*/

/*!
  \fn bool operator<=( const QString &s1, const char *s2 )
  \relates QString
  Returns TRUE if \e s1 is alphabetically less than or equal to \e s2,
  otherwise FALSE.

  Equivalent to <code>strcmp(s1,s2) \<= 0</code>.
*/

/*!
  \fn bool operator<=( const char *s1, const QString &s2 )
  \relates QString
  Returns TRUE if \e s1 is alphabetically less than or equal to \e s2,
  otherwise FALSE.

  Equivalent to <code>strcmp(s1,s2) \<= 0</code>.
*/

/*!
  \fn bool operator>( const QString &s1, const char *s2 )
  \relates QString
  Returns TRUE if \e s1 is alphabetically greater than \e s2, otherwise FALSE.

  Equivalent to <code>strcmp(s1,s2) \> 0</code>.
*/

/*!
  \fn bool operator>( const char *s1, const QString &s2 )
  \relates QString
  Returns TRUE if \e s1 is alphabetically greater than \e s2, otherwise FALSE.

  Equivalent to <code>strcmp(s1,s2) \> 0</code>.
*/

/*!
  \fn bool operator>=( const QString &s1, const char *s2 )
  \relates QString
  Returns TRUE if \e s1 is alphabetically greater than or equal to \e s2,
  otherwise FALSE.

  Equivalent to <code>strcmp(s1,s2) \>= 0</code>.
*/

/*!
  \fn bool operator>=( const char *s1, const QString &s2 )
  \relates QString
  Returns TRUE if \e s1 is alphabetically greater than or equal to \e s2,
  otherwise FALSE.

  Equivalent to <code>strcmp(s1,s2) \>= 0</code>.
*/

/*!
  \fn QString operator+( const QString &s1, const QString &s2 )
  \relates QString
  Returns the concatenated string of s1 and s2.
*/

/*!
  \fn QString operator+( const QString &s1, const char *s2 )
  \relates QString
  Returns the concatenated string of s1 and s2.
*/

/*!
  \fn QString operator+( const char *s1, const QString &s2 )
  \relates QString
  Returns the concatenated string of s1 and s2.
*/

/*!
  \fn QString operator+( const QString &s, char c )
  \relates QString
  Returns the concatenated string of s and c.
*/

/*!
  \fn QString operator+( char c, const QString &s )
  \relates QString
  Returns the concatenated string of c and s.
*/
