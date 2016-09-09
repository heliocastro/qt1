/****************************************************************************
** $Id: qvalidator.h,v 2.11.2.1 1998/08/19 16:02:44 agulbra Exp $
**
** Definition of validator classes.
**
** Created : 970610
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

#ifndef QVALIDATOR_H
#define QVALIDATOR_H

#ifndef QT_H
#include "qobject.h"
#include "qstring.h"
#endif // QT_H


class Q_EXPORT QValidator: public QObject
{
    Q_OBJECT
public:
    QValidator( QWidget * parent, const char * name = 0 );
    ~QValidator();

    enum State { Invalid, Valid, Acceptable };

    virtual State validate( QString &, int & ) = 0;
    virtual void fixup( QString & );
};


class Q_EXPORT QIntValidator: public QValidator
{
    Q_OBJECT
public:
    QIntValidator( QWidget * parent, const char * name = 0 );
    QIntValidator( int bottom, int top,
		   QWidget * parent, const char * name = 0 );
    ~QIntValidator();

    QValidator::State validate( QString &, int & );

    virtual void setRange( int bottom, int top );

    int bottom() const { return b; }
    int top() const { return t; }
    
private:
    int b, t;
};


class Q_EXPORT QDoubleValidator: public QValidator
{
    Q_OBJECT
public:
    QDoubleValidator( QWidget * parent, const char * name = 0 );
    QDoubleValidator( double bottom, double top, int decimals,
		      QWidget * parent, const char * name = 0 );
    ~QDoubleValidator();

    QValidator::State validate( QString &, int & );

    virtual void setRange( double bottom, double top, int decimals = 0 );

    double bottom() const { return b; }
    double top() const { return t; }
    int decimals() const { return d; }
    
private:
    double b, t;
    int d;
};


#endif // QVALIDATOR_H
