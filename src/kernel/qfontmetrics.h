/****************************************************************************
** $Id: qfontmetrics.h,v 2.15.2.1 1998/08/19 16:02:29 agulbra Exp $
**
** Definition of QFontMetrics class
**
** Created : 940514
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

#ifndef QFONTMETRICS_H
#define QFONTMETRICS_H

#ifndef QT_H
#include "qfont.h"
#include "qrect.h"
#endif // QT_H


class Q_EXPORT QFontMetrics
{
public:
    QFontMetrics( const QFont & );
    QFontMetrics( const QFontMetrics & );
   ~QFontMetrics();

    QFontMetrics &operator=( const QFontMetrics & );

    int		ascent()	const;
    int		descent()	const;
    int		height()	const;
    int		leading()	const;
    int		lineSpacing()	const;
    int		minLeftBearing() const;
    int		minRightBearing() const;
    int		maxWidth()	const;

    bool	inFont(char)	const;

    int		leftBearing(char) const;
    int		rightBearing(char) const;
    int		width( const char *, int len = -1 ) const;
    int		width( char ) const;
    QRect	boundingRect( const char *, int len = -1 ) const;
    QRect	boundingRect( char ) const;
    QRect	boundingRect( int x, int y, int w, int h, int flags,
			      const char *str, int len=-1, int tabstops=0,
			      int *tabarray=0, char **intern=0 ) const;
    QSize	size( int flags,
		      const char *str, int len=-1, int tabstops=0,
		      int *tabarray=0, char **intern=0 ) const;

    int		underlinePos()	const;
    int		strikeOutPos()	const;
    int		lineWidth()	const;

#if 1	/* OBSOLETE */
    const QFont &font() const;
#endif

private:
    QFontMetrics( const QWidget * );
    QFontMetrics( const QPainter * );
    static void reset( const QWidget * );
    static void reset( const QPainter * );
    const QFontDef *spec() const;
#if defined(_WS_WIN_)
    void *textMetric() const;
    HDC hdc() const;
#elif defined(_WS_X11_)
    void *fontStruct() const;
    int printerAdjusted(int) const;
#endif

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
    void    setUnderlineFlag()	   { t.flags |= 0x100; }
    void    setStrikeOutFlag()	   { t.flags |= 0x200; }

    friend class QWidget;
    friend class QPainter;
};


#endif // QFONTMETRICS_H
