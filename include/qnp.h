/****************************************************************************
** $Id: qnp.h,v 1.13 1998/07/03 00:09:26 hanord Exp $
**
** Definition of Qt extension classes for Netscape Plugin support.
**
** Created : 970601
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

#ifndef QNP_H
#define QNP_H

#ifndef QT_H
#include "qwidget.h"
#endif // QT_H


struct _NPInstance;
struct _NPStream;
class QNPInstance;

class QNPStream {
public:
    ~QNPStream();

    const char* url() const;
    uint end() const;
    uint lastModified() const;

    const char* type() const;
    bool seekable() const;
    bool okay() const;
    bool complete() const;

    void requestRead(int offset, uint length);
    int write( int len, void* buffer );

    QNPInstance* instance() { return inst; }
    QNPStream(QNPInstance*,const char*,_NPStream*,bool);
    void setOkay(bool);
    void setComplete(bool);

private:
    QNPInstance* inst;
    _NPStream* stream;
    QString mtype;
    int seek:1;
    int isokay:1;
    int iscomplete:1;
};

class QNPWidget : public QWidget {
    Q_OBJECT
public:
    QNPWidget();
    ~QNPWidget();

    void setWindow(bool);
    void unsetWindow();

    virtual void enterInstance();
    virtual void leaveInstance();

    QNPInstance* instance();

private:
    WId saveWId;
    _NPInstance* pi;
};

class QNPInstance : public QObject {
    Q_OBJECT
public:
    ~QNPInstance();

    // Arguments passed to EMBED
    int argc() const;
    const char* argn(int) const;
    const char* argv(int) const;
    const char* arg(const char* name) const;
    enum InstanceMode { Embed=1, Full=2, Background=3 };
    InstanceMode mode() const;

    // The browser's name
    const char* userAgent() const;

    // Your window.
    virtual QNPWidget* newWindow();
    QNPWidget* widget();

    // Incoming streams (SRC=... tag).
    // Defaults ignore data.
    enum StreamMode { Normal=1, Seek=2, AsFile=3, AsFileOnly=4 };
    virtual bool newStreamCreated(QNPStream*, StreamMode& smode);
    virtual int writeReady(QNPStream*);
    virtual int write(QNPStream*, int offset, int len, void* buffer);
    virtual void streamDestroyed(QNPStream*);

    void status(const char* msg);

    void getURL(const char* url, const char* window=0);
    void postURL(const char* url, const char* window,
	     uint len, const char* buf, bool file);

    QNPStream* newStream(const char* mimetype, const char* window,
	bool as_file=FALSE);
    virtual void streamAsFile(QNPStream*, const char* fname);

    void* getJavaPeer() const;

    virtual bool printFullPage();
    virtual void print(QPainter*);

protected:
    QNPInstance();

private:
    friend QNPStream;
    _NPInstance* pi;
};


class QNPlugin {
public:
    // Write this to return your QNPlugin derived class.
    static QNPlugin* create();

    static QNPlugin* actual();

    virtual ~QNPlugin();

    void getVersionInfo(int& plugin_major, int& plugin_minor,
	     int& browser_major, int& browser_minor);

    virtual QNPInstance* newInstance()=0;
    virtual const char* getMIMEDescription() const=0;
    virtual const char* getPluginNameString() const=0;
    virtual const char* getPluginDescriptionString() const=0;

    virtual void* getJavaClass();
    virtual void unuseJavaClass();
    void* getJavaEnv() const;

protected:
    QNPlugin();
};


#endif  // QNP_H
