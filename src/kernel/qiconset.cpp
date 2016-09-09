/****************************************************************************
** $Id: qiconset.cpp,v 2.11.2.7 1999/02/01 10:44:10 hanord Exp $
**
** Implementation of QIconSet class
**
** Created : 980318
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

#include "qiconset.h"
#include "qimage.h"
#include "qbitmap.h"
#include "qapplication.h"
#include "qpainter.h"

struct QIconSetPrivate: public QShared
{
    struct Variant {
	Variant(): pm(0), generated(0) {}
	~Variant()
	{
	    delete pm;
	}
	void clear()
	{
	    if ( generated ) {
		delete pm;
		pm = 0;
		generated = FALSE;
	    }
	}
	QPixmap * pm;
	bool generated;
    };
    Variant small;
    Variant large;
    Variant smallActive;
    Variant largeActive;
    Variant smallDisabled;
    Variant largeDisabled;
    QPixmap defpm;
};


/*! \class QIconSet qiconset.h

  \brief The QIconSet class provides a set of icons (normal, disabled,
  various sizes) for e.g. buttons.

  \ingroup misc

  QIconSet must be fed at least one icon, and can generate the other
  icons from the ones it is fed, or use programmer-specified icons.

  Using the icon or icons specified, QIconSet generates a set of six
  icons: <ul>
  <li> Small, normal
  <li> Small, disabled
  <li> Small, active
  <li> Large, normal
  <li> Large, disabled
  <li> Large, active
  </ul>

  You can set any of the icons using setPixmap() and when you retrieve
  one using pixmap(), QIconSet will compute and cache that from the
  closest other icon.

  The \c Disabled appearance is computed using a "shadow" algorithm
  which produces results very similar to that used in of Microsoft
  Windows 95.

  The \c Active appearance is identical to the \c Normal appearance
  unless you use setPixmap() to set it to something special.

  QIconSet provides a function, isGenerated(), that indicates whether
  an icon was set by the application programmer or computed by
  QIconSet itself.

  In Qt 1.41 only QToolButton uses QIconSet.  In Qt 2.0 we will use it
  in more classes, including the menu system.

  \sa QPixmap QLabel QToolButton
  <a href="guibooks.html#fowler">GUI Design Handbook: Iconic Label,</a>
  <q href="http://www.microsoft.com/clipgallerylive/icons.asp">Microsoft
  Icon Gallery.</a>
*/




/*!  Constructs an icon set that will generate its members from \a
  defaultPixmap, which is assumed to be of \a defaultSize.

  The default for \a defaultSize is \c Automatic, which means that
  QIconSet will determine the icon's size from its actual size.

  \sa reset()
*/

QIconSet::QIconSet( const QPixmap & defaultPixmap, Size defaultSize )
{
    d = 0;
    reset( defaultPixmap, defaultSize );
}


/*!  Constructs an a copy of \a other.  This is very fast. \sa detach() */

QIconSet::QIconSet( const QIconSet & other )
{
    d = other.d;
    if ( d )
	d->ref();
}


/*! Destroys the icon set and frees any allocated resources. */

QIconSet::~QIconSet()
{
    if ( d && d->deref() )
	delete d;
}


/*!
  Assigns \e other to this icon set and returns a reference to this
  icon set.

  This is very fast.

  \sa detach()
*/

QIconSet &QIconSet::operator=( const QIconSet &p )
{
    if ( p.d ) {
	p.d->ref();				// beware of p = p
	if ( d->deref() )
	    delete d;
	d = p.d;
	return *this;
    } else {
	if ( d && d->deref() )
	    delete d;
	d = 0;
	return *this;
    }
}



/*!  Set this icon set to display \a pm, which is assumed to be in
  size \a s.  If \a s is \c Automatic, QIconSet guesses the size from
  the size of \a pm using and unspecified algorithm.
*/

void QIconSet::reset( const QPixmap & pm, Size s )
{
    detach();
    if ( s == Small ||
	 (s == Automatic && pm.width() < 19 ) )
	setPixmap( pm, Small, Normal );
    else
	setPixmap( pm, Large, Normal );
    d->defpm = pm;
}


/*!  Sets this icon set to display \a pn in size \a s/mode \a m, and
  perhaps to use \a pm for deriving some other varieties.

  \a s must be Large or Small; it cannot be Automatic.
*/

void QIconSet::setPixmap( const QPixmap & pm, Size s, Mode m )
{
    detach();
    if ( d ) {
	d->small.clear();
	d->large.clear();
	d->smallDisabled.clear();
	d->largeDisabled.clear();
	d->smallActive.clear();
	d->largeActive.clear();
    } else {
	d = new QIconSetPrivate;
    }
    if ( s == Large ) {
	switch( m ) {
	case Active:
	    d->largeActive.pm = new QPixmap( pm );
	    break;
	case Disabled:
	    d->largeDisabled.pm = new QPixmap( pm );
	    break;
	case Normal:
	default:
	    d->large.pm = new QPixmap( pm );
	    break;
	}
    } else if ( s == Small ) {
	switch( m ) {
	case Active:
	    d->smallActive.pm = new QPixmap( pm );
	    break;
	case Disabled:
	    d->smallDisabled.pm = new QPixmap( pm );
	    break;
	case Normal:
	default:
	    d->small.pm = new QPixmap( pm );
	    break;
	}
    }
}


/*!  Sets this icon set to load \a fileName as a pixmap and display it
  in size \a s/mode \a m, and perhaps to use \a pm for deriving some
  other varieties.
*/

void QIconSet::setPixmap( const char * fileName, Size s, Mode m )
{
    QPixmap p;
    p.load( fileName );
    if ( !p.isNull() )
	setPixmap( p, s, m );
}


/*!  Returns a pixmap with size \a s and mode \a m, generating one if
  needed.
*/

QPixmap QIconSet::pixmap( Size s, Mode m ) const
{
    if ( !d ) {
	QPixmap r;
	return r;
    }
	
    QImage i;
    QIconSetPrivate * p = ((QIconSet *)this)->d;
    QPixmap * pm = 0;
    if ( s == Large ) {
	switch( m ) {
	case Normal:
	    if ( !p->large.pm ) {
		ASSERT( p->small.pm );
		i = p->small.pm->convertToImage();
		i = i.smoothScale( i.width() * 3 / 2, i.height() * 3 / 2 );
		p->large.pm = new QPixmap;
		p->large.generated = TRUE;
		p->large.pm->convertFromImage( i );
		if ( !p->large.pm->mask() ) {
		    i = i.createHeuristicMask();
		    QBitmap tmp;
		    tmp.convertFromImage( i, MonoOnly + ThresholdDither );
		    p->large.pm->setMask( tmp );
		}
	    }
	    pm = p->large.pm;
	    break;
	case Active:
	    if ( !p->largeActive.pm ) {
		p->largeActive.pm = new QPixmap( pixmap( Large, Normal ) );
		p->largeActive.generated = TRUE;
	    }
	    pm = p->largeActive.pm;
	    break;
	case Disabled:
	    if ( !p->largeDisabled.pm ) {
		QBitmap tmp;
		if ( p->large.generated && !p->smallDisabled.generated 
		     && p->smallDisabled.pm && !p->smallDisabled.pm->isNull() ) {
		    // if there's a hand-drawn disabled small image,
		    // but the normal big one is generated, use the
		    // hand-drawn one to generate this one.
		    i = p->smallDisabled.pm->convertToImage();
		    i = i.smoothScale( i.width() * 3 / 2, i.height() * 3 / 2 );
		    p->largeDisabled.pm = new QPixmap;
		    p->largeDisabled.pm->convertFromImage( i );
		    if ( !p->largeDisabled.pm->mask() ) {
			i = i.createHeuristicMask();
			tmp.convertFromImage( i, MonoOnly + ThresholdDither );
		    }
		} else {
		    if (pixmap( Large, Normal).mask())
			tmp = *pixmap( Large, Normal).mask();
		    else {
			i = pixmap( Large, Normal ).convertToImage();
			i = i.createHeuristicMask();
			tmp.convertFromImage( i, MonoOnly + ThresholdDither );
		    }
		    p->largeDisabled.pm
			= new QPixmap( p->large.pm->width()+1,
				       p->large.pm->height()+1);
		    QColorGroup dis( QApplication::palette()->disabled() );
		    p->largeDisabled.pm->fill( dis.background() );
		    QPainter painter( p->largeDisabled.pm );
		    painter.setPen( dis.base() );
		    painter.drawPixmap( 1, 1, tmp );
		    painter.setPen( dis.foreground() );
		    painter.drawPixmap( 0, 0, tmp );
		}
		if ( !p->largeDisabled.pm->mask() ) {
		    if ( !tmp.mask() )
			tmp.setMask( tmp );
		    QBitmap mask( d->largeDisabled.pm->size() );
		    mask.fill( color0 );
		    QPainter painter( &mask );
		    painter.drawPixmap( 0, 0, tmp );
		    painter.drawPixmap( 1, 1, tmp );
		    painter.end();
		    p->largeDisabled.pm->setMask( mask );
		}
		p->largeDisabled.generated = TRUE;
	    }
	    pm = p->largeDisabled.pm;
	    break;
	}
    } else {
	switch( m ) {
	case Normal:
	    if ( !p->small.pm ) {
		ASSERT( p->large.pm );
		i = p->large.pm->convertToImage();
		i = i.smoothScale( i.width() * 2 / 3, i.height() * 2 / 3 );
		p->small.pm = new QPixmap;
		p->small.generated = TRUE;
		p->small.pm->convertFromImage( i );
		if ( !p->small.pm->mask() ) {
		    i = i.createHeuristicMask();
		    QBitmap tmp;
		    tmp.convertFromImage( i, MonoOnly + ThresholdDither );
		    p->small.pm->setMask( tmp );
		}
	    }
	    pm = p->small.pm;
	    break;
	case Active:
	    if ( !p->smallActive.pm ) {
		p->smallActive.pm = new QPixmap( pixmap( Small, Normal ) );
		p->smallActive.generated = TRUE;
	    }
	    pm = p->smallActive.pm;
	    break;
	case Disabled:
	    if ( !p->smallDisabled.pm ) {
		QBitmap tmp;
		if ( p->small.generated && !p->largeDisabled.generated 
		     && p->largeDisabled.pm && !p->largeDisabled.pm->isNull() ) {
		    // if there's a hand-drawn disabled large image,
		    // but the normal small one is generated, use the
		    // hand-drawn one to generate this one.
		    i = p->largeDisabled.pm->convertToImage();
		    i = i.smoothScale( i.width() * 3 / 2, i.height() * 3 / 2 );
		    p->smallDisabled.pm = new QPixmap;
		    p->smallDisabled.pm->convertFromImage( i );
		    if ( !p->smallDisabled.pm->mask() ) {
			i = i.createHeuristicMask();
			tmp.convertFromImage( i, MonoOnly + ThresholdDither );
		    }
		} else {
		    if ( pixmap( Small, Normal).mask())
			tmp = *pixmap( Small, Normal).mask();
		    else {
			i = pixmap( Small, Normal ).convertToImage();
			i = i.createHeuristicMask();
			tmp.convertFromImage( i, MonoOnly + ThresholdDither );
		    }
		    p->smallDisabled.pm
			= new QPixmap( p->small.pm->width()+1,
				       p->small.pm->height()+1);
		    QColorGroup dis( QApplication::palette()->disabled() );
		    p->smallDisabled.pm->fill( dis.background() );
		    QPainter painter( p->smallDisabled.pm );
		    painter.setPen( dis.base() );
		    painter.drawPixmap( 1, 1, tmp );
		    painter.setPen( dis.foreground() );
		    painter.drawPixmap( 0, 0, tmp );
		}
		if ( !p->smallDisabled.pm->mask() ) {
		    if ( !tmp.mask() )
			tmp.setMask( tmp );
		    QBitmap mask( d->smallDisabled.pm->size() );
		    mask.fill( color0 );
		    QPainter painter( &mask );
		    painter.drawPixmap( 0, 0, tmp );
		    painter.drawPixmap( 1, 1, tmp );
		    painter.end();
		    p->smallDisabled.pm->setMask( mask );
		}

		p->smallDisabled.generated = TRUE;
	    }
	    pm = p->smallDisabled.pm;
	    break;
	}
    }
    ASSERT( pm );
    return *pm;
}


/*! Returns TRUE if the variant with size \a s and mode \a m was
  automatically generated, and FALSE if it was not.
*/

bool QIconSet::isGenerated( Size s, Mode m ) const
{
    if ( s == Large ) {
	if ( m == Disabled )
	    return d->largeDisabled.generated || !d->largeDisabled.pm;
	else if ( m == Active )
	    return d->largeActive.generated || !d->largeActive.pm;
	else
	    return d->large.generated || !d->large.pm;
    } else if ( s == Small ) {
	if ( m == Disabled )
	    return d->smallDisabled.generated || !d->smallDisabled.pm;
	else if ( m == Active )
	    return d->smallActive.generated || !d->smallActive.pm;
	else
	    return d->small.generated || !d->small.pm;
    }
    return FALSE;
}


/*!  Returns the pixmap originally provided to the constructor or
  reset().

  \sa reset()
*/

QPixmap QIconSet::pixmap() const
{
    return d->defpm;
}


/*!  Detaches this icon set from others with which it may share data.
  You should never need to call this function; reset() and setPixmap()
  call it as necessary.  It exists merely so that the copy constructor
  and operator= can be faster.
*/

void QIconSet::detach()
{
    if ( !d || d->count == 1 )
	return;

    QIconSetPrivate * p = new QIconSetPrivate;
    p->small.pm = d->small.pm;
    p->small.generated = d->small.generated;
    p->smallActive.pm = d->smallActive.pm;
    p->smallActive.generated = d->smallActive.generated;
    p->smallDisabled.pm = d->smallDisabled.pm;
    p->smallDisabled.generated = d->smallDisabled.generated;
    p->large.pm = d->large.pm;
    p->large.generated = d->large.generated;
    p->largeActive.pm = d->largeActive.pm;
    p->largeActive.generated = d->largeActive.generated;
    p->largeDisabled.pm = d->largeDisabled.pm;
    p->largeDisabled.generated = d->largeDisabled.generated;
    p->defpm = d->defpm;
    d->deref();
    d = p;
}
