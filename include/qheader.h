/****************************************************************************
** $Id: qheader.h,v 2.20.2.2 1998/08/21 19:13:25 hanord Exp $
**
** Definition of QHeader widget class (table header)
**
** Created : 961105
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

#ifndef QHEADER_H
#define QHEADER_H

#ifndef QT_H
#include "qtableview.h"
#endif // QT_H

struct QHeaderData;

class Q_EXPORT QHeader : public QTableView
{
    Q_OBJECT
public:
    enum Orientation { Horizontal, Vertical };

    QHeader( QWidget *parent=0, const char *name=0 );
    QHeader( int, QWidget *parent=0, const char *name=0 );
    ~QHeader();

    int		addLabel( const char *, int size = -1 );
    void	setLabel( int, const char *, int size = -1 );
    const char*	label( int );
    void	setOrientation( Orientation );
    Orientation orientation() const;
    void	setTracking( bool enable );
    bool	tracking() const;

    void 	setClickEnabled( bool, int logIdx = -1 );
    void	setResizeEnabled( bool, int logIdx = -1 );
    void	setMovingEnabled( bool );

    void	setCellSize( int i, int s );
    int		cellSize( int i ) const;
    int		cellPos( int i ) const;
    int		cellAt( int i ) const;
    int		count() const;

    int 	offset() const;

    QSize	sizeHint() const;

    int		mapToLogical( int ) const;
    int		mapToActual( int ) const;

public slots:
    void	setOffset( int );

signals:
    void	sectionClicked( int );
    void	sizeChange( int section, int oldSize, int newSize );
    void	moved( int from, int to );
protected:
    //    void	timerEvent( QTimerEvent * );

    void	resizeEvent( QResizeEvent * );

    QRect	sRect( int i );

    void	paintCell( QPainter *, int, int );
    void	setupPainter( QPainter * );

    int		cellHeight( int );
    int		cellWidth( int );

    void	mousePressEvent( QMouseEvent * );
    void	mouseReleaseEvent( QMouseEvent * );
    void	mouseMoveEvent( QMouseEvent * );

private:
    void	init( int );

    void	paintRect( int p, int s );
    void	markLine( int idx );
    void	unMarkLine( int idx );
    int		pPos( int i ) const;
    int		pSize( int i ) const;

    int 	findLine( int );

    void	handleColumnResize(int, int, bool);

    void	moveAround( int fromIdx, int toIdx );

    int		handleIdx;
    int		oldHIdxSize;
    int		moveToIdx;
    enum State { Idle, Sliding, Pressed, Moving, Blocked };
    State	state;
    QCOORD	clickPos;
    bool	trackingIsOn;

    Orientation orient;

    QHeaderData *data;

private:	// Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QHeader( const QHeader & );
    QHeader &operator=( const QHeader & );
#endif
};


inline QHeader::Orientation QHeader::orientation() const
{
    return orient;
}

inline void QHeader::setTracking( bool enable ) { trackingIsOn = enable; }
inline bool QHeader::tracking() const { return trackingIsOn; }

#endif //QHEADER_H
