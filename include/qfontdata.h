/****************************************************************************
** $Id: qfontdata.h,v 2.12 1998/07/03 00:09:33 hanord Exp $
**
**		      ***   INTERNAL HEADER FILE   ***
**
**		This file is NOT a part of the Qt interface!
**
** Definition of QFontData struct
**
** Created : 941229
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

#ifndef QFONTDATA_H
#define QFONTDATA_H


struct QFontDef {				// font definition
    QString	family;
    short	pointSize;
    uint	styleHint	: 8;
    uint	charSet		: 8;
    uint	weight		: 8;
    uint	italic		: 1;
    uint	underline	: 1;
    uint	strikeOut	: 1;
    uint	fixedPitch	: 1;
    uint	hintSetByUser	: 1;
    uint	rawMode		: 1;
    uint	dirty		: 1;
    short	lbearing;
    short	rbearing;
};


class QFontInternal;

struct QFontData : public QShared {
    QFontData()
	: exactMatch(FALSE), fin(0)
	{}
    QFontData( const QFontData &d )
	: QShared( d ), req(d.req), exactMatch(d.exactMatch), fin(d.fin)
	{}
   ~QFontData()
	{}
    QFontData &operator=( const QFontData &d )
	{
	    req = d.req;
	    exactMatch = d.exactMatch;
	    fin = d.fin;
	    return *this;
	}
    QFontDef	    req;			// requested font
    bool	    exactMatch;
    QFontInternal  *fin;
};


#endif // QFONTDATA_H
