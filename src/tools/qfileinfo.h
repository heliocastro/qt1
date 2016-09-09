/****************************************************************************
** $Id: qfileinfo.h,v 2.4.2.1 1998/08/19 16:02:36 agulbra Exp $
**
** Definition of QFileInfo class
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

#ifndef QFILEINFO_H
#define QFILEINFO_H

#ifndef QT_H
#include "qfile.h"
#include "qdatetime.h"
#endif // QT_H

class QDir;
struct QFileInfoCache;


class Q_EXPORT QFileInfo				   // file information class
{
public:
    enum PermissionSpec {
	ReadUser  = 0400, WriteUser  = 0200, ExeUser  = 0100,
	ReadGroup = 0040, WriteGroup = 0020, ExeGroup = 0010,
	ReadOther = 0004, WriteOther = 0002, ExeOther = 0001 };

    QFileInfo();
    QFileInfo( const char *file );
    QFileInfo( const QFile & );
    QFileInfo( const QDir &, const char *fileName );
    QFileInfo( const QFileInfo & );
   ~QFileInfo();

    QFileInfo  &operator=( const QFileInfo & );

    void	setFile( const char *file );
    void	setFile( const QFile & );
    void	setFile( const QDir &, const char *fileName );

    bool	exists()	const;
    void	refresh()	const;
    bool	caching()	const;
    void	setCaching( bool );

    const char *filePath()	const;
    QString	fileName()	const;
    QString	absFilePath()	const;
    QString	baseName()	const;
    QString	extension()	const;

    QString	dirPath( bool absPath = FALSE ) const;
    QDir	dir( bool absPath = FALSE )	const;

    bool	isReadable()	const;
    bool	isWritable()	const;
    bool	isExecutable()	const;

    bool	isRelative()	const;
    bool	convertToAbs();

    bool	isFile()	const;
    bool	isDir()		const;
    bool	isSymLink()	const;

    QString	readLink()	const;

    const char *owner()		const;
    uint	ownerId()	const;
    const char *group()		const;
    uint	groupId()	const;

    bool	permission( int permissionSpec ) const;

    uint	size()		const;

    QDateTime	lastModified()	const;
    QDateTime	lastRead()	const;

private:
    void	doStat() const;
    QString	fn;
    QFileInfoCache *fic;
    bool	cache;
};


inline bool QFileInfo::caching() const
{
    return cache;
}


#endif // QFILEINFO_H
