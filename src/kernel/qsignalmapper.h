/****************************************************************************
** $Id: qsignalmapper.h,v 1.4.2.1 1998/08/19 16:02:33 agulbra Exp $
**
** Definition of QSignalMapper class
**
** Created : 980503
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

#ifndef QSIGNALMAPPER_H
#define QSIGNALMAPPER_H

#ifndef QT_H
#include "qobject.h"
#endif // QT_H


class  QSignalMapperData;
struct QSignalMapperRec;


class Q_EXPORT QSignalMapper : public QObject {
    Q_OBJECT
public:
    QSignalMapper( QObject* parent, const char* name=0 );
    ~QSignalMapper();

    void setMapping( const QObject* sender, int identifier );
    void setMapping( const QObject* sender, const char* identifier );
    void removeMappings( const QObject* sender );

signals:
    void mapped(int);
    void mapped(const char*);

public slots:
    void map();

private:
    QSignalMapperData* d;
    QSignalMapperRec* getRec( const QObject* );

private slots:
    void removeMapping();
};


#endif // QSIGNALMAPPER_H
