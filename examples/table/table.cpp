/****************************************************************************
** $Id: table.cpp,v 1.7 1998/05/21 19:24:59 agulbra Exp $
**
** Copyright (C) 1992-1998 Troll Tech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#include "table.h"
#include <qpainter.h>
#include <qkeycode.h>
#include <qprinter.h>

/*
  Constructs a Table widget.
*/

Table::Table( int numRows, int numCols, QWidget *parent, const char *name )
    : QTableView(parent,name)
{
    curRow = curCol = 0;			// set currently selected cell
    setFocusPolicy( StrongFocus );		// we accept keyboard focus
    setBackgroundMode( PaletteBase );		// set widgets background
    setNumCols( numCols );			// set number of col's in table
    setNumRows( numRows );			// set number of rows in table
    setCellWidth( 100 );			// set width of cell in pixels
    setCellHeight( 30 );			// set height of cell in pixels
    setTableFlags( Tbl_vScrollBar |		// always vertical scroll bar
		   Tbl_hScrollBar |		// ditto for horizontal
		   Tbl_clipCellPainting |	// avoid drawing outside cell
		   Tbl_smoothScrolling);	// easier to see the scrolling
    resize( 400, 200 );				// set default size in pixels

    contents = new QString[numRows * numCols];	// make room for contents
}


/*
  Destructor: deallocates memory for contents
*/

Table::~Table()
{
    delete[] contents;				// deallocation
}


/*
  Return content of cell
*/

const char* Table::cellContent( int row, int col ) const
{
    return contents[indexOf( row, col )];	// contents array lookup
}


/*
  Set content of cell
*/

void Table::setCellContent( int row, int col, const char* c )
{
    contents[indexOf( row, col )] = c;		// contents lookup and assign
    updateCell( row, col );			// show new content
}


/*
  Handles cell painting for the Table widget.
*/

void Table::paintCell( QPainter* p, int row, int col )
{
    int w = cellWidth( col );			// width of cell in pixels
    int h = cellHeight( row );			// height of cell in pixels
    int x2 = w - 1;
    int y2 = h - 1;

    /*
      Draw our part of cell frame.
    */
    p->drawLine( x2, 0, x2, y2 );		// draw vertical line on right
    p->drawLine( 0, y2, x2, y2 );		// draw horiz. line at bottom

    /*
      Draw extra frame inside if this is the current cell.
    */
    if ( (row == curRow) && (col == curCol) ) {	// if we are on current cell,
	if ( hasFocus() ) {
	    p->drawRect( 0, 0, x2, y2 );	// draw rect. along cell edges
	}
	else {					// we don't have focus, so
	    p->setPen( DotLine );		// use dashed line to
	    p->drawRect( 0, 0, x2, y2 );	// draw rect. along cell edges
	    p->setPen( SolidLine );		// restore to normal
	}
    }

    /*
      Draw cell content (text)
    */
    p->drawText( 0, 0, w, h, AlignCenter, contents[indexOf(row,col)] );
}


/*
  Handles mouse press events for the Table widget.
  The current cell marker is set to the cell the mouse is clicked in.
*/

void Table::mousePressEvent( QMouseEvent* e )
{
    int oldRow = curRow;			// store previous current cell
    int oldCol = curCol;
    QPoint clickedPos = e->pos();		// extract pointer position
    curRow = findRow( clickedPos.y() );		// map to row; set current cell
    curCol = findCol( clickedPos.x() );		// map to col; set current cell
    if ( (curRow != oldRow) 			// if current cell has moved,
	 || (curCol != oldCol) ) {
	updateCell( oldRow, oldCol );		// erase previous marking
	updateCell( curRow, curCol );		// show new current cell
    }
}


/*
  Handles key press events for the Table widget.
  Allows moving the current cell marker around with the arrow keys
*/

void Table::keyPressEvent( QKeyEvent* e )
{
    int oldRow = curRow;			// store previous current cell
    int oldCol = curCol;
    int edge = 0;
    switch( e->key() ) {			// Look at the key code
	case Key_Left:				// If 'left arrow'-key, 
	    if( curCol > 0 ) {			// and cr't not in leftmost col
		curCol--;     			// set cr't to next left column
		edge = leftCell();		// find left edge
		if ( curCol < edge )		// if we have moved off  edge,
		    setLeftCell( edge - 1 );	// scroll view to rectify
	    }
	    break;
	case Key_Right:				// Correspondingly...
	    if( curCol < numCols()-1 ) {
		curCol++;
		edge = lastColVisible();
		if ( curCol >= edge )
		    setLeftCell( leftCell() + 1 );
	    }
	    break;
	case Key_Up:
	    if( curRow > 0 ) {
		curRow--;
		edge = topCell();
		if ( curRow < edge )
		    setTopCell( edge - 1 );
	    }
	    break;
	case Key_Down:
	    if( curRow < numRows()-1 ) {
		curRow++;
		edge = lastRowVisible();
		if ( curRow >= edge )
		    setTopCell( topCell() + 1 );
	    }
	    break;
	default:				// If not an interesting key,
	    e->ignore();			// we don't accept the event
	    return;	
    }
    
    if ( (curRow != oldRow) 			// if current cell has moved,
	 || (curCol != oldCol)  ) {
	updateCell( oldRow, oldCol );		// erase previous marking
	updateCell( curRow, curCol );		// show new current cell
    }
}


/*
  Handles focus reception events for the Table widget.
  Repaint only the current cell; to avoid flickering
*/

void Table::focusInEvent( QFocusEvent* )
{
    updateCell( curRow, curCol );		// draw current cell
}    


/*
  Handles focus loss events for the Table widget.
  Repaint only the current cell; to avoid flickering
*/

void Table::focusOutEvent( QFocusEvent* )
{
    updateCell( curRow, curCol );		// draw current cell
}    


/*
  Utility function for mapping from 2D table to 1D array
*/

int Table::indexOf( int row, int col ) const
{
    return (row * numCols()) + col;
}
