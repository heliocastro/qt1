/****************************************************************************
** $Id: qpngio.cpp,v 1.8 1998/07/03 00:09:26 hanord Exp $
**
** Implementation of PNG QImage IOHandler
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

extern "C" {
#include <png.h>
}
#include <qimage.h>
#include <qiodevice.h>

/*
  The following PNG Test Suite (October 1996) images do not load correctly,
  with no apparent reason:

    ct0n0g04.png
    ct1n0g04.png
    ctzn0g04.png
    cm0n0g04.png
    cm7n0g04.png
    cm9n0g04.png

  All others load apparently correctly, and to the minimal QImage equivalent.

  All QImage formats output to reasonably efficient PNG equivalents.  Never
  to greyscale.
*/

static
void iod_read_fn(png_structp png_ptr, png_bytep data, png_size_t length)
{
    QImageIO* iio = (QImageIO*)png_get_io_ptr(png_ptr);
    QIODevice* in = iio->ioDevice();

    while (length) {
	int nr = in->readBlock((char*)data, length);
	if (nr <= 0) {
	    png_error(png_ptr, "Read Error");
	    return;
	}
	length -= nr;
    }
}

static
void iod_write_fn(png_structp png_ptr, png_bytep data, png_size_t length)
{
    QImageIO* iio = (QImageIO*)png_get_io_ptr(png_ptr);
    QIODevice* out = iio->ioDevice();

    uint nr = out->writeBlock((char*)data, length);
    if (nr != length) {
	png_error(png_ptr, "Write Error");
	return;
    }
}

static
void read_png_image(QImageIO* iio)
{
    png_structp png_ptr;
    png_infop info_ptr;
    png_infop end_info;
    png_bytep* row_pointers;

    png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING,0,0,0);
    if (!png_ptr) {
	iio->setStatus(-1);
	return;
    }

    info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr) {
	png_destroy_read_struct(&png_ptr, 0, 0);
	iio->setStatus(-2);
	return;
    }

    end_info = png_create_info_struct(png_ptr);
    if (!end_info) {
	png_destroy_read_struct(&png_ptr, &info_ptr, 0);
	iio->setStatus(-3);
	return;
    }

    if (setjmp(png_ptr->jmpbuf)) {
	png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
	iio->setStatus(-4);
	return;
    }

    png_set_read_fn(png_ptr, (void*)iio, iod_read_fn);


    png_read_info(png_ptr, info_ptr);
    png_set_strip_16(png_ptr);

    if (info_ptr->bit_depth < 8
    && (info_ptr->bit_depth != 1
     || info_ptr->channels != 1
     || (info_ptr->color_type != PNG_COLOR_TYPE_GRAY
      && info_ptr->color_type != PNG_COLOR_TYPE_PALETTE)))
    {
	png_set_packing(png_ptr);
    }

    if (info_ptr->valid & PNG_INFO_gAMA)
	png_set_gamma(png_ptr, 2.2, info_ptr->gamma);

    QImage image;
    bool noalpha = FALSE;

    if (info_ptr->bit_depth == 1
     && info_ptr->channels == 1
     && info_ptr->color_type == PNG_COLOR_TYPE_GRAY)
    {
	// Black & White
	png_set_invert_mono(png_ptr);
	png_read_update_info(png_ptr, info_ptr);
	image.create(info_ptr->width,info_ptr->height, 1, 2,
	    QImage::BigEndian);
	image.setColor(1, qRgb(0,0,0) );
	image.setColor(0, qRgb(255,255,255) );
    } else if (info_ptr->color_type == PNG_COLOR_TYPE_PALETTE
    && (info_ptr->valid & PNG_INFO_PLTE)
    && info_ptr->num_palette <= 256)
    {
	// 1-bit and 8-bit color
	png_read_update_info(png_ptr, info_ptr);
	image.create(
	    info_ptr->width,
	    info_ptr->height,
	    info_ptr->bit_depth,
	    info_ptr->num_palette,
	    QImage::BigEndian
	);
	for (int i=0; i<info_ptr->num_palette; i++) {
	    image.setColor(i, qRgb(
		info_ptr->palette[i].red,
		info_ptr->palette[i].green,
		info_ptr->palette[i].blue
		)
	    );
	}
	if ( info_ptr->valid & PNG_INFO_tRNS ) {
	    image.setAlphaBuffer( TRUE );
	    int i;
	    for (i=0; i<info_ptr->num_trans; i++) {
		image.setColor(i, image.color(i) | 
		    (info_ptr->trans[i] << 24));
	    }
	    while (i < info_ptr->num_palette) {
		image.setColor(i, image.color(i) | (0xff << 24));
		i++;
	    }
	}
    } else if (info_ptr->color_type == PNG_COLOR_TYPE_GRAY)
    {
	// 8-bit greyscale
	int ncols = info_ptr->bit_depth < 8 ? 1 << info_ptr->bit_depth : 256;
	int g = info_ptr->trans_values.gray;
	if ( info_ptr->bit_depth > 8 ) {
	    g >>= (info_ptr->bit_depth-8);
	}
	png_read_update_info(png_ptr, info_ptr);
	image.create(info_ptr->width,info_ptr->height,8,ncols);
	for (int i=0; i<ncols; i++) {
	    int c = i*255/(ncols-1);
	    image.setColor( i, 0xff000000 | qRgb(c,c,c) );
	}
	if ( info_ptr->valid & PNG_INFO_tRNS ) {
	    image.setAlphaBuffer( TRUE );
	    image.setColor(g, 0x00ffffff & image.color(g));
	}
    } else {
	// 32-bit
	png_set_expand(png_ptr);

	if (info_ptr->color_type == PNG_COLOR_TYPE_GRAY ||
	    info_ptr->color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
	{
	    png_set_gray_to_rgb(png_ptr);
	}

	// Only add filler if no alpha, or we can get 5 channel data.
	if (!(info_ptr->color_type & PNG_COLOR_MASK_ALPHA)
	   && !(info_ptr->valid & PNG_INFO_tRNS))
	{
	    png_set_filler(png_ptr, 0xff,
		QImage::systemByteOrder() == QImage::BigEndian ?
		    PNG_FILLER_BEFORE : PNG_FILLER_AFTER);
	    // We want 4 bytes, but it isn't an alpha channel
	    noalpha = TRUE;
	}

	png_read_update_info(png_ptr, info_ptr);
	image.create(info_ptr->width,info_ptr->height,32);
    }

    if (!noalpha && (info_ptr->channels == 4 || 
	(info_ptr->channels == 3 && (info_ptr->valid & PNG_INFO_tRNS)))) {
	image.setAlphaBuffer(TRUE);
    }

    if ( QImage::systemByteOrder() == QImage::BigEndian ) {
	png_set_bgr(png_ptr);
	png_set_swap_alpha(png_ptr);
    }

    uchar** jt = image.jumpTable();
    row_pointers=new png_bytep[info_ptr->height];

    for (uint y=0; y<info_ptr->height; y++) {
	row_pointers[y]=jt[y];
    }

    png_read_image(png_ptr, row_pointers);

#if 0 // libpng takes care of this.
    if (image.depth()==32 && (info_ptr->valid & PNG_INFO_tRNS)) {
	QRgb trans = 0xFF000000 | qRgb(
	      (info_ptr->trans_values.red << 8 >> info_ptr->bit_depth)&0xff,
	      (info_ptr->trans_values.green << 8 >> info_ptr->bit_depth)&0xff,
	      (info_ptr->trans_values.blue << 8 >> info_ptr->bit_depth)&0xff);
	for (uint y=0; y<info_ptr->height; y++) {
	    for (uint x=0; x<info_ptr->width; x++) {
		if (((uint**)jt)[y][x] == trans) {
		    ((uint**)jt)[y][x] &= 0x00FFFFFF;
		} else {
		}
	    }
	}
    }
#endif

    delete row_pointers;

    iio->setImage(image);

    png_read_end(png_ptr, end_info);
    png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);

    iio->setStatus(0);
}

static
void write_png_image(QImageIO* iio)
{
    png_structp png_ptr;
    png_infop info_ptr;
    png_bytep* row_pointers;

    png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING,0,0,0);
    if (!png_ptr) {
	iio->setStatus(-1);
	return;
    }

    info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr) {
	png_destroy_write_struct(&png_ptr, 0);
	iio->setStatus(-2);
	return;
    }

    if (setjmp(png_ptr->jmpbuf)) {
	png_destroy_write_struct(&png_ptr, &info_ptr);
	iio->setStatus(-4);
	return;
    }

    png_set_write_fn(png_ptr, (void*)iio, iod_write_fn, 0);

    const QImage& image = iio->image();

    info_ptr->channels =
	(image.depth() == 32)
	    ? (image.hasAlphaBuffer() ? 4 : 3)
	    : 1;

    png_set_IHDR(png_ptr, info_ptr, image.width(), image.height(),
	image.depth() == 1 ? 1 : 8 /* per channel */,
	image.depth() == 32
	    ? image.hasAlphaBuffer()
		? PNG_COLOR_TYPE_RGB_ALPHA
		: PNG_COLOR_TYPE_RGB
	    : PNG_COLOR_TYPE_PALETTE, 0, 0, 0);


    //png_set_sBIT(png_ptr, info_ptr, 8);
    info_ptr->sig_bit.red = 8;
    info_ptr->sig_bit.green = 8;
    info_ptr->sig_bit.blue = 8;

#if 0 // libpng takes care of this.
    if (image.depth() == 1 && image.bitOrder() == QImage::BigEndian)
       png_set_packswap(png_ptr);
#endif

    if (image.numColors()) {
	// Paletted
	info_ptr->valid |= PNG_INFO_PLTE;
	info_ptr->palette = new png_color[image.numColors()];
	info_ptr->num_palette = image.numColors();
	int* trans = new int[info_ptr->num_palette];
	int num_trans = 0;
	for (int i=0; i<info_ptr->num_palette; i++) {
	    QRgb rgb=image.color(i);
	    info_ptr->palette[i].red = qRed(rgb);
	    info_ptr->palette[i].green = qGreen(rgb);
	    info_ptr->palette[i].blue = qBlue(rgb);
	    if (image.hasAlphaBuffer()) {
		trans[i] = rgb >> 24;
		if (trans[i] < 255) {
		    num_trans = i+1;
		}
	    }
	}
	if (num_trans) {
	    info_ptr->valid |= PNG_INFO_tRNS;
	    info_ptr->trans = new png_byte[num_trans];
	    info_ptr->num_trans = num_trans;
	    for (int i=0; i<num_trans; i++)
		info_ptr->trans[i] = trans[i];
	}
	delete trans;
    }

    if ( image.hasAlphaBuffer() ) {
	info_ptr->sig_bit.alpha = 8;
    }

    if ( QImage::systemByteOrder() == QImage::BigEndian ) {
	png_set_bgr(png_ptr);
	png_set_swap_alpha(png_ptr);
    }

    png_write_info(png_ptr, info_ptr);

    if ( image.depth() != 1 )
	png_set_packing(png_ptr);

    if ( image.depth() == 32 && !image.hasAlphaBuffer() )
	png_set_filler(png_ptr, 0,
	    QImage::systemByteOrder() == QImage::BigEndian ?
		PNG_FILLER_BEFORE : PNG_FILLER_AFTER);

    uchar** jt = image.jumpTable();
    row_pointers=new png_bytep[info_ptr->height];
    uint y;
    for (y=0; y<info_ptr->height; y++) {
	    row_pointers[y]=jt[y];
    }
    png_write_image(png_ptr, row_pointers);
    delete row_pointers;

    png_write_end(png_ptr, info_ptr);

    if (image.numColors())
	delete info_ptr->palette;
    if (info_ptr->valid & PNG_INFO_tRNS)
	delete info_ptr->trans;

    png_destroy_write_struct(&png_ptr, &info_ptr);

    iio->setStatus(0);
}


void qInitPngIO()
{
    QImageIO::defineIOHandler("PNG", "^.PNG\r", 0, read_png_image, write_png_image);
}
