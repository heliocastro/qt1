/****************************************************************************
** $Id: qdragobject.cpp,v 2.36.2.9 1998/12/07 16:25:10 warwick Exp $
**
** Implementation of Drag and Drop support
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

#include "qdragobject.h"
#include "qapplication.h"
#include "qcursor.h"
#include "qpoint.h"
#include "qwidget.h"
#include "qobjectlist.h"
#include "qobjectdict.h"
#include "qimage.h"
#include "qbuffer.h"
#include <ctype.h>


// both a struct for storing stuff in and a wrapper to avoid polluting
// the name space

struct QDragData {
    QDragData(): autoDelete( TRUE ) {}
    bool autoDelete;
    QPixmap pixmap;
    QPoint hot;
};


struct QStoredDragData {
    QStoredDragData() {}
    QString fmt;
    QByteArray enc;
};


// These pixmaps approximate the images in the Windows User Interface Guidelines.

/* XPM */
static const char * move_xpm[] = {
"11 20 3 1",
".	c None",
" 	c #000000",
"X	c #FFFFFF",
"  .........",
" X ........",
" XX .......",
" XXX ......",
" XXXX .....",
" XXXXX ....",
" XXXXXX ...",
" XXXXXXX ..",
" XXXXXXXX .",
" XXXXXXXXX ",
" XXXXXX    ",
" XXX XX ...",
" XX  XX ...",
" X .. XX ..",
"  ... XX ..",
" ..... XX .",
"...... XX .",
"....... XX ",
"....... XX ",
"........  ."};

/* XPM */
static const char * copy_xpm[] = {
"24 30 3 1",
".	c None",
" 	c #000000",
"X	c #FFFFFF",
"  ......................",
" X .....................",
" XX ....................",
" XXX ...................",
" XXXX ..................",
" XXXXX .................",
" XXXXXX ................",
" XXXXXXX ...............",
" XXXXXXXX ..............",
" XXXXXXXXX .............",
" XXXXXX    .............",
" XXX XX ................",
" XX  XX ................",
" X .. XX ...............",
"  ... XX ...............",
" ..... XX ..............",
"...... XX ..............",
"....... XX .............",
"....... XX .............",
"........  ...           ",
"............. XXXXXXXXX ",
"............. XXXXXXXXX ",
"............. XXXX XXXX ",
"............. XXXX XXXX ",
"............. XX     XX ",
"............. XXXX XXXX ",
"............. XXXX XXXX ",
"............. XXXXXXXXX ",
"............. XXXXXXXXX ",
".............           "};

/* XPM */
static const char * link_xpm[] = {
"24 30 3 1",
".	c None",
" 	c #000000",
"X	c #FFFFFF",
"  ......................",
" X .....................",
" XX ....................",
" XXX ...................",
" XXXX ..................",
" XXXXX .................",
" XXXXXX ................",
" XXXXXXX ...............",
" XXXXXXXX ..............",
" XXXXXXXXX .............",
" XXXXXX     ............",
" XXX XX ................",
" XX  XX ................",
" X .. XX ...............",
"  ... XX ...............",
" ..... XX ..............",
"...... XX ..............",
"....... XX .............",
"....... XX .............",
"........  ...           ",
"............. XXXXXXXXX ",
"............. XXX    XX ",
"............. XXXX   XX ",
"............. XXX    XX ",
"............. XX   X XX ",
"............. XX  XXXXX ",
"............. XX XXXXXX ",
"............. XXX XXXXX ",
"............. XXXXXXXXX ",
".............           "};


// the universe's only drag manager
static QDragManager * manager = 0;


QDragManager::QDragManager()
    : QObject( qApp, "global drag manager" )
{
    n_cursor = 3;
    pm_cursor = new QPixmap[n_cursor];
    pm_cursor[0] = QPixmap(move_xpm);
    pm_cursor[1] = QPixmap(copy_xpm);
    pm_cursor[2] = QPixmap(link_xpm);
    object = 0;
    dragSource = 0;
    dropWidget = 0;
    if ( !manager )
	manager = this;
    beingCancelled = FALSE;
    restoreCursor = FALSE;
    willDrop = FALSE;
}


QDragManager::~QDragManager()
{
    if ( restoreCursor )
	QApplication::restoreOverrideCursor();
    manager = 0;
    delete [] pm_cursor;
}




/*!  Creates a drag object which is a child of \a dragSource and
  named \a name.

  Note that the drag object will be deleted when \a dragSource is.
*/

QDragObject::QDragObject( QWidget * dragSource, const char * name )
    : QObject( dragSource, name )
{
    d = new QDragData();
    if ( !manager && qApp )
	(void)new QDragManager();
}


/*!  Deletes the drag object and frees up the storage used. */

QDragObject::~QDragObject()
{
    d->autoDelete = FALSE; // so cancel() won't delete this object
    if ( manager && manager->object == this )
	manager->cancel();
    delete d;
}

/*!
  Set the pixmap \a pm to display while dragging the object.
  The platform-specific
  implementation will use this in a loose fashion - so provide a small masked
  pixmap, but do not require that the user ever sees it in all its splendour.
  In particular, cursors on Windows 95 are of limited size.

  The \a hotspot is the point on (or off) the pixmap that should be under the
  cursor as it is dragged. It is relative to the top-left pixel of the pixmap.
*/
void QDragObject::setPixmap(QPixmap pm, QPoint hotspot)
{
    d->pixmap = pm;
    d->hot = hotspot;
    if ( manager && manager->object == this )
	manager->updatePixmap();
}

/*!
  \overload
  Uses a hotspot that positions the pixmap below and to the
  right of the mouse pointer.  This allows the user to clearly
  see the point on the window which they are dragging the data onto.
*/
void QDragObject::setPixmap(QPixmap pm)
{
    setPixmap(pm,QPoint(-10,-10));
}

/*!
  Returns the currently set pixmap
  (which \link QPixmap::isNull() isNull()\endlink if none is set).
*/
QPixmap QDragObject::pixmap() const
{
    return d->pixmap;
}

/*!
  Returns the currently set pixmap hitspot.
*/
QPoint QDragObject::pixmapHotSpot() const
{
    return d->hot;
}

/*!
  Starts a drag operation using the contents of this object,
  using DragDefault mode.

  See drag(DragMove) for important further information.
*/
bool QDragObject::drag()
{
    if ( manager )
	return manager->drag( this, DragDefault );
    else
	return FALSE;
}


/*!
  Starts a drag operation using the contents of this object,
  using DragMove mode.

  See drag(DragMove) for important further information.
*/
bool QDragObject::dragMove()
{
    if ( manager )
	return manager->drag( this, DragMove );
    else
	return FALSE;
}


/*!
  Starts a drag operation using the contents of this object,
  using DragCopy mode.

  See drag(DragMove) for important further information.
*/
void QDragObject::dragCopy()
{
    if ( manager )
	manager->drag( this, DragCopy );
}


/*!
  Starts a drag operation using the contents of this object.

  At this point, the object becomes owned by Qt, not the
  source widget.  You should not delete the drag object nor
  anything it references (unless you make provisions in
  the drag object to \link QObject::destroyed() deal with
  such deletion\enlink).  Many drag objects, such as QTextDrag
  do not reference any other objects.  The actual transfer of data to
  the target application may be done during future event
  processing - after that time the drag object will be deleted.

  Returns TRUE if the dragged data was dragged as a \e move,
  indicating that the caller should remove the original source
  of the data (the drag object must continue to have a copy).

  \define DragMode

  The \a mode is one of:

  <ul>
   <li>\c DragDefault - the mode is determined heuristically.
   <li>\c DragCopy - the data is copied, never moved.
   <li>\c DragMove - the data is moved, if dragged at all.
   <li>\c DragCopyOrMove - the user chooses the mode
	    by using control key to switch from the default.
  </ul>

  Normally one of simpler drag(), dragMove(), or dragCopy() functions
  would be used instead.
*/
bool QDragObject::drag(DragMode mode)
{
    if ( manager )
	return manager->drag( this, mode );
    else
	return FALSE;
}




/*!
  \fn QByteArray QDragObject::encodedData(const char*) const

  Returns the encoded payload of this object.  The drag manager
  calls this when the recipient needs to see the content of the drag;
  this generally doesn't happen until the actual drop.

  Subclasses must override this function.
*/



/*!
  Returns TRUE if the drag object can provide the data
  in format \a mimeType.  The default implementation
  iterates over format().
*/
bool QDragObject::provides(const char* mimeType) const
{
    const char* fmt;
    for (int i=0; (fmt = format(i)); i++) {
	if ( !qstricmp(mimeType,fmt) )
	    return TRUE;
    }
    return FALSE;
}


/*!
  \fn const char * QDragObject::format(int i) const

  Returns the \e ith format, or NULL.
*/


/*!  Returns a pointer to the drag source where this object originated.
*/

QWidget * QDragObject::source()
{
    if ( parent()->isWidgetType() )
	return (QWidget *)parent();
    else
	return 0;
}


/*! \class QDragObject qdragobject.h

  \brief The QDragObject encapsulates MIME-based drag-and-drop
  interaction.

  \ingroup kernel

  Drag-and-drop in Qt uses the MIME open standard, to allow
  independently developers applications to exchange information.

  To start a drag, for example in a \link QWidget::mouseMoveEvent
  mouse motion event\endlink, create an object of the
  QDragObject subclass appropriate for your media, such as
  QTextDrag for text and QImageDrag for images. Then call
  the drag() method. This is all you need for simple dragging
  of existing types.

  To be able to receive media dropped on a widget, multiply-inherit
  the QDropSite class and override the
  \link QDropSite::dragEnterEvent() dragEnterEvent()\endlink,
  \link QDropSite::dragMoveEvent() dragMoveEvent()\endlink,
  \link QDropSite::dragLeaveEvent() dragLeaveEvent()\endlink, and
  \link QDropSite::dropEvent() dropEvent()\endlink event handler methods.

  Support for specific media types is provided by subclasses of
  QDragObject.
  For example, QTextDrag provides
  support for the "<tt>text/plain</tt>" MIME type (ordinary unformated
  text), QImageDrag provides for "<tt>image/</tt><tt>*</tt>",
  where <tt>*</tt>
  is all the \link QImageIO image formats that Qt supports\endlink,
  and the QUrlDrag subclass provides "<tt>url/url</tt>",
  a standard format for transfering a list of filenames.

  To implement drag-and-drop of some type of media for which there
  is no available QDragObject subclass, the
  first and most important step is to look for existing formats
  that are appropriate - the Internet Assigned Numbers Authority
  (<a href=http://www.iana.org>IANA</a>) provides a
  <a href=http://www.isi.edu/in-notes/iana/assignments/media-types/>
  hierarchical list of MIME media types</a>
  at the Information Sciences Institute
  (<a href=http://www.isi.edu>ISI</a>).
  This maximizes inter-operability of your software.

  To support an additional media type, subclass either QDragObject
  or QStoredDrag. Subclass QDragObject when you need to provide
  support for multiple media types. Subclass the simpler QStoredDrag
  when one type is sufficient.

  Subclasses of QDragObject will override the format() and
  encodedData() members, and provide a set-method to encode
  the media data and static members canDecode()
  and decode() to decode incoming data.  QImageDrag
  is an example of such a class in Qt. Of course, you can
  provide drag-only or drop-only support for a media type
  by omitting some of these methods.

  Subclasses of QStoredDrag provide a set-method to encode
  the media data and static members canDecode()
  and decode() to decode incoming data.  QTextDrag
  in Qt 1.x is an example of such a class in Qt.

  <h3>Inter-operating with existing applications</h3>
  On X11, the public 
  <a href=http://www.cco.caltech.edu/~jafl/xdnd/>XDND protocol</a>
  is used, while on Windows Qt uses the OLE standard.  On X11,
  XDND uses MIME, so no translation is necessary.  On Windows, 
  MIME-aware applications can communicate by using clipboard
  format names that are MIME types. Internally, Qt has facilities
  for translating all proprietary clipboard formats to and from
  MIME.  This interface will be made public at some time, but
  if you need to do such translations now, contact your Qt
  Technical Support service.
*/

/*! \class QTextDrag qdragobject.h

  \brief The QTextDrag provides a drag-and-drop object for
  transferring plain text.

  \ingroup kernel

  Plain text is defined as single- or multi-line US-ASCII or an
  unspecified 8-bit character set.

  Qt provides no built-in mechanism for delivering only single-line
  or only US-ASCII text.

  Drag&Drop text does \e not have a NUL terminator when it
  is dropped onto the target.

  For detailed information about drag-and-drop, see the QDragObject class.
*/


/*!  Creates a text drag object and sets it to \a text.  \a dragSource
  must be the drag source, \a name is the object name. */

QTextDrag::QTextDrag( const char * text,
		      QWidget * dragSource, const char * name )
    : QStoredDrag( "text/plain", dragSource, name )
{
    setText( text );
}


/*!  Creates a default text drag object.  \a dragSource must be the drag
  source, \a name is the object name.
*/

QTextDrag::QTextDrag( QWidget * dragSource, const char * name )
    : QStoredDrag( "text/plain", dragSource, name )
{
}


/*!  Destroys the text drag object and frees all allocated resources.
*/

QTextDrag::~QTextDrag()
{
    // nothing
}


/*!
  Sets the text to be dragged.  You will need to call this if you did
  not pass the text during construction.
*/
void QTextDrag::setText( const char * text )
{
    int l = qstrlen(text);
    QByteArray tmp(l);
    memcpy(tmp.data(),text,l);
    setEncodedData( tmp );
}

/*!
  Returns TRUE if the information in \a e can be decoded into a QString.
  \sa decode()
*/
bool QTextDrag::canDecode( QDragMoveEvent* e )
{
    return e->provides( "text/plain" );
}

/*!
  Attempts to decode the dropped information in \a e
  into \a str, returning TRUE if successful.

  \sa decode()
*/
bool QTextDrag::decode( QDropEvent* e, QString& str )
{
    QByteArray payload = e->data( "text/plain" );
    if ( payload.size() ) {
	e->accept();
	str = QString( payload.size()+1 );
	memcpy( str.data(), payload.data(), payload.size() );
	str[(int)payload.size()] = '\0';
	return TRUE;
    }
    return FALSE;
}


/*! \class QImageDrag qdragobject.h

  \brief The QImageDrag provides a drag-and-drop object for
  tranferring images.

  \ingroup kernel

  Images are offered to the receiving application in multiple formats,
  determined by the \link QImage::outputFormats() output formats\endlink
  in Qt.

  For detailed information about drag-and-drop, see the QDragObject class.
*/


/*!  Creates an image drag object and sets it to \a image.  \a dragSource
  must be the drag source, \a name is the object name. */

QImageDrag::QImageDrag( QImage image,
			QWidget * dragSource, const char * name )
    : QDragObject( dragSource, name )
{
    setImage( image );
}

/*!  Creates a default text drag object.  \a dragSource must be the drag
  source, \a name is the object name.
*/

QImageDrag::QImageDrag( QWidget * dragSource, const char * name )
    : QDragObject( dragSource, name )
{
}


/*!  Destroys the image drag object and frees all allocated resources.
*/

QImageDrag::~QImageDrag()
{
    // nothing
}


/*!
  Sets the image to be dragged.  You will need to call this if you did
  not pass the image during construction.
*/
void QImageDrag::setImage( QImage image )
{
    img = image;
    // ### should detach?
    ofmts = QImage::outputFormats();
    ofmts.remove("PBM");
    if ( image.depth()!=32 ) {
	// BMP better than PPM for paletted images
	if ( ofmts.remove("BMP") ) // move to front
	    ofmts.insert(0,"BMP");
    }
    // Could do more magic to order mime types
}

const char * QImageDrag::format(int i) const
{
    if ( i < (int)ofmts.count() ) {
	static QString str;
	str.sprintf("image/%s",(((QImageDrag*)this)->ofmts).at(i));
	str = str.lower();
	if ( str == "image/pbmraw" )
	    str = "image/ppm";
	return str;
    } else {
	return 0;
    }
}

QByteArray QImageDrag::encodedData(const char* fmt) const
{
    if ( qstrnicmp( fmt, "image/", 6 )==0 ) {
	QString f = fmt+6;
	QByteArray data;
	QBuffer w( data );
	w.open( IO_WriteOnly );
	QImageIO io( &w, f.upper() );
	io.setImage( img );
	if  ( !io.write() )
	    return QByteArray();
	w.close();
	return data;
    } else {
	return QByteArray();
    }
}

/*!
  Returns TRUE if the information in \a e can be decoded into an image.
  \sa decode()
*/
bool QImageDrag::canDecode( QDragMoveEvent* e )
{
    return e->provides( "image/bmp" )
        || e->provides( "image/ppm" )
        || e->provides( "image/gif" );
    // ### more Qt images types
}

/*!
  Attempts to decode the dropped information in \a e
  into \a img, returning TRUE if successful.

  \sa canDecode()
*/
bool QImageDrag::decode( QDropEvent* e, QImage& img )
{
    QByteArray payload = e->data( "image/bmp" );
    if ( payload.isEmpty() )
	payload = e->data( "image/ppm" );
    if ( payload.isEmpty() )
	payload = e->data( "image/gif" );
    // ### more Qt images types
    if ( payload.isEmpty() )
	return FALSE;

    img.loadFromData(payload);
    return !img.isNull();
}

/*!
  Attempts to decode the dropped information in \a e
  into \a pm, returning TRUE if successful.

  This is a convenience function that converts
  to \a pm via a QImage.

  \sa canDecode()
*/
bool QImageDrag::decode( QDropEvent* e, QPixmap& pm )
{
    QImage img;
    if ( decode( e, img ) )
	return pm.convertFromImage( img ); 
    return FALSE;
}




/*!
  \class QStoredDrag qdragobject.h
  \brief Simple stored-value drag object for arbitrary MIME data.

  When a block of data only has one representation, you can use
  a QStoredDrag to hold it.

  For detailed information about drag-and-drop, see the QDragObject class.
*/

/*!
  Constructs a QStoredDrag.  The parameters are passed
  to the QDragObject constructor, and the format is set to \a mimeType.

  The data will be unset.  Use setEncodedData() to set it.
*/
QStoredDrag::QStoredDrag( const char* mimeType, QWidget * dragSource, const char * name ) :
    QDragObject(dragSource,name)
{
    d = new QStoredDragData();
    d->fmt = mimeType;
}

/*!
  Destroys the drag object and frees all allocated resources.
*/
QStoredDrag::~QStoredDrag()
{
    delete d;
}

const char * QStoredDrag::format(int i) const
{
    if ( i==0 )
	return d->fmt;
    else
	return 0;
}


/*!
  Sets the encoded data of this drag object to \a encodedData.  The
  encoded data is what's delivered to the drop sites, and must be in a
  strictly defined and portable format.

  The drag object can't be dropped (by the user) until this function
  has been called.
*/

void QStoredDrag::setEncodedData( const QByteArray & encodedData )
{
    d->enc = encodedData.copy();
}

/*!
  Returns the stored data.

  \sa setEncodedData()
*/
QByteArray QStoredDrag::encodedData(const char* m) const
{
    if ( m == d->fmt )
	return d->enc;
    else
	return QByteArray();
}


/*!
  \class QUrlDrag qdragobject.h
  \brief Provides for drag-and-drop of a list of file references.

  URLs are a useful way to refer to files that may be distributed
  across multiple machines.  Much of the time a URL will refer to
  a file on a machine local to both the drag source and the
  drop target, and so the URL will be equivalent to passing a
  filename, but more extensible.
*/

/*!
  Creates an object to drag the list of urls in \a urls.
  The \a dragSource and \a name arguments are passed on to
  QStoredDrag.
*/
QUrlDrag::QUrlDrag( QStrList urls,
	    QWidget * dragSource, const char * name ) :
    QStoredDrag( "url/url", dragSource, name )
{
    setUrls(urls);
}

/*!
  Creates a object to drag.  You will need to call
  setUrls() before you start the drag().
*/
QUrlDrag::QUrlDrag( QWidget * dragSource, const char * name ) :
    QStoredDrag( "url/url", dragSource, name )
{
}

/*!
  Destroys the object.
*/
QUrlDrag::~QUrlDrag()
{
}

/*!
  Changes the list of \a urls to be dragged.
*/
void QUrlDrag::setUrls( QStrList urls )
{
    QByteArray a;
    int c=0;
    for (const char* s = urls.first(); s; s = urls.next() ) {
	int l = strlen(s)+1;
	a.resize(c+l);
	memcpy(a.data()+c,s,l);
	c+=l;
    }
    a.resize(c-1); // chop off last nul
    setEncodedData(a);
}


/*!
  Returns TRUE if decode() would be able to decode \a e.
*/
bool QUrlDrag::canDecode( QDragMoveEvent* e )
{
    return e->provides( "url/url" );
}

/*!
  Decodes URLs from \a e, placing the result in \a l (which is first cleared).

  Returns TRUE if the event contained a valid list of URLs.
*/
bool QUrlDrag::decode( QDropEvent* e, QStrList& l )
{
    QByteArray payload = e->data( "url/url" );
    if ( payload.size() ) {
	e->accept();
	l.clear();
	l.setAutoDelete(TRUE);
	uint c=0;
	char* d = payload.data();
	while (c < payload.size()) {
	    uint f = c;
	    while (c < payload.size() && d[c])
		c++;
	    if ( c < payload.size() ) {
		l.append( d+f );
		c++;
	    } else {
		QString s(d+f,c-f+1);
		l.append( s );
	    }
	}
	return TRUE;
    }
    return FALSE;
}

static
int htod(int h)
{
    if (isdigit(h)) return h-'0';
    return tolower(h)-'a';
}

/*!
  Returns the name of a local file equivalent to \a url,
  or a null string if \a url is not a local file.
*/
QString QUrlDrag::urlToLocalFile(const char* url)
{
    QString result;

    if ( url && 0==qstrnicmp(url,"file:/",6) ) {
	url += 6;
	if ( url[0] != '/' || url[1] == '/' ) {
	    // It is local.
	    while (*url) {
		switch (*url) {
		  case '|': result += ':';
		    break;
		  case '+': result += ' ';
		    break;
		  case '%': {
			int ch = url[1];
			if ( ch && url[2] ) {
			    ch = htod(ch)*16 + htod(url[2]);
			    result += ch;
			}
		    }
		    break;
		  default:
		    result += *url;
		}
		++url;
	    }
	}
    }

    return result;
}

/*!
  Decodes URLs from \a e, converts them to local files if they refer to
  local files, and places them in \a l (which is first cleared).

  Returns TRUE if the event contained a valid list of URLs.
  The list will be empty if no URLs were local files.
*/
bool QUrlDrag::decodeLocalFiles( QDropEvent* e, QStrList& l )
{
    QStrList u;
    if ( !decode( e, u ) )
	return FALSE;

    l.clear();
    l.setAutoDelete(TRUE);
    for (const char* s=u.first(); s; s=u.next()) {
	QString lf = urlToLocalFile(s);
	if ( !lf.isNull() )
	    l.append( lf );
    }
    return TRUE;
}
