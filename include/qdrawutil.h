/****************************************************************************
** $Id: qdrawutil.h,v 2.8.2.2 1998/08/25 12:38:21 hanord Exp $
**
** Definition of draw utilities
**
** Created : 950920
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

#ifndef QDRAWUTIL_H
#define QDRAWUTIL_H

#ifndef QT_H
#include "qpainter.h"
#include "qpalette.h"
#endif // QT_H


//
// Standard shade drawing
//

Q_EXPORT
void qDrawShadeLine( QPainter *p, int x1, int y1, int x2, int y2,
		     const QColorGroup &g, bool sunken = TRUE,
		     int lineWidth = 1, int midLineWidth = 0 );

Q_EXPORT
void qDrawShadeLine( QPainter *p, const QPoint &p1, const QPoint &p2,
		     const QColorGroup &g, bool sunken = TRUE,
		     int lineWidth = 1, int midLineWidth = 0 );

Q_EXPORT
void qDrawShadeRect( QPainter *p, int x, int y, int w, int h,
		     const QColorGroup &, bool sunken=FALSE,
		     int lineWidth = 1, int midLineWidth = 0,
		     const QBrush *fill = 0 );

Q_EXPORT
void qDrawShadeRect( QPainter *p, const QRect &r,
		     const QColorGroup &, bool sunken=FALSE,
		     int lineWidth = 1, int midLineWidth = 0,
		     const QBrush *fill = 0 );

Q_EXPORT
void qDrawShadePanel( QPainter *p, int x, int y, int w, int h,
		      const QColorGroup &, bool sunken=FALSE,
		      int lineWidth = 1, const QBrush *fill = 0 );

Q_EXPORT
void qDrawShadePanel( QPainter *p, const QRect &r,
		      const QColorGroup &, bool sunken=FALSE,
		      int lineWidth = 1, const QBrush *fill = 0 );

Q_EXPORT
void qDrawWinButton( QPainter *p, int x, int y, int w, int h,
		     const QColorGroup &g, bool sunken = FALSE,
		     const QBrush *fill = 0 );

Q_EXPORT
void qDrawWinButton( QPainter *p, const QRect &r,
		     const QColorGroup &g, bool sunken = FALSE,
		     const QBrush *fill = 0 );

Q_EXPORT
void qDrawWinPanel( QPainter *p, int x, int y, int w, int h,
		    const QColorGroup &, bool sunken=FALSE,
		    const QBrush *fill = 0 );

Q_EXPORT
void qDrawWinPanel( QPainter *p, const QRect &r,
		    const QColorGroup &, bool sunken=FALSE,
		    const QBrush *fill = 0 );

Q_EXPORT
void qDrawPlainRect( QPainter *p, int x, int y, int w, int h, const QColor &,
		     int lineWidth = 1, const QBrush *fill = 0 );

Q_EXPORT
void qDrawPlainRect( QPainter *p, const QRect &r, const QColor &,
		     int lineWidth = 1, const QBrush *fill = 0 );


//
// Other useful drawing functions
//

Q_EXPORT
QRect qItemRect( QPainter *p, GUIStyle gs, int x, int y, int w, int h,
		int flags, bool enabled,
		const QPixmap *pixmap, const char *text, int len=-1 );

Q_EXPORT
void qDrawItem( QPainter *p, GUIStyle gs, int x, int y, int w, int h,
		int flags, const QColorGroup &g, bool enabled,
		const QPixmap *pixmap, const char *text, int len=-1 );

enum ArrowType
    { UpArrow, DownArrow, LeftArrow, RightArrow };

Q_EXPORT
void qDrawArrow( QPainter *p, ArrowType type, GUIStyle style, bool down,
		 int x, int y, int w, int h,
		 const QColorGroup &g, bool enabled );

#endif // QDRAWUTIL_H
