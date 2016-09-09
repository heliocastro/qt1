/****************************************************************************
** $Id: qglobal.cpp,v 2.17.2.1 1998/08/21 12:42:23 hanord Exp $
**
** Global functions
**
** Created : 920604
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

#include "qglobal.h"
#include "qdict.h"
#include "qstring.h"
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

/*!
  \relates QApplication
  Returns the Qt version number for the library, typically "1.30"
  or "1.31".
*/

Q_EXPORT
const char *qVersion()
{
    return QT_VERSION_STR;
}


/*****************************************************************************
  System detection routines
 *****************************************************************************/

static bool si_alreadyDone = FALSE;
static int  si_wordSize;
static bool si_bigEndian;

/*!
  \relates QApplication
  Obtains information about the system.

  The system's word size in bits (typically 32) is returned in \e *wordSize.
  The \e *bigEndian is set to TRUE if this is a big-endian machine,
  or to FALSE if this is a little-endian machine.

  This function calls fatal() with a message if the computer is truely weird
  (i.e. different endianness for 16 bit and 32 bit integers).
*/

Q_EXPORT
bool qSysInfo( int *wordSize, bool *bigEndian )
{
#if defined(CHECK_NULL)
    ASSERT( wordSize != 0 );
    ASSERT( bigEndian != 0 );
#endif

    if ( si_alreadyDone ) {			// run it only once
	*wordSize  = si_wordSize;
	*bigEndian = si_bigEndian;
	return TRUE;
    }
    si_alreadyDone = TRUE;

    si_wordSize = 0;
    uint n = (uint)(~0);
    while ( n ) {				// detect word size
	si_wordSize++;
	n /= 2;
    }
    *wordSize = si_wordSize;

    if ( *wordSize != 64 &&
	 *wordSize != 32 &&
	 *wordSize != 16 ) {			// word size: 16, 32 or 64
#if defined(CHECK_RANGE)
	fatal( "qSysInfo: Unsupported system word size %d", *wordSize );
#endif
	return FALSE;
    }
    if ( sizeof(Q_INT8) != 1 || sizeof(Q_INT16) != 2 || sizeof(Q_INT32) != 4 ||
	 sizeof(float) != 4 || sizeof(double) != 8 ) {
#if defined(CHECK_RANGE)
	fatal( "qSysInfo: Unsupported system data type size" );
#endif
	return FALSE;
    }

    bool  be16, be32;				// determine byte ordering
    short ns = 0x1234;
    int	  nl = 0x12345678;

    unsigned char *p = (unsigned char *)(&ns);	// 16-bit integer
    be16 = *p == 0x12;

    p = (unsigned char *)(&nl);			// 32-bit integer
    if ( p[0] == 0x12 && p[1] == 0x34 && p[2] == 0x56 && p[3] == 0x78 )
	be32 = TRUE;
    else
    if ( p[0] == 0x78 && p[1] == 0x56 && p[2] == 0x34 && p[3] == 0x12 )
	be32 = FALSE;
    else
	be32 = !be16;

    if ( be16 != be32 ) {			// strange machine!
#if defined(CHECK_RANGE)
	fatal( "qSysInfo: Inconsistent system byte order" );
#endif
	return FALSE;
    }

    *bigEndian = si_bigEndian = be32;
    return TRUE;
}


/*****************************************************************************
  Debug output routines
 *****************************************************************************/

static msg_handler handler = 0;			// pointer to debug handler

/*!
  \relates QApplication
  Prints a debug message, or calls the message handler (if it has been
  installed).

  This function takes a format string and a stack arguments, similar to
  the C printf() function.

  Example:
  \code
    debug( "my window handle = %x", myWidget->id() );
  \endcode

  Under X11, the text is printed to stderr.  Under Windows, the text is
  sent to the debugger.

  \warning The internal buffer is limited to 512 bytes (including the
  0-terminator.

  \sa warning(), fatal(), qInstallMsgHandler(),
  \link debug.html Debugging\endlink
*/

Q_EXPORT
void debug( const char *msg, ... )
{
    char buf[512];
    va_list ap;
    va_start( ap, msg );			// use variable arg list
    if ( handler ) {
	vsprintf( buf, msg, ap );
	va_end( ap );
	(*handler)( QtDebugMsg, buf );
    } else {
	vfprintf( stderr, msg, ap );
	va_end( ap );
	fprintf( stderr, "\n" );		// add newline
    }
}

/*!
  \relates QApplication
  Prints a warning message, or calls the message handler (if it has been
  installed).

  This function takes a format string and a stack arguments, similar to
  the C printf() function.

  Example:
  \code
    void f( int c )
    {
	if ( c > 200 )
	    warning( "f: bad argument, c == %d", c );
    }
  \endcode

  Under X11, the text is printed to stderr.  Under Windows, the text is
  sent to the debugger.

  \warning The internal buffer is limited to 512 bytes (including the
  0-terminator.

  \sa debug(), fatal(), qInstallMsgHandler(),
  \link debug.html Debugging\endlink
*/

Q_EXPORT
void warning( const char *msg, ... )
{
    char buf[512];
    va_list ap;
    va_start( ap, msg );			// use variable arg list
    if ( handler ) {
	vsprintf( buf, msg, ap );
	va_end( ap );
	(*handler)( QtWarningMsg, buf );
    } else {
	vfprintf( stderr, msg, ap );
	va_end( ap );
	fprintf( stderr, "\n" );		// add newline
    }
}


/*!
  \relates QApplication
  Prints a fatal error message and exits, or calls the message handler (if it
  has been installed).

  This function takes a format string and a stack arguments, similar to
  the C printf() function.

  Example:
  \code
    int divide( int a, int b )
    {
	if ( b == 0 )				// program error
	    fatal( "divide: cannot divide by zero" );
	return a/b;
    }
  \endcode

  Under X11, the text is printed to stderr.  Under Windows, the text is
  sent to the debugger.

  \warning The internal buffer is limited to 512 bytes (including the
  0-terminator.

  \sa debug(), warning(), qInstallMsgHandler(),
  \link debug.html Debugging\endlink
*/

Q_EXPORT
void fatal( const char *msg, ... )
{
    char buf[512];
    va_list ap;
    va_start( ap, msg );			// use variable arg list
    if ( handler ) {
	vsprintf( buf, msg, ap );
	va_end( ap );
	(*handler)( QtFatalMsg, buf );
    } else {
	vfprintf( stderr, msg, ap );
	va_end( ap );
	fprintf( stderr, "\n" );		// add newline
#if defined(UNIX) && defined(DEBUG)
	abort();				// trap; generates core dump
#else
	exit( 1 );				// goodbye cruel world
#endif
    }
}

/*!
  \fn void ASSERT( bool test )
  \relates QApplication
  Prints a warning message containing the source code file name and line number
  if \e test is FALSE.

  This is really a macro defined in qglobal.h.

  ASSERT is useful for testing required conditions in your program.

  Example:
  \code
    //
    // File: div.cpp
    //

    #include <qglobal.h>

    int divide( int a, int b )
    {
	ASSERT( b != 0 );			// this is line 9
	return a/b;
    }
  \endcode

  If \c b is zero, the ASSERT statement will output the following message
  using the warning() function:
  \code
    ASSERT: "b == 0" in div.cpp (9)
  \endcode

  \sa warning(), \link debug.html Debugging\endlink
*/


/*!
  \fn void CHECK_PTR( void *p )
  \relates QApplication
  If \e p is null, a fatal messages says that the program ran out of memory
  and exits.  If \e p is not null, nothing happens.

  This is really a macro defined in qglobal.h.

  \warning CHECK_PTR only works for the development release of the Qt
  library.  In the release library, CHECK_PTR will be substituted with
  nothing.

  Example:
  \code
    int *a;
    CHECK_PTR( a = new int[80] );	// never do this!
      // do this instead:
    a = new int[80];
    CHECK_PTR( a );			// this is fine
  \endcode

  \sa fatal(), \link debug.html Debugging\endlink
*/


//
// The CHECK_PTR macro calls this function to check if an allocation went ok.
//

Q_EXPORT
bool chk_pointer( bool c, const char *n, int l )
{
    if ( c )
	fatal( "In file %s, line %d: Out of memory", n, l );
    return TRUE;
}


Q_DECLARE(QDictM,int);

static bool firstObsoleteWarning(const char *obj, const char *oldfunc )
{
    static bool firstWarning = TRUE;
    static QDictM(int) * obsoleteDict;
    if ( firstWarning ) {
	firstWarning = FALSE;
	debug(
      "You are using obsolete functions in the Qt library. Call the function\n"
      "qSuppressObsoleteWarnings() to suppress obsolete warnings.\n"
	     );
    }
    QString s( obj );
    s += "::";
    s += oldfunc;
    if ( !obsoleteDict )
	obsoleteDict = new QDictM(int);
    if ( obsoleteDict->find(s) == 0 ) {
	obsoleteDict->insert( s, (int*) 666 );	// anything different from 0.
	return TRUE;
    }
    return FALSE;
}

static bool suppressObsolete = FALSE;

Q_EXPORT
void qSuppressObsoleteWarnings( bool suppress )
{
    suppressObsolete = suppress;
}

Q_EXPORT
void qObsolete(	 const char *obj, const char *oldfunc, const char *newfunc )
{
    if ( suppressObsolete )
	return;
    if ( !firstObsoleteWarning(obj, oldfunc) )
	return;
    debug( "%s::%s: This function is obsolete, use %s instead",
	   obj, oldfunc, newfunc );
}

Q_EXPORT
void qObsolete(	 const char *obj, const char *oldfunc )
{
    if ( suppressObsolete )
	return;
    if ( !firstObsoleteWarning(obj, oldfunc) )
	return;
    debug( "%s::%s: This function is obsolete.", obj, oldfunc );
}

Q_EXPORT
void qObsolete(	 const char *message )
{
    if ( suppressObsolete )
	return;
    if ( !firstObsoleteWarning( "Qt", message) )
	return;
    debug( "%s", message );
}


/*!
  \relates QApplication
  Installs a Qt message handler.  Returns a pointer to the message handler
  previously defined.

  The message handler is a function that prints out debug messages,
  warnings and fatal error messages.  The Qt library (debug version)
  contains hundreds of warning messages that are printed when internal
  errors (usually invalid function arguments) occur.  If you implement
  your own message handler, you get total control of these messages.

  The default message handler prints the message to the standard output
  under X11 or to the debugger under Windows.  If it is a fatal message,
  the application aborts immediately.

  Only one message handler can be defined, since this is usually done on
  an application-wide basis to control debug output.

  To restore the message handler, call \c qInstallMsgHandler(0).

  Example:
  \code
    #include <qapplication.h>
    #include <stdio.h>
    #include <stdlib.h>

    void myMessageOutput( QtMsgType type, const char *msg )
    {
	switch ( type ) {
	    case QtDebugMsg:
		fprintf( stderr, "Debug: %s\n", msg );
		break;
	    case QtWarningMsg:
		fprintf( stderr, "Warning: %s\n", msg );
		break;
	    case QtFatalMsg:
		fprintf( stderr, "Fatal: %s\n", msg );
		abort();			// dump core on purpose
	}
    }

    int main( int argc, char **argv )
    {
	qInstallMsgHandler( myMessageOutput );
	QApplication a( argc, argv );
	...
	return a.exec();
    }
  \endcode

  \sa debug(), warning(), fatal(), \link debug.html Debugging\endlink
*/

Q_EXPORT
msg_handler qInstallMsgHandler( msg_handler h )
{
    msg_handler old = handler;
    handler = h;
    return old;
}
