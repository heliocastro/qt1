/****************************************************************************
** $Id: qtgrid.h,v 1.3 1998/05/21 19:24:53 agulbra Exp $
**
** Copyright (C) 1992-1998 Troll Tech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#ifndef QGRID_H
#define QGRID_H

#ifndef QT_H
#include "qwidget.h"
#endif // QT_H

class QGridLayout;

class QtGrid : public QWidget
{
    Q_OBJECT
public:
    enum Direction { Horizontal, Vertical };
    QtGrid( int n, QWidget *parent=0, const char *name=0, WFlags f=0 );
    QtGrid( int n, Direction, QWidget *parent=0, const char *name=0, WFlags f=0 );
    bool event( QEvent * );

    void skip();
protected:
    virtual void childEvent( QChildEvent * );
private:
    QGridLayout *lay;
    int row;
    int col;
    int nRows, nCols;
    Direction dir;
};

#endif //QGRID_H
