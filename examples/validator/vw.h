/****************************************************************************
** $Id: vw.h,v 1.4 1998/06/13 21:39:58 agulbra Exp $
**
** Copyright (C) 1992-1998 Troll Tech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#ifndef VW_H
#define VW_H

#include <qvalidator.h>
#include <qstring.h>
#include <qwidget.h>

class VW: public QWidget {
    Q_OBJECT
public:
    VW( QWidget * parent = 0, const char * name = 0 );
    ~VW();

private slots:
    void modelSelected( const char * );
    void motorSelected( int );
    void yearSelected( int );

signals:
    void validSelectionMade( const char * );

private:
    void computeSelection();

    QString currentModel;
    int currentMotorSize;
    int currentYear;
};


#endif
