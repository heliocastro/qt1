/****************************************************************************
** $Id: qasyncio.h,v 1.5.2.1 1998/08/19 16:02:28 agulbra Exp $
**
**		      ***   INTERNAL HEADER FILE   ***
**
**		This file is NOT a part of the Qt interface!
**
** Definition of asynchronous I/O classes
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

#ifndef QASYNCIO_H
#define QASYNCIO_H

#ifndef QT_H
#include "qobject.h"
#include "qsignal.h"
#include "qtimer.h"
#endif // QT_H

class QIODevice;

class Q_EXPORT QAsyncIO {
public:
    virtual ~QAsyncIO();
    void connect(QObject*, const char* member);

protected:
    void ready();

private:
    QSignal signal;
};

class Q_EXPORT QDataSink : public QAsyncIO {
public:
    // Call this to know how much I can take.
    virtual int readyToReceive()=0;
    virtual void receive(const uchar*, int count)=0;
    virtual void eof()=0;
    void maybeReady();
};

class Q_EXPORT QDataSource : public QAsyncIO {
public:
    virtual int readyToSend()=0; // returns -1 when never any more ready
    virtual void sendTo(QDataSink*, int count)=0;
    void maybeReady();

    virtual bool rewindable() const;
    virtual void enableRewind(bool);
    virtual void rewind();
};

class Q_EXPORT QIODeviceSource : public QDataSource {
    const int buf_size;
    uchar *buffer;
    QIODevice* iod;
    bool rew;

public:
    QIODeviceSource(QIODevice*, int bufsize=4096);
   ~QIODeviceSource();

    int readyToSend();
    void sendTo(QDataSink* sink, int n);
    bool rewindable() const;
    void enableRewind(bool on);
    void rewind();
};

class Q_EXPORT QDataPump : public QObject {
    Q_OBJECT
    int interval;
    QTimer timer;
    QDataSource* source;
    QDataSink* sink;

public:
    QDataPump(QDataSource*, QDataSink*);

private slots:
    void kickStart();
    void tryToPump();
};

#endif
