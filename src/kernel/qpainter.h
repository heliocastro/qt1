/****************************************************************************
** $Id: qpainter.h,v 2.27.2.2 1998/08/21 19:13:23 hanord Exp $
**
** Definition of QPainter class
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

#ifndef QPAINTER_H
#define QPAINTER_H


#ifndef QT_H
#include "qpaintdevice.h"
#include "qcolor.h"
#include "qfontmetrics.h"
#include "qfontinfo.h"
#include "qregion.h"
#include "qpen.h"
#include "qbrush.h"
#include "qpointarray.h"
#include "qwmatrix.h"
#endif // QT_H


enum BGMode					// background mode
    { TransparentMode, OpaqueMode };

enum PaintUnit					// paint unit
    { PixelUnit, LoMetricUnit, HiMetricUnit, LoEnglishUnit, HiEnglishUnit,
      TwipsUnit };


#if defined(_WS_WIN_)
struct QWinFont;
#endif


class Q_EXPORT QPainter					// painter class
{
public:
    QPainter();
    QPainter( const QPaintDevice * );
    QPainter( const QPaintDevice *, const QWidget * );
   ~QPainter();

    bool	begin( const QPaintDevice * );
    bool	begin( const QPaintDevice *, const QWidget * );
    bool	end();
    QPaintDevice *device() const;

    static void redirect( QPaintDevice *pdev, QPaintDevice *replacement );

    bool	isActive() const;

    void	flush();
    void	save();
    void	restore();

  // Drawing tools

    QFontMetrics fontMetrics()	const;
    QFontInfo	 fontInfo()	const;

    const QFont &font()		const;
    void	setFont( const QFont & );
    const QPen &pen()		const;
    void	setPen( const QPen & );
    void	setPen( PenStyle );
    void	setPen( const QColor & );
    const QBrush &brush()	const;
    void	setBrush( const QBrush & );
    void	setBrush( BrushStyle );
    void	setBrush( const QColor & );

  // Drawing attributes/modes

    const QColor &backgroundColor() const;
    void	setBackgroundColor( const QColor & );
    BGMode	backgroundMode() const;
    void	setBackgroundMode( BGMode );
    RasterOp	rasterOp()	const;
    void	setRasterOp( RasterOp );
    const QPoint &brushOrigin() const;
    void	setBrushOrigin( int x, int y );
    void	setBrushOrigin( const QPoint & );

  // Scaling and transformations

//    PaintUnit unit()	       const;		// get set painter unit
//    void	setUnit( PaintUnit );		// NOT IMPLEMENTED!!!

    void	setViewXForm( bool );		// set xform on/off
    bool	hasViewXForm() const;
    QRect	window()       const;		// get window
    void	setWindow( const QRect & );	// set window
    void	setWindow( int x, int y, int w, int h );
    QRect	viewport()   const;		// get viewport
    void	setViewport( const QRect & );	// set viewport
    void	setViewport( int x, int y, int w, int h );

    void	setWorldXForm( bool );		// set world xform on/off
    bool	hasWorldXForm() const;
    const QWMatrix &worldMatrix() const;	// get/set world xform matrix
    void	setWorldMatrix( const QWMatrix &, bool combine=FALSE );

    void	translate( float dx, float dy );
    void	scale( float sx, float sy );
    void	shear( float sh, float sv );
    void	rotate( float a );
    void	resetXForm();

    QPoint	xForm( const QPoint & ) const;	// map virtual -> device
    QRect	xForm( const QRect & )	const;
    QPointArray xForm( const QPointArray & ) const;
    QPointArray xForm( const QPointArray &, int index, int npoints ) const;
    QPoint	xFormDev( const QPoint & ) const; // map device -> virtual
    QRect	xFormDev( const QRect & )  const;
    QPointArray xFormDev( const QPointArray & ) const;
    QPointArray xFormDev( const QPointArray &, int index, int npoints ) const;

  // Clipping

    void	setClipping( bool );		// set clipping on/off
    bool	hasClipping() const;
    const QRegion &clipRegion() const;
    void	setClipRect( const QRect & );	// set clip rectangle
    void	setClipRect( int x, int y, int w, int h );
    void	setClipRegion( const QRegion &);// set clip region

  // Graphics drawing functions

    void	drawPoint( int x, int y );
    void	drawPoint( const QPoint & );
    void	drawPoints( const QPointArray& a,
			    int index=0, int npoints=-1 );
    void	moveTo( int x, int y );
    void	moveTo( const QPoint & );
    void	lineTo( int x, int y );
    void	lineTo( const QPoint & );
    void	drawLine( int x1, int y1, int x2, int y2 );
    void	drawLine( const QPoint &, const QPoint & );
    void	drawRect( int x, int y, int w, int h );
    void	drawRect( const QRect & );
    void	drawWinFocusRect( int x, int y, int w, int h );
    void	drawWinFocusRect( int x, int y, int w, int h,
				  const QColor &bgColor );
    void	drawWinFocusRect( const QRect & );
    void	drawWinFocusRect( const QRect &,
				  const QColor &bgColor );
    void	drawRoundRect( int x, int y, int w, int h, int, int );
    void	drawRoundRect( const QRect &, int, int );
    void	drawEllipse( int x, int y, int w, int h );
    void	drawEllipse( const QRect & );
    void	drawArc( int x, int y, int w, int h, int a, int alen );
    void	drawArc( const QRect &, int a, int alen );
    void	drawPie( int x, int y, int w, int h, int a, int alen );
    void	drawPie( const QRect &, int a, int alen );
    void	drawChord( int x, int y, int w, int h, int a, int alen );
    void	drawChord( const QRect &, int a, int alen );
    void	drawLineSegments( const QPointArray &,
				  int index=0, int nlines=-1 );
    void	drawPolyline( const QPointArray &,
			      int index=0, int npoints=-1 );
    void	drawPolygon( const QPointArray &, bool winding=FALSE,
			     int index=0, int npoints=-1 );
    void	drawQuadBezier( const QPointArray &, int index=0 );
    void	drawPixmap( int x, int y, const QPixmap &,
			    int sx=0, int sy=0, int sw=-1, int sh=-1 );
    void	drawPixmap( const QPoint &, const QPixmap &,
			    const QRect &sr );
    void	drawPixmap( const QPoint &, const QPixmap & );
    void	drawImage( int x, int y, const QImage &,
			   int sx=0, int sy=0, int sw=-1, int sh=-1 );
    void	drawImage( const QPoint &, const QImage &,
			   const QRect &sr );
    void	drawImage( const QPoint &, const QImage & );
    void	drawTiledPixmap( int x, int y, int w, int h, const QPixmap &,
				 int sx=0, int sy=0 );
    void	drawTiledPixmap( const QRect &, const QPixmap &,
				 const QPoint & );
    void	drawTiledPixmap( const QRect &, const QPixmap & );
    void	drawPicture( const QPicture & );

    void	fillRect( int x, int y, int w, int h, const QBrush & );
    void	fillRect( const QRect &, const QBrush & );
    void	eraseRect( int x, int y, int w, int h );
    void	eraseRect( const QRect & );

  // Text drawing functions

    void	drawText( int x, int y, const char *str, int len = -1 );
    void	drawText( const QPoint &, const char *str, int len = -1 );
    void	drawText( int x, int y, int w, int h, int flags,
			  const char *str, int len = -1, QRect *br=0,
			  char **internal=0 );
    void	drawText( const QRect &, int flags,
			  const char *str, int len = -1, QRect *br=0,
			  char **internal=0 );

  // Text drawing functions

    QRect	boundingRect( int x, int y, int w, int h, int flags,
			      const char *str, int len = -1, char **intern=0 );
    QRect	boundingRect( const QRect &, int flags,
			      const char *str, int len = -1, char **intern=0 );

    int		tabStops() const;
    void	setTabStops( int );
    int	       *tabArray() const;
    void	setTabArray( int * );

    HANDLE	handle() const;

    static void initialize();
    static void cleanup();

private:
    void	init();
    void	updateFont();
    void	updatePen();
    void	updateBrush();
    void	updateXForm();
    void	updateInvXForm();
    void	map( int, int, int *rx, int *ry ) const;
    void	map( int, int, int, int, int *, int *, int *, int * ) const;
    void	mapInv( int, int, int *, int * ) const;
    void	mapInv( int, int, int, int, int *, int *, int *, int * ) const;
    void	drawPolyInternal( const QPointArray &, bool close=TRUE );
    void	drawWinFocusRect( int x, int y, int w, int h, bool xorPaint,
				  const QColor &penColor );

    enum { IsActive=0x01, ExtDev=0x02, IsStartingUp=0x04, NoCache=0x08,
	   VxF=0x10, WxF=0x20, ClipOn=0x40, SafePolygon=0x80, MonoDev=0x100,
	   DirtyFont=0x200, DirtyPen=0x400, DirtyBrush=0x800,
	   RGBColor=0x1000, FontMet=0x2000, FontInf=0x4000, CtorBegin=0x8000 };
    ushort	flags;
    bool	testf( ushort b ) const { return (flags&b)!=0; }
    void	setf( ushort b )	{ flags |= b; }
    void	setf( ushort b, bool v );
    void	clearf( ushort b )	{ flags &= (ushort)(~b); }
    void	fix_neg_rect( int *x, int *y, int *w, int *h );

    QPaintDevice *pdev;
    QColor	bg_col;
    uchar	bg_mode;
    uchar	rop;
    uchar	pu;
    QPoint	bro;
    QFont	cfont;
    QPen	cpen;
    QBrush	cbrush;
    QRegion	crgn;
    int		tabstops;
    int	       *tabarray;
    int		tabarraylen;
    QCOORD	wx, wy, ww, wh;
    QCOORD	vx, vy, vw, vh;
    QWMatrix	wxmat;
    int		wm11, wm12, wm21, wm22, wdx, wdy;
    int		im11, im12, im21, im22, idx, idy;
    int		txop;
    bool	txinv;
    void       *penRef;				// pen cache ref
    void       *brushRef;			// brush cache ref
    void       *ps_stack;
    void	killPStack();

protected:
#if defined(_WS_WIN_)
    HDC		hdc;				// device context
    HANDLE	hpen;				// current pen
    HANDLE	hbrush;				// current brush
    HANDLE	hbrushbm;			// current brush bitmap
    HANDLE	holdpal;
    uint	pixmapBrush	: 1;
    uint	nocolBrush	: 1;
    QWinFont   *winFont;
    void       *textMetric();
    void	nativeXForm( bool );
#elif defined(_WS_PM_)
    HPS		hps;				// presentation space
    int		dh;				// device height
    long	lctrl;				// outline/fill control
    void	updateCtrl();			// update lctrl
#elif defined(_WS_X11_)
    Display    *dpy;				// current display
    WId		hd;				// handle to drawable
    GC		gc;				// graphics context (standard)
    GC		gc_brush;			// graphics contect for brush
    QPoint	curPt;				// current point
#endif
    friend class QFontMetrics;
    friend class QFontInfo;
    friend void qt_format_text( const QFontMetrics& fm, int x, int y, int w, int h,
		     int tf, const char *str, int len, QRect *brect,
		     int tabstops, int* tabarray, int tabarraylen,
		     char **internal, QPainter* painter );

private:	// Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QPainter( const QPainter & );
    QPainter &operator=( const QPainter & );
#endif
};


/*****************************************************************************
  QPainter member functions
 *****************************************************************************/

inline QPaintDevice *QPainter::device() const
{
    return pdev;
}

inline bool QPainter::isActive() const
{
    return testf(IsActive);
}

inline QFontMetrics QPainter::fontMetrics() const
{
    return QFontMetrics(this);
}

inline QFontInfo QPainter::fontInfo() const
{
    return QFontInfo(this);
}

inline const QFont &QPainter::font() const
{
    return cfont;
}

inline const QPen &QPainter::pen() const
{
    return cpen;
}

inline const QBrush &QPainter::brush() const
{
    return cbrush;
}

/*
inline PaintUnit QPainter::unit() const
{
    return (PaintUnit)pu;
}
*/

inline const QColor &QPainter::backgroundColor() const
{
    return bg_col;
}

inline BGMode QPainter::backgroundMode() const
{
    return (BGMode)bg_mode;
}

inline RasterOp QPainter::rasterOp() const
{
    return (RasterOp)rop;
}

inline const QPoint &QPainter::brushOrigin() const
{
    return bro;
}

inline bool QPainter::hasViewXForm() const
{
    return testf(VxF);
}

inline bool QPainter::hasWorldXForm() const
{
    return testf(WxF);
}

inline bool QPainter::hasClipping() const
{
    return testf(ClipOn);
}

inline const QRegion &QPainter::clipRegion() const
{
    return crgn;
}

inline int QPainter::tabStops() const
{
    return tabstops;
}

inline int *QPainter::tabArray() const
{
    return tabarray;
}

inline HANDLE QPainter::handle() const
{
#if defined(_WS_WIN_)
    return hdc;
#elif defined(_WS_PM_)
    return hps;
#elif defined(_WS_X11_)
    return hd;
#endif
}


inline void QPainter::setBrushOrigin( const QPoint &p )
{
    setBrushOrigin( p.x(), p.y() );
}

inline void QPainter::setWindow( const QRect &r )
{
    setWindow( r.x(), r.y(), r.width(), r.height() );
}

inline void QPainter::setViewport( const QRect &r )
{
    setViewport( r.x(), r.y(), r.width(), r.height() );
}

inline void QPainter::setClipRect( int x, int y, int w, int h )
{
    setClipRect( QRect(x,y,w,h) );
}

inline void QPainter::drawPoint( const QPoint &p )
{
    drawPoint( p.x(), p.y() );
}

inline void QPainter::moveTo( const QPoint &p )
{
    moveTo( p.x(), p.y() );
}

inline void QPainter::lineTo( const QPoint &p )
{
    lineTo( p.x(), p.y() );
}

inline void QPainter::drawLine( const QPoint &p1, const QPoint &p2 )
{
    drawLine( p1.x(), p1.y(), p2.x(), p2.y() );
}

inline void QPainter::drawRect( const QRect &r )
{
    drawRect( r.x(), r.y(), r.width(), r.height() );
}

inline void QPainter::drawWinFocusRect( const QRect &r )
{
    drawWinFocusRect( r.x(), r.y(), r.width(), r.height() );
}

inline void QPainter::drawWinFocusRect( const QRect &r,const QColor &penColor )
{
    drawWinFocusRect( r.x(), r.y(), r.width(), r.height(), penColor );
}

inline void QPainter::drawRoundRect( const QRect &r, int xRnd, int yRnd )
{
    drawRoundRect( r.x(), r.y(), r.width(), r.height(), xRnd, yRnd );
}

inline void QPainter::drawEllipse( const QRect &r )
{
    drawEllipse( r.x(), r.y(), r.width(), r.height() );
}

inline void QPainter::drawArc( const QRect &r, int a, int alen )
{
    drawArc( r.x(), r.y(), r.width(), r.height(), a, alen );
}

inline void QPainter::drawPie( const QRect &r, int a, int alen )
{
    drawPie( r.x(), r.y(), r.width(), r.height(), a, alen );
}

inline void QPainter::drawChord( const QRect &r, int a, int alen )
{
    drawChord( r.x(), r.y(), r.width(), r.height(), a, alen );
}

inline void QPainter::drawPixmap( const QPoint &p, const QPixmap &pm,
				  const QRect &sr )
{
    drawPixmap( p.x(), p.y(), pm, sr.x(), sr.y(), sr.width(), sr.height() );
}

inline void QPainter::drawImage( const QPoint &p, const QImage &pm,
				 const QRect &sr )
{
    drawImage( p.x(), p.y(), pm, sr.x(), sr.y(), sr.width(), sr.height() );
}

inline void QPainter::drawTiledPixmap( const QRect &r, const QPixmap &pm,
				       const QPoint &sp )
{
    drawTiledPixmap( r.x(), r.y(), r.width(), r.height(), pm, sp.x(), sp.y() );
}

inline void QPainter::drawTiledPixmap( const QRect &r, const QPixmap &pm )
{
    drawTiledPixmap( r.x(), r.y(), r.width(), r.height(), pm, 0, 0 );
}

inline void QPainter::fillRect( const QRect &r, const QBrush &brush )
{
    fillRect( r.x(), r.y(), r.width(), r.height(), brush );
}

inline void QPainter::eraseRect( int x, int y, int w, int h )
{
    fillRect( x, y, w, h, backgroundColor() );
}

inline void QPainter::eraseRect( const QRect &r )
{
    fillRect( r.x(), r.y(), r.width(), r.height(), backgroundColor() );
}

inline void QPainter::drawText( const QPoint &p, const char *s, int len )
{
    drawText( p.x(), p.y(), s, len );
}

inline void QPainter::drawText( const QRect &r, int tf,
				const char *str, int len, QRect *br, char **i )
{
    drawText( r.x(), r.y(), r.width(), r.height(), tf, str, len, br, i );
}

inline QRect QPainter::boundingRect( const QRect &r, int tf,
				     const char *str, int len, char **i )
{
    return boundingRect( r.x(), r.y(), r.width(), r.height(), tf, str, len,
			 i );
}


#endif // QPAINTER_H
