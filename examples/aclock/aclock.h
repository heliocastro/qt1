/****************************************************************************
** $Id: aclock.h,v 2.3 1998/06/16 11:39:31 warwick Exp $
**
** Copyright (C) 1992-1998 Troll Tech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#ifndef ACLOCK_H
#define ACLOCK_H

#include <qwidget.h>
#include <qdatetime.h>


class AnalogClock : public QWidget		// analog clock widget
{
    Q_OBJECT
public:
    AnalogClock( QWidget *parent=0, const char *name=0 );

protected:
    void	paintEvent( QPaintEvent * );

private slots:
    void	timeout();

private:
    QTime	time;
};


#endif // ACLOCK_H
