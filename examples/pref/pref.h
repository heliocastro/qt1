/****************************************************************************
** $Id: pref.h,v 1.6 1998/05/21 19:24:56 agulbra Exp $
**
** Copyright (C) 1992-1998 Troll Tech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#ifndef PREF_H
#define PREF_H

#include <qlabel.h>

class QMultiLineEdit;
class QButtonGroup;
class QRadioButton;
class QSlider;

class Preferences: public QLabel
{
    Q_OBJECT

public:
    Preferences( QWidget * parent = 0, const char * name = 0 );
    ~Preferences();

private slots:
    void setup();
    void apply();

private:
    // for page one
    QMultiLineEdit * ed1,  * ed2;

    // for page two
    QButtonGroup * bg;
    QRadioButton * b1, * b2, * b3;
    QSlider * mood;
};

#endif
