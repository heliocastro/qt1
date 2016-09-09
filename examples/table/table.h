/****************************************************************************
** $Id: table.h,v 1.5 1998/06/16 11:39:34 warwick Exp $
**
** Copyright (C) 1992-1998 Troll Tech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#ifndef TABLE_H
#define TABLE_H

#include <qtableview.h>


class Table : public QTableView
{
    Q_OBJECT
public:
    Table( int numRows, int numCols, QWidget* parent=0, const char* name=0 );
    ~Table();
    
    const char* cellContent( int row, int col ) const;
    void setCellContent( int row, int col, const char* );

protected:
    void paintCell( QPainter*, int row, int col );
    void mousePressEvent( QMouseEvent* );
    void keyPressEvent( QKeyEvent* );
    void focusInEvent( QFocusEvent* );
    void focusOutEvent( QFocusEvent* );
    
private:
    int indexOf( int row, int col ) const;
    QString* contents;
    int curRow;
    int curCol;
};

#endif // TABLE_H
