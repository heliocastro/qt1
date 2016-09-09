/****************************************************************************
** $Id: qprogressbar.h,v 2.10.2.2 1998/08/21 19:13:26 hanord Exp $
**
** Definition of QProgressBar class
**
** Created : 970520
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

#ifndef QPROGRESSBAR_H
#define QPROGRESSBAR_H

#ifndef QT_H
#include "qframe.h"
#endif // QT_H


class Q_EXPORT QProgressBar : public QFrame
{
    Q_OBJECT
public:
    QProgressBar( QWidget *parent=0, const char *name=0, WFlags f=0 );
    QProgressBar( int totalSteps, QWidget *parent=0, const char *name=0,
		  WFlags f=0 );

    int		totalSteps() const;
    int		progress()   const;

    QSize	sizeHint() const;
    void	show();

public slots:
    void	reset();
    void	setTotalSteps( int totalSteps );
    void	setProgress( int progress );

protected:
    void	drawContents( QPainter * );
    virtual bool setIndicator( QString& progress_str, int progress,
			       int totalSteps );

private:
    int		total_steps;
    int		progress_val;
    int		percentage;
    QString	progress_str;

private:	// Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QProgressBar( const QProgressBar & );
    QProgressBar &operator=( const QProgressBar & );
#endif
};


inline int QProgressBar::totalSteps() const
{
    return total_steps;
}

inline int QProgressBar::progress() const
{
    return progress_val;
}


#endif // QPROGRESSBAR_H
