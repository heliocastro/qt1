/****************************************************************************
** $Id: qpixmapcache.cpp,v 2.9 1998/07/03 00:09:37 hanord Exp $
**
** Implementation of QPixmapCache class
**
** Created : 950504
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

#include "qpixmapcache.h"
#include "qcache.h"

/*!
  \class QPixmapCache qpixmapcache.h

  \brief The QPixmapCache class provides an application-global cache for
  pixmaps.

  \ingroup kernel

  This class is a tool for optimized drawing with \link QPixmap
  QPixmaps\endlink.  Here follows an example.  The QRadioButton widget has
  a non-trivial visual representation.	In the \link
  QRadioButton::drawButton() drawButton()\endlink function, we do not draw
  the radio button directly. Instead, we first check the global pixmap
  cache if a pixmap called "$qt_radio_s_" exists. The \c s is a numerical
  value that specifies the the radio button state.  If a pixmap is found,
  we bitBlt() it onto the widget and return. Otherwise, we create a new
  pixmap, draw the radio button in the pixmap and finally insert the
  pixmap in the global pixmap cache, using the key above.  The bitBlt() is
  10 times faster than drawing the radio button.  All radio buttons in the
  program share the cached pixmap since QPixmapCache is
  application-global.

  QPixmapCache contains no member data, only static functions to access
  the global pixmap cache.  It creates an internal QCache for caching the
  pixmaps.

  The cache associates a pixmap with a normal string (key).  If two
  pixmaps are inserted into the cache using equal keys, then the last
  pixmap will hide the first pixmap. The QDict and QCache classes do
  exactly the same.

  The cache becomes full when the total size of all pixmaps in the cache
  exceeds the cache limit.  The initial cache limit is 1024 KByte (1
  MByte).  A pixmap takes roughly width*height*depth/8 bytes of memory.

  See the QCache documentation for a more details about the cache mechanism.
*/

typedef Q_DECLARE(QCacheM,QPixmap) QPMCache;
static QPMCache *pm_cache = 0;			// global pixmap cache
const  int cache_size	  = 61;			// size of internal hash array
static int cache_limit	  = 1024;		// 1024 KB cache limit


/*!
  Returns the pixmap associated with \e key in the cache, or null if there
  is no such pixmap.

  <strong>
    NOTE: if valid, you should copy the pixmap immediately (this is quick
    since QPixmaps are \link shclass.html implicitly shared\endlink), because
    subsequent insertions into the cache could cause the pointer to become
    invalid.  For this reason, we recommend you use
    find(const char*, QPixmap&) instead.
  </strong>

  Example:
  \code
    QPixmap* pp;
    QPixmap p;
    if ( (pp=QPixmapCache::find("my_previous_copy", pm)) ) {
	p = *pp;
    } else {
	p.load("bigimage.gif");
	QPixmapCache::insert("my_previous_copy", new QPixmap(p));
    }
    painter->drawPixmap(0, 0, p);
  \endcode
*/

QPixmap *QPixmapCache::find( const char *key )
{
    return pm_cache ? pm_cache->find(key) : 0;
}


/*!
  Sets \a pm to the cached pixmap associated with \e key in the cache and
  returns TRUE.  If FALSE is returned, no cached copy was found, and
  \a pm is unchanged.

  Example:
  \code
    QPixmap p;
    if ( !QPixmapCache::find("my_previous_copy", pm) ) {
	pm.load("bigimage.gif");
	QPixmapCache::insert("my_previous_copy", pm);
    }
    painter->drawPixmap(0, 0, p);
  \endcode
*/

bool QPixmapCache::find( const char *key, QPixmap& pm )
{
    QPixmap* p = pm_cache ? pm_cache->find(key) : 0;
    if ( p ) pm = *p;
    return !!p;
}

/*!
  Inserts the pixmap \e pm associated with \e key into the cache.
  Returns TRUE if successful, or FALSE if the pixmap is too big for the cache.

  <strong>
    NOTE: If this function returns FALSE, you must delete \a pm yourself.
    Additionally, be very careful about using \a pm after calling this
    function, as any other insertions into the cache, from anywhere in
    the application, or within Qt itself, could cause the pixmap to be
    discarded from the cache, and the pointer to become invalid.

    Due to these dangers, we strongly recommend that you use
    insert(const char*, const QPixmap&) instead.
  </strong>
*/

bool QPixmapCache::insert( const char *key, QPixmap *pm )
{
    if ( !pm_cache ) {				// create pixmap cache
	pm_cache = new QPMCache( 1024*cache_limit, cache_size );
	CHECK_PTR( pm_cache );
	pm_cache->setAutoDelete( TRUE );
    }
    return pm_cache->insert( key, pm, pm->width()*pm->height()*pm->depth()/8 );
}

/*!
  Inserts a copy of the pixmap \e pm associated with \e key into the cache.
  Returns TRUE if successful, or FALSE if the pixmap is too big for the cache.

  All pixmaps inserted by the Qt library have a key starting with "$qt..".
  Use something else for you pixmaps.

  When a pixmap is inserted and the cache is about to exceed its limit, it
  removes pixmaps until there is enough room for the pixmap to be inserted.

  The oldest pixmaps (least recently accessed in the cache) are deleted
  when more space is needed.

  \sa setCacheLimit().
*/

void QPixmapCache::insert( const char *key, const QPixmap& pm )
{
    if ( !pm_cache ) {				// create pixmap cache
	pm_cache = new QPMCache( 1024*cache_limit, cache_size );
	CHECK_PTR( pm_cache );
	pm_cache->setAutoDelete( TRUE );
    }
    QPixmap *p = new QPixmap(pm);
    if ( !pm_cache->insert( key, p, p->width()*p->height()*p->depth()/8 ) )
	delete p;
}

/*!
  Returns the cache limit (in kilobytes).

  The default setting is 1024 kilobytes.

  \sa setCacheLimit().
*/

int QPixmapCache::cacheLimit()
{
    return cache_limit;
}

/*!
  Sets the cache limit to \e n kilobytes.

  The default setting is 1024 kilobytes.

  \sa cacheLimit()
*/

void QPixmapCache::setCacheLimit( int n )
{
    cache_limit = n;
    if ( pm_cache )
	pm_cache->setMaxCost( 1024*cache_limit );
}


/*!
  Removes all pixmaps from the cache.
*/

void QPixmapCache::clear()
{
    delete pm_cache;
    pm_cache = 0;
}
