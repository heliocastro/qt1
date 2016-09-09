/**********************************************************************
** $Id: qwellarray.h,v 1.6.2.1 1998/08/19 16:02:44 agulbra Exp $
**
** Definition of QWellArray widget class
**
** Created : 980114
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

#ifndef QWELLARRAY_H
#define QWELLARRAY_H

#ifndef QT_H
#include "qtableview.h"
#endif // QT_H

struct QWellArrayData;

class Q_EXPORT QWellArray : public QTableView
{
	Q_OBJECT
public:
    QWellArray( QWidget *parent=0, const char *name=0, bool popup = FALSE );

    ~QWellArray() {}
    const char* cellContent( int row, int col ) const;
    void setCellContent( int row, int col, const char* );

    int numCols() { return nCols; }
    int numRows() { return nRows; }

    QSize sizeHint() const;

    virtual void setDimension( int rows, int cols );
    void setCellBrush( int row, int col, const QBrush & );

protected:
    void setSelected( int row, int col );
    void setCurrent( int row, int col );

    void drawContents( QPainter *, int row, int col, const QRect& );
    void drawContents( QPainter * );

    void paintCell( QPainter*, int row, int col );
    void mousePressEvent( QMouseEvent* );
    void mouseMoveEvent( QMouseEvent* );
    void keyPressEvent( QKeyEvent* );
    void focusInEvent( QFocusEvent* );
    void focusOutEvent( QFocusEvent* );

private:
    int curRow;
    int curCol;
    int selRow;
    int selCol;
    int nCols;
    int nRows;
    bool smallStyle;
    QWellArrayData *d;
};

#endif
