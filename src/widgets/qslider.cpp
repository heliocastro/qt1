/****************************************************************************
** $Id: qslider.cpp,v 2.53.2.1 1998/08/12 16:55:15 agulbra Exp $
**
** Implementation of QSlider class
**
** Created : 961019
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

#include "qslider.h"
#include "qpainter.h"
#include "qdrawutil.h"
#include "qtimer.h"
#include "qkeycode.h"

static const int motifBorder = 2;
static const int motifLength = 30;
static const int winLength = 9; // Must be odd
static const int thresholdTime = 500;
static const int repeatTime    = 100;

static const bool funnyWindowsStyle = FALSE;

static int sliderStartVal = 0; //##### class member?


/*!
  \class QSlider qslider.h
  \brief The QSlider widget provides a vertical or horizontal slider.
  \ingroup realwidgets

  A slider is used to let the user control a value within a
  program-definable range. In contrast to a QScrollBar, the QSlider
  widget has a constant size slider and no arrow buttons.

  QSlider only offers integer ranges.

  The recommended thickness of a slider is given by sizeHint().

  Tickmarks may be added using setTickmarks().

  A slider has a default focusPolicy() of \a TabFocus.

  <img src=qslider-m.gif> <img src=qslider-w.gif>

  \sa QScrollBar QSpinBox
  <a href="guibooks.html#fowler">GUI Design Handbook: Slider</a>
*/


/*!
  Constructs a vertical slider.

  The \e parent and \e name arguments are sent to the QWidget constructor.
*/

QSlider::QSlider( QWidget *parent, const char *name )
    : QWidget( parent, name )
{
    orient = Vertical;
    init();
}

/*!
  Constructs a slider.

  The \e orientation must be QSlider::Vertical or QSlider::Horizontal.

  The \e parent and \e name arguments are sent to the QWidget constructor.
*/

QSlider::QSlider( Orientation orientation, QWidget *parent, const char *name )
    : QWidget( parent, name )
{
    orient = orientation;
    init();
}

/*!
  Constructs a slider.

  \arg \e minValue is the minimum slider value.
  \arg \e maxValue is the maximum slider value.
  \arg \e step is the page step value.
  \arg \e value is the initial value.
  \arg \e orientation must be QSlider::Vertical or QSlider::Horizontal.

  The \e parent and \e name arguments are sent to the QWidget constructor.
*/

QSlider::QSlider( int minValue, int maxValue, int pageStep,
		  int value, Orientation orientation,
		  QWidget *parent, const char *name )
    : QWidget( parent, name ),
      QRangeControl( minValue, maxValue, 1, pageStep, value )
{
    orient = orientation;
    init();
    sliderVal = value;
}


void QSlider::init()
{
    extra = 0;
    timer = 0;
    sliderPos = 0;
    sliderVal = 0;
    clickOffset = 0;
    state = Idle;
    track = TRUE;
    ticks = NoMarks;
    tickInt = 0;
    if ( style() == MotifStyle )
	setBackgroundMode( PaletteMid );
    else
	setBackgroundMode( PaletteBackground );
    setFocusPolicy( TabFocus );
    initTicks();
}


/*!
  Does what's needed when someone changes the tickmark status
*/

void QSlider::initTicks()
{
    int space = (orient == Horizontal) ? height() : width();
    if ( ticks == Both ) {
	tickOffset = ( space - thickness() ) / 2;
    } else if ( ticks == Above ) {
	tickOffset = space - thickness();
    } else {
	tickOffset = 0;
    }
}


/*!
  Enables slider tracking if \e enable is TRUE, or disables tracking
  if \e enable is FALSE.

  If tracking is enabled (default), the slider emits the
  valueChanged() signal whenever the slider is being dragged.  If
  tracking is disabled, the slider emits the valueChanged() signal
  when the user releases the mouse button (unless the value happens to
  be the same as before).

  \sa tracking()
*/

void QSlider::setTracking( bool enable )
{
    track = enable;
}


/*!
  \fn bool QSlider::tracking() const
  Returns TRUE if tracking is enabled, or FALSE if tracking is disabled.

  Tracking is initially enabled.

  \sa setTracking()
*/


/*!
  \fn void QSlider::valueChanged( int value )
  This signal is emitted when the slider value is changed, with the
  new slider value as an argument.
*/

/*!
  \fn void QSlider::sliderPressed()
  This signal is emitted when the user presses the slider with the mouse.
*/

/*!
  \fn void QSlider::sliderMoved( int value )
  This signal is emitted when the slider is dragged, with the
  new slider value as an argument.
*/

/*!
  \fn void QSlider::sliderReleased()
  This signal is emitted when the user releases the slider with the mouse.
*/

/*!
  Calculates slider position corresponding to value \a v. Does not perform
  rounding.
*/

int QSlider::positionFromValue( int v ) const
{
    int  a = available();
    int range = maxValue() - minValue();
    return range > 0 ? ( (v - minValue() ) * a ) / (range): 0;
}

/*!
  Returns the available space in which the slider can move.
*/

int QSlider::available() const
{
    int a;
    switch ( style() ) {
    case WindowsStyle:
	a = (orient == Horizontal) ? width() - winLength
	    : height() - winLength;
	break;
    default:
    case MotifStyle:
	a = (orient == Horizontal) ? width() -motifLength - 2*motifBorder
	    : height() - motifLength - 2*motifBorder;
	break;
    }
    return a;
}

/*!
  Calculates value corresponding to slider position \a p. Performs rounding.
*/

int QSlider::valueFromPosition( int p ) const
{
    int a = available();
    int range = maxValue() - minValue();
    return   minValue() + ( a > 0 ? (2 * p * range + a ) / ( 2*a ): 0 );
}

/*!
  Implements the virtual QRangeControl function.
*/

void QSlider::rangeChange()
{
    int newPos = positionFromValue( value() );
    if ( newPos != sliderPos ) {
	reallyMoveSlider( newPos );
    }
}

/*!
  Implements the virtual QRangeControl function.
*/

void QSlider::valueChange()
{
    if ( sliderVal != value() ) {
	int newPos = positionFromValue( value() );
	sliderVal = value();
	reallyMoveSlider( newPos );
    }
    emit valueChanged(value());
}


/*!
  Handles resize events for the slider.
*/

void QSlider::resizeEvent( QResizeEvent * )
{
    rangeChange();
    initTicks();
}


/*!
  Reimplements the virtual function QWidget::setPalette().

  Sets the background color to the mid color for Motif style sliders.
*/

void QSlider::setPalette( const QPalette &p )
{
    if ( style() == MotifStyle )
	setBackgroundMode( PaletteMid );
    else
	setBackgroundMode( PaletteBackground );
    QWidget::setPalette( p );
}



/*!
  Sets the slider orientation.  The \e orientation must be
  QSlider::Vertical or QSlider::Horizontal.
  \sa orientation()
*/

void QSlider::setOrientation( Orientation orientation )
{
    orient = orientation;
    rangeChange();
    repaint();	//slightly inefficient...
}


/*!
  \fn Orientation QSlider::orientation() const
  Returns the slider orientation; QSlider::Vertical or
  QSlider::Horizontal.
  \sa setOrientation()
*/


/*!
  Returns the slider handle rectangle. (The actual moving-around thing.)
*/

QRect QSlider::sliderRect() const
{
    QRect r;
    switch ( style() ) {
    case WindowsStyle:
	if (orient == Horizontal )
	    r.setRect( sliderPos, tickOffset,
		       winLength, thickness()  );
	else
	    r.setRect ( tickOffset, sliderPos,
			thickness(), winLength  );
	break;
    default:
    case MotifStyle:
	if (orient == Horizontal )
	    r.setRect ( sliderPos + motifBorder, tickOffset + motifBorder,
			motifLength, thickness() - 2 * motifBorder );
	else
	    r.setRect ( tickOffset + motifBorder, sliderPos + motifBorder,
			thickness() - 2 * motifBorder, motifLength );
	break;
    }
    return r;
}
enum SlDir {SlUp,SlDown,SlLeft,SlRight};

static void drawWinPointedSlider( QPainter *p,
				  const QRect r,
				  const QColorGroup &g,
				  SlDir dir)
{
    // 3333330
    // 3444410
    // 3422210
    // 3422210
    // 3422210
    // 3422210
    //  34210
    //   340
    //    0

    const QColor c0 = black;
    const QColor c1 = g.dark();
    const QColor c2 = g.background();
    const QColor c3 = g.midlight();
    const QColor c4 = g.light();

    int x1 = r.left();
    int x2 = r.right();
    int y1 = r.top();
    int y2 = r.bottom();


    QBrush oldBrush = p->brush();
    p->setBrush( c2 );
    p->setPen( NoPen );
    p->drawRect( r );
    p->setBrush( oldBrush );


    switch ( dir ) {
    case SlUp:
	y1 = y1 + r.width()/2;
	break;
    case SlDown:
	y2 = y2 - r.width()/2;
	break;
    case SlLeft:
	x1 = x1 + r.height()/2;
	break;
    case SlRight:
	x2 = x2 - r.height()/2;
	break;
    }

    if ( dir != SlUp ) {
	p->setPen( c4 );
	p->drawLine( x1, y1, x2, y1 );
	p->setPen( c3 );
	p->drawLine( x1, y1+1, x2, y1+1 );
    }
    if ( dir != SlLeft ) {
	p->setPen( c3 );
	p->drawLine( x1+1, y1+1, x1+1, y2 );
	p->setPen( c4 );
	p->drawLine( x1, y1, x1, y2 );
    }
    if ( dir != SlRight ) {
	p->setPen( c0 );
	p->drawLine( x2, y1, x2, y2 );
	p->setPen( c1 );
	p->drawLine( x2-1, y1+1, x2-1, y2-1 );
    }
    if ( dir != SlDown ) {
	p->setPen( c0 );
	p->drawLine( x1, y2, x2, y2 );
	p->setPen( c1 );
	p->drawLine( x1+1, y2-1, x2-1, y2-1 );
    }

    int d;
    switch ( dir ) {
	case SlUp:
	    p->setPen( c4 );
	    d =  (r.width() + 1) / 2 - 1;
	    p->drawLine( x1, y1, x1+d, y1-d);
	    p->setPen( c0 );
	    d = r.width() - d - 1;
	    p->drawLine( x2, y1, x2-d, y1-d);
	    p->setPen( c1 );
	    d--;
	    p->drawLine( x2-1, y1, x2-1-d, y1-d);
	    break;
	case SlDown:
	    p->setPen( c4 );
	    d =  (r.width() + 1) / 2 - 1;
	    p->drawLine( x1, y2, x1+d, y2+d);
	    p->setPen( c0 );
	    d = r.width() - d - 1;
	    p->drawLine( x2, y2, x2-d, y2+d);
	    p->setPen( c1 );
	    d--;
	    p->drawLine( x2-1, y2, x2-1-d, y2+d);
	    break;
	case SlLeft:
	    p->setPen( c4 );
	    d =  (r.height() + 1) / 2 - 1;
	    p->drawLine( x1, y1, x1-d, y1+d);
	    p->setPen( c0 );
	    d = r.height() - d - 1;
	    p->drawLine( x1, y2, x1-d, y2-d);
	    p->setPen( c1 );
	    d--;
	    p->drawLine( x1, y2-1, x1-d, y2-1-d);
	    break;
	case SlRight:
	    p->setPen( c4 );
	    d =  (r.height() + 1) / 2 - 1;
	    p->drawLine( x2, y1, x2+d, y1+d);
	    p->setPen( c0 );
	    d = r.height() - d - 1;
	    p->drawLine( x2, y2, x2+d, y2-d);
	    p->setPen( c1 );
	    d--;
	    p->drawLine( x2, y2-1, x2+d, y2-1-d);
	    break;
    }

}


/*!
  Paints the slider button using painter \a p with size and
  position given by \a r. Reimplement this function to change the
  look of the slider button.
*/

void QSlider::paintSlider( QPainter *p, const QRect &r )
{
    QColorGroup g = colorGroup();
    QBrush fill( g.background() );

    switch ( style() ) {
    case WindowsStyle:
	if ( ticks == NoMarks || ticks == Both ) {
	    qDrawWinButton( p, r, g, FALSE, &fill );
	} else {
	    SlDir d = ( orient == Horizontal ) ?
		      (ticks == Above) ? SlUp : SlDown
		    : (ticks == Left) ? SlLeft : SlRight;
	    drawWinPointedSlider( p, r, g, d );
	}
	break;
    default:
    case MotifStyle:
	qDrawShadePanel( p, r, g, FALSE, 2, &fill );
	if ( orient == Horizontal ) {
	    QCOORD mid = ( r.left() + r.right() + 1) / 2;
	    qDrawShadeLine( p, mid,  r.top(), mid,  r.bottom() - 1,
			    g, TRUE, 1);
	} else {
	    QCOORD mid = ( r.top() + r.bottom() + 1) / 2;
	    qDrawShadeLine( p, r.left(), mid,  r.right() - 1, mid,
			    g, TRUE, 1);
	}
	break;
    }
}

/*!
  Performs the actual moving of the slider.
*/

void QSlider::reallyMoveSlider( int newPos )
{
    QRect oldR = sliderRect();
    sliderPos = newPos;
    QRect newR = sliderRect();
    //since sliderRect isn't virtual, I know that oldR and newR
    // are the same size.
    if ( orient == Horizontal ) {
	if ( oldR.left() < newR.left() )
	    oldR.setRight( QMIN ( oldR.right(), newR.left()));
	else           //oldR.right() >= newR.right()
	    oldR.setLeft( QMAX ( oldR.left(), newR.right()));
    } else {
	if ( oldR.top() < newR.top() )
	    oldR.setBottom( QMIN ( oldR.bottom(), newR.top()));
	else           //oldR.bottom() >= newR.bottom()
	    oldR.setTop( QMAX ( oldR.top(), newR.bottom()));
    }
    repaint( oldR );
    repaint( newR, FALSE );
}


/*!
  Draws the "groove" on which the slider moves, using the painter \a p.
  \a c gives the distance from the top (or left) edge of the widget to
  the center of the groove.
*/

void QSlider::drawWinGroove( QPainter *p, QCOORD c )
{
    if ( orient == Horizontal ) {
	qDrawWinPanel( p, 0, c - 2,  width(), 4, colorGroup(), TRUE );
	p->setPen( black );
	p->drawLine( 1, c - 1, width() - 3, c - 1 );
    } else {
	qDrawWinPanel( p, c - 2, 0, 4, height(), colorGroup(), TRUE );
	p->setPen( black );
	p->drawLine( c - 1, 1, c - 1, height() - 3 );
    }
}


/*!
  Handles paint events for the slider.
*/

void QSlider::paintEvent( QPaintEvent *e )
{

    QPainter p( this );
    QRect paintRect = e->rect();
    p.setClipRect( paintRect );
    QRect sliderR = sliderRect();
    QColorGroup g = colorGroup();
    switch ( style() ) {
    case WindowsStyle:
	if ( hasFocus() ) {
	    QRect r;
	    if ( orient == Horizontal )
		r.setRect( 0, tickOffset-1, width(), thickness()+2 );
	    else
		r.setRect( tickOffset-1, 0, thickness()+2, height() );
	    r = r.intersect( rect() );
	    qDrawPlainRect( &p, r, g.background() );
	    p.drawWinFocusRect( r, backgroundColor() );
	}
	{
	    int mid = tickOffset + thickness()/2;
	    if ( ticks & Above )
		mid += winLength / 8;
	    if ( ticks & Below )
		mid -= winLength / 8;
	    drawWinGroove( &p, mid );
	}
	paintSlider( &p, sliderR );
	break;
    default:
    case MotifStyle:
	if ( orient == Horizontal ) {
	    qDrawShadePanel( &p, 0, tickOffset, width(), thickness(),
			     g, TRUE );
	    p.fillRect( 0, 0, width(), tickOffset, g.background() );
	    p.fillRect( 0, tickOffset + thickness(),
			width(), height()/*###*/, g.background() );
	} else {
	    qDrawShadePanel( &p, tickOffset, 0, thickness(), height(),
			     g, TRUE );
	    p.fillRect( 0, 0,  tickOffset, height(), g.background() );
	    p.fillRect( tickOffset + thickness(), 0,
			width()/*###*/, height(), g.background() );
	}

	if ( hasFocus() ) {
	    p.setPen( black );
	    if ( orient == Horizontal )
		p.drawRect(  1, tickOffset + 1, width() - 2, thickness() - 2 );
	    else
		p.drawRect( tickOffset + 1, 1, thickness() - 2, height() - 2 );
	}
	paintSlider( &p, sliderR );
	break;
    }


    int interval = tickInt;
    if ( interval <= 0 ) {
	interval = lineStep();
	if ( positionFromValue( interval ) - positionFromValue( 0 ) < 3 )
	    interval = pageStep();
    }
    if ( ticks & Above )
	drawTicks( &p, 0, tickOffset - 2, interval );
	
    if ( ticks & Below ) {
	int avail = (orient == Horizontal) ? height() : width();
	avail -= tickOffset + thickness();
	drawTicks( &p, tickOffset + thickness() + 1, avail - 2, interval );
    }
}


/*!
  Handles mouse press events for the slider.
*/

void QSlider::mousePressEvent( QMouseEvent *e )
{
    resetState();
    sliderStartVal = sliderVal;
    QRect r = sliderRect();

    if ( e->button() == RightButton ) {
	return;
    } else if ( r.contains( e->pos() ) ) {
	state = Dragging;
	clickOffset = (QCOORD)( goodPart( e->pos() ) - sliderPos );
	emit sliderPressed();
    } else if ( e->button() == MidButton ||
		(funnyWindowsStyle && style() == WindowsStyle) ) {
	int pos = goodPart( e->pos() );
	moveSlider( pos - slideLength() / 2 );
	state = Dragging;
	clickOffset = slideLength() / 2;
    } else if ( orient == Horizontal && e->pos().x() < r.left() //### goodPart
		|| orient == Vertical && e->pos().y() < r.top() ) {
	state = TimingDown;
        subtractPage();
	if ( !timer )
	    timer = new QTimer( this );
	connect( timer, SIGNAL(timeout()), SLOT(repeatTimeout()) );
	timer->start( thresholdTime, TRUE );
    } else if ( orient == Horizontal && e->pos().x() > r.right() //### goodPart
		|| orient == Vertical && e->pos().y() > r.bottom() ) {
	state = TimingUp;
	addPage();
	if ( !timer )
	    timer = new QTimer( this );
	connect( timer, SIGNAL(timeout()), SLOT(repeatTimeout()) );
	timer->start( thresholdTime, TRUE );
    }
}

/*!
  Handles mouse move events for the slider.
*/

void QSlider::mouseMoveEvent( QMouseEvent *e )
{
    if ( state != Dragging )
	return;

    if ( style() == WindowsStyle ) {
	QRect r = rect();
	if ( orientation() == Horizontal )
	    r.setRect( r.x() - 20, r.y() - 30,
		       r.width() + 40, r.height() + 60 );
	else
	    r.setRect( r.x() - 30, r.y() - 20,
		       r.width() + 60, r.height() + 40 );
	if ( !r.contains( e->pos() ) ) {
	    moveSlider( positionFromValue( sliderStartVal) );
	    return;
	}
    }

    int pos = goodPart( e->pos() );
    moveSlider( pos - clickOffset );
}


/*!
  Handles mouse release events for the slider.
*/

void QSlider::mouseReleaseEvent( QMouseEvent * )
{
    resetState();
}

/*!
  Handles focus in events for the slider.
*/

void QSlider::focusInEvent( QFocusEvent * )
{
    repaint( FALSE );
}

/*!
  Moves the left (or top) edge of the slider to position
  \a pos. Performs snapping.
*/

void QSlider::moveSlider( int pos )
{
    int  a = available();
    int newPos = QMIN( a, QMAX( 0, pos ) );
    int newVal = valueFromPosition( newPos );
    if ( sliderVal != newVal ) {
	sliderVal = newVal;
	emit sliderMoved( sliderVal );
    }
    if ( tracking() && sliderVal != value() ) {
	directSetValue( sliderVal );
	emit valueChanged( sliderVal );
    }

    switch ( style() ) {
    case WindowsStyle:
	newPos = positionFromValue( newVal );
	break;
    default:
    case MotifStyle:
	break;
    }	

    if ( sliderPos != newPos )
	reallyMoveSlider( newPos );
}


/*!
  Resets all state information and stops my timer.
*/

void QSlider::resetState()
{
    if ( timer ) {
	timer->stop();
	timer->disconnect();
    }
    switch ( state ) {
    case TimingUp:
    case TimingDown:
	break;
    case Dragging: {
	setValue( valueFromPosition( sliderPos ) );
	emit sliderReleased();
	break;
    }
    case Idle:
	break;
    default:
	warning("QSlider: (%s) in wrong state", name( "unnamed" ) );
    }
    state = Idle;
}


/*!
  Handles key press events for the slider.
*/

void QSlider::keyPressEvent( QKeyEvent *e )
{
    bool sloppy = ( style() == MotifStyle );
    switch ( e->key() ) {
    case Key_Left:
	if ( sloppy || orient == Horizontal )
	    subtractLine();
	break;
    case Key_Right:
	if ( sloppy || orient == Horizontal )
	    addLine();
	break;
    case Key_Up:
	if ( sloppy || orient == Vertical )
	    subtractLine();
	break;
    case Key_Down:
	if ( sloppy || orient == Vertical )	
	    addLine();
	break;
    case Key_Prior:
	subtractPage();
	break;
    case Key_Next:
	addPage();
	break;
    case Key_Home:
	setValue( minValue() );
	break;
    case Key_End:
	setValue( maxValue() );
	break;
    default:
	e->ignore();
	return;
    }
    e->accept();
}


/*!
  Returns the length of the slider.
*/

int QSlider::slideLength() const
{
    switch ( style() ) {
    case WindowsStyle:
	return winLength;
    default:
    case MotifStyle:
	return motifLength;
    }
}


/*!
  Makes QRangeControl::setValue() available as a slot.
*/

void QSlider::setValue( int value )
{
    QRangeControl::setValue( value );
}


/*!
  Moves the slider one pageStep() upwards.
*/

void QSlider::addStep()
{
    addPage();
}


/*!
  Moves the slider one pageStep() downwards.
*/

void QSlider::subtractStep()
{
    subtractPage();
}


/*!
  Waits for autorepeat.
*/

void QSlider::repeatTimeout()
{
    ASSERT( timer );
    timer->disconnect();
    if ( state == TimingDown )
	connect( timer, SIGNAL(timeout()), SLOT(subtractStep()) );
    else if ( state == TimingUp )
	connect( timer, SIGNAL(timeout()), SLOT(addStep()) );
    timer->start( repeatTime, FALSE );
}


/*!
  Returns the relevant dimension of \a p.
*/

int QSlider::goodPart( const QPoint &p ) const
{
    return (orient == Horizontal) ?  p.x() : p.y();
}

/*!
  Returns the recommended size of the slider. Only the thickness is
  relevant.
*/

QSize QSlider::sizeHint() const
{
    const int length = 84;
    int thick = 16;
    const int tickSpace = 5;

    if ( ticks & Above )
	thick += tickSpace;
    if ( ticks & Below )
	thick += tickSpace;
    if ( style() == WindowsStyle && ticks != Both && ticks != NoMarks )
	thick += winLength / 4;	    // pointed slider
    if ( orient == Horizontal )
	return QSize( length, thick );
    else
	return QSize( thick, length );
}


/*!
  Returns the number of pixels to use for the business part of the
  slider (i.e. the non-tickmark portion). The remaining space is shared
  equally between the tickmark regions. This function and  sizeHint()
  are closely related; if you change one, you almost certainly
  have to change the other.
*/

int QSlider::thickness() const
{
    int space = (orient == Horizontal) ? height() : width();
    int n = 0;
    if ( ticks & Above )
	n++;
    if ( ticks & Below )
	n++;
    if ( !n )
	return space;

    int thick = 6;	// Magic constant to get 5 + 16 + 5
    if ( style() == WindowsStyle && ticks != Both && ticks != NoMarks ) {
	thick += winLength / 4;
    }
    space -= thick;
    //### the two sides may be unequal in size
    if ( space > 0 )
	thick += ( space * 2 ) / ( n + 2 );
    return thick;
}


/*!
  Using \a p, draws tickmarks at a distance of \a d from the edge
  of the widget, using \a w pixels and with an interval of \a i.
*/

void QSlider::drawTicks( QPainter *p, int d, int w, int i ) const
{
    p->setPen( colorGroup().foreground() );
    int v = minValue();
    int fudge = slideLength() / 2 + 1;
    while ( v <= maxValue() + 1 ) {
	int pos = positionFromValue( v ) + fudge;
	if ( orient == Horizontal )
	    p->drawLine( pos, d, pos, d + w );
	else
	    p->drawLine( d, pos, d + w, pos );
	v += i;
    }
}


/*!
  Sets the way tickmarks are displayed by the slider. \a s can take
  the following values:
  <ul>
  <li> \c NoMarks
  <li> \c Above
  <li> \c Left
  <li> \c Below
  <li> \c Right
  <li> \c Both
  </ul>
  The initial value is \c NoMarks.
  \sa tickmarks(), setTickInterval()
*/

void QSlider::setTickmarks( TickSetting s )
{
    ticks = s;
    initTicks();
    update();
}


/*!
  \fn QSlider::TickSetting QSlider::tickmarks() const

  Returns the tickmark settings for this slider.

  \sa setTickmarks()
  */

/*!
  \fn QSlider::TickSetting tickmarks() const
  Returns the way tickmarks are displayed by the slider.
  \sa setTickmarks()
*/

/*!
  Sets the interval between tickmarks to \a i. This is a value interval,
  not a pixel interval. If \a i is 0, the slider
  will choose between lineStep() and pageStep(). The initial value of
  tickInterval() is 0.
  \sa tickInterval(), QRangeControl::lineStep(), QRangeControl::pageStep()
*/

void QSlider::setTickInterval( int i )
{
    tickInt = QMAX( 0, i );
    update();
}


/*!
  \fn int QSlider::tickInterval() const
  Returns the interval between tickmarks. Returns 0 if the slider
  chooses between pageStep() and lineStep().
  \sa setTickInterval()
*/
