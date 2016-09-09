/****************************************************************************
** $Id: qimage.cpp,v 2.103.2.6 1998/11/02 15:49:13 hanord Exp $
**
** Implementation of QImage and QImageIO classes
**
** Created : 950207
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

#define	 QIMAGE_C
#include "qimage.h"
#include "qregexp.h"
#include "qfile.h"
#include "qdatastream.h"
#include "qtextstream.h"
#include "qbuffer.h"
#include "qlist.h"
#include "qdict.h"
#include "qintdict.h"
#include "qasyncimageio.h"
#include <stdlib.h>
#include <ctype.h>

/*!
  \class QImage qimage.h
  \brief The QImage class provides a hardware-independent pixmap
  representation with direct access to the pixel data.

  \ingroup images

  It is one of the two classes Qt provides for dealing with images,
  the other being QPixmap.  QImage is designed and optimized for I/O
  and for direct pixel access/manipulation, QPixmap is designed and
  optimized for drawing.  There are (slow) functions to convert
  between QImage and QPixmp; QPixmap::convertToImage() and
  QPixmap::convertFromImage().

  An image has the parameters \link width() width\endlink, \link height()
  height\endlink and \link depth() depth\endlink (bits per pixel, bpp), a
  color table and the actual \link bits() pixels\endlink.  QImage supports
  1-bpp, 8-bpp and 32-bpp image data.  1-bpp and 8-bpp images use a color
  lookup table; the pixel value is a color table index.

  32-bpp images encode an RGB value in 24 bits and ignore the color table.
  The most significant byte is used for the \link setAlphaBuffer alpha
  buffer\endlink.

  An entry in the color table is an RGB triplet encoded as \c uint.  Use
  the qRed, qGreen and qBlue functions (qcolor.h) to access the
  components, and qRgb to make an RGB triplet (see the QColor class
  documentation).

  1-bpp (monochrome) images have a color table with maximum 2 colors.
  There are two different formats; big endian (MSB first) or little endian
  (LSB first) bit order. To access a single bit, you will have to do some
  bitshifts:

  \code
    QImage image;
      // sets bit at (x,y) to 1
    if ( image.bitOrder() == QImage::LittleEndian )
	*(image.scanLine(y) + (x >> 3)) |= 1 << (x & 7);
    else
	*(image.scanLine(y) + (x >> 3)) |= 1 << (7 -(x & 7));
  \endcode

  If this looks complicated, it might be a good idea to convert the 1-bpp
  image to an 8-bpp image using convertDepth().

  8-bpp images are much easier to work with than 1-bpp images because they
  have a single byte per pixel:

  \code
    QImage image;
      // set entry 19 in the color table to yellow
    image.setColor( 19, qRgb(255,255,0) );
      // set 8 bit pixel at (x,y) to value yellow (in color table)
    *(image.scanLine(y) + x) = 19;
  \endcode

  32-bpp images ignore the color table, instead each pixel contains the
  RGB triplet. 24 bits contain the RGB value and the most significant
  byte is reserved for the alpha buffer.

  \code
    QImage image;
      // sets 32 bit pixel at (x,y) to yellow.
    uint *p = (uint *)image.scanLine(y) + x;
    *p = qRgb(255,255,0);
  \endcode

  The scanlines are 32-bit aligned for all depths.

  The QImage class uses explicit \link shclass.html sharing\endlink,
  similar to that of QArray and QString.

  \sa QImageIO, QPixmap, \link shclass.html Shared Classes\endlink
*/


extern bool qt_image_native_bmp();

#if defined(_CC_DEC_) && defined(__alpha) && (__DECCXX_VER >= 50190001)
#pragma message disable narrowptr
#endif


/*****************************************************************************
  QImage member functions
 *****************************************************************************/

static bool  bitflip_init = FALSE;
static uchar bitflip[256];			// table to flip bits

static void setup_bitflip()			// create bitflip table
{
    if ( !bitflip_init ) {
	for ( int i=0; i<256; i++ )
	    bitflip[i] = ((i >> 7) & 0x01) | ((i >> 5) & 0x02) |
			 ((i >> 3) & 0x04) | ((i >> 1) & 0x08) |
			 ((i << 7) & 0x80) | ((i << 5) & 0x40) |
			 ((i << 3) & 0x20) | ((i << 1) & 0x10);
	bitflip_init = TRUE;
    }
}

uchar *qt_get_bitflip_array()			// called from QPixmap code
{
    setup_bitflip();
    return bitflip;
}


/*!
  Constructs a null image.
  \sa isNull()
*/

QImage::QImage()
{
    data = new QImageData;
    CHECK_PTR( data );
    init();
}

/*!
  Constructs an image with \a w width, \a h height, \a depth bits per
  pixel, \a numColors colors and bit order \a bitOrder.

  Using this constructor is the same as first constructing a null image and
  then calling the create() function.

  \sa create()
*/

QImage::QImage( int w, int h, int depth, int numColors, Endian bitOrder )
{
    data = new QImageData;
    CHECK_PTR( data );
    init();
    create( w, h, depth, numColors, bitOrder );
}

/*!
 \overload QImage::QImage( const QSize&, int depth, int numColors, Endian bitOrder )
*/
QImage::QImage( const QSize& size, int depth, int numColors, Endian bitOrder )
{
    data = new QImageData;
    CHECK_PTR( data );
    init();
    create( size, depth, numColors, bitOrder );
}


/*!
  Constructs an image from loading \a fileName and an optional
  \a format.
  \sa load()
*/

QImage::QImage( const char *fileName, const char *format )
{
    data = new QImageData;
    CHECK_PTR( data );
    init();
    load( fileName, format );
}


// helper
static void read_xpm_image_or_array( QImageIO *, const char **, QImage & );

/*!
  Constructs an image from \a xpm, which must be a valid XPM image.

  Errors are silently ignored.
*/

QImage::QImage( const char *xpm[] )
{
    data = new QImageData;
    CHECK_PTR( data );
    init();
    read_xpm_image_or_array( 0, xpm, *this );
}

/*
  Constructs an image from \a data, which must be in a supported
  image format image.

  \sa loadFromData()
*/
/* Not until Qt 2.0 - adding conversions break source code
QImage::QImage( const QByteArray & data )
{
    loadFromData(data);
}
*/

/*!
  Constructs a
  \link shclass.html shallow copy\endlink of \e image.
*/

QImage::QImage( const QImage &image )
{
    data = image.data;
    data->ref();
}


/*!
  Destroys the image and cleans up.
*/

QImage::~QImage()
{
    if ( data && data->deref() ) {
	reset();
	delete data;
    }
}


/*!
  Assigns a
  \link shclass.html shallow copy\endlink
  of \e image to this image and returns a reference to this image.

  \sa copy()
*/

QImage &QImage::operator=( const QImage &image )
{
    image.data->ref();				// avoid 'x = x'
    if ( data->deref() ) {
	reset();
	delete data;
    }
    data = image.data;
    return *this;
}

/*!
  Sets the image bits to the \e pixmap contents and returns a reference to
  the image.

  If the image shares data with other images, it will first dereference
  the shared data.

  Makes a call to QPixmap::convertToImage().
*/

QImage &QImage::operator=( const QPixmap &pixmap )
{
    *this = pixmap.convertToImage();
    return *this;
}

/*!
  Detaches from shared image data and makes sure that this image is the
  only one referring the data.

  If multiple images share common data, this image makes a copy of the
  data and detaches itself from the sharing mechanism.	Nothing is
  done if there is just a single reference.
*/

void QImage::detach()
{
    if ( data->count != 1 )
	*this = copy();
}

/*!
  Returns a
  \link shclass.html deep copy\endlink of the image.
*/
QImage QImage::copy() const
{
    QImage image;
    if ( !isNull() ) {
	image.create( width(), height(), depth(), numColors(), bitOrder() );
	memcpy( image.bits(), bits(), numBytes() );
	memcpy( image.colorTable(), colorTable(), numColors()*sizeof(QRgb) );
	image.setAlphaBuffer(hasAlphaBuffer());
    }
    return image;
}

/*!
  Returns a
  \link shclass.html deep copy\endlink of a sub-area of the image.

  \sa bitBlt()
*/
QImage QImage::copy(int x, int y, int w, int h, int conversion_flags) const
{
    // Parameter correction
    if ( w < 0 ) w = width();
    if ( h < 0 ) h = height();
    if ( x < 0 ) { w += x; x = 0; }
    if ( y < 0 ) { h += y; y = 0; }
    if ( x + w > width() ) w = width() - x;
    if ( y + h > height() ) h = height() - y;
    if ( w <= 0 || h <= 0 ) return QImage(); // Nothing left to copy

    QImage image( w, h, depth(), numColors(), bitOrder() );
    memcpy( image.colorTable(), colorTable(), numColors()*sizeof(QRgb) );
    image.setAlphaBuffer(hasAlphaBuffer());
    bitBlt( &image, 0, 0, this, x, y, -1, -1, conversion_flags );
    return image;
}

/*!
  \overload QImage QImage::copy(QRect& r) const
*/

/*!
  \fn bool QImage::isNull() const
  Returns TRUE if it is a null image.

  A null image has all parameters set to zero and no allocated data.
*/


/*!
  \fn int QImage::width() const
  Returns the width of the image.
  \sa heigth(), size(), rect()
*/

/*!
  \fn int QImage::height() const
  Returns the height of the image.
  \sa width(), size(), rect()
*/

/*!
  \fn QSize QImage::size() const
  Returns the size of the image.
  \sa width(), height(), rect()
*/

/*!
  \fn QRect QImage::rect() const
  Returns the enclosing rectangle (0,0,width(),height()) of the image.
  \sa width(), height(), size()
*/

/*!
  \fn int QImage::depth() const
  Returns the depth of the image.

  The image depth is the number of bits used to encode a single pixel, also
  called bits per pixel (bpp) or bit planes of an image.

  The supported depths are 1, 8 and 32.
*/

/*!
  \fn int QImage::numColors() const
  Returns the size of the color table for the image.

  Notice that numColors() returns 0 for 32-bpp images, since these images
  do not use color tables, but instead encode pixel values as RGB triplets.
*/

/*!
  \fn QImage::Endian QImage::bitOrder() const
  Returns the bit order for the image.

  If it is a 1-bpp image, this function returns either QImage::BigEndian or
  QImage::LittleEndian.

  If it is not a 1-bpp image, this function returns QImage::IgnoreEndian.

  \sa depth()
*/

/*!
  \fn uchar **QImage::jumpTable() const
  Returns a pointer to the scanline pointer table.

  This is the beginning of the data block for the image.
*/

/*!
  \fn QRgb *QImage::colorTable() const
  Returns a pointer to the color table.
*/

/*!
  \fn int QImage::numBytes() const
  Returns the number of bytes occupied by the image data.
  \sa bytesPerLine()
*/

/*!
  \fn int QImage::bytesPerLine() const
  Returns the number of bytes per image scanline.
  This is equivalent to numBytes()/height().
*/

/*!
  \fn QRgb QImage::color( int i ) const

  Returns the color in the color table at index \e i.

  A color value is an RGB triplet. Use the QRED, QGREEN and QBLUE functions
  (defined in qcolor.h) to get the color value components.

  \sa setColor(), QColor
*/

/*!
  \fn void QImage::setColor( int i, QRgb c )

  Sets a color in the color table at index \e i to \e c.

  A color value is an RGB triplet.  Use the qRgb function (defined in qcolor.h)
  to make RGB triplets.

  \sa color()
*/

/*!
  \fn uchar *QImage::scanLine( int i ) const

  Returns a pointer to the pixel data at the \e i'th scanline.

  The scanline data is aligned on a 32-bit boundary.

  \warning If you are accessing 32-bpp image data, cast the returned
  pointer to \c QRgb* (QRgb has a 32 bit size) and use it to read/write
  the pixel value. You cannot use the \c uchar* pointer directly, because
  the pixel format depends on the byte order on the underlying
  platform. Hint: use \link ::qRed() qRed()\endlink and friends (qcolor.h)
  to access the pixels.

  \sa bits()
*/

void QImage::warningIndexRange( const char *func, int i )
{
#if defined(DEBUG)
    warning( "QImage::%s: Index %d out of range", func, i );
#endif
}


#if !defined(_OS_WIN32_)

QRgb QImage::color( int i ) const
{
#if defined(CHECK_RANGE)
    if ( i >= data->ncols )
	warningIndexRange( "color", i );
#endif
    return data->ctbl ? data->ctbl[i] : (QRgb)-1;
}

void QImage::setColor( int i, QRgb c )
{
#if defined(CHECK_RANGE)
    if ( i >= data->ncols )
	warningIndexRange( "setColor", i );
#endif
    if ( data->ctbl )
	data->ctbl[i] = c;
}

uchar *QImage::scanLine( int i ) const
{
#if defined(CHECK_RANGE)
    if ( i >= data->h )
	warningIndexRange( "scanLine", i );
#endif
    return data->bits ? data->bits[i] : 0;
}

#endif // !_OS_WIN32_


/*!
  \fn uchar *QImage::bits() const
  Returns a pointer to the first pixel data. Equivalent to scanLine(0).
  \sa scanLine()
*/


/*!
  Resets all image parameters and deallocates the image data.
*/

void QImage::reset()
{
    freeBits();
    setNumColors( 0 );
    data->w = data->h = data->d = 0;
    data->nbytes = 0;
    data->bitordr = IgnoreEndian;
}


/*!
  Fills the entire image with the pixel value \a pixel.

  If the \link depth() depth\endlink of this image is 1, only
  the lowest bit is used. If you say fill(0), fill(2) etc., the image
  is filled with 0s. If you say fill(1), fill(3) etc., the image
  is filled with 1s. If the depth is 8, the lowest 8 bits are used.

  If the depth is 32 and the image has no alpha buffer, the \a pixel
  value is written to each pixel in the image. If the image has an
  alpha buffer, only the 24 RGB bits are set and the upper 8 bits (alpha
  value) are left unchanged.
*/

void QImage::fill( uint pixel )
{
    if ( depth() == 1 || depth() == 8 ) {
	if ( depth() == 1 ) {
	    if ( pixel & 1 )
		pixel = 0xffffffff;
	    else
		pixel = 0;
	} else {
	    uint c = pixel & 0xff;
	    pixel = c | ((c << 8) & 0xff00) | ((c << 16) & 0xff0000) |
		    ((c << 24) & 0xff000000);
	}
	int bpl = bytesPerLine();
	for ( int i=0; i<height(); i++ )
	    memset( scanLine(i), pixel, bpl );
    } else if ( depth() == 32 ) {
	if ( hasAlphaBuffer() ) {
	    pixel &= 0x00ffffff;
	    for ( int i=0; i<height(); i++ ) {
		uint *p = (uint *)scanLine(i);
		uint *end = p + width();
		while ( p < end ) {
		    *p = (*p & 0xff000000) | pixel;
		    p++;
		}
	    }
	} else {	
	    for ( int i=0; i<height(); i++ ) {
		uint *p = (uint *)scanLine(i);
		uint *end = p + width();
		while ( p < end )
		    *p++ = pixel;
	    }
	}
    }
}


/*!
  Determines the host computer byte order.
  Returns QImage::LittleEndian (LSB first) or QImage::BigEndian (MSB first).
*/

QImage::Endian QImage::systemByteOrder()
{
    static Endian sbo = IgnoreEndian;
    if ( sbo == IgnoreEndian ) {		// initialize
	int  ws;
	bool be;
	qSysInfo( &ws, &be );
	sbo = be ? BigEndian : LittleEndian;
    }
    return sbo;
}


#if defined(_WS_X11_)
#define	 GC GC_QQQ
#include <X11/Xlib.h>				// needed for systemBitOrder
#include <X11/Xutil.h>
#include <X11/Xos.h>
#if defined(_OS_WIN32_)
#undef open					// kill utterly stupid #defines
#undef close
#undef read
#undef write
#endif
#endif

/*!
  Determines the bit order of the display hardware.
  Returns QImage::LittleEndian (LSB first) or QImage::BigEndian (MSB first).
*/

QImage::Endian QImage::systemBitOrder()
{
#if defined(_WS_X11_)
    return BitmapBitOrder(qt_xdisplay()) == MSBFirst ? BigEndian :LittleEndian;
#else
    return BigEndian;
#endif
}


/*!
  Resizes the color table to \e numColors colors.

  If the color table is expanded, then all new colors will be set to black
  (RGB 0,0,0).

  \sa color(), setColor()
*/

void QImage::setNumColors( int numColors )
{
    if ( numColors == data->ncols )
	return;
    if ( numColors == 0 ) {			// use no color table
	if ( data->ctbl ) {
	    free( data->ctbl );
	    data->ctbl = 0;
	}
	data->ncols = 0;
	return;
    }
    if ( data->ctbl ) {				// already has color table
	data->ctbl = (QRgb*)realloc( data->ctbl, numColors*sizeof(QRgb) );
	if ( data->ctbl && numColors > data->ncols )
	    memset( (char *)&data->ctbl[data->ncols], 0,
		    (numColors-data->ncols)*sizeof(QRgb) );
    } else {					// create new color table
	data->ctbl = (QRgb*)calloc( numColors*sizeof(QRgb), 1 );
    }
    data->ncols = data->ctbl == 0 ? 0 : numColors;
}


/*!
  \fn bool QImage::hasAlphaBuffer() const

  Returns TRUE if alpha buffer mode is enabled, otherwise FALSE.

  \sa setAlphaBuffer()
*/

/*!
  Enables alpha buffer mode if \a enable is TRUE, otherwise disables it.
  The default setting is disabled.

  An 8-bpp image has 8 bit pixels. A pixel is an index into the \link
  color() color table\endlink, which contains 32-bit color values.
  In a 32-bpp image, the 32 bit pixels are the color values.

  This 32 bit value is encoded as follows: The lower 24 bits are used for
  the red, green and blue components. The upper 8 bits contain the alpha
  component.

  The alpha component specifies the transparency of a pixel. 0 means
  completely transparent and 255 means opaque. The alpha component is
  ignored if you do not enable alpha buffer mode.

  The alpha buffer is used to set a mask when a QImage is translated to a
  QPixmap.

  \sa hasAlphaBuffer(), createAlphaMask()
*/

void QImage::setAlphaBuffer( bool enable )
{
    data->alpha = enable;
}


/*!
  Sets the image width, height, depth, number of colors and bit order.
  Returns TRUE if successful, or FALSE if the parameters are incorrect or
  if memory cannot be allocated.

  The \e width and \e height is limited to 32767. \e depth must be 1, 8 or
  32. If \e depth is 1, then \e bitOrder must be set to either
  QImage::LittleEndian or QImage::BigEndian.  For other depths, \e
  bitOrder must be QImage::IgnoreEndian.

  This function allocates a color table and a buffer for the image data.
  The image data is not initialized.

  The image buffer is allocated as a single block that consists of a table
  of \link scanLine() scanline\endlink pointers (jumpTable()) and the
  image data (bits()).

  \sa width(), height(), depth(), numColors(), bitOrder(), jumpTable(),
  scanLine(), bits(), bytesPerLine(), numBytes()
*/

bool QImage::create( int width, int height, int depth, int numColors,
		    Endian bitOrder )
{
    reset();					// reset old data
    if ( width <= 0 || height <= 0 || depth <= 0 || numColors < 0 )
	return FALSE;				// invalid parameter(s)
    if ( depth == 1 && bitOrder == IgnoreEndian ) {
#if defined(CHECK_RANGE)
	warning( "QImage::create: Bit order is required for 1 bpp images" );
#endif
	return FALSE;
    }
    if ( depth != 1 )
	bitOrder = IgnoreEndian;

#if defined(DEBUG)
    if ( depth == 24 )
	warning( "QImage::create: 24-bpp images no longer supported, "
		 "use 32-bpp instead" );
#endif
    switch ( depth ) {
	case 1:
	case 8:
	case 32:
	    break;
	default:				// invalid depth
	    return FALSE;
    }

    setNumColors( numColors );
    if ( data->ncols != numColors )		// could not alloc color table
	return FALSE;

    int bpl    = ((width*depth+31)/32)*4;	// bytes per scanline
    int pad    = bpl - (width*depth)/8;		// pad with zeros
    int nbytes = bpl*height;			// image size
    int ptbl   = height*sizeof(uchar*);		// pointer table size
    int size   = nbytes + ptbl;			// total size of data block
    uchar **p  = (uchar **)malloc( size );	// alloc image bits
    if ( !p ) {					// no memory
	setNumColors( 0 );
	return FALSE;
    }
    data->w = width;
    data->h = height;
    data->d = depth;
    data->nbytes  = nbytes;
    data->bitordr = bitOrder;
    data->bits = p;				// set image pointer
    uchar *d = (uchar*)p + ptbl;		// setup scanline pointers
    while ( height-- ) {
	*p++ = d;
	if ( pad )
	    memset( d+bpl-pad, 0, pad );
	d += bpl;
    }
    return TRUE;
}

/*!
 \overload bool QImage::create( const QSize&, int depth, int numColors, Endian bitOrder )
*/
bool QImage::create( const QSize& size, int depth, int numColors,
		     QImage::Endian bitOrder )
{
    return create(size.width(), size.height(), depth, numColors, bitOrder);
}

/*!
  \internal
  Initializes the image data structure.
*/

void QImage::init()
{
    data->w = data->h = data->d = data->ncols = 0;
    data->nbytes = 0;
    data->ctbl = 0;
    data->bits = 0;
    data->bitordr = QImage::IgnoreEndian;
    data->alpha = FALSE;
}

/*!
  \internal
  Deallocates the image data and sets the bits pointer to 0.
*/

void QImage::freeBits()
{
    if ( data->bits ) {				// dealloc image bits
	free( data->bits );
	data->bits = 0;
    }
}


/*****************************************************************************
  Internal routines for converting image depth.
 *****************************************************************************/

//
// convert_32_to_8:  Converts a 32 bits depth (true color) to an 8 bit
// image with a colormap.  If the 32 bit image has more than 256 colors,
// we convert the red,green and blue bytes into a single byte encoded
// as 6 shades of each of red, green and blue.
//

Q_DECLARE(QIntDictM,char);
Q_DECLARE(QIntDictIteratorM,char);

struct QRgbMap {
    QRgbMap() : rgb(0xffffffff) { }
    bool used() const { return rgb!=0xffffffff; }
    uchar  pix;
    QRgb  rgb;
};

static bool convert_32_to_8( const QImage *src, QImage *dst, int conversion_flags, QRgb* palette=0, int palette_count=0 )
{
    register QRgb *p;
    uchar  *b;
    bool    do_quant = FALSE;
    int	    y, x;

    if ( !dst->create(src->width(), src->height(), 8, 256) )
	return FALSE;

    const int tablesize = 997; // prime
    QRgbMap table[tablesize];
    int   pix=0;

    if ( palette ) {
	// Preload palette into table.

	p = palette;
	// Almost same code as pixel insertion below
	while ( palette_count-- > 0 ) {
	    // Find in table...
	    int hash = *p % tablesize;
	    for (;;) {
		if ( table[hash].used() ) {
		    if ( table[hash].rgb == (*p & 0x00ffffff) ) {
			// Found previous insertion - use it
			break;
		    } else {
			// Keep searching...
			if (++hash == tablesize) hash = 0;
		    }
		} else {
		    // Cannot be in table
		    ASSERT ( pix != 256 );		// too many colors
		    // Insert into table at this unused position
		    dst->setColor( pix, *p );
		    table[hash].pix = pix++;
		    table[hash].rgb = *p & 0x00ffffff;
		    break;
		}
	    }
	    p++;
	}
    }

    if ( (conversion_flags & DitherMode_Mask) == PreferDither ) {
	do_quant = TRUE;
    } else {
	for ( y=0; y<src->height(); y++ ) {		// check if <= 256 colors
	    p = (QRgb *)src->scanLine(y);
	    b = dst->scanLine(y);
	    x = src->width();
	    while ( x-- ) {
		// Find in table...
		int hash = *p % tablesize;
		for (;;) {
		    if ( table[hash].used() ) {
			if ( table[hash].rgb == (*p & 0x00ffffff) ) {
			    // Found previous insertion - use it
			    break;
			} else {
			    // Keep searching...
			    if (++hash == tablesize) hash = 0;
			}
		    } else {
			// Cannot be in table
			if ( pix == 256 ) {		// too many colors
			    do_quant = TRUE;
			    // Break right out
			    x = 0;
			    y = src->height();
			} else {
			    // Insert into table at this unused position
			    dst->setColor( pix, *p );
			    table[hash].pix = pix++;
			    table[hash].rgb = *p & 0x00ffffff;
			}
			break;
		    }
		}
		*b++ = table[hash].pix; // May occur once incorrectly
		p++;
	    }
	}
    }
    int ncols = do_quant ? 256 : pix;

    static uint bm[16][16];
    static int init=0;
    if (!init) {
	// Build a Bayer Matrix for dithering

	init = 1;
	int n, i, j;

	bm[0][0]=0;

	for (n=1; n<16; n*=2) {
	    for (i=0; i<n; i++) {
		for (j=0; j<n; j++) {
		    bm[i][j]*=4;
		    bm[i+n][j]=bm[i][j]+2;
		    bm[i][j+n]=bm[i][j]+3;
		    bm[i+n][j+n]=bm[i][j]+1;
		}
	    }
	}

	for (i=0; i<16; i++)
	    for (j=0; j<16; j++)
		bm[i][j]<<=8;
    }

    dst->setNumColors( ncols );

    if ( do_quant ) {				// quantization needed

#define MAX_R 5
#define MAX_G 5
#define MAX_B 5
#define INDEXOF(r,g,b) (((r)*(MAX_G+1)+(g))*(MAX_B+1)+(b))

	int rc, gc, bc;

	for ( rc=0; rc<=MAX_R; rc++ )		// build 6x6x6 color cube
	    for ( gc=0; gc<=MAX_G; gc++ )
		for ( bc=0; bc<=MAX_B; bc++ ) {
		    dst->setColor( INDEXOF(rc,gc,bc),
			qRgb( rc*255/MAX_R, gc*255/MAX_G, bc*255/MAX_B ) );
		}

	int sw = src->width();

	int* line1[3];
	int* line2[3];
	int* pv[3];
	if ( ( conversion_flags & Dither_Mask ) == DiffuseDither ) {
	    line1[0] = new int[src->width()];
	    line2[0] = new int[src->width()];
	    line1[1] = new int[src->width()];
	    line2[1] = new int[src->width()];
	    line1[2] = new int[src->width()];
	    line2[2] = new int[src->width()];
	    pv[0] = new int[sw];
	    pv[1] = new int[sw];
	    pv[2] = new int[sw];
	}

	for ( y=0; y < src->height(); y++ ) {
	    p = (QRgb *)src->scanLine(y);
	    b = dst->scanLine(y);
	    QRgb *end = p + sw;

	    // perform quantization
	    if ( ( conversion_flags & Dither_Mask ) == ThresholdDither ) {
#define DITHER(p,m) ((uchar) ((p * (m) + 127) / 255))
		while ( p < end ) {
		    rc = qRed( *p );
		    gc = qGreen( *p );
		    bc = qBlue( *p );

		    *b++ =
			INDEXOF(
			    DITHER(rc, MAX_R),
			    DITHER(gc, MAX_G),
			    DITHER(bc, MAX_B)
			);

		    p++;
		}
#undef DITHER
	    } else if ( ( conversion_flags & Dither_Mask ) == OrderedDither ) {
#define DITHER(p,d,m) ((uchar) ((((256 * (m) + (m) + 1)) * (p) + (d)) / 65536 ))

		int x = 0;
		while ( p < end ) {
		    uint d = bm[y&15][x&15];

		    rc = qRed( *p );
		    gc = qGreen( *p );
		    bc = qBlue( *p );

		    *b++ =
			INDEXOF(
			    DITHER(rc, d, MAX_R),
			    DITHER(gc, d, MAX_G),
			    DITHER(bc, d, MAX_B)
			);

		    p++;
		    x++;
		}
#undef DITHER
	    } else { // Diffuse
		int endian = (QImage::systemByteOrder() == QImage::BigEndian);
		int x;
		uchar* q = src->scanLine(y);
		uchar* q2 = src->scanLine(y+1 < src->height() ? y + 1 : 0);
		for (int chan = 0; chan < 3; chan++) {
		    b = dst->scanLine(y);
		    int *l1 = (y&1) ? line2[chan] : line1[chan];
		    int *l2 = (y&1) ? line1[chan] : line2[chan];
		    if ( y == 0 ) {
			for (int i=0; i<sw; i++)
			    l1[i] = q[i*4+chan+endian];
		    }
		    if ( y+1 < src->height() ) {
			for (int i=0; i<sw; i++)
			    l2[i] = q2[i*4+chan+endian];
		    }
		    // Bi-directional error diffusion
		    if ( y&1 ) {
			for (x=0; x<sw; x++) {
			    int pix = QMAX(QMIN(5, (l1[x] * 5 + 128)/ 255), 0);
			    int err = l1[x] - pix * 255 / 5;
			    pv[chan][x] = pix;

			    // Spread the error around...
			    if ( x+1<sw ) {
				l1[x+1] += (err*7)>>4;
				l2[x+1] += err>>4;
			    }
			    l2[x]+=(err*5)>>4;
			    if (x>1)
				l2[x-1]+=(err*3)>>4;
			}
		    } else {
			for (x=sw; x-->0; ) {
			    int pix = QMAX(QMIN(5, (l1[x] * 5 + 128)/ 255), 0);
			    int err = l1[x] - pix * 255 / 5;
			    pv[chan][x] = pix;

			    // Spread the error around...
			    if ( x > 0 ) {
				l1[x-1] += (err*7)>>4;
				l2[x-1] += err>>4;
			    }
			    l2[x]+=(err*5)>>4;
			    if (x+1 < sw)
				l2[x+1]+=(err*3)>>4;
			}
		    }
		}
		if (endian) {
		    for (x=0; x<sw; x++) {
			*b++ = INDEXOF(pv[2][x],pv[1][x],pv[0][x]);
		    }
		} else {
		    for (x=0; x<sw; x++) {
			*b++ = INDEXOF(pv[0][x],pv[1][x],pv[2][x]);
		    }
		}
	    }
	}

	if ( ( conversion_flags & Dither_Mask ) == DiffuseDither ) {
	    delete line1[0];
	    delete line2[0];
	    delete line1[1];
	    delete line2[1];
	    delete line1[2];
	    delete line2[2];
	    delete pv[0];
	    delete pv[1];
	    delete pv[2];
	}

#undef MAX_R
#undef MAX_G
#undef MAX_B
#undef INDEXOF

    }

    return TRUE;
}


static bool convert_8_to_32( const QImage *src, QImage *dst )
{
    if ( !dst->create(src->width(), src->height(), 32) )
	return FALSE;				// create failed
    for ( int y=0; y<dst->height(); y++ ) {	// for each scan line...
	register uint *p = (uint *)dst->scanLine(y);
	uchar  *b = src->scanLine(y);
	uint *end = p + dst->width();
	while ( p < end )
	    *p++ = src->color(*b++);
    }
    return TRUE;
}


static bool convert_1_to_32( const QImage *src, QImage *dst )
{
    if ( !dst->create(src->width(), src->height(), 32) )
	return FALSE;				// could not create
    for ( int y=0; y<dst->height(); y++ ) {	// for each scan line...
	register uint *p = (uint *)dst->scanLine(y);
	uchar *b = src->scanLine(y);
	int x;
	if ( src->bitOrder() == QImage::BigEndian ) {
	    for ( x=0; x<dst->width(); x++ ) {
		*p++ = src->color( (*b >> (7 - (x & 7))) & 1 );
		if ( (x & 7) == 7 )
		    b++;
	    }
	} else {
	    for ( x=0; x<dst->width(); x++ ) {
		*p++ = src->color( (*b >> (x & 7)) & 1 );
		if ( (x & 7) == 7 )
		    b++;
	    }
	}
    }
    return TRUE;
}


static bool convert_1_to_8( const QImage *src, QImage *dst )
{
    if ( !dst->create(src->width(), src->height(), 8, 2) )
	return FALSE;				// something failed
    if (src->numColors() >= 2) {
	dst->setColor( 0, src->color(0) );	// copy color table
	dst->setColor( 1, src->color(1) );
    } else {
	// Unlikely, but they do exist
	if (src->numColors() >= 1)
	    dst->setColor( 0, src->color(0) );
	else
	    dst->setColor( 0, 0x00ffffff );
	dst->setColor( 1, 0x00000000 );
    }
    for ( int y=0; y<dst->height(); y++ ) {	// for each scan line...
	register uchar *p = dst->scanLine(y);
	uchar *b = src->scanLine(y);
	int x;
	if ( src->bitOrder() == QImage::BigEndian ) {
	    for ( x=0; x<dst->width(); x++ ) {
		*p++ = (*b >> (7 - (x & 7))) & 1;
		if ( (x & 7) == 7 )
		    b++;
	    }
	} else {
	    for ( x=0; x<dst->width(); x++ ) {
		*p++ = (*b >> (x & 7)) & 1;
		if ( (x & 7) == 7 )
		    b++;
	    }
	}
    }
    return TRUE;
}


//
// dither_to_1:  Uses selected dithering algorithm.
//

static bool dither_to_1( const QImage *src, QImage *dst,
		         int conversion_flags, bool fromalpha )
{
    if ( !dst->create(src->width(), src->height(), 1, 2, QImage::BigEndian) )
	return FALSE;				// something failed

    enum { Threshold, Ordered, Diffuse } dithermode;

    if ( fromalpha ) {
	if ( ( conversion_flags & AlphaDither_Mask ) == DiffuseAlphaDither )
	    dithermode = Diffuse;
	else if ( ( conversion_flags & AlphaDither_Mask ) == OrderedAlphaDither )
	    dithermode = Ordered;
	else
	    dithermode = Threshold;
    } else {
	if ( ( conversion_flags & Dither_Mask ) == ThresholdDither )
	    dithermode = Threshold;
	else if ( ( conversion_flags & Dither_Mask ) == OrderedDither )
	    dithermode = Ordered;
	else
	    dithermode = Diffuse;
    }

    dst->setColor( 0, qRgb(255, 255, 255) );
    dst->setColor( 1, qRgb(  0,	  0,   0) );
    int	  w = src->width();
    int	  h = src->height();
    int	  d = src->depth();
    uchar gray[256];				// gray map for 8 bit images
    bool  use_gray = d == 8;
    if ( use_gray ) {				// make gray map
	if ( fromalpha ) {
	    // Alpha 0x00 -> 0 pixels (white)
	    // Alpha 0xFF -> 1 pixels (black)
	    for ( int i=0; i<src->numColors(); i++ )
		gray[i] = (255 - (src->color(i) >> 24));
	} else {
	    // Pixel 0x00 -> 1 pixels (black)
	    // Pixel 0xFF -> 0 pixels (white)
	    for ( int i=0; i<src->numColors(); i++ )
		gray[i] = qGray( src->color(i) & 0x00ffffff );
	}
    }

    switch ( dithermode ) {
      case Diffuse: {
	int *line1 = new int[w];
	int *line2 = new int[w];
	int bmwidth = (w+7)/8;
	if ( !(line1 && line2) )
	    return FALSE;
	register uchar *p;
	uchar *end;
	int *b1, *b2;
	int wbytes = w * (d/8);
	p = src->bits();
	end = p + wbytes;
	b2 = line2;
	if ( use_gray ) {				// 8 bit image
	    while ( p < end )
		*b2++ = gray[*p++];
	} else {					// 32 bit image
	    if ( fromalpha ) {
		while ( p < end ) {
		    *b2++ = 255 - (*(uint*)p >> 24);
		    p += 4;
		}
	    } else {
		while ( p < end ) {
		    *b2++ = qGray(*(uint*)p);
		    p += 4;
		}
	    }
	}
	int x, y;
	for ( y=0; y<h; y++ ) {			// for each scan line...
	    int *tmp = line1; line1 = line2; line2 = tmp;
	    bool not_last_line = y < h - 1;
	    if ( not_last_line ) {			// calc. grayvals for next line
		p = src->scanLine(y+1);
		end = p + wbytes;
		b2 = line2;
		if ( use_gray ) {			// 8 bit image
		    while ( p < end )
			*b2++ = gray[*p++];
		} else {				// 24 bit image
		    if ( fromalpha ) {
			while ( p < end ) {
			    *b2++ = 255 - (*(uint*)p >> 24);
			    p += 4;
			}
		    } else {
			while ( p < end ) {
			    *b2++ = qGray(*(uint*)p);
			    p += 4;
			}
		    }
		}
	    }

	    int err;
	    p = dst->scanLine( y );
	    memset( p, 0, bmwidth );
	    b1 = line1;
	    b2 = line2;
	    int bit = 7;
	    for ( x=1; x<=w; x++ ) {
		if ( *b1 < 128 ) {			// black pixel
		    err = *b1++;
		    *p |= 1 << bit;
		} else {				// white pixel
		    err = *b1++ - 255;
		}
		if ( bit == 0 ) {
		    p++;
		    bit = 7;
		} else {
		    bit--;
		}
		if ( x < w )
		    *b1 += (err*7)>>4;		// spread error to right pixel
		if ( not_last_line ) {
		    b2[0] += (err*5)>>4;		// pixel below
		    if ( x > 1 )
			b2[-1] += (err*3)>>4;	// pixel below left
		    if ( x < w )
			b2[1] += err>>4;		// pixel below right
		}
		b2++;
	    }
	}
	delete [] line1;
	delete [] line2;
      } break;
      case Ordered: {
	static uint bm[16][16];
	static int init=0;
	if (!init) {
	    // Build a Bayer Matrix for dithering

	    init = 1;
	    int n, i, j;

	    bm[0][0]=0;

	    for (n=1; n<16; n*=2) {
		for (i=0; i<n; i++) {
		    for (j=0; j<n; j++) {
			bm[i][j]*=4;
			bm[i+n][j]=bm[i][j]+2;
			bm[i][j+n]=bm[i][j]+3;
			bm[i+n][j+n]=bm[i][j]+1;
		    }
		}
	    }

	    // Force black to black
	    bm[0][0]=1;
	}

	dst->fill( 0 );
	uchar** mline = dst->jumpTable();
	if ( d == 32 ) {
	    uint** line = (uint**)src->jumpTable();
	    for ( int i=0; i<h; i++ ) {
		uint  *p = line[i];
		uint  *end = p + w;
		uchar *m = mline[i];
		int bit = 7;
		int j = 0;
		if ( fromalpha ) {
		    while ( p < end ) {
			if ( (*p++ >> 24) >= bm[j++&15][i&15] )
			    *m |= 1 << bit;
			if ( bit == 0 ) {
			    m++;
			    bit = 7;
			} else {
			    bit--;
			}
		    }
		} else {
		    while ( p < end ) {
			if ( (uint)qGray(*p++) < bm[j++&15][i&15] )
			    *m |= 1 << bit;
			if ( bit == 0 ) {
			    m++;
			    bit = 7;
			} else {
			    bit--;
			}
		    }
		}
	    }
	} else /* ( d == 8 ) */ {
	    uchar** line = src->jumpTable();
	    for ( int i=0; i<h; i++ ) {
		uchar *p = line[i];
		uchar *end = p + w;
		uchar *m = mline[i];
		int bit = 7;
		int j = 0;
		while ( p < end ) {
		    if ( (uint)gray[*p++] < bm[j++&15][i&15] )
			*m |= 1 << bit;
		    if ( bit == 0 ) {
			m++;
			bit = 7;
		    } else {
			bit--;
		    }
		}
	    }
	}
      } break;
      default: { // Threshold:
	dst->fill( 0 );
	uchar** mline = dst->jumpTable();
	if ( d == 32 ) {
	    uint** line = (uint**)src->jumpTable();
	    for ( int i=0; i<h; i++ ) {
		uint  *p = line[i];
		uint  *end = p + w;
		uchar *m = mline[i];
		int bit = 7;
		if ( fromalpha ) {
		    while ( p < end ) {
			if ( (*p++ >> 24) >= 128 )
			    *m |= 1 << bit;  // Set mask "on"
			if ( bit == 0 ) {
			    m++;
			    bit = 7;
			} else {
			    bit--;
			}
		    }
		} else {
		    while ( p < end ) {
			if ( qGray(*p++) < 128 )
			    *m |= 1 << bit;  // Set pixel "black"
			if ( bit == 0 ) {
			    m++;
			    bit = 7;
			} else {
			    bit--;
			}
		    }
		}
	    }
	} else if ( d == 8 ) {
	    uchar** line = src->jumpTable();
	    for ( int i=0; i<h; i++ ) {
		uchar *p = line[i];
		uchar *end = p + w;
		uchar *m = mline[i];
		int bit = 7;
		while ( p < end ) {
		    if ( gray[*p++] < 128 )
			*m |= 1 << bit;		// Set mask "on"/ pixel "black"
		    if ( bit == 0 ) {
			m++;
			bit = 7;
		    } else {
			bit--;
		    }
		}
	    }
	}
      }
    }
    return TRUE;
}


/*!
  Converts the depth (bpp) of the image to \e depth and returns the
  converted image.  The original image is left undisturbed.

  The \e depth argument must be 1, 8 or 32.

  See QPixmap::convertFromImage for a description of the \a
  conversion_flags argument.

  Returns \c *this if \e depth is equal to the image depth, or a null
  image if this image cannot be converted.

  \sa depth(), isNull()
*/

QImage QImage::convertDepth( int depth, int conversion_flags ) const
{
    QImage image;
    if ( (data->d == 8 || data->d == 32) && depth == 1 ) // dither
	dither_to_1( this, &image, conversion_flags, FALSE );
    else if ( data->d == 32 && depth == 8 )	// 32 -> 8
	convert_32_to_8( this, &image, conversion_flags );
    else if ( data->d == 8 && depth == 32 )	// 8 -> 32
	convert_8_to_32( this, &image );
    else if ( data->d == 1 && depth == 8 )	// 1 -> 8
	convert_1_to_8( this, &image );
    else if ( data->d == 1 && depth == 32 )	// 1 -> 32
	convert_1_to_32( this, &image );
    else if ( data->d == depth )
	image = *this;				// no conversion
    else {
#if defined(CHECK_RANGE)
	if ( isNull() )
	    warning( "QImage::convertDepth: Image is a null image" );
	else
	    warning( "QImage::convertDepth: Depth %d not supported", depth );
#endif
    }
    return image;
}

/*!
  \overload QImage QImage::convertDepth( int depth ) const
*/

QImage QImage::convertDepth( int depth ) const
{
    return convertDepth( depth, 0 );
}

/*!
  Tests if the ( \a x, \a y ) is a valid coordinate in the image.
*/

bool QImage::valid( int x, int y ) const
{
    return x >= 0 && x < width()
        && y >= 0 && y < height();
}

/*!
  Returns the pixel index at the given coordinates.

  If (x,y) is not \link valid() valid\endlink, or if
  the image is not a paletted image (depth() \> 8), the results
  are undefined.
*/

int QImage::pixelIndex( int x, int y ) const
{
#if defined(CHECK_RANGE)
    if ( x < 0 || x > width() ) {
	warning( "QImage::pixel: x=%d out of range", x );
	return -12345;
    }
#endif
    uchar * s = scanLine( y );
    switch( depth() ) {
    case 1:
	if ( bitOrder() == QImage::LittleEndian )
	    return (*(s + (x >> 3)) >> (x & 7)) & 1;
	else
	    return (*(s + (x >> 3)) >> (7- (x & 7))) & 1;
    case 8:
	return (int)s[x];
    case 32:
#if defined(CHECK_RANGE)
	warning( "QImage::pixelIndex: Not applicable for 32-bpp images "
		 "(no palette)" );
#endif
	return 0;
    }
    return 0;
}


/*!
  Returns the actual color of the pixel at the given coordinates.

  If (x,y) is not \link valid() on the image\endlink, the results
  are undefined.
*/

QRgb QImage::pixel( int x, int y ) const
{
#if defined(CHECK_RANGE)
    if ( x < 0 || x > width() ) {
	warning( "QImage::pixel: x=%d out of range", x );
	return 12345;
    }
#endif
    uchar * s = scanLine( y );
    switch( depth() ) {
    case 1:
	if ( bitOrder() == QImage::LittleEndian )
	    return color( (*(s + (x >> 3)) >> (x & 7)) & 1 );
	else
	    return color( (*(s + (x >> 3)) >> (7- (x & 7))) & 1 );
    case 8:
	return color( (int)s[x] );
    case 32:
	return ((QRgb*)s)[x];
    default:
        return 100367;
    }
}


/*!
  Sets the pixel index or color at the given coordinates.

  If (x,y) is not \link valid() valid\endlink, or if
  the image is a paletted image (depth() \<= 8) and \a index_or_rgb
  \>= numColors(), the results are undefined.
*/

void QImage::setPixel( int x, int y, uint index_or_rgb )
{
    if ( x < 0 || x > width() ) {
#if defined(CHECK_RANGE)
	warning( "QImage::setPixel: x=%d out of range", x );
#endif
	return;
    }
    if ( depth() == 1 ) {
	uchar * s = scanLine( y );
	if ( index_or_rgb > 1) {
#if defined(CHECK_RANGE)
	    warning( "QImage::setPixel: index=%d out of range",
		     index_or_rgb );
#endif
	} else if ( bitOrder() == QImage::LittleEndian ) {
	    if (index_or_rgb==0)
		*(s + (x >> 3)) &= ~(1 << (x & 7));
	    else
		*(s + (x >> 3)) |= (1 << (x & 7));
	} else {
	    if (index_or_rgb==0)
		*(s + (x >> 3)) &= ~(1 << (7-(x & 7)));
	    else
		*(s + (x >> 3)) |= (1 << (7-(x & 7)));
	}
    } else if ( depth() == 8 ) {
	if (index_or_rgb > (uint)numColors()) {
#if defined(CHECK_RANGE)
	    warning( "QImage::setPixel: index=%d out of range",
		     index_or_rgb );
#endif
	    return;
	}
	uchar * s = scanLine( y );
	s[x] = index_or_rgb;
    } else if ( depth() == 32 ) {
	QRgb * s = (QRgb*)scanLine( y );
	s[x] = index_or_rgb;
    }
}


/*!
  Converts the bit order of the image to \e bitOrder and returns the converted
  image.

  Returns \c *this if the \e bitOrder is equal to the image bit order, or a
  null image if this image cannot be converted.

  \sa bitOrder(), setBitOrder()
*/

QImage QImage::convertBitOrder( Endian bitOrder ) const
{
    if ( isNull() || data->d != 1 ||		// invalid argument(s)
	 !(bitOrder == BigEndian || bitOrder == LittleEndian) ) {
	QImage nullImage;
	return nullImage;
    }
    if ( data->bitordr == bitOrder )		// nothing to do
	return *this;

    QImage image( data->w, data->h, 1, data->ncols, bitOrder );
    setup_bitflip();
    register uchar *p;
    uchar *end;
    uchar *b;
    p = bits();
    b = image.bits();
    end = p + numBytes();
    while ( p < end )
	*b++ = bitflip[*p++];
    memcpy( image.colorTable(), colorTable(), numColors()*sizeof(QRgb) );
    return image;
}

#if defined( HAS_BOOL_TYPE )
/*!
  OBSOLETE - Provided for backward compatibility on some compilers.
*/
QImage QImage::createAlphaMask( bool yes ) const
{
    return createAlphaMask( (int)yes );
}
#else
/*!
  OBSOLETE - Provided for backward compatibility on some compilers.
*/
QImage QImage::createAlphaMask() const
{
    return createAlphaMask( FALSE );
}
#endif

// ### Candidate (renamed) for qcolor.h
static
bool isGray(QRgb c)
{
    return qRed(c) == qGreen(c)
        && qRed(c) == qBlue(c);
}

/*!
  Returns TRUE if all the colors in the image are shades of
  gray, that is their R, G, and B components are equal.
  This function is slow for large 32-bit images.
*/
bool QImage::allGray() const
{
    if (depth()==32) {
	int p = width()*height();
	QRgb* b = (QRgb*)bits();
	while (p--)
	    if (!isGray(*b++))
		return FALSE;
    } else {
	if (!data->ctbl) return TRUE;
	for (int i=0; i<numColors(); i++)
	    if (!isGray(data->ctbl[i]))
		return FALSE;
    }
    return TRUE;
}

/*!
  Returns TRUE if the image is allGray(), \e and if the image is 32-bpp
  or a 256-color 8-bpp image for which color(i) is QRgb(i,i,i).
*/
bool QImage::isGrayscale() const
{
    switch (depth()) {
      case 32:
	return allGray();
      case 8: {
	for (int i=0; i<numColors(); i++)
	    if (data->ctbl[i] != qRgb(i,i,i))
		return FALSE;
	return TRUE;
      }
    }
    return FALSE;
}

inline int qAlpha(QRgb c)
{
    return c>>24;
}

inline QRgb qRgba(int r, int g, int b, int a)
{
    return qRgb(r,g,b) | (a<<24);
}

static
void pnmscale(const QImage& src, QImage& dst)
{
#define SCALE 4096
#define HALFSCALE 2048

    QRgb* xelrow = 0;
    QRgb* tempxelrow = 0;
    register QRgb* xP;
    register QRgb* nxP;
    int rows, cols, rowsread, newrows, newcols;
    register int row, col, needtoreadrow;
    const uchar maxval = 255;
    float xscale, yscale;
    long sxscale, syscale;
    register long fracrowtofill, fracrowleft;
    long* as;
    long* rs;
    long* gs;
    long* bs;
    int rowswritten = 0;

    cols = src.width();
    rows = src.height();
    newcols = dst.width();
    newrows = dst.height();

    xscale = (float) newcols / (float) cols;
    yscale = (float) newrows / (float) rows;

    sxscale = (long)(xscale * SCALE);
    syscale = (long)(yscale * SCALE);

    if ( newrows != rows )	/* shortcut Y scaling if possible */
	tempxelrow = new QRgb[cols];

    if ( src.hasAlphaBuffer() ) {
	dst.setAlphaBuffer(TRUE);
	as = new long[cols];
	for ( col = 0; col < cols; ++col )
	    as[col] = HALFSCALE;
    } else {
	as = 0;
    }
    rs = new long[cols];
    gs = new long[cols];
    bs = new long[cols];
    rowsread = 0;
    fracrowleft = syscale;
    needtoreadrow = 1;
    for ( col = 0; col < cols; ++col )
	rs[col] = gs[col] = bs[col] = HALFSCALE;
    fracrowtofill = SCALE;

    for ( row = 0; row < newrows; ++row ) {
	/* First scale Y from xelrow into tempxelrow. */
	if ( newrows == rows ) {
	    /* shortcut Y scaling if possible */
	    tempxelrow = xelrow = (QRgb*)src.scanLine(rowsread++);
	} else {
	    while ( fracrowleft < fracrowtofill ) {
		if ( needtoreadrow && rowsread < rows )
		    xelrow = (QRgb*)src.scanLine(rowsread++);
		for ( col = 0, xP = xelrow; col < cols; ++col, ++xP ) {
		    if (as)
			as[col] += fracrowleft * qAlpha( *xP );
		    rs[col] += fracrowleft * qRed( *xP );
		    gs[col] += fracrowleft * qGreen( *xP );
		    bs[col] += fracrowleft * qBlue( *xP );
		}
		fracrowtofill -= fracrowleft;
		fracrowleft = syscale;
		needtoreadrow = 1;
	    }
	    /* Now fracrowleft is >= fracrowtofill, so we can produce a row. */
	    if ( needtoreadrow && rowsread < rows ) {
		xelrow = (QRgb*)src.scanLine(rowsread++);
		needtoreadrow = 0;
	    }
	    for ( col = 0, xP = xelrow, nxP = tempxelrow;
		  col < cols; ++col, ++xP, ++nxP )
	    {
		register long a, r, g, b;

		r = rs[col] + fracrowtofill * qRed( *xP );
		g = gs[col] + fracrowtofill * qGreen( *xP );
		b = bs[col] + fracrowtofill * qBlue( *xP );
		r /= SCALE;
		if ( r > maxval ) r = maxval;
		g /= SCALE;
		if ( g > maxval ) g = maxval;
		b /= SCALE;
		if ( b > maxval ) b = maxval;
		if (as) {
		    a = as[col] + fracrowtofill * qAlpha( *xP );
		    a /= SCALE;
		    if ( a > maxval ) a = maxval;
		    *nxP = qRgba( (int)r, (int)g, (int)b, (int)a );
		    as[col] = HALFSCALE;
		} else {
		    *nxP = qRgb( (int)r, (int)g, (int)b );
		}
		rs[col] = gs[col] = bs[col] = HALFSCALE;
	    }
	    fracrowleft -= fracrowtofill;
	    if ( fracrowleft == 0 ) {
		fracrowleft = syscale;
		needtoreadrow = 1;
	    }
	    fracrowtofill = SCALE;
	}

	/* Now scale X from tempxelrow into dst and write it out. */
	if ( newcols == cols ) {
	    /* shortcut X scaling if possible */
	    memcpy(dst.scanLine(rowswritten++), tempxelrow, newcols*4);
	} else {
	    register long a, r, g, b;
	    register long fraccoltofill, fraccolleft = 0;
	    register int needcol;

	    nxP = (QRgb*)dst.scanLine(rowswritten++);
	    fraccoltofill = SCALE;
	    a = r = g = b = HALFSCALE;
	    needcol = 0;
	    for ( col = 0, xP = tempxelrow; col < cols; ++col, ++xP ) {
		fraccolleft = sxscale;
		while ( fraccolleft >= fraccoltofill ) {
		    if ( needcol ) {
			++nxP;
			a = r = g = b = HALFSCALE;
		    }
		    r += fraccoltofill * qRed( *xP );
		    g += fraccoltofill * qGreen( *xP );
		    b += fraccoltofill * qBlue( *xP );
		    r /= SCALE;
		    if ( r > maxval ) r = maxval;
		    g /= SCALE;
		    if ( g > maxval ) g = maxval;
		    b /= SCALE;
		    if ( b > maxval ) b = maxval;
		    if (as) {
			a += fraccoltofill * qAlpha( *xP );
			a /= SCALE;
			if ( a > maxval ) a = maxval;
			*nxP = qRgba( (int)r, (int)g, (int)b, (int)a );
		    } else {
			*nxP = qRgb( (int)r, (int)g, (int)b );
		    }
		    fraccolleft -= fraccoltofill;
		    fraccoltofill = SCALE;
		    needcol = 1;
		}
		if ( fraccolleft > 0 ) {
		    if ( needcol ) {
			++nxP;
			a = r = g = b = HALFSCALE;
			needcol = 0;
		    }
		    if (as)
			a += fraccolleft * qAlpha( *xP );
		    r += fraccolleft * qRed( *xP );
		    g += fraccolleft * qGreen( *xP );
		    b += fraccolleft * qBlue( *xP );
		    fraccoltofill -= fraccolleft;
		}
	    }
	    if ( fraccoltofill > 0 ) {
		--xP;
		if (as)
		    a += fraccolleft * qAlpha( *xP );
		r += fraccoltofill * qRed( *xP );
		g += fraccoltofill * qGreen( *xP );
		b += fraccoltofill * qBlue( *xP );
	    }
	    if ( ! needcol ) {
		r /= SCALE;
		if ( r > maxval ) r = maxval;
		g /= SCALE;
		if ( g > maxval ) g = maxval;
		b /= SCALE;
		if ( b > maxval ) b = maxval;
		if (as) {
		    a /= SCALE;
		    if ( a > maxval ) a = maxval;
		    *nxP = qRgba( (int)r, (int)g, (int)b, (int)a );
		} else {
		    *nxP = qRgb( (int)r, (int)g, (int)b );
		}
	    }
	}
    }

    if ( newrows != rows && tempxelrow )// Robust, tempxelrow might be 0 1 day
	delete [] tempxelrow;
    if ( as )				// Avoid purify complaint
	delete [] as;
    if ( rs )				// Robust, rs might be 0 one day
	delete [] rs;
    if ( gs )				// Robust, gs might be 0 one day
	delete [] gs;
    if ( bs )				// Robust, bs might be 0 one day
	delete [] bs;

#undef SCALE
#undef HALFSCALE
}

/*!
  \fn QImage QImage::smoothScale(int width, int height) const

  Returns a copy of the image smoothly scaled to \a width by \a height
  pixels.  For 32-bpp images, and 1-bpp/8-bpp color images, the result
  will be 32-bpp, while
  \link allGray() all-gray \endlink images (including black-and-white 1-bpp)
  will produce 8-bit
  \link isGrayscale() grayscale \endlink images with the palette spanning
  256 grays from black to white.

  This function uses code based on:

  pnmscale.c - read a portable anymap and scale it

  Copyright (C) 1989, 1991 by Jef Poskanzer.

  Permission to use, copy, modify, and distribute this software and its
  documentation for any purpose and without fee is hereby granted, provided
  that the above copyright notice appear in all copies and that both that
  copyright notice and this permission notice appear in supporting
  documentation.  This software is provided "as is" without express or
  implied warranty.
*/
QImage QImage::smoothScale(int w, int h) const
{
    if (depth()==32) {
	QImage img(w, h, 32);
	// 32-bpp to 32-bpp
	pnmscale(*this,img);
	return img;
    } else if (allGray() && !hasAlphaBuffer()) {
	// Inefficient
	return convertDepth(32).smoothScale(w,h).convertDepth(8);
    } else {
	// Inefficient
	return convertDepth(32).smoothScale(w,h);
    }
}

/*!
  Builds and returns a 1-bpp mask from the alpha buffer in this image.
  Returns a null image if \link setAlphaBuffer() alpha buffer mode\endlink
  is disabled.

  See QPixmap::convertFromImage for a description of the \a
  conversion_flags argument.

  The returned image has little-endian bit order, which you can
  convert to big-endianness using convertBitOrder().
*/

QImage QImage::createAlphaMask( int conversion_flags ) const
{
    if ( conversion_flags == 1 ) {
	// Old code is passing "TRUE".
	conversion_flags = DiffuseAlphaDither;
    }

    if ( isNull() || !hasAlphaBuffer() ) {
	QImage nullImage;
	return nullImage;
    }

    if ( depth() == 1 ) {
	// A monochrome pixmap, with alpha channels on those two colors.
	// Pretty unlikely, so use less efficient solution.
	return convertDepth(8, conversion_flags)
		.createAlphaMask( conversion_flags );
    }

    QImage mask1;
    dither_to_1( this, &mask1, conversion_flags, TRUE );
    return mask1;
}



/*!
  Creates and returns a 1-bpp heuristic mask for this image. It works by
  selecting a color from one of the corners, then chipping away pixels of
  that color, starting at all the edges.

  The four corners vote over which color is to be masked away. In
  case of a draw (this generally means that this function is not
  applicable to the image) the voting results are undocumented.

  The returned image has little-endian bit order, which you can
  convert to big-endianness using convertBitOrder().

  This function disregards the \link hasAlphaBuffer() alpha buffer. \endlink
*/

QImage QImage::createHeuristicMask( bool clipTight ) const
{
    if ( isNull() ) {
	QImage nullImage;
	return nullImage;
    }
    if ( depth() != 32 ) {
	QImage img32 = convertDepth(32);
	return img32.createHeuristicMask(clipTight);
    }

#define PIX(x,y)  (*((QRgb*)scanLine(y)+x) & 0x00ffffff)

    int w = width();
    int h = height();
    QImage m(w, h, 1, 2, QImage::LittleEndian);
    m.setColor( 0, 0xffffff );
    m.setColor( 1, 0 );
    m.fill( 0xff );

    QRgb background = PIX(0,0);
    if ( background != PIX(w-1,0) &&
	 background != PIX(0,h-1) &&
	 background != PIX(w-1,h-1) ) {
	background = PIX(w-1,0);
	if ( background != PIX(w-1,h-1) &&
	     background != PIX(0,h-1) &&
	     PIX(0,h-1) == PIX(w-1,h-1) ) {
	    background = PIX(w-1,h-1);
	}
    }

    int x,y;
    bool done = FALSE;
    uchar *ypp, *ypc, *ypn;
    while( !done ) {
	done = TRUE;
	ypn = m.scanLine(0);
	ypc = 0;
	for ( y = 0; y < h; y++ ) {
	    ypp = ypc;
	    ypc = ypn;
	    ypn = (y == h-1) ? 0 : m.scanLine(y+1);
	    QRgb *p = (QRgb *)scanLine(y);
	    for ( x = 0; x < w; x++ ) {
		// slowness here - it's possible to do six of these tests
		// together in one go.  oh well.
		if ( ( x == 0 || y == 0 || x == w-1 || y == h-1 ||
		       !(*(ypc + ((x-1) >> 3)) & (1 << ((x-1) & 7))) ||
		       !(*(ypc + ((x+1) >> 3)) & (1 << ((x+1) & 7))) ||
		       !(*(ypp + (x     >> 3)) & (1 << (x     & 7))) ||
		       !(*(ypn + (x     >> 3)) & (1 << (x     & 7))) ) &&
		     (	(*(ypc + (x     >> 3)) & (1 << (x     & 7))) ) &&
		     ( (*p & 0x00ffffff) == background ) ) {
		    done = FALSE;
		    *(ypc + (x >> 3)) &= ~(1 << (x & 7));
		}
		p++;
	    }
	}
    }

    if ( !clipTight ) {
	ypn = m.scanLine(0);
	ypc = 0;
	for ( y = 0; y < h; y++ ) {
	    ypp = ypc;
	    ypc = ypn;
	    ypn = (y == h-1) ? 0 : m.scanLine(y+1);
	    QRgb *p = (QRgb *)scanLine(y);
	    for ( x = 0; x < w; x++ ) {
		if ( (*p & 0x00ffffff) != background ) {
		    if ( x > 0 )
			*(ypc + ((x-1) >> 3)) |= (1 << ((x-1) & 7));
		    if ( x < w-1 )
			*(ypc + ((x+1) >> 3)) |= (1 << ((x+1) & 7));
		    if ( y > 0 )
			*(ypp + (x >> 3)) |= (1 << (x & 7));
		    if ( y < h-1 )
			*(ypn + (x >> 3)) |= (1 << (x & 7));
		}
		p++;
	    }
	}
    }

#undef PIX

    return m;
}



/*!
  Returns a string that specifies the image format of the file \e fileName,
  or null if the file cannot be read or if the format cannot be recognized.

  The QImageIO documentation lists the guaranteed supported image
  formats, or use the QImage::inputFormats() QImage::outputFormats()
  to get lists that include installed formats.

  \sa load(), save()
*/

const char *QImage::imageFormat( const char *fileName )
{
    return QImageIO::imageFormat(fileName);
}

/*!
  Returns a list of image formats which are supported for image input.
*/
QStrList QImage::inputFormats()
{
    return QImageIO::inputFormats();
}

/*!
  Returns a list of image formats which are supported for image output.
*/
QStrList QImage::outputFormats()
{
    return QImageIO::outputFormats();
}


/*!
  Loads an image from the file \a fileName.
  Returns TRUE if successful, or FALSE if the image could not be loaded.

  If \a format is specified, the loader attempts to read the image using the
  specified format. If \a format is not specified (default),
  the loader reads a few bytes from the header to guess the file format.

  The QImageIO documentation lists the supported image formats and
  explains how to add extra formats.

  \sa loadFromData(), save(), imageFormat(), QPixmap::load(), QImageIO
*/

bool QImage::load( const char *fileName, const char *format )
{
    QImageIO io( fileName, format );
    bool result = io.read();
    if ( result )
	operator=( io.image() );
    return result;
}

/*!
  Loads an image from the binary data in \e buf (\e len bytes).
  Returns TRUE if successful, or FALSE if the image could not be loaded.

  If \e format is specified, the loader attempts to read the image using the
  specified format. If \e format is not specified (default),
  the loader reads a few bytes from the header to guess the file format.

  The QImageIO documentation lists the supported image formats and
  explains how to add extra formats.

  \sa load(), save(), imageFormat(), QPixmap::loadFromData(), QImageIO
*/

bool QImage::loadFromData( const uchar *buf, uint len, const char *format )
{
    QByteArray a;
    a.setRawData( (char *)buf, len );
    QBuffer b( a );
    b.open( IO_ReadOnly );
    QImageIO io( &b, format );
    bool result = io.read();
    b.close();
    a.resetRawData( (char *)buf, len );
    if ( result )
	operator=( io.image() );
    return result;
}

/*!
  \overload
*/
bool QImage::loadFromData( QByteArray buf, const char *format )
{
    return loadFromData( (const uchar *)(buf.data()), buf.size(), format );
}

/*!
  Saves the image to the file \e fileName, using the image file format
  \e format.  Returns TRUE if successful, or FALSE if the image could not
  be saved.
  \sa load(), loadFromData(), imageFormat(), QPixmap::save(), QImageIO
*/

bool QImage::save( const char *fileName, const char *format ) const
{
    if ( isNull() )
	return FALSE;				// nothing to save
    QImageIO io( fileName, format );
    io.setImage( *this );
    return io.write();
}


/*****************************************************************************
  QImage stream functions
 *****************************************************************************/

/*!
  \relates QImage
  Writes an image to the stream as a BMP image.
  \sa QImage::save()
*/

QDataStream &operator<<( QDataStream &s, const QImage &image )
{
    QImageIO io( s.device(), "BMP" );
    io.setImage( image );
    io.write();
    return s;
}

/*!
  \relates QImage
  Reads an image from the stream.
  \sa QImage::load()
*/

QDataStream &operator>>( QDataStream &s, QImage &image )
{
    QImageIO io( s.device(), 0 );
    if ( io.read() )
	image = io.image();
    return s;
}


/*****************************************************************************
  Standard image io handlers (defined below)
 *****************************************************************************/

// standard image io handlers (defined below)
static void read_bmp_image( QImageIO * );
static void write_bmp_image( QImageIO * );
static void read_pbm_image( QImageIO * );
static void write_pbm_image( QImageIO * );
static void read_xbm_image( QImageIO * );
static void write_xbm_image( QImageIO * );

static void read_xpm_image( QImageIO * );
static void write_xpm_image( QImageIO * );

static void read_async_image( QImageIO * ); // Not in table of handlers

/*****************************************************************************
  Misc. utility functions
 *****************************************************************************/

static QString fbname( const char *fileName )	// get file basename (sort of)
{
    QString s = fileName;
    if ( !s.isEmpty() ) {
	int i;
	if ( (i=s.findRev('/')) >= 0 )
	    s = &s[i];
	if ( (i=s.findRev('\\')) >= 0 )
	    s = &s[i];
	QRegExp r( "[a-zA-Z][a-zA-Z0-9_]*" );
	int p = r.match( s, 0, &i );
	if ( p >= 0 )
	    s = &s[p];
	s.truncate(i);
    }
    if ( s.isEmpty() )
	s = "dummy";
    return s;
}

static void swapPixel01( QImage *image )	// 1-bpp: swap 0 and 1 pixels
{
    int i;
    if ( image->depth() == 1 && image->numColors() == 2 ) {
	register uint *p = (uint *)image->bits();
	int nbytes = image->numBytes();
	for ( i=0; i<nbytes/4; i++ ) {
	    *p = ~*p;
	    p++;
	}
	uchar *p2 = (uchar *)p;
	for ( i=0; i<(nbytes&3); i++ ) {
	    *p2 = ~*p2;
	    p2++;
	}
	QRgb t = image->color(0);		// swap color 0 and 1
	image->setColor( 0, image->color(1) );
	image->setColor( 1, t );
    }
}


/*****************************************************************************
  QImageIO member functions
 *****************************************************************************/

/*!
  \class QImageIO qimage.h

  \brief The QImageIO class contains parameters for loading
  and saving images.

  \ingroup images
  \ingroup io

  QImageIO contains a QIODevice object that is used for image data I/O.
  The programmer can install new image file formats in addition to those
  that Qt implements.

  Qt currently supports the following image file formats: GIF (reading
  only), BMP, XBM, XPM and PNM.  The different PNM formats are: PBM
  (P1), PGM (P2), PPM (P3), PBMRAW (P4), PGMRAW (P5) and PPMRAW (P6).

  Additional formats are available with the
  <a href="imageio.html">Qt Image IO Extension</a> package.

  You will normally not need to use this class, QPixmap::load(),
  QPixmap::save() and QImage contain most of the needed functionality.

  For image files which contain sequences of images, only the first is
  read.  See the QMovie for loading multiple images.

  PBM, PGM, and PPM format output is only supported in PPMRAW format.

  \warning Unisys has changed its position regarding GIF.  If you are
  in a country where Unisys holds a patent on LZW compression and/or
  decompression, Unisys may require you to license that technology.
  These countries include Canada, Japan, the USA, France, Germany,
  Italy and the UK.  There is more information on Unisys web site: <a
  href="http://corp2.unisys.com/LeadStory/lzwfaq.html">Overview of
  Unisys' position.</a> GIF support may be removed in a future version
  of Qt.  We recommend using the PNG format, which is available in the
  <a href="imageio.html">Qt Image IO Extension</a> package.

  \sa QImage, QPixmap, QFile, QMovie
*/

/*!
  Constructs a QImageIO object with all parameters set to zero.
*/

QImageIO::QImageIO()
{
    iostat = 0;
    iodev  = 0;
    params = descr = 0;
}

/*!
  Constructs a QImageIO object with an I/O device and a format tag.
*/

QImageIO::QImageIO( QIODevice *ioDevice, const char *format )
    : frmt(format)
{
    iostat = 0;
    iodev  = ioDevice;
    params = descr = 0;
}

/*!
  Constructs a QImageIO object with a file name and a format tag.
*/

QImageIO::QImageIO( const char *fileName, const char *format )
    : frmt(format), fname(fileName)
{
    iostat = 0;
    iodev  = 0;
    params = descr = 0;
}

/*!
  Destroys the object an all related data.
*/

QImageIO::~QImageIO()
{
    if ( params )
	delete [] params;
    if ( descr )
	delete [] descr;
}


/*****************************************************************************
  QImageIO image handler functions
 *****************************************************************************/

class QImageHandler
{
public:
    QImageHandler( const char *f, const char *h, bool tm,
		   image_io_handler r, image_io_handler w );
    QString	      format;			// image format
    QRegExp	      header;			// image header pattern
    bool	      text_mode;		// image I/O mode
    image_io_handler  read_image;		// image read function
    image_io_handler  write_image;		// image write function
};

QImageHandler::QImageHandler( const char *f, const char *h, bool t,
			      image_io_handler r, image_io_handler w )
    : format(f), header(h)
{
    text_mode	= t;
    read_image	= r;
    write_image = w;
}

typedef Q_DECLARE(QListM,QImageHandler) QIHList;// list of image handlers
static QIHList *imageHandlers = 0;

static void cleanup_image_handlers()		// cleanup image handler list
{
    delete imageHandlers;
    imageHandlers = 0;
}

static void init_image_handlers()		// initialize image handlers
{
    if ( !imageHandlers ) {
	imageHandlers = new QIHList;
	CHECK_PTR( imageHandlers );
	imageHandlers->setAutoDelete( TRUE );
	qAddPostRoutine( cleanup_image_handlers );
	QImageIO::defineIOHandler( "BMP", "^BM", 0,
				   read_bmp_image, write_bmp_image );
	QImageIO::defineIOHandler( "PBM", "^P1", "T",
				   read_pbm_image, write_pbm_image );
	QImageIO::defineIOHandler( "PBMRAW", "^P4", 0,
				   read_pbm_image, write_pbm_image );
	QImageIO::defineIOHandler( "PGM", "^P2", "T",
				   read_pbm_image, write_pbm_image );
	QImageIO::defineIOHandler( "PGMRAW", "^P5", 0,
				   read_pbm_image, write_pbm_image );
	QImageIO::defineIOHandler( "PPM", "^P3", "T",
				   read_pbm_image, write_pbm_image );
	QImageIO::defineIOHandler( "PPMRAW", "^P6", 0,
				   read_pbm_image, write_pbm_image );
	QImageIO::defineIOHandler( "XBM", "^#define", "T",
				   read_xbm_image, write_xbm_image );
	QImageIO::defineIOHandler( "XPM", "/\\*.XPM.\\*/", "T",
				   read_xpm_image, write_xpm_image );
    }
}

static QImageHandler *get_image_handler( const char *format )
{						// get pointer to handler
    if ( !imageHandlers )
	init_image_handlers();
    register QImageHandler *p = imageHandlers->first();
    while ( p ) {				// traverse list
	if ( p->format == format )
	    return p;
	p = imageHandlers->next();
    }
    return 0;					// no such handler
}

/*!
  Defines a image IO handler for a specified image format.
  An image IO handler is responsible for reading and writing images.

  \arg \e format is the name of the format.
  \arg \e header is a regular expression that recognizes the image header.
  \arg \e flags is "T" for text formats like PBM; generally you will
	  want to use 0.
  \arg \e read_image is a function to read an image of this format.
  \arg \e write_image is a function to write an image of this format.

  Both read_image and write_image are of type image_io_handler, which is
  a function pointer.

  Example:
  \code
    void readGIF( QImageIO *image )
    {
      // read the image, using the image->ioDevice()
    }

    void writeGIF( QImageIO *image )
    {
      // write the image, using the image->ioDevice()
    }

    // add the GIF image handler

    QImageIO::defineIOHandler( "GIF",
			       "^GIF[0-9][0-9][a-z]",
			       0,
			       readGIF,
			       writeGIF );
  \endcode

  Prior to comparison with the regular expression, the file header is
  converted to change all 0 bytes into 1 bytes. This is done because 0
  is such a common header byte yet regular expressions cannot match it.

  For image formats supporting incremental display, such as sequences
  of animated frames, see the QImageFormatType class.
*/

void QImageIO::defineIOHandler( const char *format,
				const char *header,
				const char *flags,
				image_io_handler read_image,
				image_io_handler write_image )
{
    if ( !imageHandlers )
	init_image_handlers();
    QImageHandler *p;
    p = new QImageHandler( format, header, flags && *flags == 'T',
			   read_image, write_image );
    CHECK_PTR( p );
    imageHandlers->insert( 0, p );
}


/*****************************************************************************
  QImageIO normal member functions
 *****************************************************************************/

/*!
  \fn const QImage &QImageIO::image() const
  Returns the image currently set.
  \sa setImage()
*/

/*!
  \fn int QImageIO::status() const
  Returns the image IO status.	A non-zero value indicates an error, while 0
  means that the IO operation was successful.
  \sa setStatus()
*/

/*!
  \fn const char *QImageIO::format() const
  Returns the image format string, or 0 if no format has been explicitly set.
*/

/*!
  \fn QIODevice *QImageIO::ioDevice() const
  Returns the IO device currently set.
  \sa setIODevice()
*/

/*!
  \fn const char *QImageIO::fileName() const
  Returns the file name currently set.
  \sa setFileName()
*/

/*!
  \fn const char *QImageIO::parameters() const
  Returns image parameters string.
  \sa setParameters()
*/

/*!
  \fn const char *QImageIO::description() const
  Returns the image description string.
  \sa setDescription()
*/


/*!
  Sets the image.
  \sa image()
*/

void QImageIO::setImage( const QImage &image )
{
    im = image;
}

/*!
  Sets the image IO status.  A non-zero value indicates an error, while 0 means
  that the IO operation was successful.
  \sa status()
*/

void QImageIO::setStatus( int status )
{
    iostat = status;
}

/*!
  Sets the image format name of the image about to be read or written.

  It is necessary to specify a format before writing an image.

  It is not necessary to specify a format before reading an image.
  If not format has been set, Qt guesses the image format before reading
  it.  If a format is set, but the image has another (valid) format,
  the image will not be read.

  \sa read(), write(), format()
*/

void QImageIO::setFormat( const char *format )
{
    frmt = format;
}

/*!
  Sets the IO device to be used for reading or writing an image.

  Setting the IO device allows images to be read/written to any
  block-oriented QIODevice.

  If \e ioDevice is not null, this IO device will override file name
  settings.

  \sa setFileName()
*/

void QImageIO::setIODevice( QIODevice *ioDevice )
{
    iodev = ioDevice;
}

/*!
  Sets the name of the file to read or write an image.
  \sa setIODevice()
*/

void QImageIO::setFileName( const char *fileName )
{
    fname = fileName;
}

/*!
  Sets the image parameters string for image handlers that require
  special parameters.

  Although all image formats supported by Qt ignore the parameters string,
  it will be useful for future extentions or contributions (like JPEG).
*/

void QImageIO::setParameters( const char *parameters )
{
    if ( params )
	delete [] params;
    params = qstrdup( parameters );
}

/*!
  Sets the image description string for image handlers that support image
  descriptions.

  Currently, no image format supported by Qt use the description string.
*/

void QImageIO::setDescription( const char *description )
{
    if ( descr )
	delete [] descr;
    descr = qstrdup( description );
}


/*!
  Returns a string that specifies the image format of the file \e fileName,
  or null if the file cannot not be read or if the format is not recognized.
*/

const char *QImageIO::imageFormat( const char *fileName )
{
    QFile file( fileName );
    if ( !file.open(IO_ReadOnly) )
	return 0;
    const char *format = imageFormat( &file );
    file.close();
    return format;
}

/*!
  Returns a string that specifies the image format of the image read from
  \e d, or null if the file cannot be read or if the format is not recognized.
*/

const char *QImageIO::imageFormat( QIODevice *d )
{
    const int buflen = 14;
    char buf[buflen];
    if ( imageHandlers == 0 )
	init_image_handlers();
    int pos   = d->at();			// save position
    int rdlen = d->readBlock( buf, buflen );	// read a few bytes

    // Try asynchronous loaders first (before we 0->1 the header),
    // but overwrite if found in IOHandlers.
    const char *format = QImageDecoder::formatName((uchar*)buf, rdlen);

    for ( int n = 0; n<rdlen; n++ )
	if ( buf[n] == '\0' )
	    buf[n] = '\001';
    if ( d->status() == IO_Ok && rdlen > 0 ) {
	buf[rdlen-1] = '\0';
	QImageHandler *p = imageHandlers->first();
	while ( p ) {
	    if ( p->header.match(buf) != -1 ) { // try match with headers
		format = p->format;
		break;
	    }
	    p = imageHandlers->next();
	}
    }
    d->at( pos );				// restore position
    return format;
}

/*!
  Returns a sorted list of image formats which are supported for image input.
*/
QStrList QImageIO::inputFormats()
{
    QStrList result;

    if ( imageHandlers == 0 )
	init_image_handlers();

    // Include asynchronous loaders first.
    result = QImageDecoder::inputFormats();

    QImageHandler *p = imageHandlers->first();
    while ( p ) {
	if ( p->read_image && !result.contains(p->format) )
	    result.inSort(p->format);
	p = imageHandlers->next();
    }

    return result;
}

/*!
  Returns a sorted list of image formats which are supported for image output.
*/
QStrList QImageIO::outputFormats()
{
    QStrList result;

    if ( imageHandlers == 0 )
	init_image_handlers();

    // Include asynchronous writers (!) first.
    // (None)

    QImageHandler *p = imageHandlers->first();
    while ( p ) {
	if ( p->write_image && !result.contains(p->format) )
	    result.inSort(p->format);
	p = imageHandlers->next();
    }

    return result;
}



/*!
  Reads an image into memory and returns TRUE if the image was successfully
  read.

  Before reading an image, you must set an IO device or a file name.
  If both an IO device and a file name has been set, then the IO device will
  be used.

  Setting the image file format string is optional.

  Note that this function does \e not set the \link format() format\endlink
  used to read the image.  If you need that information, use the imageFormat()
  static functions.

  Example:

  \code
    QImageIO iio;
    QPixmap  pixmap;
    iio.setFileName( "burger.bmp" );
    if ( image.read() )			// ok
	pixmap = iio.image();		// convert to pixmap
  \endcode

  \sa setIODevice(), setFileName(), setFormat(), write(), QPixmap::load()
*/

bool QImageIO::read()
{
    QFile	   file;
    const char	  *image_format;
    QImageHandler *h;

    if ( iodev ) {				// read from io device
	// ok, already open
    } else if ( !fname.isEmpty() ) {		// read from file
	file.setName( fname );
	if ( !file.open(IO_ReadOnly) )
	    return FALSE;			// cannot open file
	iodev = &file;
    } else {					// no file name or io device
	return FALSE;
    }
    if (frmt.isEmpty()) {
	// Try to guess format
	image_format = imageFormat( iodev );	// get image format

	if ( !image_format ) {
	    if ( file.isOpen() ) {			// unknown format
		file.close();
		iodev = 0;
	    }
	    return FALSE;
	}
    } else {
	image_format = frmt;
    }

    h = get_image_handler( image_format );
    if ( file.isOpen() ) {
#if !defined(UNIX)
	if ( h && h->text_mode ) {		// reopen in translated mode
	    file.close();
	    file.open( IO_ReadOnly | IO_Translate );
	}
	else
#endif
	    file.at( 0 );			// position to start
    }
    iostat = 1;					// assume error

    if ( h && h->read_image ) {
	(*h->read_image)( this );
    } else {
	// Format name, but no handler - must be an asychronous reader
	read_async_image( this );
    }

    if ( file.isOpen() ) {			// image was read using file
	file.close();
	iodev = 0;
    }
    return iostat == 0;				// image successfully read?
}


/*!
  Writes an image to an IO device and returns TRUE if the image was
  successfully written.

  Before writing an image, you must set an IO device or a file name.
  If both an IO device and a file name has been set, then the IO
  device will be used.

  The image will be written using the specified image format.

  Example:
  \code
    QImageIO iio;
    QImage   im;
    im = pixmap;				// convert to image
    iio.setImage( im );
    iio.setFileName( "burger.bmp" );
    iio.setFormat( "BMP" );
    iio.write();				// TRUE if ok
  \endcode

  \sa setIODevice(), setFileName(), setFormat(), read(), QPixmap::save()
*/

bool QImageIO::write()
{
    if ( frmt.isEmpty() )
	return FALSE;
    QImageHandler *h = get_image_handler( frmt );
    if ( !h || !h->write_image ) {
#if defined(CHECK_RANGE)
	warning( "QImageIO::write: No such image format handler: %s",
		 format() );
#endif
	return FALSE;
    }
    QFile file;
    if ( iodev )
	;
    else if ( !fname.isEmpty() ) {
	file.setName( fname );
	int fmode = h->text_mode ? IO_WriteOnly|IO_Translate : IO_WriteOnly;
	if ( !file.open(fmode) )		// couldn't create file
	    return FALSE;
	iodev = &file;
    }
    iostat = 1;
    (*h->write_image)( this );
    if ( file.isOpen() ) {			// image was written using file
	file.close();
	iodev = 0;
    }
    return iostat == 0;				// image successfully written?
}


/*****************************************************************************
  BMP (DIB) image read/write functions
 *****************************************************************************/

const int BMP_FILEHDR_SIZE = 14;		// size of BMP_FILEHDR data

struct BMP_FILEHDR {				// BMP file header
    char   bfType[2];				// "BM"
    INT32  bfSize;				// size of file
    INT16  bfReserved1;
    INT16  bfReserved2;
    INT32  bfOffBits;				// pointer to the pixmap bits
};

QDataStream &operator>>( QDataStream &s, BMP_FILEHDR &bf )
{						// read file header
    s.readRawBytes( bf.bfType, 2 );
    s >> bf.bfSize >> bf.bfReserved1 >> bf.bfReserved2 >> bf.bfOffBits;
    return s;
}

QDataStream &operator<<( QDataStream &s, const BMP_FILEHDR &bf )
{						// write file header
    s.writeRawBytes( bf.bfType, 2 );
    s << bf.bfSize << bf.bfReserved1 << bf.bfReserved2 << bf.bfOffBits;
    return s;
}


const int BMP_OLD  = 12;			// old Windows/OS2 BMP size
const int BMP_WIN  = 40;			// new Windows BMP size
const int BMP_OS2  = 64;			// new OS/2 BMP size

const int BMP_RGB  = 0;				// no compression
const int BMP_RLE8 = 1;				// run-length encoded, 8 bits
const int BMP_RLE4 = 2;				// run-length encoded, 4 bits

struct BMP_INFOHDR {				// BMP information header
    INT32  biSize;				// size of this struct
    INT32  biWidth;				// pixmap width
    INT32  biHeight;				// pixmap height
    INT16  biPlanes;				// should be 1
    INT16  biBitCount;				// number of bits per pixel
    INT32  biCompression;			// compression method
    INT32  biSizeImage;				// size of image
    INT32  biXPelsPerMeter;			// horizontal resolution
    INT32  biYPelsPerMeter;			// vertical resolution
    INT32  biClrUsed;				// number of colors used
    INT32  biClrImportant;			// number of important colors
};


QDataStream &operator>>( QDataStream &s, BMP_INFOHDR &bi )
{
    s >> bi.biSize;
    if ( bi.biSize == BMP_WIN || bi.biSize == BMP_OS2 ) {
	s >> bi.biWidth >> bi.biHeight >> bi.biPlanes >> bi.biBitCount;
	s >> bi.biCompression >> bi.biSizeImage;
	s >> bi.biXPelsPerMeter >> bi.biYPelsPerMeter;
	s >> bi.biClrUsed >> bi.biClrImportant;
    }
    else {					// probably old Windows format
	INT16 w, h;
	s >> w >> h >> bi.biPlanes >> bi.biBitCount;
	bi.biWidth  = w;
	bi.biHeight = h;
	bi.biCompression = BMP_RGB;		// no compression
	bi.biSizeImage = 0;
	bi.biXPelsPerMeter = bi.biYPelsPerMeter = 0;
	bi.biClrUsed = bi.biClrImportant = 0;
    }
    return s;
}

QDataStream &operator<<( QDataStream &s, const BMP_INFOHDR &bi )
{
    s << bi.biSize;
    s << bi.biWidth << bi.biHeight;
    s << bi.biPlanes;
    s << bi.biBitCount;
    s << bi.biCompression;
    s << bi.biSizeImage;
    s << bi.biXPelsPerMeter << bi.biYPelsPerMeter;
    s << bi.biClrUsed << bi.biClrImportant;
    return s;
}

static
bool read_dib( QDataStream& s, int offset, int startpos, QImage& image )
{
    BMP_INFOHDR bi;
    QIODevice* d = s.device();

    s >> bi;					// read BMP info header
    if ( d->atEnd() )				// end of stream/file
	return FALSE;
#if 0
    //debug( "bfOffBits........%d", bf.bfOffBits );
    debug( "biSize...........%d", bi.biSize );
    debug( "biWidth..........%d", bi.biWidth );
    debug( "biHeight.........%d", bi.biHeight );
    debug( "biPlanes.........%d", bi.biPlanes );
    debug( "biBitCount.......%d", bi.biBitCount );
    debug( "biCompression....%d", bi.biCompression );
    debug( "biSizeImage......%d", bi.biSizeImage );
    debug( "biXPelsPerMeter..%d", bi.biXPelsPerMeter );
    debug( "biYPelsPerMeter..%d", bi.biYPelsPerMeter );
    debug( "biClrUsed........%d", bi.biClrUsed );
    debug( "biClrImportant...%d", bi.biClrImportant );
#endif
    int w = bi.biWidth,	 h = bi.biHeight,  nbits = bi.biBitCount;
    int t = bi.biSize,	 comp = bi.biCompression;

    if ( !(nbits == 1 || nbits == 4 || nbits == 8 || nbits == 24) ||
	 bi.biPlanes != 1 || comp > BMP_RLE4 )
	return FALSE;					// weird BMP image
    if ( !(comp == BMP_RGB || (nbits == 4 && comp == BMP_RLE4) ||
	   (nbits == 8 && comp == BMP_RLE8)) )
	 return FALSE;				// weird compression type

    int ncols;
    int depth;
    switch ( nbits ) {
	case 24:
	    depth = 32;
	    break;
	case 8:
	case 4:
	    depth = 8;
	    break;
	default:
	    depth = 1;
    }
    if ( depth == 32 )				// there's no colormap
	ncols = 0;
    else					// # colors used
	ncols = bi.biClrUsed ? bi.biClrUsed : 1 << nbits;

    image.create( w, h, depth, ncols, nbits == 1 ?
		  QImage::BigEndian : QImage::IgnoreEndian );
    if ( image.isNull() )			// could not create image
	return FALSE;

    d->at( startpos + BMP_FILEHDR_SIZE + bi.biSize ); // goto start of colormap

    if ( ncols > 0 ) {				// read color table
	uchar rgb[4];
	int   rgb_len = t == BMP_OLD ? 3 : 4;
	for ( int i=0; i<ncols; i++ ) {
	    d->readBlock( (char *)rgb, rgb_len );
	    image.setColor( i, qRgb(rgb[2],rgb[1],rgb[0]) );
	    if ( d->atEnd() )			// truncated file
		return FALSE;
	}
    }

    if (offset>=0)
	d->at( startpos + offset );		// start of image data

    int	     bpl = image.bytesPerLine();
    uchar **line = image.jumpTable();

    if ( nbits == 1 ) {				// 1 bit BMP image
	while ( --h >= 0 ) {
	    if ( d->readBlock((char*)line[h],bpl) != bpl )
		break;
	}
	if ( ncols == 2 && qGray(image.color(0)) < qGray(image.color(1)) )
	    swapPixel01( &image );		// pixel 0 is white!
    }

    else if ( nbits == 4 ) {			// 4 bit BMP image
	int    buflen = ((w+7)/8)*4;
	uchar *buf    = new uchar[buflen];
	CHECK_PTR( buf );
	if ( comp == BMP_RLE4 ) {		// run length compression
	    int x=0, y=0, b, c, i;
	    register uchar *p = line[h-1];
	    while ( y < h ) {
		if ( (b=d->getch()) == EOF )
		    break;
		if ( b == 0 ) {			// escape code
		    switch ( (b=d->getch()) ) {
			case 0:			// end of line
			    x = 0;
			    y++;
			    p = line[h-y-1];
			    break;
			case 1:			// end of image
			case EOF:		// end of file
			    y = h;		// exit loop
			    break;
			case 2:			// delta (jump)
			    x += d->getch();
			    y += d->getch();
			    p = line[h-y-1] + x;
			    break;
			default:		// absolute mode
			    i = (c = b)/2;
			    while ( i-- ) {
				b = d->getch();
				*p++ = b >> 4;
				*p++ = b & 0x0f;
			    }
			    if ( c & 1 )
				*p++ = d->getch() >> 4;
			    if ( (((c & 3) + 1) & 2) == 2 )
				d->getch();	// align on word boundary
			    x += c;
		    }
		} else {			// encoded mode
		    i = (c = b)/2;
		    b = d->getch();		// 2 pixels to be repeated
		    while ( i-- ) {
			*p++ = b >> 4;
			*p++ = b & 0x0f;
		    }
		    if ( c & 1 )
			*p++ = b >> 4;
		    x += c;
		}
	    }
	} else if ( comp == BMP_RGB ) {		// no compression
	    while ( --h >= 0 ) {
		if ( d->readBlock((char*)buf,buflen) != buflen )
		    break;
		register uchar *p = line[h];
		uchar *b = buf;
		for ( int i=0; i<w/2; i++ ) {	// convert nibbles to bytes
		    *p++ = *b >> 4;
		    *p++ = *b++ & 0x0f;
		}
		if ( w & 1 )			// the last nibble
		    *p = *b >> 4;
	    }
	}
	delete [] buf;
    }

    else if ( nbits == 8 ) {			// 8 bit BMP image
	if ( comp == BMP_RLE8 ) {		// run length compression
	    int x=0, y=0, b;
	    register uchar *p = line[h-1];
	    while ( y < h ) {
		if ( (b=d->getch()) == EOF )
		    break;
		if ( b == 0 ) {			// escape code
		    switch ( (b=d->getch()) ) {
			case 0:			// end of line
			    x = 0;
			    y++;
			    p = line[h-y-1];
			    break;
			case 1:			// end of image
			case EOF:		// end of file
			    y = h;		// exit loop
			    break;
			case 2:			// delta (jump)
			    x += d->getch();
			    y += d->getch();
			    p = line[h-y-1] + x;
			    break;
			default:		// absolute mode
			    d->readBlock( (char *)p, b );
			    if ( (b & 1) == 1 )
				d->getch();	// align on word boundary
			    x += b;
			    p += b;
		    }
		} else {			// encoded mode
		    memset( p, d->getch(), b ); // repeat pixel
		    x += b;
		    p += b;
		}
	    }
	} else if ( comp == BMP_RGB ) {		// uncompressed
	    while ( --h >= 0 ) {
		if ( d->readBlock((char *)line[h],bpl) != bpl )
		    break;
	    }
	}
    }

    else if ( nbits == 24 ) {			// 24 bit BMP image
	register QRgb *p;
	QRgb  *end;
	uchar *buf24 = new uchar[bpl];
	int    bpl24 = ((w*24+31)/32)*4;
	uchar *b;
	while ( --h >= 0 ) {
	    p = (QRgb *)line[h];
	    end = p + w;
	    if ( d->readBlock( (char *)buf24,bpl24) != bpl24 )
		break;
	    b = buf24;
	    while ( p < end ) {
		*p++ = (b[0] << 16) | (b[1] << 8) | b[2];
		b += 3;
	    }
	}
	delete[] buf24;
    }

    return TRUE;
}

bool qt_read_dib( QDataStream& s, QImage& image )
{
    return read_dib(s,-1,-BMP_FILEHDR_SIZE,image);
}


static void read_bmp_image( QImageIO *iio )
{
    QIODevice  *d = iio->ioDevice();
    QDataStream s( d );
    BMP_FILEHDR bf;
    int		startpos = d->at();

    s.setByteOrder( QDataStream::LittleEndian );// Intel byte order
    s >> bf;					// read BMP file header

    if ( strncmp(bf.bfType,"BM",2) != 0 )	// not a BMP image
	return;

    QImage	image;
    if (read_dib( s, bf.bfOffBits, startpos, image )) {
	iio->setImage( image );
	iio->setStatus( 0 );			// image ok
    }
}

bool qt_write_dib( QDataStream& s, QImage image )
{
    int		nbits;
    int		bpl_bmp;
    int		bpl = image.bytesPerLine();

    QIODevice* d = s.device();

    if ( image.depth() == 8 && image.numColors() <= 16 ) {
	bpl_bmp = (((bpl+1)/2+3)/4)*4;
	nbits = 4;
    } else if ( image.depth() == 32 ) {
	bpl_bmp = ((image.width()*24+31)/32)*4;
	nbits = 24;
    } else {
	bpl_bmp = bpl;
	nbits = image.depth();
    }

    BMP_INFOHDR bi;

    bi.biSize	       = BMP_WIN;		// build info header
    bi.biWidth	       = image.width();
    bi.biHeight	       = image.height();
    bi.biPlanes	       = 1;
    bi.biBitCount      = nbits;
    bi.biCompression   = BMP_RGB;
    bi.biSizeImage     = bpl_bmp*image.height();
    bi.biXPelsPerMeter = 2834;			// 72 dpi
    bi.biYPelsPerMeter = 2834;
    bi.biClrUsed       = image.numColors();
    bi.biClrImportant  = image.numColors();
    s << bi;					// write info header

    if ( image.depth() != 32 ) {		// write color table
	uchar *color_table = new uchar[4*image.numColors()];
	uchar *rgb = color_table;
	QRgb *c = image.colorTable();
	for ( int i=0; i<image.numColors(); i++ ) {
	    *rgb++ = qBlue ( c[i] );
	    *rgb++ = qGreen( c[i] );
	    *rgb++ = qRed  ( c[i] );
	    *rgb++ = 0;
	}
	d->writeBlock( (char *)color_table, 4*image.numColors() );
	delete [] color_table;
    }

    if ( image.depth() == 1 && image.bitOrder() != QImage::BigEndian )
	image = image.convertBitOrder( QImage::BigEndian );

    int	 y;

    if ( nbits == 1 || nbits == 8 ) {		// direct output
	for ( y=image.height()-1; y>=0; y-- )
	    d->writeBlock( (char*)image.scanLine(y), bpl );
	return TRUE;
    }

    uchar *buf	= new uchar[bpl_bmp];
    uchar *b, *end;
    register uchar *p;

    memset( buf, 0, bpl_bmp );
    for ( y=image.height()-1; y>=0; y-- ) {	// write the image bits
	if ( nbits == 4 ) {			// convert 8 -> 4 bits
	    p = image.scanLine(y);
	    b = buf;
	    end = b + image.width()/2;
	    while ( b < end ) {
		*b++ = (*p << 4) | (*(p+1) & 0x0f);
		p += 2;
	    }
	    if ( image.width() & 1 )
		*b = *p << 4;
	} else {				// 32 bits: RGB -> BGR
	    QRgb *p   = (QRgb *)image.scanLine( y );
	    QRgb *end = p + image.width();
	    b = buf;
	    while ( p < end ) {
		*b++ = (uchar)(*p >> 16);
		*b++ = (uchar)(*p >> 8);
		*b++ = (uchar)*p++;
	    }
	}
	d->writeBlock( (char*)buf, bpl_bmp );
    }
    delete[] buf;
    return TRUE;
}


static void write_bmp_image( QImageIO *iio )
{
    QIODevice  *d = iio->ioDevice();
    QImage	image = iio->image();
    QDataStream s( d );
    BMP_FILEHDR bf;
    int		bpl_bmp;
    int		bpl = image.bytesPerLine();

    // Code partially repeated in qt_write_dib
    if ( image.depth() == 8 && image.numColors() <= 16 ) {
	bpl_bmp = (((bpl+1)/2+3)/4)*4;
    } else if ( image.depth() == 32 ) {
	bpl_bmp = ((image.width()*24+31)/32)*4;
    } else {
	bpl_bmp = bpl;
    }

    iio->setStatus( 0 );
    s.setByteOrder( QDataStream::LittleEndian );// Intel byte order
    strncpy( bf.bfType, "BM", 2 );		// build file header
    bf.bfReserved1 = bf.bfReserved2 = 0;	// reserved, should be zero
    bf.bfOffBits   = BMP_FILEHDR_SIZE + BMP_WIN + image.numColors()*4;
    bf.bfSize	   = bf.bfOffBits + bpl_bmp*image.height();
    s << bf;					// write file header

    qt_write_dib( s, image );
}


/*****************************************************************************
  PBM/PGM/PPM (ASCII and RAW) image read/write functions
 *****************************************************************************/

static int read_pbm_int( QIODevice *d )
{
    int	  c;
    int	  val = -1;
    bool  digit;
    const int buflen = 100;
    char  buf[buflen];
    while ( TRUE ) {
	if ( (c=d->getch()) == EOF )		// end of file
	    break;
	digit = isdigit(c);
	if ( val != -1 ) {
	    if ( digit ) {
		val = 10*val + c - '0';
		continue;
	    } else {
		if ( c == '#' )			// comment
		    d->readLine( buf, buflen );
		break;
	    }
	}
	if ( digit )				// first digit
	    val = c - '0';
	else if ( isspace(c) )
	    continue;
	else if ( c == '#' )
	    d->readLine( buf, buflen );
	else
	    break;
    }
    return val;
}

static void read_pbm_image( QImageIO *iio )	// read PBM image data
{
    const int	buflen = 300;
    char	buf[buflen];
    QIODevice  *d = iio->ioDevice();
    int		w, h, nbits, mcc, y;
    int		pbm_bpl;
    char	type;
    bool	raw;
    QImage	image;

    d->readBlock( buf, 3 );			// read P[1-6]<white-space>
    if ( !(buf[0] == 'P' && isdigit(buf[1]) && isspace(buf[2])) )
	return;
    switch ( (type=buf[1]) ) {
	case '1':				// ascii PBM
	case '4':				// raw PBM
	    nbits = 1;
	    break;
	case '2':				// ascii PGM
	case '5':				// raw PGM
	    nbits = 8;
	    break;
	case '3':				// ascii PPM
	case '6':				// raw PPM
	    nbits = 32;
	    break;
	default:
	    return;
    }
    raw = type >= '4';
    w = read_pbm_int( d );			// get image width
    h = read_pbm_int( d );			// get image height
    if ( nbits == 1 )
	mcc = 0;				// ignore max color component
    else
	mcc = read_pbm_int( d );		// get max color component
    if ( w <= 0 || w > 32767 || h <= 0 || h > 32767 || mcc < 0 || mcc > 32767 )
	return;					// weird P.M image

    if ( mcc > 255 )
	mcc = 255;
    image.create( w, h, nbits, 0,
		  nbits == 1 ? QImage::BigEndian :  QImage::IgnoreEndian );
    if ( image.isNull() )
	return;

    pbm_bpl = (nbits*w+7)/8;			// bytes per scanline in PBM

    if ( raw ) {				// read raw data
	if ( nbits == 32 ) {			// type 6
	    pbm_bpl = 3*w;
	    uchar *buf24 = new uchar[pbm_bpl], *b;
	    QRgb  *p;
	    QRgb  *end;
	    for ( y=0; y<h; y++ ) {
		d->readBlock( (char *)buf24, pbm_bpl );
		p = (QRgb *)image.scanLine( y );
		end = p + w;
		b = buf24;
		while ( p < end ) {
		    *p++ = qRgb(b[0],b[1],b[2]);
		    b += 3;
		}
	    }
	    delete[] buf24;
	} else {				// type 4,5
	    for ( y=0; y<h; y++ )
		d->readBlock( (char *)image.scanLine(y), pbm_bpl );
	}
    } else {					// read ascii data
	register uchar *p;
	int n;
	for ( y=0; y<h; y++ ) {
	    p = image.scanLine( y );
	    n = pbm_bpl;
	    if ( nbits == 1 ) {
		int b;
		while ( n-- ) {
		    b = 0;
		    for ( int i=0; i<8; i++ )
			b = (b << 1) | (read_pbm_int(d) & 1);
		    *p++ = b;
		}
	    } else if ( nbits == 8 ) {
		while ( n-- ) {
		    *p++ = read_pbm_int( d );
		}
	    } else {				// 32 bits
		n /= 4;
		int r, g, b;
		while ( n-- ) {
		    r = read_pbm_int( d );
		    g = read_pbm_int( d );
		    b = read_pbm_int( d );
		    *((QRgb*)p) = qRgb( r, g, b );
		    p += 4;
		}
	    }
	}
    }

    if ( nbits == 1 ) {				// bitmap
	image.setNumColors( 2 );
	image.setColor( 0, qRgb(255,255,255) ); // white
	image.setColor( 1, qRgb(0,0,0) );	// black
    } else if ( nbits == 8 ) {			// graymap
	image.setNumColors( mcc+1 );
	for ( int i=0; i<=mcc; i++ )
	    image.setColor( i, qRgb(i*255/mcc,i*255/mcc,i*255/mcc) );
    }

    iio->setImage( image );
    iio->setStatus( 0 );			// image ok
}


static void write_pbm_image( QImageIO *iio )
{
    QIODevice* out = iio->ioDevice();
    QString str;

    QImage  image  = iio->image();
    QString format = iio->format();
    format = format.left(3);			// ignore RAW part
    bool gray = format == "PGM";

    if ( format == "PBM" && image.depth() != 1 ) {
	image = image.convertDepth(1);
    } else if ( image.depth() == 1 ) {
	image = image.convertDepth(8);
    }

    if ( image.depth() == 1 && image.numColors() == 2 ) {
	if ( qGray(image.color(0)) < qGray(image.color(1)) ) {
	    // 0=dark/black, 1=light/white - invert
	    image.detach();
	    for ( int y=0; y<image.height(); y++ ) {
		uchar *p = image.scanLine(y);
		uchar *end = p + image.bytesPerLine();
		while ( p < end )
		    *p++ ^= 0xff;
	    }
	}
    }

    uint w = image.width();
    uint h = image.height();

    str.sprintf("P\n%d %d\n", w, h);

    switch (image.depth()) {
        case 1: {
	    str.insert(1, '4');
	    if ((uint)out->writeBlock(str, str.length()) != str.length()) {
		iio->setStatus(1);
		return;
	    }
	    w = (w+7)/8;
	    for (uint y=0; y<h; y++) {
		uchar* line = image.scanLine(y);
		if ( w != (uint)out->writeBlock((char*)line, w) ) {
	    	    iio->setStatus(1);
		    return;
		}
	    }
	    }
	    break;
	
	case 8: {
	    str.insert(1, '6');
	    str.append("255\n");
	    if ((uint)out->writeBlock(str, str.length()) != str.length()) {
		iio->setStatus(1);
		return;
	    }
	    QRgb  *color = image.colorTable();
	    uchar *buf   = new uchar[w*3];
	    for (uint y=0; y<h; y++) {
		uchar *b = image.scanLine(y);
		uchar *p = buf;
		uchar *end = buf+w*3;
		if ( gray ) {
		    while ( p < end ) {
			uchar g = (uchar)qGray(color[*b++]);
			*p++ = g;
			*p++ = g;
			*p++ = g;
		    }
		} else {
		    while ( p < end ) {
			QRgb rgb = color[*b++];
			*p++ = qRed(rgb);
			*p++ = qGreen(rgb);
			*p++ = qBlue(rgb);
		    }
		}
		if ( w*3 != (uint)out->writeBlock((char*)buf, w*3) ) {
		    iio->setStatus(1);
		    return;
		}
	    }
	    delete [] buf;
	    }
	    break;
	
	case 32: {
	    str.insert(1, '6');
	    str.append("255\n");
	    if ((uint)out->writeBlock(str, str.length()) != str.length()) {
		iio->setStatus(1);
		return;
	    }
	    uchar *buf = new uchar[w*3];
	    for (uint y=0; y<h; y++) {
		QRgb  *b = (QRgb*)image.scanLine(y);
		uchar *p = buf;
		uchar *end = buf+w*3;
		if ( gray ) {
		    while ( p < end ) {
			uchar g = (uchar)qGray(*b++);
			*p++ = g;
			*p++ = g;
			*p++ = g;
		    }
		} else {
		    while ( p < end ) {
			QRgb rgb = *b++;
			*p++ = qRed(rgb);
			*p++ = qGreen(rgb);
			*p++ = qBlue(rgb);
		    }
		}
		if ( w*3 != (uint)out->writeBlock((char*)buf, w*3) ) {
		    iio->setStatus(1);
		    return;
		}
	    }
	    delete [] buf;
	    }
    }

    iio->setStatus(0);
}


class QImageIOFrameGrabber : public QImageConsumer {
public:
    QImageIOFrameGrabber() : framecount(0) { }

    QImageDecoder *decoder;
    int framecount;

    void changed(const QRect&) { }
    void end() { }
    void frameDone() { framecount++; }
    void setLooping(int) { }
    void setFramePeriod(int) { }
    void setSize(int, int) { }
};

static void read_async_image( QImageIO *iio )
{
    const int buf_len = 512;
    uchar buffer[buf_len];
    QIODevice  *d = iio->ioDevice();
    QImageIOFrameGrabber* consumer = new QImageIOFrameGrabber();
    QImageDecoder decoder(consumer);
    consumer->decoder = &decoder;

    for (;;) {
	int length = d->readBlock((char*)buffer, buf_len);
	if ( length <= 0 ) {
	    iio->setStatus(length);
	    break;
	}
	uchar* b = buffer;
	int r = -1;
	while (length > 0 && consumer->framecount==0) {
	    r = decoder.decode(b, length);
	    if ( r <= 0 ) break;
	    b += r;
	    length -= r;
	}
	if ( consumer->framecount ) {
	    // Stopped after first frame
	    iio->setImage(decoder.image());
	    iio->setStatus(0);
	    break;
	}
	if ( r <= 0 ) {
	    iio->setStatus(r);
	    break;
	}
    }

    delete consumer;
}


/*****************************************************************************
  X bitmap image read/write functions
 *****************************************************************************/

static inline int hex2byte( register char *p )
{
    return ((isdigit(*p)     ? *p     - '0' : toupper(*p)     - 'A' + 10)<< 4)|
	   ( isdigit(*(p+1)) ? *(p+1) - '0' : toupper(*(p+1)) - 'A' + 10);
}

static void read_xbm_image( QImageIO *iio )
{
    const int	buflen = 300;
    char	buf[buflen];
    QRegExp	r1, r2;
    QIODevice  *d = iio->ioDevice();
    int		i;
    int		w=-1, h=-1;
    QImage	image;

    r1 = "^#define[\x20\t]+[a-zA-Z0-9_]+[\x20\t]+";
    r2 = "[0-9]+";
    d->readLine( buf, buflen );			// "#define .._width <num>"
    if ( r1.match(buf,0,&i)==0 && r2.match(buf,i)==i )
	w = atoi( &buf[i] );
    d->readLine( buf, buflen );			// "#define .._height <num>"
    if ( r1.match(buf,0,&i)==0 && r2.match(buf,i)==i )
	h = atoi( &buf[i] );
    if ( w <= 0 || w > 32767 || h <= 0 || h > 32767 )
	return;					// format error

    while ( TRUE ) {				// scan for data
	if ( d->readLine(buf, buflen) == 0 )	// end of file
	    return;
	if ( strstr(buf,"0x") != 0 )		// does line contain data?
	    break;
    }

    image.create( w, h, 1, 2, QImage::LittleEndian );
    if ( image.isNull() )
	return;

    image.setColor( 0, qRgb(255,255,255) );	// white
    image.setColor( 1, qRgb(0,0,0) );		// black

    int	   x = 0, y = 0;
    uchar *b = image.scanLine(0);
    char  *p = strstr( buf, "0x" );
    w = (w+7)/8;				// byte width

    while ( y < h ) {				// for all encoded bytes...
	if ( p ) {				// p = "0x.."
	    *b++ = hex2byte(p+2);
	    p += 2;
	    if ( ++x == w && ++y < h ) {
		b = image.scanLine(y);
		x = 0;
	    }
	    p = strstr( p, "0x" );
	} else {				// read another line
	    if ( !d->readLine(buf,buflen) )	// EOF ==> truncated image
		break;
	    p = strstr( buf, "0x" );
	}
    }

    iio->setImage( image );
    iio->setStatus( 0 );			// image ok
}


static void write_xbm_image( QImageIO *iio )
{
    QIODevice *d = iio->ioDevice();
    QImage     image = iio->image();
    int	       w = image.width();
    int	       h = image.height();
    int	       i;
    QString    s = fbname(iio->fileName());	// get file base name
    char       buf[100];

    sprintf( buf, "#define %s_width %d\n",  (const char *)s, w );
    d->writeBlock( buf, strlen(buf) );
    sprintf( buf, "#define %s_height %d\n", (const char *)s, h );
    d->writeBlock( buf, strlen(buf) );
    sprintf( buf, "static char %s_bits[] = {\n ", (const char *)s );
    d->writeBlock( buf, strlen(buf) );

    iio->setStatus( 0 );

    if ( image.depth() != 1 )
	image = image.convertDepth( 1 );	// dither
    if ( image.bitOrder() != QImage::LittleEndian )
	image = image.convertBitOrder( QImage::LittleEndian );

    bool invert = qGray(image.color(0)) < qGray(image.color(1));
    char hexrep[16];
    for ( i=0; i<10; i++ )
	hexrep[i] = '0' + i;
    for ( i=10; i<16; i++ )
	hexrep[i] = 'a' -10 + i;
    if ( invert ) {
	char t;
	for ( i=0; i<8; i++ ) {
	    t = hexrep[15-i];
	    hexrep[15-i] = hexrep[i];
	    hexrep[i] = t;
	}
    }
    int bcnt = 0;
    register char *p = buf;
    uchar *b = image.scanLine(0);
    int	 x=0, y=0;
    int nbytes = image.numBytes();
    w = (w+7)/8;
    while ( nbytes-- ) {			// write all bytes
	*p++ = '0';  *p++ = 'x';
	*p++ = hexrep[*b >> 4];
	*p++ = hexrep[*b++ & 0xf];
	if ( ++x == w && y < h-1 ) {
	    b = image.scanLine(++y);
	    x = 0;
	}
	if ( nbytes > 0 ) {
	    *p++ = ',';
	    if ( ++bcnt > 14 ) {
		*p++ = '\n';
		*p++ = ' ';
		*p   = '\0';
		d->writeBlock( buf, strlen(buf) );
		p = buf;
		bcnt = 0;
	    }
	}
    }
    strcpy( p, " };\n" );
    d->writeBlock( buf, strlen(buf) );
}


/*****************************************************************************
  XPM image read/write functions
 *****************************************************************************/


// Skip until ", read until the next ", return the rest in *buf
// Returns FALSE on error, TRUE on success

static bool read_xpm_string( QString &buf, QIODevice *d,
			     const char **source, int &index )
{
    if ( source ) {
	buf = source[index++];
	return TRUE;
    }

    if ( buf.size() < 69 )	    //# just an approximation
	buf.resize( 123 );

    buf[0] = '\0';
    int c;
    int i;
    while ( (c=d->getch()) != EOF && c != '"' )
	;
    if ( c == EOF ) {
	return FALSE;
    }
    i = 0;
    while ( (c=d->getch()) != EOF && c != '"' ) {
	if ( i == (int)buf.size() )
	    buf.resize( i*2+42 );
	buf[i++] = c;
    }
    if ( c == EOF ) {
	return FALSE;
    }

    if ( i == (int)buf.size() ) // always use a 0 terminator
	buf.resize( i+1 );
    buf[i] = '\0';
    return TRUE;
}


//
// INTERNAL
//
// Reads an .xpm from either the QImageIO or from the const char **.
// One of the two HAS to be 0, the other one is used.
//

static void read_xpm_image_or_array( QImageIO * iio, const char ** source,
				     QImage & image)
{
    QString buf;
    QIODevice *d = 0;
    buf.resize( 200 );

    int i, cpp, ncols, w, h, index = 0;

    if ( iio ) {
	iio->setStatus( 1 );
	d = iio ? iio->ioDevice() : 0;
	d->readLine( buf.data(), buf.size() );	// "/* XPM */"
	QRegExp r ("/\\*.XPM.\\*/" );
	if ( r.match(buf) < 0 )
	    return;					// bad magic
    } else if ( !source ) {
	return;
    }

    if ( !read_xpm_string( buf, d, source, index ) )
	return;

    if ( sscanf( buf, "%d %d %d %d", &w, &h, &ncols, &cpp ) < 4 )
	return;					// < 4 numbers parsed

    if ( ncols > 256 || cpp > 15 )
	return;

    image.create( w, h, 8, ncols );

    QDict<void> colorMap( 569, TRUE );
    colorMap.setAutoDelete( FALSE );

    int currentColor;

    for( currentColor=0; currentColor < ncols; ++currentColor ) {
	if ( !read_xpm_string( buf, d, source, index ) )
	    return;
	QString index;
	index = buf.left( cpp );
	buf = buf.mid( cpp, buf.length() ).simplifyWhiteSpace().lower();
	if ( buf[0] != 'c' || buf[1] != ' ' ) {
	    i = buf.find( " c " );
	    if ( i < 0 )
		return;				// no c specification
	    buf = buf.mid( i+1, buf.length() );
	}
	QRegExp r( "^c #[a-zA-Z0-9]*$" );
	if ( r.match( buf ) > -1 ) {
	    i = buf.find( ' ', 2 );
	    if ( i >= 0 )
		buf.resize( i );
	    QString red, green, blue;		// it's an RGB value...
	    switch( buf.length() ) {
	    case 6:
		red = buf.mid( 3, 1 );
		green = buf.mid( 4, 1 );
		blue = buf.mid( 5, 1 );
		break;
	    case 9:
		red = buf.mid( 3, 2 );
		green = buf.mid( 5, 2 );
		blue = buf.mid( 7, 2 );
		break;
	    case 15:
		// forget those lsbs
		red = buf.mid( 3, 2 );
		green = buf.mid( 7, 2 );
		blue = buf.mid( 11, 2 );
		break;
	    default:
		return;		// bad number of rgb digits
	    }
	    int r, g, b;
	    if ( sscanf( red, "%x", &r ) != 1 ||
		 sscanf( green, "%x", &g ) != 1 ||
		 sscanf( blue, "%x", &b ) != 1 )
		return;		// couldn't sscanf
	    image.setColor( currentColor, 0xff000000 | qRgb( r, g, b ) );
	    colorMap.insert( index, (void*)(currentColor+1) );
	} else if ( buf == "c none" ) {
	    image.setAlphaBuffer( TRUE );
	    int transparentColor = currentColor;
	    image.setColor( transparentColor, RGB_MASK & qRgb( 200,200,200 ) );
	    colorMap.insert( index, (void*)(transparentColor+1) );
	} else {
	    r = " [a-z] ";	// symbolic color names: die die die
	    i = r.match( buf );
	    QString colorName = buf.mid(2, i > -1 ? i-2 : buf.length());
	    QColor c( colorName );
	    image.setColor( currentColor, 0xff000000 | c.rgb() );
	    colorMap.insert( index, (void*)(currentColor+1) );
	}
    }

    // Read pixels
    for( int y=0; y<h; y++ ) {
	if ( !read_xpm_string( buf, d, source, index ) )
	    return;
	uchar *p = image.scanLine(y);
	uchar *d = (uchar *)buf.data();
	uchar *end = d + buf.length();
	int x;
	if ( cpp == 1 ) {
	    char b[2];
	    b[1] = '\0';
	    for ( x=0; x<w && d<end; x++ ) {
		b[0] = *d++;
		*p++ = (uchar)((int)(long)colorMap[b] - 1);
	    }
	} else {
	    char b[16];
	    b[cpp] = '\0';
	    for ( x=0; x<w && d<end; x++ ) {
		strncpy( b, (char *)d, cpp );
		*p++ = (uchar)((int)(long)colorMap[b] - 1);
		d += cpp;
	    }
	}
    }
    if ( iio ) {
	iio->setImage( image );
	iio->setStatus( 0 );			// image ok
    }
}


static void read_xpm_image( QImageIO * iio )
{
    QImage i;
    (void)read_xpm_image_or_array( iio, 0, i );
    return;
}


static char* xpm_color_name( int cpp, int index )
{
    static char returnable[3];
    if ( cpp > 1 ) {
	if ( index == 1 )
	    index = 64*44+21+1;
	else if ( index == 64*44+21+1 )
	    index = 1;
	returnable[0] = ".#abcdefghijklmnopqrstuvwxyz"
			"ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789"[(index-1)/64];
	returnable[1] = ".#abcdefghijklmnopqrstuvwxyz"
			"ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789"[(index-1)%64];
	returnable[2] = '\0';
    } else {
	returnable[0] = ".#abcdefghijklmnopqrstuvwxyz"
			"ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789"[index-1];
	returnable[1] = '\0';
    }
    return returnable;
}


// write XPM image data
static void write_xpm_image( QImageIO * iio )
{
    if ( iio )
	iio->setStatus( 1 );
    else
	return;

    QImage image;
    if ( iio->image().depth() != 32 )
	image = iio->image().convertDepth( 32 );
    else
	image = iio->image();

    QIntDict<void> colorMap( 569 );

    int w = image.width(), h = image.height(), colors=0;
    int x, y;

    // build color table
    for( y=0; y<h; y++ ) {
	QRgb * yp = (QRgb *)image.scanLine( y );
	for( x=0; x<w; x++ ) {
	    int color = (int)*(yp + x);
	    if ( !colorMap.find( color ) )
		colorMap.insert( color, (void*)(++colors) );
	}
    }

    int cpp = colors > 64 ? 2 : 1;
    QString line;

    // write header
    QTextStream s( iio->ioDevice() );
    s << "/* XPM */" << endl
      << "static char*" << fbname(iio->fileName()) << "[]={" << endl
      << "\"" << w << " " << h << " " << colors << " " << cpp << "\"";

    // write palette
    QIntDictIterator<void> c( colorMap );
    while ( c.current() ) {
	QRgb color = (QRgb)c.currentKey();
	if ( image.hasAlphaBuffer() && color == (color & RGB_MASK) )
	    line.sprintf( "\"%s c None\"",
			  xpm_color_name( cpp, (int)(long)c.current() ) );
	else
	    line.sprintf( "\"%s c #%02x%02x%02x\"",
			  xpm_color_name( cpp, (int)(long)c.current() ),
			  qRed( color ),
			  qGreen( color ),
			  qBlue( color ) );
	++c;
	s << "," << endl << line;
    }

    // write pixels
    for( y=0; y<h; y++ ) {
	QRgb * yp = (QRgb *) image.scanLine( y );
	line.resize( cpp*w + 1 );
	for( x=0; x<w; x++ ) {
	    int color = (int)(*(yp + x));
	    const char * chars = xpm_color_name( cpp,
						 (int)(long)colorMap.find(color) );
	    line[ x*cpp ] = chars[0];
	    if ( cpp == 2 )
		line[ x*cpp + 1 ] = chars[1];
	}
	line[ cpp*w ] = '\0';
	s << "," << endl << "\"" << line << "\"";
    }
    s << "};" << endl;

    iio->setStatus( 0 );
}

/*!
  Note:  currently no closest-color search is made.  If colors are found that
  are not in the palette, the palette may not be used at all.  This result
  should not be considered valid, as it may change in future implementations.

  Currently inefficient for non 32-bit images.
*/
QImage QImage::convertDepthWithPalette( int d, QRgb* palette, int palette_count, int conversion_flags ) const
{
    if ( depth() == 1 ) {
	return convertDepth( 8, conversion_flags )
	       .convertDepthWithPalette( d, palette, palette_count, conversion_flags );
    } else if ( depth() == 8 ) {
	// ### this could be easily made more efficient
	return convertDepth( 32, conversion_flags )
	       .convertDepthWithPalette( d, palette, palette_count, conversion_flags );
    } else {
	QImage result;
	convert_32_to_8( this, &result,
	    (conversion_flags&~DitherMode_Mask) | AvoidDither,
	    palette, palette_count );
	return result.convertDepth( d );
    }
}

static
bool
haveSamePalette(const QImage& a, const QImage& b)
{
    if (a.depth() != b.depth()) return FALSE;
    if (a.numColors() != b.numColors()) return FALSE;
    QRgb* ca = a.colorTable();
    QRgb* cb = b.colorTable();
    for (int i=a.numColors(); i--; ) {
	if (*ca++ != *cb++) return FALSE;
    }
    return TRUE;
}

/*!
  Copies a \a sw by \a sh pixel area from \a src to position (\a dx, \a dy)
  in \a dst.  The pixels copied from source (src) are converted according
  to \a conversion_flags if it is incompatible with the destination (dst).

  The copying is clipped if areas outside \a src or \a dst are specified.

  If \a sw is -1, it is adjusted to src->width().
  Similarly, if \a sh is -1, it is adjusted to src->height().

  Currently inefficient for non 32-bit images.
*/
void bitBlt( QImage* dst, int dx, int dy, const QImage* src,
		int sx, int sy, int sw, int sh, int conversion_flags )
{
    // Parameter correction
    if ( sw < 0 ) sw = src->width();
    if ( sh < 0 ) sh = src->height();
    if ( sx < 0 ) { dx -= sx; sw += sx; sx = 0; }
    if ( sy < 0 ) { dy -= sy; sh += sy; sy = 0; }
    if ( dx < 0 ) { sx -= dx; sw += dx; dx = 0; }
    if ( dy < 0 ) { sy -= dy; sh += dy; dy = 0; }
    if ( sx + sw > src->width() ) sw = src->width() - sx;
    if ( sy + sh > src->height() ) sh = src->height() - sy;
    if ( dx + sw > dst->width() ) sw = dst->width() - dx;
    if ( dy + sh > dst->height() ) sh = dst->height() - dy;
    if ( sw <= 0 || sh <= 0 ) return; // Nothing left to copy
    if ( (dst->data == src->data) && dx==sx && dy==sy ) return; // Same pixels

    // "Easy" to copy if both same depth and one of:
    //   - 32 bit
    //   - 8 bit, identical palette
    //   - 1 bit, identical palette and byte-aligned area
    //
    if ( haveSamePalette(*dst,*src)
	&& ( dst->depth() != 1 ||
	      !( (dx&7) || (sx&7) ||
		    ((sw&7) && (sx+sw < src->width()) ||
			       (dx+sw < dst->width()) ) ) ) )
    {
	// easy to copy
    } else if ( dst->depth() != 32 ) {
	QImage dstconv = dst->convertDepth( 32 );
	bitBlt( &dstconv, dx, dy, src, sx, sy, sw, sh,
	   (conversion_flags&~DitherMode_Mask) | AvoidDither );
	*dst = dstconv.convertDepthWithPalette( dst->depth(),
	    dst->colorTable(), dst->numColors() );
	return;
    }

    // Now assume dst is 32-bit

    if ( dst->depth() != src->depth() ) {
	if ( sw == src->width() && sh == src->height() || dst->depth()==32 ) {
	    QImage srcconv = src->convertDepth( dst->depth(), conversion_flags );
	    bitBlt( dst, dx, dy, &srcconv, sx, sy, sw, sh, conversion_flags );
	} else {
	    QImage srcconv = src->copy( sx, sy, sw, sh ); // ie. bitBlt
	    bitBlt( dst, dx, dy, &srcconv, 0, 0, sw, sh, conversion_flags );
	}
	return;
    }

    // Now assume both are the same depth.

    // Now assume both are 32-bit or 8-bit with compatible palettes.

    // "Easy"

    switch ( dst->depth() ) {
      case 1:
	{
	    uchar* d = dst->scanLine(dy) + dx/8;
	    uchar* s = src->scanLine(sy) + sx/8;
	    const int bw = (sw+7)/8;
	    if ( bw < 64 ) {
		// Trust ourselves
		const int dd = dst->bytesPerLine() - bw;
		const int ds = src->bytesPerLine() - bw;
		while ( sh-- ) {
		    for ( int t=bw; t--; )
			*d++ = *s++;
		    d += dd;
		    s += ds;
		}
	    } else {
		// Trust libc
		const int dd = dst->bytesPerLine();
		const int ds = src->bytesPerLine();
		while ( sh-- ) {
		    memcpy( d, s, bw );
		    d += dd;
		    s += ds;
		}
	    }
	}
	break;
      case 8:
	{
	    uchar* d = dst->scanLine(dy) + dx;
	    uchar* s = src->scanLine(sy) + sx;
	    if ( sw < 64 ) {
		// Trust ourselves
		const int dd = dst->bytesPerLine() - sw;
		const int ds = src->bytesPerLine() - sw;
		while ( sh-- ) {
		    for ( int t=sw; t--; )
			*d++ = *s++;
		    d += dd;
		    s += ds;
		}
	    } else {
		// Trust libc
		const int dd = dst->bytesPerLine();
		const int ds = src->bytesPerLine();
		while ( sh-- ) {
		    memcpy( d, s, sw );
		    d += dd;
		    s += ds;
		}
	    }
	}
	break;
      case 32:
	{
	    QRgb* d = (QRgb*)dst->scanLine(dy) + dx;
	    QRgb* s = (QRgb*)src->scanLine(sy) + sx;
	    if ( sw < 64 ) {
		// Trust ourselves
		const int dd = dst->width() - sw;
		const int ds = src->width() - sw;
		while ( sh-- ) {
		    for ( int t=sw; t--; )
			*d++ = *s++;
		    d += dd;
		    s += ds;
		}
	    } else {
		// Trust libc
		const int dd = dst->width();
		const int ds = src->width();
		const int b = sw*sizeof(QRgb);
		while ( sh-- ) {
		    memcpy( d, s, b );
		    d += dd;
		    s += ds;
		}
	    }
	}
	break;
    }
}


/*!  Returns TRUE if this image and \a i have the same contents, and
  FALSE if they differ.  This can be slow.  Of course, this function
  returns quickly if e.g. the two images' widths are different.

  \sa operator=()
*/

bool QImage::operator==( const QImage & i ) const
{
    // same object, or shared?
    if ( i.data == data )
	return TRUE;
    // obviously different stuff?
    if ( i.data->h != data->h ||
	 i.data->w != data->w )
	return FALSE;
    // that was the fast bit...
    QImage i1 = convertDepth( 32 );
    QImage i2 = i.convertDepth( 32 );
    int l;
    for( l=0; l < data->h; l++ )
	if ( memcmp( i1.scanLine( l ), i2.scanLine( l ), 4*data->w ) )
	    return FALSE;
    return TRUE;
}


/*!  Returns TRUE if this image and \a i have different contents, and
  FALSE if they they have the same.  This can be slow.  Of course,
  this function returns quickly if e.g. the two images' widths are
  different.

  \sa operator=()
*/

bool QImage::operator!=( const QImage & i ) const
{
    return !(*this == i);
}
