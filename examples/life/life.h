/****************************************************************************
** $Id: life.h,v 2.2 1998/05/21 19:24:54 agulbra Exp $
**
** Copyright (C) 1992-1998 Troll Tech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#ifndef LIFE_H
#define LIFE_H

#include <qframe.h>


class LifeWidget : public QFrame
{
    Q_OBJECT
public:
    LifeWidget( QWidget *parent = 0, const char *name = 0 );

    void	setPoint( int i, int j );

    int		maxCol() { return maxi; }
    int		maxRow() { return maxj; }

public slots:
    void	nextGeneration();
    void	clear();

protected:
    virtual void paintEvent( QPaintEvent * );
    virtual void mouseMoveEvent( QMouseEvent * );
    virtual void mousePressEvent( QMouseEvent * );
    virtual void resizeEvent( QResizeEvent * );
    void	 mouseHandle( const QPoint &pos );

private:
    enum { SCALE = 10, MAXSIZE = 50, MINSIZE = 10, BORDER = 5 };

    bool	cells[2][MAXSIZE + 2][MAXSIZE + 2];
    int		current;
    int		maxi, maxj;

    static int pos2index( int x )
    {
	return ( x - BORDER ) / SCALE + 1;
    }
    static int index2pos( int i )
    {
	return	( i - 1 ) * SCALE + BORDER;
    }

};


#endif // LIFE_H
