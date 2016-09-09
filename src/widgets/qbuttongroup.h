/****************************************************************************
** $Id: qbuttongroup.h,v 2.7.2.2 1998/08/21 19:13:24 hanord Exp $
**
** Definition of QButtonGroup class
**
** Created : 950130
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

#ifndef QBUTTONGROUP_H
#define QBUTTONGROUP_H

#ifndef QT_H
#include "qgroupbox.h"
#endif // QT_H


class QButton;
class QButtonList;


class Q_EXPORT QButtonGroup : public QGroupBox
{
    Q_OBJECT
public:
    QButtonGroup( QWidget *parent=0, const char *name=0 );
    QButtonGroup( const char *title, QWidget *parent=0, const char *name=0 );
   ~QButtonGroup();

    bool	isExclusive() const;
    void	setExclusive( bool );

    int		insert( QButton *, int id=-1 );
    void	remove( QButton * );
    QButton    *find( int id ) const;

    void	setButton( int id );

signals:
    void	pressed( int id );
    void	released( int id );
    void	clicked( int id );

protected slots:
    void	buttonPressed();
    void	buttonReleased();
    void	buttonClicked();
    void	buttonToggled( bool on );

private:
    void	init();
    bool	excl_grp;
    QButtonList *buttons;

private:	// Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QButtonGroup( const QButtonGroup & );
    QButtonGroup &operator=( const QButtonGroup & );
#endif
};


#endif // QBUTTONGROUP_H
