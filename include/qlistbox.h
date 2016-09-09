/**********************************************************************
** $Id: qlistbox.h,v 2.17.2.3 1998/08/21 19:13:25 hanord Exp $
**
** Definition of QListBox widget class
**
** Created : 941121
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

#ifndef QLISTBOX_H
#define QLISTBOX_H

#ifndef QT_H
#include "qtableview.h"
#include "qpixmap.h"
#endif // QT_H


#define LBI_Undefined	0			// list box item types
#define LBI_Text	1
#define LBI_Pixmap	2
#define LBI_UserDefined 1000


class QStrList;
class QLBItemList;

class QListBox;


class Q_EXPORT QListBoxItem
{
public:
    QListBoxItem();
    virtual ~QListBoxItem();

    virtual const char	  *text()   const { return txt; }
    virtual const QPixmap *pixmap() const { return 0; }

    virtual int	 height( const QListBox * ) const = 0;
    virtual int	 width( const QListBox * )  const = 0;

protected:
    virtual void paint( QPainter * ) = 0;
    void	 setText( const char *text ) { txt = text; }

private:
    QString txt;
    bool selected;

    friend class QListBox;

private:	// Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QListBoxItem( const QListBoxItem & );
    QListBoxItem &operator=( const QListBoxItem & );
#endif
};


class Q_EXPORT QListBoxText : public QListBoxItem
{
public:
    QListBoxText( const char * = 0 );
   ~QListBoxText();
    void  paint( QPainter * );
    int	  height( const QListBox * ) const;
    int	  width( const QListBox * )  const;
private:	// Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QListBoxText( const QListBoxText & );
    QListBoxText &operator=( const QListBoxText & );
#endif
};


class Q_EXPORT QListBoxPixmap : public QListBoxItem
{
public:
    QListBoxPixmap( const QPixmap & );
   ~QListBoxPixmap();
    const QPixmap *pixmap() const { return &pm; }
protected:
    void paint( QPainter * );
    int height( const QListBox * ) const;
    int width( const QListBox * ) const;
private:
    QPixmap pm;
private:	// Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QListBoxPixmap( const QListBoxPixmap & );
    QListBoxPixmap &operator=( const QListBoxPixmap & );
#endif
};


class Q_EXPORT QListBox : public QTableView		// list box widget
{
    Q_OBJECT
public:
    QListBox( QWidget *parent=0, const char *name=0, WFlags f=0  );
   ~QListBox();

    void	setFont( const QFont & );

    uint	count() const;

    void	insertStrList( const QStrList *, int index=-1 );
    void	insertStrList( const char**, int numStrings=-1, int index=-1 );

    void	insertItem( const QListBoxItem *, int index=-1 );
    void	insertItem( const char *text, int index=-1 );
    void	insertItem( const QPixmap &pixmap, int index=-1 );
    void	inSort( const QListBoxItem * );
    void	inSort( const char *text );

    void	removeItem( int index );
    void	clear();

    const char *text( int index )	const;
    const QPixmap *pixmap( int index )	const;

    void	changeItem( const QListBoxItem *, int index );
    void	changeItem( const char *text, int index );
    void	changeItem( const QPixmap &pixmap, int index );

    bool	autoUpdate()	const;
    void	setAutoUpdate( bool );

    int		numItemsVisible() const;
    void	setFixedVisibleLines( int lines );

    int		currentItem()	const;
    void	setCurrentItem( int index );
    void	centerCurrentItem();
    int		topItem()	const;
    void	setTopItem( int index );
    void	setBottomItem( int index ); 

    bool	dragSelect()		const;
    void	setDragSelect( bool );
    bool	autoScroll()		const;
    void	setAutoScroll( bool );
    bool	autoScrollBar()		const;
    void	setAutoScrollBar( bool );
    bool	scrollBar()		const;
    void	setScrollBar( bool );
    bool	autoBottomScrollBar()	const;
    void	setAutoBottomScrollBar( bool );
    bool	bottomScrollBar()	const;
    void	setBottomScrollBar( bool );
    bool	smoothScrolling()	const;
    void	setSmoothScrolling( bool );

    int		itemHeight()		const;
    int		itemHeight( int index ) const;

    long	maxItemWidth() const;
    long	maxItemWidth(); // only for bin. compat

    bool	isMultiSelection() const;
    void	setMultiSelection( bool );

    void	setSelected( int, bool );
    bool	isSelected( int ) const;

    QSize	sizeHint() const;

public slots:
    void	clearSelection();

signals:
    void	highlighted( int index );
    void	selected( int index );
    void	highlighted( const char * );
    void	selected( const char * );

    void	selectionChanged();

protected:
    QListBoxItem *item( int index ) const;
    bool	itemVisible( int index );

    int		cellHeight( int index = 0 );
    void	paintCell( QPainter *, int row, int col );

    void	mousePressEvent( QMouseEvent * );
    void	mouseReleaseEvent( QMouseEvent * );
    void	mouseDoubleClickEvent( QMouseEvent * );
    void	mouseMoveEvent( QMouseEvent * );
    void	keyPressEvent( QKeyEvent *e );
    void	focusInEvent( QFocusEvent *e );
    void	focusOutEvent( QFocusEvent *e );
    void	resizeEvent( QResizeEvent * );
    void	timerEvent( QTimerEvent * );

    int		findItem( int yPos ) const;
    bool	itemYPos( int index, int *yPos ) const;
    void	updateItem( int index, bool clear = TRUE );
    void	clearList();
    void	updateCellWidth();

    void	toggleCurrentItem();

private:
    void	updateNumRows( bool );
    void	insert( const QListBoxItem *, int, bool );
    void	change( const QListBoxItem *lbi, int );
    void	setMaxItemWidth( int );
    void	ensureCurrentVisible( int = -1 );

    void	emitChangedSignal( bool );

    uint	doDrag		: 1;
    uint	doAutoScroll	: 1;
    uint	isTiming	: 1;
    uint	scrollDown	: 1;
    uint	stringsOnly	: 1;
    uint	multiSelect	: 1;
    uint	goingDown	: 1;
    int		current;
    QLBItemList *itemList;

private:	// Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QListBox( const QListBox & );
    QListBox &operator=( const QListBox & );
#endif
};


inline bool QListBox::isMultiSelection() const
{
    return multiSelect;
}


#endif // QLISTBOX_H
