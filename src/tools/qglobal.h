/****************************************************************************
** $Id: qglobal.h,v 2.56.2.17 1999/02/12 17:52:57 hanord Exp $
**
** Global type declarations and definitions
**
** Created : 920529
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

#ifndef QGLOBAL_H
#define QGLOBAL_H


#define QT_VERSION	145
#define QT_VERSION_STR	"1.45"


//
// The operating system, must be one of: (_OS_x_)
//
//   MAC	- Macintosh
//   MSDOS	- MS-DOS and Windows
//   OS2	- OS/2
//   OS2EMX	- XFree86 on OS/2 (not PM)
//   WIN32	- Win32 (Windows 95/98 and Windows NT)
//   SUN	- SunOS
//   SOLARIS	- Sun Solaris
//   HPUX	- HP-UX
//   ULTRIX	- DEC Ultrix
//   LINUX	- Linux
//   FREEBSD	- FreeBSD
//   NETBSD	- NetBSD
//   OPENBSD    - OpenBSD
//   IRIX	- SGI Irix
//   OSF	- OSF Unix
//   BSDI	- BSDI Unix
//   SCO	- SCO of some sort
//   AIX	- AIX Unix
//   UNIXWARE	- SCO UnixWare
//   GNU	- GNU Hurd
//   DGUX	- DG Unix
//   UNIX	- Any UNIX bsd/sysv system
//

#if defined(macintosh)
#define _OS_MAC_
#elif defined(MSDOS) || defined(_MSDOS) || defined(__MSDOS__)
#define _OS_MSDOS_
#elif defined(OS2) || defined(_OS2) || defined(__OS2__)
#if defined(__EMX__)
#define _OS_OS2EMX_
#else
#define _OS_OS2_
#endif
#elif defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
#define _OS_WIN32_
#elif defined(__MWERKS__) && defined(__INTEL__)
#define _OS_WIN32_
#elif defined(sun) || defined(__sun) || defined(__sun__)
#define _OS_SUN_
#if defined(_OS_SUN_) && defined(__SVR4)
#define _OS_SOLARIS_
#undef _OS_SUN_
#endif
#elif defined(hpux) || defined(__hpux) || defined(__hpux__)
#define _OS_HPUX_
#elif defined(ultrix) || defined(__ultrix) || defined(__ultrix__)
#define _OS_ULTRIX_
#elif defined(linux) || defined(__linux) || defined(__linux__)
#define _OS_LINUX_
#elif defined(__FreeBSD__)
#define _OS_FREEBSD_
#elif defined(__NetBSD__)
#define _OS_NETBSD_
#elif defined(__OpenBSD__)
#define _OS_OPENBSD_
#elif defined(sgi) || defined(__sgi)
#define _OS_IRIX_
#elif defined(__osf__)
#define _OS_OSF_
#elif defined(bsdi) || defined(__bsdi__)
#define _OS_BSDI_
#elif defined(_SCO_DS) || defined(M_UNIX) || defined(M_XENIX)
#define _OS_SCO_
#elif defined(_AIX)
#define _OS_AIX_
#elif defined(__Lynx__)
#define _OS_LYNXOS_
#elif defined(_UNIXWARE)
#define _OS_UNIXWARE_
#elif defined(__GNU__)
#define _OS_GNU_
#elif defined(DGUX)
#define _OS_DGUX_
#elif defined(sco)
#define _OS_UNIXWARE7_
#elif defined(__QNX__)
#define _OS_QNX_
#else
#error "Qt has not been ported to this OS - talk to qt-bugs@troll.no"
#endif

#if !defined(UNIX)
#define UNIX
#endif
#if defined(_OS_MAC_) || defined(_OS_MSDOS_) || defined(_OS_OS2_) || defined(_OS_WIN32_)
#undef	UNIX
#endif


//
// The compiler, must be one of: (_CC_x_)
//
//   SYM	- Symantec C++ for both PC and Macintosh
//   MPW	- MPW C++
//   MWERKS	- Metroworks CodeWarrior
//   MSVC	- Microsoft Visual C/C++
//   BOR	- Borland/Turbo C++
//   WAT	- Watcom C++
//   GNU	- GNU C++
//   COMEAU	- Comeau C++
//   EDG	- Edison Design Group C++
//   OC		- CenterLine ObjectCenter C++
//   SUN	- Sun C++
//   DEC	- DEC C++
//   HP		- HPUX C++
//   USLC	- SCO UnixWare C++
//

#if defined(__SC__)
#define _CC_SYM_
#elif defined(applec)
#define _CC_MPW_
#elif defined(__MWERKS__)
#define _CC_MWERKS_
#define HAS_BOOL_TYPE
#elif defined(_MSC_VER)
#define _CC_MSVC_
#elif defined(__BORLANDC__) || defined(__TURBOC__)
#define _CC_BOR_
#elif defined(__WATCOMC__)
#define _CC_WAT_
#elif defined(__GNUC__)
#define _CC_GNU_
#elif defined(__xlC__)
#define _CC_XLC_
#elif defined(como40)
#define _CC_EDG_
#define _CC_COMEAU_
#elif defined(__EDG) || defined(__EDG__)
// one observed on SGI DCC, the other documented
#define _CC_EDG_
#elif defined(OBJECTCENTER) || defined(CENTERLINE_CLPP)
#define _CC_OC_
#elif defined(__SUNPRO_CC)
#define _CC_SUN_
#elif defined(__DECCXX)
#define _CC_DEC_
#elif defined(__USLC__)
#define _CC_USLC_
#elif defined(_OS_HPUX_)
// this test from from aCC online help
#if __cplusplus >= 199707L
// this is the aCC
#define _CC_HP_ACC_
#define HAS_BOOL_TYPE
#else
// this is the CC
#define _CC_HP_
#endif // __cplusplus >= 199707L
#else
#error "Qt has not been tested with this compiler - talk to qt-bugs@troll.no"
#endif

#if defined(_CC_COMEAU_)
#define Q_C_CALLBACKS
#endif


//
// Specify to use macro or template classes (if not set from cmd line)
//

#define USE_MACROCLASS				/* always use macro classes */
#define USE_TEMPLATECLASS			/* use template classes */


//
// Some compilers do not support templates
//

#if defined(_CC_MPW_) || (defined(_CC_MSVC_) && _MSC_VER < 900)
#define NO_TEMPLATECLASS
#endif

#if defined(NO_TEMPLATECLASS)
#undef	USE_TEMPLATECLASS			/* templates not wanted */
#endif


//
// Smart setting/checking of DEFAULT_ flag
//

#if !defined(DEFAULT_MACROCLASS) && !defined(DEFAULT_TEMPLATECLASS)
#if defined(USE_TEMPLATECLASS)
#define DEFAULT_TEMPLATECLASS
#else
#define DEFAULT_MACROCLASS
#endif
#endif

#if !defined(USE_TEMPLATECLASS) && defined(DEFAULT_TEMPLATECLASS)
#error "Can't use templates as default when USE_TEMPLATECLASS is not defined!"
#endif

#if defined(DEFAULT_MACROCLASS) && defined(DEFAULT_TEMPLATECLASS)
#error "Define DEFAULT_MACROCLASS or DEFAULT_TEMPLATECLASS, not both!"
#endif


//
// Some classes do not permit copies to be made of an object.
// These classes contains a private copy constructor and operator=
// to disable copying (the compiler gives an error message).
// Undefine Q_DISABLE_COPY to turn of this checking.
//

#define Q_DISABLE_COPY

//
// Create Qt DLL if QT_DLL is defined (Windows only)
//

#if defined(_OS_WIN32_)
#if defined(QT_NODLL)
#undef QT_MAKEDLL
#undef QT_DLL
#endif
#if defined(QT_MAKEDLL)		/* create a Qt DLL library */
#undef QT_DLL
#define Q_EXPORT  __declspec(dllexport)
#define Q_EXPORTM __declspec(dllexport)
#define Q_TEMPLATEDLL
#undef  Q_DISABLE_COPY		/* avoid unresolved externals */
#endif
#if defined(QT_DLL)		/* use a Qt DLL library */
#define Q_EXPORT  __declspec(dllimport)
#define Q_EXPORTM
#define Q_TEMPLATEDLL
#undef  Q_DISABLE_COPY		/* avoid unresolved externals */
#endif
#endif // _OS_WIN32_

#ifndef Q_EXPORT
#define Q_EXPORT
#endif
#ifndef Q_EXPORTM
#define Q_EXPORTM
#endif


//
// Useful type definitions for Qt
//

#if defined(bool)
#define HAS_BOOL_TYPE
#elif __GNUC__ > 2 || (__GNUC__ == 2 && __GNUC_MINOR__ >= 6)
#define HAS_BOOL_TYPE
#elif _MSC_VER >= 1100 || __BORLANDC__ >= 0x500
#define HAS_BOOL_TYPE
#elif defined(_CC_COMEAU_)
#define HAS_BOOL_TYPE
#elif defined(sgi) && ( (_COMPILER_VERSION >= 710) || defined(_BOOL) )
#define HAS_BOOL_TYPE
#elif defined(__DECCXX) && (__DECCXX_VER >= 60060005)
#define HAS_BOOL_TYPE
#endif

#if !defined(HAS_BOOL_TYPE)
#if defined(_CC_MSVC_)
#define _CC_BOOL_DEF_
#define bool		int
#else
typedef int		bool;
#endif
#endif

typedef unsigned char	uchar;
typedef unsigned short	ushort;
typedef unsigned	uint;
typedef unsigned long	ulong;
typedef char	       *pchar;
typedef uchar	       *puchar;
typedef const char     *pcchar;


//
// Constant bool values
//

#ifndef TRUE
const bool FALSE = 0;
const bool TRUE = !0;
#endif


//
// Utility macros and inline functions
//

#define QMAX(a,b)	((a) > (b) ? (a) : (b))
#define QMIN(a,b)	((a) < (b) ? (a) : (b))
#define QABS(a)		((a) >= 0  ? (a) : -(a))

inline int qRound( double d )
{
    return d > 0.0 ? int(d+0.5) : int(d-0.5);
}


//
// Size-dependent types (architechture-dependent byte order)
//

#if !defined(QT_CLEAN_NAMESPACE)
typedef char		INT8;			// 8 bit signed
typedef unsigned char	UINT8;			// 8 bit unsigned
typedef short		INT16;			// 16 bit signed
typedef unsigned short	UINT16;			// 16 bit unsigned
typedef int		INT32;			// 32 bit signed
typedef unsigned int	UINT32;			// 32 bit unsigned
#endif

typedef char		Q_INT8;			// 8 bit signed
typedef unsigned char	Q_UINT8;		// 8 bit unsigned
typedef short		Q_INT16;		// 16 bit signed
typedef unsigned short	Q_UINT16;		// 16 bit unsigned
typedef int		Q_INT32;		// 32 bit signed
typedef unsigned int	Q_UINT32;		// 32 bit unsigned


//
// Data stream functions is provided by many classes (defined in qdatastream.h)
//

class QDataStream;


//
// System information
//

Q_EXPORT const char *qVersion();
Q_EXPORT bool qSysInfo( int *wordSize, bool *bigEndian );


//
// Debugging and error handling
//

#if !defined(NO_CHECK)
#define CHECK_STATE				// check state of objects etc.
#define CHECK_RANGE				// check range of indexes etc.
#define CHECK_NULL				// check null pointers
#define CHECK_MATH				// check math functions
#endif

#if !defined(NO_DEBUG) && !defined(DEBUG)
#define DEBUG					// display debug messages
#endif

//
// To get C++ compiler warnings, define CC_WARNINGS or comment out the line
// "#define NO_WARNINGS"
//

#if !defined(CC_WARNINGS)
#define NO_WARNINGS
#endif
#if defined(NO_WARNINGS)
#if defined(_CC_MSVC_)
#pragma warning(disable: 4244)
#pragma warning(disable: 4275)
#pragma warning(disable: 4800)
#elif defined(_CC_BOR_)
#pragma option -w-inl
#pragma warn -inl
#pragma warn -pia
#pragma warn -ccc
#pragma warn -rch
#pragma warn -sig
#elif defined(_CC_MWERKS_)
#pragma warn_possunwant off
#endif
#endif // NO_WARNINGS

//
// Avoid compiler warning "argument defined but not used"
//

#if !defined(NOT_USED)
#define NOT_USED(argument) argument=argument;
#endif

//
// Avoid dead code
//

#if defined(_CC_EDG_) || defined(_CC_WAT_)
#define NO_DEADCODE
#endif

//
// Avoid compiler warning "function defined but not used" (gcc only)
//

#if defined(NOT_USED_FN)
// user provided
#elif __GNUC__ > 2 || (__GNUC__ == 2 && __GNUC_MINOR__ >= 7)
#define NOT_USED_FN __attribute__ ((unused))
#else
#define NOT_USED_FN
#endif

//
// Avoid "char ident[54] defined but not used" for the RCS tag
//

#if !defined(DEBUG)
#define RCSTAG(string)
#elif __GNUC__ > 2 || (__GNUC__ == 2 && __GNUC_MINOR__ >= 7)
#define RCSTAG(string) static char ident[] __attribute__ ((unused)) = string
#elif defined(_CC_DEC_) || defined(_CC_COMEAU_)
#define RCSTAG(string) typedef int QUnusedSemicolon
#else
#define RCSTAG(string)
#endif

Q_EXPORT void debug( const char *, ... )	// print debug message
#if defined(_CC_GNU_)
    __attribute__ ((format (printf, 1, 2)))
#endif
;

Q_EXPORT void warning( const char *, ... )	// print warning message
#if defined(_CC_GNU_)
    __attribute__ ((format (printf, 1, 2)))
#endif
;

Q_EXPORT void fatal( const char *, ... )	// print fatal message and exit
#if defined(_CC_GNU_)
    __attribute__ ((format (printf, 1, 2)))
#endif
;

#if defined(QT_FATAL_ASSERT)
#define ASSERT(x)  if ( !(x) )\
	fatal("ASSERT: \"%s\" in %s (%d)",#x,__FILE__,__LINE__)
#else
#define ASSERT(x)  if ( !(x) )\
	warning("ASSERT: \"%s\" in %s (%d)",#x,__FILE__,__LINE__)
#endif

Q_EXPORT bool chk_pointer( bool c, const char *, int );

#if defined(CHECK_NULL)
#define CHECK_PTR(p) (chk_pointer((p)==0,__FILE__,__LINE__))
#else
#define CHECK_PTR(p)
#endif

enum QtMsgType { QtDebugMsg, QtWarningMsg, QtFatalMsg };

typedef void (*msg_handler)(QtMsgType, const char *);
Q_EXPORT msg_handler qInstallMsgHandler( msg_handler );


//
// Enable QT_ADD_GENERIC_MACROS unless QT_REJECT_OBSOLETE is defined
//
#if !defined(QT_REJECT_OBSOLETE)
#define QT_ADD_GENERIC_MACROS
#endif

Q_EXPORT void qSuppressObsoleteWarnings( bool = TRUE );

#if !defined(QT_REJECT_OBSOLETE)
#define QT_OBSOLETE
Q_EXPORT void qObsolete( const char *obj, const char *oldfunc,
			 const char *newfunc );
Q_EXPORT void qObsolete( const char *obj, const char *oldfunc );
Q_EXPORT void qObsolete( const char *message );
#endif


#endif // QGLOBAL_H
