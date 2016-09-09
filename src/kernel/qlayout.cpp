/****************************************************************************
** $Id: qlayout.cpp,v 2.37.2.2 1998/09/21 10:14:50 aavit Exp $
**
** Implementation of layout classes
**
** Created : 960416
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

#include "qlayout.h"
#include "qmenubar.h"

/*!
  \class QLayout qlayout.h
  \brief The QLayout class is the base class of geometry specifiers.

  \ingroup geomanagement

  This is an abstract base class. The concrete layout managers
  QBoxLayout and QGridLayout inherit from this one and make QLayout's
  functionality avaialble in friendly APIs.

  Most users of Q*Layout are likely to use some of the basic functions
  provided by QLayout, such as <ul><li>activate(), which compiles the
  layout into an internal representation and activates the result,
  <li> setMenuBar(), which is necessary to manage a menu bar because
  of the special properties of menu bars, and <li> freeze(), which
  allows you to freeze the widget's size and layout. </ul>

  Geometry management stops when the layout manager is deleted.

  To make a new layout manager, you need to implement the functions
  mainVerticalChain(), mainHorizontalChain() and initGM().
*/


/*!
  Creates a new top-level QLayout with main widget \a
  parent.  \a parent may not be 0.

  \a border is the number of pixels between the edge of the widget and
  the managed children.	 \a autoBorder sets the value of defaultBorder(), which
  is interpreted by subclasses.	 If \a autoBorder is -1 the value
  of \a border is used.

  \a name is the internal object name

  Having several top-level layouts for the same widget will cause
  considerable confusion.

*/

QLayout::QLayout( QWidget *parent, int border, int autoBorder, const char *name )
    : QObject( parent, name )
{
    topLevel	 = TRUE;
    bm		 = new QGManager( parent, name );
    parent->removeChild( bm );
    insertChild( bm );

    if ( autoBorder < 0 )
	defBorder = border;
    else
	defBorder = autoBorder;
    bm->setBorder( border );
}

/*!
  \fn const char *QLayout::name() const

  Returns the internal object name.
*/


/*!
  Returns the main widget of this layout, or 0 if this layout is
  a sub-layout which is not yet inserted.
*/

QWidget * QLayout::mainWidget()
{
    return bm ? bm->mainWidget() : 0;
}


/*!
  Constructs a new child QLayout,
  If \a autoBorder is -1, this QLayout inherits \a parent's
  defaultBorder(), otherwise \a autoBorder is used.
*/

QLayout::QLayout( int autoBorder, const char *name )
    : QObject( 0, name )
{
    topLevel	 = FALSE;
    bm		 = 0;
    defBorder	 = autoBorder;
}


/*!
  Deletes all children layouts. Geometry management stops when
  a toplevel layout is deleted.
  \internal
  The layout classes will probably be fatally confused if you delete
  a sublayout
*/

QLayout::~QLayout()
{
}


/*!
  This function is called from addLayout functions in subclasses,
  to add \a l layout as a sublayout.
*/

void QLayout::addChildLayout( QLayout *l )
{
    if ( l->topLevel ) {
#if defined(CHECK_NULL)
	warning( "QLayout: Attempt to add top-level layout as child" );
#endif
	return;
    }
    l->bm = bm;
    insertChild( l );
    if ( l->defBorder < 0 )
	l->defBorder = defBorder;
    l->initGM();
}

/*!
  \fn void QLayout::initGM()

  Implement this function to do what's necessary to initialize chains,
  once the layout has a basicManager().
*/

/*!
  \fn QGManager *QLayout::basicManager()

  Returns the QGManager for this layout. Returns 0 if
  this is a child layout which has not been inserted yet.
*/


/*!
  \fn QChain *QLayout::mainVerticalChain()
  Implement this function to return the main vertical chain.
*/

/*!
  \fn QChain *QLayout::mainHorizontalChain()
  Implement this function to return the main horizontal chain.
*/

/*!
  \fn QChain *QLayout::horChain( QLayout * )
  This function works around a dubious feature in
  the C++ language definition, to provide access to mainHorizontalChain().
 */


/*!
  \fn QChain *QLayout::verChain( QLayout * )
  This function works around a dubious feature in
  the C++ language definition, to provide access to mainVerticalChain().
*/


/*!
  \fn int QLayout::defaultBorder() const
  Returns the default border for the geometry manager.
*/

/*!
  Starts geometry management - analogous to show() for widgets.
  This function should only be called for top level layouts.
*/

bool QLayout::activate()
{
    if ( topLevel && bm )
	return bm->activate();
#if defined(DEBUG)
    warning("QLayout::activate() for child layout");
#endif
    return FALSE;
}

/*!
  \overload void QLayout::freeze()

  This version of the method fixes the main widget at its minimum size.
  You can also achieve this with freeze( 0, 0 );
*/


/*!
  Fixes the size of the main widget and distributes the available
  space to the child widgets. For widgets which should not be
  resizable, but where a QLayout subclass is used to set up the initial
  geometry.

  A frozen layout cannot be unfrozen, the only sensible thing to do
  is to delete it.

  The size is adjusted to a valid value. Thus freeze(0,0) fixes the
  widget to its minimum size.
*/

void QLayout::freeze( int w, int h )
{
    if ( !topLevel ) {
#if defined(CHECK_STATE)
	warning( "QLayout::freeze: Only top-level QLayout can be frozen" );
#endif
	return;
    }
#if defined(CHECK_NULL)
    ASSERT( bm != 0 );
#endif
    bm->freeze( w, h );
    delete bm;
    bm = 0;
}


/*!
  Makes the geometry manager take account of the menu bar \a w. All
  child widgets are placed below the bottom edge of the menu bar.

  A menu bar does its own geometry managing, never do addWidget()
  on a menu bar.
*/

void QLayout::setMenuBar( QMenuBar *w )
{
    if ( !topLevel ) {
#if defined(CHECK_NULL)
	warning( "QLayout::setMenuBar: Called for sub layout" );
#endif
	return;
    }
    ASSERT( bm );
    bm->setMenuBar( w );
}


/*!
  \class QBoxLayout qlayout.h

  \brief The QBoxLayout class lines up child widgets horizontally or
  vertically.

  \ingroup geomanagement

  QBoxLayout takes the space it gets (from its parent layout or from
  the mainWindget()), divides it up into a row of boxes and makes each
  managed widget fill one box.

  If the QBoxLayout is \c Horizontal, the boxes are beside each other,
  with suitable sizes.  Each widget (or other box) will get at least
  its minimum sizes and at most its maximum size, and any excess space
  is shared according to the stretch factors (more about that below).

  If the QBoxLayout is \c Vertical, the boxes are above and below each
  other, again with suitable sizes.

  The easiest way to create a QBoxLayout is to use one of the
  convenience classes QHBoxLayout (for \c Horizontal boxes) or
  QVBoxLayout (for \c Vertical boxes). You can also use the QBoxLayout
  constuctor directly, specifying its direction as \c LeftToRight, \c
  Down, \c RightToLeft or \c Up.

  If the QBoxLayout is not the top-level layout (ie. is not managing
  all of the widget's area and children), you must add it to its
  parent layout before you can do anything with it.  The normal way to
  add a layout is by calling parentLayout->addLayout().

  Once you have done that, you can add boxes to the QBoxLayout using
  one of four functions: <ul>

  <li> addWidget() to add a widget to the QBoxLayout and set the
  widget's stretch factor.  (The stretch factor is along the row if
  boxes.)

  <li> addSpacing() to create an empty box; this is one of the
  functions you use to create nice and spacious dialogs.  See below
  for ways to set margins.

  <li> addStretch() to create an empty, stretchable box.

  <li> addLayout() to add a box containing another QLayout to the row
  and set that layout's stretch factor.

  </ul>

  Finally, if the layout is a top-level one, you activate() it.

  QBoxLayout also includes two margin widths: The border width and the
  inter-box width.  The border width is the width of the reserved
  space along each of the QBoxLayout's four sides.  The intra-widget
  width is the width of the automatically allocated spacing between
  neighbouring boxes.  (You can use addSpacing() to get more space.)

  The border width defaults to 0, and the intra-widget width defaults
  to the same as the border width.  Both are set using arguments to
  the constructor.

  You will almost always want to use the convenience classes for
  QBoxLayout: QVBoxLayout and QHBoxLayout, because of their simpler
  constructors.
*/

static inline bool horz( QGManager::Direction dir )
{
    return dir == QGManager::RightToLeft || dir == QGManager::LeftToRight;
}

static inline QGManager::Direction perp( QGManager::Direction dir )
{
    if ( horz( dir ))
	return QGManager::Down;
    else
	return QGManager::LeftToRight;
}

/*!
  Creates a new QBoxLayout with direction \a d and main widget \a
  parent.  \a parent may not be 0.

  \a border is the number of pixels between the edge of the widget and
  the managed children.	 \a autoBorder is the default number of pixels
  between neighbouring children.  If \a autoBorder is -1 the value
  of \a border is used.

  \a name is the internal object name

  \sa direction()
*/

QBoxLayout::QBoxLayout( QWidget *parent, Direction d,
			int border, int autoBorder, const char *name )
    : QLayout( parent, border, autoBorder, name )
{
    pristine = TRUE;
    dir = (QGManager::Direction)d;

    serChain = basicManager()->newSerChain( dir );
    basicManager()->setName( serChain, name );
    if ( horz( dir )  ) {
	basicManager()->add( basicManager()->xChain(), serChain );
	parChain = basicManager()->yChain();
    } else {
	basicManager()->add( basicManager()->yChain(), serChain );
	parChain = basicManager()->xChain();
    }

}

/*!
  If \a autoBorder is -1, this QBoxLayout will inherit its parent's
  defaultBorder(), otherwise \a autoBorder is used.

  You have to insert this box into another layout before using it.
*/

QBoxLayout::QBoxLayout( Direction d,
			int autoBorder, const char *name )
    : QLayout( autoBorder, name )
{
    pristine = TRUE;
    dir = (QGManager::Direction)d;
    parChain = 0; // debug
    serChain = 0; // debug
}


/*!
  Destroys this box.
*/

QBoxLayout::~QBoxLayout()
{
}

/*!
  Initializes this box.
*/

void QBoxLayout::initGM()
{
    serChain = basicManager()->newSerChain( dir );
    basicManager()->setName( serChain, name() );
    parChain = basicManager()->newParChain( perp( dir ) );
}


/*!
  Adds \a layout to the box, with serial stretch factor \a stretch.

  \sa addWidget(), addSpacing()
*/

void QBoxLayout::addLayout( QLayout *layout, int stretch )
{
    if ( !basicManager() ) {
#if defined(CHECK_STATE)
	warning( "QBoxLayout::addLayout: Box must have a widget parent or be\n"
		 "                       added in another layout before use" );
#endif
	return;
    }
    addChildLayout( layout );
    if ( !pristine && defaultBorder() )
	basicManager()->addSpacing( serChain, defaultBorder(), 0,
				    defaultBorder() );
    addB( layout, stretch );
    pristine = FALSE;
}

void QBoxLayout::addB( QLayout * l, int stretch )
{
    if ( horz( dir ) ) {
	basicManager()->QGManager::add( parChain, verChain( l ) );
	basicManager()->QGManager::add( serChain, horChain( l ), stretch );
    } else {
	basicManager()->QGManager::add( parChain, horChain( l ) );
	basicManager()->QGManager::add( serChain, verChain( l ),
					    stretch );
    }
}


/*!
  Returns the main vertical chain, so that a box can be put into
  other boxes (or other types of QLayout).
*/

QChain * QBoxLayout::mainVerticalChain()
{
    if ( horz(dir) )
	return parChain;
    else
	return serChain;
}

/*!
  Returns the main horizontal chain, so that a box can be put into
  other boxes (or other types of QLayout).
*/

QChain * QBoxLayout::mainHorizontalChain()
{
    if ( horz(dir) )
	return serChain;
    else
	return parChain;
}

/*!
  Adds a non-stretchable space with size \a size.  QBoxLayout gives
  default border and spacing. This function adds additional space.

  \sa addStretch()
*/
void QBoxLayout::addSpacing( int size )
{
    if ( !basicManager() ) {
#if defined(CHECK_STATE)
	warning("QBoxLayout::addSpacing: Box must have a widget parent or be\n"
		"                        added in another layout before use.");
#endif
	return;
    }
    basicManager()->addSpacing( serChain, size, 0, size );
}

/*!
  Adds a stretchable space with zero minimum size
  and stretch factor \a stretch.

  \sa addSpacing()
*/
//###... Should perhaps replace default space?
void QBoxLayout::addStretch( int stretch )
{
    if ( !basicManager() ) {
#if defined(CHECK_STATE)
	warning("QBoxLayout::addStretch: Box must have a widget parent or be\n"
		 "                       added in another layout before use.");
#endif
	return;
    }
    basicManager()->addSpacing( serChain, 0, stretch );
}

/*!
  Limits the perpendicular dimension of the box (e.g. height if the
  box is LeftToRight) to a minimum of \a size. Other constraints may
  increase the limit.
*/

void QBoxLayout::addStrut( int size )
{
    if ( !basicManager() ) {
#if defined(CHECK_STATE)
	warning( "QBoxLayout::addStrut: Box must have a widget parent or be\n"
		 "                      added in another layout before use." );
#endif
	return;
    }
    basicManager()->addSpacing( parChain, size );
}

/*!
  Adds \a widget to the box, with stretch factor \a stretch and
  alignment \a align.

  The stretch factor applies only in the \link direction() direction
  \endlink of the QBoxLayout, and is relative to the other boxes and
  widgets in this QBoxLayout.  Widgets and boxes with higher stretch
  factor grow more.

  If the stretch factor is 0 and nothing else in the QBoxLayout can
  grow at all, the widget may still grow up to its \link
  QWidget::setMaximumSize() maximum size. \endlink

  Alignment is perpendicular to direction(), alignment in the
  serial direction is done with addStretch().

  For horizontal boxes,	 the possible alignments are
  <ul>
  <li> \c AlignCenter centers vertically in the box.
  <li> \c AlignTop aligns to the top border of the box.
  <li> \c AlignBottom aligns to the bottom border of the box.
  </ul>

  For vertical boxes, the possible alignments are
  <ul>
  <li> \c AlignCenter centers horizontally in the box.
  <li> \c AlignLeft aligns to the left border of the box.
  <li> \c AlignRight aligns to the right border of the box.
  </ul>

  Alignment only has effect if the size of the box is greater than the
  widget's maximum size.

  \sa addLayout(), addSpacing()
*/

#if QT_VERSION == 200
#error "Binary compatibility."
#endif


void QBoxLayout::addWidget( QWidget *widget, int stretch, int align )
{
    if ( !basicManager() ) {
#if defined(CHECK_STATE)
	warning( "QBoxLayout::addLayout: Box must have a widget parent or be\n"
		 "                       added in another layout before use.");
#endif
	return;
    }

    if ( !widget ) {
#if defined(CHECK_NULL)
	warning( "QBoxLayout::addWidget: Widget can't be null" );
#endif
	return;
    }

    const int first = AlignLeft | AlignTop;
    const int last  = AlignRight | AlignBottom;

    if ( !pristine && defaultBorder() )
	basicManager()->addSpacing( serChain, defaultBorder(), 0,
				    defaultBorder() );

    if ( 0/*a == alignBoth*/ ) {
	basicManager()->addWidget( parChain, widget, 0 );
    } else {
	QGManager::Direction d = perp( dir );
	QChain *sc = basicManager()->newSerChain( d );
	QString n;
	n.sprintf( "%s-alignment", name( "widget" ) );
	basicManager()->setName( serChain, n );
	if ( align & last || align & AlignCenter ) {
	    basicManager()->addSpacing(sc, 0);
	}
	basicManager()->addWidget( sc, widget, 1 );
	if ( align & AlignCenter || align & first ) {
	    basicManager()->addSpacing(sc, 0);
	}
	basicManager()->add( parChain, sc );
    }
    basicManager()->addWidget( serChain, widget, stretch );
    pristine = FALSE;
}


/*!
  \fn QBoxLayout::Direction QBoxLayout::direction() const

  Returns the (serial) direction of the box. addWidget(), addBox()
  and addSpacing() works in this direction; the stretch stretches
  in this direction. \link QBoxLayout::addWidget Alignment \endlink
  works perpendicular to this direction.

  The directions are \c LeftToRight, \c RightToLeft, \c TopToBottom
  and \c BottomToTop. For the last two, the shorter aliases \c Down and
  \c Up are also available.

  \sa addWidget(), addBox(), addSpacing()
*/




/*!
  \class QHBoxLayout qlayout.h

  \brief The QHBoxLayout class lines up child widgets horizontally.

  \ingroup geomanagement

  This class provides an easier way to construct horizontal box layout
  objects.  See \l QBoxLayout for more details.

  The simplest way to use this class is:

  \code
     QBoxLayout * l = new QHBoxLayout( widget );
     l->addWidget( aWidget );
     l->addWidget( anotherWidget );
     l->activate()
  \endcode

  \sa QVBoxLayout QGridLayout
*/


/*!
  Creates a new top-level horizontal box.
 */
QHBoxLayout::QHBoxLayout( QWidget *parent, int border,
			  int autoBorder, const char *name )
    : QBoxLayout( parent, LeftToRight, border, autoBorder, name )
{

}

/*!
  Creates a new horizontal box. You have to add it to another
  layout before using it.
 */
QHBoxLayout::QHBoxLayout( int autoBorder, const char *name )
    :QBoxLayout( LeftToRight, autoBorder, name )
{
}


/*!
  Destroys this box.
*/

QHBoxLayout::~QHBoxLayout()
{
}



/*!
  \class QVBoxLayout qlayout.h

  \brief The QVBoxLayout class lines up child widgets vertically.

  \ingroup geomanagement

  This class provides an easier way to construct vertical box layout
  objects.  See \l QBoxLayout for more details.

  The simplest way to use this class is:

  \code
     QBoxLayout * l = new QVBoxLayout( widget );
     l->addWidget( aWidget );
     l->addWidget( anotherWidget );
     l->activate()
  \endcode

  \sa QHBoxLayout QGridLayout
*/

/*!
  Creates a new top-level vertical box.
 */
QVBoxLayout::QVBoxLayout( QWidget *parent, int border,
			  int autoBorder, const char *name )
    : QBoxLayout( parent, TopToBottom, border, autoBorder, name )
{

}

/*!
  Creates a new vertical box. You have to add it to another
  layout before using it.
 */
QVBoxLayout::QVBoxLayout( int autoBorder, const char *name )
    :QBoxLayout( TopToBottom, autoBorder, name )
{
}

/*!
  Destroys this box.
*/

QVBoxLayout::~QVBoxLayout()
{
}

/*!
  \class QGridLayout qlayout.h

  \brief The QGridLayout class lays out child widgets in a grid.

  \ingroup geomanagement

  QGridLayout takes the space it gets (from its parent layout or from
  the mainWidget()), divides it up into rows and columns, and puts
  each of the child widgets it manages into the correct cell(s).

  Columns and rows behave identically; we will discuss columns but
  there are eqivalent functions for rows.

  Each column has a minimum width and a stretch factor.  The minimum
  width is the greatest of that set using addColSpacing() and the
  minimum width of each widget in that column.  The stretch factor is
  set using setColStretch() and determines how much of the available
  space the column will get, over and above its necessary minimum.

  Normally, each child widget or layout is put into a cell of its own
  using addWidget() or addLayout(), but you can also put widget into
  multiple cells using addMultiCellWidget().  However, if you do that,
  QGridLayout does not take the child widget's minimum size into
  consideration (because it cannot know what column the minimum
  width should belong to).  Thus you must set the minimum width of
  each column using addColSpacing().

  This illustration shows a fragment of a dialog with a five-column,
  three-row grid (the grid is shown overlaid in magenta):

  <img src="gridlayout.gif" width="425" height="150">

  Columns 0, 2 and 4 in this dialog fragment are made up of a QLabel,
  a QLineEdit and a QListBox.  Columns 1 and 2 are placeholders, made
  with setColSpacing().  Row 0 consists of three QLabel objects, row 1
  of three QLineEdit objects and row 2 of three QListBox objects.

  Since we did not want any space between the rows, we had to use
  placeholder columns to get the right amount of space between the
  columns.

  Note that the columns and rows are not equally wide/tall: If you
  want two columns to be equally wide, you must set the columns'
  minimum widths and stretch factors to be the same yourself.  You do
  this using addColSpacing() and setStretch().

  If the QGridLayout is not the top-level layout (ie. is not managing
  all of the widget's area and children), you must add it to its
  parent layout when you have created it, but before you can do
  anything with it.  The normal way to add a layout is by calling
  parentLayout->addLayout().

  Once you have done that, you can start putting widgets and other
  layouts in the cells of your grid layout using addWidget(),
  addLayout() and addMultiCellWidget().

  Finally, if the grid is the top-level layout, you activate() it.

  QGridLayout also includes two margin widths: The border width and
  the inter-box width.  The border width is the width of the reserved
  space along each of the QGridLayout's four sides.  The intra-widget
  width is the width of the automatically allocated spacing between
  neighbouring boxes.

  The border width defaults to 0, and the intra-widget width defaults
  to the same as the border width.  Both are set using arguments to
  the constructor.
*/


/*! \base64 gridlayout.gif

  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

*/

/*!
  Constructs a new QGridLayout with \a nRows rows, \a nCols columns
   and main widget \a  parent.	\a parent may not be 0.

  \a border is the number of pixels between the edge of the widget and
  the managed children.	 \a autoBorder is the default number of pixels
  between cells.  If \a autoBorder is -1 the value
  of \a border is used.

  \a name is the internal object name.
*/

QGridLayout::QGridLayout( QWidget *parent, int nRows, int nCols, int border ,
			  int autoBorder , const char *name )
    : QLayout( parent, border, autoBorder, name )
{
    horChain = basicManager()->newSerChain( QGManager::LeftToRight );
    basicManager()->setName( horChain, name );
    verChain = basicManager()->newSerChain( QGManager::Down );
    basicManager()->setName( verChain, name );
    basicManager()->add( basicManager()->xChain(), horChain );
    basicManager()->add( basicManager()->yChain(), verChain );
    init( nRows, nCols );
}


/*!
  Constructs a new grid with \a nRows rows and \a  nCols columns,
  If \a autoBorder is -1, this QGridLayout will inherits its parent's
  defaultBorder(), otherwise \a autoBorder is used.

  You have to insert this grid into another layout before using it.
*/

QGridLayout::QGridLayout( int nRows, int nCols,
			  int autoBorder, const char *name )
     : QLayout( autoBorder, name )
{
    rr = nRows;
    cc = nCols;
}


/*!
  Deletes this grid. Geometry management is terminated if
  this is a top-level grid.
*/

QGridLayout::~QGridLayout()
{
    delete rows;
    delete cols;
}

/*!
  \fn int QGridLayout::numRows() const
  Returns the number of rows in this grid.
  */


/*!
  \fn int QGridLayout::numCols() const
  Returns the number of columns in this grid.
  */

/*!
  Expands this grid so that it will have \a nRows rows and \a nCols columns.
  Will not shrink the grid.
 */
void QGridLayout::expand( int nRows, int nCols )
{
    int nr = QMAX( rr, nRows );
    int nc = QMAX( cc, nCols );

    if ( !rows )
	rows = new QArray<QChain*> ( nr );
    if ( !cols )
	cols = new QArray<QChain*> ( nc );

    if ( rr == nr && cc == nc )
	return;

    if ( nr > rr ) {
	rows->resize( nr );
	for ( int i = rr; i < nr; i++ ) {
	    if ( i != 0 )
		basicManager()->addSpacing( verChain, defaultBorder(), 0,
					    defaultBorder() );
	    (*rows)[i] = basicManager()->newParChain( QGManager::Down );
	    basicManager()->add( verChain, (*rows)[i] );
	}
    }


    if ( nc > cc ) {
	cols->resize( nc );
	for ( int i = cc; i < nc; i++ ) {
	    if ( i != 0 )
		basicManager()->addSpacing( horChain, defaultBorder(), 0,
					    defaultBorder() );
	    (*cols)[i] = basicManager()->newParChain( QGManager::LeftToRight );
	    basicManager()->add( horChain, (*cols)[i] );
	}
    }

    rr = nr;
    cc = nc;
}

/*!
  Initializes this grid.
*/

void QGridLayout::initGM()
{
    horChain = basicManager()->newSerChain( QGManager::LeftToRight );
    basicManager()->setName( horChain, name( "QGridLayout" ) );
    verChain = basicManager()->newSerChain( QGManager::Down );
    basicManager()->setName( verChain, name( "QGridLayout" ) );
    init( rr, cc );
}


/*!
  Sets up the table and other internal stuff
*/

void QGridLayout::init( int nRows, int nCols )
{
    rows = 0;
    cols = 0;
    rr = 0;
    cc = 0;
    expand( nRows, nCols );
}


/*!
  Adds the widget \a w to the cell grid at \a row, \a col.
  The top left position is (0,0)

  Alignment is specified by \a align which takes the same arguments as
  QLabel::setAlignment().  Note that widgets take all the space they
  can get; alignment has no effect unless you have set
  QWidget::maximumSize().

*/

void QGridLayout::addWidget( QWidget *w, int row, int col, int align )
{
    if ( !basicManager() ) {
#if defined(CHECK_STATE)
       warning("QGridLayout::addWidget: Grid must have a widget parent or be\n"
	       "                        added in another layout before use." );
#endif
	return;
    }
    if ( rows->size() == 0 || cols->size() == 0   ) {
#if defined(CHECK_RANGE)
	warning( "QGridLayout::addWidget: Zero sized grid" );
#endif
	return;
    }

    addMultiCellWidget( w, row, row, col, col, align );
}

/*!
  Adds the widget \a w to the cell grid, spanning multiple rows/columns.

  Note that multicell widgets do not influence the minimum or maximum
  size of columns/rows they span. Use addColSpacing() or addRowSpacing()
  to set minimum sizes explicitly.

  Alignment is specified by \a align which takes the same arguments as
  QLabel::setAlignment(), alignment has no effect unless you have set
  QWidget::maximumSize().
*/

void QGridLayout::addMultiCellWidget( QWidget *w, int fromRow, int toRow,
				      int fromCol, int toCol, int align	 )
{
    if ( !basicManager() ) {
#if defined(CHECK_STATE)
	warning( "QGridLayout::addMultiCellWidget: "
		 "Grid must have a widget parent or be\n"
		 "        added in another layout before use." );
#endif
	return;
    }
    if ( rows->size() == 0 || cols->size() == 0   ) {
#if defined(CHECK_RANGE)
	warning( "QGridLayout::addMultiCellWidget: Zero sized grid" );
#endif
	return;
    }
    const int hFlags = AlignHCenter | AlignLeft | AlignRight;
    const int vFlags = AlignVCenter | AlignTop | AlignBottom;

    int a = align & hFlags;

    QChain *c;
    if ( a || fromCol != toCol ) {
	c = basicManager()->newSerChain( QGManager::LeftToRight );
	if ( fromCol == toCol )
	    basicManager()->add( (*cols)[ fromCol ], c );
	else
	    basicManager()->addBranch( horChain, c, fromCol*2, toCol*2 );
    } else {
	c =  (*cols)[ fromCol ];
    }
    if ( a & (AlignHCenter|AlignRight) )
	basicManager()->addSpacing( c, 0 );
    basicManager()->addWidget( c, w, 1 ); //stretch ignored in parallel chain
    if ( a & (AlignHCenter|AlignLeft) )
	basicManager()->addSpacing( c, 0 );

    // vertical dimension:
    a = align & vFlags;
    if ( a || fromRow != toRow ) {
	c = basicManager()->newSerChain( QGManager::Down );
	if ( fromRow == toRow )
	    basicManager()->add( (*rows)[ fromRow ], c );
	else
	    basicManager()->addBranch( verChain, c, fromRow*2, toRow*2 );
    } else {
	c =  (*rows)[ fromRow ];
    }
    if ( a & (AlignVCenter|AlignBottom) )
	basicManager()->addSpacing( c, 0 );
    basicManager()->addWidget( c, w, 1 ); //stretch ignored in parallel chain
    if ( a & (AlignVCenter|AlignTop) )
	basicManager()->addSpacing( c, 0 );
}


/*!
  Places another layout at position (\a row, \a col) in the grid.
  The top left position is (0,0)
*/

void QGridLayout::addLayout( QLayout *layout, int row, int col)
{
    if ( !basicManager() ) {
#if defined(CHECK_STATE)
       warning("QGridLayout::addLayout: Grid must have a widget parent or be\n"
	       "                        added in another layout before use." );
#endif
	return;
    }
    if ( rows->size() == 0 || cols->size() == 0   ) {
#if defined(CHECK_RANGE)
	warning( "QGridLayout::addLayout: Zero sized grid" );
#endif
	return;
    }
    addChildLayout( layout );
    QChain *c =	 (*cols)[ col ];
    basicManager()->add( c, QLayout::horChain( layout ) );
    c =	 (*rows)[ row ];
    basicManager()->add( c, QLayout::verChain( layout ) );
}


/*!
  Sets the stretch factor of row \a row to \a stretch.
  The first row is number 0.

  The stretch factor  is relative to the other rows in this grid.
  Rows with higher stretch factor take more of the available space.

  The default stretch factor is 0.
  If the stretch factor is 0 and no other row in this table can
  grow at all, the row may still grow.
*/

void QGridLayout::setRowStretch( int row, int stretch )
{
    if ( !basicManager() ) {
#if defined(CHECK_STATE)
	warning( "QGridLayout::setRowStretch: Grid must have a widget parent\n"
		 "        or be added in another layout before use.");
#endif
	return;
    }
    if ( rows->size() == 0 ) {
#if defined(CHECK_RANGE)
	warning( "QGridLayout::setRowStretch: Zero sized grid" );
#endif
	return;
    }

    QChain *c =	 (*rows)[ row ];
    basicManager()->setStretch( c, stretch );
}


/*!
  Sets the stretch factor of column \a col to \a stretch.
  The first column is number 0.

  The stretch factor  is relative to the other columns in this grid.
  Columns with higher stretch factor take more of the available space.

  The default stretch factor is 0.
  If the stretch factor is 0 and no other column in this table can
  grow at all, the column may still grow.
*/

void QGridLayout::setColStretch( int col, int stretch )
{
    if ( !basicManager() ) {
#if defined(CHECK_STATE)
	warning( "QGridLayout::setColStretch: Grid must have a widget parent\n"
		 "        or be added in another layout before use.");
#endif
	return;
    }
    if ( cols->size() == 0 ) {
#if defined(CHECK_RANGE)
	warning( "QGridLayout::setColStretch: Zero sized grid" );
#endif
	return;
    }
    QChain *c =	 (*cols)[ col ];
    basicManager()->setStretch( c, stretch );
}


/*!
  Sets the minimum height of \a row to \a minsize pixels.
 */
void QGridLayout::addRowSpacing( int row, int minsize )
{
    if ( !basicManager() ) {
#if defined(CHECK_STATE)
	warning( "QGridLayout::setColStretch: Grid must have a widget parent\n"
		 "        or be added in another layout before use.");
#endif
	return;
    }
    if ( rows->size() == 0 ) {
#if defined(CHECK_RANGE)
	warning( "QGridLayout::addRowSpacing: Zero sized grid" );
#endif
	return;
    }

    QChain *c =	 (*rows)[ row ];
    basicManager()->addSpacing( c, minsize );
}

/*!
  Sets the minimum width of \a col to \a minsize pixels.
 */
void QGridLayout::addColSpacing( int col, int minsize )
{
    if ( !basicManager() ) {
#if defined(CHECK_STATE)
	warning( "QGridLayout::setColStretch: Grid must have a widget parent\n"
		 "        or be added in another layout before use.");
#endif
	return;
    }
    if ( cols->size() == 0 ) {
#if defined(CHECK_RANGE)
	warning( "QGridLayout::setColStretch: Zero sized grid" );
#endif
	return;
    }
    QChain *c =	 (*cols)[ col ];
    basicManager()->addSpacing( c, minsize );
}

/*!
  \fn QChain *QGridLayout::mainVerticalChain()
  This function returns the main vertical chain.
*/

/*!
  \fn QChain *QGridLayout::mainHorizontalChain()
  This function returns the main horizontal chain.
*/
