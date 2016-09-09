/**********************************************************************
** $Id: qlineedit.cpp,v 2.91.2.5 1998/10/23 13:54:32 agulbra Exp $
**
** Implementation of QLineEdit widget class
**
** Created : 941011
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

#include "qlineedit.h"
#include "qpainter.h"
#include "qdrawutil.h"
#include "qfontmetrics.h"
#include "qpixmap.h"
#include "qkeycode.h"
#include "qclipboard.h"
#include "qapplication.h"
#include "qvalidator.h"
#include "qdragobject.h"
#include "qtimer.h"

#include <ctype.h>

struct QLineEditPrivate {
    QLineEditPrivate( QLineEdit * l ):
	frame(TRUE), mode(QLineEdit::Normal), validator( 0 ),
	pm(0), pmDirty( TRUE ),
	blinkTimer( l, "QLineEdit blink timer" ),
	dragTimer( l, "QLineEdit drag timer" ),
	inDoubleClick( FALSE ) {}

    bool frame;
    QLineEdit::EchoMode mode;
    QValidator * validator;
    QPixmap * pm;
    bool pmDirty;
    QTimer blinkTimer;
    QTimer dragTimer;
    QRect cursorRepaintRect;
    bool inDoubleClick;
};


/*!
  \class QLineEdit qlineedit.h

  \brief The QLineEdit widget is a simple line editor for inputting text.

  \ingroup realwidgets

  \define QLineEdit::EchoMode

  The default QLineEdit object has its own frame as specified by the
  Windows/Motif style guides, you can turn off the frame by calling
  setFrame( FALSE ).

  It draws the text using its own \link QColorGroup color group:
  \endlink \link QColorGroup::text() colorGroup().text() \endlink on
  \link QColorGroup::base() colorGroup().base(). \endlink  The cursor
  and frame use other colors from same color group, of course.

  QLineEdit can display the content of itself in three ways, depending
  on the current \link setEchoMode() echo mode. \endlink The echo
  modes available are: <ul> <li> \c Normal - display characters as
  they are entered.  This is the default. <li> \c NoEcho - do not
  display anything. <li> \c Password - display asterisks instead of
  the characters actually entered. </ul>

  The default key bindings are described in keyPressEvent(); they cannot
  be customized except by inheriting the class.

  <img src=qlined-m.gif> <img src=qlined-w.gif>

  \sa QMultiLineEdit QLabel QComboBox
  <a href="guibooks.html#fowler">GUI Design Handbook: Field, Entry,</a>
  <a href="guibooks.html#fowler">GUI Design Handbook: Field, Required.</a>
*/


/*!
  \fn void QLineEdit::textChanged( const char * )
  This signal is emitted every time the text has changed.
  The argument is the new text.
*/


static const int blinkTime  = 500;		// text cursor blink time
static const int scrollTime = 100;		// mark text scroll time


static int xPosToCursorPos( char *s, const QFontMetrics &fm,
			    int xPos, int width )
{
    char *tmp;
    int	  dist;

    if ( xPos > width )
	xPos = width;
    if ( xPos <= 0 )
	return 0;
    dist = xPos;
    tmp	 = s;
    while ( *tmp && dist > 0 )
	dist -= fm.width( tmp++, 1 );
    if ( dist < 0 && ( xPos - dist > width || fm.width( tmp - 1, 1)/2 < -dist))
	tmp--;
    return tmp - s;
}

static int showLastPartOffset( char *s, const QFontMetrics &fm, int width )
{
    if ( !s || s[0] == '\0' )
	return 0;
    char *tmp = &s[strlen( s ) - 1];
    do {
	width -= fm.width( tmp--, 1 );
    } while ( tmp >=s && width >=0 );
    return width < 0 ? tmp - s + 2 : 0;
}


/*!
  Constructs a line editor with an empty edit buffer.

  The cursor position is set to the start of the line, the maximum buffer
  size to 32767 characters, and the buffer contents to "".

  The \e parent and \e name arguments are sent to the QWidget constructor.
*/

QLineEdit::QLineEdit( QWidget *parent, const char *name )
    : QWidget( parent, name )
{
    d = new QLineEditPrivate( this );
    connect( &d->blinkTimer, SIGNAL(timeout()),
	     this, SLOT(blinkSlot()) );
    connect( &d->dragTimer, SIGNAL(timeout()),
	     this, SLOT(dragScrollSlot()) );
    cursorPos = 0;
    offset = 0;
    maxLen = 32767;
    cursorOn = TRUE;
    markAnchor = 0;
    markDrag = 0;
    dragScrolling = FALSE;
    scrollingLeft = FALSE;
    tbuf = "";
    setFocusPolicy( StrongFocus );
    setCursor( ibeamCursor );
    setBackgroundMode( PaletteBase );
    //setAcceptDrops( TRUE );
}

/*!
  Destroys the line editor.
*/

QLineEdit::~QLineEdit()
{
    if ( d->pm )
	delete d->pm;
    delete d;
}


/*!
  Sets the line editor text to \e text, clears the selection and moves
  the cursor to the end of the line.

  If necessary the text is truncated to fit maxLength().

  \sa text()
*/

void QLineEdit::setText( const char *text )
{
    QString oldText( tbuf );
    oldText.detach();
    tbuf = text ? text : "";
    if ( (int)tbuf.length() > maxLen ) {
	tbuf.resize( maxLen+1 );
	tbuf[maxLen] = '\0';
    }
    offset    = 0;
    cursorPos = 0;
    markAnchor = 0;
    markDrag = 0;
    end( FALSE );
    if ( validator() )
	(void)validator()->validate( tbuf, cursorPos );
    d->pmDirty = TRUE;
    repaint( FALSE );
    if ( oldText != tbuf )
	emit textChanged( tbuf );
}


/*!
  Selects all text (i.e. marks it) and moves the cursor to the
  end. Useful when a default value has been inserted. If the user
  types before clicking on the widget the selected text will be
  erased.
*/

void QLineEdit::selectAll()
{
    markAnchor = 0;
    markDrag = 0;
    cursorPos = 0;
    end( TRUE );
}



/*!
  Deselects all text (i.e. removes marking) and leaves the cursor at the
  current position.
*/

void QLineEdit::deselect()
{
    markAnchor = cursorPos;
    markDrag   = cursorPos;
    d->pmDirty = TRUE;
    repaint( FALSE );
}


/*!
  Returns a pointer to the text currently in the line.

  If you need to store the text, you should make a copy of it. This can
  conveniently be done with a QString object:
  \code
    QString s = lineEd->text();	 // makes a copy and stores it in s
  \endcode

  \sa setText()
*/

const char *QLineEdit::text() const
{
    return tbuf;
}

/*!
  Returns TRUE if part of the text has been marked by the user (e.g. by
  clicking and dragging).
*/

bool QLineEdit::hasMarkedText() const
{
    return markAnchor != markDrag;
}

/*!
  Returns the text marked by the user (e.g. by clicking and
  dragging), or 0 if no text is marked.
  \sa hasMarkedText()
*/

QString QLineEdit::markedText() const
{
    if ( markAnchor != markDrag ) {
	return tbuf.mid( minMark(), maxMark() - minMark() );
    } else {
	return 0;
    }
}

/*!
  Returns the current maximum length of the text in the editor.
  \sa setMaxLength()
*/

int QLineEdit::maxLength() const
{
    return maxLen;
}

/*!
  Set the maximum length of the text in the editor.  If the text is
  currently too long, it is chopped off at the limit. Any marked text will
  be unmarked.	The cursor position is set to 0 and the first part of the
  string is shown. \sa maxLength().
*/

void QLineEdit::setMaxLength( int m )
{
    if ( m > 32767 )
	m = 32767; // in case of application insanity
    maxLen = m;
    markAnchor = 0;
    markDrag = 0;
    if ( (int)tbuf.length() > maxLen ) {
	tbuf.resize( maxLen + 1 );		// include \0
	tbuf[maxLen] = '\0';
	d->pmDirty = TRUE;
    }
    if ( offset || cursorPos ) {
	offset    = 0;
	cursorPos = 0;
	d->pmDirty = TRUE;
    }
    if ( d->pmDirty )
	repaint( FALSE );
}

/*!
  \fn void  QLineEdit::returnPressed()
  This signal is emitted when the return or enter key is pressed.
*/

/*!
  The key press event handler converts a key press to some line editor
  action.

  If return or enter is pressed and the current text is valid (or if
  the validator can \link QValidator::fixup() make the text
  valid\endlink), the signal returnPressed is emitted.

  Here are the default key bindings:
  <ul>
  <li><i> Left Arrow </i> Move the cursor one character leftwards
  <li><i> Right Arrow </i> Move the cursor one character rightwards
  <li><i> Backspace </i> Delete the character to the left of the cursor
  <li><i> Home </i> Move the cursor to the beginning of the line
  <li><i> End </i>	 Move the cursor to the end of the line
  <li><i> Delete </i> Delete the character to the right of the cursor
  <li><i> Shift - Left Arrow </i> Mark text one character leftwards
  <li><i> Shift - Right Arrow </i> Mark text one character rightwards
  <li><i> Control-A </i> Move the cursor to the beginning of the line
  <li><i> Control-B </i> Move the cursor one character leftwards
  <li><i> Control-C </i> Copy the marked text to the clipboard.
  <li><i> Control-D </i> Delete the character to the right of the cursor
  <li><i> Control-E </i> Move the cursor to the end of the line
  <li><i> Control-F </i> Move the cursor one character rightwards
  <li><i> Control-H </i> Delete the character to the left of the cursor
  <li><i> Control-V </i> Paste the clipboard text into line edit.
  <li><i> Control-X </i> Cut the marked text, copy to clipboard.
  </ul>

  All other keys with valid ASCII codes insert themselves into the line.
*/

void QLineEdit::keyPressEvent( QKeyEvent *e )
{
    if ( e->key() == Key_Enter || e->key() == Key_Return ) {
	QValidator * v = validator();
	if ( !v || v->validate( tbuf, cursorPos ) == QValidator::Acceptable ) {
	    emit returnPressed();
	} else if ( v ) {
	    v->fixup( tbuf );
	    if ( v->validate( tbuf, cursorPos ) == QValidator::Acceptable )
		emit returnPressed();
	}
	// ### 2.0 must fix this
	e->ignore();
	return;
    }
    if ( e->ascii() >= 32 &&
	 e->key() != Key_Delete &&
	 e->key() != Key_Backspace ) {
	QString t( 2 );
	t[0] = e->ascii();
	t[1] = '\0';
	insert( t );
	return;
    }
    int unknown = 0;
    if ( e->state() & ControlButton ) {
	switch ( e->key() ) {
	case Key_A:
	case Key_Left:
	    home( e->state() & ShiftButton );
	    break;
	case Key_B:
	    cursorLeft( e->state() & ShiftButton );
	    break;
	case Key_C:
	    if ( hasMarkedText() && echoMode() == Normal )
		copyText();
	    break;
	case Key_D:
	    del();
	    break;
	case Key_E:
	case Key_Right:
	    end( e->state() & ShiftButton );
	    break;
	case Key_F:
	    cursorRight( e->state() & ShiftButton );
	    break;
	case Key_H:
	    backspace();
	    break;
	case Key_K:
	    if ( cursorPos < (int)tbuf.length() ) {
		QString t( tbuf );
		t.detach(); // ### 2.0
		t.truncate( cursorPos );
		validateAndSet( t, cursorPos, cursorPos, cursorPos );
	    }
	    break;
	case Key_V:
	    insert( QApplication::clipboard()->text() );
	case Key_X:
	    if ( hasMarkedText() && echoMode() == Normal ) {
		copyText();
		del();
	    }
	    break;
	default:
	    unknown++;
	}
    } else {
	switch ( e->key() ) {
	case Key_Left:
	    cursorLeft( e->state() & ShiftButton );
	    break;
	case Key_Right:
	    cursorRight( e->state() & ShiftButton );
	    break;
	case Key_Backspace:
	    backspace();
	    break;
	case Key_Home:
	    home( e->state() & ShiftButton );
	    break;
	case Key_End:
	    end( e->state() & ShiftButton );
	    break;
	case Key_Delete:
	    del();
	    break;
	default:
	    unknown++;
	}
    }

    if ( unknown ) {				// unknown key
	e->ignore();
	return;
    }
}


/*!
  Handles the cursor blinking.
*/

void QLineEdit::focusInEvent( QFocusEvent * )
{
    cursorOn = FALSE;
    d->pmDirty = TRUE;
    blinkSlot();
}


/*!
  Handles the cursor blinking and selection copying.
*/

void QLineEdit::focusOutEvent( QFocusEvent * )
{
    if ( style() == WindowsStyle ) {
#if defined(_WS_X11_)
	// X11 users are very accustomed to "auto-copy"
	copyText();
#endif
	if ( focusWidget() != this ||
	   qApp->focusWidget() == 0 ||
	   qApp->focusWidget()->topLevelWidget() != topLevelWidget() )
	    deselect();
    }
    d->dragTimer.stop();
    if ( cursorOn )
	blinkSlot();
}

/*!
  Handles selection copying.
*/
void QLineEdit::leaveEvent( QEvent * )
{
#if defined(_WS_X11_)
    if ( style() == WindowsStyle ) {
	// X11 users are very accustomed to "auto-copy"
	copyText();
    }
#endif
}


/*!
  Handles paint events for the line editor.
*/

void QLineEdit::paintEvent( QPaintEvent *e )
{
    if ( !d->pm || d->pmDirty ) {
	if ( !d->pm )
	    d->pm = new QPixmap( size() );
	QPainter p( d->pm, this );

	QColorGroup g = colorGroup();
	QColor bg = isEnabled() ? g.base() : g.background();
	QFontMetrics fm = fontMetrics();
	int markBegin = minMark();
	int markEnd = maxMark();
	int margin = frame() ? 2 : 0;

	if ( frame() ) {
	    QBrush fill( bg );
	    qDrawWinPanel( &p, 0, 0, width(), height(), g, TRUE, &fill );
	} else {
	    p.fillRect( 0, 0, width(), height(), bg );
	}

	QString displayText;

	switch( echoMode() ) {
	case Normal:
	    displayText = tbuf.mid( offset, tbuf.length() );
	    break;
	case NoEcho:
	    displayText = "";
	    break;
	case Password:
	    displayText.fill( '*', tbuf.length() - offset );
	    break;
	}

	int ypos = height() - margin - fm.descent() - 1 -
		   (height() - 2*margin - fm.height())/2;

	if ( !displayText.isEmpty() ) {
	    int charsVisible = lastCharVisible() - offset;
	    if ( displayText[ charsVisible ] != '\0' )
		charsVisible++;

	    int mark1,mark2;

	    if ( markBegin > offset ) {
		if ( markBegin <  offset + charsVisible )
		    mark1 = markBegin - offset;
		else
		    mark1 = charsVisible;
	    } else {
		mark1 = 0;
	    }

	    if ( markEnd > offset ) {
		if ( markEnd <	offset + charsVisible )
		    mark2 = markEnd - offset;
		else
		    mark2 = charsVisible;
	    } else {
		mark2 = 0;
	    }

	    // display code comes here - a bit yucky but it works
	    if ( mark1 != mark2 ) {
		QString marked( displayText.mid( mark1, mark2 - mark1 ) );
		int xpos1 =  margin + 2 + fm.width( displayText, mark1 );
		int xpos2 =  xpos1 + fm.width( marked ) - 1;
		p.fillRect( xpos1, ypos - fm.ascent(),
			    xpos2 - xpos1, fm.height(),
			    style() == WindowsStyle
			    ? QApplication::winStyleHighlightColor()
			    : g.text() );
		p.setPen(  style() == WindowsStyle ? white : g.base() );
		p.drawText( xpos1, ypos, marked );
	    }
	    p.setPen( g.text() );
	    if ( mark1 != 0 )
		p.drawText( margin + 2, ypos, displayText, mark1 );
	    if ( mark2 != charsVisible ) {
		QString rest( displayText.mid( mark2, charsVisible - mark2 ) );
		p.drawText( margin + 2 + fm.width( displayText.left( mark2) ),
			    ypos, rest );
	    }
	}

	p.setPen( g.foreground() );

	int curXPos = margin + 2;
	if ( echoMode() != NoEcho )
	    curXPos += offset > cursorPos ? -1 : // ?: for scrolling case
			    fm.width( displayText, cursorPos - offset ) - 1;
	int curYPos   = ypos - fm.ascent();
	d->cursorRepaintRect.setRect( curXPos-2, curYPos, 5, fm.height() );
	d->pmDirty = FALSE;
    }
	
    bitBlt( this, e->rect().topLeft(), d->pm, e->rect() );
    if ( hasFocus() ) {
	if ( cursorOn && d->cursorRepaintRect.intersects( e->rect() ) ) {
	    QPainter p( this );
	    int curYTop = d->cursorRepaintRect.y();
	    int curYBot = d->cursorRepaintRect.bottom();
	    int curXPos = d->cursorRepaintRect.x() + 2;
	    p.drawLine( curXPos, curYTop, curXPos, curYBot );
	    if ( style() != WindowsStyle ) {
		p.drawLine( curXPos - 2, curYTop, curXPos + 2, curYTop );
		p.drawLine( curXPos - 2, curYBot, curXPos + 2, curYBot );
	    }
	}
    } else {
	delete d->pm;
	d->pm = 0;
    }

}

/*!
  Not used.
*/

void QLineEdit::timerEvent( QTimerEvent * e )
{
    QWidget::timerEvent( e );
}


/*!
  Handles resize events for this widget.
*/

void QLineEdit::resizeEvent( QResizeEvent * )
{
    delete d->pm;
    d->pm = 0;
    int max = lastCharVisible();
    if ( cursorPos > max ) {
	QFontMetrics fm = fontMetrics();
	int w = width() - (frame() ? 8 : 4);
	int i = cursorPos;
	while ( w > 0 && i > 0 ) {
	    i--;
	    w -= fm.width( tbuf.at(i) );
	}
	if ( w < 0 && i != cursorPos )
	    i++;
	offset = i;
    } else if ( offset ) {
	int i = showLastPartOffset( tbuf.data(), fontMetrics(),
				    width() - (frame() ? 8 : 4) );
	if ( i < offset )
	    offset = i;
    }
    d->pmDirty = TRUE;
    repaint( FALSE );
}


/*!
  Handles mouse press events for this widget.
*/

void QLineEdit::mousePressEvent( QMouseEvent *e )
{
    killTimers();
    d->inDoubleClick = FALSE;
    int margin = frame() ? 4 : 2;
    cursorPos = offset + xPosToCursorPos( &tbuf[(int)offset], fontMetrics(),
					  e->pos().x() - margin,
					  width() - 2*margin );
    if ( e->button() == MidButton ) {
#if defined(_WS_X11_)
	insert( QApplication::clipboard()->text() );
#else
	if ( style() == MotifStyle )
	    insert( QApplication::clipboard()->text() );
#endif
	return;
#if 0 // it works, but it's wait until we have an API
    } else if ( hasMarkedText() &&
		e->button() == LeftButton &&
		( (markAnchor > cursorPos && markDrag < cursorPos) ||
		  (markAnchor < cursorPos && markDrag > cursorPos) ) ) {
	QTextDrag * tdo = new QTextDrag( this );
	tdo->setText( markedText() );
	tdo->dragCopy();
	delete tdo;
	return;
#endif
    }

    int m1 = minMark();
    int m2 = maxMark();
    markAnchor = cursorPos;
    newMark( markAnchor, FALSE );
    if ( cursorPos > m2 )
	m2 = cursorPos;
    else if ( cursorPos < m1 )
	m1 = cursorPos;
    repaintArea( m1, m2 );
    dragScrolling = FALSE;
}


/*!
  Handles mouse move events for the line editor, primarily for
  marking text.
*/

void QLineEdit::mouseMoveEvent( QMouseEvent *e )
{
    int margin = frame() ? 4 : 2;

    if ( e->pos().x() < margin || e->pos().x() > width() - margin ) {
	scrollingLeft = ( e->pos().x() < margin );
	if ( !dragScrolling ) {
	    dragScrolling = TRUE;
	    if ( scrollingLeft )
		newMark( offset, FALSE );
	    else
		newMark( lastCharVisible(), FALSE );
	    d->dragTimer.start( scrollTime );
	} else {
	    if ( scrollingLeft ) {
		int steps = -(e->pos().x() + margin) / 15 + 2;
		cursorLeft( TRUE, steps );
	    } else {
		int steps = (e->pos().x() - width() +  margin) / 15 + 2;
		cursorRight( TRUE, steps );
	    }
	}
    } else {
	dragScrolling = FALSE;
	int mousePos = offset + xPosToCursorPos( &tbuf[(int)offset],
						 fontMetrics(),
						 e->pos().x() - margin,
						 width() - margin - margin );
	int m1 = markDrag;
	newMark( mousePos, FALSE );
	repaintArea( m1, mousePos );
    }
}

/*!
  Handles mouse release events for this widget.
*/

void QLineEdit::mouseReleaseEvent( QMouseEvent * e )
{
    if ( d->inDoubleClick ) {
	d->inDoubleClick = FALSE;
	return;
    }

#if defined(_WS_X11_)
    if ( hasMarkedText() && echoMode() == Normal )
	copyText();
#else
    if ( style() == MotifStyle && hasMarkedText() && echoMode() == Normal )
	copyText();
#endif
    if ( dragScrolling )
	dragScrolling = FALSE;
    if ( e->button() != LeftButton )
	return;

    int margin = frame() ? 4 : 2;
    if ( !QRect( margin, margin,
		 width() - 2*margin,
		 height() - 2*margin ).contains( e->pos() ) )
	return;

    int mousePos = offset + xPosToCursorPos( &tbuf[(int)offset],
					     fontMetrics(),
					     e->pos().x() - margin,
					     width() - margin - margin );
    int m1 = markDrag;
    newMark( mousePos, FALSE );
    repaintArea( m1, mousePos );
}


/*!
  Handles mouse double click events for this widget.
*/

void QLineEdit::mouseDoubleClickEvent( QMouseEvent * )
{
    d->inDoubleClick = TRUE;

    if ( dragScrolling )
	dragScrolling = FALSE;

    markWord( cursorPos );
    repaint( FALSE ); // should use repaintArea()...
}

/*!
  Obsolete.
*/

void QLineEdit::paint( const QRect&, bool )
{
    fatal( "go away" );
}

/*!
  Obsolete.
*/

void QLineEdit::pixmapPaint( const QRect& )
{
    fatal( "go away" );
}


/*!
  Obsolete.
*/

void QLineEdit::paintText( QPainter *, const QSize &, bool )
{
    fatal( "go away" );
}


/*!
  Moves the cursor leftwards one or more characters.
  \sa cursorRight()
*/

void QLineEdit::cursorLeft( bool mark, int steps )
{
    if ( steps < 0 ) {
	cursorRight( mark, -steps );
	return;
    }
    if ( cursorPos > 0 || (!mark && hasMarkedText()) ) {
	cursorPos -= steps;
	if ( cursorPos < 0 )
	    cursorPos = 0;
	cursorOn = FALSE;
	blinkSlot();
	int minP = QMIN( minMark(), cursorPos );
	int maxP = QMAX( maxMark(), cursorPos );
	if ( mark )
	    newMark( cursorPos );
	else
	    markAnchor = markDrag = cursorPos;
	d->pmDirty = TRUE;
	repaintArea( minP, maxP );
    }
}

/*!
  Moves the cursor rightwards one or more characters.
  \sa cursorLeft()
*/

void QLineEdit::cursorRight( bool mark, int steps )
{
    if ( steps < 0 ) {
	cursorLeft( mark, -steps );
	return;
    }
    int len = (int)strlen( tbuf );
    if ( cursorPos < len || (!mark && hasMarkedText()) ) {
	int minP = QMIN( cursorPos, minMark() );
	cursorPos += steps;
	if ( cursorPos > len )
	    cursorPos = len;
	cursorOn = FALSE;
	blinkSlot();
	int maxP = QMAX( cursorPos, maxMark() );
	if ( mark )
	    newMark( cursorPos );
	else
	    markAnchor = markDrag = cursorPos;
	d->pmDirty = TRUE;
	repaintArea( minP, maxP );
    }
}

/*!
  Deletes the character on the left side of the text cursor and moves the
  cursor one position to the left. If a text has been marked by the user
  (e.g. by clicking and dragging) the cursor will be put at the beginning
  of the marked text and the marked text will be removed.  \sa del()
*/

void QLineEdit::backspace()
{
    if ( hasMarkedText() ) {
	del();
    } else {
	if ( cursorPos > 0 ) {
	    cursorLeft( FALSE );
	    del();
	}
    }
}

/*!
  Deletes the character on the right side of the text cursor. If a text
  has been marked by the user (e.g. by clicking and dragging) the cursor
  will be put at the beginning of the marked text and the marked text will
  be removed.  \sa backspace()
*/

void QLineEdit::del()
{
    QString test( tbuf.copy() );

    if ( hasMarkedText() ) {
	test.remove( minMark(), maxMark() - minMark() );
	validateAndSet( test, minMark(), minMark(), minMark() );
    } else if ( cursorPos != (int)strlen(tbuf) ) {
	test.remove( cursorPos, 1 );
	validateAndSet( test, minMark(), minMark(), minMark() );
    }
}

/*!
  Moves the text cursor to the left end of the line. If mark is TRUE text
  will be marked towards the first position, if not any marked text will
  be unmarked if the cursor is moved.  \sa end()
*/

void QLineEdit::home( bool mark )
{
    if ( cursorPos != 0 || (!mark && hasMarkedText()) ) {
	int m = cursorPos;
	cursorPos = 0;
	cursorOn = FALSE;
	blinkSlot();
	if ( mark ) {
	    m = QMAX( minMark(), m );
	    newMark( cursorPos );
	} else {
	    m = QMAX( maxMark(), m );
	    markDrag = markAnchor = 0;
	}
	d->pmDirty = TRUE;
	if ( offset ) {
	    offset = 0;
	    repaint( FALSE );
	} else {
	    repaintArea( 0, m );
	}
    }
}

/*!
  Moves the text cursor to the right end of the line. If mark is TRUE text
  will be marked towards the last position, if not any marked text will
  be unmarked if the cursor is moved.
  \sa home()
*/

void QLineEdit::end( bool mark )
{
    int tlen = strlen( tbuf );
    if ( cursorPos != tlen || (!mark && hasMarkedText()) ) {
	int mo = showLastPartOffset( &tbuf[offset], fontMetrics(),
				     width() - (frame() ? 8 : 4) );
	int markStart = cursorPos;
	cursorPos = tlen;
	cursorOn = FALSE;
	blinkSlot();
	if ( mark ) {
	    markStart = QMIN( markStart, markDrag );
	    newMark( cursorPos );
	} else {
	    markStart = QMIN( markStart, minMark() );
	    markAnchor = markDrag = cursorPos;
	}
	d->pmDirty = TRUE;
	if ( mo > 0 ) {
	    offset += mo;
	    repaint( FALSE );
	} else {
	    repaintArea( markStart, tlen );
	}
    }
}


/*!
  Sets a new marked text limit, does not repaint the widget.
*/

void QLineEdit::newMark( int pos, bool copy )
{
    if ( markDrag != pos || cursorPos != pos )
	d->pmDirty = TRUE;
    markDrag  = pos;
    cursorPos = pos;
    if ( copy && style() == MotifStyle && echoMode() == Normal )
	copyText(); // ### ????????????
}


void QLineEdit::markWord( int pos )
{
    int i = pos - 1;
    while ( i >= 0 && isprint(tbuf.at(i)) && !isspace(tbuf.at(i)) )
	i--;
    i++;
    markAnchor = i;

    int lim = tbuf.length();
    i = pos;
    while ( i < lim && isprint(tbuf.at(i)) && !isspace(tbuf.at(i)) )
	i++;
    markDrag = i;

    int maxVis	  = lastCharVisible();
    int markBegin = minMark();
    int markEnd	  = maxMark();
    if ( markBegin < offset || markBegin > maxVis ) {
	if ( markEnd >= offset && markEnd <= maxVis ) {
	    cursorPos = markEnd;
	} else {
	    offset    = markBegin;
	    cursorPos = markBegin;
	}
    } else {
	cursorPos = markBegin;
    }
    if ( style() == MotifStyle && echoMode() == Normal )
	copyText();
    d->pmDirty = TRUE;
}


/*!
  Copies the marked text to the clipboard.  Please use copy() instad.
*/

void QLineEdit::copyText()
{
        copy();
}


/*! Copies the marked text to the clipboard, if there is any.
  
  \sa cut() paste()
*/

void QLineEdit::copy() const
{
    QString t = markedText();
    if ( !t.isEmpty() ) {
        disconnect( QApplication::clipboard(), SIGNAL(dataChanged()), this, 0);
        QApplication::clipboard()->setText( t );
        connect( QApplication::clipboard(), SIGNAL(dataChanged()),
                 this, SLOT(clipboardChanged()) );
    }
}


/* Inserts the clipboard's text at the cursor position, deleting any
  previous marked text.
   
  If the end result is not acceptable for the current validator,
  nothing happens.
   
  \sa copy() cut()
*/

void QLineEdit::paste()
{
    insert( QApplication::clipboard()->text() );
}

/*!
  Copies the marked text to the clipboard and deletes it, if there is
  any.
  
  If the current validator disallows deleting the marked text, cut()
  will copy it but not delete it.
  
  \sa copy() paste()
*/

void QLineEdit::cut()
{
    QString t = markedText();
    if ( !t.isEmpty() ) {
	copy();
	del();
    }
}


/*!
  This private slot is activated when this line edit owns the clipboard and
  some other widget/application takes over the clipboard. (X11 only)
*/

void QLineEdit::clipboardChanged()
{
#if defined(_WS_X11_)
    disconnect( QApplication::clipboard(), SIGNAL(dataChanged()),
		this, SLOT(clipboardChanged()) );
    markDrag = markAnchor = cursorPos;
    repaint( !hasFocus() );
#endif
}



int QLineEdit::lastCharVisible() const
{
    int tDispWidth = width() - (frame() ? 8 : 4);
    return offset + xPosToCursorPos( &tbuf[(int)offset], fontMetrics(),
			       tDispWidth, tDispWidth );
}

int QLineEdit::minMark() const
{
    return markAnchor < markDrag ? markAnchor : markDrag;
}

int QLineEdit::maxMark() const
{
    return markAnchor > markDrag ? markAnchor : markDrag;
}



/*!  Sets the line edit to draw itself inside a two-pixel frame if \a
  enable is TRUE, and to draw itself without any frame if \a enable is
  FALSE.

  The default is TRUE.

  \sa frame() QComboBox
*/

void QLineEdit::setFrame( bool enable )
{
    if ( d->frame == enable )
	return;

    d->frame = enable;
    repaint();
}


/*!  Returns TRUE if the line edit draws itself inside a frame, FALSE
  if it draws itself without any frame.

  The default is to use a frame.

  \sa setFrame()
*/

bool QLineEdit::frame() const
{
    return d ? d->frame : TRUE;
}


/*!  Sets the echo mode of the line edit widget.

  The echo modes available are: <ul> <li> \c Normal - display
  characters as they are entered.  This is the default. <li> \c NoEcho
  - do not display anything. <li> \c Password - display asterisks
  instead of the characters actually entered. </ul>

  It is always possible to cut and paste any marked text; only the
  widget's own display is affected.

  \sa echoMode()
*/

void QLineEdit::setEchoMode( EchoMode mode )
{
    if ( d->mode == mode )
	return;

    d->mode = mode;
    repaint();
}


/*!
  Returns the current echo mode of the line edit.

  \sa setEchoMode()
*/

QLineEdit::EchoMode QLineEdit::echoMode() const
{
    return d ? d->mode : Normal;
}


/*!
  Returns a size which fits the contents of the line edit.

  The width returned tends to be enough for about 15-20 characters.
*/

QSize QLineEdit::sizeHint() const
{
    QFontMetrics fm( font() );
    int h = fm.height();
    int w = fm.width( "about 15-20 chars." );
    if ( frame() ) {
	h += 8;
	if ( style() == WindowsStyle && h < 26 )
	    h = 22;
	return QSize( w + 8, h );
    } else {
	return QSize( w + 4, h + 4 );
	
    }

}


/*!
  Sets this line edit to accept input only as accepted by \a v.

  If \a v == 0, remove the currently set input validator.  The default
  is no input validator (ie. any input is accepted up to maxLength()).

  \sa validator() QValidator
*/


void QLineEdit::setValidator( QValidator * v )
{
    if ( v == d->validator )
	return;

    d->validator = v;
    repaint();
}

/*!
  Returns a pointer to the current input validator, or 0 if no
  validator has been set.
*/

QValidator * QLineEdit::validator() const
{
    return d ? d->validator : 0;
}


/*!  This slot is equivalent to setValidator( 0 ). */

void QLineEdit::clearValidator()
{
    setValidator( 0 );
}


/*!  Don't use it if you don't mean it. */

bool QLineEdit::event( QEvent * e )
{
#if 0 // it works, but we'll wait with enabling it.
    if ( !e )
	return QWidget::event( e );

    if ( e->type() == Event_DragEnter ) {
	if ( ((QDragEnterEvent *) e)->provides( "text/plain" ) ) {
	    ((QDragEnterEvent *) e)->accept( rect() );
	    return TRUE;
	}
    } else if ( e->type() == Event_DragLeave ) {
	return TRUE;
    } else if ( e->type() == Event_Drop ) {
	QDropEvent * de = (QDropEvent *) e;
	QString str;
	if ( QTextDrag::decode( de, str ) ) {
	    if ( !hasMarkedText() ) {
		int margin = frame() ? 2 : 0;
		setCursorPosition( xPosToCursorPos( &tbuf[(int)offset],
						    fontMetrics(),
						    de->pos().x() - margin,
						    width() - 2*margin ) );
	    }
	    insert( str );
	    de->accept();
	} else {
	    de->ignore();
	}
	return TRUE;
    }
#endif
    return QWidget::event( e );
}


/*!  This private slot handles cursor blinking. */

void QLineEdit::blinkSlot()
{
    if ( hasFocus() || cursorOn ) {
	cursorOn = !cursorOn;
	if ( !d->pmDirty && d->cursorRepaintRect.isValid() )
	    repaint( d->cursorRepaintRect, FALSE );
	else
	    repaint( FALSE );
    }
    if ( !hasFocus() )
	d->blinkTimer.stop();
    else if ( !d->blinkTimer.isActive() )
	d->blinkTimer.start( blinkTime );
}


/*!  This private slot handles drag-scrolling. */

void QLineEdit::dragScrollSlot()
{
    if ( !hasFocus() || !dragScrolling )
	d->dragTimer.stop();
    else if ( scrollingLeft )
	cursorLeft( TRUE );
    else
	cursorRight( TRUE );
}


/*!  Validates and perhaps sets this line edit to contain \a newText
  with the cursor at position newPos, with marked text from \a
  newMarkAnchor to \a newMarkDrag.  Returns TRUE if it changes the line
  edit and FALSE if it doesn't.

  If \a newText contains more than one line is longer than
  maxLength(), validateAndSet() truncates it before testing its
  validity.

  Repaints and emits textChanged() if appropriate.
*/

bool QLineEdit::validateAndSet( const char * newText, int newPos,
				int newMarkAnchor, int newMarkDrag )
{
    QString t( newText );
    if ( !t.isEmpty() ) {
	uchar *p = (uchar *) t.data();
	while ( *p ) {		// unprintable/linefeed becomes space
	    if ( *p < 32 )
		*p = 32;
	    p++;
	}
    }
    if ( t.length() > (uint)maxLength() )
	t.truncate( maxLength() );

    QValidator * v = validator();

    if ( v && v->validate( t, newPos ) == QValidator::Invalid &&
	 v->validate( tbuf, cursorPos ) != QValidator::Invalid ) {
	return FALSE;
    }

    // okay, it succeeded
    if ( newMarkDrag != markDrag ||
	 newMarkAnchor |! markAnchor ||
	 newPos != cursorPos ||
	 t != tbuf ) {
	int minP = QMIN( cursorPos, minMark() );
	int maxP = QMAX( cursorPos, maxMark() );
	cursorPos = newPos;
	markAnchor = newMarkAnchor;
	markDrag = newMarkDrag;
	d->pmDirty = TRUE;

	minP = QMIN( minP, QMIN( cursorPos, minMark() ) );
	maxP = QMAX( maxP, QMAX( cursorPos, maxMark() ) );
	
	if ( tbuf == t || tbuf == t.right( tbuf.length() ) ) {
	    int i = 0;
	    while( i < minP && t[i] == tbuf[i] )
		i++;
	    minP = i;
	    i = t.length();
	    if ( i > (int) tbuf.length() ) {
		tbuf = t;
	    } else {
		while( i > maxP && t[i] == tbuf[i] )
		    i--;
	    }
	    maxP = i;
	    repaintArea( minP, maxP );
	} else {
	    tbuf = t;
	    d->pmDirty = TRUE;
	    QFontMetrics fm = fontMetrics();
	    int x;
	    if ( offset > cursorPos )
		x = 0;
	    else
		x = fm.width(t.mid( offset, cursorPos - offset) );
	    int margin = frame() ? 2 : 0;
	    if ( x >= width() - margin ) {
		while( x >= width() - margin ) {
		    int w = fm.width( tbuf[offset] );
		    x -= w;
		    offset++;
		}
	    }
	    d->pmDirty = TRUE;
	    repaint( FALSE );
	}
    }
    emit textChanged( tbuf );
    return TRUE;
}


/*!  Removes any currently selected text, inserts \a newText,
  validates the result and if it is valid, sets it as the new contents
  of the line edit.

*/

void QLineEdit::insert( const char * newText )
{
    QString t( newText );
    if ( t.isEmpty() )
	return;

    uchar *p = (uchar *) t.data();
    while ( *p ) {		// unprintable/nl becomes space
	if ( *p < 32 )
	    *p = 32;
	p++;
    }

    QString test( tbuf.copy() );
    int cp = cursorPos;
    if ( hasMarkedText() ) {
	test.remove( minMark(), maxMark() - minMark() );
	cp = minMark();
    }
    test.insert( cp, t );
    cp = QMIN( cp+t.length(), (uint)maxLength() );
    cursorOn = FALSE;
    blinkSlot();
    validateAndSet( test, cp, cp, cp );
}


/*!  Repaints all characters from \a from to \a to. If cursorPos is
  between from and to, ensures that cursorPos is visible. */

void QLineEdit::repaintArea( int from, int to )
{
    int a, b;
    if ( from < to ) {
	a = from;
	b = to;
    } else {
	a = to;
	b = from;
    }

    if ( offset > cursorPos ) {
	offset = cursorPos;
	d->pmDirty = TRUE;
	repaint( FALSE );
	return;
    }

    if ( a < offset )
	a = offset;
    if ( b < a )
	b = a;
    if ( b > (int)tbuf.length() )
	b = tbuf.length();

    QFontMetrics fm( fontMetrics() );

    int margin = frame() ? 2 : 0;
    int x3 = fm.width( tbuf.mid( offset, cursorPos - offset ) ) + 2*margin;
    if ( x3 >= width() ) {
	while( x3 >= width() ) {
	    x3 -= fm.width( tbuf[offset] );
	    offset++;
	}
	d->pmDirty = TRUE;
	repaint( FALSE );
	return;
    }

    int x1, x2;
    if ( a > offset )
	x1 = fm.width( tbuf.mid(offset, a-offset) );
    else
	x1 = 0;
    if ( b > a )
	x2 = x1 + fm.width( tbuf.mid(a, b-a) );
    else
	x2 = x1;

    x1 += margin - 3;
    x2 += margin + 2 + 3;
    if ( x2 > width() - margin + 2 )
	x2 = width() - margin + 2;
    repaint( x1, margin, x2-x1, height() - 2*margin, FALSE );
}


/*!  \reimp */

void QLineEdit::setEnabled( bool e )
{
    d->pmDirty = TRUE;
    QWidget::setEnabled( e );
}


/*! \reimp */

void QLineEdit::setFont( const QFont & f )
{
    d->pmDirty = TRUE;
    QWidget::setFont( f );
}


/*!  Syntax sugar for setText( "" ), provided to match no-argument
  signals.
*/

void QLineEdit::clear()
{
    setText( "" );
}


/*!  Sets the marked area of this line edit to start at \a start and
  be \a length characters long. */

void QLineEdit::setSelection( int start, int length )
{
    int b, e;
    b = QMIN( markAnchor, markDrag );
    e = QMAX( markAnchor, markDrag );
    b = QMIN( b, start );
    e = QMAX( e, start + length );
    markAnchor = start;
    markDrag = start + length;
    repaintArea( b, e );
}


/*!  Set the cursor position for this line edit to \a newPos and
  repaint accordingly.  \sa cursorPosition() */

void QLineEdit::setCursorPosition( int newPos )
{
    if ( newPos > (int)tbuf.length() || newPos < 0 )
	return;
    int b, e;
    b = QMIN( newPos, cursorPos );
    e = QMAX( newPos, cursorPos );
    d->pmDirty = TRUE;
    cursorPos = newPos;
    repaintArea( b, e );
}


/*!  Returns the current cursor position for this line edit.  \sa
  setCursorPosition() */

int QLineEdit::cursorPosition() const
{
    return cursorPos;
}


/*! \reimp */

void QLineEdit::setPalette( const QPalette & p )
{
    d->pmDirty = TRUE;
    QWidget::setPalette( p );
}
