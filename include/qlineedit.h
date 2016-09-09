/**********************************************************************
** $Id: qlineedit.h,v 2.29.2.4 1998/10/23 13:54:32 agulbra Exp $
**
** Definition of QLineEdit widget class
**
** Created : 941011
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

#ifndef QLINEEDIT_H
#define QLINEEDIT_H

struct QLineEditPrivate;

class QComboBox;
class QValidator;


#ifndef QT_H
#include "qwidget.h"
#include "qstring.h"
#endif // QT_H


class Q_EXPORT QLineEdit : public QWidget
{
    Q_OBJECT
public:
    QLineEdit( QWidget *parent=0, const char *name=0 );
   ~QLineEdit();

    const char *text() const;
    int		maxLength()	const;
    void	setMaxLength( int );

    void	setFrame( bool );
    bool	frame() const;

    enum	EchoMode { Normal, NoEcho, Password };
    void	setEchoMode( EchoMode );
    EchoMode 	echoMode() const;

    void	setValidator( QValidator * );
    QValidator * validator() const;

    QSize	sizeHint() const;

    void	setEnabled( bool );
    void	setFont( const QFont & );
    void	setPalette( const QPalette & );

    void	setSelection( int, int );
    void	setCursorPosition( int );
    int		cursorPosition() const;

    bool	validateAndSet( const char *, int, int, int );

    void	cut();
    void	copy() const;
    void	paste();

public slots:
    void	setText( const char * );
    void	selectAll();
    void	deselect();

    void	clearValidator();

    void	insert( const char * );

    void	clear();

signals:
    void	textChanged( const char * );
    void	returnPressed();

protected:
    void	mousePressEvent( QMouseEvent * );
    void	mouseMoveEvent( QMouseEvent * );
    void	mouseReleaseEvent( QMouseEvent * );
    void	mouseDoubleClickEvent( QMouseEvent * );
    void	keyPressEvent( QKeyEvent * );
    void	focusInEvent( QFocusEvent * );
    void	focusOutEvent( QFocusEvent * );
    void	paintEvent( QPaintEvent * );
    void	timerEvent( QTimerEvent * );
    void	resizeEvent( QResizeEvent * );
    void	leaveEvent( QEvent * );

    bool	event( QEvent * );

    bool	hasMarkedText() const;
    QString	markedText() const;


    void	repaintArea( int, int );

private slots:
    void	clipboardChanged();
    void	blinkSlot();
    void	dragScrollSlot();

private:
    // obsolete
    void	paint( const QRect& clip, bool frame = FALSE );
    void	pixmapPaint( const QRect& clip );
    // kept
    void	paintText( QPainter *, const QSize &, bool frame = FALSE );
    // to be replaced by publics
    void	cursorLeft( bool mark, int steps = 1 );
    void	cursorRight( bool mark, int steps = 1 );
    void	backspace();
    void	del();
    void	home( bool mark );
    void	end( bool mark );
    // kept
    void	newMark( int pos, bool copy=TRUE );
    void	markWord( int pos );
    void	copyText();
    int		lastCharVisible() const;
    int		minMark() const;
    int		maxMark() const;

    QString	tbuf;
    QLineEditPrivate * d;
    int		cursorPos;
    int		offset;
    int		maxLen;
    int		markAnchor;
    int		markDrag;
    bool	cursorOn;
    bool	dragScrolling;
    bool	scrollingLeft;

private:	// Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QLineEdit( const QLineEdit & );
    QLineEdit &operator=( const QLineEdit & );
#endif

    friend class QComboBox;
};


#endif // QLINEEDIT_H
