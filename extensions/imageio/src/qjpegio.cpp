/****************************************************************************
** $Id: qjpegio.cpp,v 1.6.2.3 1998/10/29 14:58:16 warwick Exp $
**
** Implementation of JPEG QImage IOHandler
**
** Created : 970521
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

#include <stdio.h> // jpeglib needs this to be pre-included
#include <setjmp.h>

extern "C" {
#define XMD_H // Shut JPEGlib up.
#include <jpeglib.h>
#ifdef const
#undef const // Remove crazy C hackery in jconfig.h
#endif
}
#include <qimage.h>
#include <qiodevice.h>

struct my_error_mgr : public jpeg_error_mgr {
    jmp_buf setjmp_buffer;
};

static
void my_error_exit (j_common_ptr cinfo)
{
    my_error_mgr* myerr = (my_error_mgr*) cinfo->err;
    char buffer[JMSG_LENGTH_MAX];
    (*cinfo->err->format_message)(cinfo, buffer);
    warning(buffer);
    longjmp(myerr->setjmp_buffer, 1);
}


static const int max_buf = 4096;

struct my_jpeg_source_mgr : public jpeg_source_mgr {
    // Nothing dynamic - cannot rely on destruction over longjump
    QImageIO* iio;
    JOCTET buffer[max_buf];

public:
    my_jpeg_source_mgr(QImageIO* iio)
    {
	jpeg_source_mgr::init_source = init_source;
	jpeg_source_mgr::fill_input_buffer = fill_input_buffer;
	jpeg_source_mgr::skip_input_data = skip_input_data;
	jpeg_source_mgr::resync_to_restart = jpeg_resync_to_restart;
	jpeg_source_mgr::term_source = term_source;
	this->iio = iio;
	bytes_in_buffer = 0;
	next_input_byte = buffer;
    }

    static void init_source(j_decompress_ptr)
    {
    }

    static boolean fill_input_buffer(j_decompress_ptr cinfo)
    {
	int num_read;
	my_jpeg_source_mgr* src = (my_jpeg_source_mgr*)cinfo->src;
	QIODevice* dev = src->iio->ioDevice();
	src->next_input_byte = src->buffer;
	num_read = dev->readBlock((char*)src->buffer, max_buf);
	if ( num_read <= 0 ) {
	    // Insert a fake EOI marker - as per jpeglib recommendation
	    src->buffer[0] = (JOCTET) 0xFF;
	    src->buffer[1] = (JOCTET) JPEG_EOI;
	    src->bytes_in_buffer = 2;
	}
	else
	    src->bytes_in_buffer = num_read;
	return TRUE;
    }

    static void skip_input_data(j_decompress_ptr cinfo, long num_bytes)
    {
	my_jpeg_source_mgr* src = (my_jpeg_source_mgr*)cinfo->src;

	// `dumb' implementation from jpeglib

	/* Just a dumb implementation for now.  Could use fseek() except
	 * it doesn't work on pipes.  Not clear that being smart is worth
	 * any trouble anyway --- large skips are infrequent.
	 */
	if (num_bytes > 0) {
	    while (num_bytes > (long) src->bytes_in_buffer) {
	        num_bytes -= (long) src->bytes_in_buffer;
	        (void) fill_input_buffer(cinfo);
	        /* note we assume that fill_input_buffer will never return FALSE,
	         * so suspension need not be handled.
	         */
	    }
	    src->next_input_byte += (size_t) num_bytes;
	    src->bytes_in_buffer -= (size_t) num_bytes;
	}
    }

    static void term_source(j_decompress_ptr)
    {
    }
};

static
void read_jpeg_image(QImageIO* iio)
{
    QImage image;

    struct jpeg_decompress_struct cinfo;

    struct my_jpeg_source_mgr *iod_src = new my_jpeg_source_mgr(iio);
    struct my_error_mgr jerr;

    jpeg_create_decompress(&cinfo);

    cinfo.src = iod_src;

    cinfo.err = jpeg_std_error(&jerr);
    jerr.error_exit = my_error_exit;

    if (!setjmp(jerr.setjmp_buffer)) {
	(void) jpeg_read_header(&cinfo, TRUE);

	(void) jpeg_start_decompress(&cinfo);

	if ( cinfo.output_components == 3 || cinfo.output_components == 4) {
	    image.create( cinfo.output_width, cinfo.output_height, 32 );
	} else if ( cinfo.output_components == 1 ) {
	    image.create( cinfo.output_width, cinfo.output_height, 8, 256 );
	    for (int i=0; i<256; i++)
		image.setColor(i, qRgb(i,i,i));
	} else {
	    // Unsupported format
	}

	if (!image.isNull()) {
	    uchar** lines = image.jumpTable();
	    while (cinfo.output_scanline < cinfo.output_height)
		(void) jpeg_read_scanlines(&cinfo,
			    lines + cinfo.output_scanline,
			    cinfo.output_height);
	    (void) jpeg_finish_decompress(&cinfo);
	}

	if ( cinfo.output_components == 3 ) {
	    // Expand 24->32 bpp.
	    for (uint j=0; j<cinfo.output_height; j++) {
		uchar *in = image.scanLine(j) + cinfo.output_width * 3;
		QRgb *out = (QRgb*)image.scanLine(j);

		for (uint i=cinfo.output_width; i--; ) {
		    in-=3;
		    out[i] = qRgb(in[0], in[1], in[2]);
		}
	    }
	}
	iio->setImage(image);
	iio->setStatus(0);
    }

    jpeg_destroy_decompress(&cinfo);
    delete iod_src;
}



struct my_jpeg_destination_mgr : public jpeg_destination_mgr {
    // Nothing dynamic - cannot rely on destruction over longjump
    QImageIO* iio;
    JOCTET buffer[max_buf];

public:
    my_jpeg_destination_mgr(QImageIO* iio)
    {
	jpeg_destination_mgr::init_destination = init_destination;
	jpeg_destination_mgr::empty_output_buffer = empty_output_buffer;
	jpeg_destination_mgr::term_destination = term_destination;
	this->iio = iio;
	next_output_byte = buffer;
	free_in_buffer = max_buf;
    }

    static void init_destination(j_compress_ptr)
    {
    }

    static void exit_on_error(j_compress_ptr cinfo, QIODevice* dev)
    {
	if (dev->status() == IO_Ok) {
	    return;
        } else {
	    // cinfo->err->msg_code = JERR_FILE_WRITE; 
	    (*cinfo->err->error_exit)((j_common_ptr)cinfo);
	}
    }

    static boolean empty_output_buffer(j_compress_ptr cinfo)
    {
	my_jpeg_destination_mgr* dest = (my_jpeg_destination_mgr*)cinfo->dest;
	QIODevice* dev = dest->iio->ioDevice();

	if ( dev->writeBlock( (char*)dest->buffer, max_buf ) != max_buf )
	    exit_on_error(cinfo, dev);

	dest->next_output_byte = dest->buffer;
	dest->free_in_buffer = max_buf;

	return TRUE;
    }

    static void term_destination(j_compress_ptr cinfo)
    {
	my_jpeg_destination_mgr* dest = (my_jpeg_destination_mgr*)cinfo->dest;
	QIODevice* dev = dest->iio->ioDevice();
	int n = max_buf - dest->free_in_buffer; 

	if ( dev->writeBlock( (char*)dest->buffer, n ) != n )
	    exit_on_error(cinfo, dev);

	dev->flush();

	exit_on_error(cinfo, dev);
    }
};



int qt_jpeg_quality = 75; // Global (no provision for parameters)

static
void write_jpeg_image(QImageIO* iio)
{
    QImage image = iio->image();

    struct jpeg_compress_struct cinfo;
    JSAMPROW row_pointer[1];
    row_pointer[0] = 0;

    struct my_jpeg_destination_mgr *iod_dest = new my_jpeg_destination_mgr(iio);
    struct my_error_mgr jerr;

    cinfo.err = jpeg_std_error(&jerr);

    jerr.error_exit = my_error_exit;

    if (!setjmp(jerr.setjmp_buffer)) {
	jpeg_create_compress(&cinfo);

	cinfo.dest = iod_dest;

	cinfo.image_width = image.width();
	cinfo.image_height = image.height();

	QRgb* cmap=0;
	bool gray=FALSE;
	switch ( image.depth() ) {
	  case 1:
	  case 8:
	    cmap = image.colorTable();
	    gray = TRUE;
	    int i;
	    for (i=image.numColors(); gray && i--; ) {
		gray &=
		       qRed(cmap[i]) == qGreen(cmap[i])
		    && qRed(cmap[i]) == qBlue(cmap[i]);
	    }
	    cinfo.input_components = gray ? 1 : 3;
	    cinfo.in_color_space = gray ? JCS_GRAYSCALE : JCS_RGB;
	    break;
	  case 32:
	    cinfo.input_components = 3;
	    cinfo.in_color_space = JCS_RGB;
	}

	jpeg_set_defaults(&cinfo);
	jpeg_set_quality(&cinfo, qt_jpeg_quality, TRUE /* limit to baseline-JPEG values */);

	jpeg_start_compress(&cinfo, TRUE);

	row_pointer[0] = new uchar[cinfo.image_width*cinfo.input_components];
	int w = cinfo.image_width;
	while (cinfo.next_scanline < cinfo.image_height) {
	    uchar *row = row_pointer[0];
	    switch ( image.depth() ) {
	      case 1:
		if (gray) {
		    uchar* data = image.scanLine(cinfo.next_scanline);
		    if ( image.bitOrder() == QImage::LittleEndian ) {
			for (int i=0; i<w; i++) {
			    bool bit = !!(*(data + (i >> 3)) & (1 << (i & 7)));
			    row[i] = qRed(cmap[bit]);
			}
		    } else {
			for (int i=0; i<w; i++) {
			    bool bit = !!(*(data + (i >> 3)) & (1 << (7 -(i & 7))));
			    row[i] = qRed(cmap[bit]);
			}
		    }
		} else {
		    uchar* data = image.scanLine(cinfo.next_scanline);
		    if ( image.bitOrder() == QImage::LittleEndian ) {
			for (int i=0; i<w; i++) {
			    bool bit = !!(*(data + (i >> 3)) & (1 << (i & 7)));
			    *row++ = qRed(cmap[bit]);
			    *row++ = qGreen(cmap[bit]);
			    *row++ = qBlue(cmap[bit]);
			}
		    } else {
			for (int i=0; i<w; i++) {
			    bool bit = !!(*(data + (i >> 3)) & (1 << (7 -(i & 7))));
			    *row++ = qRed(cmap[bit]);
			    *row++ = qGreen(cmap[bit]);
			    *row++ = qBlue(cmap[bit]);
			}
		    }
		}
		break;
	      case 8:
		if (gray) {
		    uchar* pix = image.scanLine(cinfo.next_scanline);
		    for (int i=0; i<w; i++) {
			*row = qRed(cmap[*pix]);
			++row; ++pix;
		    }
		} else {
		    uchar* pix = image.scanLine(cinfo.next_scanline);
		    for (int i=0; i<w; i++) {
			*row++ = qRed(cmap[*pix]);
			*row++ = qGreen(cmap[*pix]);
			*row++ = qBlue(cmap[*pix]);
			++pix;
		    }
		}
		break;
	      case 32: {
		QRgb* rgb = (QRgb*)image.scanLine(cinfo.next_scanline);
		for (int i=0; i<w; i++) {
		    *row++ = qRed(*rgb);
		    *row++ = qGreen(*rgb);
		    *row++ = qBlue(*rgb);
		    ++rgb;
		}
	      }
	    }
	    jpeg_write_scanlines(&cinfo, row_pointer, 1);
	}

	jpeg_finish_compress(&cinfo);
	jpeg_destroy_compress(&cinfo);

	iio->setStatus(0);
    }

    delete iod_dest;
    delete row_pointer[0];
}


void qInitJpegIO()
{
    // Not much to go on - just 3 bytes: 0xFF, M_SOI, 0xFF
    // Even the third is not strictly specified as required.
    QImageIO::defineIOHandler("JPEG", "^\377\330\377", 0, read_jpeg_image, write_jpeg_image);
}
