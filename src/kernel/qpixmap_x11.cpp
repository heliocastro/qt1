/****************************************************************************
** $Id: qpixmap_x11.cpp,v 2.38.2.2 1999/01/27 18:52:20 warwick Exp $
**
** Implementation of QPixmap class for X11
**
** Created : 940501
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

// Uncomment the next line to enable the MIT Shared Memory extension
//
// WARNING:  This has some problems:
//
//    1. Consumes a 800x600 pixmap
//    2. Qt does not handle the ShmCompletion message, so you will
//        get strange effects if you xForm() repeatedly.
//
// #define MITSHM

#include "qbitmap.h"
#include "qimage.h"
#include "qpaintdevicedefs.h"
#include "qwmatrix.h"
#include "qapplication.h"
#include <stdlib.h>
#define	 GC GC_QQQ
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#ifdef MITSHM
#include <sys/ipc.h>
#include <sys/shm.h>
#include <X11/extensions/XShm.h>
#endif

// For thread-safety:
//   image->data does not belong to X11, so we must free it ourselves.

inline static void qSafeXDestroyImage( XImage *x )
{
    if ( x->data ) {
	free( x->data );
	x->data = 0;
    }
    XDestroyImage( x );
}


/*****************************************************************************
  MIT Shared Memory Extension support: makes xForm noticeably (~20%) faster.
 *****************************************************************************/

#if defined(MITSHM)

static bool	       xshminit = FALSE;
static XShmSegmentInfo xshminfo;
static XImage	      *xshmimg = 0;
static Pixmap	       xshmpm  = 0;

static void qt_cleanup_mitshm()
{
    if ( xshmimg == 0 )
	return;
    Display *dpy = QPaintDevice::x__Display();
    if ( xshmpm ) {
	XFreePixmap( dpy, xshmpm );
	xshmpm = 0;
    }
    XShmDetach( dpy, &xshminfo );
    qSafeXDestroyImage( xshmimg );
    xshmimg = 0;
    shmdt( xshminfo.shmaddr );
    shmctl( xshminfo.shmid, IPC_RMID, 0 );
}

bool qt_create_mitshm_buffer( QPaintDevice* dev, int w, int h )
{
    static int major, minor;
    static Bool pixmaps_ok;
    Display *dpy = dev->x11Display();
    int scr	 = dev->x11Screen();
    int dd	 = dev->x11Depth();
    Visual *vis	 = (Visual*)dev->x11Visual();

    if ( xshminit ) {
	qt_cleanup_mitshm();
    } else {
	if ( !XShmQueryVersion(dpy, &major, &minor, &pixmaps_ok) )
	    return FALSE;			// MIT Shm not supported
	qAddPostRoutine( qt_cleanup_mitshm );
	xshminit = TRUE;
    }

    xshmimg = XShmCreateImage( dpy, vis, dd, ZPixmap, 0, &xshminfo, w, h );
    if ( !xshmimg )
	return FALSE;

    bool ok;
    xshminfo.shmid = shmget( IPC_PRIVATE,
			     xshmimg->bytes_per_line * xshmimg->height,
			     IPC_CREAT | 0777 );
    ok = xshminfo.shmid != -1;
    if ( ok ) {
	xshminfo.shmaddr = xshmimg->data = shmat( xshminfo.shmid, 0, 0 );
	ok = xshminfo.shmaddr != 0;
    }
    xshminfo.readOnly = FALSE;
    if ( ok )
	ok = XShmAttach( dpy, &xshminfo );
    if ( !ok ) {
	qSafeXDestroyImage( xshmimg );
	if ( xshminfo.shmaddr )
	    shmdt( xshminfo.shmaddr );
	if ( xshminfo.shmid != -1 )
	    shmctl( xshminfo.shmid, IPC_RMID, 0 );
	return FALSE;
    }
    if ( pixmaps_ok )
	xshmpm = XShmCreatePixmap( dpy, DefaultRootWindow(dpy), xshmimg->data,
				   &xshminfo, w, h, dd );

    return TRUE;
}

#else

bool qt_create_mitshm_buffer( QPaintDevice*, int, int )
{
    return FALSE;
}

#endif // MITSHM


/*****************************************************************************
  Internal functions
 *****************************************************************************/

extern uchar *qt_get_bitflip_array();		// defined in qimage.cpp

static uchar *flip_bits( const uchar *bits, int len )
{
    register const uchar *p = bits;
    const uchar *end = p + len;
    uchar *newdata = new uchar[len];
    uchar *b = newdata;
    uchar *f = qt_get_bitflip_array();
    while ( p < end )
	*b++ = f[*p++];
    return newdata;
}

static int highest_bit( uint v )
{
    int i;
    uint b = (uint)1 << 31;			// get pos of highest set bit
    for ( i=31; ((b & v) == 0) && i>=0;	 i-- )
	b >>= 1;
    return i;
}

static uint n_bits( uint v )
{
    int i = 0;
    while ( v ) {
	v = v & (v - 1);
	i++;
    }
    return i;
}

static uint *red_scale_table   = 0;
static uint *green_scale_table = 0;
static uint *blue_scale_table  = 0;

static void cleanup_scale_tables()
{
    delete red_scale_table;
    delete green_scale_table;
    delete blue_scale_table;
}

/*
  Could do smart bitshifting, but the "obvious" algorithm only works for
  nBits >= 4. This is more robust.
*/
static void build_scale_table( uint **table, uint nBits )
{
    if ( nBits > 7 ) {
#if defined(CHECK_RANGE)
	warning( "build_scale_table: internal error, nBits = %i", nBits );
#endif
	return;
    }
    if (!*table) {
	static bool firstTable = TRUE;
	if ( firstTable ) {
	    qAddPostRoutine( cleanup_scale_tables );
	    firstTable = FALSE;
	}	
	*table = new uint[256];
    }
    int   maxVal   = (1 << nBits) - 1;
    int   valShift = 8 - nBits;
    int i;
    for( i = 0 ; i < maxVal + 1 ; i++ )
	(*table)[i << valShift] = i*255/maxVal;
}

/*****************************************************************************
  QPixmap member functions
 *****************************************************************************/

bool QPixmap::optimAll = TRUE;
QPixmap::Optimization QPixmap::defOpt = QPixmap::NormalOptim;


/*!
  \internal
  Initializes the pixmap data.
*/

void QPixmap::init( int w, int h, int d )
{
    static int serial = 0;
    int dd = x11Depth();

    data = new QPixmapData;
    CHECK_PTR( data );

    data->optim	   = optimAll;
    data->uninit   = TRUE;
    data->bitmap   = FALSE;
    data->selfmask = FALSE;
    data->ser_no   = ++serial;
    data->opt	   = defOpt;
    data->mask	   = 0;
    data->ximage   = 0;
    data->maskgc   = 0;

    bool make_null = w == 0 || h == 0;		// create null pixmap
    if ( d == 1 )				// monocrome pixmap
	data->d = 1;
    else if ( d < 0 || d == dd )		// def depth pixmap
	data->d = dd;
    else					// invalid depth
	data->d = 0;
    if ( make_null || w < 0 || h < 0 || data->d == 0 ) {
	data->w = data->h = 0;
	data->d = 0;
	hd = 0;
#if defined(CHECK_RANGE)
	if ( !make_null )
	    warning( "QPixmap: Invalid pixmap parameters" );
#endif
	return;
    }
    data->w = w;
    data->h = h;
    hd = (HANDLE)XCreatePixmap( dpy, DefaultRootWindow(dpy), w, h, data->d );
}


void QPixmap::deref()
{
    if ( data && data->deref() ) {			// last reference lost
	if ( data->mask ) {
	    delete data->mask;
	    data->mask = 0;
	}
	if ( data->maskgc ) {
	    XFreeGC( dpy, (GC)data->maskgc );
	    data->maskgc = 0;
	}
	if ( data->ximage ) {
	    qSafeXDestroyImage( (XImage*)data->ximage );
	    data->ximage = 0;
	}
	if ( hd && qApp ) {
	    XFreePixmap( dpy, hd );
	    hd = 0;
	}
	delete data;
    }
}


/*!
  Constructs a monochrome pixmap which is initialized with the data in \e bits.
  This constructor is protected and used by the QBitmap class.
*/

QPixmap::QPixmap( int w, int h, const uchar *bits, bool isXbitmap )
    : QPaintDevice( PDT_PIXMAP )
{						// for bitmaps only
    init( 0, 0, 0 );
    if ( w <= 0 || h <= 0 )			// create null pixmap
	return;

    data->uninit = FALSE;
    data->w = w;
    data->h = h;
    data->d = 1;
    uchar *flipped_bits;
    if ( isXbitmap ) {
	flipped_bits = 0;
    } else {					// not X bitmap -> flip bits
	flipped_bits = flip_bits( bits, ((w+7)/8)*h );
	bits = flipped_bits;
    }
    hd = (HANDLE)XCreateBitmapFromData( dpy, DefaultRootWindow(dpy),
				(char *)bits, w, h );
    if ( flipped_bits )				// Avoid purify complaint
	delete [] flipped_bits;
}

/*!
  Constructs a pixmap which is a copy of \e pixmap.
*/

QPixmap::QPixmap( const QPixmap &pixmap )
    : QPaintDevice( PDT_PIXMAP )
{
    if ( pixmap.paintingActive() ) {		// make a deep copy
	data = 0;
	operator=( pixmap );
    } else {
	data = pixmap.data;
	data->ref();
	devFlags = pixmap.devFlags;		// copy QPaintDevice flags
	hd = pixmap.hd;				// copy QPaintDevice drawable
    }
}

/*!
  Destroys the pixmap.
*/

QPixmap::~QPixmap()
{
    deref();
}


/*!
  Special-purpose function that detaches the pixmap from shared pixmap data.

  A pixmap is automatically detached by Qt whenever its contents is about
  to change.  This is done in all QPixmap member functions that modify the
  pixmap (fill(), resize(), convertFromImage(), load() etc.), in bitBlt()
  for the destination pixmap and in QPainter::begin() on a pixmap.

  It is possible to modify a pixmap without letting Qt know.
  You can first obtain the \link handle() system-dependent handle\endlink
  and then call system-specific functions (for instance BitBlt under Windows)
  that modifies the pixmap contents.  In this case, you can call detach()
  to cut the pixmap loose from other pixmaps that share data with this one.

  detach() returns immediately if there is just a single reference or if
  the pixmap has not been initialized yet.
*/

void QPixmap::detach()
{
    if ( data->uninit || data->count == 1 )
	data->uninit = FALSE;
    else
	*this = copy();
    // reset the cache data
    if ( data->ximage ) {
	qSafeXDestroyImage( (XImage*)data->ximage );
	data->ximage = 0;
    }
    if ( data->maskgc ) {
	XFreeGC( dpy, (GC)data->maskgc );
	data->maskgc = 0;
    }
}


/*!
  Assigns the pixmap \e pixmap to this pixmap and returns a reference to
  this pixmap.
*/

QPixmap &QPixmap::operator=( const QPixmap &pixmap )
{
    if ( paintingActive() ) {
#if defined(CHECK_STATE)
	warning("QPixmap::operator=: Cannot assign to pixmap during painting");
#endif
	return *this;
    }
    pixmap.data->ref();				// avoid 'x = x'
    deref();

    if ( pixmap.paintingActive() ) {		// make a deep copy
	init( pixmap.width(), pixmap.height(), pixmap.depth() );
	data->uninit = FALSE;
	data->optim  = pixmap.data->optim;	// copy optimization flag
	data->bitmap = pixmap.data->bitmap;	// copy bitmap flag
	data->opt    = pixmap.data->opt;	// copy optimization setting
	if ( !isNull() ) {
	    bitBlt( this, 0, 0, &pixmap, pixmap.width(), pixmap.height(),
		    CopyROP, TRUE );
	    if ( pixmap.mask() )
		setMask( *pixmap.mask() );
	}
	pixmap.data->deref();
    } else {
	data = pixmap.data;
	devFlags = pixmap.devFlags;		// copy QPaintDevice flags
	hd = pixmap.hd;				// copy QPaintDevice drawable
    }
    return *this;
}


/*!
  Returns the default pixmap depth, i.e. the depth a pixmap gets
  if -1 is specified.
  \sa depth()
*/

int QPixmap::defaultDepth()
{
    return x11Depth();
}


/*!
  \fn QPixmap::Optimization QPixmap::optimization() const

  Returns the optimization setting for this pixmap.

  The default optimization setting is \c QPixmap::NormalOptim. You may
  change this settings in two ways:
  <ul>
  <li> Call setDefaultOptimization() to set the default optimization
  for all new pixmaps.
  <li> Call setOptimization() to set a the optimization for individual
  pixmaps.
  </ul>

  \sa setOptimization(), setDefaultOptimization(), defaultOptimization()
*/

/*!
  Sets pixmap drawing optimization for this pixmap.

  The optimization setting affects pixmap operations, in particular
  drawing of transparent pixmaps (bitBlt() a pixmap with a mask set) and
  pixmap transformations (the xForm() function).

  Pixmap optimization involves keeping intermediate results in a cache
  buffer and use the data in the cache to speed up bitBlt() and xForm().
  The cost is more memory consumption, up to twice as much as an
  unoptimized pixmap.

  The \a optimization parameter can be:
  <ul>
  <li> \c QPixmap::NoOptim, avoid optimization. Little or no caching is
  done. Use this setting if memory is scarce and the speed of pixmap
  operations is not critical to your application.
  <li> \c QPixmap::NormalOptim, normal optimization to make pixmap drawing
  faster. This option is the default and is suitable for most purposes.
  <li> \c QPixmap::BestOptim, heavily optimized pixmap drawing. Use this
  option for pixmap drawn extremely frequently.
  </ul>

  Use the setDefaultOptimization() to change the default optimization
  for all new pixmaps.

  \sa optimization(), setDefaultOptimization(), defaultOptimization()
*/

void QPixmap::setOptimization( Optimization optimization )
{
    if ( optimization == data->opt )
	return;
    data->optim = optimization != NoOptim;	// ### compatibility setting
    data->opt = optimization;
    if ( optimization == NoOptim && data->ximage ) {
	qSafeXDestroyImage( (XImage*)data->ximage );
	data->ximage = 0;
    }
}

/*!
  Returns the default pixmap optimization setting.
  \sa setDefaultOptimization(), setOptimization(), optimization()
*/

QPixmap::Optimization QPixmap::defaultOptimization()
{
    return defOpt;
}

/*!
  Sets the default pixmap optimization.

  All \e new pixmaps that are created will use this default optimization.
  You may also set optimization for individual pixmaps using the
  setOptimization() function.

  The initial default optimization setting is \c QPixmap::Normal.

  \sa defaultOptimization(), setOptimization(), optimization()
*/

void QPixmap::setDefaultOptimization( Optimization optimization )
{
    defOpt = optimization;
    optimAll = optimization != NoOptim;		// ### compatibility setting
}


/*!
  \fn bool QPixmap::isOptimized() const
  Deprecated function, replaced by optimization();
*/

/*!
  Deprecated function, replaced by setOptimization().
*/

void QPixmap::optimize( bool enable )
{
    setOptimization( enable ? NormalOptim : NoOptim );
}

/*!
  Deprecated function, replaced by defaultOptimization().
*/

bool QPixmap::isGloballyOptimized()
{
    return defOpt != NoOptim;
}

/*!
  Deprecated function, replaced by setDefaultOptimization.
*/

void QPixmap::optimizeGlobally( bool enable )
{
    setDefaultOptimization( enable ? NormalOptim : NoOptim );
}


/*!
  Fills the pixmap with the color \e fillColor.
*/

void QPixmap::fill( const QColor &fillColor )
{
    if ( isNull() )
	return;
    detach();					// detach other references
    GC gc = qt_xget_temp_gc( depth()==1 );
    XSetForeground( dpy, gc, fillColor.pixel() );
    XFillRectangle( dpy, hd, gc, 0, 0, width(), height() );
}


/*!
  Internal implementation of the virtual QPaintDevice::metric() function.

  Use the QPaintDeviceMetrics class instead.
*/

int QPixmap::metric( int m ) const
{
    int val;
    if ( m == PDM_WIDTH || m == PDM_HEIGHT ) {
	if ( m == PDM_WIDTH )
	    val = width();
	else
	    val = height();
    } else {
	int scr = qt_xscreen();
	switch ( m ) {
	    case PDM_WIDTHMM:
		val = (DisplayWidthMM(dpy,scr)*width())/
		      DisplayWidth(dpy,scr);
		break;
	    case PDM_HEIGHTMM:
		val = (DisplayHeightMM(dpy,scr)*height())/
		      DisplayHeight(dpy,scr);
		break;
	    case PDM_NUMCOLORS:
		val = 1 << depth();
		break;
	    case PDM_DEPTH:
		val = depth();
		break;
	    default:
		val = 0;
#if defined(CHECK_RANGE)
		warning( "QPixmap::metric: Invalid metric command" );
#endif
	}
    }
    return val;
}

/*!
  Converts the pixmap to an image. Returns a null image if the operation
  failed.

  If the pixmap has 1 bit depth, the returned image will also be 1
  bits deep.  If the pixmap has 2-8 bit depth, the returned image
  has 8 bit depth.  If the pixmap has greater than 8 bit depth, the
  returned image has 32 bit depth.

  \bug Does not support 2 or 4 bit display hardware.

  \bug Alpha masks on monochrome images are ignored.

  \sa convertFromImage()
*/

QImage QPixmap::convertToImage() const
{
    QImage image;
    if ( isNull() ) {
#if defined(CHECK_NULL)
	warning( "QPixmap::convertToImage: Cannot convert a null pixmap" );
#endif
	return image;
    }

    int	    w  = width();
    int	    h  = height();
    int	    d  = depth();
    bool    mono = d == 1;
    Visual *visual = (Visual *)x11Visual();
    bool    trucol = (visual->c_class == TrueColor) && !mono;

    if ( d > 1 && d <= 8 )			// set to nearest valid depth
	d = 8;					//   2..8 ==> 8
    else if ( d > 8 || trucol )
	d = 32;					//   > 8  ==> 32

    XImage *xi = (XImage *)data->ximage;	// any cached ximage?
    if ( !xi )					// fetch data from X server
	xi = XGetImage( dpy, hd, 0, 0, w, h, AllPlanes,
			mono ? XYPixmap : ZPixmap );
    CHECK_PTR( xi );

    QImage::Endian bitOrder = QImage::IgnoreEndian;
    if ( mono ) {
	bitOrder = xi->bitmap_bit_order == LSBFirst ?
	    QImage::LittleEndian : QImage::BigEndian;
    }
    image.create( w, h, d, 0, bitOrder );
    if ( image.isNull() )			// could not create image
	return image;

    const QBitmap* msk = mask();

    QImage alpha;
    if (msk) {
	image.setAlphaBuffer( TRUE );
	alpha = msk->convertToImage();
    }
    bool ale = alpha.bitOrder() == QImage::LittleEndian;

    if ( trucol ) {				// truecolor
	uint red_mask	 = (uint)visual->red_mask;
	uint green_mask	 = (uint)visual->green_mask;
	uint blue_mask	 = (uint)visual->blue_mask;
	int  red_shift	 = highest_bit( red_mask )   - 7;
	int  green_shift = highest_bit( green_mask ) - 7;
	int  blue_shift	 = highest_bit( blue_mask )  - 7;

	uint red_bits    = n_bits( red_mask );
	uint green_bits  = n_bits( green_mask );
	uint blue_bits   = n_bits( blue_mask );

	static uint red_table_bits   = 0;
	static uint green_table_bits = 0;
	static uint blue_table_bits  = 0;

	if ( red_bits < 8 && red_table_bits != red_bits) {
	    build_scale_table( &red_scale_table, red_bits );
	    red_table_bits = red_bits;
	}
	if ( blue_bits < 8 && blue_table_bits != blue_bits) {
	    build_scale_table( &blue_scale_table, blue_bits );
	    blue_table_bits = blue_bits;
	}
	if ( green_bits < 8 && green_table_bits != green_bits) {
	    build_scale_table( &green_scale_table, green_bits );
	    green_table_bits = green_bits;
	}

	int  r, g, b;

	QRgb  *dst;
	uchar *src;
	uint   pixel;
	int    bppc = xi->bits_per_pixel;

	if ( bppc > 8 && xi->byte_order == LSBFirst )
	    bppc++;

	for ( int y=0; y<h; y++ ) {
	    uchar* asrc = msk ? alpha.scanLine( y ) : 0;
	    dst = (QRgb *)image.scanLine( y );
	    src = (uchar *)xi->data + xi->bytes_per_line*y;
	    for ( int x=0; x<w; x++ ) {
		switch ( bppc ) {
		    case 8:
			pixel = *src++;
			break;
		    case 16:			// 16 bit MSB
			pixel = src[1] | (ushort)src[0] << 8;
			src += 2;
			break;
		    case 17:			// 16 bit LSB
			pixel = src[0] | (ushort)src[1] << 8;
			src += 2;
			break;
		    case 24:			// 24 bit MSB
			pixel = src[2] | (ushort)src[1] << 8 |
				(uint)src[0] << 16;
			src += 3;
			break;
		    case 25:			// 24 bit LSB
			pixel = src[0] | (ushort)src[1] << 8 |
				(uint)src[2] << 16;
			src += 3;
			break;
		    case 32:			// 32 bit MSB
			pixel = src[3] | (ushort)src[2] << 8 |
				(uint)src[1] << 16 | (uint)src[0] << 24;
			src += 4;
			break;
		    case 33:			// 32 bit LSB
			pixel = src[0] | (ushort)src[1] << 8 |
				(uint)src[2] << 16 | (uint)src[3] << 24;
			src += 4;
			break;
		    default:			// should not really happen
			x = w;			// leave loop
			y = h;
			pixel = 0;		// eliminate compiler warning
#if defined(CHECK_RANGE)
			warning( "QPixmap::convertToImage: Invalid depth %d",
				 bppc );
#endif
		}
		if ( red_shift > 0 )
		    r = (pixel & red_mask) >> red_shift;
		else
		    r = (pixel & red_mask) << -red_shift;
		if ( green_shift > 0 )
		    g = (pixel & green_mask) >> green_shift;
		else
		    g = (pixel & green_mask) << -green_shift;
		if ( blue_shift > 0 )
		    b = (pixel & blue_mask) >> blue_shift;
		else
		    b = (pixel & blue_mask) << -blue_shift;

		if ( red_bits < 8 )
		    r = red_scale_table[r];
		if ( green_bits < 8 )
		    g = green_scale_table[g];
		if ( blue_bits < 8 )
		    b = blue_scale_table[b];

		if (msk) {
		    if ( ale ) {
			*dst++ = (asrc[x >> 3] & (1 << (x & 7)))
			    ? (0xff000000 | qRgb(r, g, b)) : qRgb(r, g, b);
		    } else {
			*dst++ = (asrc[x >> 3] & (1 << (7 -(x & 7))))
			    ? (0xff000000 | qRgb(r, g, b)) : qRgb(r, g, b);
		    }
		} else {
		    *dst++ = qRgb(r, g, b);
		}
	    }
	}
    } else if ( xi->bits_per_pixel == d ) {	// compatible depth
	char *xidata = xi->data;		// copy each scanline
	int bpl = QMIN(image.bytesPerLine(),xi->bytes_per_line);
	for ( int y=0; y<h; y++ ) {
	    memcpy( image.scanLine(y), xidata, bpl );
	    xidata += xi->bytes_per_line;
	}
    } else {
	/* Typically 2 or 4 bits display depth */
#if defined(CHECK_RANGE)
	warning( "QPixmap::convertToImage: Display not supported (bpp=%d)",
		 xi->bits_per_pixel );
#endif
	image.reset();
	return image;
    }

    if ( mono ) {				// bitmap
	image.setNumColors( 2 );
	image.setColor( 0, qRgb(255,255,255) );
	image.setColor( 1, qRgb(0,0,0) );
    } else if ( !trucol ) {			// pixmap with colormap
	register uchar *p;
	uchar *end;
	uchar  use[256];			// pixel-in-use table
	uchar  pix[256];			// pixel translation table
	int    ncols, i, bpl;
	memset( use, 0, 256 );
	memset( pix, 0, 256 );
	bpl = image.bytesPerLine();

	if (msk) {				// which pixels are used?
	    for ( i=0; i<h; i++ ) {
		uchar* asrc = alpha.scanLine( i );
		p = image.scanLine( i );
		for ( int x = 0; x < w; x++ ) {
		    if ( ale ) {
			if (asrc[x >> 3] & (1 << (x & 7)))
			    use[*p] = 1;
		    } else {
			if (asrc[x >> 3] & (1 << (7 -(x & 7))))
			    use[*p] = 1;
		    }
		    ++p;
		}
	    }
	} else {
	    for ( i=0; i<h; i++ ) {
		p = image.scanLine( i );
		end = p + bpl;
		while ( p < end )
		    use[*p++] = 1;
	    }
	}
	ncols = 0;
	for ( i=0; i<256; i++ ) {		// build translation table
	    if ( use[i] )
		pix[i] = ncols++;
	}
	for ( i=0; i<h; i++ ) {			// translate pixels
	    p = image.scanLine( i );
	    end = p + bpl;
	    while ( p < end ) {
		*p = pix[*p];
		p++;
	    }
	}

	Colormap cmap	= x11Colormap();
	int	 ncells = x11Cells();
	XColor *carr = new XColor[ncells];
	for ( i=0; i<ncells; i++ )
	    carr[i].pixel = i;
	XQueryColors( dpy, cmap, carr, ncells );// get default colormap

	if (msk) {
	    int trans;
	    if (ncols < 256) {
		trans = ncols++;
		image.setNumColors( ncols );	// create color table
		image.setColor( trans, 0x00000000 );
	    } else {
		image.setNumColors( ncols );	// create color table
		// oh dear... no spare "transparent" pixel.
		// use first pixel in image (as good as any).
		trans = image.scanLine( i )[0];
	    }
	    for ( i=0; i<h; i++ ) {
		uchar* asrc = alpha.scanLine( i );
		p = image.scanLine( i );
		for ( int x = 0; x < w; x++ ) {
		    if ( ale ) {
			if (!(asrc[x >> 3] & (1 << (x & 7))))
			    *p = trans;
		    } else {
			if (!(asrc[x >> 3] & (1 << (7 -(x & 7)))))
			    *p = trans;
		    }
		    ++p;
		}
	    }
	} else {
	    image.setNumColors( ncols );	// create color table
	}
	int j = 0;
	for ( i=0; i<256; i++ ) {		// translate pixels
	    if ( use[i] ) {
		image.setColor( j++,
				 ( msk ? 0xff000000 : 0 )
				 | qRgb( (carr[i].red   >> 8) & 255,
			                 (carr[i].green >> 8) & 255,
			                 (carr[i].blue  >> 8) & 255 ) );
	    }
	}
	delete [] carr;
    }
    if ( data->opt == NoOptim )			// throw away image data
	qSafeXDestroyImage( xi );
    else					// keep ximage data
	((QPixmap*)this)->data->ximage = xi;
    return image;
}


/*!
  Converts an image and sets this pixmap. Returns TRUE if successful.

  The \a conversion_flags argument is a bitwise-OR from the following choices.
  The options marked \e (default) are the choice if no other choice from the
  list is included (they are zero):

  <dl>
   <dt>Color/Mono preference (ignored for QBitmap)
   <dd>
    <ul>
     <li> \c AutoColor (default) - If the \e image has \link
               QImage::depth() depth\endlink 1 and contains only
               black and white pixels, then the pixmap becomes monochrome.
     <li> \c ColorOnly - The pixmap is dithered/converted to the
               \link defaultDepth() native display depth\endlink.
     <li> \c MonoOnly - The pixmap becomes monochrome.  If necessary,
               it is dithered using the chosen dithering algorithm.
    </ul>
   <dt>Dithering mode preference, for RGB channels
   <dd>
    <ul>
     <li> \c DiffuseDither (default) - a high quality dither
     <li> \c OrderedDither - a faster more ordered dither
     <li> \c ThresholdDither - no dithering, closest color is used
    </ul>
   <dt>Dithering mode preference, for alpha channel
   <dd>
    <ul>
     <li> \c DiffuseAlphaDither - a high quality dither
     <li> \c OrderedAlphaDither - a faster more ordered dither
     <li> \c ThresholdAlphaDither (default) - no dithering
    </ul>
   <dt>Color matching versus dithering preference
   <dd>
    <ul>
     <li> \c PreferDither - always dither 32-bit images when
		the image
		is being converted to 8-bits.
		This is the default when converting to a pixmap.
     <li> \c AvoidDither - only dither 32-bit images if
		the image
		has more than 256 colours and it
		is being converted to 8-bits.
		This is the default when an image is converted
		for the purpose of saving to a file.
    </ul>
  </dl>

  Passing 0 for \a conversion_flags gives all the default options.

  Note that even though a QPixmap with depth 1 behaves much like a
  QBitmap, isQBitmap() returns FALSE.

  \bug Does not support 2 or 4 bit display hardware.

  \sa convertToImage(), isQBitmap(), QImage::convertDepth(), defaultDepth(),
    hasAlphaBuffer()
*/

bool QPixmap::convertFromImage( const QImage &img, int conversion_flags )
{
    if ( img.isNull() ) {
#if defined(CHECK_NULL)
	warning( "QPixmap::convertFromImage: Cannot convert a null image" );
#endif
	return FALSE;
    }
    detach();					// detach other references
    QImage  image = img;
    int	 w   = image.width();
    int	 h   = image.height();
    int	 d   = image.depth();
    int	 dd  = defaultDepth();
    bool force_mono = (dd == 1 || isQBitmap() ||
		       (conversion_flags & ColorMode_Mask)==MonoOnly );

    if ( data->mask ) {				// get rid of the mask
	delete data->mask;
	data->mask = 0;
    }
    if ( force_mono ) {				// must be monochrome
	if ( d != 1 ) {
	    image = image.convertDepth( 1, conversion_flags );	// dither
	    d = 1;
	}
    } else {					// can be both
	bool conv8 = FALSE;
	if ( d > 8 && dd <= 8 ) {		// convert to 8 bit
	    if ( (conversion_flags & DitherMode_Mask) == AutoDither )
		conversion_flags = (conversion_flags & ~DitherMode_Mask)
					| PreferDither;
	    conv8 = TRUE;
	} else if ( (conversion_flags & ColorMode_Mask) == ColorOnly ) {
	    conv8 = d == 1;			// native depth wanted
	} else if ( d == 1 ) {
	    if ( image.numColors() == 2 ) {
		QRgb c0 = image.color(0);	// Auto: convert to best
		QRgb c1 = image.color(1);
		conv8 = QMIN(c0,c1) != 0 || QMAX(c0,c1) != qRgb(255,255,255);
	    } else {
		// eg. 1-colour monochrome images (they do exist).
		conv8 = TRUE;
	    }
	}
	if ( conv8 ) {
	    image = image.convertDepth( 8, conversion_flags );
	    d = 8;
	}
    }

    if ( d == 1 ) {				// 1 bit pixmap (bitmap)
	if ( hd )				// delete old X pixmap
	    XFreePixmap( dpy, hd );
	char  *bits;
	uchar *tmp_bits;
	int    bpl = (w+7)/8;
	int    ibpl = image.bytesPerLine();
	if ( image.bitOrder() == QImage::BigEndian || bpl != ibpl ) {
	    tmp_bits = new uchar[bpl*h];
	    bits = (char *)tmp_bits;
	    uchar *p, *b, *end;
	    int y;
	    if ( image.bitOrder() == QImage::BigEndian ) {
		uchar *f = (uchar *)qt_get_bitflip_array();
		b = tmp_bits;
		for ( y=0; y<h; y++ ) {
		    p = image.scanLine( y );
		    end = p + bpl;
		    while ( p < end )
			*b++ = f[*p++];
		}
	    } else {				// just copy
		b = tmp_bits;
		p = image.scanLine( 0 );
		for ( y=0; y<h; y++ ) {
		    memcpy( b, p, bpl );
		    b += bpl;
		    p += ibpl;
		}
	    }
	} else {
	    bits = (char *)image.bits();
	    tmp_bits = 0;
	}
	hd = (HANDLE)XCreateBitmapFromData( dpy, DefaultRootWindow(dpy), bits, w, h );
	if ( tmp_bits )				// Avoid purify complaint
	    delete [] tmp_bits;
	data->w = w;  data->h = h;  data->d = 1;

	if ( image.hasAlphaBuffer() ) {
	    QBitmap m;
	    m = image.createAlphaMask( conversion_flags );
	    setMask( m );
	}
	return TRUE;
    }

    Visual *visual = (Visual *)x11Visual();
    XImage *xi	   = 0;
    bool    trucol = visual->c_class == TrueColor;
    int	    nbytes = image.numBytes();
    uchar  *newbits= 0;
    register uchar *p;

    if ( trucol ) {				// truecolor display
	QRgb  pix[256];				// pixel translation table
	bool  d8 = d == 8;
	uint  red_mask	  = (uint)visual->red_mask;
	uint  green_mask  = (uint)visual->green_mask;
	uint  blue_mask	  = (uint)visual->blue_mask;
	int   red_shift	  = highest_bit( red_mask )   - 7;
	int   green_shift = highest_bit( green_mask ) - 7;
	int   blue_shift  = highest_bit( blue_mask )  - 7;
	int   r, g, b;

	if ( d8 ) {				// setup pixel translation
	    QRgb *ctable = image.colorTable();
	    for ( int i=0; i<image.numColors(); i++ ) {
		r = qRed  (ctable[i]);
		g = qGreen(ctable[i]);
		b = qBlue (ctable[i]);
		r = red_shift	> 0 ? r << red_shift   : r >> -red_shift;
		g = green_shift > 0 ? g << green_shift : g >> -green_shift;
		b = blue_shift	> 0 ? b << blue_shift  : b >> -blue_shift;
		pix[i] = (b & blue_mask) | (g & green_mask) | (r & red_mask);
	    }
	}

	xi = XCreateImage( dpy, visual, dd, ZPixmap, 0, 0, w, h, 32, 0 );
	CHECK_PTR( xi );
	newbits = (uchar *)malloc( xi->bytes_per_line*h );
	uchar *src;
	uchar *dst;
	QRgb   pixel;
	int    bppc = xi->bits_per_pixel;

	if ( bppc > 8 && xi->byte_order == LSBFirst )
	    bppc++;

	for ( int y=0; y<h; y++ ) {
	    QRgb *p;
	    src = image.scanLine( y );
	    dst = newbits + xi->bytes_per_line*y;
	    p	= (QRgb *)src;
	    for ( int x=0; x<w; x++ ) {
		if ( d8 ) {
		    pixel = pix[*src++];
		} else {
		    r = qRed  ( *p );
		    g = qGreen( *p );
		    b = qBlue ( *p++ );
		    r = red_shift   > 0 ? r << red_shift   : r >> -red_shift;
		    g = green_shift > 0 ? g << green_shift : g >> -green_shift;
		    b = blue_shift  > 0 ? b << blue_shift  : b >> -blue_shift;
		    pixel = (b & blue_mask)|(g & green_mask) | (r & red_mask);
		}
		switch ( bppc ) {
		    case 8:
			*dst++ = pixel;
			break;
		    case 16:			// 16 bit MSB
			*dst++ = (pixel >> 8);
			*dst++ = pixel;
			break;
		    case 17:			// 16 bit LSB
			*dst++ = pixel;
			*dst++ = pixel >> 8;
			break;
		    case 24:			// 24 bit MSB
			*dst++ = pixel >> 16;
			*dst++ = pixel >> 8;
			*dst++ = pixel;
			break;
		    case 25:			// 24 bit LSB
			*dst++ = pixel;
			*dst++ = pixel >> 8;
			*dst++ = pixel >> 16;
			break;
		    case 32:			// 32 bit MSB
			*dst++ = pixel >> 24;
			*dst++ = pixel >> 16;
			*dst++ = pixel >> 8;
			*dst++ = pixel;
			break;
		    case 33:			// 32 bit LSB
			*dst++ = pixel;
			*dst++ = pixel >> 8;
			*dst++ = pixel >> 16;
			*dst++ = pixel >> 24;
			break;
		}
	    }
	}
	xi->data = (char *)newbits;
    }

    if ( d == 8 && !trucol ) {			// 8 bit pixmap
	int  i, j;
	int  pop[256];				// pixel popularity
	memset( pop, 0, sizeof(int)*256 );	// reset popularity array
	for ( i=0; i<h; i++ ) {			// for each scanline...
	    p = image.scanLine( i );
	    uchar *end = p + w;
	    while ( p < end )			// compute popularity
		pop[*p++]++;
	}

	newbits = (uchar *)malloc( nbytes );	// copy image into newbits
	if ( !newbits )				// no memory
	    return FALSE;
	p = newbits;
	memcpy( p, image.bits(), nbytes );	// copy image data into newbits

/*
 * The code below picks the most important colors. It is based on the
 * diversity algorithm, implemented in XV 3.10. XV is (C) by John Bradley.
 */

	struct PIX {				// pixel sort element
	    uchar r,g,b,n;			// color + pad
	    int	  use;				// popularity
	    int	  index;			// index in colormap
	    int	  mindist;
	};
	int ncols = 0;
	for ( i=0; i<image.numColors(); i++ ) { // compute number of colors
	    if ( pop[i] > 0 )
		ncols++;
	}
	for ( i=image.numColors(); i<256; i++ ) // ignore out-of-range pixels
	    pop[i] = 0;

	PIX *pixarr	   = new PIX[ncols];	// pixel array
	PIX *pixarr_sorted = new PIX[ncols];	// pixel array (sorted)
	PIX *px		   = &pixarr[0];
	int  maxpop = 0;
	int  maxpix = 0;
	CHECK_PTR( pixarr );
	j = 0;
	QRgb* ctable = image.colorTable();
	for ( i=0; i<256; i++ ) {		// init pixel array
	    if ( pop[i] > 0 ) {
		px->r = qRed  ( ctable[i] );
		px->g = qGreen( ctable[i] );
		px->b = qBlue ( ctable[i] );
		px->n = 0;
		px->use = pop[i];
		if ( pop[i] > maxpop ) {	// select most popular entry
		    maxpop = pop[i];
		    maxpix = j;
		}
		px->index = i;
		px->mindist = 1000000;
		px++;
		j++;
	    }
	}
	memcpy( &pixarr_sorted[0], &pixarr[maxpix], sizeof(PIX) );
	pixarr[maxpix].use = 0;

	for ( i=1; i<ncols; i++ ) {		// sort pixels
	    int minpix = -1, mindist = -1;
	    px = &pixarr_sorted[i-1];
	    int r = px->r;
	    int g = px->g;
	    int b = px->b;
	    int dist;
	    if ( (i & 1) || i<10 ) {		// sort on max distance
		for ( j=0; j<ncols; j++ ) {
		    px = &pixarr[j];
		    if ( px->use ) {
			dist = (px->r - r)*(px->r - r) +
			       (px->g - g)*(px->g - g) +
			       (px->b - b)*(px->b - b);
			if ( px->mindist > dist )
			    px->mindist = dist;
			if ( px->mindist > mindist ) {
			    mindist = px->mindist;
			    minpix = j;
			}
		    }
		}
	    } else {				// sort on max popularity
		for ( j=0; j<ncols; j++ ) {
		    px = &pixarr[j];
		    if ( px->use ) {
			dist = (px->r - r)*(px->r - r) +
			       (px->g - g)*(px->g - g) +
			       (px->b - b)*(px->b - b);
			if ( px->mindist > dist )
			    px->mindist = dist;
			if ( px->use > mindist ) {
			    mindist = px->use;
			    minpix = j;
			}
		    }
		}
	    }
	    memcpy( &pixarr_sorted[i], &pixarr[minpix], sizeof(PIX) );
	    pixarr[minpix].use = 0;
	}

	uint pix[256];				// pixel translation table
	px = &pixarr_sorted[0];
	for ( i=0; i<ncols; i++ ) {		// allocate colors
	    QColor c( px->r, px->g, px->b );
	    pix[px->index] = c.pixel();
	    px++;
	}
	delete [] pixarr;
	delete [] pixarr_sorted;

	p = newbits;
	for ( i=0; i<nbytes; i++ ) {		// translate pixels
	    *p = pix[*p];
	    p++;
	}
    }

    if ( !xi ) {				// X image not created
	xi = XCreateImage( dpy, visual, dd, ZPixmap, 0, 0, w, h, 32, 0 );
	if ( xi->bits_per_pixel == 16 ) {	// convert 8 bpp ==> 16 bpp
	    ushort *p2;
	    int	    p2inc = xi->bytes_per_line/sizeof(ushort);
	    ushort *newerbits = (ushort *)malloc( xi->bytes_per_line * h );
	    CHECK_PTR( newerbits );
	    p = newbits;
	    for ( int y=0; y<h; y++ ) {		// OOPS: Do right byte order!!
		p2 = newerbits + p2inc*y;
		for ( int x=0; x<w; x++ )
		    *p2++ = *p++;
	    }
	    free( newbits );
	    newbits = (uchar *)newerbits;
	} else if ( xi->bits_per_pixel != 8 ) {
#if defined(CHECK_RANGE)
	    warning( "QPixmap::convertFromImage: Display not supported "
		     "(bpp=%d)", xi->bits_per_pixel );
#endif
	}
	xi->data = (char *)newbits;
    }

    if ( hd && (width() != w || height() != h || this->depth() != dd) ) {
	XFreePixmap( dpy, hd );			// don't reuse old pixmap
	hd = 0;
    }
    if ( !hd )					// create new pixmap
	hd = (HANDLE)XCreatePixmap( dpy, DefaultRootWindow(dpy), w, h, dd );

    XPutImage( dpy, hd, qt_xget_readonly_gc(), xi, 0, 0, 0, 0, w, h );
    if ( data->opt == NoOptim ) {		// throw away image
	qSafeXDestroyImage( xi );
    } else {					// keep ximage that we created
	data->ximage = xi;
    }
    data->w = w;  data->h = h;	data->d = dd;

    if ( img.hasAlphaBuffer() ) {
	QBitmap m;
	m = img.createAlphaMask( conversion_flags );
	setMask( m );
    }

    return TRUE;
}


/*!
  Grabs the contents of a window and makes a pixmap out of it.
  Returns the pixmap.

  The argments \e (x,y) specify the offset in the window, while
  \e (w,h) specify the width and height of the area to be copied.

  If \e w is negative, the function copies everything to the right
  border of the window.	 If \e h is negative, the function copies
  everything to the bottom of the window.

  Note that grabWindows() grabs pixels from the screen, not from the
  window.  This means that If there is another window partially or
  entirely over the one you grab, you get pixels from the overlying
  window too.

  The reason we use a window identifier and not a QWidget is to enable
  grabbing of windows that are not part of the application.

  \warning Grabbing an area outside the window, or screen, is not safe
  in general.  This depends on the underlying window system.
*/

QPixmap QPixmap::grabWindow( WId window, int x, int y, int w, int h )
{
    if ( !x11DefaultVisual() )			// incompatible depth
	return QPixmap(0, 0);
    if ( w <= 0 || h <= 0 ) {
	if ( w == 0 || h == 0 ) {
	    QPixmap nullPixmap;
	    return nullPixmap;
	}
	XWindowAttributes a;
	XGetWindowAttributes( dpy, window, &a );
	if ( w < 0 )
	    w = a.width - x;
	if ( h < 0 )
	    h = a.height - y;
    }
    QPixmap pm( w, h );				// create new pixmap
    pm.data->uninit = FALSE;
    GC gc = qt_xget_temp_gc( FALSE );
    XSetSubwindowMode( dpy, gc, IncludeInferiors );
    XCopyArea( dpy, window, pm.handle(), gc, x, y, w, h, 0, 0 );
    XSetSubwindowMode( dpy, gc, ClipByChildren );
    return pm;
}


/*!
  Returns a copy of the pixmap that is transformed using \e matrix.

  Qt uses this function to implement rotated text on window systems
  that do not support such complex features.

  Example of how to manually draw a rotated text at (100,200) in a widget:
  \code
    char    *str = "Trolls R Qt";	// text to be drawn
    QFont    f( "Charter", 24 );	// use Charter 24pt font
    QPixmap  pm( 8, 8 );
    QPainter p;
    QRect    r;				// text bounding rectangle
    QPoint   bl;			// text baselink position

    p.begin( &pm );			// first get the bounding
    p.setFont( f );			//   text rectangle
    r = p.fontMetrics().boundingRect(str);
    bl = -r.topLeft();			// get baseline position
    p.end();

    pm.resize( r.size() );		// resize to fit the text
    pm.fill( white );			// fills pm with white
    p.begin( &pm );			// begin painting pm
    p.setFont( f );			// set the font
    p.setPen( blue );			// set blue text color
    p.drawText( bl, str );		// draw the text
    p.end();				// painting done

    QWMatrix m;				// transformation matrix
    m.rotate( -33.4 );			// rotate coordinate system
    QPixmap rp = pm.xForm( m );		// rp is rotated pixmap

    QWMatrix t = QPixmap::trueMatrix( m, pm.width(), pm.height() );
    int x, y;
    t.map( bl.x(),bl.y(), &x,&y );	// get pm's baseline pos in rp

    bitBlt( myWidget, 100-x, 200-y,	// blt rp into a widget
	    &rp, 0, 0, -1, -1 );
  \endcode

  This example outlines how Qt implements rotated text under X11.
  The font calculation is the most tedious part. The rotation itself is
  only 3 lines of code.

  If you want to draw rotated text, you do not have to implement all the
  code above. The code below does exactly the same thing as the example
  above, except that it uses a QPainter.

  \code
    char    *str = "Trolls R Qt";	// text to be drawn
    QFont    f( "Charter", 24 );	// use Charter 24pt font
    QPainter p;

    p.begin( myWidget );
    p.translate( 100, 200 );		// translates coord system
    p.rotate( -33.4 );			// rotates it counterclockwise
    p.setFont( f );
    p.drawText( 0, 0, str );
    p.end();
  \endcode

  \bug 2 and 4 bits pixmaps are not supported.

  \sa trueMatrix(), QWMatrix, QPainter::setWorldMatrix()
*/

QPixmap QPixmap::xForm( const QWMatrix &matrix ) const
{
    int	   w, h;				// size of target pixmap
    int	   ws, hs;				// size of source pixmap
    uchar *dptr;				// data in target pixmap
    int	   dbpl, dbytes;			// bytes per line/bytes total
    uchar *sptr;				// data in original pixmap
    int	   sbpl;				// bytes per line in original
    int	   bpp;					// bits per pixel
    bool   depth1 = depth() == 1;
    int	   y;

    if ( isNull() )				// this is a null pixmap
	return copy();

    ws = width();
    hs = height();

    const float dt = 0.0001;
    float x1,y1, x2,y2, x3,y3, x4,y4;		// get corners
    float xx = (float)ws - 1;
    float yy = (float)hs - 1;

    matrix.map( dt, dt, &x1, &y1 );
    matrix.map( xx, dt, &x2, &y2 );
    matrix.map( xx, yy, &x3, &y3 );
    matrix.map( dt, yy, &x4, &y4 );

    float ymin = y1;				// lowest y value
    if ( y2 < ymin ) ymin = y2;
    if ( y3 < ymin ) ymin = y3;
    if ( y4 < ymin ) ymin = y4;
    float xmin = x1;				// lowest x value
    if ( x2 < xmin ) xmin = x2;
    if ( x3 < xmin ) xmin = x3;
    if ( x4 < xmin ) xmin = x4;

    QWMatrix mat( 1, 0, 0, 1, -xmin, -ymin );	// true matrix
    mat = matrix * mat;

    if ( mat.m12() == 0.0F && mat.m21() == 0.0F ) {
	if ( mat.m11() == 1.0F && mat.m22() == 1.0F )
	    return *this;			// identity matrix
	h = qRound( mat.m22()*hs );
	w = qRound( mat.m11()*ws );
	h = QABS( h );
	w = QABS( w );
    } else {					// rotation or shearing
	QPointArray a( QRect(0,0,ws,hs) );
	a = mat.map( a );
	QRect r = a.boundingRect().normalize();
	w = r.width();
	h = r.height();
    }
    bool invertible;
    mat = mat.invert( &invertible );		// invert matrix

    if ( h == 0 || w == 0 || !invertible ) {	// error, return null pixmap
	QPixmap pm;
	pm.data->bitmap = data->bitmap;
	return pm;
    }

#if defined(MITSHM)
    static bool try_once = TRUE;
    if (try_once) {
	try_once = FALSE;
	if ( !xshminit )
	    qt_create_mitshm_buffer( this, 800, 600 );
    }

    bool use_mitshm = xshmimg && !depth1 &&
		      xshmimg->width >= w && xshmimg->height >= h;
#endif
    XImage *xi = (XImage*)data->ximage;		// any cached ximage?
    if ( !xi )
	xi = XGetImage( dpy, handle(), 0, 0, ws, hs, AllPlanes,
			depth1 ? XYPixmap : ZPixmap );

    if ( !xi ) {				// error, return null pixmap
	QPixmap pm;
	pm.data->bitmap = data->bitmap;
	return pm;
    }

    sbpl = xi->bytes_per_line;
    sptr = (uchar *)xi->data;
    bpp	 = xi->bits_per_pixel;

    if ( depth1 )
	dbpl = (w+7)/8;
    else
	dbpl = ((w*bpp+31)/32)*4;
    dbytes = dbpl*h;

#if defined(MITSHM)
    if ( use_mitshm ) {
	dptr = (uchar *)xshmimg->data;
	uchar fillbyte = bpp == 8 ? white.pixel() : 0xff;
	for ( y=0; y<h; y++ )
	    memset( dptr + y*xshmimg->bytes_per_line, fillbyte, dbpl );
    } else {
#endif
	dptr = (uchar *)malloc( dbytes );	// create buffer for bits
	CHECK_PTR( dptr );
	if ( depth1 )				// fill with zeros
	    memset( dptr, 0, dbytes );
	else if ( bpp == 8 )			// fill with background color
	    memset( dptr, white.pixel(), dbytes );
	else
	    memset( dptr, 0xff, dbytes );
#if defined(MITSHM)
    }
#endif

// #define DEBUG_XIMAGE
#if defined(DEBUG_XIMAGE)
    debug( "----IMAGE--INFO--------------" );
    debug( "width............. %d", xi->width );
    debug( "height............ %d", xi->height );
    debug( "xoffset........... %d", xi->xoffset );
    debug( "format............ %d", xi->format );
    debug( "byte order........ %d", xi->byte_order );
    debug( "bitmap unit....... %d", xi->bitmap_unit );
    debug( "bitmap bit order.. %d", xi->bitmap_bit_order );
    debug( "depth............. %d", xi->depth );
    debug( "bytes per line.... %d", xi->bytes_per_line );
    debug( "bits per pixel.... %d", xi->bits_per_pixel );
#endif

    int m11 = qRound((double)mat.m11()*65536.0);
    int m12 = qRound((double)mat.m12()*65536.0);
    int m21 = qRound((double)mat.m21()*65536.0);
    int m22 = qRound((double)mat.m22()*65536.0);
    int dx  = qRound((double)mat.dx() *65536.0);
    int dy  = qRound((double)mat.dy() *65536.0);

    int	  m21ydx = dx + (xi->xoffset<<16), m22ydy = dy;
    uint  trigx, trigy;
    uint  maxws = ws<<16, maxhs=hs<<16;
    uchar *p	= dptr;
    int	  xbpl, p_inc;
    bool  msbfirst = xi->bitmap_bit_order == MSBFirst;

    if ( depth1 ) {
	xbpl  = (w+7)/8;
	p_inc = dbpl - xbpl;
    } else {
	xbpl  = (w*bpp)/8;
	p_inc = dbpl - xbpl;
#if defined(MITSHM)
	if ( use_mitshm )
	    p_inc = xshmimg->bytes_per_line - xbpl;
#endif
    }

    for ( y=0; y<h; y++ ) {			// for each target scanline
	trigx = m21ydx;
	trigy = m22ydy;
	uchar *maxp = p + xbpl;
	if ( !depth1 ) {
	    switch ( bpp ) {
		case 8:				// 8 bpp transform
		while ( p < maxp ) {
		    if ( trigx < maxws && trigy < maxhs )
			*p = *(sptr+sbpl*(trigy>>16)+(trigx>>16));
		    trigx += m11;
		    trigy += m12;
		    p++;
		}
		break;

		case 16:			// 16 bpp transform
		while ( p < maxp ) {
		    if ( trigx < maxws && trigy < maxhs )
			*((ushort*)p) = *((ushort *)(sptr+sbpl*(trigy>>16) +
						     ((trigx>>16)<<1)));
		    trigx += m11;
		    trigy += m12;
		    p++;
		    p++;
		}
		break;

		case 24: {			// 24 bpp transform
		uchar *p2;
		while ( p < maxp ) {
		    if ( trigx < maxws && trigy < maxhs ) {
			p2 = sptr+sbpl*(trigy>>16) + ((trigx>>16)*3);
			p[0] = p2[0];
			p[1] = p2[1];
			p[2] = p2[2];
		    }
		    trigx += m11;
		    trigy += m12;
		    p += 3;
		}
		}
		break;

		case 32:			// 32 bpp transform
		while ( p < maxp ) {
		    if ( trigx < maxws && trigy < maxhs )
			*((uint*)p) = *((uint *)(sptr+sbpl*(trigy>>16) +
						   ((trigx>>16)<<2)));
		    trigx += m11;
		    trigy += m12;
		    p += 4;
		}
		break;

		default: {
#if defined(CHECK_RANGE)
		warning( "QPixmap::xForm: DISPLAY NOT SUPPORTED (BPP=%d)",bpp);
#endif
		QPixmap pm;
		return pm;
		}
	    }
	} else if ( msbfirst ) {		// mono bitmap MSB first
	    while ( p < maxp ) {
#undef IWX
#define IWX(b)	if ( trigx < maxws && trigy < maxhs ) {			      \
		    if ( *(sptr+sbpl*(trigy>>16)+(trigx>>19)) &		      \
			 (1 << (7-((trigx>>16)&7))) )			      \
			*p |= b;					      \
		}							      \
		trigx += m11;						      \
		trigy += m12;
	// END OF MACRO
		IWX(1);
		IWX(2);
		IWX(4);
		IWX(8);
		IWX(16);
		IWX(32);
		IWX(64);
		IWX(128);
		p++;
	    }
	} else {				// mono bitmap LSB first
	    while ( p < maxp ) {
#undef IWX
#define IWX(b)	if ( trigx < maxws && trigy < maxhs ) {			      \
		    if ( *(sptr+sbpl*(trigy>>16)+(trigx>>19)) &		      \
			 (1 << ((trigx>>16)&7)) )			      \
			*p |= b;					      \
		}							      \
		trigx += m11;						      \
		trigy += m12;
	// END OF MACRO
		IWX(1);
		IWX(2);
		IWX(4);
		IWX(8);
		IWX(16);
		IWX(32);
		IWX(64);
		IWX(128);
		p++;
	    }
	}
	m21ydx += m21;
	m22ydy += m22;
	p += p_inc;
    }
    if ( data->opt == NoOptim ) {		// throw away ximage
	qSafeXDestroyImage( xi );
    } else {					// keep ximage that we fetched
	data->ximage = xi;
    }

    if ( depth1 ) {				// mono bitmap
	QPixmap pm( w, h, dptr, TRUE );
	pm.data->bitmap = data->bitmap;
	free( dptr );
	if ( data->mask ) {
	    if ( data->selfmask )		// pixmap == mask
		pm.setMask( *((QBitmap*)(&pm)) );
	    else
		pm.setMask( data->mask->xForm(matrix) );
	}
	return pm;
    } else {					// color pixmap
	GC gc = qt_xget_readonly_gc();
	QPixmap pm( w, h );
	pm.data->uninit = FALSE;
#if defined(MITSHM)
	if ( use_mitshm ) {
	    XCopyArea( dpy, xshmpm, pm.handle(), gc, 0, 0, w, h, 0, 0 );
	} else {
#endif
	    xi = XCreateImage( dpy, (Visual *)x11Visual(), x11Depth(),
			       ZPixmap, 0, (char *)dptr, w, h, 32, 0 );
	    XPutImage( dpy, pm.handle(), gc, xi, 0, 0, 0, 0, w, h);
	    qSafeXDestroyImage( xi );
#if defined(MITSHM)
	}
#endif
	if ( data->mask )			// xform mask, too
	    pm.setMask( data->mask->xForm(matrix) );
	return pm;
    }
}


/*!
  Returns the actual matrix used for transforming a pixmap with \e w
  width and \e h height.

  When transforming a pixmap with xForm(), the transformation matrix
  is internally adjusted to compensate for unwanted translation,
  i.e. xForm() returns the smallest pixmap containing all transformed
  points of the original pixmap.

  This function returns the modified matrix, which maps points
  correctly from the original pixmap into the new pixmap.

  \sa xForm(), QWMatrix
*/

QWMatrix QPixmap::trueMatrix( const QWMatrix &matrix, int w, int h )
{
    const float dt = 0.0001;
    float x1,y1, x2,y2, x3,y3, x4,y4;		// get corners
    float xx = (float)w - 1;
    float yy = (float)h - 1;

    matrix.map( dt, dt, &x1, &y1 );
    matrix.map( xx, dt, &x2, &y2 );
    matrix.map( xx, yy, &x3, &y3 );
    matrix.map( dt, yy, &x4, &y4 );

    float ymin = y1;				// lowest y value
    if ( y2 < ymin ) ymin = y2;
    if ( y3 < ymin ) ymin = y3;
    if ( y4 < ymin ) ymin = y4;
    float xmin = x1;				// lowest x value
    if ( x2 < xmin ) xmin = x2;
    if ( x3 < xmin ) xmin = x3;
    if ( x4 < xmin ) xmin = x4;

    QWMatrix mat( 1, 0, 0, 1, -xmin, -ymin );	// true matrix
    mat = matrix * mat;
    return mat;
}
