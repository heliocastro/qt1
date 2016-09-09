/****************************************************************************
** $Id: biff.h,v 2.3 1998/06/16 11:39:32 warwick Exp $
**
** Copyright (C) 1992-1998 Troll Tech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#ifndef BIFF_H
#define BIFF_H

#include <qwidget.h>
#include <qdatetime.h>
#include <qpixmap.h>


class Biff : public QWidget
{
    Q_OBJECT
public:
    Biff( QWidget *parent=0, const char *name=0 );

protected:
    void	timerEvent( QTimerEvent * );
    void	paintEvent( QPaintEvent * );
    void	mousePressEvent( QMouseEvent * );

private:
    QDateTime	lastModified;
    QPixmap	hasNewMail;
    QPixmap	noNewMail;
    QString	mailbox;
    bool	gotMail;
};


#endif // BIFF_H
