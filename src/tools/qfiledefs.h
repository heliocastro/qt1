/****************************************************************************
** $Id: qfiledefs.h,v 2.9.2.1 1998/12/30 16:31:28 hanord Exp $
**
**		      ***   INTERNAL HEADER FILE   ***
**
**		This file is NOT a part of the Qt interface!
**
** Common macros and system include files for QFile, QFileInfo and QDir.
** This file is included by qfile.cpp, qfileinfo.cpp and qdir.cpp
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

#if defined(_CC_MWERKS_)
# include <stdlib.h>
# include <stat.h>
#elif !defined(_OS_MAC_)
# include <sys/types.h>
# include <sys/stat.h>
#endif
#include <fcntl.h>
#include <errno.h>
#if defined(UNIX)
# include <dirent.h>
# include <unistd.h>
#endif
#if defined(_OS_MSDOS_) || defined(_OS_WIN32_) || defined(_OS_OS2_)
# define _OS_FATFS_
# if defined(__CYGWIN32__)
#  include <dirent.h>
#  include <unistd.h>
# else
#  include <io.h>
#  if !defined(_CC_MWERKS_)
#   include <dos.h>
#  endif
#  include <direct.h>
# endif
#endif
#include <limits.h>


#if !defined(PATH_MAX)
#if defined( MAXPATHLEN )
#define PATH_MAX MAXPATHLEN
#else
#define PATH_MAX 1024
#endif
#endif


#undef STATBUF
#undef STAT
#undef STAT_REG
#undef STAT_DIR
#undef STAT_LNK
#undef STAT_MASK
#undef FILENO
#undef OPEN
#undef CLOSE
#undef LSEEK
#undef READ
#undef WRITE
#undef ACCESS
#undef GETCWD
#undef CHDIR
#undef MKDIR
#undef RMDIR
#undef OPEN_RDONLY
#undef OPEN_WRONLY
#undef OPEN_CREAT
#undef OPEN_TRUNC
#undef OPEN_APPEND
#undef OPEN_TEXT
#undef OPEN_BINARY


#if defined(_CC_MSVC_) || defined(_CC_SYM_)

# define STATBUF	struct _stat		// non-ANSI defs
# define STAT		::_stat
# define FSTAT		::_fstat
# define STAT_REG	_S_IFREG
# define STAT_DIR	_S_IFDIR
# define STAT_MASK	_S_IFMT
# if defined(_S_IFLNK)
#  define STAT_LNK	_S_IFLNK
# endif
# define FILENO		_fileno
# define OPEN		::_open
# define CLOSE		::_close
# define LSEEK		::_lseek
# define READ		::_read
# define WRITE		::_write
# define ACCESS		::_access
# define GETCWD		::_getcwd
# define CHDIR		::_chdir
# define MKDIR		::_mkdir
# define RMDIR		::_rmdir
# define OPEN_RDONLY	_O_RDONLY
# define OPEN_WRONLY	_O_WRONLY
# define OPEN_RDWR	_O_RDWR
# define OPEN_CREAT	_O_CREAT
# define OPEN_TRUNC	_O_TRUNC
# define OPEN_APPEND	_O_APPEND
# if defined(O_TEXT)
#  define OPEN_TEXT	_O_TEXT
#  define OPEN_BINARY	_O_BINARY
# endif

#else						// all other systems

# define STATBUF	struct stat
# define STAT		::stat
# define FSTAT		::fstat
# define STAT_REG	S_IFREG
# define STAT_DIR	S_IFDIR
# define STAT_MASK	S_IFMT
# if defined(S_IFLNK)
#  define STAT_LNK	S_IFLNK
# endif
# define FILENO		fileno
# define OPEN		::open
# define CLOSE		::close
# define LSEEK		::lseek
# define READ		::read
# define WRITE		::write
# define ACCESS		::access
# if defined(_OS_OS2EMX_)
#  define GETCWD	::_getcwd2
#  define CHDIR		::_chdir2
# else
#  define GETCWD	::getcwd
#  define CHDIR		::chdir
# endif
# define MKDIR		::mkdir
# define RMDIR		::rmdir
# define OPEN_RDONLY	O_RDONLY
# define OPEN_WRONLY	O_WRONLY
# define OPEN_RDWR	O_RDWR
# define OPEN_CREAT	O_CREAT
# define OPEN_TRUNC	O_TRUNC
# define OPEN_APPEND	O_APPEND
# if defined(O_TEXT)
#  define OPEN_TEXT	O_TEXT
#  define OPEN_BINARY	O_BINARY
# endif
#endif

#if defined(_CC_MWERKS_)
#undef mkdir
#undef MKDIR
#define MKDIR _mkdir
#undef rmdir
#undef RMDIR
#define RMDIR _rmdir
#endif


#if defined(_OS_FATFS_)
# define F_OK	0
# define X_OK	1
# define W_OK	2
# define R_OK	4
#endif
