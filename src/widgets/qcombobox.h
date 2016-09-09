/**********************************************************************
** $Id: qcombobox.h,v 2.28.2.2 1998/08/21 19:13:25 hanord Exp $
**
** Definition of QComboBox class
**
** Created : 950426
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

#ifndef QCOMBOBOX_H
#define QCOMBOBOX_H

#ifndef QT_H
#include "qwidget.h"
#endif // QT_H


struct QComboData;
class QStrList;
class QLineEdit;
class QValidator;
class QListBox;


class Q_EXPORT QComboBox : public QWidget
{
    Q_OBJECT
public:
    QComboBox( QWidget *parent=0, const char *name=0 );
    QComboBox( bool rw, QWidget *parent=0, const char *name=0 );
   ~QComboBox();

    int		count() const;

    void	insertStrList( const QStrList *, int index=-1 );
    void	insertStrList( const char **, int numStrings=-1, int index=-1);

    void	insertItem( const char *text, int index=-1 );
    void	insertItem( const QPixmap &pixmap, int index=-1 );

    void	removeItem( int index );
    void	clear();

    const char *currentText() const;
    const char *text( int index ) const;
    const QPixmap *pixmap( int index ) const;

    void	changeItem( const char *text, int index );
    void	changeItem( const QPixmap &pixmap, int index );

    int		currentItem() const;
    void	setCurrentItem( int index );

    bool	autoResize()	const;
    void	setAutoResize( bool );
    QSize	sizeHint() const;

    void	setBackgroundColor( const QColor & );
    void	setPalette( const QPalette & );
    void	setFont( const QFont & );
    void	setEnabled( bool );

    void	setSizeLimit( int );
    int		sizeLimit() const;

    void	setMaxCount( int );
    int		maxCount() const;

    enum Policy { NoInsertion, AtTop, AtCurrent, AtBottom,
		  AfterCurrent, BeforeCurrent };

    void	setInsertionPolicy( Policy policy );
    Policy 	insertionPolicy() const;

    void	setStyle( GUIStyle );

    void	setValidator( QValidator * );
    QValidator * validator() const;

    void	setListBox( QListBox * );
    QListBox * 	listBox() const;
    
    void	setAutoCompletion( bool );
    bool	autoCompletion() const;

    bool	eventFilter( QObject *object, QEvent *event );

public slots:
    void	clearValidator();
    void	clearEdit();
    void	setEditText( const char * );

signals:
    void	activated( int index );
    void	highlighted( int index );
    void	activated( const char * );
    void	highlighted( const char * );

private slots:
    void	internalActivate( int );
    void	internalHighlight( int );
    void	internalClickTimeout();
    void	returnPressed();

protected:
    void	paintEvent( QPaintEvent * );
    void	resizeEvent( QResizeEvent * );
    void	mousePressEvent( QMouseEvent * );
    void	mouseMoveEvent( QMouseEvent * );
    void	mouseReleaseEvent( QMouseEvent * );
    void	mouseDoubleClickEvent( QMouseEvent * );
    void	keyPressEvent( QKeyEvent *e );
    void	focusInEvent( QFocusEvent *e );

    void	popup();

private:
    void	popDownListBox();
    void	reIndex();
    void	currentChanged();
    QRect	arrowRect() const;
    bool	getMetrics( int *dist, int *buttonW, int *buttonH ) const;

    QComboData	*d;

private:	// Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QComboBox( const QComboBox & );
    QComboBox &operator=( const QComboBox & );
#endif
};


#endif // QCOMBOBOX_H
