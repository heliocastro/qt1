/****************************************************************************
** $Id: qvalidator.cpp,v 2.21 1998/07/03 00:09:54 hanord Exp $
**
** Implementation of validator classes.
**
** Created : 970610
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

#include "qvalidator.h"
#include "qwidget.h"
#include "qregexp.h"

#include <limits.h> // *_MIN, *_MAX
#include <ctype.h> // isdigit

/*!
  \class QValidator qvalidator.h

  \brief The QValidator class provides validation of input text.

  \ingroup misc

  The class itself is abstract; two subclasses provide rudimentary
  numeric range checking.

  The class includes two virtual functions, validate() and fixup().

  validate() is pure virtual, so it must be implemented by every
  subclass.  It returns Invalid, Valid or Acceptable depending on
  whether its argument is valid (for the class' definition of valid).

  fixup() is provided for validators that can repair some or all user
  errors.  The default does nothing.  QLineEdit, for example, will
  call fixup() if the user presses Return and the content is not
  currently valid, in case fixup() can do magic.

  QValidator is generally used with QLineEdit, QSpinBox and QComboBox.
*/

/*!
  Sets up the internal data structures used by the validator.  At
  the moment there aren't any.
*/

QValidator::QValidator( QWidget * parent, const char * name )
    : QObject( parent, name )
{
}


/*!
  Deletes the validator and frees any storage and other resources
  used.
*/

QValidator::~QValidator()
{
}


/*!
  \fn QValidator::State QValidator::validate( QString & input, int & pos )

  This pure virtual function returns \c Invalid if \a input is invalid
  according to this validator's rules, \c Valid if it is likely that a
  little more editing will make the input acceptable (e.g. the user
  types '4' into a widget which accepts 10-99) and \c Acceptable if
  the input is completely acceptable.

  The function can change \a input and \a pos (the cursor position) if
  it wants to.
*/


/*!
  Attempts to change \a to be valid according to this validator's
  rules.  Need not result in a valid string - callers of this function
  must re-test afterwards.  The default does nothing.

  Reimplementation notes:

  Note that \a input may not be the only QString object referencing
  this string, so it's almost always necessary to detach() the string
  at the start of the code:

  \code
    input.detach();
  \endcode

  You can change \a input even if you aren't able to produce a valid
  string.  For example an ISBN validator might want to delete every
  character except digits and "-", even if the result is not a valid
  ISBN, and a last-name validator might want to remove white space
  from the start and end of the string, even if the resulting string
  is not in the list of known last names.
*/

void QValidator::fixup( QString & input )
{
    NOT_USED(input);
}


/*!
  \class QIntValidator qvalidator.h

  \brief The QIntValidator class provides range checking of integers.

  \ingroup misc

  QIntValidator provides a lower and an upper bound.  It does not
  provide a fixup() function.

  \sa QDoubleValidator
*/


/*!
  Creates a validator object which accepts all integers.
*/

QIntValidator::QIntValidator( QWidget * parent, const char * name )
    : QValidator( parent, name )
{
    b = INT_MIN;
    t = INT_MAX;
}


/*!
  Creates a validator object which accepts all integers from \a
  bottom up to and including \a top.
*/

QIntValidator::QIntValidator( int bottom, int top,
			      QWidget * parent, const char * name )
    : QValidator( parent, name )
{
    b = bottom;
    t = top;
}


/*!
  Deletes the validator and frees up any storage used.
*/

QIntValidator::~QIntValidator()
{
    // nothing
}


/*!
  Returns \a Acceptable if \a input contains a number in the legal
  range, \a Valid if it contains another integer or is empty, and \a
  Invalid if \a input is not an integer.
*/

QValidator::State QIntValidator::validate( QString & input, int & )
{
    QRegExp empty( "^ *-? *$" );
    if ( empty.match( input ) >= 0 )
	return QValidator::Valid;
    bool ok;
    long int tmp = input.toLong( &ok );
    if ( !ok )
	return QValidator::Invalid;
    else if ( tmp < b || tmp > t )
	return QValidator::Valid;
    else
	return QValidator::Acceptable;
}


/*!
  Sets the validator to accept only number from \a bottom up to an
  including \a top.
*/

void QIntValidator::setRange( int bottom, int top )
{
    b = bottom;
    t = top;
}


/*!
  \fn int QIntValidator::bottom() const

  Returns the lowest valid number according to this validator.

  \sa top() setRange()
*/


/*!
  \fn int QIntValidator::top() const

  Returns the highest valid number according to this validator.

  \sa bottom() setRange()
*/


/*!
  \class QDoubleValidator qvalidator.h

  \brief The QDoubleValidator class provides range checking of
  floating-point numbers.

  \ingroup misc

  QDoubleValidator provides an upper bound, a lower bound, and a limit
  on the number of digits after the decimal point.  It does not
  provide a fixup() function.

  \sa QIntValidator
*/

/*!
  Creates a validator object which accepts all double from 2.7182818
  to 3.1415926 (please, no bug reports) with at most seven digits after
  the decimal point.

  This constructor is not meant to be useful; it is provided for
  completeness.
*/

QDoubleValidator::QDoubleValidator( QWidget * parent, const char * name )
    : QValidator( parent, name )
{
    b = 2.7182818;
    t = 3.1415926;
    d = 7;
}


/*!
  Creates a validator object which accepts all doubles from \a
  bottom up to and including \a top with at most \a decimals digits
  after the decimal point.
*/

QDoubleValidator::QDoubleValidator( double bottom, double top, int decimals,
				    QWidget * parent, const char * name )
    : QValidator( parent, name )
{
    b = bottom;
    t = top;
    d = decimals;
}


/*!
  Deletes the validator and frees any storage and other resources
  used.
*/

QDoubleValidator::~QDoubleValidator()
{
    // nothing
}


/*!
  Returns \a Acceptable if \a input contains a number in the legal
  range and format, \a Valid if it contains another number, a number
  with too many digits after the decimal point or is empty, and \a
  Invalid if \a input is not a number.
*/

QValidator::State QDoubleValidator::validate( QString & input, int & )
{
    QRegExp empty( "^ *-? *$" );
    if ( empty.match( input ) >= 0 )
	return QValidator::Valid;
    bool ok = TRUE;
    double tmp = input.toDouble( &ok );
    if ( !ok )
	return QValidator::Invalid;

    int i = input.find( '.' );
    if ( i >= 0 ) {
	// has decimal point, now count digits after that
	i++;
	int j = i;
	while( isdigit( input[j] ) )
	    j++;
	if ( j - i > d )
	    return QValidator::Valid;
    }

    if ( tmp < b || tmp > t )
	return QValidator::Valid;
    else
	return QValidator::Acceptable;
}


/*!
  Sets the validator to accept numbers from \a bottom up to and
  including \a top with at most \a decimals digits after the decimal
  point.
*/

void QDoubleValidator::setRange( double bottom, double top, int decimals )
{
    b = bottom;
    t = top;
    d = decimals;
}


/*!
  \fn double QDoubleValidator::bottom() const

  Returns the lowest valid number according to this validator.

  \sa top() decimals() setRange()
*/


/*!
  \fn double QDoubleValidator::top() const

  Returns the highest valid number according to this validator.

  \sa bottom() decimals setRange()
*/


/*!
  \fn int QDoubleValidator::decimals() const

  Returns the largest number of digits a valid number can have after
  its decimal point.

  \sa bottom() top() setRange()
*/
