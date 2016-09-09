/****************************************************************************
** $Id: qrangecontrol.h,v 2.4.2.3 1998/08/21 19:13:26 hanord Exp $
**
** Definition of QRangeControl class
**
** Created : 940427
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

#ifndef QRANGECONTROL_H
#define QRANGECONTROL_H

#include "qglobal.h"

class Q_EXPORT QRangeControl
{
public:
    QRangeControl();
    QRangeControl( int minValue, int maxValue,
		   int lineStep, int pageStep, int value );

    int		value()		const;
    void	setValue( int );
    void	addPage();
    void	subtractPage();
    void	addLine();
    void	subtractLine();

    int		minValue()	const;
    int		maxValue()	const;
    void	setRange( int minValue, int maxValue );

    int		lineStep()	const;
    int		pageStep()	const;
    void	setSteps( int line, int page );

protected:
    void	directSetValue( int val );
    int		prevValue()	const;

    virtual void valueChange();
    virtual void rangeChange();
    virtual void stepChange();

private:
    void	adjustValue();

    int		minVal, maxVal;
    int		line, page;
    int		val, prevVal;

private:	// Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QRangeControl( const QRangeControl & );
    QRangeControl &operator=( const QRangeControl & );
#endif
};


inline int QRangeControl::value() const
{ return val; }

inline int QRangeControl::prevValue() const
{ return prevVal; }

inline int QRangeControl::minValue() const
{ return minVal; }

inline int QRangeControl::maxValue() const
{ return maxVal; }

inline int QRangeControl::lineStep() const
{ return line; }

inline int QRangeControl::pageStep() const
{ return page; }


#endif // QRANGECONTROL_H
