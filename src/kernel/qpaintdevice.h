/****************************************************************************
** $Id: qpaintdevice.h,v 2.8.2.5 1999/01/18 11:22:37 aavit Exp $
**
** Definition of QPaintDevice class
**
** Created : 940721
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

#ifndef QPAINTDEVICE_H
#define QPAINTDEVICE_H

#ifndef QT_H
#include "qwindowdefs.h"
#include "qrect.h"
#endif // QT_H


// Painter device types (is-A)

#define PDT_UNDEF	0x00
#define PDT_WIDGET	0x01
#define PDT_PIXMAP	0x02
#define PDT_PRINTER	0x03
#define PDT_PICTURE	0x04
#define PDT_SYSTEM	0x05
#define PDT_MASK	0x0f


// Painter device flags

#define PDF_EXTDEV	0x10
#define PDF_PAINTACTIVE 0x20
#define PDF_OWNDEPTH	0x100

// Painter device command param (defined in qpaintdevicedefs.h)

union QPDevCmdParam;


class Q_EXPORT QPaintDevice				// device for QPainter
{
public:
    virtual ~QPaintDevice();

    int	     devType()	      const;
    bool     isExtDev()	      const;
    bool     paintingActive() const;

    // Windows:	  get device context
    // OS/2 PM:	  get presentation space
    // X-Windows: get drawable
    HANDLE   handle()  const;

#if !defined(_WS_X11_)
#define Display void
#endif
    Display *x11Display() const;		// X11 only

#if defined(_WS_X11_)
    static Display *x__Display();
    static int	    x11Screen();
    static int	    x11Depth();
    static int	    x11Cells();
    static HANDLE   x11Colormap();
    static bool	    x11DefaultColormap();
    static void	   *x11Visual();
    static bool	    x11DefaultVisual();
#endif

protected:
    QPaintDevice( uint devflags );

#if defined(_WS_WIN_)
    HDC	     hdc;				// device context
#elif defined(_WS_PM_)
    HPS	     hps;				// presentation space
#elif defined(_WS_X11_)
    static Display *dpy;			// display (common to all)
    HANDLE   hd;				// handle to drawable
#endif

    virtual bool cmd( int, QPainter *, QPDevCmdParam * );
    virtual int	 metric( int ) const;
    virtual int	 fontMet( QFont *, int, const char * = 0, int = 0 ) const;
    virtual int	 fontInf( QFont *, int ) const;

    uint     devFlags;				// device flags

    friend class QColor;
    friend class QPainter;
    friend class QPaintDeviceMetrics;
    friend Q_EXPORT void bitBlt( QPaintDevice *, int, int,
				 const QPaintDevice *,
				 int, int, int, int, RasterOp, bool );

#if defined(_WS_X11_)
private:
    static Display *x_display;
    static int	    x_screen;
    static int	    x_depth;
    static int	    x_cells;
    static HANDLE   x_colormap;
    static bool	    x_defcmap;
    static void	   *x_visual;
    static bool     x_defvisual;
#endif

private:	// Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QPaintDevice( const QPaintDevice & );
    QPaintDevice &operator=( const QPaintDevice & );
#endif
};


Q_EXPORT
void bitBlt( QPaintDevice *dst, int dx, int dy,
	     const QPaintDevice *src, int sx=0, int sy=0, int sw=-1, int sh=-1,
	     RasterOp = CopyROP, bool ignoreMask=FALSE );

Q_EXPORT
void bitBlt( QPaintDevice *dst, int dx, int dy,
	     const QImage *src, int sx=0, int sy=0, int sw=-1, int sh=-1,
	     int conversion_flags=0 );


/*****************************************************************************
  Inline functions
 *****************************************************************************/

inline int QPaintDevice::devType() const
{ return devFlags & PDT_MASK; }

inline bool QPaintDevice::isExtDev() const
{ return (devFlags & PDF_EXTDEV) != 0; }

inline bool QPaintDevice::paintingActive() const
{ return (devFlags & PDF_PAINTACTIVE) != 0; }

#if defined(_WS_WIN_)
inline HANDLE	QPaintDevice::handle()	const { return hdc; }
#elif defined(_WS_PM_)
inline HANDLE	QPaintDevice::handle()	const { return hps; }
#elif defined(_WS_X11_)
inline HANDLE	QPaintDevice::handle()	const { return hd; }
#endif

#if defined(_WS_X11_)
inline Display *QPaintDevice::x11Display() const { return dpy; }
#else
inline Display *QPaintDevice::x11Display() const { return 0; }
#undef Display
#endif

#if defined(_WS_X11_)
inline Display *QPaintDevice::x__Display()	   { return x_display; }
inline int	QPaintDevice::x11Screen()	   { return x_screen; }
inline int	QPaintDevice::x11Depth()	   { return x_depth; }
inline int	QPaintDevice::x11Cells()	   { return x_cells; }
inline HANDLE	QPaintDevice::x11Colormap()	   { return x_colormap; }
inline bool    	QPaintDevice::x11DefaultColormap() { return x_defcmap; }
inline void    *QPaintDevice::x11Visual()	   { return x_visual; }
inline bool    	QPaintDevice::x11DefaultVisual()   { return x_defvisual; }
#endif


Q_EXPORT
inline void bitBlt( QPaintDevice *dst, const QPoint &dp,
		    const QPaintDevice *src, const QRect &sr =QRect(0,0,-1,-1),
		    RasterOp rop=CopyROP, bool ignoreMask=FALSE )
{
    bitBlt( dst, dp.x(), dp.y(), src, sr.x(), sr.y(), sr.width(), sr.height(),
	    rop, ignoreMask );
}


#endif // QPAINTDEVICE_H
