/****************************************************************************
** $Id: qtextstream.h,v 2.8.2.3 1998/08/25 09:20:55 hanord Exp $
**
** Definition of QTextStream class
**
** Created : 940922
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

#ifndef QTEXTSTREAM_H
#define QTEXTSTREAM_H

#ifndef QT_H
#include "qiodevice.h"
#include "qstring.h"
#include <stdio.h>
#endif // QT_H


class Q_EXPORT QTextStream				// text stream class
{
public:
    QTextStream();
    QTextStream( QIODevice * );
    QTextStream( QByteArray, int mode );
    QTextStream( FILE *, int mode );
    virtual ~QTextStream();

    QIODevice	*device() const;
    void	 setDevice( QIODevice * );
    void	 unsetDevice();

    bool	 eof() const;

    QTextStream &operator>>( char & );
    QTextStream &operator>>( signed short & );
    QTextStream &operator>>( unsigned short & );
    QTextStream &operator>>( signed int & );
    QTextStream &operator>>( unsigned int & );
    QTextStream &operator>>( signed long & );
    QTextStream &operator>>( unsigned long & );
    QTextStream &operator>>( float & );
    QTextStream &operator>>( double & );
    QTextStream &operator>>( char * );
    QTextStream &operator>>( QString & );

    QTextStream &operator<<( char );
    QTextStream &operator<<( signed short );
    QTextStream &operator<<( unsigned short );
    QTextStream &operator<<( signed int );
    QTextStream &operator<<( unsigned int );
    QTextStream &operator<<( signed long );
    QTextStream &operator<<( unsigned long );
    QTextStream &operator<<( float );
    QTextStream &operator<<( double );
    QTextStream &operator<<( const char * );
    QTextStream &operator<<( void * );		// any pointer

    QTextStream &readRawBytes( char *, uint len );
    QTextStream &writeRawBytes( const char *, uint len );

    QString	readLine();

    enum {
	skipws	  = 0x0001,			// skip whitespace on input
	left	  = 0x0002,			// left-adjust output
	right	  = 0x0004,			// right-adjust output
	internal  = 0x0008,			// pad after sign
	bin	  = 0x0010,			// binary format integer
	oct	  = 0x0020,			// octal format integer
	dec	  = 0x0040,			// decimal format integer
	hex	  = 0x0080,			// hex format integer
	showbase  = 0x0100,			// show base indicator
	showpoint = 0x0200,			// force decimal point (float)
	uppercase = 0x0400,			// upper-case hex output
	showpos	  = 0x0800,			// add '+' to positive integers
	scientific= 0x1000,			// scientific float output
	fixed	  = 0x2000			// fixed float output
    };

    static const int basefield;			// bin | oct | dec | hex
    static const int adjustfield;		// left | right | internal
    static const int floatfield;		// scientific | fixed

    int	  flags() const;
    int	  flags( int f );
    int	  setf( int bits );
    int	  setf( int bits, int mask );
    int	  unsetf( int bits );

    void  reset();

    int	  width()	const;
    int	  width( int );
    int	  fill()	const;
    int	  fill( int );
    int	  precision()	const;
    int	  precision( int );

private:
    long	 input_int();
    QTextStream &output_int( int, ulong, bool );
    QIODevice	*dev;
    int		 fflags;
    int		 fwidth;
    int		 fillchar;
    int		 fprec;
    bool	 fstrm;
    bool	 owndev;

private:	// Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QTextStream( const QTextStream & );
    QTextStream &operator=( const QTextStream & );
#endif
};

typedef QTextStream QTS;


/*****************************************************************************
  QTextStream inline functions
 *****************************************************************************/

inline QIODevice *QTextStream::device() const
{ return dev; }

inline bool QTextStream::eof() const
{ return dev ? dev->atEnd() : FALSE; }

inline int QTextStream::flags() const
{ return fflags; }

inline int QTextStream::flags( int f )
{ int oldf = fflags;  fflags = f;  return oldf; }

inline int QTextStream::setf( int bits )
{ int oldf = fflags;  fflags |= bits;  return oldf; }

inline int QTextStream::setf( int bits, int mask )
{ int oldf = fflags;  fflags = (fflags & ~mask) | (bits & mask); return oldf; }

inline int QTextStream::unsetf( int bits )
{ int oldf = fflags;  fflags &= ~bits;	return oldf; }

inline int QTextStream::width() const
{ return fwidth; }

inline int QTextStream::width( int w )
{ int oldw = fwidth;  fwidth = w;  return oldw;	 }

inline int QTextStream::fill() const
{ return fillchar; }

inline int QTextStream::fill( int f )
{ int oldc = fillchar;	fillchar = f;  return oldc;  }

inline int QTextStream::precision() const
{ return fprec; }

inline int QTextStream::precision( int p )
{ int oldp = fprec;  fprec = p;	 return oldp;  }


/*****************************************************************************
  QTextStream manipulators
 *****************************************************************************/

typedef QTextStream & (*QTSFUNC)(QTextStream &);// manipulator function
typedef int (QTextStream::*QTSMFI)(int);	// manipulator w/int argument

class Q_EXPORT QTSManip {			// text stream manipulator
public:
    QTSManip( QTSMFI m, int a ) { mf=m; arg=a; }
    void exec( QTextStream &s ) { (s.*mf)(arg); }
private:
    QTSMFI mf;					// QTextStream member function
    int	   arg;					// member function argument
};

Q_EXPORT inline QTextStream &operator>>( QTextStream &s, QTSFUNC f )
{ return (*f)( s ); }

Q_EXPORT inline QTextStream &operator<<( QTextStream &s, QTSFUNC f )
{ return (*f)( s ); }

Q_EXPORT inline QTextStream &operator<<( QTextStream &s, QTSManip m )
{ m.exec(s); return s; }

Q_EXPORT QTextStream &bin( QTextStream &s );	// set bin notation
Q_EXPORT QTextStream &oct( QTextStream &s );	// set oct notation
Q_EXPORT QTextStream &dec( QTextStream &s );	// set dec notation
Q_EXPORT QTextStream &hex( QTextStream &s );	// set hex notation
Q_EXPORT QTextStream &endl( QTextStream &s );	// insert EOL ('\n')
Q_EXPORT QTextStream &flush( QTextStream &s );	// flush output
Q_EXPORT QTextStream &ws( QTextStream &s );	// eat whitespace on input
Q_EXPORT QTextStream &reset( QTextStream &s );	// set default flags

Q_EXPORT inline QTSManip setw( int w )
{
	QTSMFI func = &QTextStream::width;
	return QTSManip(func,w);
}

Q_EXPORT inline QTSManip setfill( int f )
{
	QTSMFI func = &QTextStream::fill;
	return QTSManip(func,f);
}

Q_EXPORT inline QTSManip setprecision( int p )
{
	QTSMFI func = &QTextStream::precision;
	return QTSManip(func,p);
}


#endif // QTEXTSTREAM_H
