/****************************************************************************
** $Id: qdragobject.h,v 2.21.2.2 1998/08/19 16:02:29 agulbra Exp $
**
** Definition of QDragObject
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

#ifndef QDRAGOBJECT_H
#define QDRAGOBJECT_H

struct QDragData;
struct QStoredDragData;
class QWidget;

#ifndef QT_H
#include "qobject.h"
#include "qimage.h"
#include "qstrlist.h"
#endif // QT_H


class Q_EXPORT QDragObject: public QObject {
    Q_OBJECT
public:
    QDragObject( QWidget * dragSource = 0, const char * name = 0 );
    ~QDragObject();

    bool drag();
    bool dragMove();
    void dragCopy();

    virtual bool provides(const char*) const;
    virtual const char * format(int) const=0;
    virtual QByteArray encodedData(const char*) const=0;

    void setPixmap(QPixmap);
    void setPixmap(QPixmap, QPoint hotspot);
    QPixmap pixmap() const;
    QPoint pixmapHotSpot() const;

    QWidget * source();

    enum DragMode { DragDefault, DragCopy, DragMove, DragCopyOrMove };

protected:
    virtual bool drag(DragMode);

private:
    QDragData * d;
};

class Q_EXPORT QStoredDrag: public QDragObject {
    Q_OBJECT
    QStoredDragData * d;

public:
    QStoredDrag( const char * mimeType,
		 QWidget * dragSource = 0, const char * name = 0 );
    ~QStoredDrag();

    void setEncodedData( const QByteArray & );

    const char * format(int i) const;
    virtual QByteArray encodedData(const char*) const;
};

class Q_EXPORT QTextDrag: public QStoredDrag {
    Q_OBJECT
public:
    QTextDrag( const char *,
	       QWidget * dragSource = 0, const char * name = 0 );
    QTextDrag( QWidget * dragSource = 0, const char * name = 0 );
    ~QTextDrag();

    void setText( const char * );

    static bool canDecode( QDragMoveEvent* e );
    static bool decode( QDropEvent* e, QString& s );
};


class Q_EXPORT QImageDrag: public QDragObject {
    Q_OBJECT
    QImage img;
    QStrList ofmts;

public:
    QImageDrag( QImage image,
		QWidget * dragSource = 0, const char * name = 0 );
    QImageDrag( QWidget * dragSource = 0, const char * name = 0 );
    ~QImageDrag();

    void setImage( QImage image );

    const char * format(int i) const;
    virtual QByteArray encodedData(const char*) const;

    static bool canDecode( QDragMoveEvent* e );
    static bool decode( QDropEvent* e, QImage& i );
    static bool decode( QDropEvent* e, QPixmap& i );
};


class Q_EXPORT QUrlDrag: public QStoredDrag {
    Q_OBJECT

public:
    QUrlDrag( QStrList urls,
		QWidget * dragSource = 0, const char * name = 0 );
    QUrlDrag( QWidget * dragSource = 0, const char * name = 0 );
    ~QUrlDrag();

    void setUrls( QStrList urls );

    static QString urlToLocalFile(const char*);
    static bool canDecode( QDragMoveEvent* e );
    static bool decode( QDropEvent* e, QStrList& i );
    static bool decodeLocalFiles( QDropEvent* e, QStrList& i );
};


// QDragManager is not part of the public API.  It is defined in a
// header file simply so different .cpp files can implement different
// member functions.
//

class Q_EXPORT QDragManager: public QObject {
    Q_OBJECT

private:
    QDragManager();
    ~QDragManager();
    // only friend classes can use QDragManager.
    friend class QDragObject;

    bool eventFilter( QObject *, QEvent * );

    bool drag( QDragObject *, QDragObject::DragMode );

    void cancel();
    void move( const QPoint & );
    void drop();
    void updatePixmap();

private:
    QDragObject * object;

    QWidget * dragSource;
    QWidget * dropWidget;
    bool beingCancelled;
    bool restoreCursor;
    bool willDrop;

    QPixmap *pm_cursor;
    int n_cursor;
};


#endif
