/****************************************************************************
** $Id: qslider.h,v 2.24.2.3 1998/08/21 19:13:26 hanord Exp $
**
** Definition of QSlider class
**
** Created : 961019
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

#ifndef QSLIDER_H
#define QSLIDER_H

#ifndef QT_H
#include "qwidget.h"
#include "qrangecontrol.h"
#endif // QT_H


class QTimer;
struct QSliderData;


class Q_EXPORT QSlider : public QWidget, public QRangeControl
{
    Q_OBJECT
public:
    enum Orientation { Horizontal, Vertical };
    enum TickSetting { NoMarks = 0, Above = 1, Left = Above,
		       Below = 2, Right = Below, Both = 3 };

    QSlider( QWidget *parent=0, const char *name=0 );
    QSlider( Orientation, QWidget *parent=0, const char *name=0 );
    QSlider( int minValue, int maxValue, int pageStep, int value, Orientation,
	     QWidget *parent=0, const char *name=0 );

    void	setOrientation( Orientation );
    Orientation orientation() const;
    void	setTracking( bool enable );
    bool	tracking() const;

    void 	setPalette( const QPalette & );
    QRect	sliderRect() const;
    QSize	sizeHint() const;

    virtual void setTickmarks( TickSetting );
    TickSetting tickmarks() const { return ticks; }

    virtual void setTickInterval( int );
    int 	tickInterval() const { return tickInt; }

public slots:
    void	setValue( int );
    void	addStep();
    void	subtractStep();

signals:
    void	valueChanged( int value );
    void	sliderPressed();
    void	sliderMoved( int value );
    void	sliderReleased();

protected:
    void	resizeEvent( QResizeEvent * );
    void	paintEvent( QPaintEvent * );

    void	keyPressEvent( QKeyEvent * );
    void	mousePressEvent( QMouseEvent * );
    void	mouseReleaseEvent( QMouseEvent * );
    void	mouseMoveEvent( QMouseEvent * );
    void	focusInEvent( QFocusEvent *e );

    void	valueChange();
    void	rangeChange();

    virtual void paintSlider( QPainter *, const QRect & );
    void	drawWinGroove( QPainter *, QCOORD );
    void	drawTicks( QPainter *, int, int, int=1 ) const;

    virtual int	thickness() const;

private slots:
    void	repeatTimeout();

private:
    enum State { Idle, Dragging, TimingUp, TimingDown };

    void	init();
    int		positionFromValue( int ) const;
    int		valueFromPosition( int ) const;
    void	moveSlider( int );
    void	reallyMoveSlider( int );
    void	resetState();
    int		slideLength() const;
    int		available() const;
    int		goodPart( const QPoint& ) const;
    void	initTicks();

    QSliderData *extra;
    QTimer	*timer;
    QCOORD	sliderPos;
    int		sliderVal;
    QCOORD	clickOffset;
    State	state;
    bool	track;
    QCOORD	tickOffset;
    TickSetting	ticks;
    int		tickInt;
    Orientation orient;

private:	// Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QSlider( const QSlider & );
    QSlider &operator=( const QSlider & );
#endif
};

inline bool QSlider::tracking() const
{
    return track;
}

inline QSlider::Orientation QSlider::orientation() const
{
    return orient;
}


#endif // QSLIDER_H
