/****************************************************************************
** $Id: qiodevice.cpp,v 2.12.2.1 1998/09/28 11:51:00 warwick Exp $
**
** Implementation of QIODevice class
**
** Created : 940913
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

#include "qiodevice.h"

/*!
  \class QIODevice qiodevice.h

  \brief The QIODevice class is the base class of I/O devices.

  \ingroup io

  An I/O device represents a medium that one can read bytes from and write
  bytes to.  The QIODevice class itself is not capable of reading or
  writing any data; it has virtual functions for doing so. These functions
  are implemented by the subclasses QFile and QBuffer.

  There are two types of I/O devices;
  \link isDirectAccess() <em>direct access</em>\endlink and
  \link isSequentialAccess() <em>sequential access </em>\endlink devices.
  Files can normally be accessed
  directly, except \c stdin etc., which must be processed sequentially.
  Buffers are always direct access devices.

  The access mode of an I/O device can be either \e raw or \e buffered.
  QFile objects can be created using one of these.  Raw access mode is more
  low level, while buffered access use smart buffering techniques.
  The raw access mode is best when I/O is block-operated using 4kB block size
  or greater.  Buffered access works better when reading small portions of
  data at a time.

  An I/O device operation can be executed in either \e synchronous or
  \e asynchronous mode.	 The I/O devices currently supported by Qt only
  execute synchronously.

  The QDataStream and QTextStream provide binary and text operations
  on QIODevice objects.

  QIODevice provides numerous pure virtual functions you need to implement
  when subclassing it. Here is a skeleton subclass:

  \code
    class YourDevice : public QIODevice
    {
    public:
	YourDevice();
       ~YourDevice();
  
	bool open( int mode );
	void close();
	void flush();
  
	uint size() const;
	int  at() const;	// not a pure virtual function
	bool at( int );		// not a pure virtual function
	bool atEnd() const;	// not a pure virtual function
  
	int readBlock( char *data, uint len );
	int writeBlock( const char *data, uint len );
	int readLine( char *data, uint maxlen );
  
	int getch();
	int putch( int );
	int ungetch( int );
    };
  \endcode

  The three non-pure virtual functions can be ignored if your device
  is sequential (e.g. a tape device).

  \sa QDataStream, QTextStream
*/


/*!
  Constructs an I/O device.
*/

QIODevice::QIODevice()
{
    ioMode = 0;					// initial mode
    ioSt = IO_Ok;
    index = 0;
}

/*!
  Destroys the I/O device.
*/

QIODevice::~QIODevice()
{
}


/*!
  \fn int QIODevice::flags() const
  Returns the current I/O device flags setting.

  Flags consists of mode flags and state flags.

  \sa mode(), state()
*/

/*!
  \fn int QIODevice::mode() const
  Returns bits OR'ed together that specify the current operation mode.

  These are the flags that were given to the open() function.

  The flags are: \c IO_ReadOnly, \c IO_WriteOnly, \c IO_ReadWrite,
  \c IO_Append, \c IO_Truncate and \c IO_Translate.
*/

/*!
  \fn int QIODevice::state() const
  Returns bits OR'ed together that specify the current state.

  The flags are: \c IO_Open.

  Subclasses may define more flags.
*/

/*!
  \fn bool QIODevice::isDirectAccess() const
  Returns TRUE if the I/O device is a direct access (not sequential) device,
  otherwise FALSE.
  \sa isSequentialAccess()
*/

/*!
  \fn bool QIODevice::isSequentialAccess() const
  Returns TRUE if the I/O device is a sequential access (not direct) device,
  otherwise FALSE.  Operations involving size() and at(int) are not valid
  on sequential devices.
  \sa isDirectAccess()
*/

/*!
  \fn bool QIODevice::isCombinedAccess() const
  Returns TRUE if the I/O device is a combined access (both direct and
  sequential) device,  otherwise FALSE.

  This access method is currently not in use.
*/

/*!
  \fn bool QIODevice::isBuffered() const
  Returns TRUE if the I/O device is a buffered (not raw) device, otherwise
  FALSE.
  \sa isRaw()
*/

/*!
  \fn bool QIODevice::isRaw() const
  Returns TRUE if the I/O device is a raw (not buffered) device, otherwise
  FALSE.
  \sa isBuffered()
*/

/*!
  \fn bool QIODevice::isSynchronous() const
  Returns TRUE if the I/O device is a synchronous device, otherwise
  FALSE.
  \sa isAsynchronous()
*/

/*!
  \fn bool QIODevice::isAsynchronous() const
  Returns TRUE if the I/O device is a asynchronous device, otherwise
  FALSE.

  This mode is currently not in use.

  \sa isSynchronous()
*/

/*!
  \fn bool QIODevice::isTranslated() const
  Returns TRUE if the I/O device translates carriage-return and linefeed
  characters.

  A QFile is translated if it is opened with the \c IO_Translate mode
  flag.
*/

/*!
  \fn bool QIODevice::isReadable() const
  Returns TRUE if the I/O device was opened using \c IO_ReadOnly or
  \c IO_ReadWrite mode.
  \sa isWritable(), isReadWrite()
*/

/*!
  \fn bool QIODevice::isWritable() const
  Returns TRUE if the I/O device was opened using \c IO_WriteOnly or
  \c IO_ReadWrite mode.
  \sa isReadable(), isReadWrite()
*/

/*!
  \fn bool QIODevice::isReadWrite() const
  Returns TRUE if the I/O device was opened using \c IO_ReadWrite mode.
  \sa isReadable(), isWritable()
*/

/*!
  \fn bool QIODevice::isInactive() const
  Returns TRUE if the I/O device state is 0, i.e. the device is not open.
  \sa isOpen()
*/

/*!
  \fn bool QIODevice::isOpen() const
  Returns TRUE if the I/O device state has been opened, otherwise FALSE.
  \sa isInactive()
*/


/*!
  \fn int QIODevice::status() const
  Returns the I/O device status.

  The I/O device status returns an error code.	If open() returns FALSE
  or readBlock() or writeBlock() return -1, this function can be called to
  get the reason why the operation did not succeed.

  The status codes are:
  <ul>
  <li>\c IO_Ok The operation was successful.
  <li>\c IO_ReadError Could not read from the device.
  <li>\c IO_WriteError Could not write to the device.
  <li>\c IO_FatalError A fatal unrecoverable error occurred.
  <li>\c IO_OpenError Could not open the device.
  <li>\c IO_ConnectError Could not connect to the device.
  <li>\c IO_AbortError The operation was unexpectedly aborted.
  <li>\c IO_TimeOutError The operation timed out.
  </ul>

  \sa resetStatus()
*/

/*!
  \fn void QIODevice::resetStatus()

  Sets the I/O device status to \c IO_Ok.

  \sa status()
*/


/*!
  \fn void QIODevice::setFlags( int f )
  \internal
  Used by subclasses to set the device flags.
*/

/*!
  \internal
  Used by subclasses to set the device type.
*/

void QIODevice::setType( int t )
{
#if defined(CHECK_RANGE)
    if ( (t & IO_TypeMask) != t )
	warning( "QIODevice::setType: Specified type out of range" );
#endif
    ioMode &= ~IO_TypeMask;			// reset type bits
    ioMode |= t;
}

/*!
  \internal
  Used by subclasses to set the device mode.
*/

void QIODevice::setMode( int m )
{
#if defined(CHECK_RANGE)
    if ( (m & IO_ModeMask) != m )
	warning( "QIODevice::setMode: Specified mode out of range" );
#endif
    ioMode &= ~IO_ModeMask;			// reset mode bits
    ioMode |= m;
}

/*!
  \internal
  Used by subclasses to set the device state.
*/

void QIODevice::setState( int s )
{
#if defined(CHECK_RANGE)
    if ( ((uint)s & IO_StateMask) != (uint)s )
	warning( "QIODevice::setState: Specified state out of range" );
#endif
    ioMode &= ~IO_StateMask;			// reset state bits
    ioMode |= (uint)s;
}

/*!
  \internal
  Used by subclasses to set the device status (not state).
*/

void QIODevice::setStatus( int s )
{
    ioSt = s;
}


/*!
  \fn bool QIODevice::open( int mode )
  Opens the I/O device using the specified \e mode.
  Returns TRUE if successful, or FALSE if the device could not be opened.

  The mode parameter \e m must be a combination of the following flags.
  <ul>
  <li>\c IO_Raw specified raw (unbuffered) file access.
  <li>\c IO_ReadOnly opens a file in read-only mode.
  <li>\c IO_WriteOnly opens a file in write-only mode.
  <li>\c IO_ReadWrite opens a file in read/write mode.
  <li>\c IO_Append sets the file index to the end of the file.
  <li>\c IO_Truncate truncates the file.
  <li>\c IO_Translate enables carriage returns and linefeed translation
  for text files under MS-DOS, Window, OS/2 and Macintosh.  Cannot be
  combined with \c IO_Raw.
  </ul>

  This virtual function must be reimplemented by all subclasses.

  \sa close()
*/

/*!
  \fn void QIODevice::close()
  Closes the I/O device.

  This virtual function must be reimplemented by all subclasses.

  \sa open()
*/

/*!
  \fn void QIODevice::flush()

  Flushes an open I/O device.

  This virtual function must be reimplemented by all subclasses.
*/


/*!
  \fn uint QIODevice::size() const
  Virtual function that returns the size of the I/O device.
  \sa at()
*/

/*!
  Virtual function that returns the current I/O device index.

  This index is the data read/write head of the I/O device.

  \sa size()
*/

int QIODevice::at() const
{
    return index;
}

/*!
  Virtual function that sets the I/O device index to \e pos.
  \sa size()
*/

bool QIODevice::at( int pos )
{
#if defined(CHECK_RANGE)
    if ( (uint)pos > size() ) {
	warning( "QIODevice::at: Index %d out of range", pos );
	return FALSE;
    }
#endif
    index = pos;
    return TRUE;
}

/*!
  Virtual function that returns TRUE if the I/O device index is at the
  end of the input.
*/

bool QIODevice::atEnd() const
{
    if ( isSequentialAccess() ) {
	QIODevice* that = (QIODevice*)this;
	int c = that->getch();
	bool result = c < 0;
	that->ungetch(c);
	return result;
    } else {
        return at() == (int)size();
    }
}

/*!
  \fn bool QIODevice::reset()
  Sets the device index to 0.
  \sa at()
*/


/*!
  \fn int QIODevice::readBlock( char *data, uint len )
  Reads at most \e len bytes from the I/O device into \e data and
  returns the number of bytes actually read.

  This virtual function must be reimplemented by all subclasses.

  \sa writeBlock()
*/

/*!
  \fn int QIODevice::writeBlock( const char *data, uint len )
  Writes \e len bytes from \e p to the I/O device and returns the number of
  bytes actually written.

  This virtual function must be reimplemented by all subclasses.

  \sa readBlock()
*/

/*!
  Reads a line of text, up to \e maxlen bytes including a terminating
  \0.  If there is a newline at the end if the line, it is not stripped.

  Returns the number of bytes read, or -1 in case of error.

  This virtual function can be reimplemented much more efficiently by
  the most subclasses.

  \sa readBlock(), QTextStream::readLine()
*/

int QIODevice::readLine( char *data, uint maxlen )
{
    if ( maxlen == 0 )				// application bug?
	return 0;
    int pos = at();				// get current position
    int s  = (int)size();			// size of I/O device
    char *p = data;
    if ( pos >= s )
	return 0;
    while ( pos++ < s && --maxlen ) {		// read one byte at a time
	readBlock( p, 1 );
	if ( *p++ == '\n' )			// end of line
	    break;
    }
    *p++ = '\0';
    return (int)((long)p - (long)data);
}


/*!
  \fn int QIODevice::getch()

  Reads a single byte/character from the I/O device.

  Returns the byte/character read, or -1 if the end of the I/O device has been
  reached.

  This virtual function must be reimplemented by all subclasses.

  \sa putch(), ungetch()
*/

/*!
  \fn int QIODevice::putch( int ch )

  Writes the character \e ch to the I/O device.

  Returns \e ch, or -1 if some error occurred.

  This virtual function must be reimplemented by all subclasses.

  \sa getch(), ungetch()
*/

/*!
  \fn int QIODevice::ungetch( int ch )

  Puts the character \e ch back into the I/O device and decrements the
  index if it is not zero.

  This function is normally called to "undo" a getch() operation.

  Returns \e ch, or -1 if some error occurred.

  This virtual function must be reimplemented by all subclasses.

  \sa getch(), putch()
*/
