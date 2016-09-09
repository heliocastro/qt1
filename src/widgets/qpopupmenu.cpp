/****************************************************************************
** $Id: qpopupmenu.cpp,v 2.81.2.9 1999/01/26 11:57:30 agulbra Exp $
**
** Implementation of QPopupMenu class
**
** Created : 941128
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

#define	 INCLUDE_MENUITEM_DEF
#include "qpopupmenu.h"
#include "qmenubar.h"
#include "qaccel.h"
#include "qpainter.h"
#include "qdrawutil.h"
#include "qapplication.h"
#include "qbitmap.h"
#include "qpixmapcache.h"
#include "qtimer.h"
#include <ctype.h>

// Motif style parameters

static const int motifPopupFrame	= 2;	// popup frame width
static const int motifItemFrame		= 2;	// menu item frame width
static const int motifSepHeight		= 2;	// separator item height
static const int motifItemHMargin	= 3;	// menu item hor text margin
static const int motifItemVMargin	= 2;	// menu item ver text margin
static const int motifArrowHMargin	= 6;	// arrow horizontal margin
static const int motifArrowVMargin	= 2;	// arrow vertical margin
static const int motifTabSpacing	= 12;	// space between text and tab
static const int motifCheckMarkHMargin	= 2;	// horiz. margins of check mark


/*

+-----------------------------
|      PopupFrame
|   +-------------------------
|   |	   ItemFrame
|   |	+---------------------
|   |	|
|   |	|			   \
|   |	|   ^	T E X T	  ^	    | ItemVMargin
|   |	|   |		  |	   /
|   |	      ItemHMargin
|

*/




// used for internal communication - to be replaced with a class
// members in 2.0
static QPopupMenu * syncMenu;
static int syncMenuId;

// Used to detect motion prior to mouse-release
static int motion;

// used to provide ONE single-shot timer
static QTimer * singleSingleShot = 0;

static bool supressAboutToShow = FALSE;

static void popupSubMenuLater( int msec, QObject * receiver ) {
    if ( !singleSingleShot )
	singleSingleShot = new QTimer( qApp, "popup submenu timer" );
    singleSingleShot->disconnect( SIGNAL(timeout()) );
    QObject::connect( singleSingleShot, SIGNAL(timeout()),
		      receiver, SLOT(subMenuTimer()) );
    singleSingleShot->start( msec, TRUE );
}


//### How to provide new member variables while keeping binary compatibility:
#if QT_VERSION == 200
#error "Remove QPopupMenu dict!"
#endif

#include "qptrdict.h"


struct QPopupMenuExtra {
    bool hasDoubleItem;
    int maxPMWidth;
};


static QPtrDict<QPopupMenuExtra> *qpm_extraStuff = 0;

static void cleanupPopupMenu()
{
    delete qpm_extraStuff;
    qpm_extraStuff = 0;
}


static QPopupMenuExtra * makePMDict( QPopupMenu *that )
{
    if ( !qpm_extraStuff ) {
	qpm_extraStuff = new QPtrDict<QPopupMenuExtra>;
	CHECK_PTR( qpm_extraStuff );
	qpm_extraStuff->setAutoDelete( TRUE );
	qAddPostRoutine( cleanupPopupMenu );
    }

    QPopupMenuExtra *x = (QPopupMenuExtra *)qpm_extraStuff->find( that );
    if ( !x ) {
	x = new QPopupMenuExtra;
	x->hasDoubleItem = FALSE;
	x->maxPMWidth = 0;
	qpm_extraStuff->insert( that, x );
    }

    return x;
}


static QPopupMenuExtra * lookInPMDict( const QPopupMenu *that )
{
    QPopupMenuExtra *x = 0;
    if ( qpm_extraStuff )
	x = (QPopupMenuExtra *)qpm_extraStuff->find( (void *)that );
    return x;
}




/*!
  \class QPopupMenu qpopupmenu.h
  \brief The QPopupMenu class provides a popup menu widget.

  \ingroup realwidgets

  The popup widget is different from other widgets in the way it
  relates to the parent widget.

  menu/menu.cpp is a typical example of QMenuBar and QPopupMenu use.

  \important insertItem clear text pixmap

  <img src=qpopmenu-m.gif> <img src=qpopmenu-w.gif>

  \sa QMenuBar
  <a href="guibooks.html#fowler">GUI Design Handbook: Menu, Drop-Down and
  Pop-Up</a>
*/


/*! \fn void QPopupMenu::aboutToShow()

  This signal is emited just before the popup menu is displayed.  You
  can connect it to any slot that sets up the menu contents (e.g. to
  ensure that the right items are enabled).

  \sa setItemEnabled() setItemChecked() insertItem() removeItem()
*/


// size of checkmark image

static void getSizeOfBitmap( int gs, int *w, int *h )
{
	if ( gs == WindowsStyle )
	    *w = *h = 7;
	else
	    *w = *h = 6;
}


static int getWidthOfCheckCol( QPopupMenu *that, int gs )
{
    QPopupMenuExtra *x= lookInPMDict( that );
    int pmw = x ? x->maxPMWidth : 0;
    int cmw = 7;   // check mark width
    int w = cmw > pmw ? cmw : pmw;
    if ( gs == MotifStyle )
	w += 2;
    w += motifItemFrame + 2 * motifCheckMarkHMargin;
    return w;
}

// Checkmark drawing -- temporarily here...
static void qDrawCheckMark( QPainter *p, int x, int y, int w, int h,
			    const QColorGroup &g, GUIStyle gs,
			    bool act, bool dis )
{
    int markW, markH;
    getSizeOfBitmap( gs, &markW, &markH );
    int posX = x + ( w - markW )/2 - 1;
    int posY = y + ( h - markH )/2;

    if ( gs == WindowsStyle ) {
	// Could do with some optimizing/caching...
	QPointArray a( 7*2 );
	int i, xx, yy;
	xx = posX;
	yy = 3 + posY;
	for ( i=0; i<3; i++ ) {
	    a.setPoint( 2*i,   xx, yy );
	    a.setPoint( 2*i+1, xx, yy+2 );
	    xx++; yy++;
	}
	yy -= 2;
	for ( i=3; i<7; i++ ) {
	    a.setPoint( 2*i,   xx, yy );
	    a.setPoint( 2*i+1, xx, yy+2 );
	    xx++; yy--;
	}
	if ( dis && !act ) {
	    uint pnt;
	    p->setPen( white );
	    QPoint offset(1,1);
	    for ( pnt = 0; pnt < a.size(); pnt++ )
		a[pnt] += offset;
	    p->drawLineSegments( a );
	    for ( pnt = 0; pnt < a.size(); pnt++ )
		a[pnt] -= offset;
	}
#if 0
	p->setPen( act && !dis ? white : g.text() );
#endif
	p->setPen( g.text() );
	p->drawLineSegments( a );
    }
    else {
	QBrush fill( g.mid() );
	qDrawShadePanel( p, posX, posY, markW, markH, g, TRUE, 2, &fill );
    }
}


//
// Creates an accelerator string for the key k.
// For instance CTRL+Key_O gives "Ctrl+O".
//

static QString accel_str( int k )
{
    QString s;
    if ( (k & SHIFT) == SHIFT )
	s = "Shift";
    if ( (k & CTRL) == CTRL ) {
	if ( s.isEmpty() )
	    s = "Ctrl";
	else
	    s += "+Ctrl";
    }
    if ( (k & ALT) == ALT ) {
	if ( s.isEmpty() )
	    s = "Alt";
	else
	    s += "+Alt";
    }
    k &= ~(SHIFT | CTRL | ALT);
    QString p;
    if ( (k & ASCII_ACCEL) == ASCII_ACCEL ) {
	k &= ~ASCII_ACCEL;
	p.sprintf( "%c", (k & 0xff) );
    } else if ( k >= Key_F1 && k <= Key_F24 ) {
	p.sprintf( "F%d", k - Key_F1 + 1 );
    } else if ( k > Key_Space && k <= Key_AsciiTilde ) {
	p.sprintf( "%c", k );
    } else {
	switch ( k ) {
	    case Key_Space:
		p = "Space";
		break;
	    case Key_Escape:
		p = "Esc";
		break;
	    case Key_Tab:
		p = "Tab";
		break;
	    case Key_Backtab:
		p = "Backtab";
		break;
	    case Key_Backspace:
		p = "Backspace";
		break;
	    case Key_Return:
		p = "Return";
		break;
	    case Key_Enter:
		p = "Enter";
		break;
	    case Key_Insert:
		p = "Ins";
		break;
	    case Key_Delete:
		p = "Del";
		break;
	    case Key_Pause:
		p = "Pause";
		break;
	    case Key_Print:
		p = "Print";
		break;
	    case Key_SysReq:
		p = "SysReq";
		break;
	    case Key_Home:
		p = "Home";
		break;
	    case Key_End:
		p = "End";
		break;
	    case Key_Left:
		p = "Left";
		break;
	    case Key_Up:
		p = "Up";
		break;
	    case Key_Right:
		p = "Right";
		break;
	    case Key_Down:
		p = "Down";
		break;
	    case Key_Prior:
		p = "PgUp";
		break;
	    case Key_Next:
		p = "PgDown";
		break;
	    case Key_CapsLock:
		p = "CapsLock";
		break;
	    case Key_NumLock:
		p = "NumLock";
		break;
	    case Key_ScrollLock:
		p = "ScrollLock";
		break;
	    default:
		p.sprintf( "<%d?>", k );
		break;
	}
    }
    if ( s.isEmpty() )
	s = p;
    else {
	s += '+';
	s += p;
    }
    return s;
}


/*****************************************************************************
  QPopupMenu member functions
 *****************************************************************************/

/*!
  Constructs a popup menu with a null parent and a widget name.

  A popup menu must be a top level widget, i.e. parent must be 0.
  This argument is present merely for API uniformity.
*/

QPopupMenu::QPopupMenu( QWidget *parent, const char *name )
    : QTableView( 0, name, WType_Popup )
{
#if defined(CHECK_RANGE)
    if ( parent != 0 )
	warning( "QPopupMenu: (%s) Parent must be null", name );
#endif
    isPopupMenu	  = TRUE;
    selfItem	  = 0;
    autoaccel	  = 0;
    accelDisabled = FALSE;
    popupActive	  = -1;
    tabCheck	  = 0;
    setTabMark( 0 );
    setNumCols( 1 );				// set number of table columns
    setNumRows( 0 );				// set number of table rows
    switch ( style() ) {
	case WindowsStyle:
	    setFrameStyle( QFrame::WinPanel | QFrame::Raised );
	    setMouseTracking( TRUE );
	    setCheckableFlag( TRUE );		
	    break;
	case MotifStyle:
	    setFrameStyle( QFrame::Panel | QFrame::Raised );
	    setLineWidth( motifPopupFrame );
	    setCheckableFlag( FALSE );		
	    break;
	default:
	    setFrameStyle( QFrame::Panel | QFrame::Plain );
	    setLineWidth( 1 );
    }
}

/*!
  Destroys the popup menu.
*/

QPopupMenu::~QPopupMenu()
{
    if ( syncMenu == this ) {
	qApp->exit_loop();
	syncMenu = 0;
    }
	
    if ( qpm_extraStuff )
	qpm_extraStuff->remove( this );
    delete autoaccel;
    if ( parentMenu )
	parentMenu->removePopup( this );	// remove from parent menu
}


void QPopupMenu::updateItem( int id )		// update popup menu item
{
    updateRow( indexOf(id) );
}


// Double use of tabMark / checkingEnabled until 2.0

/*!
  Enables or disables display of check marks by the menu items.

  Notice that checking is always enabled when in windows-style.

  \sa isCheckable(), QMenuData::setItemChecked()
*/

void QPopupMenu::setCheckable( bool enable )
{
    bool oldState = isCheckable();
    bool newState = (style() == WindowsStyle) || enable;
    if ( oldState != newState ) {
	setCheckableFlag( newState );
	if ( !newState ) {
	    // turning off isCheckable; must look for pixmaps
	    updateSize();
	}
    }
}


/*!
  Does the internal magic necessary to set the checkable flag.
 */
void QPopupMenu::setCheckableFlag( bool enable )
{
    bool oldState = isCheckable();
    bool newState = (style() == WindowsStyle) || enable;
    if ( oldState != newState ) {
	if ( newState ) {
	    setNumCols( 2 );
	    tabCheck |= 0x80000000;
	} else {
	    setNumCols( 1 );
	    tabCheck &= 0x7FFFFFFF;
	}
	badSize = TRUE;
	update();
    }
}




/*!
  Returns whether display of check marks by the menu items is enabled.

  \sa setCheckable(), QMenuData::setItemChecked()
*/

bool QPopupMenu::isCheckable() const
{
    return (tabCheck & 0x80000000) != 0;
}


void QPopupMenu::setTabMark( int t )
{
    bool e = isCheckable();
    tabCheck = t;
    if ( e )
	tabCheck |= 0x80000000;
}


int QPopupMenu::tabMark()
{
    return tabCheck & 0x7FFFFFFF;
}


void QPopupMenu::menuContentsChanged()
{
    badSize = TRUE;				// might change the size
    updateAccel( 0 );
    updateSize(); // ### SLOW, needs some rework
    if ( isVisible() ) {
	//	updateSize();
	repaint();
    }
}

void QPopupMenu::menuStateChanged()
{
    repaint();
}

void QPopupMenu::menuInsPopup( QPopupMenu *popup )
{
    popup->parentMenu = this;			// set parent menu
    connect( popup, SIGNAL(activatedRedirect(int)),
	     SLOT(subActivated(int)) );
    connect( popup, SIGNAL(highlightedRedirect(int)),
	     SLOT(subHighlighted(int)) );
}

void QPopupMenu::menuDelPopup( QPopupMenu *popup )
{
    popup->parentMenu = 0;
    popup->disconnect( SIGNAL(activatedRedirect(int)), this,
		       SLOT(subActivated(int)) );
    popup->disconnect( SIGNAL(highlightedRedirect(int)), this,
		       SLOT(subHighlighted(int)) );
}


void QPopupMenu::frameChanged()
{
    menuContentsChanged();
}


/*!
  Opens the popup menu so that the item number \a indexAtPoint will be
  at the specified \e global position \a pos.  To translate a widget's
  local coordinates into global coordinates, use QWidget::mapToGlobal().
*/

void QPopupMenu::popup( const QPoint &pos, int indexAtPoint )
{
    if (parentMenu && parentMenu->actItem == -1){
	//reuse
	parentMenu->menuDelPopup( this );
	parentMenu = 0;
    }
    // #### should move to QWidget - anything might need this functionality,
    // #### since anything can have WType_Popup window flag.

    if ( mitems->count() == 0 )			// oops, empty
	insertSeparator();			// Save Our Souls
    if ( badSize )
	updateSize();
    QWidget *desktop = QApplication::desktop();
    int sw = desktop->width();			// screen width
    int sh = desktop->height();			// screen height
    int x  = pos.x();
    int y  = pos.y();
    if ( indexAtPoint > 0 )			// don't subtract when < 0
	y -= itemPos( indexAtPoint );		// (would subtract 2 pixels!)
    int w  = width();
    int h  = height();
    if ( x+w > sw )				// the complete widget must
	x = sw - w;				//   be visible
    if ( y+h > sh )
	y = sh - h;
    if ( x < 0 )
	x = 0;
    if ( y < 0 )
	y = 0;
    move( x, y );
    show();
    motion=0;
}

/*!
  \fn void QPopupMenu::activated( int id )

  This signal is emitted when a menu item is selected; \a id is the id
  of the selected item.

  Normally, you will connect each menu item to a single slot using
  QMenuData::insertItem(), but sometimes you will want to connect
  several items to a single slot (most often if the user selects from
  an array).  This signal is handy in such cases.

  \sa highlighted(), QMenuData::insertItem()
*/

/*!
  \fn void QPopupMenu::highlighted( int id )

  This signal is emitted when a menu item is highlighted; \a id is the
  id of the highlighted item.

  Normally, you will connect each menu item to a single slot using
  QMenuData::insertItem(), but sometimes you will want to connect
  several items to a single slot (most often if the user selects from
  an array).  This signal is handy in such cases.

  \sa activated(), QMenuData::insertItem()
*/

/*! \fn void QPopupMenu::highlightedRedirect( int id )
  \internal
  Used internally to connect submenus to their parents.
*/

/*! \fn void QPopupMenu::activatedRedirect( int id )
  \internal
  Used internally to connect submenus to their parents.
*/

void QPopupMenu::subActivated( int id )
{
    emit activatedRedirect( id );
}

void QPopupMenu::subHighlighted( int id )
{
    emit highlightedRedirect( id );
}

void QPopupMenu::accelActivated( int id )
{
    QMenuItem *mi = findItem( id );
    if ( mi && mi->isEnabled() ) {
	if ( mi->signal() )			// activate signal
	    mi->signal()->activate();
	actSig( mi->id() );
    }
}

void QPopupMenu::accelDestroyed()		// accel about to be deleted
{
    autoaccel = 0;				// don't delete it twice!
}


void QPopupMenu::actSig( int id )
{
    emit activated( id );
    emit activatedRedirect( id );
}

void QPopupMenu::hilitSig( int id )
{
    emit highlighted( id );
    emit highlightedRedirect( id );
}


void QPopupMenu::setFirstItemActive()
{
    QMenuItemListIt it(*mitems);
    register QMenuItem *mi;
    actItem = 0;
    while ( (mi=it.current()) ) {
	++it;
	if ( !mi->isSeparator() ) {
	    repaint( FALSE );
	    return;
	}
	actItem++;
    }
    actItem = -1;
}

/*!
  \internal
  Hides all popup menus (in this menu tree) that are currently open.
*/

void QPopupMenu::hideAllPopups()
{
    register QMenuData *top = this;		// find top level popup
    while ( top->parentMenu && top->parentMenu->isPopupMenu )
	top = top->parentMenu;
    ((QPopupMenu*)top)->hide();			// cascade from top level
}

/*!
  \internal
  Hides all popup sub-menus.
*/

void QPopupMenu::hidePopups()
{
    QMenuItemListIt it(*mitems);
    register QMenuItem *mi;
    while ( (mi=it.current()) ) {
	++it;
	if ( mi->popup() )
	    mi->popup()->hide();
    }
    popupActive = -1;				// no active sub menu
}


/*!
  \internal
  Sends the event to the menu bar.
*/

bool QPopupMenu::tryMenuBar( QMouseEvent *e )
{
    register QMenuData *top = this;		// find top level
    while ( top->parentMenu )
	top = top->parentMenu;
    return top->isMenuBar ?
	((QMenuBar *)top)->tryMouseEvent( this, e ) : FALSE;
}

/*!
  \internal
  Tells the menu bar to go back to idle state.
*/

void QPopupMenu::byeMenuBar()
{
    hideAllPopups();
    register QMenuData *top = this;		// find top level
    while ( top->parentMenu )
	top = top->parentMenu;
    if ( top->isMenuBar )
	((QMenuBar *)top)->goodbye();
}


/*!
  \internal
  Return the item at \e pos, or -1 if there is no item there, or if
  it is a separator item.
*/

int QPopupMenu::itemAtPos( const QPoint &pos )
{
    int row = findRow( pos.y() );		// ask table for row
    int col = findCol( pos.x() );		// ask table for column
    int r = -1;
    if ( row != -1 && col != -1 ) {
	QMenuItem *mi = mitems->at(row);
	if ( !mi->isSeparator() )
	    r = row;				// normal item
    }
    return r;
}

/*!
  \internal
  Returns the y (top) position of item number \e index.
*/

int QPopupMenu::itemPos( int index )		// get y coord for item
{
    int y;
    if ( rowYPos( index, &y ) )			// ask table for position
	return y;
    else
	return 0;				// return 0 if not visible
}


/*!
  \internal
  Calculates and sets the size of the popup menu, based on the size
  of the items.
*/

void QPopupMenu::updateSize()
{
    int height	     = 0;
    int max_width    = 10;
    int max_pm_width = 0;
    GUIStyle gs	  = style();
    QFontMetrics fm = fontMetrics();
#if 0
    QFontMetrics fm( font() );
#endif
    QMenuItemListIt it( *mitems );
    register QMenuItem *mi;
    bool hasSubMenu = FALSE;
    int cellh = fm.height() + 2*motifItemVMargin + 2*motifItemFrame;
    int tab_width = 0;
    bool hasTextAndPixmapItem = FALSE;
    while ( (mi=it.current()) ) {
	int w = 0;
	int itemHeight = 0;
	if ( mi->popup() )
	    hasSubMenu = TRUE;
	if ( mi->isSeparator() )
	    itemHeight = motifSepHeight;
	else if ( mi->pixmap() ) {
	    itemHeight = mi->pixmap()->height() + 2*motifItemFrame;
	    if ( mi->text() ) {
		if ( gs == MotifStyle )
		    itemHeight += 2;		// Room for check rectangle
		hasTextAndPixmapItem = TRUE; // has text, w will be set below
		if ( mi->pixmap()->width() > max_pm_width )
		    max_pm_width = mi->pixmap()->width();
	    } else {
		w = mi->pixmap()->width();	// pixmap only
	    }
	}
	if ( mi->text() && !mi->isSeparator() ) {
	    if ( itemHeight < cellh )
		itemHeight = cellh;
	    const char *s = mi->text();
	    const char *t;
	    if ( (t=strchr(s, '\t')) ) {	// string contains tab
		w = fm.width( s, (int)((long)t-(long)s) );
		int tw = fm.width( t+1 );
		if ( tw > tab_width )
		    tab_width = tw;
	    } else {
		w += fm.width( s );
	    }
	}
	height += itemHeight;
#if defined(CHECK_NULL)
	if ( !mi->text() && !mi->pixmap() && !mi->isSeparator() )
	    warning( "QPopupMenu: (%s) Popup has invalid menu item",
		     name( "unnamed" ) );
#endif
	if ( max_width < w )
	    max_width = w;
	++it;
    }
    if ( hasTextAndPixmapItem ) {
	QPopupMenuExtra *x = makePMDict( this );
	x->hasDoubleItem = TRUE;
	x->maxPMWidth = max_pm_width;
    } else {
	QPopupMenuExtra *x = lookInPMDict( this );
	if ( x ) {
	    x->hasDoubleItem = FALSE;
	    x->maxPMWidth    = 0;
	}
    }
    if ( gs == MotifStyle ) {
	setCheckableFlag( isCheckable() || hasTextAndPixmapItem );
    }
    int extra_width = 0;
    if ( tab_width ) {
	extra_width = tab_width + motifTabSpacing;
	setTabMark( max_width + motifTabSpacing );
    }
    else
	setTabMark( 0 );

    max_width  += 2*motifItemHMargin;

    if ( isCheckable() )
	max_width += getWidthOfCheckCol( this, gs ) + motifItemFrame;
    else
	max_width += 2*motifItemFrame;

    if ( hasSubMenu ) {
	if ( fm.ascent() + motifArrowHMargin > extra_width )
	    extra_width = fm.ascent() + motifArrowHMargin;
    }
    max_width += extra_width;
    setNumRows( mitems->count() );
    resize( max_width+2*frameWidth(), height+2*frameWidth() );
    badSize = FALSE;
}



/*!
  \internal
  The \e parent is 0 when it is updated when a menu item has
  changed a state, or it is something else if called from the menu bar.
*/

void QPopupMenu::updateAccel( QWidget *parent )
{
    QMenuItemListIt it(*mitems);
    register QMenuItem *mi;
    if ( parent == 0 && autoaccel == 0 )
	return;
    if ( autoaccel )				// build it from scratch
	autoaccel->clear();
    while ( (mi=it.current()) ) {
	++it;
	if ( mi->key() ) {
	    if ( !autoaccel ) {
		autoaccel = new QAccel( parent );
		CHECK_PTR( autoaccel );
		connect( autoaccel, SIGNAL(activated(int)),
			 SLOT(accelActivated(int)) );
		connect( autoaccel, SIGNAL(destroyed()),
			 SLOT(accelDestroyed()) );
		if ( accelDisabled )
		    autoaccel->setEnabled( FALSE );
	    }
	    int k = mi->key();
	    autoaccel->insertItem( k, mi->id() );
	    if ( mi->text() ) {
		QString s = mi->text();
		int i = s.find('\t');
		QString t = accel_str( k );
		if ( i >= 0 )
		    s.replace( i+1, s.length()-i, t );
		else {
		    s += '\t';
		    s += t;
		}
		if ( s != mi->text() ) {
		    mi->setText( s );
		    badSize = TRUE;
		}
	    }
	}
	if ( mi->popup() && parent )		// call recursively
	    mi->popup()->updateAccel( parent );
    }
}

/*!
  \internal
  It would be better to check in the slot.
*/

void QPopupMenu::enableAccel( bool enable )
{
    if ( autoaccel )
	autoaccel->setEnabled( enable );
    else
	accelDisabled = TRUE;			// rememeber when updateAccel
    QMenuItemListIt it(*mitems);
    register QMenuItem *mi;
    while ( (mi=it.current()) ) {		// do the same for sub popups
	++it;
	if ( mi->popup() )			// call recursively
	    mi->popup()->enableAccel( enable );
    }
}


/*!
  Reimplements QWidget::setFont() to be able to refresh the popup menu
  when its font changes.
*/

void QPopupMenu::setFont( const QFont &font )
{
    QWidget::setFont( font );
    badSize = TRUE;
    update();
}

/*!
  Reimplements QWidget::show() for internal purposes.
*/

void QPopupMenu::show()
{
    if ( testWFlags(WState_Visible) ){
	supressAboutToShow = FALSE;
	return;
    }
    if (!supressAboutToShow)
	emit aboutToShow();
    else
	supressAboutToShow = FALSE;
    if ( badSize )
	updateSize();
    QWidget::show();
    popupActive = -1;
}

/*!
  Reimplements QWidget::hide() for internal purposes.
*/

void QPopupMenu::hide()
{
    actItem = popupActive = -1;
    mouseBtDn = FALSE;				// mouse button up
    hidePopups();
    killTimers();
    QWidget::hide();
    if ( syncMenu == this && qApp )
	qApp->exit_loop();
}

#if 0
/*!
  Reimplements QWidget::setEnabled() for internal purposes.
*/

void QPopupMenu::setEnabled( bool enable )
{
    if ( enable == isEnabled() )
	return;
    if ( parentMenu ) {
	QMenuItem *mi = parentMenu->findPopup( this );
	if ( mi ) {
	    parentMenu->setItemEnabled( mi->id(), enable );
	}
    }
    QWidget::setEnabled( enable );
}
#endif

/*****************************************************************************
  Implementation of virtual QTableView functions
 *****************************************************************************/

/*! \reimp */

int QPopupMenu::cellHeight( int row )
{
    QMenuItem *mi = mitems->at( row );
    if ( !mi )
	return 0;
    int h = 0;
    if ( mi->isSeparator() ) {			// separator height
	h = motifSepHeight;
    } else if ( mi->pixmap() ) {		// pixmap height
	h = mi->pixmap()->height() + 2*motifItemFrame;
	if ( mi->text() ) {			// pixmap and text
	    if ( style() == MotifStyle )
		h += 2;				// Room for check rectangle
	    QFontMetrics fm = fontMetrics();
	    int h2 = fm.height() + 2*motifItemVMargin + 2*motifItemFrame;
	    if ( h2 > h )
		h = h2;
	}
    } else {					// text height
	QFontMetrics fm = fontMetrics();
	h = fm.height() + 2*motifItemVMargin + 2*motifItemFrame;
    }
    return h;
}


/*! \reimp */

int QPopupMenu::cellWidth( int col )
{
    if ( isCheckable() ) {
	if ( col == 0 )
	    return getWidthOfCheckCol(this,style());
	else
	    return width() - (2*frameWidth()+getWidthOfCheckCol(this,style()));
    }	
    else
	return width() - 2*frameWidth();	
}


/*! \reimp */

void QPopupMenu::paintCell( QPainter *p, int row, int col )
{
    QColorGroup g = colorGroup();
    QMenuItem *mi = mitems->at( row );		// get menu item
    int cellh	  = cellHeight( row );
    int cellw	  = cellWidth( col );
    GUIStyle gs	  = style();
    bool act	  = row == actItem;
    bool dis	  = (selfItem && !selfItem->isEnabled()) || !mi->isEnabled();
    QColorGroup itemg = dis ? palette().disabled()
			: act ? palette().active()
			: palette().normal();

    if ( !mi->isDirty() )
	return;

    int rw = isCheckable() ? totalWidth() : cellw;

    if ( col == 0 ) {
	if ( mi->isSeparator() ) {			// draw separator
	    p->setPen( g.dark() );
	    p->drawLine( 0, 0, rw, 0 );
	    p->setPen( g.light() );
	    p->drawLine( 0, 1, rw, 1 );
	    return;
	}

	int cm = gs == MotifStyle ? 2 : 0;	// checkable margin

	if ( mi->isChecked() ) {
	    if ( gs == WindowsStyle && act && !dis ) {
		QBrush b( g.mid() );
		qDrawShadePanel( p, cm, cm, cellw-2*cm, cellh-2*cm,
				 g, TRUE, 1, &b );
	    } else if ( gs == WindowsStyle ||
			mi->pixmap() && mi->text() ) {
		QBrush b( g.background() );
		qDrawShadePanel( p, cm, cm, cellw-2*cm, cellh-2*cm,
				 g, TRUE, 1, &b );
	    }
	} else if ( !act ) {
	    qDrawPlainRect( p, cm, cm, cellw-2*cm, cellh-2*cm,
			    g.background(), 1, 0 );
	}		

	if ( mi->text() && mi->pixmap() ) {		// draw pixmap
	    QPixmap *pixmap = mi->pixmap();
	    int pixw = pixmap->width();
	    int pixh = pixmap->height();
	    if ( gs == MotifStyle ) {
		if ( act && !dis )			// active item frame
		    qDrawShadePanel( p, 0, 0, rw, cellh, g, FALSE,
				     motifItemFrame );
		else				// incognito frame
		    qDrawPlainRect( p, 0, 0, rw, cellh, g.background(),
				    motifItemFrame );
	    } else {
		if ( act && !dis ) {
		    if ( !mi->isChecked() )
			qDrawShadePanel( p, 0, 0, cellw, cellh, g, FALSE, 1 );
		}
	    }
	    QRect cr( cm, cm, cellw-2*cm, cellh-2*cm );
	    QRect pmr( 0, 0, pixw, pixh );
	    pmr.moveCenter( cr.center() );
	    if ( style() == WindowsStyle && dis ) {
		QString k;
		k.sprintf( "$qt-drawitem-%x", pixmap->serialNumber() );
		QPixmap * mask = QPixmapCache::find(k);
		bool del = FALSE;
		if ( !mask ) {
		    if ( pixmap->mask() )
			mask = new QPixmap( *pixmap->mask() );
		    else
			mask = new QPixmap( pixmap->createHeuristicMask() );
		    mask->setMask( *((QBitmap*)mask) );
		    del = !QPixmapCache::insert( k, mask );
		}
		p->setPen( itemg.light() );
		p->drawPixmap( pmr.left()+1, pmr.top()+1, *mask );
		p->setPen( itemg.text() );
		p->drawPixmap( pmr.topLeft(), *mask );
		if ( del ) delete mask;
	    } else {
		p->setPen( itemg.text() );
		p->drawPixmap( pmr.topLeft(), *pixmap );
	    }
	    if ( gs == WindowsStyle ) {
		p->fillRect( cellw + 1, 0, rw - cellw - 1, cellh,
			     act ? QApplication::winStyleHighlightColor()
			     : g.background());
	    }
	    return;
	}

	int pw = motifItemFrame;
	if ( gs != MotifStyle )
	    pw = 1;
	if ( gs == WindowsStyle ) {
	    if ( mi->isChecked() )
		p->fillRect( cellw + 1, 0, rw - cellw - 1, cellh,
			     act ? QApplication::winStyleHighlightColor()
			     : g.background() );
	    else
		p->fillRect( 0, 0, rw, cellh,
			     act ? QApplication::winStyleHighlightColor()
			     : g.background() );
	} else if ( gs == MotifStyle ) {
	    if ( act && !dis )			// active item frame
		qDrawShadePanel( p, 0, 0, rw, cellh, g, FALSE, pw );
	    else				// incognito frame
		qDrawPlainRect( p, 0, 0, rw, cellh, g.background(), pw );
	}

	if ( isCheckable() ) {	// just "checking"...
	    int mw = cellw - ( 2*motifCheckMarkHMargin + motifItemFrame );
	    int mh = cellh - 2*motifItemFrame;
	    if ( mi->isChecked() ) {
		qDrawCheckMark( p, motifItemFrame + motifCheckMarkHMargin,
				motifItemFrame, mw, mh, itemg, gs, act, dis );
	    }
	    return;
	}
    }

    if ( gs == WindowsStyle )
	p->setPen( act ? white : g.text() );
    else
	p->setPen( g.text() );

    QColor discol;
    if ( dis ) {
	discol = itemg.text();
	p->setPen( discol );
    }

    int x = motifItemHMargin + ( isCheckable() ? 0 : motifItemFrame);
    if ( mi->text() ) {			// draw text
	const char *s = mi->text();
	const char *t = strchr( s, '\t' );
	int m = motifItemVMargin;
	const int text_flags = AlignVCenter|ShowPrefix | DontClip | SingleLine;
	if ( t ) {				// draw text before tab
	    if ( gs == WindowsStyle && dis && !act ) {
		p->setPen( g.light() );
		p->drawText( x+1, m+1, cellw, cellh-2*m, text_flags,
			     s, (int)((long)t-(long)s) );
		p->setPen( discol );
	    }
	    p->drawText( x, m, cellw, cellh-2*m, text_flags,
			 s, (int)((long)t-(long)s) );
	    s = t + 1;
	    x = tabMark();
	}
	if ( gs == WindowsStyle && dis && !act ) {
	    p->setPen( g.light() );
	    p->drawText( x+1, m+1, cellw, cellh-2*m, text_flags, s );
	    p->setPen( discol );
	}
	p->drawText( x, m, cellw, cellh-2*m, text_flags, s );
    } else if ( mi->pixmap() ) {			// draw pixmap
	QPixmap *pixmap = mi->pixmap();
	if ( pixmap->depth() == 1 )
	    p->setBackgroundMode( OpaqueMode );
	p->drawPixmap( x, motifItemFrame, *pixmap );
	if ( pixmap->depth() == 1 )
	    p->setBackgroundMode( TransparentMode );
    }
    if ( mi->popup() ) {			// draw sub menu arrow
	int dim = (cellh-2*motifItemFrame) / 2;
	if ( gs == WindowsStyle && row == actItem ) {
	    if ( !dis )
		discol = white;
	    g = QColorGroup( discol, QApplication::winStyleHighlightColor(),
			     white, white,
			     dis ? discol : white,
			     discol, white );
	}
	qDrawArrow( p, RightArrow, gs,
		    row == actItem && gs == MotifStyle && mi->isEnabled(),
		    cellw - motifArrowHMargin - dim,  cellh/2-dim/2,
		    dim, dim, g,
		    gs == MotifStyle && mi->isEnabled() ||
		    gs == WindowsStyle && (mi->isEnabled() || row == actItem));
    }
    mi->setDirty( FALSE );
}


void QPopupMenu::paintAll()
{
    QMenuItemListIt it(*mitems);
    QMenuItem *mi;
    int row = 0;
    while ( (mi=it.current()) ) {
	++it;
	if ( mi->isDirty() )			// this item needs a refresh
	    updateRow( row );
	++row;
    }
}


/*****************************************************************************
  Event handlers
 *****************************************************************************/

/*!
  Handles paint events for the popup menu.
*/

void QPopupMenu::paintEvent( QPaintEvent *e )
{
    setAllDirty( TRUE );
    QTableView::paintEvent( e );
    setAllDirty( FALSE );
}


/*!
  Handles mouse press events for the popup menu.
*/

void QPopupMenu::mousePressEvent( QMouseEvent *e )
{
    mouseBtDn = TRUE;				// mouse button down
    int item = itemAtPos( e->pos() );
    if ( item == -1 ) {
	if ( !rect().contains(e->pos()) && !tryMenuBar(e) ) {
	    byeMenuBar();
	}
	return;
    }
    register QMenuItem *mi = mitems->at(item);
    if ( item != actItem ) {			// new item activated
	actItem = item;
	repaint( FALSE );
	hilitSig( mi->id() );
    }
    QPopupMenu *popup = mi->popup();
    if ( popup ) {
	if ( popup->isVisible() ) {		// sub menu already open
	    popup->actItem = -1;
	    popup->hidePopups();
	    popup->repaint( FALSE );
	} else {				// open sub menu
	    hidePopups();
	    popupSubMenuLater( 20, this );
	}
    } else {
	hidePopups();
    }
}

/*!
  Handles mouse release events for the popup menu.
*/

void QPopupMenu::mouseReleaseEvent( QMouseEvent *e )
{
    if ( !mouseBtDn && !parentMenu && actItem < 0 && motion < 5 )
	return;

    mouseBtDn = FALSE;

    int item = itemAtPos( e->pos() );
    if ( item == -1 ) {
	if ( !rect().contains( e->pos() ) && tryMenuBar(e) )
	    return;
    }
    repaint( FALSE );
    if ( actItem >= 0 ) {			// selected menu item!
	register QMenuItem *mi = mitems->at(actItem);
	QPopupMenu *popup = mi->popup();
	if ( popup ) {
	    popup->setFirstItemActive();
	} else {				// normal menu item
	    byeMenuBar();			// deactivate menu bar
	    if ( mi->isEnabled() ) {
		if ( mi->signal() )		// activate signal
		    mi->signal()->activate();
		actSig( mi->id() );
	    }
	}
    } else {
	byeMenuBar();
    }
}

/*!
  Handles mouse move events for the popup menu.
*/

void QPopupMenu::mouseMoveEvent( QMouseEvent *e )
{
    motion++;
    if ( parentMenu && parentMenu->isPopupMenu &&
	 (parentMenu->actItem != ((QPopupMenu *)parentMenu)->popupActive ) ) {
	// hack it to work: if there's a parent popup, and its active
	// item is not the same as its popped-up child, make the
	// popped-up child active
	QPopupMenu * p = (QPopupMenu *)parentMenu;
	int lastActItem = p->actItem;
	p->actItem = p->popupActive;
	if ( lastActItem >= 0 )
	    p->updateRow( lastActItem );
	if ( p->actItem >= 0 )
	    p->updateRow( p->actItem );
    }

    if ( (e->state() & MouseButtonMask) == 0 && !hasMouseTracking() )
	return;

    int	 item = itemAtPos( e->pos() );
    if ( item == -1 ) {				// no valid item
	int lastActItem = actItem;
	actItem = -1;
	if ( lastActItem >= 0 )
	    updateRow( lastActItem );

	if ( !rect().contains( e->pos() ) && !tryMenuBar( e ) )
	    popupSubMenuLater( style() == WindowsStyle ? 256 : 96, this );
	else if ( singleSingleShot )
	    singleSingleShot->stop();
    } else {					// mouse on valid item
	// but did not register mouse press
	if ( (e->state() & MouseButtonMask) && !mouseBtDn )
	    mouseBtDn = TRUE; // so mouseReleaseEvent will pop down

	register QMenuItem *mi = mitems->at( item );
	if ( actItem == item )
	    return;

	if ( mi->popup() || (popupActive >= 0 && popupActive != item) )
	    popupSubMenuLater( style() == WindowsStyle ? 256 : 96, this );
	else if ( singleSingleShot )
	    singleSingleShot->stop();

	int lastActItem = actItem;
	actItem = item;
	if ( lastActItem >= 0 )
	    updateRow( lastActItem );
	updateRow( actItem );
	hilitSig( mi->id() );
    }
}


/*!
  Handles key press events for the popup menu.
*/

void QPopupMenu::keyPressEvent( QKeyEvent *e )
{
    QMenuItem  *mi = 0;
    QPopupMenu *popup;
    int d = 0;
    bool ok_key = TRUE;

    switch ( e->key() ) {
    case Key_Up:
	d = -1;
	break;

    case Key_Down:
	d = 1;
	break;

    case Key_Alt:
	if ( style() == WindowsStyle ) {
	    byeMenuBar();
	}
	break;

    case Key_Escape:
	// just hide one
	hide();
	if ( parentMenu && parentMenu->isMenuBar )
	    ((QMenuBar *)parentMenu)->setWindowsAltMode( FALSE, -1);
	break;

    case Key_Left:
	if ( parentMenu && parentMenu->isPopupMenu ) {
	    ((QPopupMenu *)parentMenu)->hidePopups();
	    if ( singleSingleShot )
		singleSingleShot->stop();
	} else {
	    ok_key = FALSE;
	}
	break;

    case Key_Right:
	if ( actItem >= 0 && (popup=mitems->at( actItem )->popup()) ) {
	    hidePopups();
	    if ( singleSingleShot )
		singleSingleShot->stop();
	    popup->setFirstItemActive();
	    subMenuTimer();
	} else {
	    ok_key = FALSE;
	}
	break;

    case Key_Space:
	if ( style() != MotifStyle )
	    break;
	// for motif, fall through

    case Key_Return:
    case Key_Enter:
	if ( actItem < 0 )
	    break;
	mi = mitems->at( actItem );
	popup = mi->popup();
	if ( popup ) {
	    hidePopups();
	    popupSubMenuLater( 20, this );
	    popup->setFirstItemActive();
	} else {
	    byeMenuBar();
	    if ( mi->isEnabled() ) {
		if ( mi->signal() )
		    mi->signal()->activate();
		actSig( mi->id() );
	    }
	}
	break;

    default:
	ok_key = FALSE;

    }
#if 1
    if ( !ok_key && !e->state() && e->key() >= Key_0 && e->key() <= Key_Z ) {
	char c = '0' + e->key() - Key_0;

	QMenuItemListIt it(*mitems);
	register QMenuItem *m;
	int indx = 0;
	while ( (m=it.current()) ) {
	    ++it;
	    QString s = m->text();
	    if ( !s.isEmpty() ) {
		int i = s.find( '&' );
		if ( i >= 0 && isalnum(s[i+1]) ) {
		    int k = s[i+1];
		    if ( toupper(k) == c ) {
			mi = m;
			ok_key = TRUE;
			break;
		    }
		}
	    }
	    indx++;
	}
	if ( mi ) {
	    popup = mi->popup();
	    if ( popup ) {
		setActiveItem( indx );
		hidePopups();
		popupSubMenuLater( 20, this );
		popup->setFirstItemActive();
	    } else {
		byeMenuBar();
		if ( mi->isEnabled() ) {
		    if ( mi->signal() )
			mi->signal()->activate();
		    actSig( mi->id() );
		}
	    }
	}
    }
#endif
    if ( !ok_key ) {				// send to menu bar
	register QMenuData *top = this;		// find top level
	while ( top->parentMenu )
	    top = top->parentMenu;
	if ( top->isMenuBar )
	    ((QMenuBar*)top)->tryKeyEvent( this, e );
    }

    if ( d && actItem < 0 ) {
	setFirstItemActive();
    } else if ( d ) {				// highlight next/prev
	register int i = actItem;
	int c = mitems->count();
	int n = c;
	while ( n-- ) {
	    i = i + d;
	    if ( i == c )
		i = 0;
	    else if ( i < 0 )
		i = c - 1;
	    mi = mitems->at( i );
	    if ( !mi->isSeparator()
		 && ( style() != MotifStyle || mi->isEnabled() ) )
		break;
	}
	if ( i != actItem ) {
	    int lastActItem = actItem;
	    actItem = i;
	    if ( mi->id() >= 0 )
		hilitSig( mi->id() );
	    updateRow( lastActItem );
	    updateRow( actItem );
	}
    }
}


/*!
  Handles timer events for the popup menu.
*/

void QPopupMenu::timerEvent( QTimerEvent *e )
{
    QTableView::timerEvent( e );
}


/*! This private slot handles the delayed submenu effects */

void QPopupMenu::subMenuTimer() {
    if ( (actItem < 0 && popupActive < 0) || actItem == popupActive )
	return;

    if ( popupActive >= 0 ) {
	hidePopups();
	popupActive = -1;
    }

    QMenuItem *mi = mitems->at(actItem);
    if ( !mi->isEnabled() )
	return;

    QPopupMenu *popup = mi->popup();
    if ( !popup || !popup->isEnabled() )
	return;

    if (popup->parentMenu != this ){
	// reuse
	if (popup->parentMenu)
	    popup->parentMenu->menuDelPopup(popup);
	menuInsPopup(popup);
    }

    emit popup->aboutToShow();
    supressAboutToShow = TRUE;

    QPoint p( width() - motifArrowHMargin, frameWidth() + motifArrowVMargin );
    for ( int i=0; i<actItem; i++ )
	p.setY( p.y() + (QCOORD)cellHeight( i ) );
    p = mapToGlobal( p );
    if ( popup->badSize )
	popup->updateSize();
    if (p.y() + popup->height() > QApplication::desktop()->height()
	&& p.y() - popup->height()
	+ (QCOORD)(popup->cellHeight( popup->count()-1)) >= 0)
	p.setY( p.y() - popup->height()
		+ (QCOORD)(popup->cellHeight( popup->count()-1)));
    popupActive = actItem;
    bool left = FALSE;
    if ( ( parentMenu && parentMenu->isPopupMenu &&
	   ((QPopupMenu*)parentMenu)->geometry().x() > geometry().x() ) ||
	 p.x() + popup->width() > QApplication::desktop()->width() )
	left = TRUE;
    if ( left && popup->width() > mapToGlobal( QPoint(0,0) ).x() )
	left = FALSE;
    if ( left )
	p.setX( mapToGlobal(QPoint(0,0)).x() - popup->width() + frameWidth() );
    popup->popup( p );
}


void QPopupMenu::updateRow( int row )
{
    updateCell( row, 0, FALSE );
    if ( isCheckable() )
	updateCell( row, 1, FALSE );
}


/*!  Execute this popup synchronously.

  Opens the popup menu so that the item number \a indexAtPoint will be
  at the specified \e global position \a pos.  To translate a widget's
  local coordinates into global coordinates, use QWidget::mapToGlobal().

  The return code is the ID of the selected item, or -1 if no item is
  selected (normally because the user presses Escape).

  Note that all signals are emitted as usual.  If you connect a menu
  item to a slot and call the menu's exec(), you get the result both
  via the signal-slot connection and in the return value of exec().

  Common usage is to position the popup at the current
  mouse position:
  \code
      exec(QCursor::pos());
  \endcode
  or aligned to a widget:
  \code
      exec(somewidget.mapToGlobal(QPoint(0,0)));
  \endcode
  \sa popup()
*/

int QPopupMenu::exec( const QPoint & pos, int indexAtPoint )
{
    if ( !qApp )
	return -1;

    syncMenu = this;
    syncMenuId = -1;
    connect( this, SIGNAL(activated(int)),
	     this, SLOT(modalActivation(int)) );
    popup(pos,indexAtPoint);
    qApp->enter_loop();
    disconnect( this, SIGNAL(activated(int)),
		this, SLOT(modalActivation(int)) );
    syncMenu = 0;

    return syncMenuId;
}

/*!  Execute this popup synchronously.

  Similar to the above function, but the position of the
  popup is not set, so you must choose an appropriate position.
  The function move the popup if it is partially off-screen.

  More common usage is to position the popup at the current
  mouse position:
  \code
      exec(QCursor::pos());
  \endcode
  or aligned to a widget:
  \code
      exec(somewidget.mapToGlobal(QPoint(0,0)));
  \endcode
*/

int QPopupMenu::exec()
{
    return exec(mapToGlobal(QPoint(0,0)));
}


/*!  Internal slot used for exec(). */

void QPopupMenu::modalActivation( int id )
{
    syncMenuId = id;
}


/*!  Sets the currently active item to \a i and repaints as necessary.
*/

void QPopupMenu::setActiveItem( int i )
{
    int lastActItem = actItem;
    actItem = i;
    if ( lastActItem >= 0 )
	updateRow( lastActItem );
    if ( i >= 0 && i != lastActItem )
	updateRow( i );
}

/*!
  Returns the identifier of the menu item at position \a index in the
  internal list, or -1 if \a index is out of range.

  \sa QMenuData::setId(), QMenuData::indexOf()
*/

int QPopupMenu::idAt( const QPoint& pos ) const
{
    QPopupMenu* that = (QPopupMenu*) this;
    return idAt( that->itemAtPos( pos ) );
}
