/****************************************************************************
** $Id: qlcdnumber.h,v 2.9.2.2 1998/08/21 19:13:25 hanord Exp $
**
** Definition of QLCDNumber class
**
** Created : 940518
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

#ifndef QLCDNUMBER_H
#define QLCDNUMBER_H

#ifndef QT_H
#include "qframe.h"
#include "qbitarray.h"
#endif // QT_H


class Q_EXPORT QLCDNumber : public QFrame		// LCD number widget
{
    Q_OBJECT
public:
    QLCDNumber( QWidget *parent=0, const char *name=0 );
    QLCDNumber( uint numDigits, QWidget *parent=0, const char *name=0 );
   ~QLCDNumber();

    enum Mode { HEX, DEC, OCT, BIN };
    enum SegmentStyle { Outline, Filled, Flat };

    bool    smallDecimalPoint() const;

    int	    numDigits() const;
    void    setNumDigits( int nDigits );

    bool    checkOverflow( double num ) const;
    bool    checkOverflow( int	  num ) const;

    Mode mode() const;
    void    setMode( Mode );

    SegmentStyle segmentStyle() const;
    void    setSegmentStyle( SegmentStyle );

    double  value() const;
    int	    intValue() const;
    
    QSize sizeHint() const;

public slots:
    void    display( int num );
    void    display( double num );
    void    display( const char *str );
    void    setHexMode();
    void    setDecMode();
    void    setOctMode();
    void    setBinMode();
    void    setSmallDecimalPoint( bool );

signals:
    void    overflow();

protected:
    void    resizeEvent( QResizeEvent * );
    void    drawContents( QPainter * );

private:
    void    init();
    void    internalDisplay( const char * );
    void    drawString( const char *, QPainter &, QBitArray * = 0,
			bool = TRUE );
    void    drawDigit( const QPoint &, QPainter &, int, char, char = ' ' );
    void    drawSegment( const QPoint &, char, QPainter &, int, bool = FALSE );

    int	    ndigits;
    double  val;
    uint    base	: 2;
    uint    smallPoint	: 1;
    uint    fill	: 1;
    uint    shadow	: 1;
    QString digitStr;
    QBitArray points;

private:	// Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QLCDNumber( const QLCDNumber & );
    QLCDNumber &operator=( const QLCDNumber & );
#endif
};

inline bool QLCDNumber::smallDecimalPoint() const
{ return (bool)smallPoint; }

inline int QLCDNumber::numDigits() const
{ return ndigits; }


#endif // QLCDNUMBER_H
