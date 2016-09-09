/****************************************************************************
** $Id: qtabdialog.cpp,v 2.42.2.3 1999/02/19 11:38:42 hanord Exp $
**
** Implementation of QTabDialog class
**
** Created : 960825
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

#include "qtabdialog.h"

#include "qobjectlist.h"
#include "qobjectdict.h"
#include "qtabbar.h"
#include "qpushbutton.h"
#include "qpainter.h"
#include "qpixmap.h"
#include "qapplication.h"
#include "qwidgetstack.h"
#include "qlayout.h"

/*!
  \class QTabDialog qtabdialog.h

  \brief The QTabDialog class provides a stack of tabbed widgets.

  \ingroup dialogs

  A tabbed dialog is one in which several "pages" are available, and
  the user selects which page to see and use by clicking on its tab,
  or by pressing the indicated Alt-(letter) key combination.

  QTabDialog does not provide more than one row of tabs, and does not
  provide tabs along the sides or bottom of the pages.  It also does
  not offer any way to find out which page is currently visible or to
  set the visible page.

  QTabDialog provides an OK button and optionally Apply, Cancel and
  Defaults buttons.

  The normal way to use QTabDialog is to do the following in the
  constructor: <ol> <li> Create a QTabDialog, <li> create a QWidget
  for each of the pages in the tab dialog, insert children into it,
  set up geometry management for it, and use addTab() to set up a tab
  and keyboard accelerator for it, <li> set up the buttons for the tab
  dialog (Apply, Cancel and so on), and finally <li> connect to the
  signals and slots. </ol>

  The pref.cpp example does all this.

  If you don't call addTab(), the page you have created will not be
  visible.  Please don't confuse the object name you supply to the
  QWidget constructor and the tab label you supply to addTab():
  addTab() takes a name which indicates an accelerator and is
  meaningful and descriptive to the user, while the widget name is
  used primarily for debugging.

  Almost all applications have to connect the applyButtonPressed()
  signal to something.  applyButtonPressed() is emitted when either OK
  or Apply is clicked, and your slot must copy the dialog's state into
  the application.

  There are also several other signals which may be useful. <ul> <li>
  cancelButtonPressed() is emitted when the user clicks Cancel.  <li>
  defaultButtonPressed() is emitted when the user clicks Defaults; the
  slot it is connected to should reset the state of the dialog to the
  application defaults.  <li> aboutToShow() is emitted at the start of
  show(); if there is any chance that the state of the application may
  change between the creation of the tab dialog and the time it show()
  is called, you must connect this signal to a slot which resets the
  state of the dialog. <li> selected() is emitted when the user
  selects some page. </ul>

  Each tab is either enabled or disabled at any given time.  If a tab
  is enabled, the tab text is drawn in black and the user can select
  that tab.  If it is disabled, the tab is drawn in a different way
  and the user can not select that tab.  Note that even though a tab
  is disabled, the page can still be visible, for example if all of
  the tabs happen to be disabled.

  While tab dialogs can be a very good way to split up a complex
  dialog, it's also very easy to make a royal mess out of a tab
  dialog.  Here is some advice.  For more, see e.g. the <a
  href="http://www.uie.com/tabbed.html">UIE web page on tab
  dialogs.</a>

  <ol><li> Make sure that each page forms a logical whole which is
  adequately described by the label on the tab.

  If two related functions are on different pages, users will often
  not find one of the functions, or will spend far too long searching
  for it.

  <li> Do not join several independent dialogs into one tab dialog.
  Several aspects of one complex dialog is acceptable (such as the
  various aspects of "preferences") but a tab dialog is no substitute
  for a pop-up menu leading to several smaller dialogs.

  The OK button (and the other buttons) apply to the \e entire dialog.
  If the tab dialog is really several independent smaller dialogs,
  users often press Cancel to cancel just the changes he/she has made
  on the current page: Many users will treat that page as independent
  of the other pages.

  <li> Do not use tab dialogs for frequent operations.  The tab dialog
  is probably the most complex widget in common use at the moment, and
  subjecting the user to this complexity during his/her normal use of
  your application is most often a bad idea.

  The tab dialog is good for complex operations which have to be
  performed seldom, like Preferences dialogs.  Not for common
  operations, like setting left/right alignment in a word processor.
  (Often, these common operations are actually independent dialogs and
  should be treated as such.)

  The tab dialog is not a navigational aid, it is an organizational
  aid.  It is a good way to organize aspects of a complex operation
  (such as setting up caching and proxies in a web browser), but a bad
  way to navigate towards a simple operation (such as emptying the
  cache in a web browser - emptying the cache is \e not part of
  setting up the cache, it is a separate and independent operation).

  <li> The changes should take effect when the user presses Apply or
  OK.  Not before.

  Providing Apply, Cancel or OK buttons on the individual pages is
  likely to weaken the users' mental model of how tab dialogs work.
  If you think a page needs its own buttons, consider making it a
  separate dialog.

  <li> There should be no implicit ordering of the pages.  If there
  is, it is probably better to use a wizard dialog.

  If some of the pages seem to be ordered and others not, perhaps they
  ought not to be joined in a tab dialog.

  </ol>

  Most of the functionality in QTabDialog is provided by a QTabBar (at
  the top, providing the tabs) and a QWidgetStack (most of the area,
  organizing the individual pages).

  <img src=qtabdlg-m.gif> <img src=qtabdlg-w.gif>

  \sa QDialog
*/

/*! \fn void QTabDialog::selected( const char * tabLabel );

  This signal is emitted whenever a tab is selected (raised),
  including during the first show().

  \sa raise()
*/


// add comments about delete, ok and apply

struct QTabPrivate
{
    QTabPrivate()
	: tabs(0), stack(0),
	  ok(0), cb(0), db(0), ab(0),
	  tll(0) {}

    QTabBar * tabs;
    QWidgetStack * stack;

    QPushButton * ok;
    QPushButton * cb;
    QPushButton * db;
    QPushButton * ab;

    QBoxLayout * tll;
};

/*!
  Constructs a QTabDialog with only an Ok button.
*/

QTabDialog::QTabDialog( QWidget *parent, const char *name, bool modal,
			WFlags f )
    : QDialog( parent, name, modal, f )
{
    d = new QTabPrivate;
    CHECK_PTR( d );

    d->stack = new QWidgetStack( this, "tab pages" );
    setTabBar( new QTabBar( this, "tab control" ) );

    d->ok = new QPushButton( this, "ok" );
    CHECK_PTR( d->ok );
    d->ok->setText( "OK" );
    d->ok->setDefault( TRUE );
    connect( d->ok, SIGNAL(clicked()),
	     this, SIGNAL(applyButtonPressed()) );
    connect( d->ok, SIGNAL(clicked()),
	     this, SLOT(accept()) );
}


/*!
  Destroys the tab dialog.
*/

QTabDialog::~QTabDialog()
{
    delete d;
}


/*!
  Sets the font for the tabs to \e font.

  If the widget is visible, the display is updated with the new font
  immediately.  There may be some geometry changes, depending on the
  size of the old and new fonts.
*/

void QTabDialog::setFont( const QFont & font )
{
    QDialog::setFont( font );
    setSizes();
}


/*!
  \fn void QTabDialog::applyButtonPressed();

  This signal is emitted when the Apply or OK buttons are clicked.

  It should be connected to a slot (or several slots) which change the
  application's state according to the state of the dialog.

  \sa cancelButtonPressed() defaultButtonPressed() setApplyButton()
*/


/*!
  Returns TRUE if the tab dialog has a Defaults button, FALSE if not.

  \sa setApplyButton() applyButtonPressed() hasCancelButton()
  hasDefaultButton()
*/

bool QTabDialog::hasDefaultButton() const
{
     return d->db != 0;
}


/*!
  \fn void QTabDialog::cancelButtonPressed();

  This signal is emitted when the Cancel button is clicked.  It is
  automatically connected to QDialog::reject(), which will hide the
  dialog.

  The Cancel button should not change the application's state in any
  way, so generally you should not need to connect it to any slot.

  \sa applyButtonPressed() defaultButtonPressed() setCancelButton()
*/


/*!
  Returns TRUE if the tab dialog has a Cancel button, FALSE if not.

  \sa setCancelButton() cancelButtonPressed() hasDefaultButton()
  hasApplyButton()
*/

bool QTabDialog::hasCancelButton() const
{
     return d->cb != 0;
}


/*!
  \fn void QTabDialog::defaultButtonPressed();

  This signal is emitted when the Defaults button is pressed.  It
  should reset the dialog (but not the application) to the "factory
  defaults."

  The application's state should not be changed until the user clicks
  Apply or OK.

  \sa applyButtonPressed() cancelButtonPressed() setDefaultButton()
*/


/*!
  Returns TRUE if the tab dialog has an Apply button, FALSE if not.

  \sa setDefaultsButton() defaultButtonPressed() hasApplyButton()
  hasCancelButton()
*/

bool QTabDialog::hasApplyButton() const
{
    return d->ab != 0;
}


/*!
  Returns TRUE if the tab dialog has an OK button, FALSE if not.

  \sa setDefaultsButton() defaultButtonPressed() hasOkButton()
  hasCancelButton()
*/

bool QTabDialog::hasOkButton() const
{
    return d->ok != 0;
}


/*!
  \fn void QTabDialog::aboutToShow()

  This signal is emitted by show() when it's time to set the state of
  the dialog's contents.  The dialog should reflect the current state
  of the application when if appears; if there is any chance that the
  state of the application can change between the time you call
  QTabDialog::QTabDialog() and QTabDialog::show(), you should set the
  dialog's state in a slot and connect this signal to it.

  This applies mainly to QTabDialog objects that are kept around
  hidden rather than being created, show()n and deleted afterwards.

  \sa applyButtonPressed(), show(), cancelButtonPressed()
*/


/*!
  Shows the tab view and its children.  Reimplemented in order to
  delay show()'ing of every page except the initially visible one, and
  in order to emit the aboutToShow() signal.

  \sa hide(), aboutToShow()
*/

void QTabDialog::show()
{
    if ( topLevelWidget() == this )
	d->tabs->setFocus();
    emit aboutToShow();
    setSizes();
    setUpLayout();
    QDialog::show();
}


/*!
  Ensure that the selected tab's page is visible and appropriately sized.
*/

void QTabDialog::showTab( int i )
{
    if ( d->stack->widget( i ) ) {
	d->stack->raiseWidget( i );
	emit selected( d->tabs->tab( i )->label );
    }
}


/*!
  Add another tab and page to the tab view.

  The tab will be labelled \a label and \a child constitutes the new
  page.  Note the difference between the widget name (which you supply
  to widget constructors and to e.g. setTabEnabled()) and the tab
  label: The name is internal to the program and invariant, while the
  label is shown on screen and may vary according to e.g. language.

  \a label is written in the QButton style, where &P makes Qt create
  an accelerator key on Alt-P for this page.  For example:

  \code
    td->addTab( graphicsPane, "&Graphics" );
    td->addTab( soundPane, "&Sound" );
  \endcode

  If the user presses Alt-S the sound page of the tab dialog is shown,
  if the user presses Alt-P the graphics page is shown.

  If you call addTab() after show(), the screen will flicker and the
  user will be confused.
*/

void QTabDialog::addTab( QWidget * child, const char * label )
{
    QTab * t = new QTab();
    CHECK_PTR( t );
    t->label = label;
    addTab( child, t );
}

/*!
  This is a lower-level method for adding tabs, similar to the other
  addTab() method.  It is useful if you are using setTabBar() to set a
  QTabBar subclass with an overridden QTabBar::paint() routine for a
  subclass of QTab.
*/
void QTabDialog::addTab( QWidget * child, QTab* tab )
{
    tab->enabled = TRUE;
    int id = d->tabs->addTab( tab );
    d->stack->addWidget( child, id );
    d->tabs->setMinimumSize( d->tabs->sizeHint() );
}

/*!
  Replaces the QTabBar heading the dialog by the given tab bar.
  Note that this must be called \e before any tabs have been added,
  or the behavior is undefined.
  \sa tabBar()
*/
void QTabDialog::setTabBar( QTabBar* tb )
{
    delete d->tabs;
    d->tabs = tb;
    connect( d->tabs, SIGNAL(selected(int)),
	     this,    SLOT(showTab(int)) );
    d->tabs->setMinimumSize( d->tabs->sizeHint() );
    setUpLayout();
}

/*!
  Returns the currently set QTabBar.
  \sa setTabBar()
*/
QTabBar* QTabDialog::tabBar() const
{
    return d->tabs;
}

/*!  Ensures that \a w is shown.  This is useful mainly for accelerators.

  \warning Used carelessly, this function can easily surprise or
  confuse the user.

  \sa QTabBar::setCurrentTab()
*/

void QTabDialog::showPage( QWidget * w )
{
    int i = d->stack->id( w );
    if ( i >= 0 ) {
	d->stack->raiseWidget( w );
	d->tabs->setCurrentTab( i );
    }
}


/*!
  Returns TRUE if the page with object name \a name is enabled, and
  false if it is disabled.

  If \a name is 0 or not the name of any of the pages, isTabEnabled()
  returns FALSE.

  \sa setTabEnabled(), QWidget::isEnabled()
*/

bool QTabDialog::isTabEnabled( const char *name ) const
{
    if ( !name || !*name )
	return FALSE;

    QObjectList * l
	= ((QTabDialog *)this)->queryList( "QWidget", name, FALSE, TRUE );
    bool r = FALSE;
    if ( l && l->first() ) {
	QWidget * w;
	while( l->current() ) {
	    while( l->current() && !l->current()->isWidgetType() )
		l->next();
	    w = (QWidget *)(l->current());
	    if ( w && d->stack->id(w) ) {
		r = w->isEnabled();
		delete l;
		return r;
	    }
	    l->next();
	}
    }
    delete l;
    return r;
}


/*!
  Finds the page with object name \a name, enables/disables it
  according to the value of \a enable, and redraws the page's tab
  appropriately.

  QTabDialog uses QWidget::setEnabled() internally, rather than keep a
  separate flag.

  Note that even a disabled tab/page may be visible.  If the page is
  visible already, QTabDialog will not hide it, and if all the pages
  are disabled, QTabDialog will show one of them.

  The object name is used (rather than the tab label) because the tab
  text may not be invariant in multi-language applications.

  \sa isTabEnabled(), QWidget::setEnabled()
*/

void QTabDialog::setTabEnabled( const char * name, bool enable )
{
    if ( !name || !*name )
	return;
    QObjectList * l
	= ((QTabDialog *)this)->queryList( "QWidget", name, FALSE, TRUE );
    if ( l && l->first() ) {
	QWidget * w;
	while( l->current() ) {
	    while( l->current() && !l->current()->isWidgetType() )
		l->next();
	    w = (QWidget *)(l->current());
	    if ( w ) {
		int id = d->stack->id( w );
		if ( id ) {
		    w->setEnabled( enable );
		    d->tabs->setTabEnabled( id, enable );
		}
		delete l;
		return;
	    }
	    l->next();
	}
    }
    delete l;
}


/*!
  Add an Apply button to the dialog.  The button's text is set to \e
  text (and defaults to "Apply").

  The Apply button should apply the current settings in the dialog box
  to the application, while keeping the dialog visible.

  When Apply is clicked, the applyButtonPressed() signal is emitted.

  \sa setCancelButton() setDefaultButton() applyButtonPressed()
*/

void QTabDialog::setApplyButton( const char * text )
{
    if ( !text && d->ab ) {
	delete d->ab;
	d->ab = 0;
	setSizes();
    } else {
	if ( !d->ab ) {
	    d->ab = new QPushButton( this, "apply settings" );
	    connect( d->ab, SIGNAL(clicked()),
		     this, SIGNAL(applyButtonPressed()) );
	    setUpLayout();
	}
	d->ab->setText( text );
	setSizes();
	d->ab->show();
    }
}


/*!
  Add a Defaults button to the dialog.  The button's text is set to \e
  text (and defaults to "Defaults").

  The Defaults button should set the dialog (but not the application)
  back to the application defaults.

  When Defaults is clicked, the defaultButtonPressed() signal is emitted.

  \sa setApplyButton() setCancelButton() defaultButtonPressed()
*/

void QTabDialog::setDefaultButton( const char * text )
{
    if ( !text ) {
	delete d->db;
	d->db = 0;
	setSizes();
    } else {
	if ( !d->db ) {
	    d->db = new QPushButton( this, "back to default" );
	    connect( d->db, SIGNAL(clicked()),
		     this, SIGNAL(defaultButtonPressed()) );
	    setUpLayout();
	}
	d->db->setText( text );
	setSizes();
	d->db->show();
    }
}


/*!
  Add a Cancel button to the dialog.  The button's text is set to \e
  text (and defaults to "Cancel").

  The cancel button should always return the application to the state
  it was in before the tab view popped up, or if the user has clicked
  Apply, back the the state immediately after the last Apply.

  When Cancel is clicked, the cancelButtonPressed() signal is emitted.
  The dialog is closed at the same time.

  \sa setApplyButton setDefaultButton() cancelButtonPressed()
*/

void QTabDialog::setCancelButton( const char * text )
{
    if ( !text ) {
	delete d->cb;
	d->cb = 0;
	setSizes();
    } else {
	if ( !d->cb ) {
	    d->cb = new QPushButton( this, "cancel dialog" );
	    connect( d->cb, SIGNAL(clicked()),
		     this, SIGNAL(cancelButtonPressed()) );
	    connect( d->cb, SIGNAL(clicked()),
		     this, SLOT(reject()) );
	    setUpLayout();
	}
	d->cb->setText( text );
	setSizes();
	d->cb->show();
    }
}


/*!  Sets up the layout manager for the tab dialog.

  \sa setSizes() setApplyButton() setCancelButton() setDefaultButton()
*/

void QTabDialog::setUpLayout()
{
    // the next four are probably the same, really?
    const int topMargin = 6;
    const int leftMargin = 6;
    const int rightMargin = 6;
    const int bottomMargin = 6;
    const int betweenButtonsMargin = 7;
    const int aboveButtonsMargin = 8;

    delete d->tll;
    d->tll = new QBoxLayout( this, QBoxLayout::Down );

    // top margin
    d->tll->addSpacing( topMargin );

    // tab bar
    QBoxLayout * tmp = new QBoxLayout( QBoxLayout::LeftToRight );
    d->tll->addLayout( tmp, 0 );
    tmp->addSpacing( leftMargin );
    tmp->addWidget( d->tabs, 0 );
    tmp->addStretch( 1 );
    tmp->addSpacing( rightMargin + 2 );

    // widget stack.
    tmp = new QBoxLayout( QBoxLayout::LeftToRight );
    d->tll->addLayout( tmp, 1 );
    tmp->addSpacing( leftMargin + 1 );
    tmp->addWidget( d->stack, 1 );
    tmp->addSpacing( rightMargin + 2 );

    d->tll->addSpacing( aboveButtonsMargin + 2 );
    QBoxLayout * buttonRow = new QBoxLayout( QBoxLayout::RightToLeft );
    d->tll->addLayout( buttonRow, 0 );
    d->tll->addSpacing( bottomMargin );

    buttonRow->addSpacing( rightMargin );
    if ( d->cb ) {
	buttonRow->addWidget( d->cb, 0 );
	buttonRow->addSpacing( betweenButtonsMargin );
    }

    if ( d->ab ) {
	buttonRow->addWidget( d->ab, 0 );
	buttonRow->addSpacing( betweenButtonsMargin );
    }

    if ( d->db ) {
	buttonRow->addWidget( d->db, 0 );
	buttonRow->addSpacing( betweenButtonsMargin );
    }

    if ( d->ok ) {
	buttonRow->addWidget( d->ok, 0 );
	buttonRow->addSpacing( betweenButtonsMargin );
    }

    // add one custom widget here
    buttonRow->addStretch( 1 );
    // add another custom widget here

    d->tll->activate();
}


/*!  Sets up the minimum and maximum sizes for each child widget.

  \sa setUpLayout() setFont()
*/

void QTabDialog::setSizes()
{
    // compute largest button size
    QSize s( 0, 0 );
    int bw = s.width();
    int bh = s.height();

    if ( d->ok ) {
	s = d->ok->sizeHint();
	if ( s.width() > bw )
	    bw = s.width();
	if ( s.height() > bh )
	    bh = s.height();
    }

    if ( d->ab ) {
	s = d->ab->sizeHint();
	if ( s.width() > bw )
	    bw = s.width();
	if ( s.height() > bh )
	    bh = s.height();
    }

    if ( d->db ) {
	s = d->db->sizeHint();
	if ( s.width() > bw )
	    bw = s.width();
	if ( s.height() > bh )
	    bh = s.height();
    }

    if ( d->cb ) {
	s = d->cb->sizeHint();
	if ( s.width() > bw )
	    bw = s.width();
	if ( s.height() > bh )
	    bh = s.height();
    }

    if ( style() == WindowsStyle && bw < 75 )
	bw = 75;

    // and set all the buttons to that size
    if ( d->ok )
	d->ok->setFixedSize( bw, bh );
    if ( d->ab )
	d->ab->setFixedSize( bw, bh );
    if ( d->db )
	d->db->setFixedSize( bw, bh );
    if ( d->cb )
	d->cb->setFixedSize( bw, bh );

    // fiddle the tab chain so the buttons are in their natural order
    QWidget * w = d->ok;

    if ( d->db ) {
	if ( w )
	    setTabOrder( w, d->db );
	w = d->db;
    }
    if ( d->ab ) {
	if ( w )
	    setTabOrder( w, d->ab );
	w = d->ab;
    }
    if ( d->cb ) {
	if ( w )
	    setTabOrder( w, d->cb );
	w = d->cb;
    }
    setTabOrder( w, d->tabs );
}




/*!
  Handles resize events for the tab dialog.
*/

void QTabDialog::resizeEvent( QResizeEvent * e )
{
    QDialog::resizeEvent( e );
}


/*!
  Handles paint events for the tabbed dialog
*/

void QTabDialog::paintEvent( QPaintEvent * )
{
    if ( !d->tabs )
	return;

    QPainter p;
    p.begin( this );

    QRect s( d->stack->geometry() );

    QCOORD t = s.top() - 1;
    QCOORD b = s.bottom() + 2;
    QCOORD r = s.right() + 2;
    QCOORD l = s.left() - 1;

    p.setPen( colorGroup().light() );
    // note - this line overlaps the bottom line drawn by QTabBar
    p.drawLine( l, t, r - 1, t );
    p.drawLine( l, t + 1, l, b );
    p.setPen( black );
    p.drawLine( r, b, l,b );
    p.drawLine( r, b-1, r, t );
    p.setPen( colorGroup().dark() );
    p.drawLine( l+1, b-1, r-1, b-1 );
    p.drawLine( r-1, b-2, r-1, t+1 );

    p.end();
}


QRect QTabDialog::childRect() const
{
    return d->stack->geometry();
}


/*!
  Set the OK button's text to \a text (which defaults to "OK").

  When the OK button is clicked, the applyButtonPressed() signal is emitted,
  and the current settings in the dialog box should be applied to
  the application. Then the dialog closes.

  \sa setCancelButton() setDefaultButton() applyButtonPressed()
*/

void QTabDialog::setOkButton( const char * text )
{
    if ( !text ) {
	delete d->ok;
	d->ok = 0;
	setSizes();
    } else {
	if ( !d->ok ) {
	    d->ok = new QPushButton( this, "ok" );
	    connect( d->ok, SIGNAL(clicked()),
		     this, SIGNAL(applyButtonPressed()) );
	    setUpLayout();
	}
	d->ok->setText( text );
	setSizes();
	d->ok->show();
    }
}

/*!
  Old version of setOkButton(), provided for backward compatibility.
 */
void QTabDialog::setOKButton( const char * text )
{
    setOkButton( text );
}


/*!  Returns the text in the tab for page \a w.
*/

const char * QTabDialog::tabLabel( QWidget * w )
{
    QTab * t = d->tabs->tab( d->stack->id( w ) );
    return t ? ((const char *)t->label) : 0;
}	


/*!  Reimplemented to hndle a change of GUI style while on-screen.
*/

void QTabDialog::styleChange( GUIStyle s )
{
    QDialog::styleChange( s );
    setSizes();
}
