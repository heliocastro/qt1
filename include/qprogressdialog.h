/****************************************************************************
** $Id: qprogressdialog.h,v 2.15.2.3 1998/08/21 19:13:22 hanord Exp $
**
** Definition of QProgressDialog class
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

#ifndef QPROGRESSDIALOG_H
#define QPROGRESSDIALOG_H

#ifndef QT_H
#include "qsemimodal.h"
#include "qpushbutton.h"
#include "qlabel.h"
#include "qprogressbar.h"
#endif // QT_H

struct QProgressData;


class Q_EXPORT QProgressDialog : public QSemiModal
{
    Q_OBJECT
public:
    QProgressDialog( QWidget *parent=0, const char *name=0, bool modal=FALSE,
		     WFlags f=0 );
    QProgressDialog( const char *labelText, const char *cancelButtonText,
		     int totalSteps, QWidget *parent=0, const char *name=0,
		     bool modal=FALSE, WFlags f=0 );
   ~QProgressDialog();

    void	setLabel( QLabel * );
    void	setCancelButton( QPushButton * );
    void	setBar( QProgressBar * );

    bool	wasCancelled() const;

    int		totalSteps() const;
    int		progress()   const;

    QSize	sizeHint() const;

public slots:
    void	cancel();
    void	reset();
    void	setTotalSteps( int totalSteps );
    void	setProgress( int progress );
    void	setLabelText( const char * );
    void	setCancelButtonText( const char * );

    void	setMinimumDuration( int ms );
    int		minimumDuration() const;

signals:
    void	cancelled();

protected:
    void	resizeEvent( QResizeEvent * );
    void	styleChange(GUIStyle);

private:
    void	   init( QWidget *creator, const char* lbl, const char* canc,
		         int totstps);
    void	   center();
    void	   layout();
    QLabel	  *label()  const;
    QProgressBar  *bar()    const;
    QProgressData *d;

private:	// Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QProgressDialog( const QProgressDialog & );
    QProgressDialog &operator=( const QProgressDialog & );
#endif
};


#endif // QPROGRESSDIALOG_H
