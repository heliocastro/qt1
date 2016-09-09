/****************************************************************************
** $Id: lifedlg.h,v 2.5 1998/05/21 19:24:54 agulbra Exp $
**
** Copyright (C) 1992-1998 Troll Tech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#ifndef LIFEDLG_H
#define LIFEDLG_H

#include <qtimer.h>
#include <qwidget.h>

class QSlider;
class QPushButton;
class QLabel;
class QComboBox;

#include "life.h"


class LifeTimer : public QTimer
{
    Q_OBJECT
public:
    LifeTimer( QWidget *parent );
    enum { MAXSPEED = 1000 };

public slots:
    void	setSpeed( int speed );
    void	pause( bool );

private:
    int		interval;
};


class LifeDialog : public QWidget
{
    Q_OBJECT
public:
    LifeDialog( QWidget *parent = 0, const char *name = 0 );
public slots:
    void	getPattern( int );

protected:
    virtual void resizeEvent( QResizeEvent * e );

private:
    enum { TOPBORDER = 70, SIDEBORDER = 10 };

    LifeWidget	*life;
    QPushButton *qb;
    LifeTimer	*timer;
    QPushButton *pb;
    QComboBox	*cb;
    QLabel	*sp;
    QSlider	*scroll;
};


#endif // LIFEDLG_H
