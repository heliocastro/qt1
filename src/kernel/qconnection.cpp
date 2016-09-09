/****************************************************************************
** $Id: qconnection.cpp,v 2.6 1998/07/03 00:09:31 hanord Exp $
**
** Implementation of QConnection class
**
** Created : 930417
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

#include "qconnection.h"

/*!
  \class QConnection qconnection.h

  \brief The QConnection class is an internal class, used in the
  signal/slot mechanism.

  Do not use this class directly in application programs.

  \internal
  QObject has a list of QConnection for each signal that is connected to the
  outside world.
*/

/*!
  \internal
*/
QConnection::QConnection( const QObject *object, QMember member,
			  const char *memberName )
{
    obj = (QObject *)object;
    mbr = member;
    mbr_name = memberName;
    nargs = 0;
    if ( strstr(memberName,"()") == 0 ) {
        const char *p = memberName;
	nargs++;
	while ( *p ) {
	    if ( *p++ == ',' )
		nargs++;
	}
    }
}

/*!
 \fn QConnection::~QConnection()
 \internal
*/

/*!
  \fn bool QConnection::isConnected() const
  \internal
*/

/*!
  \fn QObject *QConnection::object() const
  \internal
*/

/*!
  \fn QMember *QConnection::member() const
  \internal
*/

/*!
  \fn const char *QConnection::memberName() const
  \internal
*/

/*!
  \fn int QConnection::numArgs() const
  \internal
*/
