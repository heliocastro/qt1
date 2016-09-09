/****************************************************************************
** $Id: qprogressdialog.cpp,v 2.28.2.1 1998/08/18 19:51:39 warwick Exp $
**
** Implementation of QProgressDialog class
**
** Created : 970521
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

#include "qprogressdialog.h"
#include "qaccel.h"
#include "qpainter.h"
#include "qdrawutil.h"
#include "qdatetime.h"
#include "qapplication.h"

// If the operation is expected to take this long (as predicted by
// progress time), show the progress dialog.
static const int defaultShowTime    = 4000;
// Wait at least this long before attempting to make a prediction.
static const int minWaitTime = 50;

// Various layout values
static const int margin_lr   = 10;
static const int margin_tb   = 10;
static const int spacing     = 4;


struct QProgressData
{
    QProgressData( QProgressDialog* that, QWidget* parent,
		   const char* labelText,
		   int totalSteps ) :
	creator( parent ),
	label( new QLabel(labelText,that,"label") ),
	cancel( 0 ),
	bar( new QProgressBar(totalSteps,that,"bar") ),
	shown_once( FALSE ),
	cancellation_flag( FALSE ),
	showTime( defaultShowTime )
    {
	label->setAlignment( that->style() != WindowsStyle ?
			     AlignCenter : AlignLeft|AlignVCenter );
    }

    QWidget	 *creator;
    QLabel	 *label;
    QPushButton	 *cancel;
    QProgressBar *bar;
    bool	  shown_once;
    bool	  cancellation_flag;
    QTime	  starttime;
    QCursor	  parentCursor;
    int		  showTime;
};


/*!
  \class QProgressDialog qprogressdialog.h
  \brief Provides feedback on the progress of a slow operation.
  \ingroup dialogs
  \ingroup realwidgets

  A progress dialog is used to give the user an indication of how long an
  operation is going to take to perform, and to reassure them that the
  application has not frozen.

  A potential problem with progress dialogs is that it is difficult to know
  when to use them, as operations take different amounts of time on different
  computer hardware.  QProgressDialog offers a solution to this problem:
  it estimates the time the operation will take (based on time for
  steps), and only shows itself if that estimate is beyond 3 seconds.

  Example:
  \code
    QProgressDialog progress( "Copying files...", "Abort Copy", numFiles, this );
    for (int i=0; i<numFiles; i++) {
	progress.setProgress( i );
	if ( progress.wasCancelled() )
	    break;
	... // copy one file
    }
    progress.setProgress( numFiles );
  \endcode

  <img src=qprogdlg-m.gif> <img src=qprogdlg-w.gif>
  
  \sa QDialog QProgressBar
  <a href="guibooks.html#fowler">GUI Design Handbook: Progress Indicator</a>
*/


/*!
  Returns the QLabel currently being displayed above the progress bar.
  Note that this QLabel remains owned by the QProgressDialog.

  \sa setLabel()
*/
QLabel *QProgressDialog::label() const
{
    return d->label;
}

/*!
  Returns the QProgressBar currently being used to displayed progress.
  Note that this QProgressBar remains owned by the QProgressDialog.

  \sa setBar()
*/
QProgressBar *QProgressDialog::bar() const
{
    return d->bar;
}


/*!
  Constructs a progress dialog.

  Default settings:
  <ul>
    <li>The label text is empty.
    <li>The cancel button text is "Cancel".
    <li>The total number of steps is 100.
  </ul>

  \e parent, \e name, \e modal, and \e f are sent to the
  QSemiModal::QSemiModal() constructor. Note that if \e modal is FALSE
  (the default), you will need to have an event loop proceeding for
  any redrawing of the dialog to occur.  If it is TRUE, the dialog
  ensures events are processed when needed.

  \sa setLabelText(), setLabel(), setCancelButtonText(), setCancelButton(),
  setTotalSteps()
*/

QProgressDialog::QProgressDialog( QWidget *creator, const char *name,
				  bool modal, WFlags f )
    : QSemiModal( 0, name, modal, f)
{
    init( creator, "", "Cancel", 100 );
}

/*!
  Constructs a progress dialog.

  \arg \e labelText is text telling the user what is progressing.
  \arg \e cancelButtonText is the text on the cancel button,
	    or 0 if no cancel button is to be shown.
  \arg \e totalSteps is the total number of steps in the operation of which
    this progress dialog shows the progress.  For example, if the operation
    is to examine 50 files, this value would be 50, then before examining
    the first file, call setProgress(0), and after examining the last file
    call setProgress(50).
  \arg \e name, \e modal, and \e f are sent to the
    QSemiModal::QSemiModal() constructor. Note that if \e modal is FALSE (the
    default), you will need to have an event loop proceeding for any
    redrawing of the dialog to occur.  If it is TRUE, the dialog ensures
    events are processed when needed.

  \sa setLabelText(), setLabel(), setCancelButtonText(), setCancelButton(),
  setTotalSteps()
*/

QProgressDialog::QProgressDialog( const char *labelText,
				  const char *cancelButtonText,
				  int totalSteps,
				  QWidget *creator, const char *name,
				  bool modal, WFlags f )
    : QSemiModal( 0, name, modal, f)
{
    init( creator, labelText, cancelButtonText, totalSteps );
}


/*!
  Destroys the progress dialog.
*/

QProgressDialog::~QProgressDialog()
{
    delete d;
}

void QProgressDialog::init( QWidget *creator,
			    const char* lbl, const char* canc,
			    int totstps)
{
    d = new QProgressData(this, creator, lbl, totstps);
    setCancelButtonText( canc );
    connect( this, SIGNAL(cancelled()), this, SLOT(cancel()) );
    layout();
}

/*!
  \fn void QProgressDialog::cancelled()

  This signal is emitted when the cancel button is clicked.
  It is connected to the cancel() slot by default.

  \sa wasCancelled()
*/


/*!
  Sets the label. The progress dialog resizes to fit.
  The label becomes owned by the
  progress dialog and will be deleted when necessary,
  so do not pass the address of an object on the stack.
  \sa setLabelText()
*/

void QProgressDialog::setLabel( QLabel *label )
{
    delete d->label;
    d->label = label;
    if (label) {
	if ( label->parentWidget() == this ) {
	    label->hide(); // until we resize
	} else {
	    label->recreate( this, 0, QPoint(0,0), FALSE );
	}
    }
    resize(sizeHint());
    if (label)
	label->show();
}


/*!
  Sets the label text. The progress dialog resizes to fit.
  \sa setLabel()
*/

void QProgressDialog::setLabelText( const char *text )
{
    if ( label() ) {
	label()->setText( text );
	resize(sizeHint());
    }
}


/*!
  Sets the cancellation button.  The button becomes owned by the
  progress dialog and will be deleted when necessary,
  so do not pass the address of an object on the stack.
  \sa setCancelButtonText()
*/

void QProgressDialog::setCancelButton( QPushButton *cancelButton )
{
    delete d->cancel;
    d->cancel = cancelButton;
    if (cancelButton) {
	if ( cancelButton->parentWidget() == this ) {
	    cancelButton->hide(); // until we resize
	} else {
	    cancelButton->recreate( this, 0, QPoint(0,0), FALSE );
	}
	connect( d->cancel, SIGNAL(clicked()), this, SIGNAL(cancelled()) );
	QAccel *accel = new QAccel( this );
	accel->connectItem( accel->insertItem(Key_Escape),
			    d->cancel, SIGNAL(clicked()) );
    }
    resize(sizeHint());
    if (cancelButton)
	cancelButton->show();
}

/*!
  Sets the cancellation button text.
  \sa setCancelButton()
*/

void QProgressDialog::setCancelButtonText( const char *cancelButtonText )
{
    if ( cancelButtonText ) {
	if ( d->cancel )
	    d->cancel->setText(cancelButtonText);
	else
	    setCancelButton(new QPushButton(cancelButtonText, this, "cancel"));
    } else {
	setCancelButton(0);
    }
    resize(sizeHint());
}


/*!
  Sets the progress bar widget. The progress dialog resizes to fit.  The
  progress bar becomes owned by the progress dialog and will be deleted
  when necessary.
*/

void QProgressDialog::setBar( QProgressBar *bar )
{
    if ( progress() > 0 ) {
#if defined(CHECK_STATE)
	warning( "QProgrssDialog::setBar: Cannot set a new progress bar "
		 "while the old one is active" );
#endif
    }
    delete d->bar;
    d->bar = bar;
    resize(sizeHint());
}


/*!
  Returns the TRUE if the dialog was cancelled, otherwise FALSE.
  \sa setProgress(), cancel(), cancelled()
*/

bool QProgressDialog::wasCancelled() const
{
    return d->cancellation_flag;
}


/*!
  Returns the total number of steps.
  \sa setTotalSteps(), QProgressBar::totalSteps()
*/

int QProgressDialog::totalSteps() const
{
    return bar()->totalSteps();
}


/*!
  Sets the total number of steps.
  \sa totalSteps(), QProgressBar::setTotalSteps()
*/

void QProgressDialog::setTotalSteps( int totalSteps )
{
    bar()->setTotalSteps( totalSteps );
}


/*!
  Reset the progress dialog.
  The progress dialog becomes hidden.
*/

void QProgressDialog::reset()
{
    if ( progress() >= 0 ) {
	if ( d->creator )
	    d->creator->setCursor( d->parentCursor );
    }
    if ( isVisible() )
	hide();
    bar()->reset();
    d->cancellation_flag = FALSE;
    d->shown_once = FALSE;
}

/*!
  Reset the progress dialog.  wasCancelled() becomes TRUE until
  the progress dialog is reset.
  The progress dialog becomes hidden.
*/

void QProgressDialog::cancel()
{
    reset();
    d->cancellation_flag = TRUE;
}


/*!
  Returns the current amount of progress, or -1 if the progress counting
  has not started.
  \sa setProgress()
*/

int QProgressDialog::progress() const
{
    return bar()->progress();
}


/*!
  Sets the current amount of progress made to \e prog units of the
  total number of steps.  For the progress dialog to work correctly,
  you must at least call this with the parameter 0 initially, then
  later with QProgressDialog::totalSteps(), and you may call it any
  number of times in between.

  \warning If the progress dialog is modal
    (see QProgressDialog::QProgressDialog()),
    this function calls QApplication::processEvents(), so take care that
    this does not cause undesirable re-entrancy to your code. For example,
    don't use a QProgressDialog inside a paintEvent()!

  \sa progress()
*/

void QProgressDialog::setProgress( int progress )
{
    int old_progress = bar()->progress();

    if ( progress <= old_progress ||
	 progress == 0 && old_progress > 0 ||
	 progress != 0 && old_progress < 0 )
	 return;

    bar()->setProgress(progress);

    if ( d->shown_once ) {
	if (testWFlags(WType_Modal))
	    qApp->processEvents();
    } else {
	if ( progress == 0 ) {
	    if ( d->creator ) {
		d->parentCursor = d->creator->cursor();
		d->creator->setCursor( waitCursor );
	    }
	    d->starttime.start();
	} else {
	    int elapsed = d->starttime.elapsed();
	    if ( !d->showTime || elapsed > minWaitTime ) {
		int estimate = elapsed * (totalSteps() - progress) / progress;
		if ( estimate >= d->showTime ) {
		    resize(sizeHint());
		    center();
		    show();
		    d->shown_once = TRUE;
		}
	    }
	}
    }

    if ( progress == totalSteps() )
	reset();

    return;
}


void QProgressDialog::center()
{
    QPoint p(0,0);
    QWidget* w;
    if (d->creator) {
	p = d->creator->mapToGlobal( p );
	w = d->creator;
    } else {
	w = QApplication::desktop();
    }
    setGeometry( p.x() + w->width()/2  - width()/2,
	  p.y() + w->height()/2 - height()/2, width(), height() );
}


/*!
  Returns a size which fits the contents of the progress dialog.
  The progress dialog resizes itself as required, so this should not
  be needed in user code.
*/

QSize QProgressDialog::sizeHint() const
{
    QSize sh = label()->sizeHint();
    QSize bh = bar()->sizeHint();
    int h = margin_tb*2 + bh.height() + sh.height() + spacing;
    if ( d->cancel )
	h += d->cancel->sizeHint().height() + spacing;
    return QSize( QMAX(200, sh.width()), h );
}

/*!
  Handles resize events for the progress dialog, sizing the label,
  dialog, and cancellation button.
*/
void QProgressDialog::resizeEvent( QResizeEvent * )
{
    layout();
}

/*!
  Ensures layout conforms to style of GUI.
*/
void QProgressDialog::styleChange(GUIStyle s)
{
    QSemiModal::styleChange(s);
    layout();
}

void QProgressDialog::layout()
{
    int sp = spacing;
    int mtb = margin_tb;
    int mlr = QMIN(width()/10, margin_lr);
    const bool centered = (style() != WindowsStyle);

    QSize cs = d->cancel ? d->cancel->sizeHint() : QSize(0,0);
    QSize bh = bar()->sizeHint();
    int cspc;
    int lh = 0;

    // Find spacing and sizes that fit.  It is important that a progress
    // dialog can be made very small if the user demands it so.
    for (int attempt=5; attempt--; ) {
	cspc = d->cancel ? cs.height() + sp : 0;
	lh = QMAX(0, height() - mtb - bh.height() - sp - cspc);

	if ( lh < height()/4 ) {
	    // Getting cramped
	    sp /= 2;
	    mtb /= 2;
	    if ( d->cancel ) {
		cs.setHeight(QMAX(4,cs.height()-sp-2));
	    }
	    bh.setHeight(QMAX(4,bh.height()-sp-1));
	} else {
	    break;
	}
    }

    if ( d->cancel ) {
	d->cancel->setGeometry(
	    centered ? width()/2 - cs.width()/2 : width() - mlr - cs.width(),
	    height() - mtb - cs.height() + sp,
	    cs.width(), cs.height() );
    }

    label()->setGeometry( mlr, 0, width()-mlr*2, lh );
    bar()->setGeometry( mlr, lh+sp, width()-mlr*2, bh.height() );
}

/*!
  The dialog will not appear if the anticipated duration of the
  progressing task is less than \a ms milliseconds.

  If set to 0 the dialog is always shown as soon as any progress
  is set.
*/
void QProgressDialog::setMinimumDuration( int ms )
{
    d->showTime = ms;
}

/*!
  Returns the currently set minimum duration for the QProgressDialog
  
  \sa setMinimumDuration()
*/
int QProgressDialog::minimumDuration() const
{
    return d->showTime;
}
