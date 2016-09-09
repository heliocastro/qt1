/****************************************************************************
** $Id: qdir.cpp,v 2.25.2.2 1999/02/22 07:46:16 hanord Exp $
**
** Implementation of QDir class
**
** Created : 950427
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

#include "qdir.h"
#include "qfileinfo.h"
#include "qfiledefs.h"
#include "qregexp.h"
#include <stdlib.h>
#include <ctype.h>
#if defined(_OS_WIN32_)
#if defined(_CC_BOOL_DEF_)
#undef	bool
#include <windows.h>
#define bool int
#else
#include <windows.h>
#endif
#endif
#if defined(_OS_OS2EMX_)
extern Q_UINT32 DosQueryCurrentDisk(Q_UINT32*,Q_UINT32*);
#define NO_ERROR 0
#endif

#if defined(_OS_FATFS_) || defined(_OS_OS2EMX_)

static void slashify( char *n )
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

static void slashify( char * )
{
    return;
}

#endif


/*!
  \class QDir qdir.h
  \brief Traverses directory structures and contents in a
	    platform-independent way.

  \ingroup io

  A QDir can point to a file using either a relative or an absolute file
  path. Absolute file paths begin with the directory separator ('/') or a
  drive specification (not applicable to UNIX).	 Relative file names begin
  with a directory name or a file name and specify a path relative to the
  current directory.

  An example of an absolute path is the string "/tmp/quartz", a relative
  path might look like "src/fatlib". You can use the function isRelative()
  to check if a QDir is using a relative or an absolute file path. You can
  call the function convertToAbs() to convert a relative QDir to an
  absolute one.

  The directory "example" under the current directory is checked for existence
  in the example below:

  \code
    QDir d( "example" );			// "./example"
    if ( !d.exists() )
	warning( "Cannot find the example directory" );
  \endcode

  If you always use '/' as a directory separator, Qt will translate your
  paths to conform to the underlying operating system.

  cd() and cdUp() can be used to navigate the directory tree. Note that the
  logical cd and cdUp operations are not performed if the new directory does
  not exist.

  Example:
  \code
    QDir d = QDir::root();			// "/"
    if ( !d.cd("tmp") ) {			// "/tmp"
	warning( "Cannot find the \"/tmp\" directory" );
    } else {
	QFile f( d.filePath("ex1.txt") );	// "/tmp/ex1.txt"
	if ( !f.open(IO_ReadWrite) )
	    warning( "Cannot create the file %s", f.name() );
    }
  \endcode

  To read the contents of a directory you can use the entryList() and
  entryInfoList() functions.

  Example:
  \code
    #include <stdio.h>
    #include <qdir.h>

    //
    // This program scans the current directory and lists all files
    // that are not symbolic links, sorted by size with the smallest files
    // first.
    //

    int main( int argc, char **argv )
    {
	QDir d;
	d.setFilter( QDir::Files | QDir::Hidden | QDir::NoSymLinks );
	d.setSorting( QDir::Size | QDir::Reversed );

	const QFileInfoList *list = d.entryInfoList();
	QFileInfoListIterator it( *list );	// create list iterator
	QFileInfo *fi;				// pointer for traversing

	printf( "     BYTES FILENAME\n" );	// print header
	while ( (fi=it.current()) ) {		// for each file...
	    printf( "%10li %s\n", fi->size(), fi->fileName().data() );
	    ++it;				// goto next list element
	}
    }
  \endcode
*/


/*!
  Constructs a QDir pointing to the current directory.
  \sa currentDirPath()
*/

QDir::QDir()
{
    dPath = ".";
    init();
}

/*!
  Constructs a QDir.

  \arg \e path is the directory.
  \arg \e nameFilter is the file name filter.
  \arg \e sortSpec is the sort specification, which describes how to
  sort the files in the directory.
  \arg \e filterSpec is the filter specification, which describes how
  to filter the files in the directory.

  Most of these arguments (except \e path) have optional values.

  Example:
  \code
    // lists all files in /tmp

    QDir d( "/tmp" );
    for ( int i=0; i<d.count(); i++ )
	printf( "%s\n", d[i] );
  \endcode

  If \e path is "" or null, the directory is set to "." (the current
  directory).  If \e nameFilter is "" or null, it is set to "*" (all
  files).

  No check is made to ensure that the directory exists.

  \sa exists(), setPath(), setNameFilter(), setFilter(), setSorting()
*/

QDir::QDir( const char *path, const char *nameFilter, int sortSpec,
	    int filterSpec )
{
    init();
    dPath = cleanDirPath( path );
    if ( dPath.isEmpty() )
	dPath = ".";
    nameFilt = nameFilter;
    if ( nameFilt.isEmpty() )
	nameFilt = "*";
    filtS = (FilterSpec)filterSpec;
    sortS = (SortSpec)sortSpec;
}

/*!
  Constructs a QDir that is a copy of the given directory.
  \sa operator=()
*/

QDir::QDir( const QDir &d )
{
    dPath    = d.dPath;
    fList    = 0;
    fiList   = 0;
    nameFilt = d.nameFilt;
    dirty    = TRUE;
    allDirs  = d.allDirs;
    filtS    = d.filtS;
    sortS    = d.sortS;
}

void QDir::init()
{
    fList     = 0;
    fiList    = 0;
    nameFilt = "*";
    dirty    = TRUE;
    allDirs  = FALSE;
    filtS    = All;
    sortS    = SortSpec(Name | IgnoreCase);
}

/*!
  Destroys the QDir and cleans up.
*/

QDir::~QDir()
{
    if ( fList )
       delete fList;
    if ( fiList )
       delete fiList;
}


/*!
  Sets the path of the directory. The path is cleaned of redundant ".", ".."
  and multiple separators. No check is made to ensure that a directory
  with this path exists.

  The path can be either absolute or relative. Absolute paths begin with the
  directory separator ('/') or a drive specification (not
  applicable to UNIX).
  Relative file names begin with a directory name or a file name and specify
  a path relative to the current directory. An example of
  an absolute path is the string "/tmp/quartz", a relative path might look like
  "src/fatlib". You can use the function isRelative() to check if a QDir
  is using a relative or an absolute file path. You can call the function
  convertToAbs() to convert a relative QDir to an absolute one.

  \sa path(), absPath(), exists, cleanDirPath(), dirName(),
      absFilePath(), isRelative(), convertToAbs()
*/

void QDir::setPath( const char *path )
{
    dPath = cleanDirPath( path );
    if ( dPath.isEmpty() )
	dPath = ".";
    dirty = TRUE;
}

/*!
  \fn  const char *QDir::path() const
  Returns the path, this may contain symbolic links, but never contains
  redundant ".", ".." or multiple separators.

  The returned path can be either absolute or relative (see setPath()).

  \sa setPath(), absPath(), exists(), cleanDirPath(), dirName(),
  absFilePath(), convertSeparators()
*/

/*!
  Returns the absolute (a path that starts with '/') path, which may
  contain symbolic links, but never contains redundant ".", ".." or
  multiple separators.

  \sa setPath(), canonicalPath(), exists(),  cleanDirPath(), dirName(),
  absFilePath()
*/

QString QDir::absPath() const
{
    if ( QDir::isRelativePath(dPath) ) {
	QString tmp = currentDirPath();
	if ( tmp.right(1) != "/" )
	    tmp += '/';
	tmp += dPath;
	return cleanDirPath( tmp );
    } else {
	return cleanDirPath( dPath );
    }
}

/*!
  Returns the canonical path, i.e. a path without symbolic links.

  On systems that do not have symbolic links this function will
  always return the same string that absPath returns.
  If the canonical path does not exist a null string is returned.

  \sa path(), absPath(), exists(), cleanDirPath(), dirName(),
      absFilePath(), QString::isNull()
*/

QString QDir::canonicalPath() const
{
    QString cur( PATH_MAX );
    QString tmp( PATH_MAX );

    GETCWD( cur.data(), PATH_MAX );
    if ( CHDIR(dPath) >= 0 )
	GETCWD( tmp.data(), PATH_MAX );
    CHDIR( cur );

    return tmp;
}

/*!
  Returns the name of the directory, this is NOT the same as the path, e.g.
  a directory with the name "mail", might have the path "/var/spool/mail".
  If the directory has no name (e.g. the root directory) a null string is
  returned.

  No check is made to ensure that a directory with this name actually exists.

  \sa path(), absPath(), absFilePath(), exists(), QString::isNull()
*/

QString QDir::dirName() const
{
    int pos = dPath.findRev( '/' );
    if ( pos == -1  )
	return dPath;
    return dPath.right( dPath.length() - pos - 1 );
}

/*!
  Returns the path name of a file in the directory. Does NOT check if
  the file actually exists in the directory. If the QDir is relative
  the returned path name will also be relative. Redundant multiple separators
  or "." and ".." directories in \e fileName will not be removed (see
  cleanDirPath()).

  If \e acceptAbsPath is TRUE a \e fileName starting with a separator
  ('/') will be returned without change.
  If \e acceptAbsPath is FALSE an absolute path will be appended to
  the directory path.

  \sa absFilePath(), isRelative(), canonicalPath()
*/

QString QDir::filePath( const char *fileName,
			bool acceptAbsPath ) const
{
    if ( acceptAbsPath && !isRelativePath(fileName) )
	return QString(fileName);

    QString tmp = dPath.copy();
    if ( tmp.isEmpty() || (tmp[(int)tmp.length()-1] != '/' && fileName &&
			   fileName[0] != '/') )
	tmp += '/';
    tmp += fileName;
    return tmp;
}

/*!
  Returns the absolute path name of a file in the directory. Does NOT check if
  the file actually exists in the directory. Redundant multiple separators
  or "." and ".." directories in \e fileName will NOT be removed (see
  cleanDirPath()).

  If \e acceptAbsPath is TRUE a \e fileName starting with a separator
  ('/') will be returned without change.
  if \e acceptAbsPath is FALSE an absolute path will be appended to
  the directory path.

  \sa filePath()
*/

QString QDir::absFilePath( const char *fileName,
			   bool acceptAbsPath ) const
{
    if ( acceptAbsPath && !isRelativePath( fileName ) )
	return fileName;

    QString tmp = absPath();
    if ( tmp.isEmpty() || (tmp[(int)tmp.length()-1] != '/' && fileName &&
			   fileName[0] != '/') )
	tmp += '/';
    tmp += fileName;
    return tmp;
}


/*!
  Converts the '/' separators in \a pathName to system native
  separators.  Returns the translated string.

  On Windows, convertSeparators("c:/winnt/system32") returns
  "c:\winnt\system32".

  No conversion is done on UNIX.
*/

QString QDir::convertSeparators( const char *pathName )
{
    QString n( pathName );
#if defined(_OS_FATFS_) || defined(_OS_OS2EMX_)
    char *p = n.data();
    while ( p && *p ) {
	if ( *p == '/' )
	    *p = '\\';
	p++;
    }
#endif
    return n;
}


/*!
  Changes directory by descending into the given directory. Returns
  TRUE if the new directory exists and is readable. Note that the logical
  cd operation is NOT performed if the new directory does not exist.

  If \e acceptAbsPath is TRUE a path starting with a separator ('/')
  will cd to the absolute directory, if \e acceptAbsPath is FALSE
  any number of separators at the beginning of \e dirName will be removed.

  Example:
  \code
  QDir d = QDir::home();  // now points to home directory
  if ( !d.cd("c++") ) {	  // now points to "c++" under home directory if OK
      QFileInfo fi( d, "c++" );
      if ( fi.exists() ) {
	  if ( fi.isDir() )
	      warning( "Cannot cd into \"%s\".", (char*)d.absFilePath("c++") );
	  else
	      warning( "Cannot create directory \"%s\"\n"
		       "A file named \"c++\" already exists in \"%s\"",
		       (const char *)d.absFilePath("c++"),
		       (const char *)d.path() );
	  return;
      } else {
	  warning( "Creating directory \"%s\"",
		   (const char *) d.absFilePath("c++") );
	  if ( !d.mkdir( "c++" ) ) {
	      warning("Could not create directory \"%s\"",
		      (const char *)d.absFilePath("c++") );
	      return;
	  }
      }
  }
  \endcode

  Calling cd( ".." ) is equivalent to calling cdUp().

  \sa cdUp(), isReadable(), exists(), path()
*/

bool QDir::cd( const char *dirName, bool acceptAbsPath )
{
    if ( !dirName || !*dirName || strcmp(dirName,".") == 0 )
	return TRUE;
    QString old = dPath;
    dPath.detach();			// dPath can be shared by several QDirs
    if ( acceptAbsPath && !isRelativePath(dirName) ) {
	dPath = cleanDirPath( dirName );
    } else {
	if ( !isRoot() )
	    dPath += '/';
	dPath += dirName;
	if ( strchr(dirName,'/') || old == "." || strcmp(dirName,"..") == 0 )
	    dPath = cleanDirPath( dPath.data() );
    }
    if ( !exists() ) {
	dPath = old;			// regret
	return FALSE;
    }
    dirty = TRUE;
    return TRUE;
}

/*!
  Changes directory by moving one directory up the path followed to arrive
  at the current directory.

  Returns TRUE if the new directory exists and is readable. Note that the
  logical cdUp() operation is not performed if the new directory does not
  exist.

  \sa cd(), isReadable(), exists(), path()
*/

bool QDir::cdUp()
{
    return cd( ".." );
}

/*!
  \fn const char *QDir::nameFilter() const
  Returns the string set by setNameFilter()
  \sa setNameFilter()
*/

/*!
  Sets the name filter used by entryList() and entryInfoList().

  The name filter is a wildcarding filter that understands "*" and "?"
  wildcards, if you want entryList() and entryInfoList() to list all files
  ending with ".cpp", you simply call dir.setNameFilter("*.cpp");

  \sa nameFilter()
*/

void QDir::setNameFilter( const char *nameFilter )
{
    nameFilt = nameFilter;
    if ( nameFilt.isEmpty() )
	nameFilt = "*";
    dirty = TRUE;
}

/*!
  \fn QDir::FilterSpec QDir::filter() const
  Returns the value set by setFilter()
  \sa setFilter()
*/

/*!
  Sets the filter used by entryList() and entryInfoList(). The filter is used
  to specify the kind of files that should be returned by entryList() and
  entryInfoList(). The filter is specified by or-ing values from the enum
  FilterSpec. The different values are:


  <dl compact>
  <dt>Dirs<dd> List directories only.
  <dt>Files<dd> List files only.
  <dt>Drives<dd> List drives.
  <dt>NoSymLinks<dd> Do not list symbolic links.

  <dt>Readable<dd> List files with read permission.
  <dt>Writable<dd> List files with write permission.
  <dt>Executable<dd> List files with execute permission.

  Setting none of the three flags above is equivalent to setting all of them.

  <dt>Modified<dd> Only list files that have been modified (does nothing
			  under UNIX).
  <dt>Hidden<dd> List hidden files also (.* under UNIX).
  <dt>System<dd> List system files (does nothing under UNIX).

  </dl>

  \sa nameFilter()
*/

void QDir::setFilter( int filterSpec )
{
    if ( filtS == (FilterSpec) filterSpec )
	return;
    filtS = (FilterSpec) filterSpec;
    dirty = TRUE;
}

/*!
  \fn QDir::SortSpec QDir::sorting() const

  Returns the value set by setSorting()

  \sa setSorting()
*/

/*!
  Sets the sorting order used by entryList() and entryInfoList().

  The \e sortSpec is specified by or-ing values from the enum
  SortSpec. The different values are:

  One of these:
  <dl compact>
  <dt>Name<dd> Sort by name (alphabetical order).
  <dt>Time<dd> Sort by time (most recent first).
  <dt>Size<dd> Sort by size (largest first).
  <dt>Unsorted<dd> Use the operating system order (UNIX does NOT sort
  alphabetically).

  ORed with zero or more of these:

  <dt>DirsFirst<dd> Always put directory names first.
  <dt>Reversed<dd> Reverse sort order.
  <dt>IgnoreCase<dd> Ignore case when sorting by name.
  </dl>
*/

void QDir::setSorting( int sortSpec )
{
    if ( sortS == (SortSpec) sortSpec )
	return;
    sortS = (SortSpec) sortSpec;
    dirty = TRUE;
}

/*!
  \fn bool QDir::matchAllDirs() const
  Returns the value set by setMatchAllDirs()

  \sa setMatchAllDirs()
*/

/*!
  If \e enable is TRUE, all directories will be listed (even if they do not
  match the filter or the name filter), otherwise only matched directories
  will be listed.

  \bug Currently, directories that do not match the filter will not be
  included (the name filter will be ignored as expected).

  \sa matchAllDirs()
*/

void QDir::setMatchAllDirs( bool enable )
{
    if ( (bool)allDirs == enable )
	return;
    allDirs = enable;
    dirty = TRUE;
}


/*!
  Returns the number of files that was found.
  Equivalent to entryList()->count().
  \sa operator[], entryList()
*/

uint QDir::count() const
{
    entryList();
    return fList->count();
}

/*!
  Returns the file name at position \e index in the list of found file
  names.
  Equivalent to entryList()->at(index).

  Returns null if the \e index is out of range or if the entryList()
  function failed.

  \sa count(), entryList()
*/

const char *QDir::operator[]( int index ) const
{
    entryList();
    return fList && index >= 0 && index < (int)fList->count() ?
	fList->at(index) : 0;
}


/*!
  Returns a list of the names of all files and directories in the directory
  pointed to using the setSorting(), setFilter() and setNameFilter()
  specifications.

  The the filter and sorting specifications can be overridden using the
  \e filterSpec and \e sortSpec arguments.

  Returns 0 if the directory is unreadable or does not exist.

  \sa entryInfoList(), setNameFilter(), setSorting(), setFilter()
*/

const QStrList *QDir::entryList( int filterSpec, int sortSpec ) const
{
    if ( !dirty && filterSpec == (int)DefaultFilter &&
		   sortSpec   == (int)DefaultSort )
	return fList;
    return entryList( nameFilt, filterSpec, sortSpec );
}

/*!
  Returns a list of the names of all files and directories in the directory
  pointed to using the setSorting(), setFilter() and setNameFilter()
  specifications.

  The the filter and sorting specifications can be overridden using the
  \e nameFilter, \e filterSpec and \e sortSpec arguments.

  Returns 0 if the directory is unreadable or does not exist.

  \sa entryInfoList(), setNameFilter(), setSorting(), setFilter()
*/

const QStrList *QDir::entryList( const char *nameFilter,
				 int filterSpec, int sortSpec ) const
{
    if ( filterSpec == (int)DefaultFilter )
	filterSpec = filtS;
    if ( sortSpec == (int)DefaultSort )
	sortSpec = sortS;
    QDir *that = (QDir*)this;			// mutable function
    if ( that->readDirEntries(nameFilter, filterSpec, sortSpec) )
	return that->fList;
    else
	return 0;
}

/*!
  Returns a list of QFileInfo objects for all files and directories in
  the directory pointed to using the setSorting(), setFilter() and
  setNameFilter() specifications.

  The the filter and sorting specifications can be overridden using the
  \e filterSpec and \e sortSpec arguments.

  Returns 0 if the directory is unreadable or does not exist.

  \sa entryList(), setNameFilter(), setSorting(), setFilter()
*/

const QFileInfoList *QDir::entryInfoList( int filterSpec, int sortSpec ) const
{
    if ( !dirty && filterSpec == (int)DefaultFilter &&
		   sortSpec   == (int)DefaultSort )
	return fiList;
    return entryInfoList( nameFilt, filterSpec, sortSpec );
}

/*!
  Returns a list of QFileInfo objects for all files and directories in
  the directory pointed to using the setSorting(), setFilter() and
  setNameFilter() specifications.

  The the filter and sorting specifications can be overridden using the
  \e nameFilter, \e filterSpec and \e sortSpec arguments.

  Returns 0 if the directory is unreadable or does not exist.

  \sa entryList(), setNameFilter(), setSorting(), setFilter()
*/

const QFileInfoList *QDir::entryInfoList( const char *nameFilter,
					  int filterSpec, int sortSpec ) const
{
    if ( filterSpec == (int)DefaultFilter )
	filterSpec = filtS;
    if ( sortSpec == (int)DefaultSort )
	sortSpec = sortS;
    QDir *that = (QDir*)this;			// mutable function
    if ( that->readDirEntries(nameFilter, filterSpec, sortSpec) )
	return that->fiList;
    else
	return 0;
}

/*!
  Creates a directory.

  If \e acceptAbsPath is TRUE a path starting with a separator ('/')
  will create the absolute directory, if \e acceptAbsPath is FALSE
  any number of separators at the beginning of \e dirName will be removed.

  Returns TRUE if successful, otherwise FALSE.

  \sa rmdir()
*/

bool QDir::mkdir( const char *dirName, bool acceptAbsPath ) const
{
#if defined (UNIX) || defined(__CYGWIN32__)
    return MKDIR( filePath(dirName,acceptAbsPath), 0777 ) == 0;
#else
    return MKDIR( filePath(dirName,acceptAbsPath) ) == 0;
#endif
}

/*!
  Removes a directory.

  If \e acceptAbsPath is TRUE a path starting with a separator ('/')
  will remove the absolute directory, if \e acceptAbsPath is FALSE
  any number of separators at the beginning of \e dirName will be removed.

  The directory must be empty for rmdir() to succeed.

  Returns TRUE if successful, otherwise FALSE.

  \sa mkdir()
*/

bool QDir::rmdir( const char *dirName, bool acceptAbsPath ) const
{
    return RMDIR( filePath(dirName,acceptAbsPath) ) == 0;
}

/*!
  Returns TRUE if the directory is readable AND we can open files by
  name. This function will return FALSE if only one of these is present.
  \warning A FALSE value from this function is not a guarantee that files
  in the directory are not accessible.

  \sa QFileInfo::isReadable()
*/

bool QDir::isReadable() const
{
#if defined(UNIX)
    return ACCESS( dPath.data(), R_OK | X_OK ) == 0;
#else
    return ACCESS( dPath.data(), R_OK ) == 0;
#endif
}

/*!
  Returns TRUE if the directory exists. (If a file with the same
  name is found this function will of course return FALSE).

  \sa QFileInfo::exists(), QFile::exists()
*/

bool QDir::exists() const
{
    QFileInfo fi( dPath );
    return fi.exists() && fi.isDir();
}

/*!
  Returns TRUE if the directory is the root directory, otherwise FALSE.

  Note: If the directory is a symbolic link to the root directory this
  function returns FALSE. If you want to test for this you can use
  canonicalPath():

  Example:
  \code
    QDir d( "/tmp/root_link" );
    d = d.canonicalPath();
    if ( d.isRoot() )
	warning( "It IS a root link!" );
  \endcode

  \sa root(), rootDirPath()
*/

bool QDir::isRoot() const
{
#if defined(_OS_FATFS_) || defined(_OS_OS2EMX_)
    return dPath == "/" || dPath == "//" ||
	(isalpha(dPath[0]) && dPath.mid(1,dPath.length()) == ":/");
#else
    return dPath == "/";
#endif
}

/*!
  Returns TRUE if the directory path is relative to the current directory,
  FALSE if the path is absolute (e.g. under UNIX a path is relative if it
  does not start with a '/').

  According to Einstein this function should always return TRUE.

  \sa convertToAbs()
*/

bool QDir::isRelative() const
{
    return isRelativePath( dPath.data() );
}

/*!
  Converts the directory path to an absolute path. If it is already
  absolute nothing is done.

  \sa isRelative()
*/

void QDir::convertToAbs()
{
    dPath = absPath();
}

/*!
  Makes a copy of d and assigns it to this QDir.
*/

QDir &QDir::operator=( const QDir &d )
{
    dPath    = d.dPath;
    fList    = 0;
    fiList   = 0;
    nameFilt = d.nameFilt;
    dirty    = TRUE;
    allDirs  = d.allDirs;
    filtS    = d.filtS;
    sortS    = d.sortS;
    return *this;
}

/*!
  Sets the directory path to be the given path.
*/

QDir &QDir::operator=( const char *path )
{
    dPath = cleanDirPath( path );
    dirty = TRUE;
    return *this;
}


/*!
  \fn bool QDir::operator!=( const QDir &d ) const
  Returns TRUE if the \e d and this dir havee different path or
  different sort/filter settings, otherwise FALSE.
*/

/*!
  Returns TRUE if the \e d and this dir have the same path and all sort
  and filter settings are equal, otherwise FALSE.
*/

bool QDir::operator==( const QDir &d ) const
{
    return dPath    == d.dPath &&
	   nameFilt == d.nameFilt &&
	   allDirs  == d.allDirs &&
	   filtS    == d.filtS &&
	   sortS    == d.sortS;
}


/*!
  Removes a file.

  If \e acceptAbsPath is TRUE a path starting with a separator ('/')
  will remove the file with the absolute path, if \e acceptAbsPath is FALSE
  any number of separators at the beginning of \e fileName will be removed.

  Returns TRUE if successful, otherwise FALSE.
*/

bool QDir::remove( const char *fileName, bool acceptAbsPath )
{
    if ( fileName == 0 || fileName[0] == '\0' ) {
#if defined(CHECK_NULL)
	warning( "QDir::remove: Empty or null file name" );
#endif
	return FALSE;
    }
    QString p = filePath( fileName, acceptAbsPath );
    return QFile::remove( p );
}

/*!
  Renames a file.

  If \e acceptAbsPaths is TRUE a path starting with a separator ('/')
  will rename the file with the absolute path, if \e acceptAbsPath is FALSE
  any number of separators at the beginning of \e name will be removed.

  Returns TRUE if successful, otherwise FALSE.

  On most file systems, rename() fails only if oldName does not exist
  or if \a newName and \a oldName are not on the same partition, but
  there are also other reasons why rename() can fail.  For example, on
  at least one file system rename() fails if newName points to an open
  file
*/

bool QDir::rename( const char *name, const char *newName,
		   bool acceptAbsPaths	)
{
    if ( name == 0 || name[0] == '\0' || newName == 0 || newName[0] == '\0' ) {
#if defined(CHECK_NULL)
	warning( "QDir::rename: Empty or null file name(s)" );
#endif
	return FALSE;
    }
    QString fn1 = filePath( name, acceptAbsPaths );
    QString fn2 = filePath( newName, acceptAbsPaths );
    return ::rename(fn1, fn2) == 0;
}

/*!
  Checks for existence of a file.

  If \e acceptAbsPaths is TRUE a path starting with a separator ('/')
  will check the file with the absolute path, if \e acceptAbsPath is FALSE
  any number of separators at the beginning of \e name will be removed.

  Returns TRUE if the file exists, otherwise FALSE.

  \sa QFileInfo::exists(), QFile::exists()
*/

bool QDir::exists( const char *name, bool acceptAbsPath )
{
    if ( name == 0 || name[0] == '\0' ) {
#if defined(CHECK_NULL)
	warning( "QDir::exists: Empty or null file name" );
#endif
	return FALSE;
    }
    QString tmp = filePath( name, acceptAbsPath );
    return QFile::exists( tmp.data() );
}

/*!
  Returns the native directory separator; '/' under UNIX and '\' under
  MS-DOS, Windows NT and OS/2.

  You do not need to use this function to build file paths. If you always
  use '/', Qt will translate your paths to conform to the underlying
  operating system.
*/

char QDir::separator()
{
#if defined( UNIX )
    return '/';
#elif defined (_OS_FATFS_)
    return '\\';
#elif defined (_OS_MAC_)
    return ':';
#else
    return '/';
#endif
}


/*!
  Sets the the current directory. Returns TRUE if successful.
*/

bool QDir::setCurrent( const char *path )
{
    if ( CHDIR(path) >= 0 )
	return TRUE;
    else
	return FALSE;
}

/*!
  Returns the current directory.
  \sa currentDirPath(), QDir::QDir()
*/

QDir QDir::current()
{
    return QDir( currentDirPath() );
}

/*!
  Returns the home directory.
  \sa homeDirPath()
*/

QDir QDir::home()
{
    return QDir( homeDirPath() );
}

/*!
  Returns the root directory.
  \sa rootDirPath() drives()
*/

QDir QDir::root()
{
    return QDir( rootDirPath() );
}


/*!
  Returns the absolute path of the current directory.
  \sa current()
*/

QString QDir::currentDirPath()
{
    static bool forcecwd = TRUE;
    static ino_t cINode;
    static dev_t cDevice;
    QString currentName( PATH_MAX );

    STATBUF st;

    if ( STAT( ".", &st ) == 0 ) {
	if ( forcecwd || cINode != st.st_ino || cDevice != st.st_dev ) {
	    if ( GETCWD( currentName.data(), PATH_MAX ) != 0 ) {
		cINode	 = st.st_ino;
		cDevice	 = st.st_dev;
		slashify( currentName.data() );
		// forcecwd = FALSE;   ### caching removed, not safe
	    } else {
		warning( "QDir::currentDirPath: getcwd() failed" );
		currentName = 0;
		forcecwd    = TRUE;
	    }
	}
    } else {
#if defined(DEBUG)
	debug( "QDir::currentDirPath: stat(\".\") failed" );
#endif
	currentName = 0;
	forcecwd    = TRUE;
    }
    return currentName.copy();
}

/*!
  Returns the absolute path for the user's home directory,
  \sa home()
*/

QString QDir::homeDirPath()
{
    QString d( PATH_MAX );
    d = getenv("HOME");
    slashify( d.data() );
    if ( d.isNull() )
	d = rootDirPath();
    return d;
}

/*!
  Returns the absolute path for the root directory ("/" under UNIX).

  \sa root() drives()
*/

QString QDir::rootDirPath()
{
#if defined(_OS_FATFS_)
    QString d( "c:/" );
#elif defined(_OS_OS2EMX_)
    char dir[4];
    _abspath( dir, "/", _MAX_PATH );
    QString d( dir );
#elif defined(UNIX)
    QString d( "/" );
#else
# error Not implemented
#endif
    return d;
}

/*!
  Returns TRUE if the \e fileName matches the wildcard \e filter.
  \sa QRegExp
*/

bool QDir::match( const char *filter, const char *fileName )
{
    QRegExp tmp( filter, TRUE, TRUE ); // case sensitive and wildcard mode on
    return tmp.match( fileName ) != -1;
}


/*!
  Removes all multiple directory separators ('/') and resolves
  any "." or ".." found in the path.

  Symbolic links are kept.  This function does not return the
  canonical path.
*/

QString QDir::cleanDirPath( const char *filePath )
{
    QString name = filePath;
    QString newPath;

    if ( name.isEmpty() )
	return name;

    slashify( name.data() );

    bool addedSeparator;
    if ( isRelativePath(name) ) {
	addedSeparator = TRUE;
	name.insert( 0, '/' );
    } else {
	addedSeparator = FALSE;
    }

    int ePos, pos, upLevel;

    pos = ePos = name.size() - 1;
    upLevel = 0;
    int len;

    while ( pos && (pos = name.findRev('/',--pos)) != -1 ) {
	len = ePos - pos - 1;
	if ( len == 2 && name.at(pos + 1) == '.'
		      && name.at(pos + 2) == '.' ) {
	    upLevel++;
	} else {
	    if ( len != 0 && (len != 1 || name.at(pos + 1) != '.') ) {
		if ( !upLevel )
		    newPath = "/" + name.mid(pos + 1, len) + newPath;
		else
		    upLevel--;
	    }
	}
	ePos = pos;
    }
    if ( addedSeparator ) {
	while ( upLevel-- )
	    newPath.insert( 0, "/.." );
	if ( !newPath.isEmpty() )
	    newPath.remove( 0, 1 );
	else
	    newPath = ".";
    } else {
	if ( newPath.isEmpty() )
	    newPath = "/";
#if defined(_OS_FATFS_) || defined(_OS_OS2EMX_)
	if ( name[0] == '/' ) {
	    if ( name[1] == '/' )		// "\\machine\x\ ..."
		newPath.insert( 0, '/' );
	} else {
	    newPath = name.left(2) + newPath;
	}
#endif
    }
    return newPath;
}

/*!
  Returns TRUE if the path is relative, FALSE if it is absolute.
  \sa isRelative()
*/

bool QDir::isRelativePath( const char *path )
{
    int len = strlen( path );
    if ( len == 0 )
	return TRUE;
#if defined(_OS_FATFS_) || defined(_OS_OS2EMX_)
    int i = 0;
    if ( isalpha(path[0]) && path[1] == ':' )		// drive, e.g. a:
	i = 2;
    return path[i] != '/' && path[i] != '\\';
#elif defined(UNIX)
    return path[0] != '/';
#else
# error Not implemented for this operating system
#endif
}


static void dirInSort( QStrList *fl, QFileInfoList *fil, const char *fileName,
		       const QFileInfo &fi, int sortSpec )
{
    QFileInfo *newFi = new QFileInfo( fi );
    CHECK_PTR( newFi );
    int sortBy = sortSpec & QDir::SortByMask;
    if ( sortBy == QDir::Unsorted ) {
	if ( sortSpec & QDir::Reversed ) {
	    fl ->insert( 0, fileName );
	    fil->insert( 0, newFi );
	} else {
	    fl ->append( fileName );
	    fil->append( newFi );
	}
	return;
    }

    char      *tmp1;
    QFileInfo *tmp2 = 0;
    tmp1 = ( sortSpec & QDir::Reversed ) ? fl->last() : fl->first();
    if ( sortBy != QDir::Name )
	tmp2 = ( sortSpec & QDir::Reversed ) ? fil->last() : fil->first();
    bool stop = FALSE;
    while ( tmp1 ) {
	switch( sortBy ) {
	    case QDir::Name:
		if ( sortSpec & QDir::IgnoreCase ) {
		    if ( stricmp(fileName,tmp1) < 0 )
			stop = TRUE;
		} else {
		    if ( strcmp(fileName,tmp1) < 0 )
			stop = TRUE;
		}
		break;
	    case QDir::Time:
		if ( fi.lastModified() > tmp2->lastModified() )
		    stop = TRUE;
		break;
	    case QDir::Size:
		if ( fi.size() > tmp2->size() )
		    stop = TRUE;
		break;
	}
	if (stop)
	    break;
	tmp1 = ( sortSpec & QDir::Reversed ) ? fl->prev() : fl->next();
	if ( sortBy != QDir::Name )
	    tmp2 = ( sortSpec & QDir::Reversed ) ? fil->prev() : fil->next();
    }
    int pos;
    if ( stop )
	pos = fl->at() + (( sortSpec & QDir::Reversed ) ? 1 : 0);
    else
	pos = ( sortSpec & QDir::Reversed ) ? 0 : fl->count();
    fl ->insert( pos, fileName );
    fil->insert( pos, newFi );
}


/*!
  \internal
  Reads directory entries.
*/

bool QDir::readDirEntries( const QString &nameFilter,
			   int filterSpec, int sortSpec )
{
    if ( !fList ) {
	fList  = new QStrList;
	CHECK_PTR( fList );
	fiList = new QFileInfoList;
	CHECK_PTR( fiList );
	fiList->setAutoDelete( TRUE );
    } else {
	fList->clear();
	fiList->clear();
    }

    bool doDirs	    = (filterSpec & Dirs)	!= 0;
    bool doFiles    = (filterSpec & Files)	!= 0;
    bool noSymLinks = (filterSpec & NoSymLinks) != 0;
    bool doReadable = (filterSpec & Readable)	!= 0;
    bool doWritable = (filterSpec & Writable)	!= 0;
    bool doExecable = (filterSpec & Executable) != 0;
    bool doHidden   = (filterSpec & Hidden)	!= 0;
#if !defined(UNIX)
    // show hidden files if the user asks explicitly for e.g. .*
    if ( !doHidden && !nameFilter.isEmpty() && nameFilter[0] == '.' )
	doHidden = TRUE;
    bool doModified = (filterSpec & Modified)	!= 0;
    bool doSystem   = (filterSpec & System)	!= 0;
#endif
    bool dirsFirst  = (sortSpec	  & DirsFirst)	!= 0;

    QStrList	  *dList  = 0;
    QFileInfoList *diList = 0;

    if ( dirsFirst ) {
	dList = new QStrList;
	CHECK_PTR( dList );
	diList	 = new QFileInfoList;
	CHECK_PTR( dList );
    }

#if defined(_OS_WIN32_) || defined(_OS_MSDOS_)

    QRegExp   wc( nameFilter, FALSE, TRUE );	// wild card, case insensitive
    bool      first = TRUE;
    QString   p = dPath.copy();
    int	      plen = p.length();
#if defined(_OS_WIN32_)
    HANDLE    ff;
    WIN32_FIND_DATA finfo;
#else
    long      ff;
    _finddata_t finfo;
#endif
    QFileInfo fi;

#undef	IS_SUBDIR
#undef	IS_RDONLY
#undef	IS_ARCH
#undef	IS_HIDDEN
#undef	IS_SYSTEM
#undef	FF_GETFIRST
#undef	FF_GETNEXT
#undef	FF_ERROR

#if defined(_OS_WIN32_)
#define IS_SUBDIR   FILE_ATTRIBUTE_DIRECTORY
#define IS_RDONLY   FILE_ATTRIBUTE_READONLY
#define IS_ARCH	    FILE_ATTRIBUTE_ARCHIVE
#define IS_HIDDEN   FILE_ATTRIBUTE_HIDDEN
#define IS_SYSTEM   FILE_ATTRIBUTE_SYSTEM
#define FF_GETFIRST FindFirstFile
#define FF_GETNEXT  FindNextFile
#define FF_ERROR    INVALID_HANDLE_VALUE
#else
#define IS_SUBDIR   _A_SUBDIR
#define IS_RDONLY   _A_RDONLY
#define IS_ARCH	    _A_ARCH
#define IS_HIDDEN   _A_HIDDEN
#define IS_SYSTEM   _A_SYSTEM
#define FF_GETFIRST _findfirst
#define FF_GETNEXT  _findnext
#define FF_ERROR    -1
#endif

    if ( plen == 0 ) {
#if defined(CHECK_NULL)
	warning( "QDir::readDirEntries: No directory name specified" );
#endif
	return FALSE;
    }
    if ( p.at(plen-1) != '/' && p.at(plen-1) != '\\' )
	p += '/';
    p += "*.*";

    ff = FF_GETFIRST( p.data(), &finfo );
    if ( ff == FF_ERROR ) {
#if defined(DEBUG)
	warning( "QDir::readDirEntries: Cannot read the directory: %s",
		 (const char *)dPath );
#endif
	return FALSE;
    }

    while ( TRUE ) {
	if ( first )
	    first = FALSE;
	else {
#if defined(_OS_WIN32_)
	    if ( !FF_GETNEXT(ff,&finfo) )
		break;
#else
	    if ( FF_GETNEXT(ff,&finfo) == -1 )
		break;
#endif
	}
#if defined(_OS_WIN32_)
	int  attrib = finfo.dwFileAttributes;
#else
	int  attrib = finfo.attrib;
#endif
	bool isDir	= (attrib & IS_SUBDIR) != 0;
	bool isFile	= !isDir;
	bool isSymLink	= FALSE;
	bool isReadable = TRUE;
	bool isWritable = (attrib & IS_RDONLY) == 0;
	bool isExecable = FALSE;
	bool isModified = (attrib & IS_ARCH)   != 0;
	bool isHidden	= (attrib & IS_HIDDEN) != 0;
	bool isSystem	= (attrib & IS_SYSTEM) != 0;

#if defined(_OS_WIN32_)
	const char *fname = finfo.cFileName;
#else
	const char *fname = finfo.name;
#endif
	if ( wc.match(fname) == -1 && !(allDirs && isDir) )
	    continue;

	QString name = fname;
	if ( doExecable ) {
	    QString ext = name.right(4).lower();
	    if ( ext == ".exe" || ext == ".com" || ext == ".bat" ||
		 ext == ".pif" || ext == ".cmd" )
		isExecable = TRUE;
	}

	if  ( (doDirs && isDir) || (doFiles && isFile) ) {
	    if ( noSymLinks && isSymLink )
		continue;
	    if ( (filterSpec & RWEMask) != 0 )
		if ( (doReadable && !isReadable) ||
		     (doWritable && !isWritable) ||
		     (doExecable && !isExecable) )
		    continue;
	    if ( doModified && !isModified )
		continue;
	    if ( !doHidden && isHidden )
		continue;
	    if ( !doSystem && isSystem )
		continue;
	    fi.setFile( *this, name );
	    if ( dirsFirst && isDir )
		dirInSort( dList, diList, name, fi, sortSpec );
	    else
		dirInSort( fList, fiList, name, fi, sortSpec );
	}
    }
#if defined(_OS_WIN32_)
    FindClose( ff );
#else
    _findclose( ff );
#endif

#undef	IS_SUBDIR
#undef	IS_RDONLY
#undef	IS_ARCH
#undef	IS_HIDDEN
#undef	IS_SYSTEM
#undef	FF_GETFIRST
#undef	FF_GETNEXT
#undef	FF_ERROR


#elif defined(UNIX)

#if defined(_OS_OS2EMX_)
    QRegExp   wc( nameFilter, FALSE, TRUE );	// wild card, case insensitive
#else
    QRegExp   wc( nameFilter, TRUE, TRUE );	// wild card, case sensitive
#endif
    QFileInfo fi;
    DIR	     *dir;
    dirent   *file;

    dir = opendir( dPath );
    if ( !dir ) {
#if defined(CHECK_NULL)
	warning( "QDir::readDirEntries: Cannot read the directory: %s",
		 (const char *)dPath );
#endif
	return FALSE;
    }

    while ( (file = readdir(dir)) ) {
	fi.setFile( *this, file->d_name );
	if ( wc.match(file->d_name) == -1 && !(allDirs && fi.isDir()) )
	    continue;
	if  ( (doDirs && fi.isDir()) || (doFiles && fi.isFile()) ) {
	    if ( noSymLinks && fi.isSymLink() )
		continue;
	    if ( (filterSpec & RWEMask) != 0 )
		if ( (doReadable && !fi.isReadable()) ||
		     (doWritable && !fi.isWritable()) ||
		     (doExecable && !fi.isExecutable()) )
		    continue;
	    if ( !doHidden && (file->d_name[0] == '.') &&
		 (file->d_name[1] != '\0') &&
		 (file->d_name[1] != '.' || file->d_name[2] != '\0') )
		continue;
	    if ( dirsFirst && fi.isDir() )
		dirInSort( dList, diList , file->d_name, fi, sortSpec );
	    else
		dirInSort( fList, fiList, file->d_name, fi, sortSpec );
	}
    }
    if ( closedir(dir) != 0 ) {
#if defined(CHECK_NULL)
	warning( "QDir::readDirEntries: Cannot close the directory: %s",
		 (const char *)dPath );
#endif
    }

#endif // UNIX

    if ( dirsFirst ) {
	char	  *tmp	 = dList ->last();
	QFileInfo *fiTmp = diList->last();
	while ( tmp ) {
	    fList->insert( 0, tmp );
	    tmp = dList->prev();
	    fiList->insert( 0, fiTmp );
	    fiTmp = diList->prev();
	}
	delete dList;
	delete diList;
    }
    if ( filterSpec == (FilterSpec)filtS && sortSpec == (SortSpec)sortS &&
	 nameFilter == nameFilt )
	dirty = FALSE;
    else
	dirty = TRUE;
    return TRUE;
}


/*!  Returns a list if the root directories on this system.  On
  win32, this returns a number of QFileInfo objects containing "C:/",
  "D:/" etc.  On other operating systems, it returns a list containing
  just one root directory (e.g. "/").
*/

const QFileInfoList * QDir::drives()
{
    // at most one instance of QFileInfoList is leaked, and this variable
    // points to that list
    static QFileInfoList * knownMemoryLeak = 0;

    if ( !knownMemoryLeak ) {
	knownMemoryLeak = new QFileInfoList;

#if defined(_OS_WIN32_)

	Q_UINT32 driveBits = (Q_UINT32) GetLogicalDrives() & 0x3ffffff;
#elif defined(_OS_OS2EMX_)
	Q_UINT32 driveBits, cur;
	if (DosQueryCurrentDisk(&cur,&driveBits) != NO_ERROR)
	    exit(1);
	driveBits &= 0x3ffffff;
#endif
#if defined(_OS_WIN32_) || defined(_OS_OS2EMX_)
	char driveName[4];
	qstrcpy( driveName, "a:/" );
	while( driveBits ) {
	    if ( driveBits & 1 )
		knownMemoryLeak->append( new QFileInfo( driveName ) );
	    driveName[0]++;
	    driveBits = driveBits >> 1;
	}

#else

	// non-win32 versions both use just one root directory
	knownMemoryLeak->append( new QFileInfo( rootDirPath() ) );

#endif
    }

    return knownMemoryLeak;
}
