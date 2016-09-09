/****************************************************************************
** $Id: qmovie.cpp,v 1.39.2.3 1998/11/13 09:37:03 ettrich Exp $
**
** Implementation of movie classes
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

#include "qtimer.h"
#include "qpainter.h"
#include "qlist.h"
#include "qbitmap.h"
#include "qmovie.h"
#include "qfile.h"
#include "qbuffer.h"
#include "qshared.h"

#include "qasyncio.h"
#include "qasyncimageio.h"

#include <stdlib.h>

/*!
  \class QMovie qmovie.h
  \brief Incrementally loads an animation or image, signalling as it progresses.

  \ingroup images

  A QMovie provides a QPixmap as the framePixmap(), and connections can
  be made via connectResize() and connectUpdate() to receive notification
  of size and pixmap changes.  All decoding is driven by
  the normal event processing mechanisms.  The simplest way to display
  a QMovie, is to use a QLabel and QLabel::setMovie().

  The movie begins playing as soon as the QMovie is created (actually,
  once control returns to the event loop).  When the last frame in the
  movie has been played, it may loop back to the start if such looping
  is defined in the input source.

  QMovie objects are explicitly shared.  This means that a QMovie copied
  from another QMovie will be displaying the same frame at all times.
  If one shared movie pauses, all pause.  To make \e independent movies,
  they must be constructed separately.

  The set of data formats supported by QMovie is determined by the decoder
  factories which have been installed, and the format of the input is
  determined as the input is decoded.

  In Qt 1.30, the decoder factory interface is not available for adding
  support for new formats. Only GIF support is installed.  The GIF decoder
  supports interlaced images, transparency, looping, image-restore
  disposal, local color maps, and background colors.  The Netscape looping
  extension is obeyed.

  Archives of animated GIFs and tools for building them can be found
  at <a href=http://www.yahoo.com/Arts/Visual_Arts/Animation/Computer_Animation/Animated_GIFs/>
  Yahoo!</a>.

  We are required to state: The Graphics Interchange Format(c) is the
  Copyright property of CompuServe Incorporated. GIF(sm) is a Service
  Mark property of CompuServe Incorporated.

  \warning Unisys has changed its position regarding GIF.  If you are
  in a country where Unisys holds a patent on LZW compression and/or
  decompression, Unisys may require a license from you.  These
  countries include Canada, Japan, the USA, France, Germany, Italy and
  the UK.  There is more information on Unisys web site: <a
  href="http://corp2.unisys.com/LeadStory/lzwfaq.html">Overview of
  Unisys' position.</a> GIF support may be removed in a future version
  of Qt.  We recommend using the PNG format, which is available in the
  <a href="imageio.html">Qt Image IO Extension</a> package.

  <img src="qmovie.gif">

  \sa QLabel::setMovie()
*/

class QMoviePrivate : public QObject, public QShared,
		      private QDataSink, private QImageConsumer
{
    Q_OBJECT

public: // for QMovie

    // Creates a null QMoviePrivate
    QMoviePrivate();

    // NOTE:  The ownership of the QDataSource is transferred to the QMoviePrivate
    QMoviePrivate(QDataSource* src, QMovie* movie, int bufsize);

    virtual ~QMoviePrivate();

    bool isNull() const;

    // Initialize, possibly to the null state
    void init(bool fully);
    void flushBuffer();
    void updatePixmapFromImage();
    void showChanges();

    // This as QImageConsumer
    void changed(const QRect& rect);
    void end();
    void frameDone();
    void restartTimer();
    void setLooping(int l);
    void setFramePeriod(int milliseconds);
    void setSize(int w, int h);

    // This as QDataSink
    int readyToReceive();
    void receive(const uchar* b, int bytecount);
    void eof();
    void pause();

signals:
    void sizeChanged(const QSize&);
    void areaChanged(const QRect&);
    void dataStatus(int);

public slots:
    void refresh();

public:
    QMovie *that;

    QImageDecoder *decoder;

    // Cyclic buffer
    int buf_size;
    uchar *buffer;
    int buf_r, buf_w, buf_usage;

    int framenumber;
    int frameperiod;
    int speed;
    QTimer *frametimer;
    int lasttimerinterval;
    int loop;

    bool waitingForFrameTick;
    int stepping;
    QRect changed_area;
    QRect valid_area;
    QDataPump *pump;
    QDataSource *source;
    QPixmap mypixmap;
    QBitmap mymask;
    QColor bg;

    int error;
    bool empty;
};

QMoviePrivate::QMoviePrivate()
{
    buffer = 0;
    pump = 0;
    source = 0;
    decoder = 0;
    init(FALSE);
}

// NOTE:  The ownership of the QDataSource is transferred to the QMoviePrivate
QMoviePrivate::QMoviePrivate(QDataSource* src, QMovie* movie, int bufsize) :
    that(movie),
    buf_size(bufsize)
{
    frametimer = new QTimer(this);
    pump = new QDataPump(src, this);
    QObject::connect(frametimer, SIGNAL(timeout()), this, SLOT(refresh()));
    source = src;
    buffer = 0;
    decoder = 0;
    speed = 100;
    init(TRUE);
}

QMoviePrivate::~QMoviePrivate()
{
    if ( buffer )				// Avoid purify complaint
	delete [] buffer;
    delete pump;
    delete decoder;
    delete source;
}

bool QMoviePrivate::isNull() const
{
    return !pump;
}

// Initialize.  Only actually allocate any space if \a fully is TRUE,
// otherwise, just enough to be a valid null QMoviePrivate.
void QMoviePrivate::init(bool fully)
{
    buf_usage = buf_r = buf_w = 0;
    if ( buffer )				// Avoid purify complaint
	delete [] buffer;
    buffer = fully ? new uchar[buf_size] : 0;

    delete decoder;
    decoder = fully ? new QImageDecoder(this) : 0;

#ifdef AVOID_OPEN_FDS
    if ( source && !source->isOpen() )
	source->open(IO_ReadOnly);
#endif

    waitingForFrameTick = FALSE;
    stepping = -1;
    framenumber = 0;
    frameperiod = -1;
    if (fully) frametimer->stop();
    lasttimerinterval = -1;
    changed_area.setRect(0,0,-1,-1);
    valid_area = changed_area;
    loop = -1;
    error = 0;
    empty = TRUE;
}

void QMoviePrivate::flushBuffer()
{
    while (buf_usage && !waitingForFrameTick && stepping != 0 && !error) {
	int used = decoder->decode(buffer + buf_r,
			QMIN(buf_usage, buf_size - buf_r));
	if (used<=0) {
	    error = 1;
	    emit dataStatus(QMovie::UnrecognizedFormat);
	    break;
	}
	buf_r = (buf_r + used)%buf_size;
	buf_usage -= used;
    }
    if(error) frametimer->stop();
    maybeReady();
}

void QMoviePrivate::updatePixmapFromImage()
{
    if (changed_area.isEmpty()) return;

    // Create temporary QImage to hold the part we want
    const QImage& gimg = decoder->image();
    QImage img(changed_area.size(), 8, gimg.numColors());
    img.setAlphaBuffer(gimg.hasAlphaBuffer());

    // Copy color map.
    memcpy(img.colorTable(), gimg.colorTable(),
	sizeof(QRgb)*gimg.numColors());

    // Copy pixels.
    const int t = changed_area.top();
    const int b = changed_area.bottom();
    const int l = changed_area.left();
    const int w = changed_area.width();
    const int h = changed_area.height();
    for (int y=t; y<=b; y++) {
	memcpy(img.scanLine(y-t), gimg.scanLine(y)+l, w);
    }

    // Resize to size of image
    if (mypixmap.width() != gimg.width() || mypixmap.height() != gimg.height())
	mypixmap.resize(gimg.width(), gimg.height());

    if (bg.isValid()) {
	QPainter p;
	p.begin(&mypixmap);
	p.fillRect(l, t, w, h, bg);
	p.end();
    } else {
	if (gimg.hasAlphaBuffer()) {
	    // Resize to size of image
	    if (mymask.isNull()) {
		mymask.resize(gimg.width(), gimg.height());
		mymask.fill(color1);
	    }
	}
	mypixmap.setMask(QBitmap()); // Remove reference to my mask
    }

    // Convert to pixmap and paste that onto myself
    QPixmap lines;
    lines.convertFromImage(img);
    bitBlt(&mypixmap, l, t, &lines, 0, 0, w, h, CopyROP, !bg.isValid());

    if (!bg.isValid() && gimg.hasAlphaBuffer()) {
	bitBlt(&mymask, l, t, lines.mask(), 0, 0, w, h, CopyROP, TRUE);
	mypixmap.setMask(mymask);
    }
}

void QMoviePrivate::showChanges()
{
    if (changed_area.isValid()) {
	updatePixmapFromImage();

	valid_area = valid_area.unite(changed_area);
	emit areaChanged(changed_area);

	changed_area.setWidth(-1); // make empty
    }
}

// QMoviePrivate as QImageConsumer
void QMoviePrivate::changed(const QRect& rect)
{
    if (!frametimer->isActive()) frametimer->start(0);
    changed_area = changed_area.unite(rect);
}

void QMoviePrivate::end()
{
}

void QMoviePrivate::frameDone()
{
    if (stepping > 0) {
	stepping--;
	if (!stepping) {
	    frametimer->stop();
	    emit dataStatus( QMovie::Paused );
	}
    } else {
	waitingForFrameTick = TRUE;
	restartTimer();
    }
    showChanges();
    emit dataStatus(QMovie::EndOfFrame);
    framenumber++;
}

void QMoviePrivate::restartTimer()
{
    if (speed > 0) {
	int i = frameperiod >= 0 ? frameperiod * 100/speed : 0;
	if ( i != lasttimerinterval || !frametimer->isActive() ) {
	    lasttimerinterval = i;
	    frametimer->start( i );
	}
    } else {
	frametimer->stop();
    }
}

void QMoviePrivate::setLooping(int nloops)
{
    if (loop == -1) { // Only if we don't already know how many loops!
	if (source->rewindable()) {
	    source->enableRewind(TRUE);
	    loop = nloops;
	} else {
	    // Cannot loop from this source
	    loop = -2;
	}
    }
}

void QMoviePrivate::setFramePeriod(int milliseconds)
{
    // Animation:  only show complete frame
    frameperiod = milliseconds;
    if (stepping<0 && frameperiod >= 0) restartTimer();
}

void QMoviePrivate::setSize(int w, int h)
{
    if (mypixmap.width() != w || mypixmap.height() != h) {
	mypixmap.resize(w, h);
	emit sizeChanged(QSize(w, h));
    }
}


// QMoviePrivate as QDataSink

int QMoviePrivate::readyToReceive()
{
    // Could pre-fill buffer, but more efficient to just leave the
    // data back at the source.
    return (waitingForFrameTick || !stepping || buf_usage || error)
	? 0 : buf_size;
}

void QMoviePrivate::receive(const uchar* b, int bytecount)
{
    if ( bytecount ) empty = FALSE;

    while (bytecount && !waitingForFrameTick && stepping != 0) {
	int used = decoder->decode(b, bytecount);
	if (used<=0) {
	    error = 1;
	    emit dataStatus(QMovie::UnrecognizedFormat);
	    break;
	}
	b+=used;
	bytecount-=used;
    }

    // Append unused to buffer
    while (bytecount--) {
	buffer[buf_w] = *b++;
	buf_w = (buf_w+1)%buf_size;
	buf_usage++;
    }
}

void QMoviePrivate::eof()
{
    if ( empty )
	emit dataStatus(QMovie::SourceEmpty);

    emit dataStatus(QMovie::EndOfLoop);

    if (loop >= 0) {
	if (loop) {
	    loop--;
	    if (!loop) return;
	}
	delete decoder;
	decoder = new QImageDecoder(this);
	source->rewind();
	framenumber = 0;
    } else {
	delete decoder;
	decoder = 0;
	if ( buffer )				// Avoid purify complaint
	    delete [] buffer;
	buffer = 0;
	emit dataStatus(QMovie::EndOfMovie);
#ifdef AVOID_OPEN_FDS
	if ( source )
	    source->close();
#endif
    }
}

void QMoviePrivate::pause()
{
    if ( stepping ) {
	stepping = 0;
	frametimer->stop();
	emit dataStatus( QMovie::Paused );
    }
}

void QMoviePrivate::refresh()
{
    if (!decoder) {
	frametimer->stop();
	return;
    }

    if (frameperiod < 0 && loop == -1) {
	// Only show changes if probably not an animation
	showChanges();
    }

    if (!buf_usage) {
	frametimer->stop();
    }

    waitingForFrameTick = FALSE;
    flushBuffer();
}

///////////////// End of QMoviePrivate /////////////////





/*!
  Creates a null QMovie.  The only interesting thing to do to such
  a movie is to assign another movie to it.

  \sa isNull()
*/
QMovie::QMovie()
{
    d = new QMoviePrivate();
}

/*!
  Creates a QMovie which reads an image sequence from the given
  QDataSource.  The source must be allocated dynamically,
  as it becomes owned by the QMovie, and will be destroyed
  when the movie is destroyed.
  The movie starts playing as soon as event processing continues.

  The \a bufsize argument sets the maximum amount of data the movie
  will transfer from the data source per event loop.  The lower this
  value, the better interleaved the movie playback will be with other
  event processing, but the slower the overall processing.
*/
QMovie::QMovie(QDataSource* src, int bufsize)
{
    d = new QMoviePrivate(src, this, bufsize);
}

/*!
  \overload
  Creates a QMovie which reads an image sequence from the named file.
*/
QMovie::QMovie(const char* fileName, int bufsize)
{
    QFile* file = new QFile(fileName);
    file->open(IO_ReadOnly);
    d = new QMoviePrivate(new QIODeviceSource(file), this, bufsize);
}

/*!
  \overload
  Creates a QMovie which reads an image sequence from given data.
*/
QMovie::QMovie(QByteArray data, int bufsize)
{
    QBuffer* buffer = new QBuffer(data);
    buffer->open(IO_ReadOnly);
    d = new QMoviePrivate(new QIODeviceSource(buffer), this, bufsize);
}

/*!
  Constructs a movie that uses the same data as another movie.
  QMovies use explicit sharing, so operations on the copy will
  effect the same operations on the original.
*/
QMovie::QMovie(const QMovie& movie)
{
    d = movie.d;
    d->ref();
}

/*!
  Destroys the QMovie.  If this is the last reference to the data of the
  movie, that will also be destroyed.
*/
QMovie::~QMovie()
{
    if (d->deref()) delete d;
}

/*!
  Returns TRUE if the movie is null.
*/
bool QMovie::isNull() const
{
    return d->isNull();
}

/*!
  Makes this movie use the same data as another movie.
  QMovies use explicit sharing.
*/
QMovie& QMovie::operator=(const QMovie& movie)
{
    movie.d->ref();
    if (d->deref()) delete d;
    d = movie.d;
    return *this;
}


/*!
  Set the background color of the pixmap.  If the background color
  isValid(), the pixmap will never have a mask, as the background
  color will be used in transparent regions of the image.

  \sa backgroundColor()
*/
void QMovie::setBackgroundColor(const QColor& c)
{
    d->bg = c;
}

/*!
  Returns the background color of the movie set by setBackgroundColor().
*/
const QColor& QMovie::backgroundColor() const
{
    return d->bg;
}

/*!
  Returns the area of the pixmap for which pixels have been generated.
*/
const QRect& QMovie::getValidRect() const
{
    return d->valid_area;
}

/*!
  Returns the current frame of the movie.  It is not generally useful to
  keep a copy of this pixmap.  Better to keep a copy of the QMovie and
  get the framePixmap() only when needed for drawing.
*/
const QPixmap& QMovie::framePixmap() const
{
    return d->mypixmap;
}

/*!
  Returns the number of steps remaining after a call to step(), 0 if paused,
  or a negative value if the movie is running normally or is finished.
*/
int QMovie::steps() const
{
    return d->stepping;
}

/*!
  Returns the number of times EndOfFrame has been emitted since the
  start of the current loop of the movie.  Thus, before
  any EndOfFrame has been emitted, the value will be 0,
  within slots processing the first signal, frameNumber() will be 1, and
  so on.
*/
int QMovie::frameNumber() const
{
    return d->framenumber;
}

/*!
  Returns TRUE if the image is paused.
*/
bool QMovie::paused() const
{
    return d->stepping==0;
}

/*!
  Returns TRUE if the image is no longer playing - this happens when all
  loops of all frames is complete.
*/
bool QMovie::finished() const
{
    return !d->decoder;
}

/*!
  Returns TRUE if the image is not single-stepping, not paused,
  and not finished.
*/
bool QMovie::running() const
{
    return d->stepping<0 && d->decoder;
}

/*!
  Pauses the progress of the animation.

  \sa unpause()
*/
void QMovie::pause()
{
    d->pause();
}

/*!
  Unpauses the progress of the animation.

  \sa pause()
*/
void QMovie::unpause()
{
    if ( d->stepping >= 0 ) {
	if (d->isNull()) return;
	d->stepping = -1;
	d->restartTimer();
    }
}

/*!
  Steps forward, showing the given number of frames, then pauses.
*/
void QMovie::step(int steps)
{
    if (d->isNull()) return;

    d->stepping = steps;
    d->frametimer->start(0);
    d->waitingForFrameTick = FALSE; // Full speed ahead!
}

/*!
  Steps forward 1 frame, then pauses.
*/
void QMovie::step()
{
    step(1);
}

/*!
  Rewinds the movie to the beginning.  If the movie has not been paused,
  it begins playing again.
*/
void QMovie::restart()
{
    if (d->isNull()) return;

    if (d->source->rewindable()) {
	d->source->enableRewind(TRUE);
	d->source->rewind();
	int s = d->stepping;
	d->init(TRUE);
	if ( !s ) s = 1; // Don't pause or we'll not get to the FIRST frame
	if (s>0) step(s);
    }
}

/*!
  Returns the speed-up factor of the movie.  The default is 100 percent.
  \sa setSpeed()
*/
int QMovie::speed() const
{
    return d->speed;
}

/*!
  Sets the speed-up factor of the movie.  This is a percentage of the
  speed dictated by the input data format.  The default is 100 percent.
*/
void QMovie::setSpeed(int percent)
{
    int oldspeed = d->speed;

    if ( oldspeed != percent && percent >= 0 ) {
	d->speed = percent;

	// Restart timer only if really needed
	if (d->stepping < 0) {
	    if ( !percent || !oldspeed    // To or from zero
	    || oldspeed*4 / percent > 4   // More than 20% slower
	    || percent*4 / oldspeed > 4   // More than 20% faster
	    )
		d->restartTimer();
	}
    }
}


/*!
  Connects the given member, of type \code void member(const QSize&) \endcode
  such that it is signalled when the movie changes size.

  Note that due to the explicit sharing of QMovie objects, these connections
  persist until they are explicitly disconnected with disconnectResize(), or
  until \e every shared copy of the movie is deleted.
*/
void QMovie::connectResize(QObject* receiver, const char* member)
{
    QObject::connect(d, SIGNAL(sizeChanged(const QSize&)), receiver, member);
}

/*!
  Disconnects the given member, or all members if member is zero,
  previously connected by
  connectResize().
*/
void QMovie::disconnectResize(QObject* receiver, const char* member)
{
    QObject::disconnect(d, SIGNAL(sizeChanged(const QSize&)), receiver, member);
}

/*!
  Connects the given member, of type \code void member(const QRect&) \endcode
  such that it is signalled when an area of the framePixmap() has
  changed since the previous frame.

  Note that due to the explicit sharing of QMovie objects, these connections
  persist until they are explicitly disconnected with disconnectUpdate(), or
  until \e every shared copy of the movie is deleted.
*/
void QMovie::connectUpdate(QObject* receiver, const char* member)
{
    QObject::connect(d, SIGNAL(areaChanged(const QRect&)), receiver, member);
}

/*!
  Disconnects the given member, or all members if member is zero,
  previously connected by
  connectUpdate().
*/
void QMovie::disconnectUpdate(QObject* receiver, const char* member)
{
    QObject::disconnect(d, SIGNAL(areaChanged(const QRect&)), receiver, member);
}

/*!
  Connects the given member, of type \code void member(int) \endcode
  such that it is signalled when the movie changes status.  The status
  code are negative for errors and positive for information, and they
  are currently:

  <ul>
   <li> \c QMovie::SourceEmpty - signalled if the input cannot be read.
   <li> \c QMovie::UnrecognizedFormat - signalled if the input data is unrecognized.
   <li> \c QMovie::Paused - signalled when the movie is paused by a call to paused(),
			or by after \link step() stepping \endlink pauses.
   <li> \c QMovie::EndOfFrame - signalled at end-of-frame, after any update and Paused signals.
   <li> \c QMovie::EndOfLoop - signalled at end-of-loop, after any update signals,
				EndOfFrame, but before EndOfMovie.
   <li> \c QMovie::EndOfMovie - signalled when the movie completes and is not about
				 to loop.
  </ul>

  More status messages may be added in the future, so a general test for
  error would test for negative.

  Note that due to the explicit sharing of QMovie objects, these connections
  persist until they are explicitly disconnected with disconnectStatus(), or
  until \e every shared copy of the movie is deleted.
*/
void QMovie::connectStatus(QObject* receiver, const char* member)
{
    QObject::connect(d, SIGNAL(dataStatus(int)), receiver, member);
}

/*!
  Disconnects the given member, or all members if member is zero,
  previously connected by
  connectStatus().
*/
void QMovie::disconnectStatus(QObject* receiver, const char* member)
{
    QObject::disconnect(d, SIGNAL(dataStatus(int)), receiver, member);
}


/* tmake ignore Q_OBJECT */

//       MANUALLY INCLUDED.  Regenerate in vi with:   !Gmoc %
//       WARNING! All changes made below will be lost
//
/****************************************************************************
** QMoviePrivate meta object code from reading C++ file 'qmovie.cpp'
**
** Created: Mon Aug 31 17:13:23 1998
**      by: The Qt Meta Object Compiler ($Revision: 1.39.2.3 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#if !defined(Q_MOC_OUTPUT_REVISION)
#define Q_MOC_OUTPUT_REVISION 2
#elif Q_MOC_OUTPUT_REVISION != 2
#error "Moc format conflict - please regenerate all moc files"
#endif

#include <qmetaobject.h>


const char *QMoviePrivate::className() const
{
    return "QMoviePrivate";
}

QMetaObject *QMoviePrivate::metaObj = 0;

void QMoviePrivate::initMetaObject()
{
    if ( metaObj )
	return;
    if ( strcmp(QObject::className(), "QObject") != 0 )
	badSuperclassWarning("QMoviePrivate","QObject");
    if ( !QObject::metaObject() )
	QObject::initMetaObject();
    typedef void(QMoviePrivate::*m1_t0)();
    m1_t0 v1_0 = &QMoviePrivate::refresh;
    QMetaData *slot_tbl = new QMetaData[1];
    slot_tbl[0].name = "refresh()";
    slot_tbl[0].ptr = *((QMember*)&v1_0);
    typedef void(QMoviePrivate::*m2_t0)(const QSize&);
    typedef void(QMoviePrivate::*m2_t1)(const QRect&);
    typedef void(QMoviePrivate::*m2_t2)(int);
    m2_t0 v2_0 = &QMoviePrivate::sizeChanged;
    m2_t1 v2_1 = &QMoviePrivate::areaChanged;
    m2_t2 v2_2 = &QMoviePrivate::dataStatus;
    QMetaData *signal_tbl = new QMetaData[3];
    signal_tbl[0].name = "sizeChanged(const QSize&)";
    signal_tbl[1].name = "areaChanged(const QRect&)";
    signal_tbl[2].name = "dataStatus(int)";
    signal_tbl[0].ptr = *((QMember*)&v2_0);
    signal_tbl[1].ptr = *((QMember*)&v2_1);
    signal_tbl[2].ptr = *((QMember*)&v2_2);
    metaObj = new QMetaObject( "QMoviePrivate", "QObject",
	slot_tbl, 1,
	signal_tbl, 3 );
}
#include <qobjectdefs.h>

#if QT_VERSION >= 141
// newer implementation
#include <qsignalslotimp.h>
#else
// for late-model 1.x header files
#if !defined(Q_MOC_CONNECTIONLIST_DECLARED)
#define Q_MOC_CONNECTIONLIST_DECLARED
#include <qlist.h>
Q_DECLARE(QListM,QConnection);
Q_DECLARE(QListIteratorM,QConnection);
#endif
#endif

// SIGNAL sizeChanged
void QMoviePrivate::sizeChanged( const QSize& t0 )
{
    QConnectionList *clist = receivers("sizeChanged(const QSize&)");
    if ( !clist || signalsBlocked() )
	return;
    typedef void (QObject::*RT0)();
    typedef RT0 *PRT0;
    typedef void (QObject::*RT1)(const QSize&);
    typedef RT1 *PRT1;
    RT0 r0;
    RT1 r1;
    QConnectionListIt it(*clist);
    QConnection   *c;
    QSenderObject *object;
    while ( (c=it.current()) ) {
	++it;
	object = (QSenderObject*)c->object();
	object->setSender( this );
	switch ( c->numArgs() ) {
	    case 0:
		r0 = *((PRT0)(c->member()));
		(object->*r0)();
		break;
	    case 1:
		r1 = *((PRT1)(c->member()));
		(object->*r1)(t0);
		break;
	}
    }
}

// SIGNAL areaChanged
void QMoviePrivate::areaChanged( const QRect& t0 )
{
    QConnectionList *clist = receivers("areaChanged(const QRect&)");
    if ( !clist || signalsBlocked() )
	return;
    typedef void (QObject::*RT0)();
    typedef RT0 *PRT0;
    typedef void (QObject::*RT1)(const QRect&);
    typedef RT1 *PRT1;
    RT0 r0;
    RT1 r1;
    QConnectionListIt it(*clist);
    QConnection   *c;
    QSenderObject *object;
    while ( (c=it.current()) ) {
	++it;
	object = (QSenderObject*)c->object();
	object->setSender( this );
	switch ( c->numArgs() ) {
	    case 0:
		r0 = *((PRT0)(c->member()));
		(object->*r0)();
		break;
	    case 1:
		r1 = *((PRT1)(c->member()));
		(object->*r1)(t0);
		break;
	}
    }
}

// SIGNAL dataStatus
void QMoviePrivate::dataStatus( int t0 )
{
    activate_signal( "dataStatus(int)", t0 );
}
