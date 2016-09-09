/****************************************************************************
** $Id: qcollection.cpp,v 2.6.2.1 1998/08/12 16:55:12 agulbra Exp $
**
** Implementation of base class for all collection classes
**
** Created : 920820
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

#include "qcollection.h"

/*!
  \class QCollection qcollection.h
  \brief The QCollection class is the base class of all Qt collections.

  \ingroup collection
  \ingroup tools

  The QCollection class is an abstract base class for the Qt \link
  collection.html collection classes\endlink QDict, QList etc. via QGDict,
  QGList etc.

  A QCollection knows only about the number of objects in the collection and
  the \link setAutoDelete() deletion strategy\endlink.

  A collection is implemented using the \c GCI (generic collection item)
  type, which is a \c void*.  The template (or macro) classes that
  create the real collections cast the \c GCI to the required type.

  \sa \link collection.html Collection Classes\endlink
*/


/*!
  \fn QCollection::QCollection()

  Constructs a collection. The constructor is protected because
  QCollection is an abstract class.
*/

/*!
  \fn QCollection::QCollection( const QCollection & source )

  Constructs a copy of \a source with autoDelete() set to FALSE. The
  constructor is protected because QCollection is an abstract class.

  Note that if \a source has autoDelete turned on, copying it is a
  good way to get memory leaks, reading freed memory, or both.
*/

/*!
  \fn QCollection::~QCollection()
  Destroys the collection. The destructor is protected because QCollection
  is an abstract class.
*/


/*!
  \fn bool QCollection::autoDelete() const
  Returns the setting of the auto-delete option (default is FALSE).
  \sa setAutoDelete()
*/

/*!
  \fn void QCollection::setAutoDelete( bool enable )
  Sets the auto-delete option of the collection.

  Enabling auto-delete (\e enable is TRUE) will delete objects that
  are removed from the collection.  This can be useful if the
  collection has the only reference to the objects.  (Note that the
  object can still be copied using the copy constructor - copying such
  objects is a good way to get memory leaks, reading freed memory or
  both.)

  Disabling auto-delete (\e enable is FALSE) will \e not delete objects
  that are removed from the collection.	 This is useful if the objects
  are part of many collections.

  The default setting is FALSE.

  \sa autoDelete()
*/


/*!
  \fn virtual uint QCollection::count() const
  Returns the number of objects in the collection.
*/

/*!
  \fn virtual void QCollection::clear()
  Removes all objects from the collection.  The objects will be deleted
  if auto-delete has been enabled.
  \sa setAutoDelete()
*/


/*!
  Virtual function that creates a copy of an object that is about to
  be inserted into the collection.

  The default implementation returns the \e d pointer, i.e. no copy
  is made.

  This function is seldom reimplemented in the collection template
  classes. It is not common practice to make a copy of something
  that is being inserted.

  \sa deleteItem()
*/

GCI QCollection::newItem( GCI d )
{
    return d;					// just return reference
}

/*!
  Virtual function that deletes an item that is about to be removed from
  the collection.

  The default implementation deletes \e d pointer if and only if
  auto-delete has been enabled.

  This function is always reimplemented in the collection template
  classes.

  \sa newItem(), setAutoDelete()
*/

void QCollection::deleteItem( GCI d )
{
    if ( del_item )
	delete d;				// default operation
}
