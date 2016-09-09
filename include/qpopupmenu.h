/****************************************************************************
** $Id: qpopupmenu.h,v 2.15.2.3 1999/01/25 16:56:03 ettrich Exp $
**
** Definition of QPopupMenu class
**
** Created : 941128
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

#ifndef QPOPUPMENU_H
#define QPOPUPMENU_H

#ifndef QT_H
#include "qtableview.h"
#include "qmenudata.h"
#endif // QT_H


class Q_EXPORT QPopupMenu : public QTableView, public QMenuData
{
friend class QMenuData;
friend class QMenuBar;
    Q_OBJECT
public:
    QPopupMenu( QWidget *parent=0, const char *name=0 );
   ~QPopupMenu();

    void	popup( const QPoint & pos, int indexAtPoint = 0 );// open popup
    void	updateItem( int id );

    void	setCheckable( bool );
    bool	isCheckable() const;

    void	setFont( const QFont & );	// reimplemented set font
    void	show();				// reimplemented show
    void	hide();				// reimplemented hide

    int		exec();
    int 	exec( const QPoint & pos, int indexAtPoint = 0 );// modal popup

    void	setActiveItem( int );

    int idAt( int index ) const { return QMenuData::idAt( index ); }
    int idAt( const QPoint& pos ) const;

signals:
    void	activated( int itemId );
    void	highlighted( int itemId );
    void	activatedRedirect( int itemId );// to parent menu
    void	highlightedRedirect( int itemId );
    void	aboutToShow();

protected:
    int		cellHeight( int );
    int		cellWidth( int );
    void	paintCell( QPainter *, int, int );

    void	paintEvent( QPaintEvent * );
    void	mousePressEvent( QMouseEvent * );
    void	mouseReleaseEvent( QMouseEvent * );
    void	mouseMoveEvent( QMouseEvent * );
    void	keyPressEvent( QKeyEvent * );
    void	timerEvent( QTimerEvent * );

private slots:
    void	subActivated( int itemId );
    void	subHighlighted( int itemId );
    void	accelActivated( int itemId );
    void	accelDestroyed();
    void	modalActivation( int );

    void	subMenuTimer();

private:
    void	menuContentsChanged();
    void	menuStateChanged();
    void	menuInsPopup( QPopupMenu * );
    void	menuDelPopup( QPopupMenu * );
    void	frameChanged();

    void	paintAll();
    void	actSig( int );
    void	hilitSig( int );
    void	setFirstItemActive();
    void	hideAllPopups();
    void	hidePopups();
    bool	tryMenuBar( QMouseEvent * );
    void	byeMenuBar();

    int		itemAtPos( const QPoint & );
    int		itemPos( int index );
    void	updateSize();
    void	updateRow( int row );
    void	updateAccel( QWidget * );
    void	enableAccel( bool );

    void	setTabMark( int );
    int		tabMark();
    void	setCheckableFlag( bool );

    QMenuItem  *selfItem;
    QAccel     *autoaccel;
    bool	accelDisabled;
    int		popupActive;
    int		tabCheck;

private:	// Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QPopupMenu( const QPopupMenu & );
    QPopupMenu &operator=( const QPopupMenu & );
#endif
};


#endif // QPOPUPMENU_H
