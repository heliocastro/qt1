/****************************************************************************
** $Id: qfile.cpp,v 2.23.2.1 1998/08/20 17:41:49 warwick Exp $
**
** Implementation of QFile class
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

#include "qfile.h"
#include "qfiledefs.h"

/*!
  \class QFile qfile.h
  \brief The QFile class is an I/O device that operates on files.

  \ingroup io

  QFile is an I/O device for reading and writing binary and text files.	A
  QFile may be used by itself (readBlock and writeBlock) or by more
  conveniently using QDataStream or QTextStream.

  Here is a code fragment that uses QTextStream to read a text
  file line by line. It prints each line with a line number.
  \code
    QFile f("file.txt");
    if ( f.open(IO_ReadOnly) ) {    // file opened successfully
	QTextStream t( &f );	    // use a text stream
	QString s;
	int n = 1;
	while ( !t.eof() ) {	    // until end of file...
	    s = t.readLine();	    // line of text excluding '\n'
	    printf( "%3d: %s\n", n++, (const char *)s );
	}
	f.close();
    }
  \endcode

  The QFileInfo class holds detailed information about a file, such as
  access permissions, file dates and file types.

  The QDir class manages directories and lists of file names.

  \sa QDataStream, QTextStream
*/


/*!
  Constructs a QFile with no name.
*/

QFile::QFile()
{
    init();
}

/*!
  Constructs a QFile with a file name \e name.
  \sa setName()
*/

QFile::QFile( const char *name )
    : fn(name)
{
    init();
}


/*!
  Destroys a QFile.  Calls close().
*/

QFile::~QFile()
{
    close();
}


/*!
  \internal
  Initialize internal data.
*/

void QFile::init()
{
    setFlags( IO_Direct );
    setStatus( IO_Ok );
    fh	   = 0;
    fd	   = 0;
    length = 0;
    index  = 0;
    ext_f  = FALSE;				// not an external file handle
}


/*!
  \fn const char *QFile::name() const
  Returns the name set by setName().
  \sa setName(), QFileInfo::fileName()
*/

/*!
  Sets the name of the file. The name can include an absolute directory
  path or it can be a name or a path relative to the current directory.

  Do not call this function if the file has already been opened.

  Note that if the name is relative QFile does not associate it with the
  current directory.  If you change directory before calling open(), open
  uses the new current directory.

  Example:
  \code
     QFile f;
     QDir::setCurrent( "/tmp" );
     f.setName( "readme.txt" );
     QDir::setCurrent( "/home" );
     f.open( IO_ReadOnly );	   // opens "/home/readme.txt" under UNIX
  \endcode

  Also note that the directory separator '/' works for all operating
  systems supported by Qt.

  \sa name(), QFileInfo, QDir
*/

void QFile::setName( const char *name )
{
    if ( isOpen() ) {
#if defined(CHECK_STATE)
	warning( "QFile::setName: File is open" );
#endif
	close();
    }
    fn = name;
}


/*!
  Returns TRUE if the file exists, otherwise FALSE.
  \sa name()
*/

bool QFile::exists() const
{
    if ( fn.isEmpty() )
	return FALSE;
    return ACCESS(fn.data(), F_OK) == 0;
}

/*!
  Returns TRUE if the file given by \e fileName exists, otherwise FALSE.
*/

bool QFile::exists( const char *fileName )
{
#if defined(CHECK_NULL)
    ASSERT( fileName != 0 );
#endif
    return ACCESS( fileName, F_OK ) == 0;
}


/*!
  Removes the file specified by the file name currently set.
  Returns TRUE if successful, otherwise FALSE.

  The file is closed before it is removed.
*/

bool QFile::remove()
{
    close();
    return remove( fn.data()  );
}

/*!
  Removes the file \a fileName.
  Returns TRUE if successful, otherwise FALSE.
*/

bool QFile::remove( const char *fileName )      // remove file
{
    if ( fileName == 0 || fileName[0] == '\0' ) {
#if defined(CHECK_NULL)
        warning( "QFile::remove: Empty or NULL file name" );
#endif
        return FALSE;
    }
#if defined(UNIX)
    return unlink( fileName ) == 0;		// unlink more common in UNIX
#else
    return ::remove( fileName ) == 0;		// use standard ANSI remove
#endif
}


#if defined(_OS_MAC_) || defined(_OS_MSDOS_) || defined(_OS_WIN32_) || defined(_OS_OS2_)
# define HAS_TEXT_FILEMODE			// has translate/text filemode
#endif
#if defined(O_NONBLOCK)
# define HAS_ASYNC_FILEMODE
# define OPEN_ASYNC O_NONBLOCK
#elif defined(O_NDELAY)
# define HAS_ASYNC_FILEMODE
# define OPEN_ASYNC O_NDELAY
#endif

/*!
  Opens the file specified by the file name currently set, using the mode \e m.
  Returns TRUE if successful, otherwise FALSE.

  The mode parameter \e m must be a combination of the following flags:
  <ul>
  <li>\c IO_Raw specified raw (non-buffered) file access.
  <li>\c IO_ReadOnly opens the file in read-only mode.
  <li>\c IO_WriteOnly opens the file in write-only mode (and truncates).
  <li>\c IO_ReadWrite opens the file in read/write mode, equivalent to
  \c (IO_ReadOnly|IO_WriteOnly).
  <li>\c IO_Append opens the file in append mode. This mode is very useful
  when you want to write something to a log file. The file index is set to
  the end of the file. Note that the result is undefined if you position the
  file index manually using at() in append mode.
  <li>\c IO_Truncate truncates the file.
  <li>\c IO_Translate enables carriage returns and linefeed translation
  for text files under MS-DOS, Windows and OS/2.
  </ul>

  The raw access mode is best when I/O is block-operated using 4kB block size
  or greater. Buffered access works better when reading small portions of
  data at a time.

  <strong>Important:</strong> When working with buffered files, data may
  not be written to the file at once. Call \link flush() flush\endlink
  to make sure the data is really written.

  \warning We have experienced problems with some C libraries when a buffered
  file is opened for both reading and writing. If a read operation takes place
  immediately after a write operation, the read buffer contains garbage data.
  Worse, the same garbage is written to the file. Calling flush() before
  readBlock() solved this problem.

  If the file does not exist and \c IO_WriteOnly or \c IO_ReadWrite is
  specified, it is created.

  Example:
  \code
    QFile f1( "/tmp/data.bin" );
    QFile f2( "readme.txt" );
    f1.open( IO_Raw | IO_ReadWrite | IO_Append );
    f2.open( IO_ReadOnly | IO_Translate );
  \endcode

  \sa name(), close(), isOpen(), flush()
*/

bool QFile::open( int m )
{
    if ( isOpen() ) {				// file already open
#if defined(CHECK_STATE)
	warning( "QFile::open: File already open" );
#endif
	return FALSE;
    }
    if ( fn.isNull() ) {			// no file name defined
#if defined(CHECK_NULL)
	warning( "QFile::open: No file name specified" );
#endif
	return FALSE;
    }
    init();					// reset params
    setMode( m );
    if ( !(isReadable() || isWritable()) ) {
#if defined(CHECK_RANGE)
	warning( "QFile::open: File access not specified" );
#endif
	return FALSE;
    }
    bool ok = TRUE;
    if ( isRaw() ) {				// raw file I/O
	int oflags = OPEN_RDONLY;
	if ( isReadable() && isWritable() )
	    oflags = OPEN_RDWR;
	else if ( isWritable() )
	    oflags = OPEN_WRONLY;
	if ( flags() & IO_Append ) {		// append to end of file?
	    if ( flags() & IO_Truncate )
		oflags |= (OPEN_CREAT | OPEN_TRUNC);
	    else
		oflags |= (OPEN_APPEND | OPEN_CREAT);
	    setFlags( flags() | IO_WriteOnly ); // append implies write
	} else if ( isWritable() ) {		// create/trunc if writable
	    if ( flags() & IO_Truncate )
		oflags |= (OPEN_CREAT | OPEN_TRUNC);
	    else
		oflags |= OPEN_CREAT;
	}
#if defined(HAS_TEXT_FILEMODE) && !defined(_OS_MAC_)
	if ( isTranslated() )
	    oflags |= OPEN_TEXT;
	else
	    oflags |= OPEN_BINARY;
#endif
#if defined(HAS_ASYNC_FILEMODE)
	if ( isAsynchronous() )
	    oflags |= OPEN_ASYNC;
#endif
	fd = OPEN( (const char *)fn, oflags, 0666 );
	if ( fd != -1 ) {			// open successful
	    STATBUF st;
	    FSTAT( fd, &st );
	    length = (int)st.st_size;
	    index  = (flags() & IO_Append) == 0 ? 0 : length;
	} else {
	    ok = FALSE;
	}
    } else {					// buffered file I/O
	const char *perm = 0;
	char perm2[4];
	bool try_create = FALSE;
	if ( flags() & IO_Append ) {		// append to end of file?
	    setFlags( flags() | IO_WriteOnly ); // append implies write
	    perm = isReadable() ? "a+" : "a";
	} else {
	    if ( isReadWrite() ) {
		if ( flags() & IO_Truncate ) {
		    perm = "w+";
		} else {
		    perm = "r+";
		    try_create = TRUE;		// try to create if not exists
		}
	    } else if ( isReadable() ) {
		perm = "r";
	    } else if ( isWritable() ) {
		perm = "w";
	    }
	}
	strcpy( perm2, perm );
#if defined(HAS_TEXT_FILEMODE)
	if ( isTranslated() )
	    strcat( perm2, "t" );
	else
	    strcat( perm2, "b" );
#endif
	fh = fopen( (const char *)fn, perm2 );
	if ( !fh && try_create ) {
	    perm2[0] = 'w';			// try "w+" instead of "r+"
	    fh = fopen( (const char *)fn, perm2 );
	}
	if ( fh ) {
	    STATBUF st;
	    FSTAT( FILENO(fh), &st );
	    length = (int)st.st_size;
	    index  = (flags() & IO_Append) == 0 ? 0 : length;
	} else {
	    ok = FALSE;
	}
    }
    if ( ok ) {
	setState( IO_Open );
    } else {
	init();
	if ( errno == EMFILE )			// no more file handles/descrs
	    setStatus( IO_ResourceError );
	else
	    setStatus( IO_OpenError );
    }
    return ok;
}


/*!
  Opens a file in the mode \e m using an existing file handle \e f.
  Returns TRUE if successful, otherwise FALSE.

  Example:
  \code
    #include <stdio.h>

    void printError( const char *msg )
    {
	QFile f;
	f.open( IO_WriteOnly, stderr );
	f.writeBlock( msg, strlen(msg) );	// write to stderr
	f.close();
    }
  \endcode

  When a QFile is opened using this function, close() does not actually
  close the file, only flushes it.

  \warning If \e f is \c stdin, \c stdout, \c stderr, you may not
  be able to seek.  See QIODevice::isSequentialAccess() for more
  information.

  \sa close()
*/

bool QFile::open( int m, FILE *f )
{
    if ( isOpen() ) {
#if defined(CHECK_RANGE)
	warning( "QFile::open: File already open" );
#endif
	return FALSE;
    }
    init();
    setMode( m &~IO_Raw );
    setState( IO_Open );
    fh = f;
    ext_f = TRUE;
    STATBUF st;
    FSTAT( FILENO(fh), &st );
    index = (int)ftell( fh );
    if ( (st.st_mode & STAT_MASK) != STAT_REG ) {
	// non-seekable
	setType( IO_Sequential );
	length = INT_MAX;
    } else {
	length = (int)st.st_size;
    }
    return TRUE;
}


/*!
  Opens a file in the mode \e m using an existing file descriptor \e f.
  Returns TRUE if successful, otherwise FALSE.

  When a QFile is opened using this function, close() does not actually
  close the file.

  \warning If \e f is one of 0 (stdin), 1 (stdout) or 2 (stderr), you may not
  be able to seek. size() is set to \c INT_MAX (in limits.h).

  \sa close()
*/

bool QFile::open( int m, int f )
{
    if ( isOpen() ) {
#if defined(CHECK_RANGE)
	warning( "QFile::open: File already open" );
#endif
	return FALSE;
    }
    init();
    setMode( m |IO_Raw );
    setState( IO_Open );
    fd = f;
    ext_f = TRUE;
    if ( fd == 0 || fd == 1 || fd == 2 ) {
	length = INT_MAX;
    } else {
	STATBUF st;
	FSTAT( fd, &st );
	length = (int)st.st_size;
	index  = (int)LSEEK(fd, 0, SEEK_CUR);
    }
    return TRUE;
}


/*!
  Closes an open file.

  The file is closed even if it was opened with an existing file
  handle or a file descriptor, \e except that stdin, stdout and stderr
  are never closed.

  \sa open(), flush()
*/

void QFile::close()
{
    if ( !isOpen() )				// file is not open
	return;
    if ( fh ) {					// buffered file
	if ( ext_f )
	    fflush( fh );			// cannot close
	else
	    fclose( fh );
    } else {					// raw file
	if ( ext_f )
	    ;					// cannot close
	else
	    CLOSE( fd );
    }
    init();					// restore internal state
}


/*!
  Flushes the file buffer to the disk.

  close() also flushes the file buffer.
*/

void QFile::flush()
{
    if ( isOpen() && fh )			// can only flush open/buffered
	fflush( fh );				//   file
}


/*!
  Returns the file size.
  \sa at()
*/

uint QFile::size() const
{
    STATBUF st;
    if ( isOpen() )
	FSTAT( fh ? FILENO(fh) : fd, &st );
    else
	STAT( fn, &st );
    return st.st_size;
}


/*!
  \fn int QFile::at() const
  Returns the file index.
  \sa size()
*/

/*!
  Sets the file index to \e pos. Returns TRUE if successful, otherwise FALSE.

  Example:
  \code
    QFile f( "data.bin" );
    f.open( IO_ReadOnly );			// index set to 0
    f.at( 100 );				// set index to 100
    f.at( f.at()+50 );				// set index to 150
    f.at( f.size()-80 );			// set index to 80 before EOF
    f.close();
  \endcode

  \warning The result is undefined if the file was \link open() opened\endlink
  using the \c IO_Append specifier.

  \sa size(), open()
*/

bool QFile::at( int pos )
{
    if ( !isOpen() ) {
#if defined(CHECK_STATE)
	warning( "QFile::at: File is not open" );
#endif
	return FALSE;
    }
    bool ok;
    if ( isRaw() ) {				// raw file
	pos = (int)LSEEK(fd, pos, SEEK_SET);
	ok = pos != -1;
    } else {					// buffered file
	ok = fseek(fh, pos, SEEK_SET) == 0;
    }
    if ( ok )
	index = pos;
#if defined(CHECK_RANGE)
    else
	warning( "QFile::at: Cannot set file position %d", pos );
#endif
    return ok;
}


/*!
  Returns TRUE if the end of file has been reached, otherwise FALSE.
  \sa size()
*/

bool QFile::atEnd() const
{
    if ( !isOpen() ) {
#if defined(CHECK_STATE)
	warning( "QFile::atEnd: File is not open" );
#endif
	return FALSE;
    }
    return QIODevice::atEnd();
}


/*!
  Reads at most \e len bytes from the file into \e p and returns the
  number of bytes actually read.

  Returns -1 if a serious error occurred.

  \warning We have experienced problems with some C libraries when a buffered
  file is opened for both reading and writing. If a read operation takes place
  immediately after a write operation, the read buffer contains garbage data.
  Worse, the same garbage is written to the file. Calling flush() before
  readBlock() solved this problem.

  \sa writeBlock()
*/

int QFile::readBlock( char *p, uint len )
{
#if defined(CHECK_NULL)
    if ( !p )
	warning( "QFile::readBlock: Null pointer error" );
#endif
#if defined(CHECK_STATE)
    if ( !isOpen() ) {				// file not open
	warning( "QFile::readBlock: File not open" );
	return -1;
    }
    if ( !isReadable() ) {			// reading not permitted
	warning( "QFile::readBlock: Read operation not permitted" );
	return -1;
    }
#endif
    int nread;					// number of bytes read
    if ( isRaw() ) {				// raw file
	nread = READ( fd, p, len );
	if ( len && nread <= 0 ) {
	    nread = 0;
	    setStatus(IO_ReadError);
	}
    } else {					// buffered file
	nread = fread( p, 1, len, fh );
	if ( (uint)nread != len ) {
	    if ( ferror( fh ) || nread==0 )
		setStatus(IO_ReadError);
	}
    }
    index += nread;
    return nread;
}


/*!
  Writes \e len bytes from \e p to the file and returns the number of
  bytes actually written.

  Returns -1 if a serious error occurred.

  <strong>Important:</strong> When working with buffered files, data may
  not be written to the file at once. Call \link flush() flush\endlink
  to make sure the data is really written.

  \sa readBlock()
*/

int QFile::writeBlock( const char *p, uint len )
{
#if defined(CHECK_NULL)
    if ( p == 0 && len != 0 )
	warning( "QFile::writeBlock: Null pointer error" );
#endif
#if defined(CHECK_STATE)
    if ( !isOpen() ) {				// file not open
	warning( "QFile::writeBlock: File not open" );
	return -1;
    }
    if ( !isWritable() ) {			// writing not permitted
	warning( "QFile::writeBlock: Write operation not permitted" );
	return -1;
    }
#endif
    int nwritten;				// number of bytes written
    if ( isRaw() )				// raw file
	nwritten = WRITE( fd, p, len );
    else					// buffered file
	nwritten = fwrite( p, 1, len, fh );
    if ( nwritten != (int)len ) {		// write error
	if ( errno == ENOSPC )			// disk is full
	    setStatus( IO_ResourceError );
	else
	    setStatus( IO_WriteError );
	if ( isRaw() )				// recalc file position
	    index = (int)LSEEK( fd, 0, SEEK_CUR );
	else
	    index = fseek( fh, 0, SEEK_CUR );
    } else {
	index += nwritten;
    }
    if ( index > length )			// update file length
	length = index;
    return nwritten;
}


/*!
  Reads a line of text.

  Reads bytes from the file until end-of-line is reached, or up to \a
  maxlen bytes, and returns the number of bytes read, or -1 in case of
  error.  The terminating newline is not stripped.

  This function is efficient only for buffered files.  Avoid
  readLine() for files that have been opened with the \c IO_Raw
  flag.

  \sa readBlock(), QTextStream::readLine()
*/

int QFile::readLine( char *p, uint maxlen )
{
    if ( maxlen == 0 )				// application bug?
	return 0;
#if defined(CHECK_STATE)
    CHECK_PTR( p );
    if ( !isOpen() ) {				// file not open
	warning( "QFile::readLine: File not open" );
	return -1;
    }
    if ( !isReadable() ) {			// reading not permitted
	warning( "QFile::readLine: Read operation not permitted" );
	return -1;
    }
#endif
    int nread;					// number of bytes read
    if ( isRaw() ) {				// raw file
	nread = QIODevice::readLine( p, maxlen );
    } else {					// buffered file
	p = fgets( p, maxlen, fh );
	if ( p ) {
	    nread = strlen( p );
	    index += nread;
	} else {
	    nread = -1;
	    setStatus(IO_ReadError);
	}
    }
    return nread;
}


/*!
  Reads a single byte/character from the file.

  Returns the byte/character read, or -1 if the end of the file has been
  reached.

  \sa putch(), ungetch()
*/

int QFile::getch()
{
#if defined(CHECK_STATE)
    if ( !isOpen() ) {				// file not open
	warning( "QFile::getch: File not open" );
	return EOF;
    }
    if ( !isReadable() ) {			// reading not permitted
	warning( "QFile::getch: Read operation not permitted" );
	return EOF;
    }
#endif
    int ch;
    if ( isRaw() ) {				// raw file (inefficient)
	char buf[1];
	ch = readBlock( buf, 1 ) == 1 ? buf[0] : EOF;
    } else {					// buffered file
	if ( (ch = getc( fh )) != EOF )
	    index++;
	else
	    setStatus(IO_ReadError);
    }
    return ch;
}

/*!
  Writes the character \e ch to the file.

  Returns \e ch, or -1 if some error occurred.

  \sa getch(), ungetch()
*/

int QFile::putch( int ch )
{
#if defined(CHECK_STATE)
    if ( !isOpen() ) {				// file not open
	warning( "QFile::putch: File not open" );
	return EOF;
    }
    if ( !isWritable() ) {			// writing not permitted
	warning( "QFile::putch: Write operation not permitted" );
	return EOF;
    }
#endif
    if ( isRaw() ) {				// raw file (inefficient)
	char buf[1];
	buf[0] = ch;
	ch = writeBlock( buf, 1 ) == 1 ? ch : EOF;
    } else {					// buffered file
	if ( (ch = putc( ch, fh )) != EOF ) {
	    index++;
	    if ( index > length )		// update file length
		length = index;
	} else {
	    setStatus(IO_WriteError);
	}
    }
    return ch;
}

/*!
  Puts the character \e ch back into the file and decrements the index if it
  is not zero.

  This function is normally called to "undo" a getch() operation.

  Returns \e ch, or -1 if some error occurred.

  \sa getch(), putch()
*/

int QFile::ungetch( int ch )
{
#if defined(CHECK_STATE)
    if ( !isOpen() ) {				// file not open
	warning( "QFile::ungetch: File not open" );
	return EOF;
    }
    if ( !isReadable() ) {			// reading not permitted
	warning( "QFile::ungetch: Read operation not permitted" );
	return EOF;
    }
#endif
    if ( ch == EOF )				// cannot unget EOF
	return ch;
    if ( isRaw() ) {				// raw file (very inefficient)
	char buf[1];
	at( index-1 );
	buf[0] = ch;
	if ( writeBlock(buf, 1) == 1 )
	    at ( index-1 );
	else
	    ch = EOF;
    } else {					// buffered file
	if ( (ch = ungetc(ch, fh)) != EOF )
	    index--;
	else
	    setStatus( IO_ReadError );
    }
    return ch;
}


/*!
  Returns the file handle of the file.

  This is a small positive integer, suitable for use with C library
  functions such as fdopen() and fcntl(), as well as with QSocketNotifier.

  If the file is not open or there is an error, handle() returns -1.

  \sa QSocketNotifier
*/

int QFile::handle() const
{
    if ( !isOpen() )
	return -1;
    else if ( fh )
	return fileno( fh );
    else
	return fd;
}
