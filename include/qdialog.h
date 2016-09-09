/****************************************************************************
** $Id: qdialog.h,v 2.7.2.2 1998/08/21 19:13:22 hanord Exp $
**
** Definition of QDialog class
**
** Created : 950502
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

#ifndef QDIALOG_H
#define QDIALOG_H

#ifndef QT_H
#include "qwidget.h"
#endif // QT_H


class QPushButton;


class Q_EXPORT QDialog : public QWidget			// dialog widget
{
friend class QPushButton;
    Q_OBJECT
public:
    QDialog( QWidget *parent=0, const char *name=0, bool modal=FALSE,
	     WFlags f=0 );
   ~QDialog();

    enum DialogCode { Rejected, Accepted };

    int		exec();
    int		result()  const { return rescode; }

    void	show();
    void	move( int x, int y );
    void	move( const QPoint &p );
    void	resize( int w, int h );
    void	resize( const QSize & );
    void	setGeometry( int x, int y, int w, int h );
    void	setGeometry( const QRect & );

protected slots:
    virtual void done( int );
    void	accept();
    void	reject();

protected:
    void	setResult( int r )	{ rescode = r; }
    void	keyPressEvent( QKeyEvent * );
    void	closeEvent( QCloseEvent * );

private:
    void	setDefault( QPushButton * );
    int		rescode;
    uint	did_move   : 1;
    uint	did_resize : 1;

private:	// Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QDialog( const QDialog & );
    QDialog &operator=( const QDialog & );
#endif
};


#endif // QDIALOG_H
