/****************************************************************************
** $Id: qpalette.cpp,v 2.19 1998/07/03 00:09:36 hanord Exp $
**
** Implementation of QColorGroup and QPalette classes
**
** Created : 950323
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

#include "qpalette.h"
#include "qdatastream.h"

/*****************************************************************************
  QColorGroup member functions
 *****************************************************************************/

/*!
  \class QColorGroup qpalette.h
  \brief The QColorGroup class contains a group of widget colors.

  \ingroup color
  \ingroup drawing

  A color group contains a group of colors used by widgets for drawing
  themselves.  Widgets should not use colors like "red" and "turqoise"
  but rather "foreground" and "base", where possible.

  We have identified eight distinct color roles:
  <ol>
  <li>Foreground (graphics foreground color)
  <li>Background (general background color)
  <li>Light (lighter than background color, for shadow effects)
  <li>Midlight (between Background and Light, for shadow effects)
  <li>Dark (darker than the background color, for shadow effects)
  <li>Medium (between background and dark, used for shadow and contrast
    effects)
  <li>Text (usually the same as the foreground color, but sometimes text
    and other foreground are not the same)
  <li>Base (used as background color for some widgets). Usually white or
    another light color.
  </ol>

  We have not seen any good, well-made and usable widgets that use more
  than these eight color roles.

  A QPalette contains three color groups.

  The current widget color group is returned by QWidget::colorGroup().

  \sa QColor, QPalette
*/


/*!
  Constructs a color group with all colors set to black.
*/

QColorGroup::QColorGroup()
{						// all colors become black
}

/*!
  Constructs a color group with the specified colors.
*/

QColorGroup::QColorGroup( const QColor &foreground, const QColor &background,
			  const QColor &light, const QColor &dark,
			  const QColor &mid,
			  const QColor &text, const QColor &base )
{
    fg_col    = foreground;
    bg_col    = background;
    light_col = light;
    dark_col  = dark;
    mid_col   = mid;
    text_col  = text;
    base_col  = base;
}

/*!
  Destroys the color group.
*/

QColorGroup::~QColorGroup()
{
}


/*!
  \fn const QColor & QColorGroup::foreground() const
  Returns the foreground color of the color group.
*/

/*!
  \fn const QColor & QColorGroup::background() const
  Returns the background color of the color group.
*/

/*!
  \fn const QColor & QColorGroup::light() const
  Returns the light color of the color group.
*/

/*!
  \fn QColor QColorGroup::midlight() const
  Returns the midlight color of the color group. Currently, this is
  a lightened version of the background, but this may change
  in the future, to return a <tt>const QColor &</tt> from the
  palette.
*/

/*!
  \fn const QColor & QColorGroup::dark() const
  Returns the dark color of the color group.
*/

/*!
  \fn const QColor & QColorGroup::mid() const
  Returns the medium color of the color group.
*/

/*!
  \fn const QColor & QColorGroup::text() const
  Returns the text foreground color of the color group.
*/

/*!
  \fn const QColor & QColorGroup::base() const
  Returns the base color of the color group.
*/

/*!
  \fn bool QColorGroup::operator!=( const QColorGroup &g ) const
  Returns TRUE if this color group is different from \e g, or FALSE if
  it is equal to \e g.
  \sa operator!=()
*/

/*!
  Returns TRUE if this color group is equal to \e g, or FALSE if
  it is different from \e g.
  \sa operator==()
*/

bool QColorGroup::operator==( const QColorGroup &g ) const
{
    return fg_col    == g.fg_col    && bg_col	== g.bg_col   &&
	   light_col == g.light_col && dark_col == g.dark_col &&
	   mid_col   == g.mid_col   && text_col == g.text_col &&
	   base_col  == g.base_col;
}


/*****************************************************************************
  QPalette member functions
 *****************************************************************************/

/*!
  \class QPalette qpalette.h

  \brief The QPalette class contains color groups for each widget state.

  \ingroup color
  \ingroup shared
  \ingroup drawing

  A palette consists of three color groups: a \e normal, a \e disabled
  and an \e active color group.	 All \link QWidget widgets\endlink
  contain a palette, and all the widgets in Qt use their palette to draw
  themselves.  This makes the user interface consistent and easily
  configurable.

  If you make a new widget you are strongly advised to use the colors in
  the palette rather than hard-coding specific colors.

  The \e active group is used for the widget in focus.	Normally it
  contains the same colors as \e normal so as not to overwhelm the user
  with bright and flashing colors, but if you need to you can change it.

  The \e disabled group is used for widgets that are currently
  inactive or not usable.

  The \e normal color group is used in all other cases.

  \sa QApplication::setPalette(), QWidget::setPalette(), QColorGroup, QColor
*/


static int palette_count = 1;

/*!
  Constructs a palette that consists of color groups with only black colors.
*/

QPalette::QPalette()
{
    data = new QPalData;
    CHECK_PTR( data );
    data->ser_no = palette_count++;
}

/*!
  Constructs a palette from the \e background color. The other colors are
  automatically calculated, based on this color.
*/

QPalette::QPalette( const QColor &background )
{
    data = new QPalData;
    CHECK_PTR( data );
    data->ser_no = palette_count++;
    QColor bg = background, fg, base, disfg;
    int h, s, v;
    bg.hsv( &h, &s, &v );
    if ( v > 128 ) {				// light background
	fg   = black;
	base = white;
	disfg = darkGray;
    } else {					// dark background
	fg   = white;
	base = black;
	disfg = darkGray;
    }
    data->normal   = QColorGroup( fg, bg, bg.light(150), bg.dark(),
				  bg.dark(150), fg, base );
    data->active   = data->normal;
    data->disabled = QColorGroup( disfg, bg, bg.light(150), bg.dark(),
				  bg.dark(150), disfg, base );
}

/*!
  Constructs a palette that consists of the three color groups \e normal,
  \e disabled and \e active.
*/

QPalette::QPalette( const QColorGroup &normal, const QColorGroup &disabled,
		    const QColorGroup &active )
{
    data = new QPalData;
    CHECK_PTR( data );
    data->ser_no = palette_count++;
    data->normal = normal;
    data->disabled = disabled;
    data->active = active;
}

/*!
  Constructs a palette that is a
  \link shclass.html shallow copy\endlink of \e p.
  \sa copy()
*/

QPalette::QPalette( const QPalette &p )
{
    data = p.data;
    data->ref();
}

/*!
  Destroys the palette.
*/

QPalette::~QPalette()
{
    if ( data->deref() )
	delete data;
}

/*!
  Assigns \e p to this palette and returns a reference to this palette.
  Note that a \e shallow copy of \a p is used.
  \sa copy()
*/

QPalette &QPalette::operator=( const QPalette &p )
{
    p.data->ref();
    if ( data->deref() )
	delete data;
    data = p.data;
    return *this;
}


/*!
  Returns a
  \link shclass.html deep copy\endlink of the palette.
*/

QPalette QPalette::copy() const
{
    QPalette p( data->normal, data->disabled, data->active );
    return p;
}


/*!
  Detaches this palette from any other QPalette objects with which
  it might implicitly share \link QColorGroup QColorGroups. \endlink

  Calling this should generally not be necessary; QPalette calls this
  itself when necessary.
*/

void QPalette::detach()
{
    if ( data->count != 1 )
	*this = copy();
}


/*!
  \fn const QColorGroup & QPalette::normal() const
  Returns the normal color group of this palette.
  \sa QColorGroup, disabled(), active(), setNormal()
*/

/*!
  Sets the \c normal color group to \e g.
  \sa normal()
*/

void QPalette::setNormal( const QColorGroup &g )
{
    detach();
    data->ser_no = palette_count++;
    data->normal = g;
}

/*!
  \fn const QColorGroup & QPalette::disabled() const
  Returns the disabled color group of this palette.
  \sa QColorGroup, normal(), active(), setDisabled()
*/

/*!
  Sets the \c disabled color group to \e g.
  \sa disabled()
*/

void QPalette::setDisabled( const QColorGroup &g )
{
    detach();
    data->ser_no = palette_count++;
    data->disabled = g;
}

/*!
  \fn const QColorGroup & QPalette::active() const
  Returns the active color group of this palette.
  \sa QColorGroup, normal(), disabled(), setActive()
*/

/*!
  Sets the \c active color group to \e g.
  \sa active()
*/

void QPalette::setActive( const QColorGroup &g )
{
    detach();
    data->ser_no = palette_count++;
    data->active = g;
}


/*!
  \fn bool QPalette::operator!=( const QPalette &p ) const
  Returns TRUE if this palette is different from \e p, or FALSE if they
  are equal.
*/

/*!
  Returns TRUE if this palette is equal to \e p, or FALSE if they
  are different.
*/

bool QPalette::operator==( const QPalette &p ) const
{
    return data->normal == p.data->normal &&
	   data->disabled == p.data->disabled &&
	   data->active == p.data->active;
}


/*!
  \fn int QPalette::serialNumber() const

  Returns a number that uniquely identifies this QPalette object. The
  serial number is very useful for caching.

  \sa QPixmap, QPixmapCache
*/


/*****************************************************************************
  QColorGroup/QPalette stream functions
 *****************************************************************************/

/*!
  \relates QColorGroup
  Writes a color group to the stream.

  Serialization format:
  <ol>
  <li> QColor foreground
  <li> QColor background
  <li> QColor light
  <li> QColor dark
  <li> QColor mid
  <li> QColor text
  <li> QColor base
  </ol>
  The colors are serialized in the listed order.
*/

QDataStream &operator<<( QDataStream &s, const QColorGroup &g )
{
    return s << g.foreground()
	     << g.background()
	     << g.light()
	     << g.dark()
	     << g.mid()
	     << g.text()
	     << g.base();
}

/*!
  \related QColorGroup
  Reads a color group from the stream.
*/

QDataStream &operator>>( QDataStream &s, QColorGroup &g )
{
    QColor fg;
    QColor bg;
    QColor light;
    QColor dark;
    QColor mid;
    QColor text;
    QColor base;
    s >> fg >> bg >> light >> dark >> mid >> text >> base;
    QColorGroup newcg( fg, bg, light, dark, mid, text, base );
    g = newcg;
    return s;
}


/*!
  \relates QPalette
  Writes a palette to the stream and returns a reference to the stream.

  Serialization format:
  <ol>
  <li> QColorGroup normal
  <li> QColorGroup disabled
  <li> QColorGroup active
  </ol>
  The color groups are serialized in the listed order.
*/

QDataStream &operator<<( QDataStream &s, const QPalette &p )
{
    return s << p.normal()
	     << p.disabled()
	     << p.active();
}

/*!
  \relates QPalette
  Reads a palette from the stream and returns a reference to the stream.
*/

QDataStream &operator>>( QDataStream &s, QPalette &p )
{
    QColorGroup normal, disabled, active;
    s >> normal >> disabled >> active;
    QPalette newpal( normal, disabled, active );
    p = newpal;
    return s;
}


/*!  Returns TRUE if this palette and \a p are copies of each other,
  ie. one of them was created as a copy of the other and neither was
  subsequently modified.  This is much stricter than equality.
  
  \sa operator=, operator==
*/

bool QPalette::isCopyOf( const QPalette & p )
{
    return data && data == p.data;
}
