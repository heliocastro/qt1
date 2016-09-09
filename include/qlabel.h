/**********************************************************************
** $Id: qlabel.h,v 2.8.2.2 1998/08/21 19:13:25 hanord Exp $
**
** Definition of QLabel widget class
**
** Created : 941215
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

#ifndef QLABEL_H
#define QLABEL_H

#ifndef QT_H
#include "qframe.h"
#endif // QT_H


class Q_EXPORT QLabel : public QFrame
{
    Q_OBJECT
public:
    QLabel( QWidget *parent=0, const char *name=0, WFlags f=0 );
    QLabel( const char *text, QWidget *parent=0, const char *name=0,
	    WFlags f=0 );
    QLabel( QWidget * buddy, const char * text,
	    QWidget * parent, const char * name=0, WFlags f=0 );
   ~QLabel();

    const char *text()		const	{ return ltext; }
    QPixmap    *pixmap()	const	{ return lpixmap; }
    QMovie     *movie()		const;

    int		alignment()	const	{ return align; }
    void	setAlignment( int );
    int		margin()	const	{ return extraMargin; }
    void	setMargin( int );

    bool	autoResize()	const	{ return autoresize; }
    void	setAutoResize( bool );
    QSize	sizeHint() const;

    void	setBuddy( QWidget * );
    QWidget    *buddy() const;

public slots:
    void	setText( const char * );
    void	setPixmap( const QPixmap & );
    void	setMovie( const QMovie & );
    void	setNum( int );
    void	setNum( double );
    void	clear();

protected:
    void	drawContents( QPainter * );

private slots:
    void	acceleratorSlot();
    void	buddyDied();
    void	movieUpdated(const QRect&);
    void	movieResized(const QSize&);

private:
    void	updateLabel();
    QString	ltext;
    QPixmap    *lpixmap;
    int		extraMargin;
    int		align;
    bool	autoresize;
    void	unsetMovie();

private:	// Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QLabel( const QLabel & );
    QLabel &operator=( const QLabel & );
#endif
};


#endif // QLABEL_H
