/****************************************************************************
** $Id: qpalette.h,v 2.15.2.2 1998/08/25 09:20:52 hanord Exp $
**
** Definition of QColorGroup and QPalette classes
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

#ifndef QPALETTE_H
#define QPALETTE_H

#ifndef QT_H
#include "qwindowdefs.h"
#include "qcolor.h"
#include "qshared.h"
#endif // QT_H


class Q_EXPORT QColorGroup				// color group class
{
public:
    QColorGroup();				// all colors black
    QColorGroup( const QColor &foreground, const QColor &background,
		 const QColor &light, const QColor &dark, const QColor &mid,
		 const QColor &text, const QColor &base);
   ~QColorGroup();

    const QColor &foreground()	const	{ return fg_col; }
    const QColor &background()	const	{ return bg_col; }
    const QColor &light()	const	{ return light_col; }
          QColor midlight()	const	{ return bg_col.light( 115 ); }
    const QColor &dark()	const	{ return dark_col; }
    const QColor &mid()		const	{ return mid_col; }
    const QColor &text()	const	{ return text_col; }
    const QColor &base()	const	{ return base_col; }

    bool	operator==( const QColorGroup &g ) const;
    bool	operator!=( const QColorGroup &g ) const
					{ return !(operator==(g)); }
private:
    QColor fg_col;
    QColor bg_col;
    QColor light_col;
    QColor dark_col;
    QColor mid_col;
    QColor text_col;
    QColor base_col;
};


class Q_EXPORT QPalette					// palette class
{
public:
    QPalette();
    QPalette( const QColor &background );
    QPalette( const QColorGroup &normal, const QColorGroup &disabled,
	      const QColorGroup &active );
    QPalette( const QPalette & );
   ~QPalette();
    QPalette &operator=( const QPalette & );

    QPalette	copy() const;

    const QColorGroup &normal()	  const { return data->normal; }
    const QColorGroup &disabled() const { return data->disabled; }
    const QColorGroup &active()	  const { return data->active; }

    void	setNormal( const QColorGroup & );
    void	setDisabled( const QColorGroup & );
    void	setActive( const QColorGroup & );

    bool	operator==( const QPalette &p ) const;
    bool	operator!=( const QPalette &p ) const
					{ return !(operator==(p)); }
    bool	isCopyOf( const QPalette & );

    int		serialNumber() const	{ return data->ser_no; }

private:
    void	detach();

    struct QPalData : public QShared {
	QColorGroup normal;
	QColorGroup disabled;
	QColorGroup active;
	int	    ser_no;
    } *data;
};


/*****************************************************************************
  QColorGroup/QPalette stream functions
 *****************************************************************************/

Q_EXPORT QDataStream &operator<<( QDataStream &, const QColorGroup & );
Q_EXPORT QDataStream &operator>>( QDataStream &, QColorGroup & );

Q_EXPORT QDataStream &operator<<( QDataStream &, const QPalette & );
Q_EXPORT QDataStream &operator>>( QDataStream &, QPalette & );


#endif // QPALETTE_H
