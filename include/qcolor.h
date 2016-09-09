/****************************************************************************
** $Id: qcolor.h,v 2.11.2.2 1998/08/25 09:20:52 hanord Exp $
**
** Definition of QColor class
**
** Created : 940112
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

#ifndef QCOLOR_H
#define QCOLOR_H

#ifndef QT_H
#include "qwindowdefs.h"
#endif // QT_H


const QRgb  RGB_DIRTY	= 0x80000000;		// flags unset color
const QRgb  RGB_INVALID = 0x40000000;		// flags invalid color
const QRgb  RGB_DIRECT	= 0x20000000;		// flags directly set pixel
const QRgb  RGB_MASK	= 0x00ffffff;		// masks RGB values


Q_EXPORT inline int qRed( QRgb rgb )		// get red part of RGB
{ return (int)(rgb & 0xff); }

Q_EXPORT inline int qGreen( QRgb rgb )		// get green part of RGB
{ return (int)((rgb >> 8) & 0xff); }

Q_EXPORT inline int qBlue( QRgb rgb )		// get blue part of RGB
{ return (int)((rgb >> 16) & 0xff); }

Q_EXPORT inline QRgb qRgb( int r, int g, int b )// set RGB value
{ return (uint)(r & 0xff) |((uint)(g & 0xff) << 8) |((uint)(b & 0xff) << 16); }

Q_EXPORT inline int qGray( int r, int g, int b )// convert R,G,B to gray 0..255
{ return (r*11+g*16+b*5)/32; }

Q_EXPORT inline int qGray( QRgb rgb )		// convert RGB to gray 0..255
{ return qGray( qRed(rgb), qGreen(rgb), qBlue(rgb) ); }


class Q_EXPORT QColor
{
public:
    enum Spec { Rgb, Hsv };

    QColor();
    QColor( int r, int g, int b );
    QColor( int x, int y, int z, Spec );
    QColor( QRgb rgb, uint pixel=0xffffffff);
    QColor( const char *name );
    QColor( const QColor & );
    QColor &operator=( const QColor & );

    bool   isValid() const;
    bool   isDirty() const;

    void   setNamedColor( const char *name );

    void   rgb( int *r, int *g, int *b ) const;
    QRgb   rgb()    const;
    void   setRgb( int r, int g, int b );
    void   setRgb( QRgb rgb );

    int	   red()    const;
    int	   green()  const;
    int	   blue()   const;

    void   hsv( int *h, int *s, int *v ) const;
    void   setHsv( int h, int s, int v );

    QColor light( int f = 150 ) const;
    QColor dark( int f = 200 )	const;

    bool   operator==( const QColor &c ) const;
    bool   operator!=( const QColor &c ) const;

    static bool lazyAlloc();
    static void setLazyAlloc( bool );
    uint   alloc();
    uint   pixel()  const;

    static int  maxColors();
    static int  numBitPlanes();

    static int  enterAllocContext();
    static void leaveAllocContext();
    static int  currentAllocContext();
    static void destroyAllocContext( int );

#if defined(_WS_WIN_)
    static HANDLE hPal()  { return hpal; }
    static uint	  realizePal( QWidget * );
#endif

    static void initialize();
    static void cleanup();

private:
    void   setSystemNamedColor( const char *name );
    static void initGlobalColors();
    static bool color_init;
    static bool globals_init;
    static bool lalloc;
#if defined(_WS_WIN_)
    static HANDLE hpal;
#endif
    uint   pix;
    QRgb   rgbVal;
};


inline QColor::QColor()
{ rgbVal = RGB_INVALID; pix = 0; }

inline QColor::QColor( int r, int g, int b )
{ setRgb( r, g, b ); }

inline bool QColor::isValid() const
{ return (rgbVal & RGB_INVALID) == 0; }

inline bool QColor::isDirty() const
{ return (rgbVal & RGB_DIRTY) != 0; }

inline QRgb QColor::rgb() const
{ return rgbVal & RGB_MASK; }

inline int QColor::red() const
{ return qRed(rgbVal); }

inline int QColor::green() const
{ return qGreen(rgbVal); }

inline int QColor::blue() const
{ return qBlue(rgbVal); }

inline uint QColor::pixel() const
{ return (rgbVal & RGB_DIRTY) == 0 ? pix : ((QColor*)this)->alloc(); }

inline bool QColor::lazyAlloc()
{ return lalloc; }


inline bool QColor::operator==( const QColor &c ) const
{
    return (((rgbVal | c.rgbVal) & RGB_DIRECT) == 0 &&
	    (rgbVal & 0x00ffffff) == (c.rgbVal & 0x00ffffff)) ||
	   ((rgbVal & c.rgbVal & RGB_DIRECT) != 0 &&
	    (rgbVal & 0x00ffffff) == (c.rgbVal & 0x00ffffff) && pix == c.pix);
}

inline bool QColor::operator!=( const QColor &c ) const
{
    return !operator==(c);
}


/*****************************************************************************
  Global colors
 *****************************************************************************/

extern Q_EXPORT const QColor color0;
extern Q_EXPORT const QColor color1;
extern Q_EXPORT const QColor black;
extern Q_EXPORT const QColor white;
extern Q_EXPORT const QColor darkGray;
extern Q_EXPORT const QColor gray;
extern Q_EXPORT const QColor lightGray;
extern Q_EXPORT const QColor red;
extern Q_EXPORT const QColor green;
extern Q_EXPORT const QColor blue;
extern Q_EXPORT const QColor cyan;
extern Q_EXPORT const QColor magenta;
extern Q_EXPORT const QColor yellow;
extern Q_EXPORT const QColor darkRed;
extern Q_EXPORT const QColor darkGreen;
extern Q_EXPORT const QColor darkBlue;
extern Q_EXPORT const QColor darkCyan;
extern Q_EXPORT const QColor darkMagenta;
extern Q_EXPORT const QColor darkYellow;


/*****************************************************************************
  QColor stream functions
 *****************************************************************************/

Q_EXPORT QDataStream &operator<<( QDataStream &, const QColor & );
Q_EXPORT QDataStream &operator>>( QDataStream &, QColor & );


#endif // QCOLOR_H
