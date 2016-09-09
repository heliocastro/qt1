/****************************************************************************
** $Id: qcheckbox.h,v 2.7.2.2 1998/08/21 19:13:24 hanord Exp $
**
** Definition of QCheckBox class
**
** Created : 940222
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

#ifndef QCHECKBOX_H
#define QCHECKBOX_H

#ifndef QT_H
#include "qbutton.h"
#endif // QT_H


class Q_EXPORT QCheckBox : public QButton
{
    Q_OBJECT
public:
    QCheckBox( QWidget *parent=0, const char *name=0 );
    QCheckBox( const char *text, QWidget *parent, const char *name=0 );

    bool    isChecked() const;
    void    setChecked( bool check );

    QSize sizeHint() const;

protected:
    void    drawButton( QPainter * );
    void    drawButtonLabel( QPainter * );

private:	// Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QCheckBox( const QCheckBox & );
    QCheckBox &operator=( const QCheckBox & );
#endif
};


inline bool QCheckBox::isChecked() const
{ return isOn(); }

inline void QCheckBox::setChecked( bool check )
{ setOn( check ); }


#endif // QCHECKBOX_H
