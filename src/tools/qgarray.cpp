/****************************************************************************
** $Id: qgarray.cpp,v 2.7 1998/07/03 00:09:44 hanord Exp $
**
** Implementation of QGArray class
**
** Created : 930906
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

#define	 QGARRAY_CPP
#include "qgarray.h"
#include "qstring.h"
#include <stdlib.h>

#define USE_MALLOC				// comment to use new/delete

#undef NEW
#undef DELETE

#if defined(USE_MALLOC)
#define NEW(type,size)	((type*)malloc(size*sizeof(type)))
#define DELETE(array)	(free((char*)array))
#else
#define NEW(type,size)	(new type[size])
#define DELETE(array)	(delete[] array)
#define DONT_USE_REALLOC			// comment to use realloc()
#endif


/*!
  \class QShared qshared.h
  \brief The QShared struct is internally used for implementing shared classes.

  It only contains a reference count and member functions to increment and
  decrement it.

  Shared classes normally have internal classes that inherit QShared and
  add the shared data.

  \sa \link shclass.html Shared Classes\endlink
*/


/*!
  \class QGArray qgarray.h
  \brief The QGArray class is an internal class for implementing the QArray class.

  QGArray is a strictly internal class that acts as base class for the
  QArray template array.

  It contains an array of bytes and has no notion of an array element.
*/


/*!
  \internal
  Constructs a null array.
*/

QGArray::QGArray()
{
    shd = newData();
    CHECK_PTR( shd );
}

/*!
  \internal
  Dummy constructor; does not allocate any data.

  This constructor does not initialize any array data so subclasses
  must do it. The intention is to make the code more efficient.
*/

QGArray::QGArray( int, int )
{
}

/*!
  \internal
  Constructs an array with room for \e size bytes.
*/

QGArray::QGArray( int size )
{
    if ( size < 0 ) {
#if defined(CHECK_RANGE)
	warning( "QGArray: Cannot allocate array with negative length" );
#endif
	size = 0;
    }
    shd = newData();
    CHECK_PTR( shd );
    if ( size == 0 )				// zero length
	return;
    shd->data = NEW(char,size);
    CHECK_PTR( shd->data );
    shd->len = size;
}

/*!
  \internal
  Constructs a shallow copy of \e a.
*/

QGArray::QGArray( const QGArray &a )
{
    shd = a.shd;
    shd->ref();
}

/*!
  \internal
  Dereferences the array data and deletes it if this was the last
  reference.
*/

QGArray::~QGArray()
{
    if ( shd && shd->deref() ) {		// delete when last reference
	if ( shd->data )			// is lost
	    DELETE(shd->data);
	deleteData( shd );
    }
}


/*!
  \fn QGArray &QGArray::operator=( const QGArray &a )
  \internal
  Assigns a shallow copy of \e a to this array and returns a reference to
  this array.  Equivalent to assign().
*/

/*!
  \fn void QGArray::detach()
  \internal
  Detaches this array from shared array data.
*/

/*!
  \fn char *QGArray::data() const
  \internal
  Returns a pointer to the actual array data.
*/

/*!
  \fn uint QGArray::nrefs() const
  \internal
  Returns the reference count.
*/

/*!
  \fn uint QGArray::size() const
  \internal
  Returns the size of the array, in bytes.
*/


/*!
  \internal
  Returns TRUE if this array is equal to \e a, otherwise FALSE.
  The comparison is bitwise, of course.
*/

bool QGArray::isEqual( const QGArray &a ) const
{
    if ( size() != a.size() )			// different size
	return FALSE;
    if ( data() == a.data() )			// has same data
	return TRUE;
    return (size() ? memcmp( data(), a.data(), size() ) : 0) == 0;
}


/*!
  \internal
  Resizes the array to \e newsize bytes.
*/

bool QGArray::resize( uint newsize )
{
    if ( newsize == shd->len )			// nothing to do
	return TRUE;
    if ( newsize == 0 ) {			// remove array
	duplicate( 0, 0 );
	return TRUE;
    }
    if ( shd->data ) {				// existing data
#if defined(DONT_USE_REALLOC)
	char *newdata = NEW(char,newsize);	// manual realloc
	memcpy( newdata, shd->data, QMIN(shd->len,newsize) );
	DELETE(shd->data);
	shd->data = newdata;
#else
	shd->data = (char *)realloc( shd->data, newsize );
#endif
    } else {
	shd->data = NEW(char,newsize);
    }
    CHECK_PTR( shd->data );
    if ( !shd->data )				// no memory
	return FALSE;
    shd->len = newsize;
    return TRUE;
}

/*!
  \internal
  Fills the array with the repeated occurrences of \e d, which is
  \e sz bytes long.
  If \e len is specified as different from -1, then the array will be
  resized to \e len*sz before it is filled.

  Returns TRUE if successful, or FALSE if the memory cannot be allocated
  (only when \e len != -1).

  \sa resize()
*/

bool QGArray::fill( const char *d, int len, uint sz )
{
    if ( len < 0 )
	len = shd->len/sz;			// default: use array length
    else if ( !resize( len*sz ) )
	return FALSE;
    if ( sz == 1 )				// 8 bit elements
	memset( data(), *d, len );
    else if ( sz == 4 ) {			// 32 bit elements
	register Q_INT32 *x = (Q_INT32*)data();
	Q_INT32 v = *((Q_INT32*)d);
	while ( len-- )
	    *x++ = v;
    } else if ( sz == 2 ) {			// 16 bit elements
	register Q_INT16 *x = (Q_INT16*)data();
	INT16 v = *((Q_INT16*)d);
	while ( len-- )
	    *x++ = v;
    } else {					// any other size elements
	register char *x = data();
	while ( len-- ) {			// more complicated
	    memcpy( x, d, sz );
	    x += sz;
	}
    }
    return TRUE;
}

/*!
  \internal
  Shallow copy. Dereference the current array and references the data
  contained in \e a instead. Returns a reference to this array.
  \sa operator=()
*/

QGArray &QGArray::assign( const QGArray &a )
{
    a.shd->ref();				// avoid 'a = a'
    if ( shd->deref() ) {			// delete when last reference
	if ( shd->data )			// is lost
	    DELETE(shd->data);
	deleteData( shd );
    }
    shd = a.shd;
    return *this;
}

/*!
  \internal
  Shallow copy. Dereference the current array and references the
  array data \e d, which contains \e len bytes.
  Returns a reference to this array.

  Do not delete \e d later, because QGArray takes care of that.
*/

QGArray &QGArray::assign( const char *d, uint len )
{
    if ( shd->count > 1 ) {			// disconnect this
	shd->count--;
	shd = newData();
	CHECK_PTR( shd );
    } else {
	if ( shd->data )
	    DELETE(shd->data);
    }
    shd->data = (char *)d;
    shd->len = len;
    return *this;
}

/*!
  \internal
  Deep copy. Dereference the current array and obtains a copy of the data
  contained in \e a instead. Returns a reference to this array.
  \sa assign(), operator=()
*/

QGArray &QGArray::duplicate( const QGArray &a )
{
    if ( a.shd == shd ) {			// a.duplicate(a) !
	if ( shd->count > 1 ) {
	    shd->count--;
	    register array_data *n = newData();
	    CHECK_PTR( n );
	    if ( (n->len=shd->len) ) {
		n->data = NEW(char,n->len);
		CHECK_PTR( n->data );
		if ( n->data )
		    memcpy( n->data, shd->data, n->len );
	    } else {
		n->data = 0;
	    }
	    shd = n;
	}
	return *this;
    }
    char *oldptr = 0;
    if ( shd->count > 1 ) {			// disconnect this
	shd->count--;
	shd = newData();
	CHECK_PTR( shd );
    } else {					// delete after copy was made
	oldptr = shd->data;
    }
    if ( a.shd->len ) {				// duplicate data
	shd->data = NEW(char,a.shd->len);
	CHECK_PTR( shd->data );
	if ( shd->data )
	    memcpy( shd->data, a.shd->data, a.shd->len );
    } else {
	shd->data = 0;
    }
    shd->len = a.shd->len;
    if ( oldptr )
	DELETE(oldptr);
    return *this;
}

/*!
  \internal
  Deep copy. Dereferences the current array and obtains a copy of the
  array data \e d instead.  Returns a reference to this array.
  \sa assign(), operator=()
*/

QGArray &QGArray::duplicate( const char *d, uint len )
{
    char *data;
    if ( d == 0 || len == 0 ) {
	data = 0;
	len  = 0;
    } else {
	if ( shd->count == 1 && shd->len == len ) {
	    memcpy( shd->data, d, len );	// use same buffer
	    return *this;
	}
	data = NEW(char,len);
	CHECK_PTR( data );
	memcpy( data, d, len );
    }
    if ( shd->count > 1 ) {			// detach
	shd->count--;
	shd = newData();
	CHECK_PTR( shd );
    } else {					// just a single reference
	if ( shd->data )
	    DELETE(shd->data);
    }
    shd->data = data;
    shd->len  = len;
    return *this;
}

/*!
  \internal
  Resizes this array to \e len bytes and copies the \e len bytes at
  address \e into it.

  \warning This function disregards the reference count mechanism.  If
  other QGArrays reference the same data as this, all will be updated.
*/

void QGArray::store( const char *d, uint len )
{						// store, but not deref
    resize( len );
    memcpy( shd->data, d, len );
}


/*!
  \fn array_data *QGArray::sharedBlock() const
  \internal
  Returns a pointer to the shared array block.
*/

/*!
  \fn void QGArray::setSharedBlock( array_data *p )
  \internal
  Sets the shared array block to \e p.
*/


/*!
  \internal
  Sets raw data and returns a reference to the array.

  Dereferences the current array and sets the new array data to \e d and
  the new array size to \e len.	 Do not attempt to resize or re-assign the
  array data when raw data has been set.
  Call resetRawData(d,len) to reset the array.

  Setting raw data is useful because it set QArray data without allocating
  memory or copying data.

  Example of intended use:
  \code
    static uchar bindata[] = { 231, 1, 44, ... };
    QByteArray	a;
    a.setRawData( bindata, sizeof(bindata) );	// a points to bindata
    QDataStream s( a, IO_ReadOnly );		// open on a's data
    s >> <something>;				// read raw bindata
    s.close();
    a.resetRawData( bindata, sizeof(bindata) ); // finished
  \endcode

  Example of misuse (do not do this):
  \code
    static uchar bindata[] = { 231, 1, 44, ... };
    QByteArray	a, b;
    a.setRawData( bindata, sizeof(bindata) );	// a points to bindata
    a.resize( 8 );				// will crash
    b = a;					// will crash
    a[2] = 123;					// might crash
      // forget to resetRawData - will crash
  \endcode

  \warning If you do not call resetRawData(), QGArray will attempt to
  deallocate or reallocate the raw data, which might not be too good.
  Be careful.
*/

QGArray &QGArray::setRawData( const char *d, uint len )
{
    duplicate( 0, 0 );				// set null data
    shd->data = (char *)d;
    shd->len  = len;
    return *this;
}

/*!
  \internal
  Resets raw data.

  The arguments must be the data and length that were passed to
  setRawData().	 This is for consistency checking.
*/

void QGArray::resetRawData( const char *d, uint len )
{
    if ( d != shd->data || len != shd->len ) {
#if defined(CHECK_STATE)
	warning( "QGArray::resetRawData: Inconsistent arguments" );
#endif
	return;
    }
    shd->data = 0;
    shd->len  = 0;
}


/*!
  \internal
  Finds the first occurrence of \e d in the array from position \e index,
  where \e sz is the size of the \e d element.

  Note that \e index is given in units of \e sz, not bytes.

  This function only compares whole cells, not bytes.
*/

int QGArray::find( const char *d, uint index, uint sz ) const
{
    index *= sz;
    if ( index >= shd->len ) {
#if defined(CHECK_RANGE)
	warning( "QGArray::find: Index %d out of range", index/sz );
#endif
	return -1;
    }
    register uint i;
    uint ii;
    switch ( sz ) {
	case 1: {				// 8 bit elements
	    register char *x = data();
	    char v = *d;
	    for ( i=index; i<shd->len; i++ ) {
		if ( *x++ == v )
		    break;
	    }
	    ii = i;
	    }
	    break;
	case 2: {				// 16 bit elements
	    register Q_INT16 *x = (INT16*)(data() + index);
	    Q_INT16 v = *((Q_INT16*)d);
	    for ( i=index; i<shd->len; i+=2 ) {
		if ( *x++ == v )
		    break;
	    }
	    ii = i/2;
	    }
	    break;
	case 4: {				// 32 bit elements
	    register Q_INT32 *x = (Q_INT32*)(data() + index);
	    Q_INT32 v = *((Q_INT32*)d);
	    for ( i=index; i<shd->len; i+=4 ) {
		if ( *x++ == v )
		    break;
	    }
	    ii = i/4;
	    }
	    break;
	default: {				// any size elements
	    for ( i=index; i<shd->len; i+=sz ) {
		if ( memcmp( d, &shd->data[i], sz ) == 0 )
		    break;
	    }
	    ii = i/sz;
	    }
	    break;
    }
    return i<shd->len ? (int)ii : -1;
}

/*!
  \internal
  Returns the number of occurrences of \e d in the array, where \e sz is
  the size of the \e d element.

  This function only compares whole cells, not bytes.
*/

int QGArray::contains( const char *d, uint sz ) const
{
    register uint i = shd->len;
    int count = 0;
    switch ( sz ) {
	case 1: {				// 8 bit elements
	    register char *x = data();
	    char v = *d;
	    while ( i-- ) {
		if ( *x++ == v )
		    count++;
	    }
	    }
	    break;
	case 2: {				// 16 bit elements
	    register Q_INT16 *x = (Q_INT16*)data();
	    Q_INT16 v = *((Q_INT16*)d);
	    i /= 2;
	    while ( i-- ) {
		if ( *x++ == v )
		    count++;
	    }
	    }
	    break;
	case 4: {				// 32 bit elements
	    register Q_INT32 *x = (Q_INT32*)data();
	    Q_INT32 v = *((Q_INT32*)d);
	    i /= 4;
	    while ( i-- ) {
		if ( *x++ == v )
		    count++;
	    }
	    }
	    break;
	default: {				// any size elements
	    for ( i=0; i<shd->len; i+=sz ) {
		if ( memcmp(d, &shd->data[i], sz) == 0 )
		    count++;
	    }
	    }
	    break;
    }
    return count;
}

/*!
  \fn char *QGArray::at( uint index ) const
  \internal
  Returns a pointer to the byte at offset \e index in the array.
*/

/*!
  \internal
  Expand the array if necessary, and copies (the first part of) its
  contents from the \e index*zx bytes at \e d.

  Returns TRUE if the operation succeeds, FALSE if it runs out of
  memory.

  \warning This function disregards the reference count mechanism.  If
  other QGArrays reference the same data as this, all will be changed.
*/

bool QGArray::setExpand( uint index, const char *d, uint sz )
{
    index *= sz;
    if ( index >= shd->len ) {
	if ( !resize( index+sz ) )		// no memory
	    return FALSE;
    }
    memcpy( data() + index, d, sz );
    return TRUE;
}


/*!
  \fn array_data *QGArray::newData()
  \internal
  Returns a new shared array block.
*/

/*!
  \fn void QGArray::deleteData( array_data *p )
  \internal
  Deletes the shared array block.
*/


/*!
  \internal
  Prints a warning message if at() or [] is given a bad index.
*/

void QGArray::msg_index( uint index )
{
#if defined(CHECK_RANGE)
    warning( "QGArray::at: Absolute index %d out of range", index );
#endif
}
