/****************************************************************************
** $Id: qtooltip.h,v 2.19.2.1 1998/08/19 16:02:44 agulbra Exp $
**
** Definition of Tool Tips (or Balloon Help) for any widget or rectangle
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

#ifndef QTOOLTIP_H
#define QTOOLTIP_H

#ifndef QT_H
#include "qwidget.h"
#include "qtimer.h"
#endif // QT_H


class QTipManager;
class QLabel;


class Q_EXPORT QToolTipGroup: public QObject
{
    Q_OBJECT
public:
    QToolTipGroup( QObject *parent, const char *name = 0 );
   ~QToolTipGroup();

signals:
    void showTip( const char * );
    void removeTip();

private:
    friend class QTipManager;
};


class Q_EXPORT QToolTip
{
public:
    QToolTip( QWidget *, QToolTipGroup * = 0 );

    static void add( QWidget *, const char * );
    static void add( QWidget *, const char *,
		     QToolTipGroup *, const char * );
    static void remove( QWidget * );

    static void add( QWidget *, const QRect &, const char * );
    static void add( QWidget *, const QRect &, const char *,
		     QToolTipGroup *, const char * );
    static void remove( QWidget *, const QRect & );

    static QFont    font();
    static void	    setFont( const QFont & );
    static QPalette palette();
    static void	    setPalette( const QPalette & );

protected:
    virtual void maybeTip( const QPoint & ) = 0;
    void    tip( const QRect &, const char * );
    void    tip( const QRect &, const char *, const char * );
    void    clear();

public:
    QWidget	  *parentWidget() const { return p; }
    QToolTipGroup *group()	  const { return g; }

private:
    QWidget	    *p;
    QToolTipGroup   *g;
    static QFont    *ttFont;
    static QPalette *ttPalette;

    static void initialize();
    static void cleanup();

    friend class QTipManager;
};


#endif // QTOOLTIP_H
