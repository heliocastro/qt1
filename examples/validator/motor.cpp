/****************************************************************************
** $Id: motor.cpp,v 1.9 1998/06/16 11:39:35 warwick Exp $
**
** Copyright (C) 1992-1998 Troll Tech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#include "motor.h"
#include "qlineedit.h"
#include "qpushbutton.h"
#include <math.h>

MotorValidator::MotorValidator( QSpinBox * parent, const char * name )
    : QValidator( parent, name )
{
    // just some random junk.
    b = 0;
    t = 42;
    s = 2;
}


MotorValidator::~MotorValidator()
{
    // nothing.  wow. programming qt is easy.
}


void MotorValidator::setRange( int bottom, int top, int step )
{
    b = bottom;
    t = top;
    s = step;
}


// the guts of this example: return TRUE if motorSize describes an
// integer in the range b to t which can be described as n*s+b for
// some integer n.

QValidator::State MotorValidator::validate( QString & motorSize, int & )
{
    bool ok;
    long int tmp = motorSize.toLong( &ok );
    if ( !ok )
	return QValidator::Invalid;
    else if ( tmp < b || tmp > t || ((tmp-b)%s) != 0 )
	return QValidator::Valid;
    else
	return QValidator::Acceptable;
}
