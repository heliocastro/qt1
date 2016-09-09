/****************************************************************************
** $Id: qfileinfo.cpp,v 2.12 1998/07/03 00:09:44 hanord Exp $
**
** Implementation of QFileInfo class
**
** Created : 950628
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
#if defined(_OS_SUN_)
#define readlink _qt_hide_readlink
#endif

#include "qfileinfo.h"
#include "qfiledefs.h"
#include "qdatetime.h"
#include "qdir.h"
#if defined(UNIX)
#include <pwd.h>
#include <grp.h>
#endif

#if defined(_OS_SUN_)
#undef readlink
extern "C" int readlink( const char *, void *, uint );
#endif

#if defined(_OS_FATFS_)

static void convertSeparators( char *n )
{
    if ( !n )
	return;
    while ( *n ) {
	if ( *n ==  '\\' )
	    *n = '/';
	n++;
    }
}

#elif defined(UNIX)

static void convertSeparators( char * )
{
    return;
}

#endif


struct QFileInfoCache
{
    STATBUF st;
    bool isSymLink;
};


/*!
  \class QFileInfo qfileinfo.h
  \brief The QFileInfo class provides system-independent file information.

  \ingroup io

  QFileInfo provides information about a file's name and position (path) in
  the file system, its access rights and whether it is a directory or a
  symbolic link.  Its size and last modified/read times are also available.

  To speed up performance QFileInfo caches information about the file. Since
  files can be changed by other users or programs, or even by other parts of
  the same program there is a function that refreshes the file information;
  refresh(). If you would rather like a QFileInfo to access the file system
  every time you request information from it, you can call the function
  setCaching( FALSE ).

  A QFileInfo can point to a file using either a relative or an absolute
  file path. Absolute file paths begin with the directory separator
  ('/') or a drive specification (not applicable to UNIX).
  Relative file names begin with a directory name or a file name and specify
  a path relative to the current directory. An example of
  an absolute path is the string "/tmp/quartz". A relative path might look like
  "src/fatlib". You can use the function isRelative() to check if a QFileInfo
  is using a relative or an absolute file path. You can call the function
  convertToAbs() to convert a relative QFileInfo to an absolute one.

  If you need to read and traverse directories, see the QDir class.
*/


/*!
  Constructs a new empty QFileInfo.
*/

QFileInfo::QFileInfo()
{
    fic	  = 0;
    cache = TRUE;
}

/*!
  Constructs a new QFileInfo that gives information about the given file.
  The string given can be an absolute or a relative file path.

  \sa bool setFile(const char*), isRelative(), QDir::setCurrent(),
  QDir::isRelativePath()
*/

QFileInfo::QFileInfo( const char *file )
{
    fn	  = file;
    convertSeparators( fn.data() );
    fic	  = 0;
    cache = TRUE;
}

/*!
  Constructs a new QFileInfo that gives information about \e file.

  If the file has a relative path, the QFileInfo will also have one.

  \sa isRelative()
*/

QFileInfo::QFileInfo( const QFile &file )
{
    fn	  = file.name();
    convertSeparators( fn.data() );
    fic	  = 0;
    cache = TRUE;
}

/*!
  Constructs a new QFileInfo that gives information about the file
  named \e fileName in the directory \e d.

  If the directory has a relative path, the QFileInfo will also have one.

  \sa isRelative()
*/

QFileInfo::QFileInfo( const QDir &d, const char *fileName )
{
    fn	  = d.filePath( fileName );
    convertSeparators( fn.data() );
    fic	  = 0;
    cache = TRUE;
}

/*!
  Constructs a new QFileInfo that is a copy of \e fi.
*/

QFileInfo::QFileInfo( const QFileInfo &fi )
{
    fn = fi.fn;
    if ( fi.fic ) {
	fic = new QFileInfoCache;
	*fic = *fi.fic;
    } else {
	fic = 0;
    }
    cache = fi.cache;
}

/*!
  Destroys the QFileInfo.
*/

QFileInfo::~QFileInfo()
{
    delete fic;
}


/*!
  Makes a copy of \e fi and assigns it to this QFileInfo.
*/

QFileInfo &QFileInfo::operator=( const QFileInfo &fi )
{
    fn = fi.fn;
    if ( !fi.fic ) {
	delete fic;
	fic = 0;
    } else {
	if ( !fic ) {
	    fic = new QFileInfoCache;
	    CHECK_PTR( fic );
	}
	*fic = *fi.fic;
    }
    cache = fi.cache;
    return *this;
}


/*!
  Sets the file to obtain information about.

  The string given can be an absolute or a relative file path. Absolute file
  paths begin with the directory separator (e.g. '/' under UNIX) or a drive
  specification (not applicable to UNIX). Relative file names begin with a
  directory name or a file name and specify a path relative to the current
  directory.

  Example:
  \code
    #include <qfileinfo.h>
    #include <qdir.h>

    void test()
    {
	const char *absolute = "/liver/aorta";
	const char *relative = "liver/aorta";
	QFileInfo fi1( absolute );
	QFileInfo fi2( relative );

	QDir::setCurrent( QDir::rootDirPath() );
				// fi1 and fi2 now point to the same file

	QDir::setCurrent( "/tmp" );
				// fi1 now points to "/liver/aorta",
				// while fi2 points to "/tmp/liver/aorta"
    }
  \endcode

  \sa isRelative(), QDir::setCurrent(), QDir::isRelativePath()
*/

void QFileInfo::setFile( const char *file )
{
    fn = file;
    convertSeparators( fn.data() );
    delete fic;
    fic = 0;
}

/*!
  Sets the file to obtain information about.

  If the file has a relative path, the QFileInfo will also have one.

  \sa isRelative()
*/

void QFileInfo::setFile( const QFile &file )
{
    fn	= file.name();
    convertSeparators( fn.data() );
    delete fic;
    fic = 0;
}

/*!
  Sets the file to obains information about to \e fileName in the
  directory \e d.

  If the directory has a relative path, the QFileInfo will also have one.

  \sa isRelative()
*/

void QFileInfo::setFile( const QDir &d, const char *fileName )
{
    fn	= d.filePath( fileName );
    convertSeparators( fn.data() );
    delete fic;
    fic = 0;
}


/*!
  Returns TRUE if the file pointed to exists, otherwise FALSE.
*/

bool QFileInfo::exists() const
{
    if ( !fn.isNull() )
	return ACCESS( fn.data(), F_OK ) == 0;
    else
	return FALSE;
}

/*!
  Refresh the information about the file, i.e. read in information from the
  file system the next time a cached property is fetched.

  \sa setCaching()
*/

void QFileInfo::refresh() const
{
    QFileInfo *that = (QFileInfo*)this;		// Mutable function
    delete that->fic;
    that->fic = 0;
}

/*!
  \fn bool QFileInfo::caching() const
  Returns TRUE if caching is enabled.
  \sa setCaching(), refresh()
*/

/*!
  Enables caching of file information if \e enable is TRUE, or disables it
  if \e enable is FALSE.

  When caching is enabled, QFileInfo reads the file information the first
  time

  Caching is enabled by default.

  \sa refresh(), caching()
*/

void QFileInfo::setCaching( bool enable )
{
    if ( cache == enable )
	return;
    cache = enable;
    if ( cache ) {
	delete fic;
	fic = 0;
    }
}


/*!
  Returns the name, i.e. the file name including the path (which can be
  absolute or relative).

  \sa isRelative(), absFilePath()
*/

const char *QFileInfo::filePath() const
{
    return fn.data();
}

/*!
  Returns the name of the file, the file path is not included.

  Example:
  \code
     QFileInfo fi( "/tmp/abdomen.lower" );
     QString name = fi.fileName();		// name = "abdomen.lower"
  \endcode

  \sa isRelative(), filePath(), baseName(), extension()
*/

QString QFileInfo::fileName() const
{
    int p = fn.findRev( '/' );
    if ( p == -1 )
	return fn.copy();
    else
	return QString( &fn[p+1] );
}

/*!
  Returns the absolute path name.

  The absolute path name is the file name including the absolute path. If
  the QFileInfo is absolute (i.e. not relative) this function will return
  the same string as filePath().

  Note that this function can be time-consuming under UNIX. (in the order
  of milliseconds on a 486 DX2/66 running Linux).

  \sa isRelative(), filePath()
*/

QString QFileInfo::absFilePath() const
{
    if ( QDir::isRelativePath(fn) ) {
	QString tmp = QDir::currentDirPath();
	tmp.detach();
	tmp += '/';
	tmp += fn;
	return QDir::cleanDirPath( tmp );
    } else {
	return QDir::cleanDirPath( fn );
    }

}

/*!
  Returns the base name of the file.

  The base name consists of all characters in the file name up to (but not
  including) the first '.' character.  The path is not included.

  Example:
  \code
     QFileInfo fi( "/tmp/abdomen.lower" );
     QString base = fi.baseName();		// base = "abdomen"
  \endcode

  \sa fileName(), extension()
*/

QString QFileInfo::baseName() const
{
    QString tmp = fileName();
    int pos = tmp.find( '.' );
    if ( pos == -1 )
	return tmp;
    else
	return tmp.left( pos );
}

/*!
  Returns the extension name of the file.

  The file name extension consists of all characters in the file name
  after (but not including) the first '.' character.

  For a file named "archive.tar.gz" this function will return "tar.gz".

  Example:
  \code
     QFileInfo fi( "/tmp/abdomen.lower" );
     QString ext = fi.extension();		// ext = "lower"
  \endcode

  \sa fileName(), baseName()
*/

QString QFileInfo::extension() const
{
    QString s = fileName();
    int pos = s.find( '.' );
    if ( pos == -1 )
	return QString( "" );
    else
	return s.right( s.length() - pos - 1 );
}


/*!
  Returns the directory path of the file.

  If \e absPath is TRUE an absolute path is always returned.

  \sa dir(), filePath(), fileName(), isRelative()
*/

QString QFileInfo::dirPath( bool absPath ) const
{
    QString s;
    if ( absPath )
	s = absFilePath();
    else
	s = fn;
    int pos = s.findRev( '/' );
    if ( pos == -1 )
	return ".";
    else
	return s.left( pos );
}

/*!
  Returns the directory path of the file.

  If the QFileInfo is relative and \e absPath is FALSE, the QDir will be
  relative, otherwise it will be absolute.

  \sa dirPath(), filePath(), fileName(), isRelative()
*/

QDir QFileInfo::dir( bool absPath ) const
{
    return QDir( dirPath(absPath) );
}


/*!
  Returns TRUE if the file is readable.
  \sa isWritable(), isExecutable(), permission()
*/

bool QFileInfo::isReadable() const
{
    if ( !fn.isNull() )
	return ACCESS( fn.data(), R_OK ) == 0;
    else
	return FALSE;
}

/*!
  Returns TRUE if the file is writable.
  \sa isReadable(), isExecutable(), permission()
*/

bool QFileInfo::isWritable() const
{
    if ( !fn.isNull() )
	return ACCESS( fn.data(), W_OK ) == 0 ;
    else
	return FALSE;
}

/*!
  Returns TRUE if the file is executable.
  \sa isReadable(), isWritable(), permission()
*/

bool QFileInfo::isExecutable() const
{
    if ( !fn.isNull() )
	return ACCESS( fn.data(), X_OK ) == 0;
    else
	return FALSE;
}


/*!
  Returns TRUE if the file path name is relative to the current directory,
  FALSE if the path is absolute (e.g. under UNIX a path is relative if it
  does not start with a '/').

  According to Einstein this function should always return TRUE.
*/

bool QFileInfo::isRelative() const
{
    return QDir::isRelativePath( fn.data() );
}

/*!
  Converts the file path name to an absolute path.

  If it is already absolute nothing is done.

  \sa filePath(), isRelative()
*/

bool QFileInfo::convertToAbs()
{
    if ( isRelative() )
	fn = absFilePath();
    return QDir::isRelativePath( fn.data() );
}


/*!
  Returns TRUE if we are pointing to a file or a symbolic link to a file.
  \sa isDir(), isSymLink()
*/

bool QFileInfo::isFile() const
{
#if defined(_OS_MAC_)
    return FALSE;
#else
    if ( !fic || !cache )
	doStat();
    return fic ? (fic->st.st_mode & STAT_MASK) == STAT_REG : FALSE;
#endif
}

/*!
  Returns TRUE if we are pointing to a directory or a symbolic link to
  a directory.
  \sa isFile(), isSymLink()
*/

bool QFileInfo::isDir() const
{
#if defined(_OS_MAC_)
    return FALSE;
#else
    if ( !fic || !cache )
	doStat();
    return fic ? (fic->st.st_mode & STAT_MASK) == STAT_DIR : FALSE;
#endif
}

/*!
  Returns TRUE if we are pointing to a symbolic link.
  \sa isFile(), isDir(), readLink()
*/

bool QFileInfo::isSymLink() const
{
    if ( !fic || !cache )
	doStat();
    return fic ? fic->isSymLink : FALSE;
}


/*!
  Returns the name a symlink points to, or a null QString if the
  object does not refer to a symbolic link.

  This name may not represent an existing file; it is only a string.
  QFileInfo::exists() returns TRUE if the symlink points to an
  existing file.

  \sa exists(), isSymLink(), isDir(), isFile()
*/

QString QFileInfo::readLink() const
{
    QString s;
#if defined(UNIX) && !defined(_OS_OS2EMX_)
    if ( !isSymLink() )
	return s;
    s.resize( PATH_MAX+1 );
    int len = readlink( fn.data(), s.data(), s.size() );
    if ( len < 0 )
	len = 0;				// error, return empty string
    s.truncate( len );
#endif
    return s;
}



/*!
  Returns the owner of the file.

  On systems where files do not have owners this function returns 0.

  Note that this function can be time-consuming under UNIX. (in the order
  of milliseconds on a 486 DX2/66 running Linux).

  \sa ownerId(), group(), groupId()
*/

const char *QFileInfo::owner() const
{
#if defined(UNIX)
    passwd *pw = getpwuid( ownerId() );
    return pw ? pw->pw_name : 0;
#else
    return 0;
#endif
}

static const uint nobodyID = (uint) -2;

/*!
  Returns the id of the owner of the file.

  On systems where files do not have owners this function returns ((uint) -2).

  \sa owner(), group(), groupId()
*/

uint QFileInfo::ownerId() const
{
#if defined(UNIX)
    if ( !fic || !cache )
	doStat();
    if ( fic )
	return fic->st.st_uid;
#endif
    return nobodyID;
}

/*!
  Returns the group the file belongs to.

  On systems where files do not have groups this function always
  returns 0.

  Note that this function can be time-consuming under UNIX (in the order of
  milliseconds on a 486 DX2/66 running Linux).

  \sa groupId(), owner(), ownerId()
*/

const char *QFileInfo::group() const
{
#if defined(UNIX)
    struct group *gr = getgrgid( groupId() );
    return gr ? gr->gr_name : 0;
#else
    return 0;
#endif
}

/*!
  Returns the id of the group the file belongs to.

  On systems where files do not have groups this function always
  returns ((uind) -2).

  \sa group(), owner(), ownerId()
*/

uint QFileInfo::groupId() const
{
#if defined(UNIX)
    if ( !fic || !cache )
	doStat();
    if ( fic )
	return fic->st.st_gid;
#endif
    return nobodyID;
}


/*!
  \fn bool QFileInfo::permission( int permissionSpec ) const

  Tests for file permissions.  The \e permissionSpec argument can be several
  flags of type PermissionSpec or'ed together to check for permission
  combinations.

  On systems where files do not have permissions this function always
  returns TRUE.

  Example:
  \code
    QFileInfo fi( "/tmp/tonsils" );
    if ( fi.permission( QFileInfo::WriteUser | QFileInfo::ReadGroup ) )
	warning( "Tonsils can be changed by me, and the group can read them.");
    if ( fi.permission( QFileInfo::WriteGroup | QFileInfo::WriteOther ) )
	warning( "Danger! Tonsils can be changed by the group or others!" );
  \endcode

  \sa isReadable(), isWritable(), isExecutable()
*/

#if defined(UNIX)
bool QFileInfo::permission( int permissionSpec ) const
{
    if ( !fic || !cache )
	doStat();
    if ( fic ) {
	uint mask = 0;
	if ( permissionSpec & ReadUser)
	    mask |= S_IRUSR;
	if ( permissionSpec & WriteUser)
	    mask |= S_IWUSR;
	if ( permissionSpec & ExeUser)
	    mask |= S_IXUSR;
	if ( permissionSpec & ReadGroup)
	    mask |= S_IRGRP;
	if ( permissionSpec & WriteGroup)
	    mask |= S_IWGRP;
	if ( permissionSpec & ExeGroup)
	    mask |= S_IXGRP;
	if ( permissionSpec & ReadOther)
	    mask |= S_IROTH;
	if ( permissionSpec & WriteOther)
	    mask |= S_IWOTH;
	if ( permissionSpec & ExeOther)
	    mask |= S_IXOTH;
	if ( mask ) {
	   return (fic->st.st_mode & mask) == mask;
	} else {
#if defined(CHECK_NULL)
	   warning( "QFileInfo::permission: permissionSpec is 0" );
#endif
	   return TRUE;
	}
    } else {
	return FALSE;
    }
}

#else	/* not UNIX */

bool QFileInfo::permission( int ) const
{
    return TRUE;
}
#endif

/*!
  Returns the file size in bytes, or 0 if the file does not exist if the size
  cannot be fetched.
*/

uint QFileInfo::size() const
{
    if ( !fic || !cache )
	doStat();
    if ( fic )
	return (uint)fic->st.st_size;
    else
	return 0;
}


/*!
  Returns the date and time when the file was last modified.
  \sa lastRead()
*/

QDateTime QFileInfo::lastModified() const
{
    QDateTime dt;
    if ( !fic || !cache )
	doStat();
    if ( fic )
	dt.setTime_t( fic->st.st_mtime );
    return dt;
}

/*!
  Returns the date and time when the file was last read (accessed).

  On systems that do not support last read times, the modification time is
  returned.

  \sa lastModified()
*/

QDateTime QFileInfo::lastRead() const
{
    QDateTime dt;
    if ( !fic || !cache )
	doStat();
    if ( fic )
	dt.setTime_t( fic->st.st_atime );
    return dt;
}


void QFileInfo::doStat() const
{
    QFileInfo *that = ((QFileInfo*)this);	// mutable function
    if ( !that->fic )
	that->fic = new QFileInfoCache;
    STATBUF *b = &that->fic->st;
    that->fic->isSymLink = FALSE;

#if defined( UNIX ) && defined(S_IFLNK)
    if ( ::lstat(fn.data(),b) == 0 ) {
	if ( S_ISLNK( b->st_mode ) )
	    that->fic->isSymLink = TRUE;
	else
	    return;
    }
#endif

    if ( STAT(fn.data(),b) != 0 ) {
	delete that->fic;
	that->fic = 0;
    }
}
