/****************************************************************************
** $Id: qsignalmapper.cpp,v 1.5.2.2 1999/01/13 19:11:42 ettrich Exp $
**
** Implementation of QSignalMapper class
**
** Created : 980503
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

#include "qsignalmapper.h"
#include "qptrdict.h"

struct QSignalMapperRec {
    QSignalMapperRec()
    {
	has_int = 0;
	str_id = 0;
    }

    uint has_int:1;

    int int_id;
    const char* str_id;
    // extendable to other types of identification
};

class QSignalMapperData {
public:
    QSignalMapperData()
    {
	dict.setAutoDelete( TRUE );
    }

    QPtrDict<QSignalMapperRec> dict;
};

/*!
  \class QSignalMapper qsignalmapper.h
  \brief A QSignalMapper object bundles signals from identifiable senders.

  Collects a set of parameterless signals, re-emitting them with an
  integer or string parameters corresponding to the object which sent the
  signal.
*/

/*!
  Constructs a QSignalMapper.  Like all QObjects, it will be deleted when the
  parent is deleted.
*/
QSignalMapper::QSignalMapper( QObject* parent, const char* name ) :
    QObject( parent, name )
{
    d = new QSignalMapperData;
}

/*!
  Destructs the QSignalMapper.
*/
QSignalMapper::~QSignalMapper()
{
    delete d;
}

/*!
  Adds a mapping such that when map() is signalled from the given
  sender, the signal mapper(identifier) is emitted.

  There may be at most one integer identifier for each object.
*/
void QSignalMapper::setMapping( const QObject* sender, int identifier )
{
    QSignalMapperRec* rec = getRec(sender);
    rec->int_id = identifier;
    rec->has_int = 1;
}

/*!
  Adds a mapping such that when map() is signalled from the given
  sender, the signal mapped(identifier) is emitted.

  There may be at most one string identifier for each object, and
  it may not be null.
*/
void QSignalMapper::setMapping( const QObject* sender, const char* identifier )
{
    QSignalMapperRec* rec = getRec(sender);
    rec->str_id = identifier;
}

/*!
  Removes all mappings for \a sender.  This is done automatically
  when mapped objects are destroyed.
*/
void QSignalMapper::removeMappings( const QObject* sender )
{
    d->dict.remove((void*)sender);
}

void QSignalMapper::removeMapping()
{
    removeMappings(sender());
}

/*!
  This slot emits signals based on which object sends signals
  to it.
*/
void QSignalMapper::map()
{
    const QObject* s = sender();
    QSignalMapperRec* rec = d->dict.find( (void*)s );
    if ( rec ) {
	if ( rec->has_int )
	    emit mapped( rec->int_id );
	if ( rec->str_id )
	    emit mapped( rec->str_id );
    }
}

QSignalMapperRec* QSignalMapper::getRec( const QObject* sender )
{
    QSignalMapperRec* rec = d->dict.find( (void*)sender );
    if (!rec) {
	rec = new QSignalMapperRec;
	d->dict.insert( (void*)sender, rec );
	connect( sender, SIGNAL(destroyed()), this, SLOT(removeMapping()) );
    }
    return rec;
}

/*!
  \fn void QSignalMapper::mapped(int)

  This signal is emitted when map() is signalled from an object which
  has an integer mapping set.

  \sa setMapping(int)
*/

/*!
  \fn void QSignalMapper::mapped(const char*)


  This signal is emitted when map() is signalled from an object which
  has a string mapping set.

  \sa setMapping(const char*)
*/
