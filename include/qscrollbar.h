/****************************************************************************
** $Id: qscrollbar.h,v 2.7.2.2 1998/08/21 19:13:26 hanord Exp $
**
** Definition of QScrollBar class
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

#ifndef QSCROLLBAR_H
#define QSCROLLBAR_H

#ifndef QT_H
#include "qwidget.h"
#include "qrangecontrol.h"
#include "qdrawutil.h"
#endif // QT_H

class Q_EXPORT QScrollBar : public QWidget, public QRangeControl
{
    Q_OBJECT
public:
    enum Orientation { Horizontal, Vertical };

    QScrollBar( QWidget *parent=0, const char *name=0 );
    QScrollBar( Orientation, QWidget *parent=0, const char *name=0 );
    QScrollBar( int minValue, int maxValue, int LineStep, int PageStep,
		int value, Orientation,
		QWidget *parent=0, const char *name=0 );

    void	setOrientation( Orientation );
    Orientation orientation() const;
    void	setTracking( bool enable );
    bool	tracking() const;

    bool	draggingSlider() const;

    void	setPalette( const QPalette & );
    QSize	sizeHint() const;

signals:
    void	valueChanged( int value );
    void	sliderPressed();
    void	sliderMoved( int value );
    void	sliderReleased();
    void	nextLine();
    void	prevLine();
    void	nextPage();
    void	prevPage();

protected:
    void	timerEvent( QTimerEvent * );
    void	keyPressEvent( QKeyEvent * );
    void	resizeEvent( QResizeEvent * );
    void	paintEvent( QPaintEvent * );

    void	mousePressEvent( QMouseEvent * );
    void	mouseReleaseEvent( QMouseEvent * );
    void	mouseMoveEvent( QMouseEvent * );

    void	valueChange();
    void	stepChange();
    void	rangeChange();

    int		sliderStart() const;
    QRect	sliderRect() const;

private:
    void	init();
    void	positionSliderFromValue();
    int		calculateValueFromSlider() const;

    uint	pressedControl	 : 8;
    uint	track		 : 1;
    uint	clickedAt	 : 1;
    uint	orient		 : 1;
    uint	thresholdReached : 1;
    uint	isTiming	 : 1;

    int		slidePrevVal;
    QCOORD	sliderPos;
    QCOORD	clickOffset;

private:	// Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QScrollBar( const QScrollBar & );
    QScrollBar &operator=( const QScrollBar & );
#endif
};


inline void QScrollBar::setTracking( bool t )
{
    track = t;
}

inline bool QScrollBar::tracking() const
{
    return track;
}

inline QScrollBar::Orientation QScrollBar::orientation() const
{
    return (Orientation)orient;
}

inline int QScrollBar::sliderStart() const
{
    return sliderPos;
}


void qDrawArrow( QPainter *, ArrowType type, GUIStyle style, bool down,
		 int x, int y, int w, int h, const QColorGroup & );


#endif // QSCROLLBAR_H
