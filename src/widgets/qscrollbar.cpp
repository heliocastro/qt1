/****************************************************************************
** $Id: qscrollbar.cpp,v 2.39.2.4 1998/10/04 17:21:18 agulbra Exp $
**
** Implementation of QScrollBar class
**
** Created : 940427
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

#include "qscrollbar.h"
#include "qpainter.h"
#include "qbitmap.h"
#include "qkeycode.h"
#include "qapplication.h"
#include <limits.h>

/*!
  \class QScrollBar qscrollbar.h

  \brief The QScrollBar widget provides a vertical or horizontal scroll bar.

  \ingroup realwidgets

  A scroll bar is used to let the user control a value within a
  program-definable range, and to give the user visible indication of
  the current value of a \link QRangeControl range control \endlink.

  Scroll bars include four separate controls, in order: <ul> <li> The
  \e line-up control is a little triangle with which the user can move
  one line up.  The meaning of line is configurable. In e.g. editors
  and list boxes means one line of text.  <li> The \e slider is the
  handle that indicates the current value of the scroll bar, and which
  the user can drag to change the value.  <li> The \a page-up/down
  control is the area on which the slider slides (the scroll bar's
  background).  Clicking here moves the scroll bar up or down one
  page.  The meaning of page too is configurable - in editors and list
  boxes it means as many lines as there is space for in the widget.
  <li> Finally, the line-down control is the arrow on the other end of
  the scroll bar.  Clicking there moves the scroll bar down/rightwards
  one line.</ul>

  QScrollBar has not much of an API of its own; it mostly relies on
  QRangeControl.  The most useful functions are setValue() to set the
  scrollbar directly to some value; addPage(), addLine(), subPage()
  and subLine() to simulate the effects of clicking (neat for
  accelerator keys; setSteps() to define the values of pageStep() and
  lineStep(); and last but NOT least setRange() to set the minValue()
  and maxValue() of the scrollbar.  (QScrollBar has a convenience
  constructor with which you can set most of that.)

  In addition to the access functions from QRangeControl, QScrollBar
  has a comprehensive set of signals: <ul>

  <li> valueChanged() - emitted when the scroll bar's value has changed.

  <li> sliderPressed() - emitted when the user starts to drag the
  slider

  <li> sliderMoved() - emitted when the user drags the slider

  <li> sliderReleased() - emitted when the user releases the slider

  <li> nextLine() - emitted when the scroll bar has moved one line
  down/rightwards.  Line is defined in QRangeControl.

  <li> prevLine() - emitted when the scroll bar has moved one line
  up/leftwards.

  <li> nextPage() - emitted when the scroll bar has moved one page
  down/rightwards.

  <li> prevPage() - emitted when the scroll bar has moved one page
  up/leftwards.

  </ul>

  QScrollBar only offers integer ranges, and the current
  implementation has problems when the range is greater than a million
  or so.  (A million is more than sufficient for today's display
  sizes, however.)

  A scroll bar can be controlled by the keyboard, but it has a
  default focusPolicy() of \c NoFocus. Use setFocusPolicy() to
  enable keyboard focus.

  <img src=qscrbar-m.gif> <img src=qscrbar-w.gif>

  \sa QSlider QSpinBox
  <a href="guibooks.html#fowler">GUI Design Handbook: Scroll Bar</a>
*/


/*!
  \fn void QScrollBar::valueChanged( int value )
  This signal is emitted when the scroll bar value is changed, with the
  new scroll bar value as an argument.
*/

/*!
  \fn void QScrollBar::sliderPressed()
  This signal is emitted when the user presses the slider with the mouse.
*/

/*!
  \fn void QScrollBar::sliderMoved( int value )

  This signal is emitted when the slider is moved by the user, with
  the new scroll bar value as an argument.

  This signal is emitted even when tracking is turned off.

  \sa tracking() valueChanged() nextLine() prevLine() nextPage() prevPage()
*/

/*!
  \fn void QScrollBar::sliderReleased()
  This signal is emitted when the user releases the slider with the mouse.
*/

/*!
  \fn void QScrollBar::nextLine()
  This signal is emitted when the scroll bar scrolls one line down/right.
*/

/*!
  \fn void QScrollBar::prevLine()
  This signal is emitted when the scroll bar scrolls one line up/left.
*/

/*!
  \fn void QScrollBar::nextPage()
  This signal is emitted when the scroll bar scrolls one page down/right.
*/

/*!
  \fn void QScrollBar::prevPage()
  This signal is emitted when the scroll bar scrolls one page up/left.
*/


enum ScrollControl { ADD_LINE = 0x1 , SUB_LINE = 0x2 , ADD_PAGE = 0x4,
		     SUB_PAGE = 0x8 , FIRST    = 0x10, LAST	= 0x20,
		     SLIDER   = 0x40, NONE     = 0x80 };


class QScrollBar_Private : public QScrollBar
{
public:
    void	  sliderMinMax( int *, int * )		const;
    void	  metrics( int *, int *, int * )	const;

    ScrollControl pointOver( const QPoint &p )		const;

    int		  rangeValueToSliderPos( int val )	const;
    int		  sliderPosToRangeValue( int  val )	const;

    void	  action( ScrollControl control );

    void	  drawControls( uint controls, uint activeControl ) const;
    void	  drawControls( uint controls, uint activeControl,
				QPainter *p ) const;
};


#undef PRIV
#define PRIV	((QScrollBar_Private *)this)

static const int thresholdTime = 500;
static const int repeatTime	= 10;

#define HORIZONTAL	(orientation() == Horizontal)
#define VERTICAL	!HORIZONTAL
#define MOTIF_BORDER	2
#define SLIDER_MIN	9 // ### motif says 6 but that's too small


/*!
  Constructs a vertical scroll bar.

  The \e parent and \e name arguments are sent to the QWidget constructor.
*/

QScrollBar::QScrollBar( QWidget *parent, const char *name )
    : QWidget( parent, name )
{
    orient = Vertical;
    init();
}

/*!
  Constructs a scroll bar.

  The \e orientation must be QScrollBar::Vertical or QScrollBar::Horizontal.

  The \e parent and \e name arguments are sent to the QWidget constructor.
*/

QScrollBar::QScrollBar( Orientation orientation, QWidget *parent,
			const char *name )
    : QWidget( parent, name )
{
    orient = orientation;
    init();
}

/*!
  Constructs a scroll bar.

  \arg \e minValue is the minimum scroll bar value.
  \arg \e maxValue is the maximum scroll bar value.
  \arg \e lineStep is the line step value.
  \arg \e pageStep is the page step value.
  \arg \e value is the initial value.
  \arg \e orientation must be QScrollBar::Vertical or QScrollBar::Horizontal.

  The \e parent and \e name arguments are sent to the QWidget constructor.
*/

QScrollBar::QScrollBar( int minValue, int maxValue, int lineStep, int pageStep,
			int value,  Orientation orientation,
			QWidget *parent, const char *name )
    : QWidget( parent, name ),
      QRangeControl( minValue, maxValue, lineStep, pageStep, value )
{
    orient = orientation;
    init();
}

void QScrollBar::init()
{
    track	     = TRUE;
    sliderPos	     = 0;
    pressedControl   = NONE;
    clickedAt	     = FALSE;
    setFocusPolicy( NoFocus );
    if ( style() == MotifStyle )
	setBackgroundMode( PaletteMid );
    else
	setBackgroundMode( PaletteBackground );
}


/*!
  Sets the scroll bar orientation.  The \e orientation must be
  QScrollBar::Vertical or QScrollBar::Horizontal.
  \sa orientation()
*/

void QScrollBar::setOrientation( Orientation orientation )
{
    orient = orientation;
    positionSliderFromValue();
    update();
}

/*!
  \fn Orientation QScrollBar::orientation() const
  Returns the scroll bar orientation; QScrollBar::Vertical or
  QScrollBar::Horizontal.
  \sa setOrientation()
*/

/*!
  \fn void QScrollBar::setTracking( bool enable )
  Enables scroll bar tracking if \e enable is TRUE, or disables tracking
  if \e enable is FALSE.

  If tracking is enabled (default), the scroll bar emits the
  valueChanged() signal whenever the slider is being dragged.  If
  tracking is disabled, the scroll bar emits the valueChanged() signal
  when the user relases the mouse button (unless the value happens to
  be the same as before).

  \sa tracking()
*/

/*!
  \fn bool QScrollBar::tracking() const
  Returns TRUE if tracking is enabled, or FALSE if tracking is disabled.

  Tracking is initially enabled.

  \sa setTracking()
*/


/*!
  Returns TRUE if the user has clicked the mouse on the slider
  and is currenly dragging it, or FALSE if not.
*/

bool QScrollBar::draggingSlider() const
{
    return pressedControl == SLIDER;
}


/*!
  Reimplements the virtual function QWidget::setPalette().

  Sets the background color to the mid color for Motif style scroll bars.
*/

void QScrollBar::setPalette( const QPalette &p )
{
    QWidget::setPalette( p );
    if ( style() == MotifStyle )
	setBackgroundMode( PaletteMid );
    else
	setBackgroundMode( PaletteBackground );
}


/*!
  Returns a size hint for this scroll bar.
*/

QSize QScrollBar::sizeHint() const
{
    QSize s( size() );
    if ( orient == Horizontal ) {
	s.setHeight( 16 );
    } else {
	s.setWidth( 16 );
    }
    return s;
}


/*!
  \internal
  Implements the virtual QRangeControl function.
*/

void QScrollBar::valueChange()
{
    int tmp = sliderPos;
    positionSliderFromValue();
    if ( tmp != sliderPos )
	PRIV->drawControls( ADD_PAGE | SLIDER | SUB_PAGE , pressedControl );
    emit valueChanged(value());
}

/*!
  \internal
  Implements the virtual QRangeControl function.
*/

void QScrollBar::stepChange()
{
    rangeChange();
}

/*!
  \internal
  Implements the virtual QRangeControl function.
*/

void QScrollBar::rangeChange()
{
    positionSliderFromValue();
    PRIV->drawControls( ADD_LINE | ADD_PAGE | SLIDER | SUB_PAGE | SUB_LINE,
			pressedControl );
}


/*!
  Handles timer events for the scroll bar.
*/

void QScrollBar::timerEvent( QTimerEvent * )
{
    if ( !isTiming )
	return;
    if ( !thresholdReached ) {
	thresholdReached = TRUE;	// control has been pressed for a time
	killTimers();			// kill the threshold time timer
	startTimer( repeatTime );	//   and start repeating
    }
    if ( clickedAt )
	PRIV->action( (ScrollControl) pressedControl );
	QApplication::syncX();
}


/*!
  Handles key press events for the scroll bar.
*/

void QScrollBar::keyPressEvent( QKeyEvent *e )
{
    switch ( e->key() ) {
    case Key_Left:
	if ( orient == Horizontal )
	    setValue( value() - lineStep() );
	break;
    case Key_Right:
	if ( orient == Horizontal )
	    setValue( value() + lineStep() );
	break;
    case Key_Up:
	if ( orient == Vertical )
	    setValue( value() - lineStep() );
	break;
    case Key_Down:
	if ( orient == Vertical )
	    setValue( value() + lineStep() );
	break;
    case Key_PageUp:
	if ( orient == Vertical )
	    setValue( value() - pageStep() );
	break;
    case Key_PageDown:
	if ( orient == Vertical )
	    setValue( value() + pageStep() );
	break;
    case Key_Home:
	setValue( minValue() );
	break;
    case Key_End:
	setValue( maxValue() );
	break;
    default:
	e->ignore();
	break;
    }
}


/*!
  Handles resize events for the scroll bar.
*/

void QScrollBar::resizeEvent( QResizeEvent * )
{
    positionSliderFromValue();
}


/*!
  Handles paint events for the scroll bar.
*/

void QScrollBar::paintEvent( QPaintEvent *event )
{
    QPainter p( this );
    if ( event )
	p.setClipRect( event->rect() );

    qDrawShadePanel( &p, rect(), colorGroup(), TRUE );
    if ( hasFocus() ) {
	if ( style() != WindowsStyle ) {
	    p.setPen( black );
	    p.drawRect(  1, 1, width() - 2, height() - 2 );
	}
    }
    PRIV->drawControls( ADD_LINE | SUB_LINE | ADD_PAGE | SUB_PAGE | SLIDER,
			pressedControl, &p );
}


static QCOORD sliderStartPos = 0;

/*!
  Handles mouse press events for the scroll bar.
*/

void QScrollBar::mousePressEvent( QMouseEvent *e )
{
    if ( !(e->button() == LeftButton ||
	   (style() == MotifStyle && e->button() == MidButton) ) )
	return;

    if ( maxValue() == minValue() ) // nothing to be done
	return;

    clickedAt	   = TRUE;
    pressedControl = PRIV->pointOver( e->pos() );

    if ( (pressedControl == ADD_PAGE ||
	  pressedControl == SUB_PAGE ||
	  pressedControl == SLIDER ) &&
	 style() == MotifStyle &&
	 e->button() == MidButton ) {
	int dummy1, dummy2, sliderLength;
	PRIV->metrics( &dummy1, &dummy2, &sliderLength );
	int newSliderPos = (HORIZONTAL ? e->pos().x() : e->pos().y())
			   - sliderLength/2;
	setValue( PRIV->sliderPosToRangeValue(newSliderPos) );
	sliderPos = newSliderPos;
	pressedControl = SLIDER;
    }

    if ( pressedControl == SLIDER ) {
	clickOffset = (QCOORD)( (HORIZONTAL ? e->pos().x() : e->pos().y())
				- sliderPos );
	slidePrevVal   = value();
	sliderStartPos = sliderPos;
	emit sliderPressed();
    } else if ( pressedControl != NONE ) {
	PRIV->drawControls( pressedControl, pressedControl );
	PRIV->action( (ScrollControl) pressedControl );
	thresholdReached = FALSE;	// wait before starting repeat
	startTimer(thresholdTime);
	isTiming = TRUE;
    }
}


/*!
  Handles mouse release events for the scroll bar.
*/

void QScrollBar::mouseReleaseEvent( QMouseEvent *e )
{
    if ( !clickedAt || !(e->button() == LeftButton ||
			 (style() == MotifStyle && e->button() == MidButton)) )
	return;
    ScrollControl tmp = (ScrollControl) pressedControl;
    clickedAt = FALSE;
    if ( isTiming )
	killTimers();
    mouseMoveEvent( e );  // Might have moved since last mouse move event.
    pressedControl = NONE;

    switch( tmp ) {
	case SLIDER: // Set value directly, we know we don't have to redraw.
	    directSetValue( calculateValueFromSlider() );
	    emit sliderReleased();
	    if ( value() != prevValue() )
		emit valueChanged( value() );
	    break;
	case ADD_LINE:
	case SUB_LINE:
	    PRIV->drawControls( tmp, pressedControl );
	    break;
	default:
	    break;
    }
}


/*!
  Handles mouse move events for the scroll bar.
*/

void QScrollBar::mouseMoveEvent( QMouseEvent *e )
{
    if ( !isVisible() ) {
	clickedAt = FALSE;
	return;
    }
    if ( !clickedAt || !(e->state() & LeftButton ||
			 ((e->state() & MidButton) && style() == MotifStyle)) )
	return;
    int newSliderPos;
    if ( pressedControl == SLIDER ) {
	int sliderMin, sliderMax;
	PRIV->sliderMinMax( &sliderMin, &sliderMax );
	QRect r = rect();
	if ( orientation() == Horizontal )
	    r.setRect( r.x() - 20, r.y() - 40, r.width() + 40, r.height() + 80 );
	else
	    r.setRect( r.x() - 40, r.y() - 20, r.width() + 80, r.height() + 40 );
	if ( style() == WindowsStyle && !r.contains( e->pos() ) )
	    newSliderPos = sliderStartPos;
        else
	    newSliderPos = (HORIZONTAL ? e->pos().x() :
			                 e->pos().y()) -clickOffset;
	if ( newSliderPos < sliderMin )
	    newSliderPos = sliderMin;
	else if ( newSliderPos > sliderMax )
	    newSliderPos = sliderMax;
	if ( newSliderPos == sliderPos )
	    return;
	int newVal = PRIV->sliderPosToRangeValue(newSliderPos);
	if ( newVal != slidePrevVal )
	    emit sliderMoved( newVal );
	if ( track && newVal != value() ) {
	    directSetValue( newVal ); // Set directly, painting done below
	    emit valueChanged( value() );
	}
	slidePrevVal = newVal;
	sliderPos = (QCOORD)newSliderPos;
	PRIV->drawControls( ADD_PAGE | SLIDER | SUB_PAGE, pressedControl );
    }
}


/*!
  \fn int QScrollBar::sliderStart() const
  Returns the pixel position where the scroll bar slider starts.

  It is equivalent to sliderRect().y() for vertical
  scroll bars or sliderRect().x() for horizontal scroll bars.
*/

/*!
  Returns the scroll bar slider rectangle.
  \sa sliderStart()
*/

QRect QScrollBar::sliderRect() const
{
    int sliderMin, sliderMax, sliderLength;
    PRIV->metrics( &sliderMin, &sliderMax, &sliderLength );
    int b = style() == MotifStyle ? MOTIF_BORDER : 0;

    if ( HORIZONTAL )
	return QRect( sliderStart(), b,
		      sliderLength, height() - b*2 );
    else
	return QRect( b, sliderStart(),
		      width() - b*2, sliderLength );
}

void QScrollBar::positionSliderFromValue()
{
    sliderPos = (QCOORD)PRIV->rangeValueToSliderPos( value() );
}

int QScrollBar::calculateValueFromSlider() const
{
    return PRIV->sliderPosToRangeValue( sliderPos );
}


/*****************************************************************************
  QScrollBar_Private member functions
 *****************************************************************************/

void QScrollBar_Private::sliderMinMax( int *sliderMin, int *sliderMax) const
{
    int dummy;
    metrics( sliderMin, sliderMax, &dummy );
}


void QScrollBar_Private::metrics( int *sliderMin, int *sliderMax,
				  int *sliderLength ) const
{
    int buttonDim, maxLength;
    int b = style() == MotifStyle ? MOTIF_BORDER : 0;
    int length = HORIZONTAL ? width()  : height();
    int extent = HORIZONTAL ? height() : width();

    if ( length > ( extent - b*2 - 1 )*2 + b*2 + SLIDER_MIN )
	buttonDim = extent - b*2;
    else
	buttonDim = ( length - b*2 - SLIDER_MIN )/2 - 1;

    *sliderMin = b + buttonDim;
    maxLength  = length - b*2 - buttonDim*2;

    if ( maxValue() == minValue() ) {
	*sliderLength = maxLength;
    } else {
	*sliderLength = (pageStep()*maxLength)/
			(maxValue()-minValue()+pageStep());
	if ( *sliderLength < SLIDER_MIN )
	    *sliderLength = SLIDER_MIN;
	if ( *sliderLength > maxLength )
	    *sliderLength = maxLength;
    }
    *sliderMax = *sliderMin + maxLength - *sliderLength;
}


ScrollControl QScrollBar_Private::pointOver(const QPoint &p) const
{
    if ( !rect().contains( p ) )
	return NONE;
    int sliderMin, sliderMax, sliderLength, pos;
    metrics( &sliderMin, &sliderMax, &sliderLength );
    pos = HORIZONTAL ? p.x() : p.y();
    if ( pos < sliderMin )
	return SUB_LINE;
    if ( pos < sliderStart() )
	return SUB_PAGE;
    if ( pos < sliderStart() + sliderLength )
	return SLIDER;
    if ( pos < sliderMax + sliderLength )
	return ADD_PAGE;
    return ADD_LINE;
}


int QScrollBar_Private::rangeValueToSliderPos( int v ) const
{
    int smin, smax;
    sliderMinMax( &smin, &smax );
    if ( maxValue() == minValue() )
	return smin;
    int sliderMin=smin, sliderMax=smax;

    int r;
    if ( 16.0 * sliderMax * maxValue() > INT_MAX )
	r = (int) (((sliderMax-sliderMin)*2*(v-minValue())+1.0)/
		   ((maxValue()-minValue())*2)) + sliderMin;
    else
	r = ((sliderMax-sliderMin)*2*(v-minValue())+1)/
	    ((maxValue()-minValue())*2) + sliderMin;
    return r;
}

int QScrollBar_Private::sliderPosToRangeValue( int pos ) const
{
    int sliderMin, sliderMax;
    sliderMinMax( &sliderMin, &sliderMax );
    if ( pos <= sliderMin || sliderMax == sliderMin )
	return minValue();
    if ( pos >= sliderMax )
	return maxValue();
    int r;
    if ( 16.0 * sliderMax * maxValue() > INT_MAX )
        r = (int) ((maxValue() - minValue() + 1.0)*(pos - sliderMin)/
		   (sliderMax - sliderMin)) + minValue();
    else
	r = (maxValue() - minValue() + 1)*(pos - sliderMin)/
	    (sliderMax - sliderMin) + minValue();
    return r;
}


void QScrollBar_Private::action( ScrollControl control )
{
    switch( control ) {
	case ADD_LINE:
	    emit nextLine();
	    addLine();
	    break;
	case SUB_LINE:
	    emit prevLine();
	    subtractLine();
	    break;
	case ADD_PAGE:
	    emit nextPage();
	    addPage();
	    break;
	case SUB_PAGE:
	    emit prevPage();
	    subtractPage();
	    break;
#if defined(CHECK_RANGE)
	default:
	    warning( "QScrollBar_Private::action: (%s) internal error",
		     name( "unnamed" ) );
#endif
    }
}


void QScrollBar_Private::drawControls( uint controls,
				       uint activeControl ) const
{
    QPainter p ( this );
    drawControls( controls, activeControl, &p );
}


void QScrollBar_Private::drawControls( uint controls, uint activeControl,
				       QPainter *p ) const
{
#define ADD_LINE_ACTIVE ( activeControl == ADD_LINE )
#define SUB_LINE_ACTIVE ( activeControl == SUB_LINE )
    QColorGroup g  = colorGroup();

    int sliderMin, sliderMax, sliderLength;
    metrics( &sliderMin, &sliderMax, &sliderLength );

    int b = style() == MotifStyle ? MOTIF_BORDER : 0;
    int dimB = sliderMin - b;
    QRect addB;
    QRect subB;
    QRect addPageR;
    QRect subPageR;
    QRect sliderR;
    int addX, addY, subX, subY;
    int length = HORIZONTAL ? width()  : height();
    int extent = HORIZONTAL ? height() : width();

    if ( HORIZONTAL ) {
	subY = addY = ( extent - dimB ) / 2;
	subX = b;
	addX = length - dimB - b;
    } else {
	subX = addX = ( extent - dimB ) / 2;
	subY = b;
	addY = length - dimB - b;
    }

    subB.setRect( subX,subY,dimB,dimB );
    addB.setRect( addX,addY,dimB,dimB );

    int sliderEnd = sliderStart() + sliderLength;
    int sliderW = extent - b*2;
    if ( HORIZONTAL ) {
	subPageR.setRect( subB.right() + 1, b,
			  sliderStart() - subB.right() - 1 , sliderW );
	addPageR.setRect( sliderEnd, b, addX - sliderEnd, sliderW );
	sliderR .setRect( sliderStart(), b, sliderLength, sliderW );
    } else {
	subPageR.setRect( b, subB.bottom() + 1, sliderW,
			  sliderStart() - subB.bottom() - 1 );
	addPageR.setRect( b, sliderEnd, sliderW, addY - sliderEnd );
	sliderR .setRect( b, sliderStart(), sliderW, sliderLength );
    }

    if ( style() == WindowsStyle ) {
	bool maxedOut = (maxValue() == minValue());
	if ( controls & ADD_LINE ) {
	    qDrawWinPanel( p, addB.x(), addB.y(),
			   addB.width(), addB.height(), g,
			   ADD_LINE_ACTIVE );
	    qDrawArrow( p, VERTICAL ? DownArrow : RightArrow,
			WindowsStyle, ADD_LINE_ACTIVE, addB.x()+2, addB.y()+2,
			addB.width()-4, addB.height()-4, g, !maxedOut );
	}
	if ( controls & SUB_LINE ) {
	    qDrawWinPanel( p, subB.x(), subB.y(),
			   subB.width(), subB.height(), g,
			   SUB_LINE_ACTIVE );
	    qDrawArrow( p, VERTICAL ? UpArrow : LeftArrow,
			WindowsStyle, SUB_LINE_ACTIVE, subB.x()+2, subB.y()+2,
			subB.width()-4, subB.height()-4, g, !maxedOut );
	}
	p->setBrush( QBrush(white,Dense4Pattern) );
	p->setPen( NoPen );
	p->setBackgroundMode( OpaqueMode );
	if ( maxedOut ) {
	    p->drawRect( sliderR );
	} else {
	    if ( controls & SUB_PAGE )
		p->drawRect( subPageR );
	    if ( controls & ADD_PAGE )
		p->drawRect( addPageR );
	    if ( controls & SLIDER ) {
		if ( !maxedOut ) {
		    QBrush fill( g.background() );
		    qDrawWinPanel( p, sliderR.x(), sliderR.y(),
				   sliderR.width(), sliderR.height(), g,
				   FALSE, &fill );
		}
	    }
	}
	// ### perhaps this should not be able to accept focus if maxedOut?
	if ( hasFocus() && (controls & SLIDER) )
	    p->drawWinFocusRect( sliderR.x()+2, sliderR.y()+2,
				 sliderR.width()-5, sliderR.height()-5,
				 backgroundColor() );
    } else {
	if ( controls & ADD_LINE )
	    qDrawArrow( p, VERTICAL ? DownArrow : RightArrow, MotifStyle,
			ADD_LINE_ACTIVE, addB.x(), addB.y(),
			addB.width(), addB.height(), g, value()<maxValue() );
	if ( controls & SUB_LINE )
	    qDrawArrow( p, VERTICAL ? UpArrow : LeftArrow, MotifStyle,
			SUB_LINE_ACTIVE, subB.x(), subB.y(),
			subB.width(), subB.height(), g, value()>minValue() );
	if ( controls & SUB_PAGE )
	    p->fillRect( subPageR, g.mid() );
	if ( controls & ADD_PAGE )
	    p->fillRect( addPageR, g.mid() );
	if ( controls & SLIDER ) {
	    QBrush fill( g.background() );
	    qDrawShadePanel( p, sliderR, g, FALSE, 2, &fill );
	}

    }
}


#undef ADD_LINE_ACTIVE
#undef SUB_LINE_ACTIVE


void qDrawArrow( QPainter *p, ArrowType type, GUIStyle style, bool down,
		 int x, int y, int w, int h,
		 const QColorGroup & g )
{
    qDrawArrow( p, type, style, down, x, y, w, h, g, TRUE );
}
