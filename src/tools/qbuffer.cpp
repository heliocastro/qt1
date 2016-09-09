/****************************************************************************
** $Id: qbuffer.cpp,v 2.16 1998/07/03 00:09:43 hanord Exp $
**
** Implementation of QBuffer class
**
** Created : 930812
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

#include "qbuffer.h"
#include <stdlib.h>

/*!
  \class QBuffer qbuffer.h
  \brief The QBuffer class is an I/O device that operates on a QByteArray

  \ingroup io

  QBuffer is an I/O device for reading and writing a memory buffer. A
  QBuffer may be used directly (readBlock() and writeBlock()) or more
  conveniently via QDataStream or QTextStream.  Most of its behavior
  is inherited from QIODevice.

  A QBuffer has an associated QByteArray which holds the buffer data.
  Writing data at the end (i.e. size()) of the buffer expands the byte
  array.

  For convenience, the byte stream classes QDataStream and QTextStream
  can operate on a QByteArray (or a QString) via an internal QBuffer:
  \code
    QString str;
    QTextStream ts( str, IO_WriteOnly );
    ts << "pi = " << 3.14;			// str == "pi = 3.14"
  \endcode

  \sa QFile, QDataStream, QTextStream
*/


/*!
  Constructs an empty buffer.
*/

QBuffer::QBuffer()
{
    setFlags( IO_Direct );
    a_inc = 16;					// initial increment
    a_len = 0;
    index = 0;
}


/*!
  Constructs a buffer and sets the buffer contents to \e buf.
  \sa setBuffer()
*/

QBuffer::QBuffer( QByteArray buf ) : a(buf)
{
    setFlags( IO_Direct );
    a_len = a.size();
    a_inc = (a_len > 512) ? 512 : a_len;	// initial increment
    if ( a_inc < 16 )
	a_inc = 16;
    index = 0;
}

/*!
  Destroys the buffer.
*/

QBuffer::~QBuffer()
{
}


/*!
  Replaces the buffer's contents with \e buf.

  This may not be done while the buffer is \link open() open\endlink.

  Note that if you open the buffer in write mode (\c IO_WriteOnly or
  IO_ReadWrite) and write something into the buffer, \e buf is also
  modified because QByteArray is an explicitly shared class.

  Example:
  \code
    QString str = "abc";
    QBuffer b( str );
    b.open( IO_WriteOnly );
    b.at( 3 );					// position at \0
    b.writeBlock( "def", 4 );			// write including \0
    b.close();
      // Now, str == "abcdef"
  \endcode

  \sa open, \link shclass.html Shared Classes\endlink
*/

bool QBuffer::setBuffer( QByteArray buf )
{
    if ( isOpen() ) {
#if defined(CHECK_STATE)
	warning( "QBuffer::setBuffer: Buffer is open");
#endif
	return FALSE;
    }
    a = buf;
    a_len = a.size();
    a_inc = (a_len > 512) ? 512 : a_len;	// initial increment
    if ( a_inc < 16 )
	a_inc = 16;
    index = 0;
    return TRUE;
}

/*!
  \fn QByteArray QBuffer::buffer() const

  Returns the buffer most recently set by setBuffer(), or at construction.
*/

/*!
  Opens the file specified by the file name currently set, using the mode \e m.
  Returns TRUE if successful, otherwise FALSE.

  The mode parameter \e m must be a combination of the following flags.
  <ul>
  <li>\c IO_ReadOnly opens a buffer in read-only mode.
  <li>\c IO_WriteOnly opens a buffer in write-only mode.
  <li>\c IO_ReadWrite opens a buffer in read/write mode.
  <li>\c IO_Append sets the buffer index to the end of the buffer.
  <li>\c IO_Truncate truncates the buffer.
  </ul>

  \sa close(), isOpen()
*/

bool QBuffer::open( int m  )
{
    if ( isOpen() ) {				// buffer already open
#if defined(CHECK_STATE)
	warning( "QBuffer::open: Buffer already open" );
#endif
	return FALSE;
    }
    setMode( m );
    if ( m & IO_Truncate ) {			// truncate buffer
	a.resize( 0 );
	a_len = 0;
    }
    if ( m & IO_Append ) {			// append to end of buffer
	index = a.size();
    } else {
	index = 0;
    }
    a_inc = 16;
    setState( IO_Open );
    setStatus( 0 );
    return TRUE;
}

/*!
  Closes an open buffer.
  \sa open()
*/

void QBuffer::close()
{
    if ( isOpen() ) {
	setFlags( IO_Direct );
	index = 0;
	a_inc = 16;
    }
}

/*!
  The flush function does nothing.
*/

void QBuffer::flush()
{
    return;
}


/*!
  \fn int QBuffer::at() const
  Returns the buffer index.
  \sa size()
*/

/*!
  \fn uint QBuffer::size() const
  Returns the number of bytes in the buffer.
  \sa at()
*/

/*!
  Sets the buffer index to \e pos. Returns TRUE if successful, otherwise FALSE.
  \sa size()
*/

bool QBuffer::at( int pos )
{
#if defined(CHECK_STATE)
    if ( !isOpen() ) {
	warning( "QBuffer::at: Buffer is not open" );
	return FALSE;
    }
#endif
    if ( (uint)pos > a_len ) {
#if defined(CHECK_RANGE)
	warning( "QBuffer::at: Index %d out of range", pos );
#endif
	return FALSE;
    }
    index = pos;
    return TRUE;
}


/*!
  Reads at most \e len bytes from the buffer into \e p and returns the
  number of bytes actually read.

  Returns -1 if a serious error occurred.

  \sa writeBlock()
*/

int QBuffer::readBlock( char *p, uint len )
{
#if defined(CHECK_STATE)
    CHECK_PTR( p );
    if ( !isOpen() ) {				// buffer not open
	warning( "QBuffer::readBlock: Buffer not open" );
	return -1;
    }
    if ( !isReadable() ) {			// reading not permitted
	warning( "QBuffer::readBlock: Read operation not permitted" );
	return -1;
    }
#endif
    if ( (uint)index + len > a.size() ) {	// overflow
	if ( (uint)index >= a.size() ) {
	    setStatus( IO_ReadError );
	    return -1;
	} else {
	    len = a.size() - (uint)index;
	}
    }
    memcpy( p, a.data()+index, len );
    index += len;
    return len;
}

/*!
  Writes \e len bytes from \e p into the buffer at the current index,
  overwriting any characters there and extending the buffer if necessary.
  Returns the number of bytes actually written.

  Returns -1 if a serious error occurred.

  \sa readBlock()
*/

int QBuffer::writeBlock( const char *p, uint len )
{
#if defined(CHECK_NULL)
    if ( p == 0 && len != 0 )
	warning( "QBuffer::writeBlock: Null pointer error" );
#endif
#if defined(CHECK_STATE)
    if ( !isOpen() ) {				// buffer not open
	warning( "QBuffer::writeBlock: Buffer not open" );
	return -1;
    }
    if ( !isWritable() ) {			// writing not permitted
	warning( "QBuffer::writeBlock: Write operation not permitted" );
	return -1;
    }
#endif
    if ( (uint)index + len >= a_len ) {		// overflow
	uint new_len = a_len + a_inc*(((uint)index+len-a_len)/a_inc+1);
	if ( !a.resize( new_len ) ) {		// could not resize
#if defined(CHECK_NULL)
	    warning( "QBuffer::writeBlock: Memory allocation error" );
#endif
	    setStatus( IO_ResourceError );
	    return -1;
	}
	a_inc *= 2;				// double increment
	a_len = new_len;
	a.shd->len = (uint)index + len;
    }
    memcpy( a.data()+index, p, len );
    index += len;
    if ( a.shd->len < (uint)index )
	a.shd->len = (uint)index;		// fake (not alloc'd) length
    return len;
}


/*!
  Reads a line of text.

  Reads bytes from the buffer until end-of-line is reached, or up to
  \e maxlen bytes.

  \sa readBlock()
*/

int QBuffer::readLine( char *p, uint maxlen )
{
#if defined(CHECK_STATE)
    CHECK_PTR( p );
    if ( !isOpen() ) {				// buffer not open
	warning( "QBuffer::readLine: Buffer not open" );
	return -1;
    }
    if ( !isReadable() ) {			// reading not permitted
	warning( "QBuffer::readLine: Read operation not permitted" );
	return -1;
    }
#endif
    if ( maxlen == 0 )
	return 0;
    uint start = (uint)index;
    char *d = a.data() + index;
    maxlen--;					// make room for 0-terminator
    if ( a.size() - (uint)index < maxlen )
	maxlen = a.size() - (uint)index;
    while ( maxlen-- ) {
	if ( (*p++ = *d++) == '\n' )
	    break;
    }
    *p = '\0';
    index = d - a.data();
    return (uint)index - start;
}


/*!
  Reads a single byte/character from the buffer.

  Returns the byte/character read, or -1 if the end of the buffer has been
  reached.

  \sa putch(), ungetch()
*/

int QBuffer::getch()
{
#if defined(CHECK_STATE)
    if ( !isOpen() ) {				// buffer not open
	warning( "QBuffer::getch: Buffer not open" );
	return -1;
    }
    if ( !isReadable() ) {			// reading not permitted
	warning( "QBuffer::getch: Read operation not permitted" );
	return -1;
    }
#endif
    if ( (uint)index+1 > a.size() ) {		// overflow
	setStatus( IO_ReadError );
	return -1;
    }
    return *(a.data()+index++);
}

/*!
  Writes the character \e ch into the buffer, overwriting
  the character at the current index, extending the buffer
  if necessary.

  Returns \e ch, or -1 if some error occurred.

  \sa getch(), ungetch()
*/

int QBuffer::putch( int ch )
{
#if defined(CHECK_STATE)
    if ( !isOpen() ) {				// buffer not open
	warning( "QBuffer::putch: Buffer not open" );
	return -1;
    }
    if ( !isWritable() ) {			// writing not permitted
	warning( "QBuffer::putch: Write operation not permitted" );
	return -1;
    }
#endif
    if ( (uint)index + 1 >= a_len ) {		// overflow
	char buf[1];
	buf[0] = (char)ch;
	if ( writeBlock(buf,1) != 1 )
	    return -1;				// write error
    } else {
	*(a.data() + index++) = (char)ch;
	if ( a.shd->len < (uint)index )
	    a.shd->len = (uint)index;
    }
    return ch;
}

/*!
  Puts the character \e ch back into the buffer and decrements the index if
  it is not zero.

  This function is normally called to "undo" a getch() operation.

  Returns \e ch, or -1 if some error occurred.

  \sa getch(), putch()
*/

int QBuffer::ungetch( int ch )
{
#if defined(CHECK_STATE)
    if ( !isOpen() ) {				// buffer not open
	warning( "QBuffer::ungetch: Buffer not open" );
	return -1;
    }
    if ( !isReadable() ) {			// reading not permitted
	warning( "QBuffer::ungetch: Read operation not permitted" );
	return -1;
    }
#endif
    if ( ch != -1 ) {
	if ( index )
	    index--;
	else
	    ch = -1;
    }
    return ch;
}
