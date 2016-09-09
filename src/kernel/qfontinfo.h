/****************************************************************************
** $Id: qfontinfo.h,v 2.8.2.1 1998/08/19 16:02:29 agulbra Exp $
**
** Definition of QFontInfo class
**
** Created : 950131
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

#ifndef QFONTINFO_H
#define QFONTINFO_H

#ifndef QT_H
#include "qfont.h"
#endif // QT_H


class Q_EXPORT QFontInfo
{
public:
    QFontInfo( const QFont & );
    QFontInfo( const QFontInfo & );
   ~QFontInfo();

    QFontInfo	       &operator=( const QFontInfo & );

    const char	       *family()	const;
    int			pointSize()	const;
    bool		italic()	const;
    int			weight()	const;
    bool		bold()		const;
    bool		underline()	const;
    bool		strikeOut()	const;
    bool		fixedPitch()	const;
    QFont::StyleHint	styleHint()	const;
    QFont::CharSet	charSet()	const;
    bool		rawMode()	const;

    bool		exactMatch()	const;

#if 1	/* OBSOLETE */
    const QFont &font() const;
#endif

private:
    QFontInfo( const QWidget * );
    QFontInfo( const QPainter * );
    static void reset( const QWidget * );
    static void reset( const QPainter * );
    const QFontDef *spec() const;

    enum Type { FontInternal, Widget, Painter };
    union {
	int   flags;
	void *dummy;
    } t;
    union {
	QFontInternal *f;
	QWidget	      *w;
	QPainter      *p;
    } u;

    int	    type()	     const { return t.flags & 0xff; }
    bool    underlineFlag()  const { return (t.flags & 0x100) != 0; }
    bool    strikeOutFlag()  const { return (t.flags & 0x200) != 0; }
    bool    exactMatchFlag() const { return (t.flags & 0x400) != 0; }
    void    setUnderlineFlag()	   { t.flags |= 0x100; }
    void    setStrikeOutFlag()	   { t.flags |= 0x200; }
    void    setExactMatchFlag()	   { t.flags |= 0x400; }

    friend class QWidget;
    friend class QPainter;
};


inline bool QFontInfo::bold() const
{ return weight() > QFont::Normal; }


#endif // QFONTINFO_H
