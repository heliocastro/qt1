/**********************************************************************
** $Id: qcombobox.cpp,v 2.88.2.8 1999/01/13 17:30:13 ettrich Exp $
**
** Implementation of QComboBox widget class
**
** Created : 940426
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

#include "qcombobox.h"
#include "qpopupmenu.h"
#include "qlistbox.h"
#include "qpainter.h"
#include "qdrawutil.h"
#include "qscrollbar.h"   // we need the qDrawArrow function
#include "qkeycode.h"
#include "qstrlist.h"
#include "qpixmap.h"
#include "qtimer.h"
#include "qapplication.h"
#include "qlineedit.h"
#include <limits.h>

/*!
  \class QComboBox qcombobox.h
  \brief The QComboBox widget is a combined button and popup list.

  \ingroup realwidgets

  \define QComboBox::Policy

  A combo box may be defined as a selection widget which displays the
  current selection, and which can pop up a list of possible
  selections.  Some combo boxes also allow the user to select
  arbitrary strings, using a line editor.

  Since combo boxes occupy little screen space and always display the
  current selection, they are very well suited to displaying and
  selecting modes (such as font family and size): The user can always
  see what mode he/she is in, and the majority of the screen space is
  available for real work.

  QComboBox supports three different appearances: Motif 1.x, Motif 2.0
  and Windows 95.  In Motif 1.x, a combo box was called XmOptionMenu.
  In Motif 2.0, OSF introduced an improved combo box and
  named that XmComboBox.  QComboBox provides both.

  QComboBox provides two different constructors.  The simplest one
  creates an old-style combo box in Motif style:
  \code
      QComboBox * c = new QCombBox( this, "read-only combo" );
  \endcode

  The other one creates a new-style combo box in Motif style, and can
  create both read-only and read-write combo boxes:
  \code
      QComboBox * c1 = new QCombBox( FALSE, this, "read-only combo" );
      QComboBox * c2 = new QCombBox( TRUE, this, "read-write combo" );
  \endcode

  New-style combo boxes use a list box in both Motif and Windows
  styles, and both the content size and the on-screen size of the list
  box can be limited.  Old-style combo boxes use a popup in Motif
  style, and that popup will happily grow larger than the desktop if
  you put enough data in it.

  The two constructors create identical-looking combos in Windows
  style.

  Read-only combo boxes can contain pixmaps as well as texts; the
  insert() and changeItem() functions are suitably overloaded.  If you
  try to insert a pixmap in a read-write combo box, QComboBox simply
  ignores you.  For read-write combo boxes, the function clearEdit()
  is provided, to clear the displayed string without changing the
  combo box' contents.

  A combo box emits two signals, activated() and highlighted(), when a
  new item has been activated (selected) or highlighted (set to
  current).  Both signals exist in two versions, one with a \c char*
  argument and one with an \c int argument.  If the user highlights or
  activates a pixmap, only the \c int signals are emitted.

  Read-write combo boxes offer four policies for dealing with typed
  input: <ul> <li> \c NoInsertion means to simply emit the activated()
  signal, <li> \c AtBottom means to insert the string at the bottom of
  the combo box and emit activated(), <li> \c AtTop means to insert
  the string at the top of the combo box and emit activated(), and
  finally <li> \c AtCurrent means to replace the previously selected
  item with the typed string, and emit activated(). </ul> If inserting
  the typed string would cause the combo box to breach its content
  size limit, the item at the other end of the list is deleted.  The
  default insertion policy is \c AtBottom, you can change it using
  setInsertionPolicy().

  It is possible to constrain the input to an editable combo box using
  QValidator; see setValidator().  By default, all input is accepted.

  A combo box has a default focusPolicy() of \c TabFocus, i.e. it will
  not grab focus if clicked.  This differs from both Windows and Motif.

  <img src="qcombo1-m.gif">(Motif 1, read-only)<br clear=all>
  <img src="qcombo2-m.gif">(Motif 2, read-write)<br clear=all>
  <img src="qcombo3-m.gif">(Motif 2, read-only)<br clear=all>
  <img src="qcombo1-w.gif">(Windows style)

  \sa QLineEdit QListBox QSpinBox QRadioButton QButtonGroup
  <a href="guibooks.html#fowler">GUI Design Handbook: Combo Box,</a>
  <a href="guibooks.html#fowler">GUI Design Handbook: Drop-Down List Box.</a>
*/


/*! \fn void QComboBox::activated( int index )

  This signal is emitted when a new item has been activated (selected).
  The \e index is the position of the item in the popup list.
*/

/*! \fn void QComboBox::activated( const char * string )

  This signal is emitted when a new item has been activated
  (selected). \a string is the activated string.

  You can also use activated(int) signal, but be aware that its
  argument meaningful only for selected strings, not for typed
  strings.
*/

/*! \fn void QComboBox::highlighted( int index )

  This signal is emitted when a new item has been set to current.
  The \e index is the position of the item in the popup list.
*/

/*! \fn void QComboBox::highlighted( const char * string )

  This signal is emitted when a new item has been highlighted. \a
  string is the highlighted string.

  You can also use highlighted(int) signal.
*/

struct QComboData
{
    int		current;
    int		maxCount;
    int		sizeLimit;
    QComboBox::Policy p;
    bool	usingListBox;
    bool	autoresize;
    bool	poppedUp;
    bool	mouseWasInsidePopup;
    bool	arrowPressed;
    bool	arrowDown;
    bool	discardNextMousePress;
    bool	shortClick;
    union {
	QPopupMenu *popup;
	QListBox   *listBox;
    };
    bool	useCompletion;
    bool	completeNow;
    int		completeAt;

    class ComboEdit: public QLineEdit
    {
    public:
	ComboEdit( QWidget * parent )
	    : QLineEdit( parent,"combo edit" )
	{
	}
	bool validateAndSet( const char * newText, int newPos,
				int newMarkAnchor, int newMarkDrag )
	{
	    return QLineEdit::validateAndSet( newText, newPos,
					      newMarkAnchor, newMarkDrag );
	}
    };

    ComboEdit * ed;  // /bin/ed rules!
};


bool QComboBox::getMetrics( int *dist, int *buttonW, int *buttonH ) const
{
    if ( d->usingListBox && style() == WindowsStyle ) {
	QRect r  = arrowRect();
	*buttonW = r.width();
	*buttonH = r.height();
	*dist    = 4;
    } else if ( d->usingListBox ) {
	*dist = 6;
	*buttonW = 16;
	*buttonH = 18;
    } else {
	*dist     = 8;
	*buttonH  = 7;
	*buttonW  = 11;
    }
    return TRUE;
}


static inline bool checkInsertIndex( const char *method, const char * name,
				     int count, int *index)
{
    bool range_err = (*index > count);
#if defined(CHECK_RANGE)
    if ( range_err )
	warning( "QComboBox::%s: (%s) Index %d out of range",
		 method, name ? name : "<no name>", *index );
#endif
    if ( *index < 0 )				// append
	*index = count;
    return !range_err;
}


static inline bool checkIndex( const char *method, const char * name,
			       int count, int index )
{
    bool range_err = (index >= count);
#if defined(CHECK_RANGE)
    if ( range_err )
	warning( "QComboBox::%s: (%s) Index %i out of range",
		 method, name ? name : "<no name>", index );
#endif
    return !range_err;
}


/*!
  Constructs a combo box widget with a parent and a name.

  This constructor creates a popup menu if the program uses Motif look
  and feel; this is compatible with Motif 1.x.
*/

QComboBox::QComboBox( QWidget *parent, const char *name )
    : QWidget( parent, name )
{
    d = new QComboData;
    if ( style() == WindowsStyle ) {
	d->listBox           = new QListBox( 0, 0, WType_Popup );
	d->listBox->setAutoScrollBar( FALSE );
	d->listBox->setBottomScrollBar( FALSE );
	d->listBox->setAutoBottomScrollBar( FALSE );
	d->listBox->setFrameStyle( QFrame::Box | QFrame::Plain );
	d->listBox->setLineWidth( 1 );
	d->listBox->resize( 100, 10 );
	
	d->usingListBox      = TRUE;
	connect( d->listBox, SIGNAL(selected(int)),
		             SLOT(internalActivate(int)) );
	connect( d->listBox, SIGNAL(highlighted(int)),
		             SLOT(internalHighlight(int)));
    } else {
	d->popup             = new QPopupMenu;
	d->usingListBox      = FALSE;
	connect( d->popup, SIGNAL(activated(int)),
		           SLOT(internalActivate(int)) );
	connect( d->popup, SIGNAL(highlighted(int)),
		           SLOT(internalHighlight(int)) );
    }
    d->ed                    = 0;
    d->current               = 0;
    d->maxCount              = INT_MAX;
    d->sizeLimit	     = 10;
    d->p                    =  AtBottom;
    d->autoresize            = FALSE;
    d->poppedUp              = FALSE;
    d->arrowDown             = FALSE;
    d->discardNextMousePress = FALSE;
    d->shortClick            = FALSE;
    d->useCompletion = FALSE;

    setFocusPolicy( TabFocus );
    setPalettePropagation( AllChildren );
    setFontPropagation( AllChildren );
}


/*!
  Constructs a combo box with a maximum size and either Motif 2.0 or
  Windows look and feel.

  The input field can be edited if \a rw is TRUE, otherwise the user
  may only choose one of the items in the combo box.
*/


QComboBox::QComboBox( bool rw, QWidget *parent, const char *name )
    : QWidget( parent, name )
{
    d = new QComboData;
    d->listBox = new QListBox( 0, 0, WType_Popup );
    d->listBox->setAutoScrollBar( FALSE );
    d->listBox->setBottomScrollBar( FALSE );
    d->listBox->setAutoBottomScrollBar( TRUE );
    d->listBox->setFrameStyle( QFrame::Box | QFrame::Plain );
    d->listBox->setLineWidth( 1 );
    d->listBox->resize( 100, 10 );
	
    d->usingListBox = TRUE;
    connect( d->listBox, SIGNAL(selected(int)),
	     SLOT(internalActivate(int)) );
    connect( d->listBox, SIGNAL(highlighted(int)),
	     SLOT(internalHighlight(int)));

    d->current = 0;
    d->maxCount = INT_MAX;
    d->sizeLimit = 10;
    d->p = AtBottom;
    d->autoresize = FALSE;
    d->poppedUp = FALSE;
    d->arrowDown = FALSE;
    d->discardNextMousePress = FALSE;
    d->shortClick = FALSE;
    d->useCompletion = FALSE;

    setFocusPolicy( StrongFocus );

    if ( rw ) {
	d->ed = new QComboData::ComboEdit( this );
	d->ed->setFrame( FALSE );
	if ( style() == WindowsStyle )
	    d->ed->setGeometry( 2, 2, width() - 2 - 2 - 16, height() - 2 - 2 );
	else
	    d->ed->setGeometry( 3, 3, width() - 3 - 3 - 21, height() - 3 - 3 );
	d->ed->installEventFilter( this );
	setFocusProxy( d->ed );

	setBackgroundMode( NoBackground );

	connect( d->ed, SIGNAL(returnPressed()), SLOT(returnPressed()) );
    } else {
	d->ed = 0;
    }
    setPalettePropagation( AllChildren );
    setFontPropagation( AllChildren );
}



/*!
  Destroys the combo box.
*/

QComboBox::~QComboBox()
{
    if ( !QApplication::closingDown() ) {
	if ( d->usingListBox )
	    delete d->listBox;
	else
	    delete d->popup;
    } else {
	if ( d->usingListBox )
	    d->listBox = 0;
	else
	    d->popup   = 0;
    }
    delete d;
}



/*!  Reimplemented for implementational reasons.

  Note that QComboBox always turns into a new-style Motif combo box
  when it is changed from Windows to Motif style (even if it was an
  old-style combo box before).
*/

void QComboBox::setStyle( GUIStyle s )
{
    if ( s != style() ) {
	QWidget::setStyle( s );
	if ( !d->usingListBox ) {
	    QPopupMenu * p = d->popup;
	    d->listBox = new QListBox( 0, 0, WType_Popup );
	    d->listBox->setAutoScrollBar( FALSE );
	    d->listBox->setBottomScrollBar( FALSE );
	    d->listBox->setAutoBottomScrollBar( FALSE );
	    d->listBox->setFrameStyle( QFrame::Box | QFrame::Plain );
	    d->listBox->setLineWidth( 1 );
	    d->listBox->resize( 100, 10 );
	    d->usingListBox      = TRUE;
	    connect( d->listBox, SIGNAL(selected(int)),
		     SLOT(internalActivate(int)) );
	    connect( d->listBox, SIGNAL(highlighted(int)),
		     SLOT(internalHighlight(int)));
	    if ( p ) {
		int n;
		for( n=p->count()-1; n>=0; n-- ) {
		    if ( p->text( n ) )
			d->listBox->insertItem( p->text( n ), 0 );
		    else if ( p->pixmap( n ) )
			d->listBox->insertItem( *(p->pixmap( n )), 0 );
		}
		delete p;
	    }
	}

    }
    if ( d->ed ) {
	d->ed->setStyle( s );
	d->ed->setFrame( s == MotifStyle );
    }
    if ( d->listBox )
	d->listBox->setStyle( s );
}



/*!
  Returns the number of items in the combo box.
*/

int QComboBox::count() const
{
    if ( d->usingListBox )
	return d->listBox->count();
    else
	return d->popup->count();
}


/*!
  Inserts the list of strings at the index \e index in the combo box.
*/

void QComboBox::insertStrList( const QStrList *list, int index )
{
    if ( !list ) {
#if defined(CHECK_NULL)
	ASSERT( list != 0 );
#endif
	return;
    }
    QStrListIterator it( *list );
    const char *tmp;
    if ( index < 0 )
	index = count();
    while ( (tmp=it.current()) ) {
	++it;
	if ( d->usingListBox )
	    d->listBox->insertItem( tmp, index );
	else
	    d->popup->insertItem( tmp, index );
	if ( index++ == d->current ) {
	    if ( d->ed )
		d->ed->setText( text( d->current ) );
	    else
		repaint();
	    currentChanged();
	}
    }
    if ( index != count() )
	reIndex();
}

/*!
  Inserts the array of strings at the index \e index in the combo box.

  The \e numStrings argument is the number of strings.
  If \e numStrings is -1 (default), the \e strs array must be
  terminated with 0.

  Example:
  \code
    static const char *items[] = { "red", "green", "blue", 0 };
    combo->insertStrList( items );
  \endcode
*/

void QComboBox::insertStrList( const char **strings, int numStrings, int index)
{
    if ( !strings ) {
#if defined(CHECK_NULL)
	ASSERT( strings != 0 );
#endif
	return;
    }
    if ( index < 0 )
	index = count();
    int i = 0;
    while ( (numStrings<0 && strings[i]!=0) || i<numStrings ) {
	if ( d->usingListBox )
	    d->listBox->insertItem( strings[i], index );
	else
	    d->popup->insertItem( strings[i], index );
	i++;
	if ( index++ == d->current ) {
	    if ( d->ed )
		d->ed->setText( text( d->current ) );
	    else
		repaint();
	    currentChanged();
	}
    }
    if ( index != count() )
	reIndex();
}


/*!
  Inserts a text item at position \e index. The item will be appended if
  \e index is negative.
*/

void QComboBox::insertItem( const char *t, int index )
{
    int cnt = count();
    if ( !checkInsertIndex( "insertItem", name(), cnt, &index ) )
	return;
    if ( d->usingListBox )
        d->listBox->insertItem( t, index );
    else
        d->popup->insertItem( t, index );
    if ( index != cnt )
	reIndex();
    if ( index == d->current ) {
	if ( d->ed )
	    d->ed->setText( text( d->current ) );
	else
	    repaint();
    }
    if ( index == d->current )
	currentChanged();
}

/*!
  Inserts a pixmap item at position \e index. The item will be appended if
  \e index is negative.

  If the combo box is writable, the pixmap is not inserted.
*/

void QComboBox::insertItem( const QPixmap &pixmap, int index )
{
    if ( d->ed )
	return;

    int cnt = count();
    bool append = index < 0 || index == cnt;
    if ( !checkInsertIndex( "insertItem", name(), cnt, &index ) )
	return;
    if ( d->usingListBox )
        d->listBox->insertItem( pixmap, index );
    else
        d->popup->insertItem( pixmap, index );
    if ( !append )
	reIndex();
    if ( index == d->current )
	currentChanged();
}


/*!
  Removes the item at position \e index.
*/

void QComboBox::removeItem( int index )
{
    int cnt = count();
    if ( !checkIndex( "removeItem", name(), cnt, index ) )
	return;
    if ( d->usingListBox )
	d->listBox->removeItem( index );
    else
	d->popup->removeItemAt( index );
    if ( index != cnt-1 )
	reIndex();

     if ( index == d->current ) {
	if ( d->ed ) {
	    QString s = "";
	    if (d->current < cnt - 1)
		s = text( d->current );
	    d->ed->setText( s );
	}
	else
	    repaint();
	currentChanged();
    }
}


/*!
  Removes all combo box items.
*/

void QComboBox::clear()
{
    if ( d->usingListBox )
	d->listBox->clear();
    else
	d->popup->clear();
    d->current = 0;
    if ( d->ed )
	d->ed->setText( "" );
    currentChanged();
}


/*!
  Returns the text item being edited, or the current text item if the combo
  box is not editable.
  \sa text()
*/

const char *QComboBox::currentText() const
{
    if ( d->ed ) {
	return d->ed->text();
    } else {
	return text( currentItem() );
    }
}

/*!
  Returns the text item at a given index, or 0 if the item is not a string.
  \sa currentText()
*/

const char *QComboBox::text( int index ) const
{
    if ( !checkIndex( "text", name(), count(), index ) )
	return 0;
    if ( d->usingListBox )
	return d->listBox->text( index );
    else
	return d->popup->text( index );
}

/*!
  Returns the pixmap item at a given index, or 0 if the item is not a pixmap.
*/

const QPixmap *QComboBox::pixmap( int index ) const
{
    if ( !checkIndex( "pixmap", name(), count(), index ) )
	return 0;
    if ( d->usingListBox )
	return d->listBox->pixmap( index );
    else
	return d->popup->pixmap( index );
}

/*!
  Replaces the item at position \e index with a text.
*/

void QComboBox::changeItem( const char *t, int index )
{
    if ( !checkIndex( "changeItem", name(), count(), index ) )
	return;
    if ( d->usingListBox )
	d->listBox->changeItem( t, index );
    else
	d->popup->changeItem( t, index );
    if ( index == d->current && d->ed )
	d->ed->setText( text( d->current ) );
}

/*!
  Replaces the item at position \e index with a pixmap, unless the
  combo box is writable.

  \sa insertItem()
*/

void QComboBox::changeItem( const QPixmap &im, int index )
{
    if ( d->ed != 0 || !checkIndex( "changeItem", name(), count(), index ) )
	return;
    if ( d->usingListBox )
	d->listBox->changeItem( im, index );
    else
	d->popup->changeItem( im, index );
}


/*!
  Returns the index of the current combo box item.
  \sa setCurrentItem()
*/

int QComboBox::currentItem() const
{
    return d->current;
}

/*!
  Sets the current combo box item.
  This is the item to be displayed on the combo box button.
  \sa currentItem()
*/

void QComboBox::setCurrentItem( int index )
{
    if ( index == d->current )
	return;
    if ( !checkIndex( "setCurrentItem", name(), count(), index ) ) {
	return;
    }
    d->current = index;
    if ( d->ed )
	d->ed->setText( text( index ) );
    if ( d->poppedUp ) {
	if ( d->usingListBox && d->listBox )
	    d->listBox->setCurrentItem( index );
	else if ( d->popup )
	    // the popup will soon send an override, but for the
	    // moment this is correct
	    internalHighlight( index );
    }
    currentChanged();
}


/*!
  Returns TRUE if auto-resizing is enabled, or FALSE if auto-resizing is
  disabled.

  Auto-resizing is disabled by default.

  \sa setAutoResize()
*/

bool QComboBox::autoResize() const
{
    return d->autoresize;
}

/*!
  Enables auto-resizing if \e enable is TRUE, or disables it if \e enable is
  FALSE.

  When auto-resizing is enabled, the combo box button will resize itself
  whenever the current combo box item change.

  \sa autoResize(), adjustSize()
*/

void QComboBox::setAutoResize( bool enable )
{
    if ( (bool)d->autoresize != enable ) {
	d->autoresize = enable;
	if ( enable )
	    adjustSize();
    }
}

/*!
  Returns a size which fits the contents of the combo box button.
*/
QSize QComboBox::sizeHint() const
{
    int i, w, h;
    const char *tmp;
    QFontMetrics fm = fontMetrics();

    int extraW = 20;
    int maxW = count() ? 18 : 7 * fm.width('x') + 18;
    int maxH = QMAX( fm.height(), 12 );

    for( i = 0; i < count(); i++ ) {
	tmp = text( i );
	if ( tmp ) {
	    w = fm.width( tmp );
	    h = 0;
	} else {
	    const QPixmap *pix = pixmap( i );
	    if ( pix ) {
		w = pix->width();
		h = pix->height();
	    } else {
		w = 0;
		h = height() - 4;
	    }
	}
	if ( w > maxW )
	    maxW = w;
	if ( h > maxH )
	    maxH = h;
    }
    if ( maxH <= 16 && parentWidget() &&
	 (parentWidget()->inherits( "QToolBar" ) ||
	  parentWidget()->inherits( "QDialog" ) && style() == WindowsStyle) )
	maxH = 12;
    return QSize( 4 + 4 + maxW + extraW, maxH + 5 + 5 );
}


/*!
  \internal
  Receives activated signals from an internal popup list and emits
  the activated() signal.
*/

void QComboBox::internalActivate( int index )
{
    if ( d->current != index ) {
	d->current = index;
	currentChanged();
    }
    if ( d->usingListBox )
	popDownListBox();
    else
	d->popup->removeEventFilter( this );
    d->poppedUp = FALSE;

    QString t( text( index ) );
    emit activated( index );
    if ( d->ed )
	d->ed->setText( t );
    emit activated( t );
}

/*!
  \internal
  Receives highlighted signals from an internal popup list and emits
  the highlighted() signal.
*/

void QComboBox::internalHighlight( int index )
{
    emit highlighted( index );
    const char *t = text( index );
    if ( t )
	emit highlighted( t );
}

/*!
  \internal
  Receives timeouts after a click. Used to decide if a Motif style
  popup should stay up or not after a click.
*/
void QComboBox::internalClickTimeout()
{
    d->shortClick = FALSE;
}


/*!
  Reimplements QWidget::setBackgroundColor().

  Sets the background color for both the combo box button and the
  combo box popup list.
*/

void QComboBox::setBackgroundColor( const QColor &color )
{
    QWidget::setBackgroundColor( color );
    if ( !d->usingListBox )
	d->popup->setBackgroundColor( color );
}

/*!
  Reimplements QWidget::setPalette().

  Sets the palette for both the combo box button and the
  combo box popup list.
*/

void QComboBox::setPalette( const QPalette &palette )
{
    QWidget::setPalette( palette );
    if ( d->usingListBox )
	d->listBox->setPalette( palette );
    else
	d->popup->setPalette( palette );
}

/*!
  Reimplements QWidget::setFont().

  Sets the font for both the combo box button and the
  combo box popup list.
*/

void QComboBox::setFont( const QFont &font )
{
    QWidget::setFont( font );
    if ( d->usingListBox )
	d->listBox->setFont( font );
    else
	d->popup->setFont( font );
    if (d->ed)
	d->ed->setFont( font );
    if ( d->autoresize )
	adjustSize();
}


/*!
  Handles resize events for the combo box.
*/

void QComboBox::resizeEvent( QResizeEvent * )
{
    if ( !d->ed )
	return;
    else if ( style() == WindowsStyle )
	d->ed->setGeometry( 2, 2, width() - 2 - 2 - 16, height() - 2 - 2 );
    else
	d->ed->setGeometry( 3, 3, width() - 3 - 3 - 21, height() - 3 - 3 );
}


/*!
  Handles paint events for the combo box.
*/

void QComboBox::paintEvent( QPaintEvent *event )
{
    QPainter p( this );
    if ( event )
	p.setClipRect( event->rect() );
    QColorGroup g  = colorGroup();

    if ( width() < 5 || height() < 5 ) {
	QBrush fill( g.background() );
	qDrawShadePanel( &p, rect(), g, FALSE, 2, &fill );
	return;
    }

    if ( !d->usingListBox ) {			// motif 1.x style
	int dist, buttonH, buttonW;
	QBrush fill( g.background() );

	getMetrics( &dist, &buttonW, &buttonH );
	int xPos = width() - dist - buttonW - 1;
	qDrawShadePanel( &p, rect(), g, FALSE, 2, &fill );
	qDrawShadePanel( &p, xPos, (height() - buttonH)/2,
			 buttonW, buttonH, g, FALSE, 2 );
	QRect clip( 4, 2, xPos - 2 - 4, height() - 4 );
	const char *str = d->popup->text( d->current );
	if ( str ) {
	    p.drawText( clip, AlignCenter | SingleLine, str );
	} else {
	    QPixmap *pix = d->popup->pixmap( d->current );
	    if ( pix ) {
		p.setClipRect( clip );
		p.drawPixmap( 4, (height()-pix->height())/2, *pix );
		p.setClipping( FALSE );
	    }
	}

	if ( hasFocus() )
	    p.drawRect( xPos - 5, 4, width() - xPos + 1 , height() - 8 );

    } else if ( style() == MotifStyle ) {	// motif 2.0 style
	int awh, ax, ay, sh, sy;
	QBrush fill( g.background() );

	if ( height() < 6 ) {
	    awh = height();
	    ay = 0;
	} else if ( height() < 18 ) {
	    awh = height() - 6;
	    ay = 0;
	} else {
	    awh = height()*4/10;
	    ay = awh/2;
	}

	sh = (awh+3)/4;
	sy = height() - ay - sh;
	if ( sh < 3 ) {
	    sy = sy+sh-3;
	    sh = 3;
	}
	if ( sy - ay - awh > 3 ) {
	    sy -= ( sy-ay-awh-3 )/2;
	    ay += ( sy-ay-awh-3 )/2;
	}
	awh = awh;

	if ( d->ed )
	    ax = d->ed->geometry().right() + 4;
	else
	    ax = width() - 3 - 21;

	if ( ax + awh + 2 < width() )
	    ax += ( width() - 2 - ax - awh ) / 2;

	qDrawShadePanel( &p, rect(), g, FALSE, d->ed ? 1 : 2, &fill );

	qDrawArrow( &p, DownArrow, MotifStyle, FALSE,
		    ax, ay, awh, awh, g );

	p.setPen( g.light() );
	p.drawLine( ax, sy, ax+awh-1, sy );
	p.drawLine( ax, sy, ax, sy+sh-1 );
	p.setPen( g.dark() );
	p.drawLine( ax+1, sy+sh-1, ax+awh-1, sy+sh-1 );
	p.drawLine( ax+awh-1, sy+1, ax+awh-1, sy+sh-1 );

	if ( d->ed ) {
	    QRect r( d->ed->geometry() );
	    r.setRect( r.left()-1, r.top()-1, r.width()+2, r.height()+2 );
	    qDrawShadePanel( &p, r, g, TRUE, d->ed ? 1 : 2, &fill );
	} else {
	    QRect clip( 3, 3, width() - 3 - 3 - 21, height() - 3 - 3 );
	    const char *str = d->listBox->text( d->current );
	    if ( str ) {
		p.setPen( g.foreground() );
		p.drawText( clip, AlignCenter | SingleLine, str );
	    } else {
		const QPixmap *pix = d->listBox->pixmap( d->current );
		if ( pix ) {
		    p.setClipRect( clip );
		    p.drawPixmap( 4, (height()-pix->height())/2, *pix );
		    p.setClipping( FALSE );
		}
	    }
	}

	if ( hasFocus() )
	    p.drawRect( ax - 2, ay - 2, awh+4, sy+sh+4-ay );

    } else {					// windows 95 style
	QColor bg = isEnabled() ? g.base() : g.background();
	const char *str = d->listBox->text( d->current );

	QBrush fill( bg );
	qDrawWinPanel( &p, 0, 0, width(), height(), g, TRUE, &fill );

	QRect arrowR = arrowRect();
	qDrawWinPanel(&p, arrowR, g, d->arrowDown );
	qDrawArrow( &p, DownArrow, WindowsStyle, d->arrowDown,
		    arrowR.x() + 2, arrowR.y() + 2,
		    arrowR.width() - 4, arrowR.height() - 4, g );

	QRect textR( 5, 4, width()  - 5 - 4 - arrowR.width(),
		     height() - 4 - 4 );

	if ( hasFocus() ) {
	    QBrush fill( QApplication::winStyleHighlightColor() );
	    p.fillRect( textR.x()-1, textR.y(),
			textR.width(), textR.height(), fill );
	    p.drawWinFocusRect( textR.x()-2, textR.y()-1,
				textR.width()+2, textR.height()+2, backgroundColor() );
	}

	p.setClipRect( textR );

	if ( hasFocus() ) {
	    p.setPen( white );
	    p.setBackgroundColor( QApplication::winStyleHighlightColor() );
	} else {
	    p.setPen( g.text() );
	    p.setBackgroundColor( bg );
	}
	if ( str ) {
	    p.drawText( textR, AlignLeft | AlignVCenter | SingleLine, str);
	} else {
	    const QPixmap *pix = d->listBox->pixmap( d->current );
	    if ( pix )
		p.drawPixmap( 4, (height()-pix->height())/2, *pix );
	}
	p.setClipping( FALSE );
    }
}


/*!
  \internal
  Returns the button arrow rectangle for windows style combo boxes.
*/
QRect QComboBox::arrowRect() const
{
    return QRect( width() - 2 - 16, 2, 16, height() - 4 );
}


/*!
  Handles mouse press events for the combo box.
*/

void QComboBox::mousePressEvent( QMouseEvent *e )
{
    if ( d->discardNextMousePress ) {
	d->discardNextMousePress = FALSE;
	return;
    }
    d->arrowPressed = FALSE;
    if ( style() == WindowsStyle ) {
	popup();
	if ( arrowRect().contains( e->pos() ) ) {
	    d->arrowPressed = TRUE;
	    d->arrowDown    = TRUE;
	    repaint( FALSE );
	}
    } else if ( d->usingListBox ) {
	popup();
	QTimer::singleShot( 200, this, SLOT(internalClickTimeout()));
	d->shortClick = TRUE;
    } else {
	popup();
	QTimer::singleShot( 200, this, SLOT(internalClickTimeout()));
	d->shortClick = TRUE;
    }
}

/*!
  Handles mouse move events for the combo box.
*/

void QComboBox::mouseMoveEvent( QMouseEvent * )
{
}

/*!
  Handles mouse release events for the combo box.
*/

void QComboBox::mouseReleaseEvent( QMouseEvent * )
{
}

/*!
  Handles mouse double click events for the combo box.
*/

void QComboBox::mouseDoubleClickEvent( QMouseEvent *e )
{
    mousePressEvent( e );
}


/*!
  Handles key press events for the combo box.

  In Motif style, up and down change the selected item and both enter
  and return pops up the list.  In Windows style, all four arrow keys
  change the selected item, and Space pops up the list.
*/

void QComboBox::keyPressEvent( QKeyEvent *e )
{
    int c;

    if ( e->key() == Key_F4 ||
	 ( e->key() == Key_Down && (e->state() & AltButton) ) ||
	 ( !d->ed && e->key() == Key_Space ) ) {
	e->accept();
	d->popup->setActiveItem( d->current );
	popup();
	return;
    }
    else if ( d->usingListBox && e->key() == Key_Up ) {
	c = currentItem();
	if ( c > 0 )
	    setCurrentItem( c-1 );
	else
	    setCurrentItem( count()-1 );
	e->accept();
    } else if ( d->usingListBox && e->key() == Key_Down ) {
	c = currentItem();
	if ( ++c < count() )
	    setCurrentItem( c );
	else
	    setCurrentItem( 0 );
	e->accept();
    } else {
	e->ignore();
	return;
    }

    c = currentItem();
    emit highlighted( c );
    if ( text( c ) )
	emit activated( text( c ) );
    emit activated( c );
}


/*!
  Handles focus in events.
*/

void QComboBox::focusInEvent( QFocusEvent * )
{
    repaint( FALSE );
}

/*!
  \internal
   Calculates the listbox height needed to contain all items, or as
   many as the list box is supposed to contain.
*/
static int listHeight( QListBox *l, int sl )
{
    int i;
    int sumH = 0;
    for( i = 0 ; i < (int) l->count() && i < sl ; i++ ) {
	sumH += l->itemHeight( i );
    }
    return sumH;
}


/*!
  Popups the combo box popup list.  If the list is empty, inserts a
  single, empty, string.
*/

void QComboBox::popup()
{
    if ( !count() )
	insertItem( "", 0 );

    if ( d->usingListBox ) {
	                // Send all listbox events to eventFilter():
	d->listBox->installEventFilter( this );
	d->mouseWasInsidePopup = FALSE;
	d->listBox->resize( width(),
			    listHeight( d->listBox, d->sizeLimit ) + 2 );
	if (d->listBox->bottomScrollBar() )
	    d->listBox->resize( width(),
				listHeight( d->listBox, d->sizeLimit ) + 2 + 16 ); //### hardcoded scrollbar height 16
	QWidget *desktop = QApplication::desktop();
	int sw = desktop->width();			// screen width
	int sh = desktop->height();			// screen height
	QPoint pos = mapToGlobal( QPoint(0,height()) );

	// XXX Similar code is in QPopupMenu
	int x = pos.x();
	int y = pos.y();
	int w = d->listBox->width();
	int h = d->listBox->height();

	// the complete widget must be visible
	if ( x + w > sw )
	    x = sw - w;
	else if ( x < 0 )
	    x = 0;
	if (y + h > sh && y - h - height() >= 0 )
	    y = y - h - height();

	d->listBox->move( x,y );
	d->listBox->raise();
	d->listBox->blockSignals( TRUE );
	d->listBox->setCurrentItem( d->current );
	d->listBox->blockSignals( FALSE );
	d->listBox->setAutoScrollBar( TRUE );
	d->listBox->show();
    } else {
	d->popup->installEventFilter( this );
	d->popup->popup( mapToGlobal( QPoint(0,0) ), d->current );
    }
    d->poppedUp = TRUE;
}

/*!
  \internal
  Pops down (removes) the combo box popup list box.
*/
void QComboBox::popDownListBox()
{
    ASSERT( d->usingListBox );
    d->listBox->removeEventFilter( this );
    d->listBox->hide();
    d->listBox->setCurrentItem( d->current );
    if ( d->arrowDown ) {
	d->arrowDown = FALSE;
	repaint( FALSE );
    }
    d->poppedUp = FALSE;
}


/*!
  \internal
  Re-indexes the identifiers in the popup list.
*/

void QComboBox::reIndex()
{
    if ( !d->usingListBox ) {
	int cnt = count();
	while ( cnt-- )
	    d->popup->setId( cnt, cnt );
    }
}

/*!
  \internal
  Repaints the combo box.
*/

void QComboBox::currentChanged()
{
    if ( d->autoresize )
	adjustSize();
    repaint();
}



/*!
  This event filter is used to manipulate the line editor in magic
  ways.  In Qt 2.0 it will all change, until then binary compatibility
  lays down the law.

  The event filter steals events from the popup or listbox when they
  are popped up. It makes the popup stay up after a short click in
  motif style. In windows style it toggles the arrow button of the
  combo box field, and activates an item and takes down the listbox
  when the mouse button is released.
*/

bool QComboBox::eventFilter( QObject *object, QEvent *event )
{
    if ( !event )
	return TRUE;
    else if ( object == d->ed ) {
	if ( event->type() == Event_KeyPress ) {
	    keyPressEvent( (QKeyEvent *)event );
	    if ( ((QKeyEvent *)event)->isAccepted() ) {
		d->completeNow = FALSE;
		return TRUE;
	    } else if ( ((QKeyEvent *)event)->key() != Key_End ) {
		d->completeNow = TRUE;
		d->completeAt = d->ed->cursorPosition();
	    }
	} else if ( event->type() == Event_KeyRelease ) {
	    d->completeNow = FALSE;
	    keyReleaseEvent( (QKeyEvent *)event );
	    return ((QKeyEvent *)event)->isAccepted();
	} else if ( (event->type() == Event_FocusIn ||
		     event->type() == Event_FocusOut ) ) {
	    d->completeNow = FALSE;
	    // to get the focus indication right
	    update();
	} else if ( d->useCompletion && d->completeNow ) {
	    if ( d->ed->text() &&  d->ed->cursorPosition() > d->completeAt &&
		 d->ed->cursorPosition() == (int)qstrlen(d->ed->text()) ) {
		d->completeNow = FALSE;
		QString ct( d->ed->text() );
		QString it;
		int i =0;
		int foundAt = -1;
		int foundLength = 100000; // lots
		while( i<count() ) {
		    it = text( i );
		    if ( it.length() >= ct.length() ) {
			it.truncate( ct.length() );
			int itlen = qstrlen( text( i ) );
			if ( it == ct && itlen < foundLength ) {
			    foundAt = i;
			    foundLength = qstrlen( text( i ) );
			}
		    }
		    i++;
		}
		if ( foundAt > -1 ) {
		    it = text( foundAt );
		    d->ed->validateAndSet( it, ct.length(),
					   ct.length(), it.length() );
		}
	    }
	}
    } else if ( d->usingListBox && object == d->listBox ) {
	QMouseEvent *e = (QMouseEvent*)event;
	switch( event->type() ) {
        case Event_MouseMove:
	    if ( !d->mouseWasInsidePopup  ) {
		QPoint pos = e->pos();
		if ( d->listBox->rect().contains( pos ) )
		    d->mouseWasInsidePopup = TRUE;
		// Check if arrow button should toggle
		// this applies only to windows style
		if ( d->arrowPressed ) {
		    QPoint comboPos;
		    comboPos = mapFromGlobal( d->listBox->mapToGlobal(pos) );
		    if ( arrowRect().contains( comboPos ) ) {
			if ( !d->arrowDown  ) {
			    d->arrowDown = TRUE;
			    repaint( FALSE );
			}
		    } else {
			if ( d->arrowDown  ) {
			    d->arrowDown = FALSE;
			    repaint( FALSE );
			}
		    }
		}
	    }
	    break;
	case Event_MouseButtonRelease:
	    if ( d->listBox->rect().contains( e->pos() ) ) {
		QMouseEvent tmp( Event_MouseButtonDblClick,
				 e->pos(), e->button(), e->state() ) ;
		// will hide popup
		QApplication::sendEvent( object, &tmp );
		return TRUE;
	    } else {
		if ( d->mouseWasInsidePopup ) {
		    popDownListBox();
		} else {
		    d->arrowPressed = FALSE;
		    if ( d->arrowDown  ) {
			d->arrowDown = FALSE;
			repaint( FALSE );
		    }
		}
	    }
	    break;
        case Event_MouseButtonDblClick:
        case Event_MouseButtonPress:
	    if ( !d->listBox->rect().contains( e->pos() ) ) {
		QPoint globalPos = d->listBox->mapToGlobal(e->pos());
		if ( QApplication::widgetAt( globalPos, TRUE ) == this ) {
		    d->discardNextMousePress = TRUE;
		    // avoid popping up again
		}
		popDownListBox();
		return TRUE;
	    }
	    break;
	case Event_KeyPress:
	    switch( ((QKeyEvent *)event)->key() ) {
 	    case Key_Up:
 	    case Key_Down:
 		if ( !(((QKeyEvent *)event)->state() & AltButton) )
 		    break;
	    case Key_F4:
	    case Key_Escape:
		popDownListBox();
		((QKeyEvent*)event)->accept();
		return TRUE;
	    case Key_Enter:
	    case Key_Return:
		// magic to work around QDialog's enter handling
		((QKeyEvent*)event)->accept();
		return FALSE;
	    default:
		break;
	    }
	default:
	    break;
	}
    } else if ( !d->usingListBox && object == d->popup ) {
	QMouseEvent *e = (QMouseEvent*)event;
	switch ( event->type() ) {
        case Event_MouseButtonRelease:
	    if ( d->shortClick ) {
		QMouseEvent tmp( Event_MouseMove,
				 e->pos(), e->button(), e->state() ) ;
		// highlight item, but don't pop down:
		QApplication::sendEvent( object, &tmp );
		return TRUE;
	    }
	    break;
        case Event_MouseButtonDblClick:
        case Event_MouseButtonPress:
	    if ( !d->popup->rect().contains( e->pos() ) ) {
		// remove filter, event will take down popup:
		d->listBox->removeEventFilter( this );
		// ### uglehack!
		// call internalHighlight so the highlighed signal
		// will be emitted at least as often as necessary.
		// it may be called more often than necessary
		internalHighlight( d->current );
	    }
	    break;
	default:
	    break;
	}
    }
    return FALSE;
}


/*!
  Returns the current maximum on-screen size of the combo box.  The
  default is ten lines.

  \sa setSizeLimit() count() maxCount()
*/

int QComboBox::sizeLimit() const
{
    return d ? d->sizeLimit : INT_MAX;
}


/*!

  Sets the maximum on-screen size of the combo box to \a lines.  This
  is disregarded in Motif 1.x style.  The default limit is ten lines.

  If the number of items in the combo box is/grows larger than
  \c lines, a list box is added.

  \sa sizeLimit() count() setMaxCount()
*/

void QComboBox::setSizeLimit( int lines )
{
    d->sizeLimit = lines;
}



/*!
  Returns the current maximum size of the combo box.  By default,
  there is no limit, so this function returns INT_MAX.

  \sa setMaxCount() count()
*/

int QComboBox::maxCount() const
{
    return d ? d->maxCount : INT_MAX;
}


/*!
  Sets the maximum number of items the combo box can hold to \a count.

  If \a count is smaller than the current number of items, the list is
  truncated at the end.  There is no limit by default.

  \sa maxCount() count()
*/

void QComboBox::setMaxCount( int count )
{
    int l = this->count();
    while( --l > count )
	removeItem( l );
    d->maxCount = count;
}

/*!
  Returns the current insertion policy of the combo box.

  \sa setPolicy()
*/

QComboBox::Policy QComboBox::insertionPolicy() const
{
    return d->p;
}


/*!
  Sets the insertion policy of the combo box to \a policy.

  The insertion policy governs where items typed in by the user are
  inserted in the list.  The possible values are <ul> <li> \c
  NoInsertion: Strings typed by the user aren't inserted anywhere <li>
  \c AtTop: Strings typed by the user are inserted above the top item
  in the list <li> AtCurrent: Strings typed by the user replace the
  last selected item <li> AtBottom: Strings typed by the user are
  inserted at the bottom of the list. </ul>

  The default insertion policy is \c AtBottom.

  \sa policy()
*/

void QComboBox::setInsertionPolicy( Policy policy )
{
    d->p = policy;
}



/*!
  Internal slot to keep the line editor up to date.
*/

void QComboBox::returnPressed()
{
    QString s( d->ed->text() );
    int c = 0;
    switch ( insertionPolicy() ) {
    case AtCurrent:
	if ( qstrcmp( s, text( currentItem() ) ) )
	    changeItem( s, currentItem() );
	emit activated( currentItem() );
	emit activated( s );
	return;
    case NoInsertion:
	emit activated( s );
	return;
    case AtTop:
	c = 0;
	break;
    case AtBottom:
	c = count();
	break;
    case BeforeCurrent:
	c = currentItem();
	break;
    case AfterCurrent:
	c = currentItem() + 1;
	break;
    }
    if ( count() == d->maxCount )
	removeItem( count() - 1 );
    insertItem( s, c );
    setCurrentItem( c );
    emit activated( c );
    emit activated( s );
}


/*!  Reimplemented for internal purposes.
*/

void QComboBox::setEnabled( bool enable )
{
    if ( d && d->ed )
	d->ed->setEnabled( enable );
    QWidget::setEnabled( enable );
}



/*!  Sets this combo box to be editable only as allowed by \a v.

  This function does nothing if the combo is not editable.

  \sa validator() clearValidator() QValidator
*/

void QComboBox::setValidator( QValidator * v )
{
    if ( d && d->ed )
	d->ed->setValidator( v );
}


/*!  Returns the validator which constrains editing for this combo
  box if there is any, or else 0.

  \sa setValidator() clearValidator() QValidator
*/

QValidator * QComboBox::validator() const
{
    return d && d->ed ? d->ed->validator() : 0;
}


/*!  This slot is equivalent to setValidator( 0 ). */

void QComboBox::clearValidator()
{
    if ( d && d->ed )
	d->ed->setValidator( 0 );
}


/*!  Sets the combo box to use \a newListBox instead of the current
  list box or popup.  As a site effect, clears the combo box of its
  current contents.

  \warning QComboBox assumes that newListBox->text(n) returns
  non-null for 0 \<= n \< newListbox->count().  This assumption is
  necessary becuase of the line edit in QComboBox.
*/

void QComboBox::setListBox( QListBox * newListBox )
{
    clear();

    if ( d->usingListBox )
	delete d->listBox;
    else
	delete d->popup;

    newListBox->recreate( 0, WType_Popup, QPoint(0,0), FALSE );

    d->listBox = newListBox;
    d->usingListBox = TRUE;

    d->listBox->setAutoScrollBar( FALSE );
    d->listBox->setBottomScrollBar( FALSE );
    d->listBox->setAutoBottomScrollBar( FALSE );
    d->listBox->setFrameStyle( QFrame::Box | QFrame::Plain );
    d->listBox->setLineWidth( 1 );
    d->listBox->resize( 100, 10 );

    connect( d->listBox, SIGNAL(selected(int)),
	     SLOT(internalActivate(int)) );
    connect( d->listBox, SIGNAL(highlighted(int)),
	     SLOT(internalHighlight(int)));
}


/*!  Returns the current list box, or 0 if there is no list box
  currently.  (QComboBox can use QPopupMenu instead of QListBox.)
  Provided to match setListBox().

  \sa setListBox()
*/

QListBox * QComboBox::listBox() const
{
    return d && d->usingListBox ? d->listBox : 0;
}


/*!  Clears the line edit without changing the combo's contents.  Does
  nothing if the combo isn't editable.

  This is particularly handy when using a combo box as a line edit
  with history.  For example you can connect the combo's activated()
  signal to clearEdit() in order to present the user with a new, empty
  line as soon as return is pressed.

  \sa setEditText()
*/

void QComboBox::clearEdit()
{
    if ( d && d->ed )
	d->ed->clear();
}


/*!  Sets the text in the embedded line edit to \a newText without
  changing the combo's contents.  Does nothing if the combo isn't
  editable.

  This is useful e.g. for providing a good starting point for the
  user's editing and entering the change in the combo only when the
  user presses enter.

  \sa clearEditText() insertItem()
*/

void QComboBox::setEditText( const char * newText )
{
    if ( d && d->ed )
	d->ed->setText( newText );
}


/*!  Sets this combo box to offer auto-completion while the user is
  editing if \a enable is TRUE, or not to offer auto-completion of \a
  enable is FALSE (the default).

  The combo box uses the list of items as candidates for completion.

  \sa autoCompletion() setEditText()
*/

void QComboBox::setAutoCompletion( bool enable )
{
    d->useCompletion = enable && (d->ed != 0);
    d->completeNow = FALSE;
}


/*!  Returns TRUE if this combo box is in auto-completion mode.

  \sa setAutoCompletion()
*/

bool QComboBox::autoCompletion() const
{
    return d->useCompletion;
}
