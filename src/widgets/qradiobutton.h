/****************************************************************************
** $Id: qradiobutton.h,v 2.9.2.2 1998/08/21 19:13:26 hanord Exp $
**
** Definition of QRadioButton class
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

#ifndef QRADIOBUTTON_H
#define QRADIOBUTTON_H

#ifndef QT_H
#include "qbutton.h"
#endif // QT_H


class Q_EXPORT QRadioButton : public QButton
{
    Q_OBJECT
public:
    QRadioButton( QWidget *parent=0, const char *name=0 );
    QRadioButton( const char *text, QWidget *parent=0, const char *name=0 );

    bool    isChecked() const;
    void    setChecked( bool check );

    QSize    sizeHint() const;

protected:
    bool    hitButton( const QPoint & ) const;
    void    drawButton( QPainter * );
    void    drawButtonLabel( QPainter * );

    void    mouseReleaseEvent( QMouseEvent * );
    void    keyPressEvent( QKeyEvent * );

private:
    void    init();
    uint    noHit : 1;

private:	// Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QRadioButton( const QRadioButton & );
    QRadioButton &operator=( const QRadioButton & );
#endif
};


inline bool QRadioButton::isChecked() const
{ return isOn(); }

inline void QRadioButton::setChecked( bool check )
{ setOn( check ); }


#endif // QRADIOBUTTON_H
