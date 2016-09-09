/****************************************************************************
** $Id: qmainwindow.cpp,v 2.28.2.4 1998/11/01 20:45:01 agulbra Exp $
**
** Implementation of QMainWindow class
**
** Created : 980312
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

#include "qmainwindow.h"

#include "qlist.h"

#include "qtimer.h"
#include "qlayout.h"
#include "qobjectlist.h"
#include "qobjectdict.h"
#include "qapplication.h"

#include "qpainter.h"

#include "qmenubar.h"
#include "qtoolbar.h"
#include "qstatusbar.h"

#include "qtooltip.h"

/*! \class QMainWindow qmainwindow.h

  \brief The QMainWindow class provides a typical application window,
  with a menu bar, some tool bars and a status bar.

  \ingroup realwidgets
  \ingroup application

  \define QMainWindow::ToolBarDock

  In addition, you need the large central widget, which you supply and
  tell QMainWindow about using setCentralWidget(), and perhaps a few
  tool bars, which you can add using addToolBar().

  The central widget is not touched by QMainWindow.  QMainWindow
  manages its geometry, and that is all.  For example, the
  application/application.cpp exmple (an editor) sets a QMultiLineEdit
  to be the central widget.

  QMainWindow automatically detects the creation of a menu bar or
  status bar if you specify the QMainWindow as parent, or you can use
  the provided menuBar() and statusBar() functions.  menuBar() and
  statusBar() create a suitable widget if one doesn't exist, and
  updates the window's layout to make space.

  QMainWindow also provides a QToolTipGroup connected to the status
  bar.  toolTipGroup() provides access to the QToolTipGroup, but there
  is no way to set the tool tip group.

  By default, QMainWindow only allows toolbars above the central
  widget.  You can use setDockEnabled() to allow toolbars in other
  docks (a \e dock is a place where toolbars can stay).  Currently,
  only \c Top, \c Left, \c Right and \c Bottom are meaningful.

  Several functions let you change the appearance of a QMainWindow
  globally: setRightJustification() determines whether QMainWindow
  should ensure that the toolbars fill the available space,
  setUsesBigPixmaps() determines whether QToolButton (and other
  classes) should draw small or large pixmaps (see QIconSet for more
  about that).

  The current release of QMainWindow does not provide draggable
  toolbars.  This feature is planned for inclusion in one of the next
  releases.

  <img src=qmainwindow-m.gif> <img src=qmainwindow-w.gif>

  \sa QToolBar QStatusBar QMenuBar QToolTipGroup QDialog
*/

class QMainWindowPrivate {
public:
    struct ToolBar {
	ToolBar() : t(0), l(0), nl(FALSE) {}
	ToolBar( QToolBar * tb, const char * label, bool n=FALSE )
	    : t(tb), l(label), nl(n) {}
	QToolBar * t;
	QString l;
	bool nl;
    };
	
    typedef QList<ToolBar> ToolBarDock;

    QMainWindowPrivate()
	: top(0), left(0), right(0), bottom(0), tornOff(0), unmanaged(0),
	  mb(0), sb(0), ttg(0), mc(0), timer(0), tll(0), ubp( FALSE ),
	  justify( FALSE ), moving( 0 )
    {
	// nothing
    }

    ~QMainWindowPrivate()
    {
	if ( top ) {
	    top->setAutoDelete( TRUE );
	    delete top;
	}
	if ( left ) {
	    left->setAutoDelete( TRUE );
	    delete left;
	}
	if ( right ) {
	    right->setAutoDelete( TRUE );
	    delete right;
	}
	if ( bottom ) {
	    bottom->setAutoDelete( TRUE );
	    delete bottom;
	}
	if ( tornOff ) {
	    tornOff->setAutoDelete( TRUE );
	    delete tornOff;
	}
	if ( unmanaged ) {
	    unmanaged->setAutoDelete( TRUE );
	    delete unmanaged;
	}
    }

    ToolBarDock * top, * left, * right, * bottom, * tornOff, * unmanaged;

    QMenuBar * mb;
    QStatusBar * sb;
    QToolTipGroup * ttg;

    QWidget * mc;

    QTimer * timer;

    QBoxLayout * tll;

    bool ubp;
    bool justify;

    QToolBar * moving;
    QPoint pos;
    QPoint offset;
};


/*!  Constructs an empty main window. */

QMainWindow::QMainWindow( QWidget * parent, const char * name, WFlags f )
    : QWidget( parent, name, f )
{
    d = new QMainWindowPrivate;
    d->timer = new QTimer( this );
    connect( d->timer, SIGNAL(timeout()),
	     this, SLOT(setUpLayout()) );
}


/*! Destroys the object and frees any allocated resources.

*/

QMainWindow::~QMainWindow()
{
    delete d;
}


/*!  Sets this main window to use the menu bar \a newMenuBar.

  The old menu bar, if there was any, is deleted along with its
  contents.

  \sa menuBar()
*/

void QMainWindow::setMenuBar( QMenuBar * newMenuBar )
{
    if ( !newMenuBar )
	return;
    if ( d->mb )
	delete d->mb;
    d->mb = newMenuBar;
    d->mb->installEventFilter( this );
    triggerLayout();
}


/*!  Returns the menu bar for this window.  If there isn't any,
  menuBar() creates an empty menu bar on the fly.

  \sa statusBar()
*/

QMenuBar * QMainWindow::menuBar() const
{
    if ( d->mb )
	return d->mb;

    QObjectList * l
	= ((QObject*)this)->queryList( "QMenuBar", 0, FALSE, FALSE );
    QMenuBar * b;
    if ( l && l->count() )
	b = (QMenuBar *)l->first();
    else
	b = new QMenuBar( (QMainWindow *)this, "automatic menu bar" );
    delete l;
    ((QMainWindowPrivate*)d)->mb = b;
    d->mb->installEventFilter( this );
    ((QMainWindow *)this)->triggerLayout();
    return b;
}


/*!  Sets this main window to use the status bar \a newStatusBar.

  The old status bar, if there was any, is deleted along with its
  contents.

  \sa setMenuBar() statusBar()
*/

void QMainWindow::setStatusBar( QStatusBar * newStatusBar )
{
    if ( !newStatusBar || newStatusBar == d->sb )
	return;
    if ( d->sb )
	delete d->sb;
    d->sb = newStatusBar;
    connect( toolTipGroup(), SIGNAL(showTip(const char *)),
	     d->sb, SLOT(message(const char *)) );
    connect( toolTipGroup(), SIGNAL(removeTip()),
	     d->sb, SLOT(clear()) );
    d->sb->installEventFilter( this );
    triggerLayout();
}


/*!  Returns the status bar for this window.  If there isn't any,
  statusBar() creates an empty status bar on the fly, and if necessary
  a tool tip group too.

  \sa setMenuBar() statusBar() toolTipGroup()
*/

QStatusBar * QMainWindow::statusBar() const
{
    if ( d->sb )
	return d->sb;

    QObjectList * l
	= ((QObject*)this)->queryList( "QStatusBar", 0, FALSE, FALSE );
    QStatusBar * s;
    if ( l && l->count() )
	s = (QStatusBar *)l->first();
    else
	s = new QStatusBar( (QMainWindow *)this, "automatic status bar" );
    delete l;
    ((QMainWindow *)this)->setStatusBar( s );
    return s;
}


/*!  Sets this main window to use the tool tip group \a newToolTipGroup.

  The old tool tip group, if there was any, is deleted along with its
  contents.  All the tool tips connected to it lose the ability to
  display the group texts.

  \sa menuBar() toolTipGroup()
*/

void QMainWindow::setToolTipGroup( QToolTipGroup * newToolTipGroup )
{
    if ( !newToolTipGroup || newToolTipGroup == d->ttg )
	return;
    if ( d->ttg )
	delete d->ttg;
    d->ttg = newToolTipGroup;

    connect( toolTipGroup(), SIGNAL(showTip(const char *)),
	     statusBar(), SLOT(message(const char *)) );
    connect( toolTipGroup(), SIGNAL(removeTip()),
	     statusBar(), SLOT(clear()) );
    triggerLayout();
}


/*!  Returns the tool tip group for this window.  If there isn't any,
  menuBar() creates an empty tool tip group on the fly.

  \sa menuBar() statusBar()
*/

QToolTipGroup * QMainWindow::toolTipGroup() const
{
    if ( d->ttg )
	return d->ttg;

    QToolTipGroup * t = new QToolTipGroup( (QMainWindow*)this,
					   "automatic tool tip group" );
    ((QMainWindowPrivate*)d)->ttg = t;
    ((QMainWindow *)this)->triggerLayout();
    return t;
}


/*!  Sets \a dock to be available if \a enable is TRUE, and not
  available if \a enable is FALSE.

  The user can drag a toolbar to any enabled dock.
*/

void QMainWindow::setDockEnabled( ToolBarDock dock, bool enable )
{
    if ( enable ) {
	switch ( dock ) {
	case Top:
	    if ( !d->top )
		d->top = new QMainWindowPrivate::ToolBarDock();
	    break;
	case Left:
	    if ( !d->left )
		d->left = new QMainWindowPrivate::ToolBarDock();
	    break;
	case Right:
	    if ( !d->right )
		d->right = new QMainWindowPrivate::ToolBarDock();
	    break;
	case Bottom:
	    if ( !d->bottom )
		d->bottom = new QMainWindowPrivate::ToolBarDock();
	    break;
	case TornOff:
	    if ( !d->tornOff )
		d->tornOff = new QMainWindowPrivate::ToolBarDock();
	    break;
	case Unmanaged:
	    if ( !d->unmanaged )
		d->unmanaged = new QMainWindowPrivate::ToolBarDock();
	    break;
	}
    } else {
	warning( "oops! unimplemented, untested, and not quite thought out." );
    }
}


/*!  Returns TRUE if \a dock is enabled, or FALSE if it is not.

  \sa setDockEnabled()
*/

bool QMainWindow::isDockEnabled( ToolBarDock dock ) const
{
    switch ( dock ) {
    case Top:
	return d->top != 0;
    case Left:
	return d->left != 0;
    case Right:
	return d->right != 0;
    case Bottom:
	return d->bottom != 0;
    case TornOff:
	return d->tornOff != 0;
    case Unmanaged:
	return d->unmanaged != 0;
    }
    return FALSE; // for illegal values of dock
}


/*!  Adds \a toolbar to this the \a edge window of this window.

*/

void QMainWindow::addToolBar( QToolBar * toolBar, const char * label,
			      ToolBarDock edge, bool nl )
{
    if ( !toolBar )
	return;

    setDockEnabled( edge, TRUE );

    QMainWindowPrivate::ToolBarDock * dl = 0;
    if ( edge == Top ) {
	dl = d->top;
	toolBar->setOrientation( QToolBar::Horizontal );
	toolBar->installEventFilter( this );
    } else if ( edge == Left ) {
	dl = d->left;
	toolBar->setOrientation( QToolBar::Vertical );
	toolBar->installEventFilter( this );
    } else if ( edge == Bottom ) {
	dl = d->bottom;
	toolBar->setOrientation( QToolBar::Horizontal );
	toolBar->installEventFilter( this );
    } else if ( edge == Right ) {
	dl = d->right;
	toolBar->setOrientation( QToolBar::Vertical );
	toolBar->installEventFilter( this );
    } else if ( edge == TornOff ) {
	dl = d->tornOff;
    } else if ( edge == Unmanaged ) {
	dl = d->unmanaged;
    }

    if ( !dl )
	return;

    dl->append( new QMainWindowPrivate::ToolBar( toolBar, label, nl ) );
    triggerLayout();
}



static QMainWindowPrivate::ToolBar * takeToolBarFromDock( QToolBar * t,
							  QMainWindowPrivate::ToolBarDock *l )
{
    if ( !l || !l->count() )
	return 0;
    QMainWindowPrivate::ToolBar * ct = l->first();
    do {
	if ( ct->t == t ) {
	    l->take();
	    return ct;
	}
    } while( (ct=l->next()) != 0 );
    return 0;
}


/*!  Removes \a toolBar from this main window, if \a toolBar is
  non-null and known by this main window.
*/

void QMainWindow::removeToolBar( QToolBar * toolBar )
{
    if ( !toolBar )
	return;
    QMainWindowPrivate::ToolBar * ct;
    ct = takeToolBarFromDock( toolBar, d->top );
    if ( !ct )
	ct = takeToolBarFromDock( toolBar, d->left );
    if ( !ct )
	ct = takeToolBarFromDock( toolBar, d->right );
    if ( !ct )
	ct = takeToolBarFromDock( toolBar, d->bottom );
    if ( !ct )
	ct = takeToolBarFromDock( toolBar, d->tornOff );
    if ( !ct )
	ct = takeToolBarFromDock( toolBar, d->unmanaged );
    if ( ct ) {
	delete ct;
	triggerLayout();
    }
}


static void addToolBarToLayout( QMainWindowPrivate::ToolBarDock * dock,
				QBoxLayout * tl,
				QBoxLayout::Direction direction,
				QBoxLayout::Direction dockDirection,
				bool mayNeedDockLayout,
				bool justify,
				GUIStyle style )
{
    if ( !dock || dock->isEmpty() )
	return;

    bool moreThanOneRow = FALSE;
    bool anyToolBars = FALSE;

    dock->first();
    while ( dock->next() ) {
	if ( dock->current()->nl ) {
	    moreThanOneRow = TRUE;
	    break;
	}
    }

    QBoxLayout * dockLayout;
    if ( mayNeedDockLayout && moreThanOneRow ) {
	dockLayout = new QBoxLayout( dockDirection );
	tl->addLayout( dockLayout );
    } else {
	dockLayout = tl;
    }

    QBoxLayout * toolBarRowLayout = 0;
    QMainWindowPrivate::ToolBar * t = dock->first();
    do {
	if ( !toolBarRowLayout || t->nl ) {
	    if ( toolBarRowLayout ) {
		if ( !justify )
		    toolBarRowLayout->addStretch( 1 );
	    }
	    toolBarRowLayout = new QBoxLayout( direction );
	    dockLayout->addLayout( toolBarRowLayout, 0 );
	}
	if ( t->t->isVisible() && !t->t->testWFlags( WState_DoHide ) ) {
	    toolBarRowLayout->addWidget( t->t, 0 );
	    anyToolBars = TRUE;
	}
    } while ( (t=dock->next()) != 0 );

    if ( anyToolBars && style == MotifStyle )
	dockLayout->addSpacing( 2 );

    if ( toolBarRowLayout && (!justify || !anyToolBars) )
	toolBarRowLayout->addStretch( 1 );
}


/*!  Sets up the \link QBoxLayout geometry management \endlink of this
  window.  Called automatically when needed, so you should never need
  to call this.
*/

void QMainWindow::setUpLayout()
{
    d->timer->stop();
    delete d->tll;
    d->tll = new QBoxLayout( this, QBoxLayout::Down );
    if ( !d->mb ) {
	// slightly evil hack here.  reconsider this after 1.40.
	QObjectList * l
	    = ((QObject*)this)->queryList( "QMenuBar", 0, FALSE, FALSE );
	if ( l && l->count() )
	    d->mb = menuBar();
	delete l;
    }
    if ( d->mb && !d->mb->testWFlags( WState_DoHide ) )
	d->tll->setMenuBar( d->mb );
    if ( style() == WindowsStyle )
	d->tll->addSpacing( 1 );
    addToolBarToLayout( d->top, d->tll,
			QBoxLayout::LeftToRight, QBoxLayout::Down, FALSE,
			d->justify, style() );
    QBoxLayout * mwl = new QBoxLayout( QBoxLayout::LeftToRight );
    d->tll->addLayout( mwl, 1 );
    addToolBarToLayout( d->left, mwl,
			QBoxLayout::Down, QBoxLayout::LeftToRight, FALSE,
			d->justify, style() );
    if ( centralWidget() && !centralWidget()->testWFlags( WState_DoHide ) )
	mwl->addWidget( centralWidget(), 1 );
    else
	mwl->addStretch( 1 );
    addToolBarToLayout( d->right, mwl,
			QBoxLayout::Down, QBoxLayout::LeftToRight, FALSE,
			d->justify, style() );
    addToolBarToLayout( d->bottom, d->tll,
			QBoxLayout::LeftToRight, QBoxLayout::Up, TRUE,
			d->justify, style() );
    if ( !d->sb ) {
	// as above.
	QObjectList * l
	    = ((QObject*)this)->queryList( "QStatusBar", 0, FALSE, FALSE );
	if ( l && l->count() )
	    d->sb = statusBar();
	delete l;
    }
    if ( d->sb && !d->sb->testWFlags( WState_DoHide ) )
	d->tll->addWidget( d->sb, 0 );
    //debug( "act %d, %d", x(), y() );
    d->tll->activate();
}


/*!  \reimp */

void QMainWindow::show()
{
    setUpLayout();
    QWidget::show();
}


/*!  Sets the central widget for this window to \a w.  The centail
  widget is the one around which the toolbars etc. are arranged.
*/

void QMainWindow::setCentralWidget( QWidget * w )
{
    if ( d->mc )
	d->mc->removeEventFilter( this );
    d->mc = w;
    d->mc->installEventFilter( this );
    triggerLayout();
}


/*!  Returns a pointer to the main child of this main widget.  The
  main child is the big widget around which the tool bars are
  arranged.

  \sa setCentralWidget()
*/

QWidget * QMainWindow::centralWidget() const
{
    return d->mc;
}


/*! \reimp */

void QMainWindow::paintEvent( QPaintEvent * )
{
    //debug( "pe %d, %d", x(), y() );
    if ( style() == WindowsStyle ) {
	QPainter p( this );
	int y = menuBar()->height();
	p.setPen( colorGroup().dark() );
	p.drawLine( 0, y, width()-1, y );
    }
}


/*!
  Monitors events to ensure layout is updated.
*/

bool QMainWindow::eventFilter( QObject* o, QEvent *e )
{
    if ( e->type() == Event_Show || e->type() == Event_Hide ) {
	triggerLayout();
    } else if ( ( e->type() == Event_MouseButtonPress ||
		  e->type() == Event_MouseMove ||
		  e->type() == Event_MouseButtonRelease ) &&
		0 && // 1.4x
		o && ( d->moving || o->inherits( "QToolBar" ) ) ) {
	moveToolBar( d->moving ? d->moving : (QToolBar *)o, (QMouseEvent *)e );
	return TRUE;
    }
    return QWidget::eventFilter( o, e );
}

/*!
  Monitors events to ensure layout is updated.
*/

bool QMainWindow::event( QEvent * e )
{
    if ( e->type() == Event_ChildRemoved ) {
	QChildEvent * c = (QChildEvent *) e;
	if ( c->child() == 0 ||
	     c->child()->testWFlags( WType_TopLevel ) ) {
	    // nothing
	} else if ( c->child() == d->sb ) {
	    d->sb = 0;
	    triggerLayout();
	} else if ( c->child() == d->mb ) {
	    d->mb = 0;
	    triggerLayout();
	} else if ( c->child() == d->mc ) {
	    d->mc = 0;
	    triggerLayout();
	} else {
	    removeToolBar( (QToolBar *)(c->child()) );
	    triggerLayout();
	}
    }
    return QWidget::event( e );
}


/*!  Returns the state last set by setUsesBigPixmaps().  The initial
  state is FALSE.
  \sa setUsesBigPixmaps();
*/

bool QMainWindow::usesBigPixmaps() const
{
    return d->ubp;
}


/*!  Sets tool buttons in this main windows to use big pixmaps if \a
  enable is TRUE, and small pixmaps if \a enable is FALSE.

  The default is FALSE.

  Tool buttons and other interested widgets are responsible for
  reading the correct state on startup, and for connecting to this
  widget's pixmapSizeChanged() signal.

  \sa QToolButton::setUsesBigPixmap()
*/

void QMainWindow::setUsesBigPixmaps( bool enable )
{
    if ( d->ubp == enable )
	return;

    d->ubp = enable;
    emit pixmapSizeChanged( enable );
}


/*! \fn void QMainWindow::pixmapSizeChanged( bool )

  This signal is called whenever the setUsesBigPixmaps() is called
  with a value which is different from the current setting.  All
  relevant widgets must connect to this signal.
*/


/*!  Sets this main window to expand its toolbars to fill all
  available space if \a enable is TRUE, and to give the toolbars just
  the space they need if \a enable is FALSE.

  The default is FALSE.

  \sa rightJustification();
*/

void QMainWindow::setRightJustification( bool enable )
{
    if ( enable == d->justify )
	return;
    d->justify = enable;
    triggerLayout();
}


/*!  Returns TRUE if this main windows right-justifies its toolbars, and
  FALSE if it uses a ragged right edge.

  The default is to use a ragged right edge.

  ("Right edge" sometimes means "bottom edge".)

  \sa setRightJustification()
*/

bool QMainWindow::rightJustification() const
{
    return d->justify;
}


void QMainWindow::triggerLayout()
{
    if ( isVisibleToTLW() ) {
	setUpLayout();
	d->timer->stop();
    } else {
	d->timer->start( 0, TRUE );
    }
}


/*!  Handles mouse event \e e on behalf of tool bar \a t and does all
  the funky docking.
*/

void QMainWindow::moveToolBar( QToolBar * t , QMouseEvent * e )
{
    if ( e->type() == Event_MouseButtonPress ) {
	//debug( "saw press" );
	d->moving = 0;
	d->offset = e->pos();
	d->pos = QCursor::pos();
	return;
    } else if ( e->type() == Event_MouseButtonRelease ) {
	//debug( "saw release" );
	if ( d->moving ) {
	    qApp->removeEventFilter( this );
	    d->moving = 0;
	}
	return;
    }

    //debug( "saw move" );

    // with that out of the way, let's concentrate on the moves...

    // first, the threshold

    QPoint p( QCursor::pos() );
    if ( !d->moving &&
	 QABS( p.x() - d->pos.x() ) < 3 &&
	 QABS( p.y() - d->pos.y() ) < 3 )
	return;

    // okay.  it's a real move.

    //    debug( "move event to %d, %d", p.x(), p.y() );

    if ( !d->moving ) {
	d->moving = t;
	qApp->installEventFilter( this );
    }

    QPoint lp( mapFromGlobal( p ) );
    QMainWindowPrivate::ToolBarDock * dock = 0;
    // five possible cases: in each of the docs, and outside.
    if ( centralWidget()->geometry().contains( lp ) ||
	 !rect().contains( lp ) ) {
	// not a dock
	if ( t->parentWidget() ) {
	    t->recreate( 0, 0,
			 QPoint( p.x() - d->offset.x(),
				 p.y() - d->offset.y() ),
			 TRUE );
	    QApplication::syncX();
	    dock = d->tornOff;
	} else {
	    t->move( p.x() - d->offset.x(),
		     p.y() - d->offset.y() );
	}
    } else if ( lp.y() < centralWidget()->y() ) {
	//top dock
	dock = d->top;
    } else if ( lp.y() >= centralWidget()->y() + centralWidget()->height() ) {
	// bottom dock
	dock = d->bottom;
    } else if ( lp.x() < centralWidget()->x() ) {
	// bottom dock
	dock = d->left;
    } else if ( lp.x() >= centralWidget()->x() + centralWidget()->width() ) {
	// right dock
	dock = d->right;
    } else {
	fatal( "never to happen" );
    }

    if ( !dock )
	return;

    debug( "1" );
    // at this point dock points to the new dock
    QMainWindowPrivate::ToolBar * ct;
    ct = takeToolBarFromDock( t, d->top );
    if ( !ct )
	ct = takeToolBarFromDock( t, d->left );
    if ( !ct )
	ct = takeToolBarFromDock( t, d->right );
    if ( !ct )
	ct = takeToolBarFromDock( t, d->bottom );
    if ( dock != d->tornOff && !ct )
	ct = takeToolBarFromDock( t, d->tornOff );
    if ( dock == d->tornOff || ct == 0 )
	return;

    debug( "2" );
    QMainWindowPrivate::ToolBar * c = dock->first();
    QRect inLine;
    QRect betweenLines;
    int linestart = 0;
    while( c && ct ) {
	debug( "3 %p %p", c, ct );
	if ( c->nl ) {
	    if ( dock == d->top ) {
		betweenLines.setRect( 0, 0, width(),
				      c->t->y() + c->t->height()/4 );
	    } else if ( dock == d->bottom ) {
		betweenLines.setRect( 0, c->t->y() + c->t->height()/4,
				      width(), c->t->height()/2 );
	    } else if ( dock == d->left ) {
		betweenLines.setRect( 0, 0, c->t->x() + c->t->width()/4,
				      height() );
	    } else {
		betweenLines.setRect( c->t->x() + 3*c->t->width()/4, 0,
				      c->t->width()/2, height() );
	    }
	    linestart = dock->at();
	}
	if ( dock == d->top || dock == d->bottom ) {
	    inLine.setRect( c->t->x()-c->t->height()/4, c->t->y(),
			    c->t->height()/2, c->t->height() );
	} else {
	    inLine.setRect( c->t->x(), c->t->y() - c->t->width()/4,
			    c->t->width(), c->t->width()/2 );
	}
	if ( inLine.contains( lp ) ) {
	    // ct goes in just before c, and takes over nl
	    dock->insert( dock->at(), ct );
	    if ( t->parentWidget() != this )
		t->recreate( this, 0, QPoint( 0, -t->height() ), TRUE );
	    t->setOrientation( (dock == d->top || dock == d->bottom )
			       ? QToolBar::Horizontal : QToolBar::Vertical );
	    ct->nl = c->nl;
	    c->nl = FALSE;
	    ct = 0;
	    triggerLayout();
	} else {
	    QMainWindowPrivate::ToolBar * c2 = dock->next();
	    if ( c2 == 0 || c2->nl ) {
		// about to do the next line, so check whether c
		// should go in above this line
		if ( betweenLines.contains( lp ) ) {
		    dock->insert( linestart, ct );
		    if ( t->parentWidget() != this )
			t->recreate( this, 0, QPoint( 0, -t->height() ),
				     TRUE );
		    t->setOrientation( (dock == d->top || dock == d->bottom )
				       ? QToolBar::Horizontal
				       : QToolBar::Vertical );
		    ct->nl = TRUE;
		    ct = 0;
			triggerLayout();
		} else {
		    // perhaps at the end of this line?  let's see
		    if ( dock == d->top || dock == d->bottom )
			inLine.setRect( c->t->x() + c->t->width(),
					c->t->y(),
					width() - c->t->x() - c->t->width(),
					c->t->height() );
		    else
			inLine.setRect( c->t->x(),
					c->t->y() + c->t->height(),
					c->t->width(),
					height() - c->t->y() - c->t->height());
		    if ( inLine.contains( lp ) ) {
			dock->insert( dock->at(), ct );
			if ( t->parentWidget() != this )
			    t->recreate( this, 0, QPoint( 0, -t->height() ),
					 TRUE );
			t->setOrientation( (dock == d->top ||
					    dock == d->bottom )
					   ? QToolBar::Horizontal
					   : QToolBar::Vertical );
			ct->nl = FALSE;
			ct = 0;
			triggerLayout();
		    }
		}
	    }
	    c = c2;
	}
    }
    debug( "4" );
    // okay, is it at the very end?
    if ( ct ) {
	debug( "4a" );
	dock->append( ct );
	if ( t->parentWidget() != this )
	    t->recreate( this, 0, QPoint( 0, -t->height() ), TRUE );
	t->setOrientation( (dock == d->top || dock == d->bottom )
			   ? QToolBar::Horizontal : QToolBar::Vertical );
	ct->nl = TRUE;
	triggerLayout();
    }
}
