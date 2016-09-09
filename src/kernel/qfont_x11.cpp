/****************************************************************************
** $Id: qfont_x11.cpp,v 2.41.2.4 1998/11/23 11:40:30 warwick Exp $
**
** Implementation of QFont, QFontMetrics and QFontInfo classes for X11
**
** Created : 940515
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

#include "qwidget.h"
#include "qpainter.h"
#include "qfontdata.h"
#include "qcache.h"
#include "qdict.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#define GC GC_QQQ
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include <X11/Xatom.h>

static const int fontFields = 14;

enum FontFieldNames {				// X LFD fields
    Foundry,
    Family,
    Weight_,
    Slant,
    Width,
    AddStyle,
    PixelSize,
    PointSize,
    ResolutionX,
    ResolutionY,
    Spacing,
    AverageWidth,
    CharsetRegistry,
    CharsetEncoding };


// Internal functions

static bool	parseXFontName( QString &fontName, char **tokens );
static char   **getXFontNames( const char *pattern, int *count );
static bool	smoothlyScalable( const char *fontName );
static bool	fontExists( const char *fontName );
static int	getWeight( const char *weightString, bool adjustScore=FALSE );


// QFont_Private accesses QFont protected functions

class QFont_Private : public QFont
{
public:
    int	    fontMatchScore( char *fontName, QString &buffer,
			    float *pointSizeDiff, int *weightDiff,
			    bool *scalable, bool *polymorphic,
			    int *resx, int *resy );
    QString bestMatch( const char *pattern, int *score );
    QString bestFamilyMember( const char *family, int *score );
    QString findFont( bool *exact );
};

#undef	PRIV
#define PRIV ((QFont_Private*)this)


/*****************************************************************************
  QFontInternal contains X11 font data: an XLFD font name ("-*-*-..") and
  an X font struct.

  Two global dictionaries and a cache hold QFontInternal objects, which
  are shared between all QFonts.
  This mechanism makes font loading much faster than using XLoadQueryFont.
 *****************************************************************************/

class QFontInternal
{
public:
   ~QFontInternal();
    bool	    dirty() const;
    const char	   *name()  const;
    int		    xResolution() const;
    XFontStruct	   *fontStruct() const;
    const QFontDef *spec()  const;
    int		    lineWidth() const;
    void	    reset();
private:
    QFontInternal( const QString & );
    void computeLineWidth();

    QString	    n;
    XFontStruct	   *f;
    QFontDef	    s;
    int		    lw;
    int		    xres;
    friend void QFont::load(HANDLE) const;
    friend void QFont::initFontInfo() const;
};

inline QFontInternal::QFontInternal( const QString &name )
    : n(name), f(0)
{
    s.dirty = TRUE;
}

inline bool QFontInternal::dirty() const
{
    return f == 0;
}

inline const char *QFontInternal::name() const
{
    return n;
}

inline XFontStruct *QFontInternal::fontStruct() const
{
    return f;
}

inline const QFontDef *QFontInternal::spec() const
{
    return &s;
}

inline int QFontInternal::lineWidth() const
{
    return lw;
}

inline int QFontInternal::xResolution() const
{
    return xres;
}

inline void QFontInternal::reset()
{
    if ( f ) {
	XFreeFont( QPaintDevice::x__Display(), f );
	f = 0;
    }
}

inline QFontInternal::~QFontInternal()
{
    reset();
}


static const int reserveCost   = 1024*100;
static const int fontCacheSize = 1024*1024*4;


Q_DECLARE(QCacheM,QFontInternal);		// inherited by QFontCache
typedef Q_DECLARE(QCacheIteratorM,QFontInternal) QFontCacheIt;
typedef Q_DECLARE(QDictM,QFontInternal)		 QFontDict;
typedef Q_DECLARE(QDictIteratorM,QFontInternal)	 QFontDictIt;


class QFontCache : public QCacheM(QFontInternal)
{
public:
    QFontCache( int maxCost, int size=17, bool cs=TRUE, bool ck=TRUE )
	: QCacheM(QFontInternal)(maxCost,size,cs,ck) {}
    void deleteItem( GCI );
};

void QFontCache::deleteItem( GCI d )
{
    QFontInternal *fin = (QFontInternal *)d;
    fin->reset();
}


struct QXFontName
{
    QXFontName( const QString &n, bool e ) : name(n), exactMatch(e) {}
    QString name;
    bool    exactMatch;
};

typedef Q_DECLARE(QDictM,QXFontName) QFontNameDict;


static QFontCache    *fontCache	     = 0;	// cache of loaded fonts
static QFontDict     *fontDict	     = 0;	// dict of all loaded fonts
static QFontNameDict *fontNameDict   = 0;	// dict of matched font names
QFont		     *QFont::defFont = 0;	// default font


//
// This function returns the X font struct for a QFontData.
// It is called from QPainter::drawText().
//

XFontStruct *qt_get_xfontstruct( QFontData *d )
{
    return d->fin->fontStruct();
}


/*****************************************************************************
  QFont member functions
 *****************************************************************************/

/*!
  Internal function that initializes the font system.

  \internal
  The font cache and font dict do not alloc the keys. The key is a QString
  which is shared between QFontInternal and QXFontName.
*/

void QFont::initialize()
{
    if ( fontCache )
	return;
    fontCache = new QFontCache( fontCacheSize, 29, TRUE, FALSE );
    CHECK_PTR( fontCache );
    fontDict  = new QFontDict( 29, TRUE, FALSE );
    CHECK_PTR( fontDict );
    fontNameDict = new QFontNameDict( 29, TRUE, TRUE );
    CHECK_PTR( fontNameDict );
    fontNameDict->setAutoDelete( TRUE );
    if ( !defFont )
	defFont = new QFont( TRUE );		// create the default font
}

/*!
  Internal function that cleans up the font system.
*/

void QFont::cleanup()
{
    delete defFont;
    defFont = 0;
    delete fontCache;
    fontCache = 0;
    fontDict->setAutoDelete( TRUE );
    delete fontDict;
    delete fontNameDict;
}

/*!
  Internal function that dumps font cache statistics.
*/

void QFont::cacheStatistics()
{
#if defined(DEBUG)
    fontCache->statistics();
    QFontCacheIt it(*fontCache);
    QFontInternal *fin;
    debug( "{" );
    while ( (fin = it.current()) ) {
	++it;
	debug( "   [%s]", fin->name() );
    }
    debug( "}" );
#endif
}


/*!
  \internal
  Constructs a font object that refers to the default font.
*/

QFont::QFont( bool )
{
    init();
    d->req.family    = "6x13";
    d->req.pointSize = 11*10;
    d->req.weight    = QFont::Normal;
    d->req.rawMode   = TRUE;
}


// If d->req.dirty is not TRUE the font must have been loaded
// and we can safely assume that d->fin is a valid pointer:

#define DIRTY_FONT (d->req.dirty || d->fin->dirty())


/*!
  Returns the window system handle to the font, for low-level
  access.  <em>Using this function is not portable.</em>
*/

HANDLE QFont::handle( HANDLE ) const
{
    static Font last = 0;
    if ( DIRTY_FONT ) {
	load();
    } else {
	if ( d->fin->fontStruct()->fid != last )
	    fontCache->find( d->fin->name() );
    }
    last = d->fin->fontStruct()->fid;
    return last;
}

/*!
  Returns the name of the font within the underlying system.
  <em>Using the return value of this function is usually not
  portable.</em>

  \sa setRawMode(), rawMode()
*/
const char* QFont::rawName() const
{
    if ( DIRTY_FONT )
	load();
    return d->fin->name();
}



/*!
  Returns TRUE if the font attributes have been changed and the font has to
  be (re)loaded, or FALSE if no changes have been made.
*/

bool QFont::dirty() const
{
    return DIRTY_FONT;
}


/*!
  Returns the family name that corresponds to the current style hint.
*/

QString QFont::defaultFamily() const
{
    switch( d->req.styleHint ) {
	case Times:
	    return "times";
	case Courier:
	    return "courier";
	case Decorative:
	    return "old english";
	case Helvetica:
	case System:
	default:
	    return "helvetica";
    }
}

/*!
  Returns a last resort family name for the \link fontmatch.html font
  matching algorithm. \endlink

  \sa lastResortFont()
*/

QString QFont::lastResortFamily() const
{
    return "helvetica";
}

static const char *tryFonts[] = {
    "6x13",
    "7x13",
    "8x13",
    "9x15",
    "fixed",
    "-*-helvetica-medium-r-*-*-*-120-*-*-*-*-*-*",
    "-*-courier-medium-r-*-*-*-120-*-*-*-*-*-*",
    "-*-times-medium-r-*-*-*-120-*-*-*-*-*-*",
    "-*-lucida-medium-r-*-*-*-120-*-*-*-*-*-*",
    "-*-helvetica-*-*-*-*-*-120-*-*-*-*-*-*",
    "-*-courier-*-*-*-*-*-120-*-*-*-*-*-*",
    "-*-times-*-*-*-*-*-120-*-*-*-*-*-*",
    "-*-lucida-*-*-*-*-*-120-*-*-*-*-*-*",
    "-*-helvetica-*-*-*-*-*-*-*-*-*-*-*-*",
    "-*-courier-*-*-*-*-*-*-*-*-*-*-*-*",
    "-*-times-*-*-*-*-*-*-*-*-*-*-*-*",
    "-*-lucida-*-*-*-*-*-*-*-*-*-*-*-*",
    0 };

/*!
  Returns a last resort raw font name for the \link fontmatch.html font
  matching algorithm. \endlink

  This is used if not even the last resort family is available.

  \sa lastResortFamily()
*/

QString QFont::lastResortFont() const
{
    static const char *last = 0;
    if ( last )					// already found
	return last;
    int i = 0;
    const char *f;
    while ( (f = tryFonts[i]) ) {
	if ( fontExists(f) ) {
	    last = f;
	    return last;
	}
	i++;
    }
#if defined(CHECK_NULL)
    fatal( "QFont::lastResortFont: Cannot find any reasonable font" );
#endif
    return last;
}


static void resetFontDef( QFontDef *def )	// used by initFontInfo()
{
    def->pointSize     = 0;
    def->styleHint     = QFont::AnyStyle;
    def->weight	       = QFont::Normal;
    def->italic	       = FALSE;
    def->charSet       = QFont::Latin1;
    def->underline     = FALSE;
    def->strikeOut     = FALSE;
    def->fixedPitch    = FALSE;
    def->hintSetByUser = FALSE;
    def->lbearing      = SHRT_MIN;
    def->rbearing      = SHRT_MIN;
}

/*!
  Initializes the font information in the font's QFontInternal data.
  This function is called from load() for a new font.
*/

void QFont::initFontInfo() const
{
    QFontInternal *f = d->fin;
    if ( !f->s.dirty )				// already initialized
	return;
    f->s.lbearing = SHRT_MIN;
    f->s.rbearing = SHRT_MIN;
    f->computeLineWidth();
    if ( exactMatch() ) {			// match: copy font description
	f->s = d->req;
	f->s.dirty = FALSE;
	return;
    }

    char *tokens[fontFields];
    QString buffer = f->name();
    if ( !parseXFontName(buffer, tokens) ) {	// not an XLFD name
	resetFontDef( &f->s );
	f->s.family   = f->name();
	f->s.rawMode  = TRUE;
	d->exactMatch = FALSE;
	return;
    }
    f->s.family	   = tokens[Family];
    f->s.pointSize = atoi(tokens[PointSize]);
    f->s.styleHint = QFont::AnyStyle;	// ### any until we match families

    if ( strcmp( tokens[CharsetRegistry], "iso8859" ) == 0 ) {
	if ( strcmp( tokens[CharsetEncoding], "1" ) == 0 )
	    f->s.charSet = QFont::Latin1;
	else if ( strcmp( tokens[CharsetEncoding], "2" ) == 0 )
	    f->s.charSet = QFont::Latin2;
	else if ( strcmp( tokens[CharsetEncoding], "3" ) == 0 )
	    f->s.charSet = QFont::Latin3;
	else if ( strcmp( tokens[CharsetEncoding], "4" ) == 0 )
	    f->s.charSet = QFont::Latin4;
	else if ( strcmp( tokens[CharsetEncoding], "5" ) == 0 )
	    f->s.charSet = QFont::Latin5;
	else if ( strcmp( tokens[CharsetEncoding], "6" ) == 0 )
	    f->s.charSet = QFont::Latin6;
	else if ( strcmp( tokens[CharsetEncoding], "7" ) == 0 )
	    f->s.charSet = QFont::Latin7;
	else if ( strcmp( tokens[CharsetEncoding], "8" ) == 0 )
	    f->s.charSet = QFont::Latin8;
	else if ( strcmp( tokens[CharsetEncoding], "9" ) == 0 )
	    f->s.charSet = QFont::Latin9;
    } else if( strcmp( tokens[CharsetRegistry], "koi8" ) == 0 &&
	       strcmp( tokens[CharsetEncoding], "r" ) == 0) {
	f->s.charSet = QFont::KOI8R;
    } else {
	f->s.charSet = QFont::AnyCharSet;
    }

    char slant	= tolower( tokens[Slant][0] );
    f->s.italic = (slant == 'o' || slant == 'i');
    char fixed	= tolower( tokens[Spacing][0] );
    f->s.fixedPitch = (fixed == 'm' || fixed == 'c');
    f->s.weight = getWeight( tokens[Weight_] );

    if ( strcmp(tokens[ResolutionY], "75") != 0 ) { // if not 75 dpi
	f->s.pointSize = ( 2*f->s.pointSize*atoi(tokens[ResolutionY]) + 1)
			   / (75 * 2);		// adjust actual pointsize
    }
    f->s.underline = d->req.underline;
    f->s.strikeOut = d->req.strikeOut;
    f->s.rawMode = FALSE;
    f->s.dirty	 = FALSE;
}


/*!
  Loads the font.
*/

void QFont::load( HANDLE ) const
{
    if ( !fontCache )				// not initialized
	return;

    QString k = key();
    QXFontName *fn = fontNameDict->find( k );

    if ( !fn ) {
	QString name;
	bool match;
	if ( d->req.rawMode ) {
	    name = substitute( family() );
	    match = fontExists( name );
	    if ( !match )
		name = lastResortFont();
	} else {
	    name = PRIV->findFont( &match );
	}
	fn = new QXFontName( name, match );
	CHECK_PTR( fn );
	fontNameDict->insert( k, fn );
    }

    QString n = fn->name;
    d->fin = fontCache->find( n );
    if ( !d->fin ) {				// font not loaded
	d->fin = fontDict->find( n );
	if ( !d->fin ) {			// font was never loaded
	    d->fin = new QFontInternal( n );
	    CHECK_PTR( d->fin );
	    fontDict->insert( d->fin->name(), d->fin );
	}
    }
    XFontStruct *f = d->fin->f;
    if ( !f ) {					// font not loaded
	f = XLoadQueryFont( QPaintDevice::x__Display(), n );
	if ( !f ) {
	    f = XLoadQueryFont( QPaintDevice::x__Display(), lastResortFont());
	    fn->exactMatch = FALSE;
#if defined(CHECK_NULL)
	    if ( !f )
		fatal( "QFont::load: Internal error" );
#endif
	}
	int size = (f->max_bounds.ascent + f->max_bounds.descent) *
		   f->max_bounds.width *
		   (f->max_char_or_byte2 - f->min_char_or_byte2) / 8;
	// If we get a cache overflow, we make room for this font only
	if ( size > fontCache->maxCost() + reserveCost )
	    fontCache->setMaxCost( size + reserveCost );
	if ( !fontCache->insert(d->fin->name(), d->fin, size) ) {
#if defined(DEBUG)
	    fatal( "QFont::load: Cache overflow error" );
#endif
	}
	d->fin->f = f;
	initFontInfo();
    }
    d->exactMatch = fn->exactMatch;
    d->req.dirty = FALSE;
}


/*****************************************************************************
  QFont_Private member functions
 *****************************************************************************/

#define exactScore	 0xffff

#define CharSetScore	 0x40
#define PitchScore	 0x20
#define ResolutionScore	 0x10
#define SizeScore	 0x08
#define WeightScore	 0x04
#define SlantScore	 0x02
#define WidthScore	 0x01

//
// Returns a score describing how well a font name matches the contents
// of a font.
//

int QFont_Private::fontMatchScore( char	 *fontName,	 QString &buffer,
				   float *pointSizeDiff, int  *weightDiff,
				   bool	 *scalable     , bool *polymorphic,
				   int	 *resx	       , int  *resy )
{
    char *tokens[fontFields];
    bool   exactMatch = TRUE;
    int	   score      = 0;
    *scalable	      = FALSE;
    *polymorphic      = FALSE;
    *weightDiff	      = 0;
    *pointSizeDiff    = 0;

    strcpy( buffer.data(), fontName );	// NOTE: buffer must be large enough
    if ( !parseXFontName( buffer, tokens ) )
	return 0;	// Name did not conform to X Logical Font Description

    if ( strncmp( tokens[CharsetRegistry], "ksc", 3 ) == 0 &&
		  isdigit( tokens[CharsetRegistry][3] )	  ||
	 strncmp( tokens[CharsetRegistry], "jisx", 4 ) == 0  &&
		  isdigit( tokens[CharsetRegistry][4] )	  ||
	 strncmp( tokens[CharsetRegistry], "gb", 2 ) == 0  &&
		  isdigit( tokens[CharsetRegistry][2] ) ) {
	     return 0; // Dirty way of avoiding common 16 bit charsets ###
    }

#undef	IS_ZERO
#define IS_ZERO(X) (X[0] == '0' && X[1] == 0)

    if ( IS_ZERO(tokens[Weight_]) ||
	 IS_ZERO(tokens[Slant])	  ||
	 IS_ZERO(tokens[Width]) )
	*polymorphic = TRUE;			// polymorphic font

    if ( IS_ZERO(tokens[PixelSize]) &&
	 IS_ZERO(tokens[PointSize]) &&
	 IS_ZERO(tokens[AverageWidth]) )
	*scalable = TRUE;			// scalable font

    if ( charSet() == AnyCharSet ) {
	score |= CharSetScore;
     } else if ( charSet() == KOI8R ) {
       if ( strcmp( tokens[CharsetRegistry], "koi8" ) == 0 &&
	    strcmp( tokens[CharsetEncoding], "r" ) == 0 )
	       score |= CharSetScore;
       else
	       exactMatch = FALSE;
    } else if ( strcmp( tokens[CharsetRegistry], "iso8859" ) == 0 ) {
	// need to mask away non-8859 charsets here
	switch( charSet() ) {
	case Latin1:
	    if ( strcmp( tokens[CharsetEncoding], "1" ) == 0 )
		score |= CharSetScore;
	    else
		exactMatch = FALSE;
	    break;
	case Latin2:
	    if ( strcmp( tokens[CharsetEncoding], "2" ) == 0 )
		score |= CharSetScore;
	    else
		exactMatch = FALSE;
	    break;
	case Latin3:
	    if ( strcmp( tokens[CharsetEncoding], "3" ) == 0 )
		score |= CharSetScore;
	    else
		exactMatch = FALSE;
	    break;
	case Latin4:
	    if ( strcmp( tokens[CharsetEncoding], "4" ) == 0 )
		score |= CharSetScore;
	    else
		exactMatch = FALSE;
	    break;
	case Latin5:
	    if ( strcmp( tokens[CharsetEncoding], "5" ) == 0 )
		score |= CharSetScore;
	    else
		exactMatch = FALSE;
	    break;
	case Latin6:
	    if ( strcmp( tokens[CharsetEncoding], "6" ) == 0 )
		score |= CharSetScore;
	    else
		exactMatch = FALSE;
	    break;
	case Latin7:
	    if ( strcmp( tokens[CharsetEncoding], "7" ) == 0 )
		score |= CharSetScore;
	    else
		exactMatch = FALSE;
	    break;
	case Latin8:
	    if ( strcmp( tokens[CharsetEncoding], "8" ) == 0 )
		score |= CharSetScore;
	    else
		exactMatch = FALSE;
	    break;
	case Latin9:
	    if ( strcmp( tokens[CharsetEncoding], "9" ) == 0 )
		score |= CharSetScore;
	    else
		exactMatch = FALSE;
	    break;
	default:
	    // ### should not get here
	    break;
	}
    }

    char pitch = tolower( tokens[Spacing][0] );
    if ( fixedPitch() ) {
	if ( pitch == 'm' || pitch == 'c' )
	    score |= PitchScore;
	else
	    exactMatch = FALSE;
    } else {
	if ( pitch != 'p' )
	    exactMatch = FALSE;
    }

    int pSize;
    if ( *scalable )
	pSize = deciPointSize();	// scalable font
    else
	pSize = atoi( tokens[PointSize] );

    if ( strcmp(tokens[ResolutionX], "0") == 0 &&
	 strcmp(tokens[ResolutionY], "0") == 0 ) {
	// smoothly scalable font, we can ask for any resolution
	score |= ResolutionScore;
    } else {
	int localResx = atoi(tokens[ResolutionX]);
	int localResy = atoi(tokens[ResolutionY]);
	if ( *resx == 0 || *resy == 0 ) {
	    *resx = localResx;
	    *resy = localResy;
	}
	if ( localResx == *resx && localResy == *resy )
	    score |= ResolutionScore;
	else
	    exactMatch = FALSE;
    }

    float diff;
    if ( deciPointSize() != 0 )
	diff = ((float)QABS(pSize - deciPointSize())/deciPointSize())*100.0F;
    else
	diff = (float)pSize;

    if ( diff < 20 ) {
	if ( pSize != deciPointSize() )
	    exactMatch = FALSE;
	score |= SizeScore;
    } else {
	exactMatch = FALSE;
    }
    if ( pointSizeDiff )
	*pointSizeDiff = diff;

    int weightVal = getWeight( tokens[Weight_], TRUE );

    if ( weightVal == weight() )
	score |= WeightScore;
    else
	exactMatch = FALSE;

    *weightDiff = QABS( weightVal - weight() );
    char slant = tolower( tokens[Slant][0] );
    if ( italic() ) {
	if ( slant == 'o' || slant == 'i' )
	    score |= SlantScore;
	else
	    exactMatch = FALSE;
    } else {
	if ( slant == 'r' )
	    score |= SlantScore;
	else
	    exactMatch = FALSE;
    }
    if ( stricmp( tokens[Width], "normal" ) == 0 )
	score |= WidthScore;
    else
	exactMatch = FALSE;
    return exactMatch ? exactScore : score;
}


struct MatchData {			// internal for bestMatch
    MatchData() { score=0; name=0; pointDiff=99; weightDiff=99; }
    int	    score;
    char   *name;
    float   pointDiff;
    int	    weightDiff;
};

QString QFont_Private::bestMatch( const char *pattern, int *score )
{
    MatchData	best;
    MatchData	bestScalable;

    QString	matchBuffer( 256 );	// X font name always <= 255 chars
    char **	xFontNames;
    int		count;
    int		sc;
    float	pointDiff;	// difference in % from requested point size
    int		weightDiff;	// difference from requested weight
    int		resx	    = 0;
    int		resy	    = 0;
    bool	scalable    = FALSE;
    bool	polymorphic = FALSE;
    int		i;

    xFontNames = getXFontNames( pattern, &count );

    for( i = 0; i < count; i++ ) {
	sc = fontMatchScore( xFontNames[i], matchBuffer,
			     &pointDiff, &weightDiff,
			     &scalable, &polymorphic, &resx, &resy );
	if ( sc > best.score ||
	     sc == best.score && pointDiff < best.pointDiff ||
	     sc == best.score && pointDiff == best.pointDiff &&
				 weightDiff < best.weightDiff ) {
	    if ( scalable ) {
		if ( sc > bestScalable.score ||
		     sc == bestScalable.score &&
			   weightDiff < bestScalable.weightDiff ) {
		    bestScalable.score	    = sc;
		    bestScalable.name	    = xFontNames[i];
		    bestScalable.pointDiff  = pointDiff;
		    bestScalable.weightDiff = weightDiff;
		}
	    } else {
		best.score	= sc;
		best.name	= xFontNames[i];
		best.pointDiff	= pointDiff;
		best.weightDiff = weightDiff;
	    }
	}
    }
    QString bestName;
    char *tokens[fontFields];

    if ( best.score != exactScore && ( bestScalable.score > best.score ||
	     bestScalable.score == best.score && (
		 best.pointDiff != 0 ||
		 best.weightDiff < bestScalable.weightDiff ) ) ) {
	if ( smoothlyScalable( bestScalable.name ) ) {
	    if ( resx == 0 || resy == 0 ) {
		resx = 75;
		resy = 75;
	    }
	    best.score = bestScalable.score;
	    strcpy( matchBuffer.data(), bestScalable.name );
	    if ( parseXFontName( matchBuffer, tokens ) ) {
		bestName.sprintf( "-%s-%s-%s-%s-%s-%s-*-%i-%i-%i-%s-*-%s-%s",
				  tokens[Foundry],
				  tokens[Family],
				  tokens[Weight_],
				  tokens[Slant],
				  tokens[Width],
				  tokens[AddStyle],
				  deciPointSize(),
				  resx, resy,
				  tokens[Spacing],
				  tokens[CharsetRegistry],
				  tokens[CharsetEncoding] );
		best.name = bestName.data();
	    }
	}
    }
    *score = best.score;
    bestName = best.name;
    XFreeFontNames( xFontNames );

    return bestName;
}


QString QFont_Private::bestFamilyMember( const char *family, int *score )
{
    char pattern[256];
    sprintf( pattern, "-*-%s-*-*-*-*-*-*-*-*-*-*-*-*", family );
    return bestMatch( pattern, score );
}


QString QFont_Private::findFont( bool *exact )
{
    QString familyName = family();
    *exact = TRUE;				// assume exact match
    if ( familyName.isEmpty() ) {
	familyName = defaultFamily();
	*exact = FALSE;
    }

    int score;
    QString bestName = bestFamilyMember( familyName, &score );
    if ( score != exactScore )
	*exact = FALSE;

    if( score == 0 )
    {
       QString f = substitute( family() );
       if( familyName != f ) {
           familyName = f;                     // try substitution
           bestName = bestFamilyMember( familyName, &score );
       }
    }
    if ( score == 0 ) {
	QString f = defaultFamily();
	if( familyName != f ) {
	    familyName = f;			// try default family for style
	    bestName = bestFamilyMember( familyName, &score );
	}
	if ( score == 0 ) {
	    f = lastResortFamily();
	    if ( familyName != f ) {
		familyName = f;			// try system default family
		bestName = bestFamilyMember( familyName, &score );
	    }
	}
    }
    if ( bestName.isNull() )			// no matching fonts found
	bestName = lastResortFont();
    return bestName;
}


/*****************************************************************************
  QFontMetrics member functions
 *****************************************************************************/

const QFontDef *QFontMetrics::spec() const
{
    const QFontDef *s;
    if ( type() == FontInternal ) {
	s = u.f->spec();
    } else if ( type() == Widget && u.w ) {
	QFont *f = (QFont *)&u.w->font();
	f->handle();
	s = f->d->fin->spec();
    } else if ( type() == Painter && u.p ) {
	QFont *f = (QFont *)&u.p->font();
	f->handle();
	s = f->d->fin->spec();
    } else {
	s = 0;
    }
#if defined(CHECK_NULL)
    if ( !s )
	warning( "QFontMetrics: Invalid font metrics" );
#endif
    return s;
}

void *QFontMetrics::fontStruct() const
{
    if ( type() == FontInternal ) {
	return u.f->fontStruct();
    } else if ( type() == Widget && u.w ) {
	QFont *f = (QFont *)&u.w->font();
	f->handle();
	return f->d->fin->fontStruct();
    } else if ( type() == Painter && u.p ) {
	QFont *f = (QFont *)&u.p->font();
	f->handle();
	return f->d->fin->fontStruct();
    } else {
#if defined(CHECK_NULL)
	warning( "QFontMetrics: Invalid font metrics" );
#endif
	return 0;
    }
}

#undef	FS
#define FS (type() == FontInternal ? u.f->fontStruct() : (XFontStruct*)fontStruct())

int QFontMetrics::printerAdjusted(int val) const
{
    if ( type() == Painter && u.p->device() &&		
	 u.p->device()->devType() == PDT_PRINTER ) {	
	QFont fnt =  u.p->font();				
	fnt.handle();					
	int xres = fnt.d->fin->xResolution();		
	return qRound(val*75.0/(xres*1.0));		
    } else {						
	return val;
    }							
}

/*!
  Returns the maximum ascent of the font.

  The ascent is the distance from the base line to the uppermost line
  where pixels may be drawn.

  \sa descent()
*/

int QFontMetrics::ascent() const
{
    return printerAdjusted(FS->max_bounds.ascent);
}


/*!
  Returns the maximum descent of the font.

  The descent is the distance from the base line to the lowermost line
  where pixels may be drawn. (Note that this is different from X, which
  adds 1 pixel.)

  \sa ascent()
*/

int QFontMetrics::descent() const
{
    return printerAdjusted(FS->max_bounds.descent - 1);
}

/*!
  Returns TRUE if \a ch is a valid character in the font.
*/
bool QFontMetrics::inFont(char ch) const
{
    XFontStruct *f = FS;
    return (uint)(uchar)ch >= f->min_char_or_byte2
	&& (uint)(uchar)ch <= f->max_char_or_byte2;
}

/*!
  Returns the left bearing of character \a ch in the font.

  The left bearing is the rightward distance of the left-most pixel
  of the character from the logical origin of the character.
  This value is negative if the pixels of the character extend
  to the left of the logical origin.

  <em>See width(char) for a graphical description of this metric.</em>

  \sa rightBearing(char), minLeftBearing(), width()
*/
int QFontMetrics::leftBearing(char ch) const
{
    XFontStruct *f = FS;
    if ( !inFont(ch) )
	ch = f->default_char;
    XCharStruct* cs = f->per_char
			? f->per_char + ((uchar)ch - f->min_char_or_byte2)
			: &f->max_bounds;
    return printerAdjusted(cs->lbearing);
}

/*!
  Returns the right bearing of character \a ch in the font.

  The right bearing is the leftward distance of the right-most pixel
  of the character from the logical origin of a subsequent character.
  This value is negative if the pixels of the character extend
  to the right of the width() of the character.

  <em>See width() for a graphical description of this metric.</em>

  \sa leftBearing(char), minRightBearing(), width()
*/
int QFontMetrics::rightBearing(char ch) const
{
    XFontStruct *f = FS;
    if ( !inFont(ch) )
	ch = f->default_char;
    XCharStruct* cs = f->per_char
			? f->per_char + ((uchar)ch - f->min_char_or_byte2)
			: &f->max_bounds;
    return printerAdjusted(cs->width - cs->rbearing);
}

/*!
  Returns the minimum left bearing of the font.

  This is the smallest leftBearing(char)
  of all characters in the font.

  \sa minRightBearing(), leftBearing(char)
*/
int QFontMetrics::minLeftBearing() const
{
    // Don't need def->lbearing, the FS stores it.
    return printerAdjusted(FS->min_bounds.lbearing);
}

/*!
  Returns the minimum right bearing of the font.

  This is the smallest rightBearing(char)
  of all characters in the font.

  \sa minLeftBearing(), rightBearing(char)
*/
int QFontMetrics::minRightBearing() const
{
    // Safely cast away const, as we cache rbearing there.
    QFontDef* def = (QFontDef*)spec();

    if ( def->rbearing == SHRT_MIN ) {
	XFontStruct *f = FS;
	if ( f->per_char ) {
	    XCharStruct *c = f->per_char;
	    int nc = f->max_char_or_byte2 - f->min_char_or_byte2 + 1;
	    int mx = c->width - c->rbearing;
	    for ( int i=1; i < nc; i++ ) {
		int nmx = c[i].width - c[i].rbearing;
		if ( nmx < mx )
		    mx = nmx;
	    }
	    def->rbearing = mx;
	} else {
	    def->rbearing = f->max_bounds.width - f->max_bounds.rbearing;
	}
    }

    return printerAdjusted(def->rbearing);
}


/*!
  Returns the height of the font.

  This is always equal to ascent()+descent()+1 (the 1 is for the base line).

  \sa leading(), lineSpacing()
*/

int QFontMetrics::height() const
{
    XFontStruct *f = FS;
    return printerAdjusted(f->max_bounds.ascent + f->max_bounds.descent);
}

/*!
  Returns the leading of the font.

  This is the natural inter-line spacing.

  \sa height(), lineSpacing()
*/

int QFontMetrics::leading() const
{
    XFontStruct *f = FS;
    int l = f->ascent		 + f->descent -
	    f->max_bounds.ascent - f->max_bounds.descent;
    if ( l > 0 ) {
	return printerAdjusted(l);
    } else {
	return 0;
    }
}


/*!
  Returns the distance from one base line to the next.

  This value is always equal to leading()+height().

  \sa height(), leading()
*/

int QFontMetrics::lineSpacing() const
{
    return leading() + height();
}

/*!
  Returns the logical width of a \e ch in pixels.  This is
  a distance appropriate for drawing a subsequent character
  after \e ch.

  <img src=bearings.gif align=right>
  Some of the metrics are described in the image to the right.
  The tall dark rectangle covers the logical width() of a character.
  The shorter
  pale rectangles cover the
  \link QFontMetrics::leftBearing() left\endlink and
  \link QFontMetrics::rightBearing() right\endlink bearings
  of the characters.  Notice that the bearings of "f" in this particular
  font are both negative, while the bearings of "o" are both positive.

  \sa boundingRect()
*/

int QFontMetrics::width( char ch ) const
{
    XFontStruct *f = FS;
    if ( !inFont(ch) )
	ch = f->default_char;
    XCharStruct* cs = f->per_char
			? f->per_char + ((uchar)ch - f->min_char_or_byte2)
			: &f->max_bounds;
    return printerAdjusted(cs->width);
}

/*!
  Returns the width in pixels of the first \e len characters of \e str.

  If \e len is negative (default value), the whole string is used.

  Note that this value is \e not equal to boundingRect().width();
  boundingRect() returns a rectangle describing the pixels this string
  will cover whereas width() returns the distance to where the next string
  should be drawn.  Thus, width(stra)+width(strb) is always equal to
  width(strcat(stra, strb)).  This is almost never the case with
  boundingRect().

  \sa boundingRect()
*/

int QFontMetrics::width( const char *str, int len ) const
{
    if ( len < 0 )
	len = strlen( str );
    return printerAdjusted(XTextWidth( FS, str, len ));
}


/*!
  Returns the bounding rectangle of the first \e len characters of \e str,
  which is the set of pixels the text would cover if drawn at (0,0).

  If \e len is negative (default value), the whole string is used.

  Note that the bounding rectangle may extend to the left of (0,0),
  e.g. for italicized fonts, and that the text output may cover \e all
  pixels in the bounding rectangle.

  Newline characters are processed as regular characters, \e not as
  linebreaks.

  Due to the different actual character heights, the height of the
  bounding rectangle of "Yes" and "yes" may be different.

  \sa width(), QPainter::boundingRect() */

QRect QFontMetrics::boundingRect( const char *str, int len ) const
{
    // Values are printerAdjusted during calculations.

    if ( len < 0 )
	len = strlen( str );
    XFontStruct	   *f = FS;
    int direction;
    int ascent;
    int descent;
    XCharStruct overall;

    bool underline;
    bool strikeOut;
    if ( type() == FontInternal ) {
	underline = underlineFlag();
	strikeOut = strikeOutFlag();
    } else if ( type() == Widget && u.w ) {	
	const QFont &f = u.w->font();
	underline = f.underline();
	strikeOut = f.strikeOut();
    } else if ( type() == Painter && u.p ) {
	const QFont &f = u.p->font();
	underline = f.underline();
	strikeOut = f.strikeOut();
    } else {
	underline = strikeOut = FALSE;
    }

    XTextExtents( f, str, len, &direction, &ascent, &descent, &overall );

    overall.lbearing = printerAdjusted(overall.lbearing);
    overall.rbearing = printerAdjusted(overall.rbearing);
    overall.ascent = printerAdjusted(overall.ascent);
    overall.descent = printerAdjusted(overall.descent);
    overall.width = printerAdjusted(overall.width);

    int startX = overall.lbearing;
    int width  = overall.rbearing - startX;
    ascent     = overall.ascent;
    descent    = overall.descent;
    if ( !underline && !strikeOut ) {
	width  = overall.rbearing - startX;
    } else {
	if ( startX > 0 )
	    startX = 0;
	if ( overall.rbearing < overall.width )
	   width =  overall.width - startX;
	else
	   width =  overall.rbearing - startX;
	if ( underline && len != 0 ) {
	    int ulTop = underlinePos();
	    int ulBot = ulTop + lineWidth(); // X descent is 1
	    if ( descent < ulBot )	// more than logical descent, so don't
		descent = ulBot;	// subtract 1 here!
	    if ( ascent < -ulTop )
		ascent = -ulTop;
	}
	if ( strikeOut && len != 0 ) {
	    int soTop = strikeOutPos();
	    int soBot = soTop - lineWidth(); // same as above
	    if ( descent < -soBot )
		descent = -soBot;
	    if ( ascent < soTop )
		ascent = soTop;
	}
    }
    return QRect( startX, -ascent, width, descent + ascent );
}


/*!
  Returns the width of the widest character in the font.
*/

int QFontMetrics::maxWidth() const
{
    return printerAdjusted(FS->max_bounds.width);
}


/*!
  Returns the distance from the base line to where an underscore should be
  drawn.
  \sa strikeOutPos(), lineWidth()
*/

int QFontMetrics::underlinePos() const
{
    int pos = (lineWidth()*2 + 3)/6;
    if ( pos ) {
	return pos;
    } else {
	return 1;
    }
}


/*!
  Returns the distance from the base line to where the strike-out line
  should be drawn.
  \sa underlinePos(), lineWidth()
*/

int QFontMetrics::strikeOutPos() const
{
    int pos = FS->max_bounds.ascent/3;
    if ( pos ) {
	return printerAdjusted(pos);
    } else {
	return 1;
    }
}


/*!
  Returns the width of the underline and strike-out lines, adjusted for
  the point size of the font.
  \sa underlinePos(), strikeOutPos()
*/

int QFontMetrics::lineWidth() const
{
    if ( type() == FontInternal ) {
	return u.f->lineWidth();
    } else {
	QFont f = font();
	f.handle();
	return printerAdjusted(f.d->fin->lineWidth());
    }
}


/*****************************************************************************
  QFontInfo member functions
 *****************************************************************************/

const QFontDef *QFontInfo::spec() const
{
    const QFontDef *s;
    if ( type() == FontInternal ) {
	s = u.f->spec();
    } else if ( type() == Widget && u.w ) {
	QFont *f = (QFont *)&u.w->font();
	f->handle();
	s = f->d->fin->spec();
    } else if ( type() == Painter && u.p ) {
	QFont *f = (QFont *)&u.p->font();
	f->handle();
	s = f->d->fin->spec();
    } else {
	s = 0;
    }
#if defined(CHECK_NULL)
    if ( !s )
	warning( "QFontInfo: Invalid font info" );
#endif
    return s;
}


/*****************************************************************************
  Internal X font functions
 *****************************************************************************/

//
// Splits an X font name into fields separated by '-'
//

static bool parseXFontName( QString &fontName, char **tokens )
{
    if ( fontName.isEmpty() || fontName[0] != '-' ) {
	tokens[0] = 0;
	return FALSE;
    }
    int	  i;
    char *f = fontName.data() + 1;
    for ( i=0; i<fontFields && f && f[0]; i++ ) {
	tokens[i] = f;
	f = strchr( f, '-' );
	if( f )
	    *f++ = '\0';
    }
    if ( i < fontFields ) {
	for( int j=i ; j<fontFields; j++ )
	    tokens[j] = 0;
	return FALSE;
    }
    return TRUE;
}


//
// Get an array of X font names that matches a pattern
//

static char **getXFontNames( const char *pattern, int *count )
{
    static int maxFonts = 256;
    char **list;
    while( 1 ) {
	list = XListFonts( QPaintDevice::x__Display(), (char*)pattern,
			   maxFonts, count );
	if ( *count != maxFonts || maxFonts >= 32768 )
	    return list;
	XFreeFontNames( list );
	maxFonts *= 2;
    }
}


//
// Returns TRUE if the font can be smoothly scaled
//

static bool smoothlyScalable ( const char * /* fontName */  )
{
    return TRUE;
}


//
// Returns TRUE if the font exists, FALSE otherwise
//

static bool fontExists( const char *fontName )
{
    char **fontNames;
    int	   count;
    fontNames = getXFontNames( fontName, &count );
    XFreeFontNames( fontNames );
    return count != 0;
}


//
// Computes the line width (underline,strikeout) for the X font
// and fills in the X resolution of the font.
//

void QFontInternal::computeLineWidth()
{
    char *tokens[fontFields];
    QString buffer(256);		// X font name always <= 255 chars
    strcpy( buffer.data(), name() );
    if ( !parseXFontName(buffer, tokens) ) {
	lw   = 1;		    // name did not conform to X LFD
	xres = 75;
	return;
    }
    int weight = getWeight( tokens[Weight_] );
    int pSize  = atoi( tokens[PointSize] ) / 10;
    if ( strcmp( tokens[ResolutionX], "75") != 0 || // adjust if not 75 dpi
	 strcmp( tokens[ResolutionY], "75") != 0 )
	pSize = ( 2*pSize*atoi(tokens[ResolutionY]) + 75 ) / ( 75 * 2 );
    QString tmp = tokens[ResolutionX];
    bool ok;
    xres = tmp.toInt( &ok );
    if ( !ok || xres == 0 )
	xres = 75;		
    int score = pSize*weight;		// ad hoc algorithm
    lw = ( score ) / 700;
    if ( lw < 2 && score >= 1050 )	// looks better with thicker line
	lw = 2;				//   for small pointsizes
    if ( lw == 0 )
	lw = 1;
}


//
// Converts a weight string to a value
//

static int getWeight( const char *weightString, bool adjustScore )
{
    // Test in decreasing order of commonness
    //
    if (!qstricmp( weightString, "medium" ))	   return QFont::Normal;
    else if (!qstricmp( weightString, "bold" ))	   return QFont::Bold;
    else if (!qstricmp( weightString, "demibold")) return QFont::DemiBold;
    else if (!qstricmp( weightString, "black" ))   return QFont::Black;
    else if (!qstricmp( weightString, "light" ))   return QFont::Light;

    QString s = weightString;
    s = s.lower();
    if ( s.contains("bold") ) {
	if ( adjustScore )
	    return (int) QFont::Bold - 1;  // - 1, not sure that this IS bold
	else
	    return (int) QFont::Bold;
    }
    if ( s.contains("light") ) {
	if ( adjustScore )
	    return (int) QFont::Light - 1; // - 1, not sure that this IS light
       else
	    return (int) QFont::Light;
    }
    if ( s.contains("black") ) {
	if ( adjustScore )
	    return (int) QFont::Black - 1; // - 1, not sure this IS black
	else
	    return (int) QFont::Black;
    }
    if ( adjustScore )
	return (int) QFont::Normal - 2;	   // - 2, we hope it's close to normal

    return (int) QFont::Normal;
}
