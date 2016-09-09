/****************************************************************************
** $Id: qmovie.h,v 1.12.2.1 1998/08/19 16:02:30 agulbra Exp $
**
** Definition of movie classes
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

#ifndef QMOVIE_H
#define QMOVIE_H

#ifndef QT_H
#include "qobject.h"
#include "qpixmap.h"
#endif // QT_H

class QDataSource;
class QMoviePrivate;

class Q_EXPORT QMovie {
public:
    QMovie();
    QMovie(QDataSource*, int bufsize=1024);
    QMovie(const char* fileName, int bufsize=1024);
    QMovie(QByteArray data, int bufsize=1024);
    QMovie(const QMovie&);
    ~QMovie();

    QMovie& operator=(const QMovie&);

    const QColor& backgroundColor() const;
    void setBackgroundColor(const QColor&);

    const QRect& getValidRect() const;
    const QPixmap& framePixmap() const;

    bool isNull() const;

    int  frameNumber() const;
    int  steps() const;
    bool paused() const;
    bool finished() const;
    bool running() const;

    void unpause();
    void pause();
    void step();
    void step(int);
    void restart();

    int  speed() const;
    void setSpeed(int);

    void connectResize(QObject* receiver, const char* member);
    void disconnectResize(QObject* receiver, const char* member);

    void connectUpdate(QObject* receiver, const char* member);
    void disconnectUpdate(QObject* receiver, const char* member);

    enum Status { SourceEmpty=-2,
	          UnrecognizedFormat=-1,
	          Paused=1,
	          EndOfFrame=2,
	          EndOfLoop=3,
	          EndOfMovie=4,
	          SpeedChanged=5 };
    void connectStatus(QObject* receiver, const char* member);
    void disconnectStatus(QObject* receiver, const char* member=0);

private:
    friend class QMoviePrivate;
    QMoviePrivate *d;
};

#endif
