/****************************************************************************
** $Id: qpicture.cpp,v 2.7.2.1 1998/11/02 19:29:52 hanord Exp $
**
** Implementation of QPicture class
**
** Created : 940802
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

#include "qpicture.h"
#include "qpaintdevicedefs.h"
#include "qpainter.h"
#include "qpixmap.h"
#include "qimage.h"
#include "qfile.h"
#include "qdatastream.h"

/*!
  \class QPicture qpicture.h
  \brief The QPicture class is a paint device that records and replays QPainter
  commands.

  \ingroup drawing

  A picture serializes painter commands to an IO device in a
  platform-independent format.	A picture created under Windows
  can be read on a Sun SPARC.

  Pictures are called meta-files on some platforms.

  Qt pictures use a proprietary binary format.	Unlike native picture
  (meta-file) formats on many window systems, Qt pictures have no
  limitations regarding the contents.  Everything that can be painted can
  also be stored in a picture (fonts, pixmaps, regions, transformed
  graphics etc.)

  Example of how to record a picture:
  \code
    QPicture  pic;
    QPainter  p;
    p.begin( &pic );				// paint in picture
    p.drawEllipse( 10,20, 80,70 );		// draw an ellipse
    p.end();					// painting done
    pic.save( "drawing.pic" );			// save picture
  \endcode

  Example of how to replay a picture:
  \code
    QPicture  pic;
    pic.load( "drawing.pic" );			// load picture
    QPainter  p;
    p.begin( &myWidget );			// paint in myWidget
    p.drawPicture( pic );			// draw the picture
    p.end();					// painting done
  \endcode
*/


static const char  *mfhdr_tag = "QPIC";		// header tag
static const UINT16 mfhdr_maj = 1;		// major version #
static const UINT16 mfhdr_min = 0;		// minor version #


/*!
  \fn QPicture::QPicture()
  Constructs an empty picture.
*/

/*!
  \fn QPicture::~QPicture()
  Destroys the picture.
*/


/*!
  \fn bool QPicture::isNull() const
  Returns TRUE if the picture contains no data, otherwise FALSE.
*/

/*!
  \fn uint QPicture::size() const
  Returns the size of the picture data.
  \sa data()
*/

/*!
  \fn const char *QPicture::data() const
  Returns a pointer to the picture data.  The returned pointer is null
  if the picture contains no data.
  \sa size(), isNull()
*/

/*!
  Sets the picture data directly from \a data and \a size. This function
  copies the input data.
  \sa data(), size()
*/

void QPicture::setData( const char *data, uint size )
{
    QByteArray a( size );
    memcpy( a.data(), data, size );
    pictb.setBuffer( a );			// set byte array in buffer
    formatOk = FALSE;				// we'll have to check
}


/*!
  Loads a picture from the file specified by \e fileName and returns TRUE
  if successful, otherwise FALSE.

  \sa save()
*/

bool QPicture::load( const char *fileName )
{
    QByteArray a;
    QFile f( fileName );
    if ( !f.open(IO_ReadOnly) )
	return FALSE;
    a.resize( (uint)f.size() );
    f.readBlock( a.data(), (uint)f.size() );	// read file into byte array
    f.close();
    pictb.setBuffer( a );			// set byte array in buffer
    formatOk = FALSE;				// we'll have to check
    return TRUE;
}

/*!
  Saves a picture to the file specified by \e fileName and returns TRUE
  if successful, otherwise FALSE.

  \sa load()
*/

bool QPicture::save( const char *fileName )
{
    QFile f( fileName );
    if ( !f.open( IO_WriteOnly ) )
	return FALSE;
    f.writeBlock( pictb.buffer().data(), pictb.buffer().size() );
    f.close();
    return TRUE;
}


/*!
  Replays the picture using \e painter and returns TRUE if successful, or
  FALSE if the internal picture data is inconsistent.

  This function does exactly the same as QPainter::drawPicture().
*/

bool QPicture::play( QPainter *painter )
{
    if ( pictb.size() == 0 )			// nothing recorded
	return TRUE;

    pictb.open( IO_ReadOnly );			// open buffer device
    QDataStream s;
    s.setDevice( &pictb );			// attach data stream to buffer

    if ( !formatOk ) {				// first time we read it
	char mf_id[4];				// picture header tag
	s.readRawBytes( mf_id, 4 );		// read actual tag
	if ( memcmp(mf_id, mfhdr_tag, 4) != 0 ) { // wrong header id
#if defined(CHECK_RANGE)
	    warning( "QPicture::play: Incorrect header" );
#endif
	    pictb.close();
	    return FALSE;
	}

	int cs_start = sizeof(UINT32);		// pos of checksum word
	int data_start = cs_start + sizeof(UINT16);
	UINT16 cs,ccs;
	QByteArray buf = pictb.buffer();	// pointer to data
	s >> cs;				// read checksum
	ccs = qchecksum( buf.data() + data_start, buf.size() - data_start );
	if ( ccs != cs ) {
#if defined(CHECK_STATE)
	    warning( "QPicture::play: Invalid checksum %x, %x expected",
		     ccs, cs );
#endif
	    pictb.close();
	    return FALSE;
	}

	UINT16 major, minor;
	s >> major >> minor;			// read version number
	if ( major > mfhdr_maj ) {		// new, incompatible version
#if defined(CHECK_RANGE)
	    warning( "QPicture::play: Incompatible version %d.%d",
		     major, minor);
#endif
	    pictb.close();
	    return FALSE;
	}
	formatOk = TRUE;			// picture seems to be ok
    } else {
	s.device()->at( 10 );			// go directly to the data
    }

    UINT8  c, clen;
    UINT32 nrecords;
    s >> c >> clen;
    if ( c == PDC_BEGIN ) {
	s >> nrecords;
	if ( !exec( painter, s, nrecords ) )
	    c = 0;
    }
    if ( c !=  PDC_BEGIN ) {
#if defined(CHECK_RANGE)
	warning( "QPicture::play: Format error" );
#endif
	pictb.close();
	return FALSE;
    }
    pictb.close();
    return TRUE;				// no end-command
}


/*!
  \internal
  Iterates over the internal picture data and draws the picture using
  \e painter.
*/

bool QPicture::exec( QPainter *painter, QDataStream &s, int nrecords )
{
#if defined(DEBUG)
    int		strm_pos;
#endif
    UINT8	c;				// command id
    UINT8	tiny_len;			// 8-bit length descriptor
    INT32	len;				// 32-bit length descriptor
    INT16	i_16, i1_16, i2_16;		// parameters...
    INT8	i_8;
    UINT32	ul;
    char       *str;
    QPoint	p, p1, p2;
    QRect	r;
    QPointArray a;
    QColor	color;
    QFont	font;
    QPen	pen;
    QBrush	brush;
    QRegion	rgn;
    QWMatrix	matrix;

    while ( nrecords-- && !s.eof() ) {
	s >> c;					// read cmd
	s >> tiny_len;				// read param length
	if ( tiny_len == 255 )			// longer than 254 bytes
	    s >> len;
	else
	    len = tiny_len;
#if defined(DEBUG)
	strm_pos = s.device()->at();
#endif
	switch ( c ) {				// exec cmd
	    case PDC_NOP:
		break;
	    case PDC_DRAWPOINT:
		s >> p;
		painter->drawPoint( p );
		break;
	    case PDC_MOVETO:
		s >> p;
		painter->moveTo( p );
		break;
	    case PDC_LINETO:
		s >> p;
		painter->lineTo( p );
		break;
	    case PDC_DRAWLINE:
		s >> p1 >> p2;
		painter->drawLine( p1, p2 );
		break;
	    case PDC_DRAWRECT:
		s >> r;
		painter->drawRect( r );
		break;
	    case PDC_DRAWROUNDRECT:
		s >> r >> i1_16 >> i2_16;
		painter->drawRoundRect( r, i1_16, i2_16 );
		break;
	    case PDC_DRAWELLIPSE:
		s >> r;
		painter->drawEllipse( r );
		break;
	    case PDC_DRAWARC:
		s >> r >> i1_16 >> i2_16;
		painter->drawArc( r, i1_16, i2_16 );
		break;
	    case PDC_DRAWPIE:
		s >> r >> i1_16 >> i2_16;
		painter->drawPie( r, i1_16, i2_16 );
		break;
	    case PDC_DRAWCHORD:
		s >> r >> i1_16 >> i2_16;
		painter->drawChord( r, i1_16, i2_16 );
		break;
	    case PDC_DRAWLINESEGS:
		s >> a;
		painter->drawLineSegments( a );
		break;
	    case PDC_DRAWPOLYLINE:
		s >> a;
		painter->drawPolyline( a );
		break;
	    case PDC_DRAWPOLYGON:
		s >> a >> i_8;
		painter->drawPolygon( a, i_8 );
		break;
	    case PDC_DRAWQUADBEZIER:
		s >> a;
		painter->drawQuadBezier( a );
		break;
	    case PDC_DRAWTEXT:
		s >> p >> str;
		painter->drawText( p, str );
		delete str;
		break;
	    case PDC_DRAWTEXTFRMT:
		s >> r >> i_16 >> str;
		painter->drawText( r, i_16, str );
		delete str;
		break;
	    case PDC_DRAWPIXMAP: {
		QPixmap pixmap;
		s >> p >> pixmap;
		painter->drawPixmap( p, pixmap );
		}
		break;
	    case PDC_DRAWIMAGE: {
		QImage image;
		s >> p >> image;
		painter->drawImage( p, image );
		}
		break;
	    case PDC_BEGIN:
		s >> ul;			// number of records
		if ( !exec( painter, s, ul ) )
		    return FALSE;
		break;
	    case PDC_END:
		if ( nrecords == 0 )
		    return TRUE;
		break;
	    case PDC_SAVE:
		painter->save();
		break;
	    case PDC_RESTORE:
		painter->restore();
		break;
	    case PDC_SETBKCOLOR:
		s >> color;
		painter->setBackgroundColor( color );
		break;
	    case PDC_SETBKMODE:
		s >> i_8;
		painter->setBackgroundMode( (BGMode)i_8 );
		break;
	    case PDC_SETROP:
		s >> i_8;
		painter->setRasterOp( (RasterOp)i_8 );
		break;
	    case PDC_SETBRUSHORIGIN:
		s >> p;
		painter->setBrushOrigin( p );
		break;
	    case PDC_SETFONT:
		s >> font;
		painter->setFont( font );
		break;
	    case PDC_SETPEN:
		s >> pen;
		painter->setPen( pen );
		break;
	    case PDC_SETBRUSH:
		s >> brush;
		painter->setBrush( brush );
		break;
	    case PDC_SETTABSTOPS:
		s >> i_16;
		painter->setTabStops( i_16 );
		break;
	    case PDC_SETTABARRAY:
		s >> i_16;
		if ( i_16 == 0 ) {
		    painter->setTabArray( 0 );
		} else {
		    int *ta = new int[i_16];
		    CHECK_PTR( ta );
		    for ( int i=0; i<i_16; i++ ) {
			s >> i1_16;
			ta[i] = i1_16;
		    }
		    painter->setTabArray( ta );
		    delete [] ta;
		}
		break;
	    case PDC_SETVXFORM:
		s >> i_8;
		painter->setViewXForm( i_8 );
		break;
	    case PDC_SETWINDOW:
		s >> r;
		painter->setWindow( r );
		break;
	    case PDC_SETVIEWPORT:
		s >> r;
		painter->setViewport( r );
		break;
	    case PDC_SETWXFORM:
		s >> i_8;
		painter->setWorldXForm( i_8 );
		break;
	    case PDC_SETWMATRIX:
		s >> matrix >> i_8;
		painter->setWorldMatrix( matrix, i_8 );
		break;
	    case PDC_SETCLIP:
		s >> i_8;
		painter->setClipping( i_8 );
		break;
	    case PDC_SETCLIPRGN:
		s >> rgn;
		painter->setClipRegion( rgn );
		break;
	    default:
#if defined(CHECK_RANGE)
		warning( "QPicture::play: Invalid command %d", c );
#endif
		if ( len )			// skip unknown command
		    s.device()->at( s.device()->at()+len );
	}
#if defined(DEBUG)
	ASSERT( s.device()->at() - strm_pos == len );
#endif
    }
    return FALSE;
}


/*!
  \internal
  Records painter commands and stores them in the pictb buffer.
*/

bool QPicture::cmd( int c, QPainter *, QPDevCmdParam *p )
{
    QDataStream s;
    s.setDevice( &pictb );
    if ( c ==  PDC_BEGIN ) {			// begin; write header
	QByteArray empty( 0 );
	pictb.setBuffer( empty );		// reset byte array in buffer
	pictb.open( IO_WriteOnly );
	s.writeRawBytes( mfhdr_tag, 4 );
	s << (UINT16)0 << mfhdr_maj << mfhdr_min;
	s << (UINT8)c << (UINT8)sizeof(INT32);
	trecs = 0;
	s << (UINT32)trecs;			// total number of records
	formatOk = FALSE;
	return TRUE;
    } else if ( c == PDC_END ) {		// end; calc checksum and close
	trecs++;
	s << (UINT8)c << (UINT8)0;
	QByteArray buf = pictb.buffer();
	int cs_start = sizeof(UINT32);		// pos of checksum word
	int data_start = cs_start + sizeof(UINT16);
	int nrecs_start = data_start + 2*sizeof(INT16) + 2*sizeof(UINT8);
	int pos = pictb.at();
	pictb.at( nrecs_start );
	s << (UINT32)trecs;			// write number of records
	pictb.at( cs_start );
	UINT16 cs = (UINT16)qchecksum( buf.data()+data_start, pos-data_start );
	s << cs;				// write checksum
	pictb.close();
	return TRUE;
    }
    trecs++;
    s << (UINT8)c;				// write cmd to stream
    s << (UINT8)0;				// write dummy length info
    int pos = (int)pictb.at();			// save position
    switch ( c ) {
	case PDC_DRAWPOINT:
	case PDC_MOVETO:
	case PDC_LINETO:
	case PDC_SETBRUSHORIGIN:
	    s << *p[0].point;
	    break;
	case PDC_DRAWLINE:
	    s << *p[0].point << *p[1].point;
	    break;
	case PDC_DRAWRECT:
	case PDC_DRAWELLIPSE:
	    s << *p[0].rect;
	    break;
	case PDC_DRAWROUNDRECT:
	case PDC_DRAWARC:
	case PDC_DRAWPIE:
	case PDC_DRAWCHORD:
	    s << *p[0].rect << (INT16)p[1].ival << (INT16)p[2].ival;
	    break;
	case PDC_DRAWLINESEGS:
	case PDC_DRAWPOLYLINE:
	case PDC_DRAWQUADBEZIER:
	    s << *p[0].ptarr;
	    break;
	case PDC_DRAWPOLYGON:
	    s << *p[0].ptarr << (INT8)p[1].ival;
	    break;
	case PDC_DRAWTEXT:
	    s << *p[0].point << p[1].str;
	    break;
	case PDC_DRAWTEXTFRMT:
	    s << *p[0].rect << (INT16)p[1].ival << p[2].str;
	    break;
	case PDC_DRAWPIXMAP:
	    s << *p[0].point;
	    s << *p[1].pixmap;
	    break;
	case PDC_DRAWIMAGE:
	    s << *p[0].point;
	    s << *p[1].image;
	    break;
	case PDC_SAVE:
	case PDC_RESTORE:
	    break;
	case PDC_SETBKCOLOR:
	    s << *p[0].color;
	    break;
	case PDC_SETBKMODE:
	case PDC_SETROP:
	    s << (INT8)p[0].ival;
	    break;
	case PDC_SETFONT:
	    s << *p[0].font;
	    break;
	case PDC_SETPEN:
	    s << *p[0].pen;
	    break;
	case PDC_SETBRUSH:
	    s << *p[0].brush;
	    break;
	case PDC_SETTABSTOPS:
	    s << (INT16)p[0].ival;
	    break;
	case PDC_SETTABARRAY:
	    s << (INT16)p[0].ival;
	    if ( p[0].ival ) {
		int *ta = p[1].ivec;
		for ( int i=0; i<p[0].ival; i++ )
		    s << (INT16)ta[i];
	    }
	    break;
	case PDC_SETUNIT:
	case PDC_SETVXFORM:
	case PDC_SETWXFORM:
	case PDC_SETCLIP:
	    s << (INT8)p[0].ival;
	    break;
	case PDC_SETWINDOW:
	case PDC_SETVIEWPORT:
	    s << *p[0].rect;
	    break;
	case PDC_SETWMATRIX:
	    s << *p[0].matrix << (INT8)p[1].ival;
	    break;
	case PDC_SETCLIPRGN:
	    s << *p[0].rgn;
	    break;
#if defined(CHECK_RANGE)
	default:
	    warning( "QPicture::cmd: Command %d not recognized", c );
#endif
    }
    int newpos = (int)pictb.at();		// new position
    int length = newpos - pos;
    if ( length < 255 ) {			// write 8-bit length
	pictb.at(pos - 1);			// position to right index
	s << (UINT8)length;
    } else {					// write 32-bit length
	s << (UINT32)0;				// extend the buffer
	pictb.at(pos - 1);			// position to right index
	s << (UINT8)255;			// indicate 32-bit length
	char *p = pictb.buffer().data();
	memmove( p+pos+4, p+pos, length );	// make room for 4 byte
	s << (UINT32)length;
	newpos += 4;
    }
    pictb.at( newpos );				// set to new position
    return TRUE;
}


/*!
  Internal implementation of the virtual QPaintDevice::metric() function.

  Use the QPaintDeviceMetrics class instead.

  A picture has the following hard coded values:
  width=640, height=480. widthMM=236, heightMM=176, numcolors=16777216
  and depth=24.
*/

int QPicture::metric( int m ) const
{
    int val;
    switch ( m ) {
	case PDM_WIDTH:
	    val = 640;
	    break;
	case PDM_HEIGHT:
	    val = 480;
	    break;
	case PDM_WIDTHMM:
	    val = 236;
	    break;
	case PDM_HEIGHTMM:
	    val = 176;
	    break;
	case PDM_NUMCOLORS:
	    val = 16777216;
	    break;
	case PDM_DEPTH:
	    val = 24;
	    break;
	default:
	    val = 0;
#if defined(CHECK_RANGE)
	    warning( "QPicture::metric: Invalid metric command" );
#endif
    }
    return val;
}


/*****************************************************************************
  QPainter member functions
 *****************************************************************************/

/*!
  Replays the picture \e pic.

  This function does exactly the same as QPicture::play().
*/

void QPainter::drawPicture( const QPicture &pic )
{
    ((QPicture*)&pic)->play( (QPainter*)this );
}
