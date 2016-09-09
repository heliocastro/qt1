/**********************************************************************
** $Id: qlistbox.cpp,v 2.71.2.11 1999/02/13 15:50:58 hanord Exp $
**
** Implementation of QListBox widget class
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

#include "qlistbox.h"
#include "qfontmetrics.h"
#include "qpainter.h"
#include "qstrlist.h"
#include "qkeycode.h"
#include "qscrollbar.h"
#include "qpixmap.h"
#include "qapplication.h"

Q_DECLARE(QListM, QListBoxItem);

class QLBItemList : public QListM(QListBoxItem) // internal class
{
    int compareItems( GCI i1, GCI i2);
public:
    int timerId;				//### bincomp
};

int QLBItemList::compareItems( GCI i1, GCI i2)
{
    QListBoxItem *lbi1 = (QListBoxItem *)i1;
    QListBoxItem *lbi2 = (QListBoxItem *)i2;
    return strcmp( lbi1->text(), lbi2->text() );
}


static inline bool checkInsertIndex( const char *method, const char * name,
				     int count, int *index)
{
    bool range_err = (*index > count);
#if defined(CHECK_RANGE)
    if ( *index > count )
	warning( "QListBox::%s: (%s) Index %i out of range",
		 method, name ? name : "<no name>", *index );
#endif
    if ( *index < 0 )				// append
	*index = count;
    return !range_err;
}

static inline bool checkIndex( const char *method, const char * name,
			       int count, int index )
{
    bool range_err = ((uint)index >= (uint)count);
#if defined(CHECK_RANGE)
    if ( range_err )
	warning( "QListBox::%s: (%s) Index %d out of range",
		 method, name ? name : "<no name>", index );
#endif
    return !range_err;
}


/*!
  \class QListBoxItem qlistbox.h
  \brief This is the base class of all list box items.

  This class is the abstract base class of all list box items. If you
  need to insert customized items into a QListBox, you must inherit
  this class and reimplement paint(), height() and width().

  The following shows how to define a list box item which shows a
  pixmap and a text:
  \code
    class MyListBoxItem : public QListBoxItem
    {
    public:
	MyListBoxItem( const char *s, const QPixmap p )
	    : QListBoxItem(), pm(p)
	    { setText( s ); }

    protected:
	virtual void paint( QPainter * );
	virtual int height( const QListBox * ) const;
	virtual int width( const QListBox * ) const;
	virtual const QPixmap *pixmap() { return &pm; }

    private:
	QPixmap pm;
    };

    void MyListBoxItem::paint( QPainter *p )
    {
	p->drawPixmap( 3, 0, pm );
	QFontMetrics fm = p->fontMetrics();
	int yPos;			// vertical text position
	if ( pm.height() < fm.height() )
	    yPos = fm.ascent() + fm.leading()/2;
	else
	    yPos = pm.height()/2 - fm.height()/2 + fm.ascent();
	p->drawText( pm.width() + 5, yPos, text() );
    }

    int MyListBoxItem::height(const QListBox *lb ) const
    {
	return QMAX( pm.height(), lb->fontMetrics().lineSpacing() + 1 );
    }

    int MyListBoxItem::width(const QListBox *lb ) const
    {
	return pm.width() + lb->fontMetrics().width( text() ) + 6;
    }
  \endcode

  \sa QListBox
*/


/*!
  Constructs an empty list box item.
*/

QListBoxItem::QListBoxItem()
{
    selected = FALSE;
}

/*!
  Destroys the list box item.
*/

QListBoxItem::~QListBoxItem()
{
}


/*!
  \fn void QListBoxItem::paint( QPainter *p )

  Implement this function to draw your item.

  \sa height(), width()
*/

/*!
  \fn int QListBoxItem::height( const QListBox * ) const

  Implement this function to return the height of your item

  \sa paint(), width()
*/

/*!
  \fn int QListBoxItem::width(	const QListBox * ) const

  Implement this function to return the width of your item

  \sa paint(), height()
*/

/*!
  \fn const char *QListBoxItem::text() const

  Returns the text of the item, which is used for sorting.

  \sa setText()
*/

/*!
  \fn const QPixmap *QListBoxItem::pixmap() const

  Returns the pixmap connected with the item, if any.

  The default implementation of this function returns a null pointer.
*/


/*!
  \fn void QListBoxItem::setText( const char *text )

  Sets the text of the widget, which is used for sorting.
  The text is not shown unless explicitly drawn in paint().

  \sa text()
*/


/*!
  \class QListBoxText qlistbox.h
  \brief The QListBoxText class provides list box items with text.

  The text is drawn in the widget's current font. If you need several
  different fonts, you have to make your own subclass of QListBoxItem.

  \sa QListBox, QListBoxItem
*/


/*!
  Constructs a list box item showing the text \e text.
*/

QListBoxText::QListBoxText( const char *text )
    :QListBoxItem()
{
    setText( text );
}

/*!
  Destroys the item.
*/

QListBoxText::~QListBoxText()
{
}

/*!
  Draws the text using painter \e p.
*/

void QListBoxText::paint( QPainter *p )
{
    QFontMetrics fm = p->fontMetrics();
    p->drawText( 3,  fm.ascent() + fm.leading()/2, text() );
}

/*!
  Returns the height of a line of text.

  \sa paint(), width()
*/

int QListBoxText::height( const QListBox *lb ) const
{
    if ( lb )
	return lb->fontMetrics().lineSpacing() + 2;
    return -1;
}

/*!
  Returns the width of this line.

  \sa paint(), height()
*/

int QListBoxText::width( const QListBox *lb ) const
{
    if ( lb )
	return lb->fontMetrics().width( text() ) + 6;
    return -1;
}


/*!
  \class QListBoxPixmap qlistbox.h
  \brief The QListBoxPixmap class provides list box items with a pixmap.

  \sa QListBox, QListBoxItem
*/

/*!
  Creates a new list box item showing the pixmap \e pixmap.
*/

QListBoxPixmap::QListBoxPixmap( const QPixmap &pixmap )
    : QListBoxItem()
{
    pm = pixmap;
}

/*!
  Destroys the item.
*/

QListBoxPixmap::~QListBoxPixmap()
{
}

/*!
  \fn const QPixmap *QListBoxPixmap::pixmap() const

  Returns the pixmap connected with the item.
*/


/*!
  Draws the pixmap using painter \e p.
*/

void QListBoxPixmap::paint( QPainter *p )
{
    p->drawPixmap( 3, 0, pm );
}

/*!
  Returns the height of the pixmap.

  \sa paint(), width()
*/

int QListBoxPixmap::height( const QListBox * ) const
{
    return pm.height();
}

/*!
  Returns the width of the pixmap.

  \sa paint(), height()
*/

int QListBoxPixmap::width( const QListBox * ) const
{
    return pm.width() + 6;
}


/*!
  \class QListBox qlistbox.h
  \brief The QListBox widget provides a single-column list of items that
  can be scrolled.

  \ingroup realwidgets

  Each item in a QListBox contains a QListBoxItem.  One of the items
  can be the current item.  The highlighted() signal is emitted when
  the user highlights a new current item; selected() is emitted when
  the user double-clicks on an item or presses return when an item is
  highlighted.

  If the user does not select anything, no signals are emitted and
  currentItem() returns -1.

  A list box has \c StrongFocus as a default focusPolicy(), i.e. it can
  get keyboard focus both by tabbing and clicking.

  New items may be inserted using either insertItem(), insertStrList()
  and inSort().  The list box is automatically updated to reflect the
  change; if you are going to insert a lot of data it may be
  worthwhile to wrap the insertion in calls to setAutoUpdate():

  \code
      listBix->setAutoUpdate( FALSE );
      for( i=1; i< hugeArray->count(); i++ )
          listBox->insertItem( hugeArray[i] );
      listBox->setAutoUpdate( TRUE );
      listBox->repaint();
  \endcode

  Each change to insertItem() normally causes a screen update, and for
  a large change only a few of those updates are really necessary.  Be
  careful to call repaint() when you re-enable updates, so the widget
  is completely repainted.

  By default, vertical and horizontal scroll bars are added and
  removed as necessary.	 setAutoScrollBar() can be used to force a
  specific policy.

  If you need to insert other types than texts and pixmaps, you must
  define new classes which inherit QListBoxItem.

  \warning The list box assumes ownership of all list box items
  and will delete them when they are not needed.

  <img src=qlistbox-m.gif> <img src=qlistbox-w.gif>

  \sa QListView QComboBox QButtonGroup
  <a href="guibooks.html#fowler">GUI Design Handbook: List Box (two
  sections)</a>
*/



//### How to provide new member variables while keeping binary compatibility:

#if QT_VERSION == 200
#error "Remove QListBox dict."
#endif

#include "qintdict.h"

static QIntDict<int> *qlb_maxLenDict = 0;

static void cleanupListbox()
{
    delete qlb_maxLenDict;
    qlb_maxLenDict = 0;
}


/*!
  Constructs a list box.  The arguments are passed directly to the
  QTableView constructor.

  Note that the \a f argument is \e not \link setTableFlags() table
  flags \endlink but rather \link QWidget::QWidget() widget
  flags. \endlink

*/

QListBox::QListBox( QWidget *parent, const char *name, WFlags f )
    : QTableView( parent, name, f )
{
    doDrag	  = TRUE;
    doAutoScroll  = TRUE;
    current	  = -1;
    isTiming	  = FALSE;
    stringsOnly	  = TRUE;
    multiSelect   = FALSE;
    goingDown	  = FALSE;
    itemList	  = new QLBItemList;
    CHECK_PTR( itemList );
    itemList->timerId = 0;
    setCellWidth( 0 );
    QFontMetrics fm = fontMetrics();
    setCellHeight( fm.lineSpacing() + 1 );
    setNumCols( 1 );
    setTableFlags( Tbl_autoVScrollBar|Tbl_autoHScrollBar | //Tbl_snapToVGrid |
		   Tbl_smoothVScrolling | Tbl_clipCellPainting  );
    switch ( style() ) {
	case WindowsStyle:
	case MotifStyle:
	    setFrameStyle( QFrame::WinPanel | QFrame::Sunken );
	    setBackgroundMode( PaletteBase );
	    break;
	default:
	    setFrameStyle( QFrame::Panel | QFrame::Plain );
	    setLineWidth( 1 );
    }
    setFocusPolicy( StrongFocus );
    if ( !qlb_maxLenDict ) {
	qlb_maxLenDict = new QIntDict<int>;
	CHECK_PTR( qlb_maxLenDict );
	qAddPostRoutine( cleanupListbox );
    }
}

#if QT_VERSION == 200
#error "Remove QListBox pointer."
#endif

static QListBox * changedListBox = 0;

/*!
  Destroys the list box.  Deletes all list box items.
*/

QListBox::~QListBox()
{
    if ( changedListBox == this )
	changedListBox = 0;
    goingDown = TRUE;
    clearList();
    if ( qlb_maxLenDict )
	qlb_maxLenDict->remove( (long)this );
    delete itemList;
}

/*! \fn void QListBox::selectionChanged()

  This signal is emitted when the selection set of a multiple-choice
  listbox changes. If the user selects five items by drag-selecting,
  QListBox tries to emit just one selectionChanged() signal, so the
  signal can be connected to computationally expensive slots.

  \sa selected() currentItem()
*/

/*! \fn void QListBox::highlighted( int index )

  This signal is emitted when the user highlights a new current item.
  The argument is the index of the new item, which is already current.

  \sa selected() currentItem() selectionChanged()
*/

/*! \fn void QListBox::highlighted( const char * )

  This signal is emitted when the user highlights a new current item
  and the new item is a string.  The argument is the text of the
  new current item.

  \sa selected() currentItem() selectionChanged()
*/

/*! \fn void QListBox::selected( int index )

  This signal is emitted when the user double-clicks on an item or
  presses return when an item is highlighted.  The argument is the
  index of the selected item.

  \sa highlighted() selectionChanged()
*/

/*! \fn void QListBox::selected( const char * )

  This signal is emitted when the user double-clicks on an item or
  presses return while an item is highlighted, and the selected item
  is (or has) a string.  The argument is the text of the selected
  item.

  \sa highlighted() selectionChanged()
*/

/*!
  Reimplements QWidget::setFont() to update the list box line height
  and maxItemWidth().
*/

void QListBox::setFont( const QFont &font )
{
    QWidget::setFont( font );
    if ( stringsOnly )
	setCellHeight( fontMetrics().lineSpacing() + 1 );
    // else ...?

    updateCellWidth();
}


/*!
  Returns the number of items in the list box.
*/

uint QListBox::count() const
{
    return itemList->count();
}


/*!
  Inserts the string list \e list into the list at item \e index.

  If \e index is negative, \e list is inserted at the end of the list.	If
  \e index is too large, the operation is ignored.

  \sa insertItem(), inSort()
*/

void QListBox::insertStrList( const QStrList *list, int index )
{
    if ( !checkInsertIndex( "insertStrList", name(), count(), &index ) )
	return;
    if ( !list ) {
#if defined(CHECK_NULL)
	ASSERT( list != 0 );
#endif
	return;
    }
    QStrListIterator it( *list );
    const char *txt;
    if ( index < 0 )
	index = itemList->count();
    while ( (txt=it.current()) ) {
	++it;
	insert( new QListBoxText(txt), index++, FALSE );
    }
    if ( currentItem() < 0 && numRows() > 0 && hasFocus() )
	setCurrentItem( 0 );
    updateNumRows( TRUE );
    if ( autoUpdate() && isVisible() )
	repaint();
}

/*!
  Inserts the \e numStrings strings of the array \e strings into the
  list at item\e index.

  If \e index is negative, insertStrList() inserts \e strings at the end
  of the list.	If \e index is too large, the operation is ignored.

  \sa insertItem(), inSort()
*/

void QListBox::insertStrList( const char **strings, int numStrings, int index )
{
    if ( !checkInsertIndex( "insertStrList", name(), count(), &index ) )
	return;
    if ( !strings ) {
#if defined(CHECK_NULL)
	ASSERT( strings != 0 );
#endif
	return;
    }
    if ( index < 0 )
	index = itemList->count();
    int i = 0;
    while ( (numStrings<0 && strings[i]!=0) || i<numStrings ) {
	insert( new QListBoxText(strings[i]), index + i, FALSE );
	i++;
    }
    updateNumRows( TRUE );
    if ( currentItem() < 0 && numRows() > 0 && hasFocus() )
	setCurrentItem( 0 );
    if ( autoUpdate() && isVisible() )
	repaint();
}


/*!
  Inserts the item \e lbi into the list at \e index.

  If \e index is negative or larger than the number of items in the list
  box, \e lbi is inserted at the end of the list.

  \sa insertStrList()
*/

void QListBox::insertItem( const QListBoxItem *lbi, int index )
{
    if ( !checkInsertIndex( "insertItem", name(), count(), &index ) )
	return;
    if ( !lbi ) {
#if defined ( CHECK_NULL )
	ASSERT( lbi != 0 );
#endif
	return;
    }
    if ( stringsOnly ) {
	stringsOnly = FALSE;
	setCellHeight( 0 );
    }
    insert( lbi, index, TRUE );
    updateNumRows( FALSE );
    if ( currentItem() < 0 && numRows() > 0 && hasFocus() )
	setCurrentItem( 0 );
    if ( autoUpdate() )
	repaint();
}

/*!
  Inserts \e text into the list at \e index.

  If \e index is negative, \e text is inserted at the end of the list.

  \sa insertStrList()
*/

void QListBox::insertItem( const char *text, int index )
{
    if ( !checkInsertIndex( "insertItem", name(), count(), &index ) )
	return;
    if ( !text ) {
#if defined ( CHECK_NULL )
	ASSERT( text != 0 );
#endif
	return;
    }
    insert( new QListBoxText(text), index, TRUE );
    updateNumRows( FALSE );
    if ( currentItem() < 0 && numRows() > 0 && hasFocus() )
	setCurrentItem( 0 );
    if ( autoUpdate() && itemVisible(index) ) {
	int x, y;
	colXPos( 0, &x );
	rowYPos( index, &y );
	repaint( x, y, -1, -1 );
    }
}

/*!
  Inserts \e pixmap into the list at \e index.

  If \e index is negative, \e pixmap is inserted at the end of the list.

  \sa insertStrList()
*/

void QListBox::insertItem( const QPixmap &pixmap, int index )
{
    if ( !checkInsertIndex( "insertItem", name(), count(), &index ) )
	return;
    if ( stringsOnly ) {
	stringsOnly = FALSE;
	setCellHeight( 0 );
    }
    insert( new QListBoxPixmap(pixmap), index, TRUE );
    updateNumRows( FALSE );
    if ( currentItem() < 0 && numRows() > 0 && hasFocus() )
	setCurrentItem( 0 );
    if ( autoUpdate() && itemVisible(index) ) {
	int x, y;
	colXPos( index, &x );
	rowYPos( index, &y );
	repaint( x, y, -1, -1 );
    }
}


/*!
  Inserts \e lbi at its sorted position in the list box.

  All items must be inserted with inSort() to maintain the sorting
  order.  inSort() treats any pixmap (or user-defined type) as
  lexicographically less than any string.

  \sa insertItem()
*/

void QListBox::inSort( const QListBoxItem *lbi )
{
    if ( !lbi->text() ) {
#if defined (CHECK_NULL)
	ASSERT( lbi->text() != 0 );
#endif
	return;
    }

    itemList->inSort( lbi );
    int index = itemList->at();
    itemList->remove();
    insertItem( lbi, index );
}


/*!
  \overload void QListBox::inSort( const char *text )
*/

void QListBox::inSort( const char *text )
{
    if ( !text ) {
#if defined ( CHECK_NULL )
	ASSERT( text != 0 );
#endif
	return;
    }
    QListBoxText lbi( text );
    itemList->inSort(&lbi);
    int index = itemList->at();
    itemList->remove();
    insertItem( text, index );
}


/*!
  Removes the item at position \e index. If \a index is equal to
  currentItem(), a new item gets selected and the highlighted()
  signal is emitted.
  \sa insertItem(), clear()
*/

void QListBox::removeItem( int index )
{
    if ( !checkIndex( "removeItem", name(), count(), index ) )
	return;
    bool currentChanged = ( current == index );

    if( current > 0 && (current > index || current >= (int)count()-1) )
	current--;
    bool    updt = autoUpdate() && itemVisible( index );
    QListBoxItem *lbi = itemList->take( index );
    int w             = lbi->width( this );
    updateNumRows( w == cellWidth() );
    delete lbi;
    if ( count() == 0 ) {
	current = -1;
    } else if ( currentChanged ) {
	QString tmp = 0;
	if ( item( currentItem() ) )
	    tmp = item( currentItem() )->text();
	emit highlighted( current );
	if ( !tmp.isNull() )
	    emit highlighted( tmp );
    }
    if ( updt )
	repaint();
}

/*!
  Deletes all items in the list.
  \sa removeItem(), setStrList()
*/

void QListBox::clear()
{
    clearList();
    updateNumRows( TRUE );
    if ( autoUpdate() )
	erase();
}


/*!
  Returns a pointer to the text at position \e index, or 0 if there is no
  text there.
  \sa pixmap()
*/

const char *QListBox::text( int index ) const
{
    if ( (uint)index >= count() )
	return 0;
    return itemList->at(index)->text();
}

/*!
  Returns a pointer to the pixmap at position \e index, or 0 if there is no
  pixmap there.
  \sa text()
*/

const QPixmap *QListBox::pixmap( int index ) const
{
    if ( (uint)index >= count() )
	return 0;
    return itemList->at(index)->pixmap();
}

/*!
  Replaces the item at position \e index with \e text.

  The operation is ignored if \e index is out of range.

  \sa insertItem(), removeItem()
*/

void QListBox::changeItem( const char *text, int index )
{
    if ( !checkIndex( "changeItem", name(), count(), index ) )
	return;
    change( new QListBoxText(text), index );
}

/*!
  Replaces the item at position \e index with \e pixmap.

  The operation is ignored if \e index is out of range.

  \sa insertItem(), removeItem()
*/

void QListBox::changeItem( const QPixmap &pixmap, int index )
{
    if ( !checkIndex( "changeItem", name(), count(), index ) )
	return;
    change( new QListBoxPixmap(pixmap), index );
}


/*!
  Replaces the item at posistion \e index with \e lbi.	If \e
  index is negative or too large, changeItem() does nothing.

  \sa insertItem(), removeItem()
*/

void QListBox::changeItem( const QListBoxItem *lbi, int index )
{
    if ( !checkIndex( "changeItem", name(), count(), index ) )
	return;
    change( lbi, index );
}


/*!
  Returns TRUE if the list box updates itself automatically when
  items are inserted or removed.

  The default setting is TRUE.

  \sa setAutoUpdate()
*/

bool QListBox::autoUpdate() const
{
    return QTableView::autoUpdate();
}

/*!
  Specifies whether the list box should update itself automatically
  when items are inserted or removed.

  Auto-update is enabled by default.

  If \e enable is TRUE, the list box will update itself.  If \e enable
  is FALSE, the list box will not update itself.

  \warning Do not leave the view in this state for a long time
  (i.e. between events ). If the user interacts with the view when
  auto-update is off, strange things can happen.

  \sa autoUpdate()
*/

void QListBox::setAutoUpdate( bool enable )
{
    QTableView::setAutoUpdate( enable );
}

/*!
  Returns the number of visible items.	This may change at any time
  since the user may resize the widget.

  \sa setFixedVisibleLines()
*/

int QListBox::numItemsVisible() const
{
    return (lastRowVisible() - topCell() + 1);
}

/*!
  Returns the index of the current (highlighted) item of the list box,
  or -1 if no item has been selected.

  \sa topItem()
*/

int QListBox::currentItem() const
{
    return current;
}

/*!
  Sets the highlighted item to the item at position \e index in the list.
  The highlighting is moved and the list box scrolled as necessary.
  \sa currentItem()
*/

void QListBox::setCurrentItem( int index )
{
    if ( index == current )
	return;
    if ( !checkIndex( "setCurrentItem", name(), count(), index ) )
	return;
    int oldCurrent = current;
    current	   = index;
    updateItem( oldCurrent );
    updateItem( current, FALSE ); // Do not clear, current marker covers item
    QString tmp = 0;
    if ( item( currentItem() ) )
	tmp = item( currentItem() )->text();
    emit highlighted( current );
    if ( !tmp.isNull() )
	emit highlighted( tmp );
}

/*!
  Scrolls the list box so the current (highlighted) item is
  centered in the list box.
  \sa currentItem(), setTopItem()
*/

void QListBox::centerCurrentItem()
{
    int top;
    if ( stringsOnly )
	top = current - numItemsVisible() / 2; // ###
    else
	top = current - numItemsVisible() / 2;
    if ( top < 0 )
	top = 0;
    int max = maxRowOffset();
    if ( top > max )
	top = max;
    setTopItem( top );
}

/*!
  Returns index of the item that is on the top line of the list box.
  \sa setTopItem(), currentItem()
*/

int QListBox::topItem() const
{
    return topCell();
}

/*!
  Scrolls the list box so the item at position \e index in the list
  becomes the top row of the list box.
  \sa topItem(), centerCurrentItem()
*/

void QListBox::setTopItem( int index )
{
    setTopCell( index );
}

/*!
  Scrolls the list box so the item at position \e index in the list
  becomes the bottom row of the list box.
  \sa setTopItem()
*/

void QListBox::setBottomItem( int index )
{
    int i = index+1;
    int y = 0;
    while (i) y += cellHeight(--i);
    y -= viewHeight();
    setYOffset( y );
}


/*!
  Returns TRUE if drag-selection is enabled, otherwise FALSE.
  \sa setDragSelect(), autoScroll()
*/

bool QListBox::dragSelect() const
{
    return doDrag;
}

/*!
  Sets drag-selection if \e enable is TRUE, or disables it if \e enable
  is FALSE.

  If drag-selection is enabled, the list box will highlight new items when
  the user drags the mouse inside the list box.

  The default setting is TRUE.

  \sa drawSelect(), setAutoScroll()
*/

void QListBox::setDragSelect( bool enable )
{
    doDrag = enable;
}

/*!
  Returns TRUE if auto-scrolling is enabled, otherwise FALSE.
  \sa setAutoScroll, dragSelect()
*/

bool QListBox::autoScroll() const
{
    return doAutoScroll;
}

/*!
  Sets auto-scrolling if \e enable is TRUE, or disables it if \e enable
  is FALSE.

  If auto-scrolling is enabled, the list box will scroll its contents when
  the user drags the mouse outside (below or above) the list  box.
  Auto-scrolling only works if \link setDragSelect() drag-selection\endlink
  is enabled.

  The default setting is TRUE.

  \sa autoScroll(), setDragSelect()
*/

void QListBox::setAutoScroll( bool enable )
{
    doAutoScroll = enable;
}

/*!
  Returns TRUE if the list box has an automatic (vertical) scroll bar.
  \sa setAutoScrollBar(), autoBottomScrollBar()
*/

bool QListBox::autoScrollBar() const
{
    return testTableFlags( Tbl_autoVScrollBar );
}

/*!
  Enables an automatic (vertical) scroll bar if \e enable is TRUE, or disables
  it if \e enable is FALSE.

  If it is enabled, then the list box will get a (vertical) scroll bar if
  the list box items exceed the list box height.

  The default setting is TRUE.

  \sa autoScrollBar(), setScrollBar(), setAutoBottomScrollBar()
*/

void QListBox::setAutoScrollBar( bool enable )
{
    if ( enable )
	setTableFlags( Tbl_autoVScrollBar );
    else
	clearTableFlags( Tbl_autoVScrollBar );
}

/*!
  Returns TRUE if the list box has a (vertical) scroll bar.
  \sa setScrollBar(), autoScrollBar(), bottomScrollBar()
*/

bool QListBox::scrollBar() const
{
    return testTableFlags( Tbl_vScrollBar );
}

/*!
  Enables a (vertical) scroll bar if \e enable is TRUE, or disables it if
  \e enable is FALSE.

  The default setting is FALSE.

  \sa scrollBar(), setAutoScrollBar(), setBottomScrollBar()
*/

void QListBox::setScrollBar( bool enable )
{
    if ( enable )
	setTableFlags( Tbl_vScrollBar );
    else
	clearTableFlags( Tbl_vScrollBar );
}

/*!
  Returns TRUE if the list box has an automatic bottom scroll bar.
  \sa setAutoBottomScrollBar(), autoScrollBar()
*/

bool QListBox::autoBottomScrollBar() const
{
    return testTableFlags( Tbl_autoHScrollBar );
}

/*!
  Enables an automatic bottom scroll bar if \e enable is TRUE, or disables
  it if \e enable is FALSE.

  If it is enabled, then the list box will get a bottom scroll bar if the
  maximum list box item width exceeds the list box width.

  The default setting is TRUE.

  \sa autoBottomScrollBar(), setBottomScrollBar(), setAutoScrollBar()
*/

void QListBox::setAutoBottomScrollBar( bool enable )
{
    if ( enable )
	setTableFlags( Tbl_autoHScrollBar );
    else
	clearTableFlags( Tbl_autoHScrollBar );
}

/*!
  Returns TRUE if the list box has a bottom scroll bar.
  \sa setBottomScrollBar(), autoBottomScrollBar(), scrollBar()
*/

bool QListBox::bottomScrollBar() const
{
    return testTableFlags( Tbl_hScrollBar );
}

/*!
  Enables a bottom scroll bar if \e enable is TRUE, or disables it if
  \e enable is FALSE.

  The default setting is FALSE.

  \sa bottomScrollBar(), setAutoBottomScrollBar(), setScrollBar()
*/

void QListBox::setBottomScrollBar( bool enable )
{
    if ( enable )
	setTableFlags( Tbl_hScrollBar );
    else
	clearTableFlags( Tbl_hScrollBar );
}

/*!
  Returns TRUE if smooth list box scrolling is enabled, otherwise FALSE.
  \sa setSmoothScrolling()
*/

bool QListBox::smoothScrolling() const
{
    return testTableFlags( Tbl_smoothVScrolling );
}

/*!
  Enables smooth list box scrolling if \e enable is TRUE, or disables
  it if \e enable is FALSE.

  The default setting is TRUE.

  \sa smoothScrolling()
*/

void QListBox::setSmoothScrolling( bool enable )
{
    if ( enable )
	setTableFlags( Tbl_smoothVScrolling );
    else
	clearTableFlags( Tbl_smoothVScrolling );
}


/*!
  Returns a pointer to the item at position \e index.
*/

QListBoxItem *QListBox::item( int index ) const
{
    if (!checkIndex( "item", name(), count(), index ) )
	return 0;
    return itemList->at( index );
}

/*!
  Returns the height of the item at position \e index in pixels.
*/

int QListBox::cellHeight( int index )
{
    if ( stringsOnly )
	return QTableView::cellHeight();
    QListBoxItem *lbi = item( index );
    return lbi ? lbi->height(this) : 0;
}


/*!
  Returns the standard item height (in pixels), or -1 if the list box has
  variable item height.
*/

int QListBox::itemHeight() const
{
    return stringsOnly ? ((QListBox*)this)->cellHeight( 0 ) : -1;
}


/*!
  Returns the height (in pixels) of item at \e index.
*/

int QListBox::itemHeight( int index ) const
{
    return ((QListBox*)this)->cellHeight( index );
}

/*!
  Returns TRUE if the item at position \e index is at least partly
  visible.
*/

bool QListBox::itemVisible( int index )
{
    return rowIsVisible( index );
}


/*!
  Repaints the cell at position \e row using \e p.  The \e col argument
  is ignored, it is present because QTableView is more general. This
  function has the responsibility of showing focus and highlighting.

  \sa QTableView::paintCell()
*/

void QListBox::paintCell( QPainter *p, int row, int col )
{
    QListBoxItem *lbi = itemList->at( row );
    if ( !lbi )
	return;

    QColorGroup g = colorGroup();
    if ( isSelected( row ) ) {
	QColor	 fc;				// fill color
	if ( style() == WindowsStyle )
	    fc = QApplication::winStyleHighlightColor();
	else
	    fc = g.text();
	p->fillRect( 0, 0, cellWidth(col), cellHeight(row), fc );
	p->setPen( style() == WindowsStyle ? white : g.base() );
	p->setBackgroundColor( fc );
    } else {
	p->setBackgroundColor( g.base() );
	p->setPen( g.text() );
    }
    lbi->paint( p );
    if ( current == row && hasFocus() ) {
	if ( style() == WindowsStyle ) {
	    p->drawWinFocusRect( 1, 1, cellWidth(col)-2 , cellHeight(row)-2 );
	} else {
	    if ( isSelected( row ) )
		p->setPen( g.base() );
	    else
		p->setPen( g.text() );
	    p->setBrush( NoBrush );
	    p->drawRect( 1, 1, cellWidth(col)-2 , cellHeight(row)-2 );
	}
    }
    p->setBackgroundColor( g.base() );
    p->setPen( g.text() );
}


/*!
  Handles mouse press events.  Makes the clicked item the current item.
  \sa currentItem()
*/

void QListBox::mousePressEvent( QMouseEvent *e )
{
    int itemClicked = findItem( e->pos().y() );
    if ( itemClicked != -1 ) {
	ensureCurrentVisible( itemClicked );
	toggleCurrentItem();
    } else if ( contentsRect().contains( e->pos() ) &&
		lastRowVisible() >= (int) count() ) {
	ensureCurrentVisible( count()-1 );
	toggleCurrentItem();
    }
}

/*!
  Handles mouse release events.
*/

void QListBox::mouseReleaseEvent( QMouseEvent *e )
{
    if ( doDrag )
	mouseMoveEvent( e );
    if ( isTiming ) {
	killTimer( itemList->timerId );
	isTiming = FALSE;
    }
    emitChangedSignal( FALSE );
}

/*!
  Handles mouse double click events.  Emits the selected() signal for
  the item that was double-clicked.
*/

void QListBox::mouseDoubleClickEvent( QMouseEvent *e )
{
    mouseReleaseEvent( e );
    if ( currentItem() >= 0 ) {
	ASSERT(item( currentItem() ));
	QString tmp = item( currentItem() )->text();
	emit selected( currentItem());
	if ( !tmp.isNull() )
	    emit selected( tmp );
    }
}

/*!
  Handles mouse move events.  Scrolls the list box if auto-scroll
  is enabled.
  \sa autoScroll()
*/

void QListBox::mouseMoveEvent( QMouseEvent *e )
{
    if ( doDrag && (e->state() & (RightButton|LeftButton|MidButton)) != 0 ) {
	int itemClicked = findItem( e->pos().y() );
	if ( itemClicked >= 0 ) {
	    if ( isTiming ) {
		killTimer( itemList->timerId );
		isTiming = FALSE;
	    }
	    if ( multiSelect ) {
		bool s = currentItem() >= 0
			 ? isSelected( currentItem() ) : TRUE;
		int i = QMIN( itemClicked, currentItem() );
		if ( i < 0 )
		    i = 0;
		while( i <= itemClicked || i <= currentItem() ) {
		    setSelected( i, s );
		    i++;
		}
	    }
	    setCurrentItem( itemClicked );	// already current -> return
	    return;
	} else {
	    if ( !doAutoScroll )
		return;
	    if ( e->pos().y() < frameWidth() )
		scrollDown = FALSE;
	    else
		scrollDown = TRUE;
	    if ( !isTiming ) {
		isTiming = TRUE;
		itemList->timerId = startTimer( 100 );
	    }
	}
    }
}


/*!
  Handles key press events.

  \c Up and \c down arrow keys make the highlighted item move and if
  necessary scroll the list box.

  \c Enter makes the list box emit the selected() signal.

  \sa selected(), setCurrentItem()
*/

void QListBox::keyPressEvent( QKeyEvent *e )
{
    if ( numRows() == 0 )
	return;
    if ( currentItem() < 0 )
	setCurrentItem( topItem() );

    int oldCurrent;

    switch ( e->key() ) {
    case Key_Up:
	if ( currentItem() > 0 ) {
	    ensureCurrentVisible( currentItem() - 1 );
	    if ( e->state() & ShiftButton )
		toggleCurrentItem();
	}
	e->accept();
	break;
    case Key_Down:
	if ( currentItem() < (int)count() - 1 ) {
	    ensureCurrentVisible( currentItem()+1 );
	    if ( e->state() & ShiftButton )
		toggleCurrentItem();
	}
	e->accept();
	break;
    case Key_Left:
    case Key_Right:
	if ( bottomScrollBar() )
	    QApplication::sendEvent( (QObject *)horizontalScrollBar(), e );
	break;
    case Key_Next:
	e->accept();
	if ( style() == MotifStyle) {
	    if (lastRowVisible() == (int) count() - 1){
		int o = yOffset();
		setBottomItem( lastRowVisible() );
		if ( currentItem() < lastRowVisible() && currentItem() == topItem()
		     && yOffset() != o)
		    setCurrentItem(currentItem() + 1);
		break;
	    }
	    if (currentItem() != topItem() ){
		setTopItem( currentItem() );
		break;
	    }
	}
	else {
	    if (currentItem() != lastRowVisible() || lastRowVisible() == (int) count() - 1) {
		ensureCurrentVisible(lastRowVisible());
		break;
	    }
	}
	oldCurrent = currentItem();
	setYOffset(yOffset() + viewHeight() );
	if ( style() == MotifStyle)
	    ensureCurrentVisible( topItem() );
	else
	    ensureCurrentVisible(lastRowVisible());
	if (oldCurrent == currentItem() && currentItem() + 1 <  (int) count() )
	    ensureCurrentVisible( currentItem() + 1 );
	break;
    case Key_Prior:
	e->accept();
	if ( style() != MotifStyle) {
	    if (currentItem() != topItem() || topItem() == 0){
		ensureCurrentVisible(topItem());
		break;
	    }
	}
	else {
	    if (topItem() == 0){
		int o = yOffset();
		setTopItem( topItem() );
		if ( currentItem() > 0 && currentItem() == lastRowVisible() && yOffset() != o)
		    setCurrentItem(currentItem()-1);
		break;
	    }
	    if (currentItem() != lastRowVisible()) {
		setBottomItem( currentItem() );
		break;
	    }
	}
	oldCurrent = currentItem();
	setYOffset(yOffset() - viewHeight() );
	if ( style() == MotifStyle)
	    ensureCurrentVisible( lastRowVisible() );
	else
	    ensureCurrentVisible( topItem() );
	if (oldCurrent == currentItem() && currentItem() > 0)
	    ensureCurrentVisible( currentItem() -1);
	break;

    case Key_Space:
	toggleCurrentItem();
	e->accept();
	break;

    case Key_Return:
    case Key_Enter:
	if ( currentItem() >= 0 ) {
	    QString tmp = item( currentItem() )->text();
	    emit selected( currentItem());
	    if ( !tmp.isEmpty() )
		emit selected( tmp );
	}
	// do NOT accept here.  qdialog.
	break;
    default:
	break;
    }
    emitChangedSignal( FALSE );
}


/*!
  Handles focus events.  Repaints the current item (if not set,
  topItem() is made current).
  \sa keyPressEvent(), focusOutEvent()
*/

void QListBox::focusInEvent( QFocusEvent * )
{
    emitChangedSignal( FALSE );
    if ( currentItem() < 0 && numRows() > 0 )
	setCurrentItem( topItem() );
    updateCell( currentItem(), 0); //show focus
}


/*!
  Handles focus out events. Repaints the current item, if set.
  \sa keyPressEvent(), focusOutEvent()
*/

void QListBox::focusOutEvent( QFocusEvent * )
{
    emitChangedSignal( FALSE );
    if ( currentItem() >= 0 )
	updateCell( currentItem(), 0); //show lack of focus
}


/*!
  Handles resize events.  Updates internal parameters for the new list box
  size.
*/

void QListBox::resizeEvent( QResizeEvent *e )
{
    bool u = autoUpdate();
    setAutoUpdate( FALSE );
    QTableView::resizeEvent( e );
    setCellWidth( (int)maxItemWidth() );
    setCellWidth( QMAX( contentsRect().width(), (int)maxItemWidth()) );
    setAutoUpdate( u );
}


/*!
  Handles timer events.	 Does auto-scrolling.
*/

void QListBox::timerEvent( QTimerEvent * )
{
    if ( scrollDown ) {
	if ( currentItem() + 1 < (int)count() ) {
	    int y = QMAX(currentItem(),lastRowVisible())+1;
	    if ( y >= (int)count() )
		y = count() - 1;
	    if ( currentItem() >= 0 && multiSelect ) {
		bool s = isSelected( currentItem() );
		int i = currentItem();
		while( i <= y ) {
		    setSelected( i, s );
		    i++;
		}
	    }
	    ensureCurrentVisible( y );
	}
    } else {
	if ( topItem() > 0 ) {
	    setTopItem( topItem() - 1 );
	    if ( currentItem() > 0 && multiSelect ) {
		bool s = isSelected( currentItem() );
		int i = currentItem();
		while( i >= topItem() ) {
		    setSelected( i, s );
		    i--;
		}
	    }
	    setCurrentItem( topItem() );
	}
    }
}


/*!
  Returns the vertical pixel-coordinate in \e *yPos, of the list box
  item at position \e index in the list.  Returns FALSE if the item is
  outside the visible area.

  \sa findItem
*/

bool QListBox::itemYPos( int index, int *yPos ) const
{

    return rowYPos( index, yPos );
}

/*!
  Returns the index of the list box item at the vertical pixel-coordinate
  \e yPos.

  \sa itemYPos()
*/

int QListBox::findItem( int yPos ) const
{
    return findRow( yPos );
}


/*!
  Repaints the item at position \e index in the list.  Erases the line
  first if \e erase is TRUE.
*/

void QListBox::updateItem( int index, bool erase )
{
    updateCell( index, 0,  erase );
}


/*!
  Deletes all items in the list.  Protected function that does NOT
  update the list box.
*/

void QListBox::clearList()
{
    stringsOnly = TRUE;
    QListBoxItem *lbi;
    while ( itemList->count() ) {
	lbi = itemList->take( 0 );
	delete lbi;
    }
    if ( goingDown || QApplication::closingDown() )
	return;
    bool a = autoUpdate();
    setAutoUpdate( FALSE );
    updateNumRows( TRUE );
    current = -1;
    setTopCell( 0 );
    setAutoUpdate( a );
}


/*!
  Traverses the list and finds an item with the maximum width, and
  updates the internal list box structures accordingly.
*/

void QListBox::updateCellWidth()
{
    QListBoxItem *lbi = itemList->first();
    int maxW = 0;
    int w;
    while ( lbi ) {
	w = lbi->width( this );
	if ( w > maxW )
	    maxW = w;
	lbi = itemList->next();
    }
    setMaxItemWidth( maxW );
    setCellWidth( QMAX( maxW, viewWidth() ) );
}


/*!
  \internal
  Inserts a new list box item.

  The caller must also call update() if autoUpdate() is TRUE.
*/

void QListBox::insert( const QListBoxItem *lbi, int index,
		       bool updateCellWidth )
{
#if defined(CHECK_RANGE)
    ASSERT( lbi );
    ASSERT( (uint)index <= itemList->count() );
#endif
    itemList->insert( index, lbi );
    if ( current >= index )
	current++;
    if ( updateCellWidth ) {
	int w = lbi->width( this );
	if ( w > maxItemWidth() )
	    setMaxItemWidth( w );
	if ( w > cellWidth() )
	    setCellWidth( w );
    }
}

/*!
  \internal
  Changes a list box item.
*/

void QListBox::change( const QListBoxItem *lbi, int index )
{
#if defined(CHECK_RANGE)
    ASSERT( lbi );
    ASSERT( (uint)index < itemList->count() );
#endif

    QListBoxItem *old = itemList->take( index );
    int w = old->width( this );
    int h = old->height( this );
    delete old;
    itemList->insert( index, lbi );
    if ( w == cellWidth() ) {		     // I.e. index was the widest item
	updateCellWidth();
    }
    else {
	int ww = lbi->width( this );
	if ( ww > maxItemWidth() )
	    setMaxItemWidth( ww );
	if ( ww > cellWidth() )
	    setCellWidth( ww );
    }
    int nh = cellHeight( index );
    int y;
    if ( autoUpdate() && rowYPos( index, &y ) ) {
	if ( nh == h )
	    repaint( frameWidth(), y, viewWidth(), h );
	else
	    repaint( frameWidth(), y, viewWidth(), viewHeight() - y );
    }
}


/*!
  \internal
  Updates the num-rows setting in the table view.
*/

void QListBox::updateNumRows( bool updateWidth )
{
    bool autoU = autoUpdate();
    if ( autoU )
	setAutoUpdate( FALSE );
    bool sbBefore = testTableFlags( Tbl_vScrollBar );
    setNumRows( itemList->count() );
    if ( updateWidth || sbBefore != testTableFlags(Tbl_vScrollBar) )
	updateCellWidth();
    if ( autoU )
	setAutoUpdate( TRUE );
}


/*!
  Returns the width in pixels of the widest item.
*/

long QListBox::maxItemWidth() const
{
    if ( !qlb_maxLenDict )
	return 0;
    return (long) qlb_maxLenDict->find( (long)this );
}

/*!
  For binary compatibility.
*/
long QListBox::maxItemWidth()
{
    // This is only here for binary compatibility
    if ( !qlb_maxLenDict )
	return 0;
    return (long) qlb_maxLenDict->find( (long)this );
    // This is only here for binary compatibility
}


/*!
  Updates the cached value used by maxItemWidth().
*/

void QListBox::setMaxItemWidth( int len )
{
    ASSERT( qlb_maxLenDict );
    qlb_maxLenDict->remove( (long)this );
    if ( len )
	qlb_maxLenDict->insert( (long)this, (int*)len );
}


/*!
  \fn bool QListBox::isMultiSelection() const

  Returns TRUE if the listbox is in multi-selection mode, and FALSE if
  it is in single-selection mode.

  \sa setMultiSelection()
*/

/*!

  Sets the list box to multi-selection mode if \a enable is TRUE, and to
  single-selection mode if \a enable is FALSE.

  Single- and multi-selections modes work the same, except that the
  highlighted() and selected() signals are emitted at different times.

  \sa isMultiSelection()
*/

void QListBox::setMultiSelection( bool enable )
{
    if ( enable != (bool)multiSelect ) {
	multiSelect = enable;
	update();
    }
}


/*!
  Toggles the selection status of currentItem() and repaints, if
  the listbox is a multi-selection listbox.

  Does nothing if the listbox is a single-selection listbox.

  \sa setMultiSelection()
*/

void QListBox::toggleCurrentItem()
{
    if ( !multiSelect || currentItem() < 0 )
	return;

    QListBoxItem * i = item( currentItem() );
    if ( !i )
	return;

    i->selected = !i->selected;
    updateItem( currentItem() );
    emitChangedSignal( TRUE );
}


/*!
  Selects the item at position \a index if \a select is TRUE, or
  unselects it if \a select is FALSE.  May also repaint the item.

  If the listbox is a single-selection listbox and and \a select is TRUE,
  setCurrentItem will be called.

  If the listbox is a single-selection listbox and and \a select is FALSE,
  clearSelection() will be called if \a index is the currently selected
  item.

  \sa setMultiSelection(), setCurrentItem(), clearSelection(), currentItem()
*/

void QListBox::setSelected( int index, bool select )
{
    if ( !multiSelect ) {
	if ( select ) {
	    setCurrentItem( index );
	} else {
	    if ( index == current )
		clearSelection();
	}
	return;
    }

    if ( currentItem() < 0 )
	return;

    QListBoxItem *lbi = item( index );
    if ( !lbi || lbi->selected == select )
	return;

    lbi->selected = select;
    updateItem( index );
    emitChangedSignal( TRUE );
}


/*!
  Returns TRUE if item \a i is selected. Returns FALSE if it is not
  selected or if there is an error.
*/

bool QListBox::isSelected( int i ) const
{
    if ( !multiSelect )
	return i == current;

    QListBoxItem * lbi = item( i );
    return lbi ? lbi->selected : FALSE;
}



/*!
  Deselects all items. Note that a single-selection listbox
  will automatically select its first item if it has keyboard focus.
*/

void QListBox::clearSelection()
{
    if ( multiSelect ) {
	for ( int i = 0; i < (int)count(); i++ )
	    setSelected( i, FALSE );
    } else {
	int i = current;
	if ( hasFocus() ) {
	    current = 0;
	    updateItem( current );
	} else {
	    current = -1;
	}
	updateItem( i );
    }
}





/*!  If \a lazy is FALSE, maybe emit the changed() signal.  If \a lazy
  is TRUE, note that it's to be sent out at some later time.
*/

void QListBox::emitChangedSignal( bool lazy ) {
    if ( !multiSelect )
	return;

    if ( changedListBox && (!lazy || changedListBox != this) )
	emit changedListBox->selectionChanged();

    changedListBox = lazy ? this : 0;
}


/*!  Make sure that all of currentItem() is on-screen */

void QListBox::ensureCurrentVisible( int newCurrent )
{
    if ( newCurrent < 0 )
	newCurrent = currentItem();
    if ( newCurrent <= topItem() && newCurrent < lastRowVisible() )
	 setTopItem( newCurrent );
    else if ( newCurrent >= lastRowVisible() )
	setBottomItem( newCurrent );
    if ( newCurrent != currentItem() )
	setCurrentItem( newCurrent );
}

/*!
  Sets a \link QWidget::setFixedHeight() fixed height\endlink for the
  widget, so that it shows the given number of lines of text for the
  current font size.
*/
void QListBox::setFixedVisibleLines( int lines )
{
    int ls = fontMetrics().lineSpacing() + 1; // #### explain +1
    // #### What about auto-scrollbars?
    int sb = testTableFlags(Tbl_hScrollBar)
		? horizontalScrollBar()->height() : 0;
    setFixedHeight( frameWidth()*2 + ls*lines + sb );
    return;
}

/*!
  Returns a size mased on maxItemWidth() and any value set by
  setFixedVisibleLines().
*/
QSize QListBox::sizeHint() const
{
    QSize sz = QTableView::sizeHint();

    int w = (int)maxItemWidth() + 2*frameWidth();
    if ( testTableFlags(Tbl_vScrollBar) )
	w += verticalScrollBar()->width();
    sz.setWidth(w);

    // For when setFixedVisibleLines is used
    int h = maximumSize().height();
    if ( h<1000 ) sz.setHeight(h);

    return sz;
}
