/****************************************************************************
** $Id: qsplitter.cpp,v 1.25.2.1 1999/01/05 15:49:31 paul Exp $
**
**  Splitter widget
**
**  Created:  980105
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
#include "qsplitter.h"

#include "qpainter.h"
#include "qdrawutil.h"
#include "qbitmap.h"


//#############################################

#define split_width 32
#define split_height 32
static unsigned char split_bits[] = {
 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x02, 0x00, 0x00, 0x40, 0x02, 0x00,
 0x00, 0x40, 0x02, 0x00, 0x00, 0x40, 0x02, 0x00, 0x00, 0x40, 0x02, 0x00,
 0x00, 0x41, 0x82, 0x00, 0x80, 0x41, 0x82, 0x01, 0xc0, 0x7f, 0xfe, 0x03,
 0x80, 0x41, 0x82, 0x01, 0x00, 0x41, 0x82, 0x00, 0x00, 0x40, 0x02, 0x00,
 0x00, 0x40, 0x02, 0x00, 0x00, 0x40, 0x02, 0x00, 0x00, 0x40, 0x02, 0x00,
 0x00, 0x40, 0x02, 0x00, 0x00, 0x40, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00,
 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

#define splitm_width 32
#define splitm_height 32
static unsigned char splitm_bits[] = {
 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
 0x00, 0xe0, 0x07, 0x00, 0x00, 0xe0, 0x07, 0x00, 0x00, 0xe0, 0x07, 0x00,
 0x00, 0xe0, 0x07, 0x00, 0x00, 0xe2, 0x47, 0x00, 0x00, 0xe3, 0xc7, 0x00,
 0x80, 0xe3, 0xc7, 0x01, 0xc0, 0xff, 0xff, 0x03, 0xe0, 0xff, 0xff, 0x07,
 0xc0, 0xff, 0xff, 0x03, 0x80, 0xe3, 0xc7, 0x01, 0x00, 0xe3, 0xc7, 0x00,
 0x00, 0xe2, 0x47, 0x00, 0x00, 0xe0, 0x07, 0x00, 0x00, 0xe0, 0x07, 0x00,
 0x00, 0xe0, 0x07, 0x00, 0x00, 0xe0, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00,
 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

#define vsplit_width 32
#define vsplit_height 32
static unsigned char vsplit_bits[] = {
 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
 0x00, 0x80, 0x00, 0x00, 0x00, 0xc0, 0x01, 0x00, 0x00, 0xe0, 0x03, 0x00,
 0x00, 0x80, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00,
 0x00, 0x80, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0xff, 0x7f, 0x00,
 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x7f, 0x00,
 0x00, 0x80, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00,
 0x00, 0x80, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0xe0, 0x03, 0x00,
 0x00, 0xc0, 0x01, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

#define vsplitm_width 32
#define vsplitm_height 32
static unsigned char vsplitm_bits[] = {
 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00,
 0x00, 0xc0, 0x01, 0x00, 0x00, 0xe0, 0x03, 0x00, 0x00, 0xf0, 0x07, 0x00,
 0x00, 0xf8, 0x0f, 0x00, 0x00, 0xc0, 0x01, 0x00, 0x00, 0xc0, 0x01, 0x00,
 0x00, 0xc0, 0x01, 0x00, 0x80, 0xff, 0xff, 0x00, 0x80, 0xff, 0xff, 0x00,
 0x80, 0xff, 0xff, 0x00, 0x80, 0xff, 0xff, 0x00, 0x80, 0xff, 0xff, 0x00,
 0x80, 0xff, 0xff, 0x00, 0x00, 0xc0, 0x01, 0x00, 0x00, 0xc0, 0x01, 0x00,
 0x00, 0xc0, 0x01, 0x00, 0x00, 0xf8, 0x0f, 0x00, 0x00, 0xf0, 0x07, 0x00,
 0x00, 0xe0, 0x03, 0x00, 0x00, 0xc0, 0x01, 0x00, 0x00, 0x80, 0x00, 0x00,
 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };


static QCursor *vSplitterCur = 0;
static QCursor *hSplitterCur = 0;



class QInternalSplitter : public QWidget
{
public:
    QInternalSplitter( QSplitter::Orientation o,
		       QSplitter *parent, const char *name=0 );
    void setOrientation( QSplitter::Orientation o );
    QSplitter::Orientation orientation() const { return orient; }

protected:
    //    void resizeEvent( QResizeEvent * );
    void paintEvent( QPaintEvent * );
    void mouseMoveEvent( QMouseEvent * );
    void mousePressEvent( QMouseEvent * );
    void mouseReleaseEvent( QMouseEvent * );


private:
    QSplitter::Orientation orient;
    QSplitter *s;
};

QInternalSplitter::QInternalSplitter( QSplitter::Orientation o,
				      QSplitter *parent, const char *name )
    : QWidget( parent, name )
{
    if ( !hSplitterCur )
	hSplitterCur = new QCursor( QBitmap( split_width, split_height,
					     split_bits, TRUE),
				 QBitmap( split_width, split_height,
					  splitm_bits, TRUE) );
    if ( !vSplitterCur )
	vSplitterCur = new QCursor( QBitmap( vsplit_width, vsplit_height,
					     vsplit_bits, TRUE),
				 QBitmap( vsplit_width, vsplit_height,
					  vsplitm_bits, TRUE) );
    orient = o;
    s = parent;
    if ( o == QSplitter::Horizontal )
	setCursor( *hSplitterCur );
    else
	setCursor( *vSplitterCur );
}

#if 0
int QSplitter::hit( QPoint pnt )
{
    //### fancy 2-dim hit for Motif...
    QCOORD p = pick(pnt);
    if ( w1 && p > pick( w1->geometry().bottomRight() ) &&
	 w2 && p < pick( w2->pos() ) )
	return 1;
    else
	return 0;
}
#endif


void QInternalSplitter::setOrientation( QSplitter::Orientation o )
{
    orient = o;
    if ( o == QSplitter::Horizontal )
	setCursor( *hSplitterCur );
    else if ( vSplitterCur )
	setCursor( *vSplitterCur );
}

void QInternalSplitter::mouseMoveEvent( QMouseEvent *e )
{
    s->moveTo( mapToParent( e->pos() ));
}
void QInternalSplitter::mousePressEvent( QMouseEvent *e )
{
    if ( e->button() == LeftButton ) {
	s->startMoving();
	s->moveTo( mapToParent( e->pos() ));
    }
}
void QInternalSplitter::mouseReleaseEvent( QMouseEvent *e )
{
    if ( e->button() == LeftButton )
	s->stopMoving();
}

void QInternalSplitter::paintEvent( QPaintEvent * )
{
    QPainter p( this );
    s->drawSplitter( &p, 0, 0, width(), height() );
}



/*!
  \class QSplitter qsplitter.h
  \brief QSplitter implements a splitter widget.

  \ingroup realwidgets

  A splitter lets the user control the size of child widgets by
  dragging the boundary between the children.

  The current implementation is limited to two children.  The two
  widgets to be managed are the first two children added.  If you
  need to split more than two widgets, you can nest splitters (although
  it may be difficult to control the relative sizing to your requirements).

  To show a QListBox and a QMultiLineEdit side by side:

  \code
    QSplitter *split = new QSplitter( parent );
    QListBox *lb = new QListBox( split );
    QMultiLineEdit *lb = new QMultiLineEdit( split );
  \endcode


  In QSplitter the boundary can be either horizontal or vertical.  The
  default is horizontal (the children are side by side) and you
  can use setOrientation( QSplitter::Vertical ) to set it to vertical.

  By default, both widgets can be as large or as small as the user
  wishes. You can naturally use setMinimumSize() and/or
  setMaximumSize() on the children. Use setResizeMode() to specify that
  a widget should keep its size when the splitter is resized.

  QSplitter normally resizes the children only at the end of a
  resize operation, but if you call setOpaqueResize( TRUE ), the
  widgets are resized as often as possible.

  <img src=qsplitter-m.gif> <img src=qsplitter-w.gif>

  \sa QTabBar
*/

static int opaqueOldPos = -1; //### there's only one mouse, but this is a bit risky

/*!
  Creates a horizontal splitter.
*/

QSplitter::QSplitter( QWidget *parent, const char *name )
    :QFrame(parent,name,WPaintUnclipped)
{
     orient = Horizontal;
     init();
}
/*!
  Creates splitter with orientation \a o.
*/

QSplitter::QSplitter( Orientation o, QWidget *parent, const char *name )
    :QFrame(parent,name,WPaintUnclipped)
{
     orient = o;
     init();
}


void QSplitter::init()
{
    //ratio = -1;
    fixedWidget = 0;
    opaque = 0;

    d = new QInternalSplitter( orient, this );

    setMouseTracking( TRUE );
    moving = 0;
    w1 = w2 = 0;
    if ( style() == WindowsStyle )
	bord = 3;
    else
	bord = 5;
}

/*!
  \fn void QSplitter::refresh()

  Updates the splitter state. You should not need to call this
  function during normal operations.
*/


/*!  Sets the orientation to \a o.  By default the orientation is
  horizontal (the two widgets are side by side).

  \sa orientation()
*/

void QSplitter::setOrientation( Orientation o )
{
    if ( orient == o )
	return;
    orient = o;
    d->setOrientation( o );
    recalc( isVisible() );
}

/*!
   \fn QSplitter::Orientation QSplitter::orientation() const

   Returns the orientation (\c Horizontal or \c Vertical) of the splitter.
   \sa setOrientation()
*/


QCOORD QSplitter::newpos() const
{
    int s = pick(contentsRect().size());
    int p0 = pick(contentsRect().topLeft());
    int s1 = w1 ? pick(w1->size()) : 1;
    int s2 = w2 ? pick(w2->size()) : 1;
    if ( fixedWidget ) {
	return fixedWidget == w1 ? s1 + p0 : s - s2 - 2*bord + p0;
    } else {
	float r = (1.0*s1) / (s1 + s2 + 2*bord);
	return (QCOORD)( s * r+0.5) +  p0;
    }
}

/*!
  Reimplemented to provide childRemoveEvent(), childInsertEvent() and
  layoutHintEvent()  without breaking binary compatibility.
*/
bool QSplitter::event( QEvent *e )
{
    switch( e->type() ) {
    case Event_ChildInserted:
	childInsertEvent( (QChildEvent*) e );
	break;
    case Event_ChildRemoved:
	childRemoveEvent( (QChildEvent*) e );
	break;
    case Event_LayoutHint:
	layoutHintEvent( e );
	break;
    default:
	return QWidget::event( e );
    }
    return TRUE;
}

void QSplitter::resizeEvent( QResizeEvent * )
{
    doResize();
}

/*!
  Tells the splitter that a child widget has been removed.
*/
void QSplitter::childRemoveEvent( QChildEvent *c )
{
    if ( c->child() == w1 ) {
	w1 = 0;
	recalc( TRUE );
    } else if ( c->child() == w2 ) {
	w2 = 0;
	recalc( TRUE );
    }
}

/*!
  Tells the splitter that a child widget has been inserted.
*/
void QSplitter::childInsertEvent( QChildEvent *c )
{
    if ( c->child() == d ||  c->child() == w1 || c->child() == w2 )
	return;

    if ( !w1  ) {
	w1 = c->child();
    } else if ( !w2 ) {
	w2 = c->child();
    }
#if defined CHECK_RANGE
    else
	warning( "QSplitter (%s): Error when inserting %s ( %s ), \n"
		 "max two child widgets currently supported",
		 name( "unnamed" ), c->child()->className(),
		 c->child()->name( "unnamed")  );
#endif
    recalc( isVisible() );
}



/*!
  Tells the splitter that a child widget has changed layout parameters
*/

void QSplitter::layoutHintEvent( QEvent * )
{
    recalc( isVisible() );
}



void QSplitter::stopMoving()
{
    moving = 0;
    if ( !opaque && opaqueOldPos >= 0 ) {
	int p = opaqueOldPos;
	setRubberband( -1 );
	moveSplitter( p );
    }
}

void QSplitter::startMoving()
{
    moving = TRUE;
}

void QSplitter::moveTo( QPoint mp )
{
    if ( moving ) {
	int p = adjustPos( pick(mp) - bord ); // measure from w1->right
	if ( opaque )
	    moveSplitter( p );
	else
	    setRubberband( p );
    } else {
    }
}

/*!
  Draws the splitter handle in the rectangle described by \a x, \a y,
  \a w, \a h using painter \a p.
*/
void QSplitter::drawSplitter( QPainter *p, QCOORD x, QCOORD y, QCOORD w, QCOORD h )
{
    static const int motifOffset = 10;
    if ( style() == WindowsStyle ) {
	qDrawWinPanel( p, x, y, w, h, colorGroup() );
    } else {
    	if ( orient == Horizontal ) {
	    QCOORD xPos = x + w/2;
	    QCOORD kPos = motifOffset;
	    QCOORD kSize = bord*2 - 2;

	    qDrawShadeLine( p, xPos, kPos + kSize - 1 ,
			    xPos, h, colorGroup() );
	    qDrawShadePanel( p, xPos-bord+1, kPos,
			     kSize, kSize, colorGroup() );
	    qDrawShadeLine( p, xPos, 0, xPos, kPos ,colorGroup() );
	} else {
	    QCOORD yPos = y + h/2;
	    QCOORD kPos = w - motifOffset - 2*bord;
	    QCOORD kSize = bord*2 - 2;

	    qDrawShadeLine( p, 0, yPos, kPos, yPos, colorGroup() );
	    qDrawShadePanel( p, kPos, yPos-bord+1,
			     kSize, kSize, colorGroup() );
	    qDrawShadeLine( p, kPos + kSize -1, yPos,
			    w, yPos, colorGroup() );
	}
    }
}

/*!
  Moves the left/top edge of the splitter handle as close as possible to
  \a p which is the distance from the left (or top) edge of the widget.

  Only has effect if both widgets are set.

*/
void QSplitter::moveSplitter( QCOORD p )
{
    if ( !w1 || !w2 )
	return;
    QRect r = contentsRect();
    if ( orient == Horizontal ) {
	w1->setGeometry( r.x(), r.y(), p - r.x(), r.height() );
	d->setGeometry( p, r.y(), 2*bord, r.height() );
	p += 2*bord;
	w2->setGeometry( p, r.y(), r.width() - p + r.x(), r.height() );
    } else {
	w1->setGeometry( r.x(), r.y(), r.width(), p - r.y() );
	d->setGeometry( r.x(), p, r.width(), 2*bord );
	p += 2*bord;
	w2->setGeometry( r.x(), p, r.width(), r.height() - p + r.y() );
    }
}




/*!
  Returns the legal position of the splitter closest to \a p.
*/

int QSplitter::adjustPos( int p )
{
    //    ratio = p2r( p );

    QRect r = contentsRect();

    QCOORD p0 = pick( r.topLeft() );
    QCOORD p1 = pick( r.bottomRight() );

    QCOORD min = p0 + 1; //### no zero size widgets
    min = QMAX( min, p0 + pick( w1->minimumSize() ) );
    min = QMAX( min, p1 - pick( w2->maximumSize() ) -2*bord + 1 );

    QCOORD max = p1 - 1; //### no zero size widgets
    max = QMIN( max, p1 - pick( w2->minimumSize() ) -2*bord + 1 );
    max = QMIN( max, p0 + pick( w1->maximumSize() ) );

    p = QMAX( min, QMIN( p, max ) );

    return p;
}


/*!
  Shows a rubber band at position \a p. If \a p is negative, the rubber band is removed.
*/

void QSplitter::setRubberband( int p )
{
    QPainter paint( this );
    paint.setPen( gray );
    paint.setBrush( gray );
    paint.setRasterOp( XorROP );
    QRect r = contentsRect();
    const int rBord = 3; //###

    if ( orient == Horizontal ) {
	if ( opaqueOldPos >= 0 )
	    paint.drawRect( opaqueOldPos + bord - rBord , r.y(),
			    2*rBord, r.height() );
	if ( p >= 0 )
	    paint.drawRect( p  + bord - rBord, r.y(), 2*rBord, r.height() );
    } else {
	if ( opaqueOldPos >= 0 )
	    paint.drawRect( r.x(), opaqueOldPos + bord - rBord,
			    r.width(), 2*rBord );
	if ( p >= 0 )
	    paint.drawRect( r.x(), p + bord - rBord, r.width(), 2*rBord );
    }
    opaqueOldPos = p;
}




void QSplitter::doResize()
{
    if ( !w1 || !w2 ) {
	QRect r = contentsRect();
	if ( w1 )
	    w1->setGeometry( r.x(), r.y(), r.width(), r.height() );
	else if ( w2 )
	    w2->setGeometry( r.x(), r.y(), r.width(), r.height() );
	return;
    }
    moveSplitter( adjustPos( newpos() ) );
}


void QSplitter::recalc( bool update)
{
    int fi = 2*frameWidth();
    if ( !w1 || !w2 ) {
	QRect r = contentsRect();
	QWidget *w = w1 ? w1 : w2;
	if ( w ) {
	    int ww = (int)w->maximumSize().width() + fi;
	    ww = QMIN( ww, QCOORD_MAX );
	    int hh = (int)w->maximumSize().height() + fi;
	    hh = QMIN( hh, QCOORD_MAX );
	    setMaximumSize( ww, hh );

	    QSize fs( fi, fi );
	    setMinimumSize( w->minimumSize() + fs );
	    if ( update )
		w->setGeometry( r.x(), r.y(), r.width(), r.height() );
	}
	return;
    }

    int maxl = pick(w1->maximumSize()) + pick(w2->maximumSize()) + bord*2 + fi;
    maxl = QMIN( maxl, QCOORD_MAX );
    int minl = pick(w1->minimumSize()) + pick(w2->minimumSize()) + bord*2 + fi;

    int maxt = QMIN( trans(w1->maximumSize()),trans(w2->maximumSize()) ) + fi;
    maxt = QMIN( maxt, QCOORD_MAX );
    int mint = QMAX( trans(w1->minimumSize()), trans(w2->minimumSize()) ) + fi;

    if ( maxt < mint )
	maxt = mint;


    if ( orient == Horizontal ) {
	setMaximumSize( maxl, maxt );
	setMinimumSize( minl, mint );
    } else {
	setMaximumSize( maxt, maxl );
	setMinimumSize( mint, minl );
    }
    if ( update )
	moveSplitter( adjustPos( newpos() ) );
}


/*!
  Sets resize mode of \a w to \a mode.
  \a mode can be one of:

  \define QSplitter::ResizeMode

  <ul>
    <li> \c Stretch (the default) - \a w will resize when the splitter resizes
    <li> \c KeepSize - \a w will keep its size.
  </ul>
*/

void QSplitter::setResizeMode( QWidget *w, ResizeMode mode )
{
    if ( mode == KeepSize )
	fixedWidget = w;
    else
	fixedWidget = 0;  //#### not completely correct
    doResize();
}


/*!
  \fn bool QSplitter::opaqueResize() const

  Returns TRUE if opaque resize is on, FALSE otherwise.

  \sa setOpaqueResize()
*/

/*!
  Sets opaque resize to \a on. Opaque resize is initially turned off.

  \sa opaqueResize()
*/

void QSplitter::setOpaqueResize( bool on )
{
    opaque = on;
}

QWidget * QSplitter::splitterWidget()
{
    return (QWidget*)d;
}

#if 0
/*!
  Hides \a w if \a hide is TRUE, and updates the splitter.

  \warning Due to a limitation in the current implementation,
  calling QWidget::hide() will not work.
*/

void QSplitter::setHidden( QWidget *w, bool hide )
{
    if ( w == w1 ) {
	w1show = !hide;
    } else if ( w == w2 ) {
	w2show = !hide;
    } else {
#ifdef CHECK_RANGE
	warning( "QSplitter::setHidden(), unknown widget" );
#endif	
	return;
    }
    if ( hide )
	w->hide();
    else
	w->show();
    recalc( TRUE );
}


/*!
  Returns the hidden status of \a w
*/

bool QSplitter::isHidden( QWidget *w ) const
{
    if ( w == w1 )
	return !w1show;
     else if ( w == w2 )
	return !w2show;
#ifdef CHECK_RANGE
    else
	warning( "QSplitter::isHidden(), unknown widget" );
#endif	
    return FALSE;
}
#endif

/*!
  Moves \a w to the leftmost/top position.
*/

void QSplitter::moveToFirst( QWidget *w )
{
    if ( !w || w1 == w )
	return;
    if ( !w1 ) {
	w1 = w;
	if ( w2 == w1 )
	    w2 = 0;
    } else if ( w == w2 ) {
	w2 = w1;
	w1 = w;
    } else if ( !w2 ) {
	w2 = w1;
	w1 = w;
    } else {
	warning( "QSplitter (%s)::toFirst  %s ( %s ), \n"
		 "max two widgets currently supported",
		 name( "unnamed" ), w->className(),
		 w->name( "unnamed")  );
    }
	
    recalc( isVisible() );

}

/*!
  Moves \a w to the rightmost/bottom position.
*/

void QSplitter::moveToLast( QWidget *w )
{
    if ( !w || w2 == w )
	return;
    if ( !w2 ) {
	w2 = w;
	if ( w2 == w1 )
	    w1 = 0;
    } else if ( w == w1 ) {
	w1 = w2;
	w2 = w;
    } else if ( !w2 ) {
	w1 = w2;
	w2 = w;
    } else {
	warning( "QSplitter (%s)::toLast  %s ( %s ), \n"
		 "max two widgets currently supported",
		 name( "unnamed" ), w->className(),
		 w->name( "unnamed")  );
    }
	
    recalc( isVisible() );
}
