/****************************************************************************
** $Id: qasyncimageio.cpp,v 1.38.2.6 1999/02/13 14:57:55 hanord Exp $
**
** Implementation of asynchronous image/movie loading classes
**
** Created : 970617
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

#include "qasyncimageio.h"
#include "qpainter.h"
#include "qlist.h"
#include "qt_gif.h"
#include <stdlib.h>


/*!
  \class QImageConsumer qasyncimageio.h
  \brief An abstraction used by QImageDecoder.

  \ingroup images

  A QImageConsumer consumes information about changes to the
  QImage maintained by a QImageDecoder.  It represents the
  a view of the image which the decoder produces.

  \sa QImageDecoder
*/

/*!
  \fn void QImageConsumer::changed(const QRect&)

  Called when the given area of the image has changed.
*/

/*!
  \fn void QImageConsumer::end()

  Called when all data of all frames has been decoded and revealed
  as changed().
*/

/*!
  \fn void QImageConsumer::frameDone()

  Called when a frame of an animated image has ended and been revealed
  as changed().  The decoder will not make
  any further changes to the image until the next call to
  QImageFormat::decode().
*/

/*!
  \fn void QImageConsumer::setLooping(int n)

  Called to indicate that the sequence of frames in the image
  should be repeated \a n times, including the sequence during
  decoding.

  <ul>
    <li> 0 = Forever
    <li> 1 = Only display frames the first time through
    <li> 2 = Repeat once after first pass through images
    <li> etc.
  </ul>

  To make the QImageDecoder
  do this just delete it and pass the information to it again
  for decoding (setLooping() will be called again of course, but
  that can be ignored), or keep copies of the
  changed areas at the ends of frames.
*/

/*!
  \fn void QImageConsumer::setFramePeriod(int milliseconds)

  Notes that the frame about to be decoded should not be displayed until
  the given number of \a milliseconds after the time that this function
  is called.  Of course, the image may not have been decoded by then, in
  which case the frame should not be displayed until it is complete.
  A value of -1 (the assumed default) indicates that the image should
  be diplayed even while it is only partially loaded.
*/

/*!
  \fn void QImageConsumer::setSize(int, int)

  This function is called as soon as the size of the image has
  been determined.
*/


/*!
  \class QImageDecoder qasyncimageio.h
  \brief Incremental image decoder for all supported image formats.

  \ingroup images

  New formats are installed by creating objects of class
  QImageFormatType, and the QMovie class can be used for using
  all installed incremental image formats; QImageDecoder is only
  useful for creating new ways of feeding data to an QImageConsumer.
*/

static const int max_header = 32;





// See qt_gif.h for important information regarding this option
#if defined(QT_BUILTIN_GIF_READER) && QT_BUILTIN_GIF_READER == 1
class Q_EXPORT QGIFFormat : public QImageFormat {
public:
    QGIFFormat();
    virtual ~QGIFFormat();

    int decode(QImage& img, QImageConsumer* consumer,
	    const uchar* buffer, int length);

private:
    void fillRect(QImage&, int x, int y, int w, int h, uchar col);

    // GIF specific stuff
    QRgb* globalcmap;
    QImage backingstore;
    unsigned char hold[16];
    bool gif89;
    int count;
    int ccount;
    int expectcount;
    enum State {
	Header,
	LogicalScreenDescriptor,
	GlobalColorMap,
	LocalColorMap,
	Introducer,
	ImageDescriptor,
	TableImageLZWSize,
	ImageDataBlockSize,
	ImageDataBlock,
	ExtensionLabel,
	GraphicControlExtension,
	ApplicationExtension,
	NetscapeExtensionBlockSize,
	NetscapeExtensionBlock,
	SkipBlockSize,
	SkipBlock,
	Done,
	Error
    } state;
    int gncols;
    int ncols;
    int lzwsize;
    bool lcmap;
    int swidth, sheight;
    int left, top, right, bottom;
    enum Disposal { NoDisposal, DoNotChange, RestoreBackground, RestoreImage };
    Disposal disposal;
    bool disposed;
    int trans;
    bool preserve_trans;
    bool gcmap;
    int bgcol;
    int interlace;
    int accum;
    int bitcount;

    enum { max_lzw_bits=12 }; // (poor-compiler's static const int)

    int code_size, clear_code, end_code, max_code_size, max_code;
    int firstcode, oldcode, incode;
    short table[2][1<< max_lzw_bits];
    short stack[(1<<(max_lzw_bits))*2];
    short *sp;
    bool needfirst;
    int x, y;
    int frame;
    bool out_of_bounds;
    bool digress;
    void nextY(QImage& img, QImageConsumer* consumer);
    void disposePrevious( QImage& img, QImageConsumer* consumer );
};

class Q_EXPORT QGIFFormatType : public QImageFormatType
{
    QImageFormat* decoderFor(const uchar* buffer, int length);
    const char* formatName() const;
};

#endif


struct QImageDecoderPrivate {
    QImageDecoderPrivate()
    {
	count = 0;
    }

    static void cleanup();

    static void ensureFactories()
    {
	if ( !factories ) {
	    factories = new QListT<QImageFormatType>;
// See qt_gif.h for important information regarding this option
#if defined(QT_BUILTIN_GIF_READER) && QT_BUILTIN_GIF_READER == 1
	    gif_decoder_factory = new QGIFFormatType;
#endif
	    qAddPostRoutine( cleanup );
	}
    }

    static QList<QImageFormatType> * factories;

// See qt_gif.h for important information regarding this option
#if defined(QT_BUILTIN_GIF_READER) && QT_BUILTIN_GIF_READER == 1
    static QGIFFormatType * gif_decoder_factory;
#endif

    uchar header[max_header];
    int count;
};

QList<QImageFormatType> * QImageDecoderPrivate::factories = 0;
// See qt_gif.h for important information regarding this option
#if defined(QT_BUILTIN_GIF_READER) && QT_BUILTIN_GIF_READER == 1
QGIFFormatType * QImageDecoderPrivate::gif_decoder_factory = 0;
#endif


void QImageDecoderPrivate::cleanup()
{
    delete factories;
    factories = 0;
// See qt_gif.h for important information regarding this option
#if defined(QT_BUILTIN_GIF_READER) && QT_BUILTIN_GIF_READER == 1
    delete gif_decoder_factory;
    gif_decoder_factory = 0;
#endif
}


/*!
  Constructs a QImageDecoder which will send change information to
  a given QImageConsumer.
*/
QImageDecoder::QImageDecoder(QImageConsumer* c)
{
    d = new QImageDecoderPrivate;
    CHECK_PTR(d);
    consumer = c;
    actual_decoder = 0;
}

/*!
  Destroys a QImageDecoder.  The image it built is destroyed.  The decoder
  built by the factory for the file format is destroyed. The consumer
  for which it decoded the image is \e not destroyed.
*/
QImageDecoder::~QImageDecoder()
{
    delete d;
    delete actual_decoder;
}

/*!
  \fn const QImage& QImageDecoder::image()

  Returns the image currently being decoded.
*/

/*!
  Call this function to decode some data into image changes.  The data
  will be decoded, sending change information to the QImageConsumer of
  this QImageDecoder, until one of the change functions of the consumer
  returns FALSE.

  Returns the number of bytes consumed, 0 if consumption is complete,
  and -1 if decoding fails dur to invalid data.
*/
int QImageDecoder::decode(const uchar* buffer, int length)
{
    if (actual_decoder) {
	return actual_decoder->decode(img, consumer, buffer, length);
    } else {
	int consumed=0;
	while (consumed < length && d->count < max_header) {
	    d->header[d->count++] = buffer[consumed++];
	}
	
	QImageDecoderPrivate::ensureFactories();

	for (QImageFormatType* f = QImageDecoderPrivate::factories->first();
	    f && !actual_decoder;
	    f = QImageDecoderPrivate::factories->next())
	{
	    actual_decoder = f->decoderFor(d->header, d->count);
	}

	if (actual_decoder) {
	    uchar* b = d->header;
	    int more = 1;
	    while (d->count > 0)  {
		more = actual_decoder->decode(img, consumer, b, d->count);
		if ( more <= 0 ) break;
		d->count -= more;
		b += more;
	    }
	    if (more <= 0) {
		// Decoder must have failed.  Input not valid.  We assume
		// consumer has been notified.
		delete actual_decoder;
		actual_decoder = 0;
		return more;
	    }
	}

	return consumed;
    }
}

/*!
  Call this function to find the name of the format of the given header.
  The returned string is statically allocated.

  Returns 0 if the format is not recognized.
*/
const char* QImageDecoder::formatName(const uchar* buffer, int length)
{
    QImageDecoderPrivate::ensureFactories();

    const char* name = 0;
    for (QImageFormatType* f = QImageDecoderPrivate::factories->first();
	f && !name;
	f = QImageDecoderPrivate::factories->next())
    {
	QImageFormat *decoder = f->decoderFor(buffer, length);
	if (decoder) {
	    name = f->formatName();
	    delete decoder;
	}
    }
    return name;
}

/*!
  Returns a sorted list of formats for which asynchronous loading is supported.
*/
QStrList QImageDecoder::inputFormats()
{
    QImageDecoderPrivate::ensureFactories();

    QStrList result;

    for (QImageFormatType* f = QImageDecoderPrivate::factories->first();
	 f;
	 f = QImageDecoderPrivate::factories->next())
    {
	if ( !result.contains(  f->formatName() ) ) {
	    result.inSort(  f->formatName() );
	}
    }

    return result;
}

/*!
  Registers a new QImageFormatType.  This is not needed in
  application code as factories call this themselves.
*/
void QImageDecoder::registerDecoderFactory(QImageFormatType* f)
{
    QImageDecoderPrivate::ensureFactories();

    QImageDecoderPrivate::factories->insert(0,f);
}

/*!
  Unregisters a new QImageFormatType.  This is not needed in
  application code as factories call this themselves.
*/
void QImageDecoder::unregisterDecoderFactory(QImageFormatType* f)
{
    if ( !QImageDecoderPrivate::factories )
	return;

    QImageDecoderPrivate::factories->remove(f);
}

/*!
  \class QImageFormat qasyncimageio.h
  \brief Incremental image decoder for a specific image format.

  \ingroup images

  By making a derived classes of QImageFormatType which in turn
  creates objects that are a subclass of QImageFormat, you can add
  support for more incremental image formats, allowing such formats to
  be sources for a QMovie, or for the first frame of the image stream
  to be loaded as a QImage or QPixmap.

  Your new subclass must override the decode() function in order to
  process your new format.

  New QImageFormat objects are generated by new QImageFormatType factories.
*/

/*!
  Destructs the object.

  \internal
  More importantly, destructs derived classes.
*/
QImageFormat::~QImageFormat()
{
}

/*!
  \fn int QImageFormat::decode(QImage& img, QImageConsumer* consumer,
	    const uchar* buffer, int length)

  New subclasses must override this method.

  It should decode some or all of the bytes from \a buffer into
  \a img, calling the methods of \a consumer as the decoding proceeds to
  inform that consumer of changes to the image.
  The consumer may be 0, in which case the function should just process
  the data into \a img without telling any consumer about the changes.
  Note that the decoder must store enough state
  to be able to continue in subsequent calls to this method - this is
  the essence of the incremental image loading.

  The function should return without processing all the data if it
  reaches the end of a frame in the input.

  The function must return the number of bytes it has processed.
*/

/*!
  \class QImageFormatType qasyncimageio.h
  \brief Factory that makes QImageFormat objects.

  \ingroup images

  While the QImageIO class allows for \e complete loading of images,
  QImageFormatType allows for \e incremental loading of images.

  New image file formats are installed by creating objects of derived
  classes of QImageFormatType.  They must implement decoderFor()
  and formatName().

  QImageFormatType is a very simple class.  Its only task is to
  recognize image data in some format and make a new object, subclassed
  from QImageFormat, which can decode that format.

  The factories for formats built into Qt
  are automatically defined before any other factory is initialized.
  If two factories would recognize an image format, the factory created
  last will override the earlier one, thus you can override current
  and future built-in formats.
*/

/*!
  \fn virtual QImageFormat* QImageFormatType::decoderFor(const
	    uchar* buffer, int length)

  Returns a decoder for decoding an image which starts with the give bytes.
  This function should only return a decoder if it is definate that the
  decoder applies to data with the given header.  Returns 0 if there is
  insufficient data in the header to make a positive identification,
  or if the data is not recognized.
*/

/*!
  \fn virtual const char* QImageFormatType::formatName() const

  Returns the name of the format supported by decoders from this factory.
  The string is statically allocated.
*/

/*!
  Creates a factory.  It automatically registers itself with QImageDecoder.
*/
QImageFormatType::QImageFormatType()
{
    QImageDecoder::registerDecoderFactory(this);
}

/*!
  Destroys a factory.  It automatically unregisters itself from QImageDecoder.
*/
QImageFormatType::~QImageFormatType()
{
    QImageDecoder::unregisterDecoderFactory(this);
}


bool qt_builtin_gif_reader()
{
#if defined(QT_BUILTIN_GIF_READER)
    return QT_BUILTIN_GIF_READER == 1;
#else
    return 0;
#endif
}

// See qt_gif.h for important information regarding this option
#if defined(QT_BUILTIN_GIF_READER) && QT_BUILTIN_GIF_READER == 1

/* -- NOTDOC
  \class QGIFFormat qasyncimageio.h
  \brief Incremental image decoder for GIF image format.

  \ingroup images

  This subclass of QImageFormat decodes GIF format images,
  including animated GIFs.  Internally in
*/

/*!
  Constructs a QGIFFormat.
*/
QGIFFormat::QGIFFormat()
{
    globalcmap = 0;
    disposal = NoDisposal;
    out_of_bounds = FALSE;
    disposed = TRUE;
    frame = -1;
    state = Header;
    count = 0;
    lcmap = FALSE;
}

/*!
  Destructs a QGIFFormat.
*/
QGIFFormat::~QGIFFormat()
{
    delete[] globalcmap;
}


/* -- NOTDOC
  \class QGIFFormatType qasyncimageio.h
  \brief Incremental image decoder for GIF image format.

  \ingroup images

  This subclass of QImageFormatType recognizes GIF
  format images, creating a QGIFFormat when required.  An instance
  of this class is created automatically before any other factories,
  so you should have no need for such objects.
*/

QImageFormat* QGIFFormatType::decoderFor(
    const uchar* buffer, int length)
{
    if (length < 6) return 0;
    if (buffer[0]=='G'
     && buffer[1]=='I'
     && buffer[2]=='F'
     && buffer[3]=='8'
     && (buffer[4]=='9' || buffer[4]=='7')
     && buffer[5]=='a')
        return new QGIFFormat;
    return 0;
}

const char* QGIFFormatType::formatName() const
{
    return "GIF";
}


void QGIFFormat::disposePrevious( QImage& img, QImageConsumer* consumer )
{
    if ( out_of_bounds ) // flush anything that survived
	consumer->changed(QRect(0,0,swidth,sheight));

    // Handle disposal of previous image before processing next one

    if ( disposed ) return;

    int l = QMIN(swidth-1,left);
    int r = QMIN(swidth-1,right);
    int t = QMIN(sheight-1,top);
    int b = QMIN(sheight-1,bottom);

    switch (disposal) {
      case NoDisposal:
	break;
      case DoNotChange:
	break;
      case RestoreBackground:
	preserve_trans = FALSE;
	if (trans>=0) {
	    // Easy:  we use the transparent colour
	    fillRect(img, l, t, r-l+1, b-t+1, trans);
	} else if (bgcol>=0) {
	    // Easy:  we use the bgcol given
	    fillRect(img, l, t, r-l+1, b-t+1, bgcol);
	} else {
	    // Impossible:  We don't know of a bgcol - use pixel 0
	    uchar** line = img.jumpTable();
	    fillRect(img, l, t, r-l+1, b-t+1, line[0][0]);
	}
	if (consumer)
	    consumer->changed(QRect(l, t, r-l+1, b-t+1));
	break;
      case RestoreImage: {
	uchar** line = img.jumpTable();
	preserve_trans = FALSE;
	for (int ln=t; ln<=b; ln++) {
	    memcpy(line[ln]+l,
		backingstore.scanLine(ln-t),
		r-l+1);
	}
	consumer->changed(QRect(l, t, r-l+1, b-t+1));
      }
    }
    disposal = NoDisposal; // Until an extension says otherwise.

    disposed = TRUE;
}

/*!
  This function decodes some data into image changes.

  Returns the number of bytes consumed.
*/
int QGIFFormat::decode(QImage& img, QImageConsumer* consumer,
	const uchar* buffer, int length)
{
    // We are required to state that
    //    "The Graphics Interchange Format(c) is the Copyright property of
    //    CompuServe Incorporated. GIF(sm) is a Service Mark property of
    //    CompuServe Incorporated."

    #define LM(l, m) (((m)<<8)|l)
    digress = FALSE;
    int initial = length;
    uchar** line = img.jumpTable();
    while (!digress && length) {
	length--;
	unsigned char ch=*buffer++;
	switch (state) {
	  case Header:
	    hold[count++]=ch;
	    if (count==6) {
		// Header
		gif89=(hold[3]!='8' || hold[4]!='7');
		state=LogicalScreenDescriptor;
		count=0;
	    }
	    break;
	  case LogicalScreenDescriptor:
	    hold[count++]=ch;
	    if (count==7) {
		// Logical Screen Descriptor
		swidth=LM(hold[0], hold[1]);
		sheight=LM(hold[2], hold[3]);
		gcmap=!!(hold[4]&0x80);
		//UNUSED: bpchan=(((hold[4]&0x70)>>3)+1);
		//UNUSED: gcmsortflag=!!(hold[4]&0x08);
		gncols=2<<(hold[4]&0x7);
		bgcol=(gcmap && hold[5]) ? hold[5] : -1;
		//aspect=hold[6] ? double(hold[6]+15)/64.0 : 1.0;

		trans = -1;
		preserve_trans = FALSE;
		count=0;
		ncols=gncols;
		if (gcmap) {
		    ccount=0;
		    state=GlobalColorMap;
		    globalcmap = new QRgb[gncols];
		} else {
		    state=ImageDescriptor;
		}
	    }
	    break;
	  case GlobalColorMap: case LocalColorMap:
	    hold[count++]=ch;
	    if (count==3) {
		QRgb rgb = (ccount==trans ? 0 : 0xff000000)
		    | qRgb(hold[0], hold[1], hold[2]);
		if ( state == LocalColorMap ) {
		    img.setColor(ccount, rgb);
		} else {
		    globalcmap[ccount] = rgb;
		}
		if (++ccount >= ncols) {
		    if ( state == LocalColorMap )
			state=TableImageLZWSize;
		    else
			state=Introducer;
		}
		count=0;
	    }
	    break;
	  case Introducer:
	    hold[count++]=ch;
	    switch (ch) {
	      case ',':
		state=ImageDescriptor;
		break;
	      case '!':
		state=ExtensionLabel;
		break;
	      case ';':
		if (consumer) {
		    if ( out_of_bounds ) // flush anything that survived
			consumer->changed(QRect(0,0,swidth,sheight));
		    consumer->end();
		}
		state=Done;
		break;
	      default:
		digress=TRUE;
		// Unexpected Introducer - ignore block
		state=Error;
	    }
	    break;
	  case ImageDescriptor:
	    hold[count++]=ch;
	    if (count==10) {
		int newleft=LM(hold[1], hold[2]);
		int newtop=LM(hold[3], hold[4]);
		int width=LM(hold[5], hold[6]);
		int height=LM(hold[7], hold[8]);

		if ( swidth <= 0 )
		    swidth = newleft + width;
		if ( sheight <= 0 )
		    sheight = newtop + height;

		if (img.isNull()) {
		    img.create(swidth, sheight, 8, gcmap ? gncols : 256);
		    if (consumer) consumer->setSize(swidth, sheight);
		}
		img.setAlphaBuffer(trans >= 0);
		line = img.jumpTable();

		disposePrevious( img, consumer );
		disposed = FALSE;

		left = newleft;
		top = newtop;

		// Sanity check frame size - must fit on "screen".
		if (left >= swidth) left=swidth-1;
		if (top >= sheight) top=sheight-1;
		if (left+width >= swidth) {
		    if ( width <= swidth )
			left=swidth-width;
		    else
			width=swidth-left;
		}
		if (top+height >= sheight) {
		    if ( height <= sheight )
			top=sheight-height;
		    else
			height=sheight-top;
		}

		right=left+width-1;
		bottom=top+height-1;
		lcmap=!!(hold[9]&0x80);
		interlace=!!(hold[9]&0x40);
		//bool lcmsortflag=!!(hold[9]&0x20);
		int lncols=lcmap ? (2<<(hold[9]&0x7)) : 0;
		if (lncols) {
		    if (lncols > ncols) img.setNumColors(lncols);
		    ncols = lncols;
		} else {
		    ncols = gncols;
		}
		frame++;
		if ( frame == 0 ) {
		    if ( left || top || width!=swidth || height!=sheight ) {
			// Not full-size image - erase with bg or transparent
			if ( bgcol>=0 ) {
			    fillRect(img, 0, 0, swidth, sheight, bgcol);
			    if (consumer) consumer->changed(QRect(0,0,swidth,sheight));
			} else if ( trans > 0 ) {
			    fillRect(img, 0, 0, swidth, sheight, trans);
			    if (consumer) consumer->changed(QRect(0,0,swidth,sheight));
			}
		    }
		}

		if ( disposal == RestoreImage ) {
		    int l = QMIN(swidth-1,left);
		    int r = QMIN(swidth-1,right);
		    int t = QMIN(sheight-1,top);
		    int b = QMIN(sheight-1,bottom);
		    int w = r-l+1;
		    int h = b-t+1;

		    if (backingstore.width() < w
			|| backingstore.height() < h) {
			// We just use the backing store as a byte array
			backingstore.create( QMAX(backingstore.width(),
						  w),
					     QMAX(backingstore.height(),
						  h),
					     8,1);
		    }
		    for (int ln=0; ln<h; ln++) {
			memcpy(backingstore.scanLine(ln),
			       line[t+ln]+l, w);
		    }
		}

		count=0;
		if (lcmap) {
		    ccount=0;
		    state=LocalColorMap;
		} else {
		    if (gcmap) {
			memcpy(img.colorTable(), globalcmap,
			    ncols * sizeof(QRgb));
		    }
		    state=TableImageLZWSize;
		}
		x = left;
		y = top;
		accum = 0;
		bitcount = 0;
		sp = stack;
		needfirst = FALSE;
		out_of_bounds = FALSE;
	    }
	    break;
	  case TableImageLZWSize: {
	    lzwsize=ch;
	    if ( lzwsize > max_lzw_bits ) {
		state=Error;
	    } else {
		code_size=lzwsize+1;
		clear_code=1<<lzwsize;
		end_code=clear_code+1;
		max_code_size=2*clear_code;
		max_code=clear_code+2;
		int i;
		for (i=0; i<clear_code && i<(1<<max_lzw_bits); i++) {
		    table[0][i]=0;
		    table[1][i]=i;
		}
		for (i=clear_code; i<(1<<max_lzw_bits); i++) {
		    table[0][i]=table[1][i]=0;
		}
		state=ImageDataBlockSize;
	    }
	    count=0;
	    break;
	  } case ImageDataBlockSize:
	    expectcount=ch;
	    if (expectcount) {
		state=ImageDataBlock;
	    } else {
		if (consumer) {
		    consumer->frameDone();
		    digress = TRUE;
		}

		state=Introducer;
	    }
	    break;
	  case ImageDataBlock:
	    count++;
	    accum|=(ch<<bitcount);
	    bitcount+=8;
	    while (bitcount>=code_size && state==ImageDataBlock) {
		int code=accum&((1<<code_size)-1);
		bitcount-=code_size;
		accum>>=code_size;

		if (code==clear_code) {
		    if (!needfirst) {
			int i;
			code_size=lzwsize+1;
			max_code_size=2*clear_code;
			max_code=clear_code+2;
			for (i=0; i<clear_code; i++) {
			    table[0][i]=0;
			    table[1][i]=i;
			}
			for (i=clear_code; i<(1<<max_lzw_bits); i++) {
			    table[0][i]=table[1][i]=0;
			}
		    }
		    needfirst=TRUE;
		} else if (code==end_code) {
		    bitcount = -32768;
		    // Left the block end arrive
		} else {
		    if (needfirst) {
			firstcode=oldcode=code;
			if (!out_of_bounds && !(preserve_trans && firstcode==trans))
			    line[y][x] = firstcode;
			x++;
			if (x>=swidth) out_of_bounds = TRUE;
			needfirst=FALSE;
			if (x>right) {
			    x=left;
			    if (out_of_bounds)
				out_of_bounds = left>=swidth || y>=sheight;
			    nextY(img,consumer);
			}
		    } else {
			incode=code;
			if (code>=max_code) {
			    *sp++=firstcode;
			    code=oldcode;
			}
			while (code>=clear_code) {
			    *sp++=table[1][code];
			    if (code==table[0][code]) {
				state=Error;
				break;
			    }
			    if (sp-stack>=(1<<(max_lzw_bits))*2) {
				state=Error;
				break;
			    }
			    code=table[0][code];
			}
			*sp++=firstcode=table[1][code];
			code=max_code;
			if (code<(1<<max_lzw_bits)) {
			    table[0][code]=oldcode;
			    table[1][code]=firstcode;
			    max_code++;
			    if ((max_code>=max_code_size)
			     && (max_code_size<(1<<max_lzw_bits)))
			    {
				max_code_size*=2;
				code_size++;
			    }
			}
			oldcode=incode;
			while (sp>stack) {
			    --sp;
			    if (!out_of_bounds && !(preserve_trans && *sp==trans))
				line[y][x] = *sp;
			    x++;
			    if (x>=swidth) out_of_bounds = TRUE;
			    if (x>right) {
				x=left;
				if (out_of_bounds)
				    out_of_bounds = left>=swidth || y>=sheight;
				nextY(img,consumer);
			    }
			}
		    }
		}
	    }
	    if (count==expectcount) {
		count=0;
		state=ImageDataBlockSize;
	    }
	    break;
	  case ExtensionLabel:
	    switch (ch) {
	     case 0xf9:
		state=GraphicControlExtension;
		break;
	     case 0xff:
		state=ApplicationExtension;
		break;
/////////////////////////////////////////// Ignored at this time //////
//           case 0xfe:
//                state=CommentExtension;
//                break;
//           case 0x01:
//                break;
///////////////////////////////////////////////////////////////////////
	     default:
		state=SkipBlockSize;
	    }
	    count=0;
	    break;
	  case ApplicationExtension:
	    if (count<11) hold[count]=ch;
	    count++;
	    if (count==hold[0]+1) {
		if (strncmp((char*)(hold+1), "NETSCAPE", 8)==0) {
		    // Looping extension
		    state=NetscapeExtensionBlockSize;
		} else {
		    state=SkipBlockSize;
		}
		count=0;
	    }
	    break;
	  case NetscapeExtensionBlockSize:
	    expectcount=ch;
	    count=0;
	    if (expectcount) state=NetscapeExtensionBlock;
	    else state=Introducer;
	    break;
	  case NetscapeExtensionBlock:
	    if (count<3) hold[count]=ch;
	    count++;
	    if (count==expectcount) {
		int loop = hold[0]+hold[1]*256;

		// Why if the extension here, if it is supposed to only
		// play through once?  We assume that the creator meant
		// 0, which is infinite.
		if (loop == 1) loop = 0;

		if (consumer) consumer->setLooping(loop);
		state=SkipBlockSize; // Ignore further blocks
	    }
	    break;
	  case GraphicControlExtension:
	    if (count<5) hold[count]=ch;
	    count++;
	    if (count==hold[0]+1) {
		disposePrevious( img, consumer );
		disposal=Disposal((hold[1]>>2)&0x7);
		//UNUSED: waitforuser=!!((hold[1]>>1)&0x1);
		int delay=count>3 ? LM(hold[2], hold[3]) : 0;
		bool havetrans=hold[1]&0x1;
		int newtrans=havetrans ? hold[4] : -1;
		if (newtrans >= ncols) {
		    // Ignore invalid transparency.
		    newtrans=-1;
		}
		if (newtrans >= 0 && frame>=0)
		    preserve_trans = TRUE;
		if (newtrans != trans) {
		    // Unset old transparency
		    if (trans >= 0) {
			img.setColor(trans, 0xff000000|img.color(trans));
			if ( newtrans >= 0 ) {
			    // Changed transparency.  Groan.
			    // Change all occurrence of old to new.
			    uchar** line = img.jumpTable();
			    for (int j=0; j<sheight; j++) {
				for (int i=0; i<swidth; i++) {
				    if (line[j][i]==trans) {
					line[j][i]=newtrans;
				    }
				}
			    }
			}
		    }
		    trans = newtrans;
		    if (trans >= 0) {
			if (globalcmap)
			    globalcmap[trans]&=0x00ffffff;
		    }
		}
		if (consumer) consumer->setFramePeriod(delay*10);
		count=0;
		state=SkipBlockSize;
	    }
	    break;
	  case SkipBlockSize:
	    expectcount=ch;
	    count=0;
	    if (expectcount) state=SkipBlock;
	    else state=Introducer;
	    break;
	  case SkipBlock:
	    count++;
	    if (count==expectcount) state=SkipBlockSize;
	    break;
	  case Done:
	    length++; // Unget
	    digress=TRUE;
	    state=Error; // More calls to this is an error
	    break;
	  case Error:
	    return -1; // Called again after done.
	}
    }
    return initial-length;
}

void QGIFFormat::fillRect(QImage& img, int col, int row, int w, int h, uchar color)
{
    if (w>0) {
	uchar** line = img.jumpTable() + row;
	for (int j=0; j<h; j++) {
	    memset(line[j]+col, color, w);
	}
    }
}

void QGIFFormat::nextY(QImage& img, QImageConsumer* consumer)
{
    int my;
    switch (interlace) {
      case 0:
	// Non-interlaced
	if (consumer && !out_of_bounds)
	    consumer->changed(QRect(left, y, right-left+1, 1));
	y++;
	break;
      case 1:
	{
	    int i;
	    my = QMIN(7, bottom-y);
	    for (i=1; i<=my; i++)
	        memcpy(img.scanLine(y+i), img.scanLine(y), img.width());
	    if (consumer && !out_of_bounds)
	        consumer->changed(QRect(left, y, right-left+1, my+1));
	    y+=8;
	    if (y>bottom) { interlace++; y=4; }
	} break;
      case 2:
	{
	    int i;
	    my = QMIN(3, bottom-y);
	    for (i=1; i<=my; i++)
	        memcpy(img.scanLine(y+i), img.scanLine(y), img.width());
    	    if (consumer && !out_of_bounds)
	        consumer->changed(QRect(left, y, right-left+1, my+1));
	    y+=8;
	    if (y>bottom) { interlace++; y=2; }
	} break;
      case 3:
	{
	    my = QMIN(1, bottom-y);
	    for (int i=1; i<=my; i++)
	        memcpy(img.scanLine(y+i), img.scanLine(y), img.width());
	    if (consumer && !out_of_bounds)
	        consumer->changed(QRect(left, y, right-left+1, my+1));
	    y+=4;
	    if (y>bottom) { interlace++; y=1; }
	} break;
      case 4:
	if (consumer && !out_of_bounds)
	    consumer->changed(QRect(left, y, right-left+1, 1));
	y+=2;
    }

    // Consume bogus extra lines
    if (y >= sheight) out_of_bounds=TRUE; //y=bottom;
}

#endif // QT_BUILTIN_GIF_READER
