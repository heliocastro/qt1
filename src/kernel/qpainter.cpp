/****************************************************************************
** $Id: qpainter.cpp,v 2.54.2.5 1998/12/15 15:42:31 hanord Exp $
**
** Implementation of QPainter, QPen and QBrush classes
**
** Created : 940112
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

#include "qpainter.h"
#include "qpaintdevicedefs.h"
#include "qbitmap.h"
#include "qstack.h"
#include "qdatastream.h"
#include "qwidget.h"
#include "qimage.h"
#include <stdlib.h>

/*!
  \class QPainter qpainter.h
  \brief The QPainter class paints on paint devices.

  \ingroup drawing

  The painter provides efficient graphics rendering on any QPaintDevice
  object. QPainter can draw everything from simple lines to complex shapes
  like pies and chords. It can also draw aligned text and pixmaps.

  Graphics can be transformed using view transformation, world
  transformation or a combination of these two.	 View transformation
  is a window/viewport transformation with translation and
  scaling. World transformation is a full 2D transformation including
  rotation and shearing.

  The typical use of a painter is:
  <ol>
  <li> Construct a painter.
  <li> Set a pen, a brush etc.
  <li> Draw.
  <li> Destroy the painter.
  </ol>

  This example uses a convenience constructor that calls begin(), and
  relies on the destructor to call end():

  \code
    void MyWidget::paintEvent()
    {
	QPainter paint( this );			// start painting widget
	paint.setPen( blue );			// set blue pen
	paint.drawText( rect(),			// draw a text, centered
			AlignCenter,		//   in the widget
			"The Text" );
    }
  \endcode

  You can also use the begin() and end() functions to begin and end
  painting explicitly:

  \code
    void MyWidget::paintEvent()
    {
	QPainter paint;
	paint.begin( this );			// start painting widget
	paint.setPen( blue );			// set blue pen
	paint.drawText( rect(),			// draw a text, centered
			AlignCenter,		//   in the widget
			"The Text" );
	paint.end();				// painting done
    }
  \endcode

  This is useful since it is not possible to have two painters active
  on the same paint device at a time.

  QPainter is almost never used outside \link QWidget::paintEvent()
  paintEvent()\endlink.  Any widget <em>must</em> be able to repaint
  itself at any time via paintEvent(), therefore it's almost always
  best to design the widget so that it does all the painting in
  paintEvent() and use either QWidget::update() or QWidget::repaint()
  force a paint event as necessary.

  Note that both painters and some paint devices have attributes such
  as current font, current foreground colors and so on.

  QPainter::begin() copies these attributes from the paint device, and
  changing a paint device's attributes will have effect only the next
  time a painter is opened on it.

  \warning QPainter::begin() resets all attributes to their default
  values, from the device, thus setting fonts, brushes, etc, before
  begin() will have \e no effect.

  \header qdrawutil.h

  \sa QPaintDevice, QWidget, QPixmap
*/


/*!
  Constructs a painter.

  Notice that all painter settings (setPen,setBrush etc.) are reset to
  default values when begin() is called.

  \sa begin(), end()
*/

QPainter::QPainter()
{
    init();
}


/*!
  Constructs a painter that begins painting the paint device \a pd
  immediately.

  This constructor is convenient for short-lived painters, e.g. in
  a \link QWidget::paintEvent() paint event\endlink and should be
  used only once. The constructor calls begin() for you and the QPainter
  destructor automatically calls end().

  Example using begin() and end():
  \code
    void MyWidget::paintEvent( QPaintEvent * )
    {
	QPainter p( this );
	p.drawLine( ... );	// drawing code
    }
  \endcode

  Example using this constructor:
  \code
    void MyWidget::paintEvent( QPaintEvent * )
    {
	QPainter p( this );
	p.drawLine( ... );	// drawing code
    }
  \endcode

  \sa begin(), end()
*/

QPainter::QPainter( const QPaintDevice *pd )
{
    init();
    begin( pd );
    flags |= CtorBegin;
}


/*!
  Constructs a painter that begins painting the paint device \a pd
  immediately, with the default arguments taken from \a copyAttributes.

  \sa begin()
*/

QPainter::QPainter( const QPaintDevice *pd,
		    const QWidget *copyAttributes )
{
    init();
    begin( pd, copyAttributes );
    flags |= CtorBegin;
}


/*!
  Destroys the painter.

  If you called begin() but not end(), the destructor outputs a warning
  message.  Note that there is no need to call end() if you used one of
  the constructors which takes a paint device argument.
*/

QPainter::~QPainter()
{
    if ( isActive() ) {
	if ( (flags & CtorBegin) == 0 ) {
#if defined(CHECK_STATE)
	    warning( "QPainter: You called begin() but not end()" );
#endif
	}
	end();
    }
    if ( tabarray )				// delete tab array
	delete [] tabarray;
    if ( ps_stack )
	killPStack();
}


/*!
  \overload bool QPainter::begin( const QPaintDevice *pd, const QWidget *copyAttributes )

  This version opens the painter on a paint device \a pd and sets the initial
  pen, background color and font from \a copyAttributes.  This is equivalent
  with:
  \code
    QPainter p;
    p.begin( pd );
    p.setPen( copyAttributes->foregroundColor() );
    p.setBackgroundColor( copyAttributes->backgroundColor() );
    p.setFont( copyAttributes->font() );
  \endcode

  This begin function is convenient for double buffering.  When you
  draw in a pixmap instead of directly in a widget (to later bitBlt
  the pixmap into the widget) you will need to set the widgets's
  font etc.  This function does exactly that.

  Example:
  \code
    void MyWidget::paintEvent( QPaintEvent * )
    {
	QPixmap pm(rect());
	QPainter p;
	p.begin(&pm, this);
	// ... potential flickering paint operation ...
	p.end();
	bitBlt(this, 0, 0, &pm);
    }
  \endcode

  \sa end()
*/

bool QPainter::begin( const QPaintDevice *pd, const QWidget *copyAttributes )
{
    if ( pd == 0 ) {
#if defined(CHECK_NULL)
	warning( "QPainter::begin: The widget to copy attributes from cannot "
		 "be null" );
#endif
	return FALSE;
    }
    if ( begin(pd) ) {
	setPen( copyAttributes->foregroundColor() );
	setBackgroundColor( copyAttributes->backgroundColor() );
	setFont( copyAttributes->font() );
	return TRUE;
    }
    return FALSE;
}


/*!
  \internal
  Sets or clears a pointer flag.
*/

void QPainter::setf( ushort b, bool v )
{
    if ( v )
	setf( b );
    else
	clearf( b );
}


/*!
  \fn bool QPainter::isActive() const
  Returns the TRUE if the painter is active painting, i.e. begin() has
  been called and end() has not yet been called.
  \sa QPaintDevice::paintingActive()
*/

/*!
  \fn QPaintDevice *QPainter::device() const
  Returns the paint device currently active for this painter, or null if
  begin() has not been called.
  \sa QPaintDevice::paintingActive()
*/


struct QPState {				// painter state
    QFont	font;
    QPen	pen;
    QBrush	brush;
    QColor	bgc;
    uchar	bgm;
    uchar	pu;
    uchar	rop;
    QPoint	bro;
    QRect	wr, vr;
    QWMatrix	wm;
    bool	vxf;
    bool	wxf;
    QRegion	rgn;
    bool	clip;
    int		ts;
    int	       *ta;
};

Q_DECLARE(QStackM,QPState);
typedef QStackM(QPState) QPStateStack;


void QPainter::killPStack()
{
    delete (QPStateStack *)ps_stack;
}

/*!
  Saves the current painter state (pushes the state onto a stack).

  A save() must have a corresponding restore().

  \sa restore()
*/

void QPainter::save()
{
    if ( testf(ExtDev) ) {
	if ( testf(DirtyFont) )
	    updateFont();
	if ( testf(DirtyPen) )
	    updatePen();
	if ( testf(DirtyBrush) )
	    updateBrush();
	pdev->cmd( PDC_SAVE, this, 0 );
    }
    QPStateStack *pss = (QPStateStack *)ps_stack;
    if ( pss == 0 ) {
	pss = new QStackM(QPState);
	CHECK_PTR( pss );
	pss->setAutoDelete( TRUE );
	ps_stack = pss;
    }
    register QPState *ps = new QPState;
    CHECK_PTR( ps );
    ps->font  = cfont;
    ps->pen   = cpen;
    ps->brush = cbrush;
    ps->bgc   = bg_col;
    ps->bgm   = bg_mode;
    ps->rop   = rop;
    ps->bro   = bro;
#if 0
    ps->pu    = pu;				// !!!not used
#endif
    ps->wr    = QRect( wx, wy, ww, wh );
    ps->vr    = QRect( vx, vy, vw, vh );
    ps->wm    = wxmat;
    ps->vxf   = testf(VxF);
    ps->wxf   = testf(WxF);
    ps->rgn   = crgn;
    ps->clip  = testf(ClipOn);
    ps->ts    = tabstops;
    ps->ta    = tabarray;
    pss->push( ps );
}

/*!
  Restores the current painter state (pops a saved state off the stack).
  \sa save()
*/

void QPainter::restore()
{
    if ( testf(ExtDev) ) {
	pdev->cmd( PDC_RESTORE, this, 0 );
    }
    QPStateStack *pss = (QPStateStack *)ps_stack;
    if ( pss == 0 || pss->isEmpty() ) {
#if defined(CHECK_STATE)
	warning( "QPainter::restore: Empty stack error" );
#endif
	return;
    }
    register QPState *ps = pss->pop();
    if ( ps->font != cfont )
	setFont( ps->font );
    if ( ps->pen != cpen )
	setPen( ps->pen );
    if ( ps->brush != cbrush )
	setBrush( ps->brush );
    if ( ps->bgc != bg_col )
	setBackgroundColor( ps->bgc );
    if ( ps->bgm != bg_mode )
	setBackgroundMode( (BGMode)ps->bgm );
    if ( ps->rop != rop )
	setRasterOp( (RasterOp)ps->rop );
#if 0
    if ( ps->pu != pu )				// !!!not used
	pu = ps->pu;
#endif
    QRect wr( wx, wy, ww, wh );
    QRect vr( vx, vy, vw, vh );
    if ( ps->wr != wr )
	setWindow( ps->wr );
    if ( ps->vr != vr )
	setViewport( ps->vr );
    if ( ps->wm != wxmat )
	setWorldMatrix( ps->wm );
    if ( ps->vxf != testf(VxF) )
	setViewXForm( ps->vxf );
    if ( ps->wxf != testf(WxF) )
	setWorldXForm( ps->wxf );
    if ( ps->rgn != crgn )
	setClipRegion( ps->rgn );
    if ( ps->clip != testf(ClipOn) )
	setClipping( ps->clip );
    tabstops = ps->ts;
    tabarray = ps->ta;
    delete ps;
    if ( pss->isEmpty() ) {
	delete pss;
	ps_stack = 0;
    }
}


/*!
  \fn QFontMetrics QPainter::fontMetrics() const
  Returns the font metrics for the painter.
  Font metrics can only be obtained when the painter is active.
  \sa fontInfo(), isActive()
*/

/*!
  \fn QFontInfo QPainter::fontInfo() const
  Returns the font info for the painter.
  Font info can only be obtained when the painter is active.
  \sa fontMetrics(), isActive()
*/


/*!
  \fn const QPen &QPainter::pen() const
  Returns the current pen for the painter.
  \sa setPen()
*/

/*!
  Sets a new painter pen.

  The pen defines how to draw lines and outlines, and it also defines
  the text color.

  \sa pen()
*/

void QPainter::setPen( const QPen &pen )
{
#if defined(CHECK_STATE)
    if ( !isActive() )
	warning( "QPainter::setPen: Will be reset by begin()" );
#endif
    cpen = pen;
    updatePen();
}

/*!
  Sets a new painter pen with style \c style, width 0 and black color.
  \sa pen(), QPen
*/

void QPainter::setPen( PenStyle style )
{
#if defined(CHECK_STATE)
    if ( !isActive() )
	warning( "QPainter::setPen: Will be reset by begin()" );
#endif
    register QPen::QPenData *d = cpen.data;	// low level access
    if ( d->count != 1 ) {
	cpen.detach();
	d = cpen.data;
    }
    d->style = style;
    d->width = 0;
    d->color = black;
    updatePen();
}

/*!
  Sets a new painter pen with style \c SolidLine, width 0 and the specified
  \e color.
  \sa pen(), QPen
*/

void QPainter::setPen( const QColor &color )
{
#if defined(CHECK_STATE)
    if ( !isActive() )
	warning( "QPainter::setPen: Will be reset by begin()" );
#endif
    register QPen::QPenData *d = cpen.data;	// low level access
    if ( d->count != 1 ) {
	cpen.detach();
	d = cpen.data;
    }
    d->style = SolidLine;
    d->width = 0;
    d->color = color;
    updatePen();
}

/*!
  \fn const QBrush &QPainter::brush() const
  Returns the current painter brush.
  \sa QPainter::setBrush()
*/

/*!
  Sets a new painter brush.

  The brush defines how to fill shapes.

  \sa brush()
*/

void QPainter::setBrush( const QBrush &brush )
{
#if defined(CHECK_STATE)
    if ( !isActive() )
	warning( "QPainter::setBrush: Will be reset by begin()" );
#endif
    cbrush = brush;
    updateBrush();
}

/*!
  Sets a new painter brush with black color and the specified \e style.
  \sa brush(), QBrush
*/

void QPainter::setBrush( BrushStyle style )
{
#if defined(CHECK_STATE)
    if ( !isActive() )
	warning( "QPainter::setBrush: Will be reset by begin()" );
#endif
    register QBrush::QBrushData *d = cbrush.data; // low level access
    if ( d->count != 1 ) {
	cbrush.detach();
	d = cbrush.data;
    }
    d->style = style;
    d->color = black;
    if ( d->pixmap ) {
	delete d->pixmap;
	d->pixmap = 0;
    }
    updateBrush();
}

/*!
  Sets a new painter brush with the style \c SolidPattern and the specified
  \e color.
  \sa brush(), QBrush
*/

void QPainter::setBrush( const QColor &color )
{
#if defined(CHECK_STATE)
    if ( !isActive() )
	warning( "QPainter::setBrush: Will be reset by begin()" );
#endif
    register QBrush::QBrushData *d = cbrush.data; // low level access
    if ( d->count != 1 ) {
	cbrush.detach();
	d = cbrush.data;
    }
    d->style = SolidPattern;
    d->color = color;
    if ( d->pixmap ) {
	delete d->pixmap;
	d->pixmap = 0;
    }
    updateBrush();
}


/*!
  \fn const QColor &QPainter::backgroundColor() const
  Returns the background color currently set.
  \sa setBackgroundColor()
*/

/*!
  \fn BGMode QPainter::backgroundMode() const
  Returns the background mode currently set.
  \sa setBackgroundMode()
*/

/*!
  \fn RasterOp QPainter::rasterOp() const
  Returns the raster operation currently set.
  \sa setRasterOp()
*/

/*!
  \fn const QPoint &QPainter::brushOrigin() const
  Returns the brush origin currently set.
  \sa setBrushOrigin()
*/


/*!
  \fn int QPainter::tabStops() const
  Returns the tab stop setting.
  \sa setTabStops()
*/

/*!
  Set the number of pixels per tab stop to a fixed number.

  Tab stops are used when drawing formatted text with \c ExpandTabs set.
  This fixed tab stop value has lower precedence than tab array
  settings.

  \sa tabStops(), setTabArray(), drawText(), fontMetrics()
*/

void QPainter::setTabStops( int ts )
{
#if defined(CHECK_STATE)
    if ( !isActive() )
	warning( "QPainter::setTabStops: Will be reset by begin()" );
#endif
    tabstops = ts;
    if ( isActive() && testf(ExtDev) ) {	// tell extended device
	QPDevCmdParam param[1];
	param[0].ival = ts;
	pdev->cmd( PDC_SETTABSTOPS, this, param );
    }
}

/*!
  \fn int *QPainter::tabArray() const
  Returns the tab stop array currently set.
  \sa setTabArray()
*/

/*!
  Set an array containing the tab stops.

  Tab stops are used when drawing formatted text with \c ExpandTabs set.

  The last tab stop must be 0 (terminates the array).

  Notice that setting a tab array overrides any fixed tabulator stop
  that is set using setTabStops().

  \sa tabArray(), setTabStops(), drawText(), fontMetrics()
*/

void QPainter::setTabArray( int *ta )
{
#if defined(CHECK_STATE)
    if ( !isActive() )
	warning( "QPainter::setTabArray: Will be reset by begin()" );
#endif
    if ( ta != tabarray ) {
	tabarraylen = 0;
	if ( tabarray )				// Avoid purify complaint
	    delete [] tabarray;			// delete old array
	if ( ta ) {				// tabarray = copy of 'ta'
	    while ( ta[tabarraylen] )
		tabarraylen++;
	    tabarraylen++; // and 0 terminator
	    tabarray = new int[tabarraylen];	// duplicate ta
	    memcpy( tabarray, ta, sizeof(int)*tabarraylen );
	} else {
	    tabarray = 0;
	}
    }
    if ( isActive() && testf(ExtDev) ) {	// tell extended device
	QPDevCmdParam param[2];
	param[0].ival = tabarraylen;
	param[1].ivec = tabarray;
	pdev->cmd( PDC_SETTABARRAY, this, param );
    }
}


/*!
  \fn HANDLE QPainter::handle() const
  Returns the platform-dependent handle used for drawing.
*/


/*****************************************************************************
  QPainter xform settings
 *****************************************************************************/

/*!
  Enables view transformations if \e enable is TRUE, or disables view
  transformations if \e enable is FALSE.
  \sa hasViewXForm(), setWindow(), setViewport(), setWorldMatrix(),
  setWorldXForm(), xForm()
*/

void QPainter::setViewXForm( bool enable )
{
#if defined(CHECK_STATE)
    if ( !isActive() )
	warning( "QPainter::setViewXForm: Will be reset by begin()" );
#endif
    if ( !isActive() || enable == testf(VxF) )
	return;
    setf( VxF, enable );
    if ( testf(ExtDev) ) {
	QPDevCmdParam param[1];
	param[0].ival = enable;
	pdev->cmd( PDC_SETVXFORM, this, param );
    }
    updateXForm();
}

/*!
  \fn bool QPainter::hasViewXForm() const
  Returns TRUE if view transformation is enabled, otherwise FALSE.
  \sa setViewXForm(), xForm()
*/

/*!
  Returns the window rectangle.
  \sa setWindow(), setViewXForm()
*/

QRect QPainter::window() const
{
    return QRect( wx, wy, ww, wh );
}

/*!
  Sets the window rectangle view transformation for the painter and
  enables view transformation.

  The window rectangle is part of the view transformation.  The window
  specifies the logical coordinate system.

  The window and the \link setViewport() viewport\endlink are initially set
  to \e (0,0,width,height), where \e (width,height) is the pixel size of the
  paint device.

  You can use this method to normalize the coordinate system of the
  painter. The following example will draw a vertical line, from top to
  bottom, at the center of a pixmap, independent of the size of the pixmap:

  \code
      int width, height;
      ...
      QPixmap icon( width, height );
      QPainter p( icon );
      p.setWindow( 0, 0, 100, 100 );
      p.drawLine( 50, 0, 50, 100 );		// draw center line
  \endcode

  The setWindow() method is often used in conjunction with
  setViewport(), as in this example:

  \code
      QPainter p( myWidget );
      p.setWindow( 0, 0, 1000, 2000 );
      p.setViewport( 100,100, 200,200 );
      p.drawPoint( 500, 500 );			// draws pixel at (150,125)
  \endcode

  The preceding example sets up a transformation that maps the logical
  coordinates (0,0,1000,2000) into a (200,200) rectangle at (100,100).

  View transformations can be combined with world transformations.
  World transformations are applied after the view transformations.

  \sa window(), setViewport(), setViewXForm(), setWorldMatrix(),
  setWorldXForm()
*/

void QPainter::setWindow( int x, int y, int w, int h )
{
#if defined(CHECK_STATE)
    if ( !isActive() )
	warning( "QPainter::setWindow: Will be reset by begin()" );
#endif
    wx = x;
    wy = y;
    ww = w;
    wh = h;
    if ( testf(ExtDev) ) {
	QRect r( x, y, w, h );
	QPDevCmdParam param[1];
	param[0].rect = (QRect*)&r;
	pdev->cmd( PDC_SETWINDOW, this, param );
    }
    if ( testf(VxF) )
	updateXForm();
    else
	setViewXForm( TRUE );
}

/*!
  Returns the viewport rectangle.
  \sa setViewport(), setViewXForm()
*/

QRect QPainter::viewport() const		// get viewport
{
    return QRect( vx, vy, vw, vh );
}

/*!
  Sets the viewport rectangle view transformation for the painter and
  enables view transformation.

  The viewport rectangle is part of the view transformation. The viewport
  specifies the device coordinate system.

  The viewport and the \link setWindow() window\endlink are initially set
  to \e (0,0,width,height), where \e (width,height) is the pixel size of
  the paint device.

  You can use this method to normalize the coordinate system of the
  painter when drawing on a part of a paint device. The following example
  will draw a line from the top left to the bottom right corner of a page,
  excluding margins:

  \code
      QPrinter page;
      int margin, pageWidth, pageHeight;
      ...
      QPainter p( page );
      p.setViewPort( margin, margin, pageWidth - margin, pageHeight - margin );
      p.drawLine( 0, 0, pageWidth - 2*margin, pageHeight - 2*margin );
  \endcode

  The setViewPort() method is often used in conjunction with
  setWindow(), as in this example:

  \code
      QPainter p( myWidget );
      p.setWindow( 0, 0, 1000, 2000 );
      p.setViewport( 100,100, 200,200 );
      p.drawPoint( 500, 500 );			// draws pixel at (150,125)
  \endcode

  The preceding example sets up a transformation that maps the logical
  coordinates (0,0,1000,2000) into a (200,200) rectangle at (100,100).

  View transformations can be combined with world transformations.
  World transformations are applied after the view transformations.

  \sa viewport(), setWindow(), setViewXForm(), setWorldMatrix(),
  setWorldXForm(), xForm()
*/

void QPainter::setViewport( int x, int y, int w, int h )
{
#if defined(CHECK_STATE)
    if ( !isActive() )
	warning( "QPainter::setViewport: Will be reset by begin()" );
#endif
    vx = x;
    vy = y;
    vw = w;
    vh = h;
    if ( testf(ExtDev) ) {
	QRect r( x, y, w, h );
	QPDevCmdParam param[1];
	param[0].rect = (QRect*)&r;
	pdev->cmd( PDC_SETVIEWPORT, this, param );
    }
    if ( testf(VxF) )
	updateXForm();
    else
	setViewXForm( TRUE );
}

/*!
  Enables world transformations if \e enable is TRUE, or disables
  world transformations if \e enable is FALSE.

  \sa setWorldMatrix(), setWindow(), setViewport(), setViewXForm(), xForm()
*/

void QPainter::setWorldXForm( bool enable )
{
#if defined(CHECK_STATE)
    if ( !isActive() )
	warning( "QPainter::setWorldXForm: Will be reset by begin()" );
#endif
    if ( !isActive() || enable == testf(WxF) )
	return;
    setf( WxF, enable );
    if ( testf(ExtDev) ) {
	QPDevCmdParam param[1];
	param[0].ival = enable;
	pdev->cmd( PDC_SETWXFORM, this, param );
    }
    updateXForm();
}

/*!
  \fn bool QPainter::hasWorldXForm() const
  Returns TRUE if world transformation is enabled, otherwise FALSE.
  \sa setWorldXForm()
*/

/*!
  Returns the world transformation matrix.
  \sa setWorldMatrix()
*/

const QWMatrix &QPainter::worldMatrix() const
{
    return wxmat;
}

/*!
  Sets the world transformation matrix to \e m and enables world
  transformation.

  If \e combine is TRUE, then \e m is combined with the current
  transformation matrix, otherwise \e m will replace the current
  transformation matrix.

  World transformations are applied after the view transformations
  (i.e. \link setWindow window\endlink and \link setViewport viewport\endlink).

  If the matrix set is the identity matrix (\link QWMatrix::m11()
  m11\endlink and \link QWMatrix::m22() m22\endlink are 1.0 and the
  rest are 0.0), this function calls setWorldXForm(FALSE).

  The following functions can transform the coordinate system without using
  a QWMatrix:
  <ul>
  <li>translate()
  <li>scale()
  <li>shear()
  <li>rotate()
  </ul>

  They operate on the painter's \link worldMatrix() world matrix\endlink
  and are implemented like this:

  \code
    void QPainter::rotate( float a )
    {
	wxmat.rotate( a );
	setWorldMatrix( wxmat );
    }
  \endcode

  See the \link QWMatrix QWMatrix documentation\endlink for a general
  discussion on coordinate system transformations.

  \sa worldMatrix(), setWorldXForm(), setWindow(), setViewport(),
  setViewXForm(), xForm()
*/

void QPainter::setWorldMatrix( const QWMatrix &m, bool combine )
{
#if defined(CHECK_STATE)
    if ( !isActive() )
	warning( "QPainter::setWorldMatrix: Will be reset by begin()" );
#endif
    if ( combine )
	wxmat = m * wxmat;			// combines
    else
	wxmat = m;				// set new matrix
    bool identity = wxmat.m11() == 1.0F && wxmat.m22() == 1.0F &&
		    wxmat.m12() == 0.0F && wxmat.m21() == 0.0F &&
		    wxmat.dx()	== 0.0F && wxmat.dy()  == 0.0F;
    if ( testf(ExtDev) ) {
	QPDevCmdParam param[2];
	param[0].matrix = &wxmat;
	param[1].ival = combine;
	pdev->cmd( PDC_SETWMATRIX, this, param );
    }
    if ( identity )
	setWorldXForm( FALSE );
    else if ( !testf(WxF) )
	setWorldXForm( TRUE );
    else
	updateXForm();
}


/*!
  Translates the coordinate system by \e (dx,dy).

  For example, the following code draws a single vertical line 20 pixels high.
  \code
    void MyWidget::paintEvent()
    {
	QPainter paint( this );
	paint.drawLine(10,0,10,20);
	paint.translate(100.0,100.0);
	paint.drawLine(-90,-80,-90,-70);
    }
  \endcode

  \sa scale(), shear(), rotate(), resetXForm(), setWorldMatrix(), xForm()
*/

void QPainter::translate( float dx, float dy )
{
    wxmat.translate( dx, dy );
    setWorldMatrix( wxmat );
}

/*!
  Scales the coordinate system by \e (sx,sy).
  \sa translate(), shear(), rotate(), resetXForm(), setWorldMatrix(),
  xForm()
*/

void QPainter::scale( float sx, float sy )
{
    wxmat.scale( sx, sy );
    setWorldMatrix( wxmat );
}

/*!
  Shears the coordinate system \e (sh,sv).
  \sa translate(), scale(), rotate(), resetXForm(), setWorldMatrix(),
  xForm()
*/

void QPainter::shear( float sh, float sv )
{
    wxmat.shear( sv, sh );
    setWorldMatrix( wxmat );
}

/*!
  Rotates the coordinate system \e a degrees.
  \sa translate(), scale(), shear(), resetXForm(), setWorldMatrix(),
  xForm()
*/

void QPainter::rotate( float a )
{
    wxmat.rotate( a );
    setWorldMatrix( wxmat );
}

/*!
  Resets any transformations that were made using translate(), scale(),
  shear(), rotate(), setWorldMatrix(), setViewport() and setWindow()
  \sa worldMatrix(), viewPort(), window()
*/

void QPainter::resetXForm()
{
    if ( !isActive() )
	return;
    wx = wy = vx = vy = 0;			// default view origins
    ww = vw = pdev->metric( PDM_WIDTH );
    wh = vh = pdev->metric( PDM_HEIGHT );
    wxmat = QWMatrix();
    setWorldXForm( FALSE );
}


const int TxNone      = 0;			// transformation codes
const int TxTranslate = 1;			// copy in qptr_xyz.cpp
const int TxScale     = 2;
const int TxRotShear  = 3;


/*!
  \internal
  Updates an internal integer transformation matrix.
*/

void QPainter::updateXForm()
{
    QWMatrix m;
    if ( testf(VxF) ) {
	m.translate( vx, vy );
	m.scale( 1.0*vw/ww, 1.0*vh/wh );
	m.translate( -wx, -wy );
    }
    if ( testf(WxF) ) {
	if ( testf(VxF) )
	    m = wxmat * m;
	else
	    m = wxmat;
    }
    wm11 = qRound((double)m.m11()*65536.0);	// make integer matrix
    wm12 = qRound((double)m.m12()*65536.0);
    wm21 = qRound((double)m.m21()*65536.0);
    wm22 = qRound((double)m.m22()*65536.0);
    wdx	 = qRound((double)m.dx() *65536.0);
    wdy	 = qRound((double)m.dy() *65536.0);

    if ( txop >= TxScale )
	setf(DirtyFont);
    txinv = FALSE;				// no inverted matrix
    txop  = TxNone;
    if ( wm12 == 0 && wm21 == 0 && wm11 >= 0 && wm22 >= 0 ) {
	if ( wm11 == 65536 && wm22 == 65536 ) {
	    if ( wdx != 0 || wdy != 0 )
		txop = TxTranslate;
	} else {
	    txop = TxScale;
#if defined(_WS_WIN_)
	    setf(DirtyFont);
#endif
	}
    } else {
	txop = TxRotShear;
#if defined(_WS_WIN_)
	setf(DirtyFont);
#endif
    }
}


/*!
  \internal
  Updates an internal integer inverse transformation matrix.
*/

void QPainter::updateInvXForm()
{
#if defined(CHECK_STATE)
    ASSERT( txinv == FALSE );
#endif
    txinv = TRUE;				// creating inverted matrix
    bool invertible;
    QWMatrix m;
    if ( testf(VxF) ) {
	m.translate( vx, vy );
	m.scale( 1.0*vw/ww, 1.0*vh/wh );
	m.translate( -wx, -wy );
    }
    if ( testf(WxF) ) {
	if ( testf(VxF) )
	    m = wxmat * m;
	else
	    m = wxmat;
    }
    m = m.invert( &invertible );		// invert matrix
    im11 = qRound((double)m.m11()*65536.0);	// make integer matrix
    im12 = qRound((double)m.m12()*65536.0);
    im21 = qRound((double)m.m21()*65536.0);
    im22 = qRound((double)m.m22()*65536.0);
    idx	 = qRound((double)m.dx() *65536.0);
    idy	 = qRound((double)m.dy() *65536.0);
}


/*!
  \internal
  Maps a point from logical coordinates to device coordinates.
*/

void QPainter::map( int x, int y, int *rx, int *ry ) const
{
     switch ( txop ) {
	case TxNone:
	    *rx = x;  *ry = y;
	    break;
	case TxTranslate:
	    *rx = x + wdx/65536;
	    *ry = y + wdy/65536;
	    break;
	case TxScale:
	    *rx = wm11*x + wdx;
	    *rx = *rx > 0 ? (*rx + 32768)/65536 : (*rx - 32768)/65536;
	    *ry = wm22*y + wdy;
	    *ry = *ry > 0 ? (*ry + 32768)/65536 : (*ry - 32768)/65536;
	    break;
	default:
	    *rx = wm11*x + wm21*y+wdx;
	    *rx = *rx > 0 ? (*rx + 32768)/65536 : (*rx - 32768)/65536;
	    *ry = wm12*x + wm22*y+wdy;
	    *ry = *ry > 0 ? (*ry + 32768)/65536 : (*ry - 32768)/65536;
	    break;
    }
}

/*!
  \internal
  Maps a rectangle from logical coordinates to device coordinates.
  This internal function does not handle rotation and/or shear.
*/

void QPainter::map( int x, int y, int w, int h,
		    int *rx, int *ry, int *rw, int *rh ) const
{
     switch ( txop ) {
	case TxNone:
	    *rx = x;  *ry = y;
	    *rw = w;  *rh = h;
	    break;
	case TxTranslate:
	    *rx = x + wdx/65536;
	    *ry = y + wdy/65536;
	    *rw = w;  *rh = h;
	    break;
	case TxScale:
	    *rx = wm11*x + wdx;
	    *rx = *rx > 0 ? (*rx + 32768)/65536 : (*rx - 32768)/65536;
	    *ry = wm22*y + wdy;
	    *ry = *ry > 0 ? (*ry + 32768)/65536 : (*ry - 32768)/65536;
	    *rw = wm11*w;
	    *rw = *rw > 0 ? (*rw + 32768)/65536 : (*rw - 32768)/65536;
	    *rh = wm22*h;
	    *rh = *rh > 0 ? (*rh + 32768)/65536 : (*rh - 32768)/65536;
	    break;
	default:
#if defined(CHECK_STATE)
	    warning( "QPainter::map: Internal error" );
#endif
	    break;
    }
}

/*!
  \internal
  Maps a point from device coordinates to logical coordinates.
*/

void QPainter::mapInv( int x, int y, int *rx, int *ry ) const
{
#if defined(CHECK_STATE)
    if ( !txinv )
	warning( "QPainter::mapInv: Internal error" );
#endif
    *rx = im11*x + im21*y+idx;
    *rx = *rx > 0 ? (*rx + 32768)/65536 : (*rx - 32768)/65536;
    *ry = im12*x + im22*y+idy;
    *ry = *ry > 0 ? (*ry + 32768)/65536 : (*ry - 32768)/65536;
}

/*!
  \internal
  Maps a rectangle from device coordinates to logical coordinates.
  Cannot handle rotation and/or shear.
*/

void QPainter::mapInv( int x, int y, int w, int h,
		       int *rx, int *ry, int *rw, int *rh ) const
{
#if defined(CHECK_STATE)
    if ( !txinv || txop == TxRotShear )
	warning( "QPainter::mapInv: Internal error" );
#endif
    *rx = im11*x + idx;
    *rx = *rx > 0 ? (*rx + 32768)/65536 : (*rx - 32768)/65536;
    *ry = im22*y + idy;
    *ry = *ry > 0 ? (*ry + 32768)/65536 : (*ry - 32768)/65536;
    *rw = im11*w;
    *rw = *rw > 0 ? (*rw + 32768)/65536 : (*rw - 32768)/65536;
    *rh = im22*h;
    *rh = *rh > 0 ? (*rh + 32768)/65536 : (*rh - 32768)/65536;
}


/*!
  Returns the point \e pv transformed from user coordinates to device
  coordinates.

  \sa xFormDev(), QWMatrix::xForm()
*/

QPoint QPainter::xForm( const QPoint &pv ) const
{
    if ( txop == TxNone )
	return pv;
    int x=pv.x(), y=pv.y();
    map( x, y, &x, &y );
    return QPoint( x, y );
}

/*!
  Returns the rectangle \e rv transformed from user coordinates to device
  coordinates.

  If world transformation is enabled and rotation or shearing has been
  specified, then the bounding rectangle is returned.

  \sa xFormDev(), QWMatrix::xForm()
*/

QRect QPainter::xForm( const QRect &rv ) const
{
    if ( txop == TxNone )
	return rv;

    if ( txop == TxRotShear ) {			// rotation/shear
	QPointArray a( rv );
	a = xForm( a );
	return a.boundingRect();
    }

    // Just translation/scale
    int x, y, w, h;
    rv.rect( &x, &y, &w, &h );
    map( x, y, w, h, &x, &y, &w, &h );
    return QRect( x, y, w, h );
}

/*!
  Returns the point array \e av transformed from user coordinates to device
  coordinates.
  \sa xFormDev(), QWMatrix::xForm()
*/

QPointArray QPainter::xForm( const QPointArray &av ) const
{
    QPointArray a = av;
    if ( txop != TxNone ) {
	a = a.copy();
	int x, y, i;
	for ( i=0; i<(int)a.size(); i++ ) {
	    a.point( i, &x, &y );
	    map( x, y, &x, &y );
	    a.setPoint( i, x, y );
	}
    }
    return a;
}

/*!
  Returns the point array \a av transformed from user coordinates to device
  coordinates.  The \a index is the first point in the array and \a npoints
  denotes the number of points to be transformed.  If \a npoints is negative,
  all points from \a av[index] until the last point in the array are
  transformed.

  The returned point array consists of the number of points that were
  transformed.

  Example:
  \code
    QPointArray a(10);
    QPointArray b;
    b = painter.xForm(a,2,4);	// b.size() == 4
    b = painter.xForm(a,2,-1);	// b.size() == 8
  \endcode

  \sa xFormDev(), QWMatrix::xForm()
*/

QPointArray QPainter::xForm( const QPointArray &av, int index,
			     int npoints ) const
{
    int lastPoint = npoints < 0 ? av.size() : index+npoints;
    QPointArray a( lastPoint-index );
    int x, y, i=index, j=0;
    while ( i<lastPoint ) {
	av.point( i++, &x, &y );
	map( x, y, &x, &y );
	a.setPoint( j++, x, y );
    }
    return a;
}

/*!
  Returns the point \e pv transformed from device coordinates to user
  coordinates.
  \sa xForm(), QWMatrix::xForm()
*/

QPoint QPainter::xFormDev( const QPoint &pd ) const
{
    if ( txop == TxNone )
	return pd;
    if ( !txinv ) {
	QPainter *that = (QPainter*)this;	// mutable
	that->updateInvXForm();
    }
    int x=pd.x(), y=pd.y();
    mapInv( x, y, &x, &y );
    return QPoint( x, y );
}

/*!
  Returns the rectangle \e rv transformed from device coordinates to user
  coordinates.

  If world transformation is enabled and rotation or shearing is used,
  then the bounding rectangle is returned.

  \sa xForm(), QWMatrix::xForm()
*/

QRect QPainter::xFormDev( const QRect &rd ) const
{
    if ( txop == TxNone )
	return rd;
    if ( !txinv ) {
	QPainter *that = (QPainter*)this;	// mutable
	that->updateInvXForm();
    }
    if ( txop == TxRotShear ) {			// rotation/shear
	QPointArray a( rd );
	a = xFormDev( a );
	return a.boundingRect();
    }

    // Just translation/scale
    int x, y, w, h;
    rd.rect( &x, &y, &w, &h );
    mapInv( x, y, w, h, &x, &y, &w, &h );
    return QRect( x, y, w, h );
}

/*!
  Returns the point array \e av transformed from device coordinates to user
  coordinates.
  \sa xForm(), QWMatrix::xForm()
*/

QPointArray QPainter::xFormDev( const QPointArray &ad ) const
{
    QPointArray a = ad;
    if ( txop != TxNone ) {
	a = a.copy();
	int x, y, i;
	for ( i=0; i<(int)a.size(); i++ ) {
	    a.point( i, &x, &y );
	    mapInv( x, y, &x, &y );
	    a.setPoint( i, x, y );
	}
    }
    return a;
}

/*!
  Returns the point array \a ad transformed from device coordinates to user
  coordinates.  The \a index is the first point in the array and \a npoints
  denotes the number of points to be transformed.  If \a npoints is negative,
  all points from \a av[index] until the last point in the array are
  transformed.

  The returned point array consists of the number of points that were
  transformed.

  Example:
  \code
    QPointArray a(10);
    QPointArray b;
    b = painter.xFormDev(a,1,3);	// b.size() == 3
    b = painter.xFormDev(a,1,-1);	// b.size() == 9
  \endcode

  \sa xForm(), QWMatrix::xForm()
*/

QPointArray QPainter::xFormDev( const QPointArray &ad, int index,
				int npoints ) const
{
    int lastPoint = npoints < 0 ? ad.size() : index+npoints;
    QPointArray a( lastPoint-index );
    int x, y, i=index, j=0;
    while ( i<lastPoint ) {
	ad.point( i++, &x, &y );
	map( x, y, &x, &y );
	a.setPoint( j++, x, y );
    }
    return a;
}


/*!
  Fills the rectangle \e (x,y,w,h) with the \e brush.

  You can specify a QColor as \e brush, since there is a QBrush constructor
  that takes a QColor argument and creates a solid pattern brush.

  \sa drawRect()
*/

void QPainter::fillRect( int x, int y, int w, int h, const QBrush &brush )
{
    QPen   oldPen   = pen();			// save pen
    QBrush oldBrush = this->brush();		// save brush
    setPen( NoPen );
    setBrush( brush );
    drawRect( x, y, w, h );			// draw filled rect
    setBrush( oldBrush );			// restore brush
    setPen( oldPen );				// restore pen
}


/*!
  \overload void QPainter::setBrushOrigin( const QPoint &p )
*/

/*!
  \overload void QPainter::setWindow( const QRect &r )
*/


/*!
  \overload void QPainter::setViewport( const QRect &r )
*/


/*!
  \fn bool QPainter::hasClipping() const
  Returns TRUE if clipping has been set, otherwise FALSE.
  \sa setClipping()
*/

/*!
  \fn const QRegion &QPainter::clipRegion() const

  Returns the clip region currently set.  Note that the clip region is
  given in physical device coordinates and \e not subject to any
  \link setWorldMatrix() coordinate transformation\endlink.

  \sa setClipRegion(), setClipRect(), setClipping()
*/

/*!
  \fn void QPainter::setClipRect( int x, int y, int w, int h )

  Sets the clip region to \e (x,y,w,h) and enables clipping.

  Note that the clip rectangle is given in physical device coordinates and
  \e not subject to any \link setWorldMatrix() coordinate
  transformation\endlink.

  \sa setClipRegion(), clipRegion(), setClipping()
*/

/*!
  \overload void QPainter::drawPoint( const QPoint &p )
*/


/*!
  \overload void QPainter::moveTo( const QPoint &p )
*/

/*!
  \overload void QPainter::lineTo( const QPoint &p )
*/

/*!
  \overload void QPainter::drawLine( const QPoint &p1, const QPoint &p2 )
*/

/*!
  \overload void QPainter::drawRect( const QRect &r )
*/

/*!
  \overload void QPainter::drawWinFocusRect( const QRect &r )
*/

/*!
  \overload void QPainter::drawWinFocusRect( const QRect &r, const QColor &bgColor )
*/


#if !defined(_WS_X11_)
// The doc and X implementation of this functions is in qpainter_x11.cpp
void QPainter::drawWinFocusRect( int, int, int, int,
				 bool, const QColor & )
{
    // do nothing, only called from X11 specific functions
}
#endif


/*!
  \overload void QPainter::drawRoundRect( const QRect &r, int xRnd, int yRnd )
*/

/*!
  \overload void QPainter::drawEllipse( const QRect &r )
*/

/*!
  \overload void QPainter::drawArc( const QRect &r, int a, int alen )
*/

/*!
  \overload void QPainter::drawPie( const QRect &r, int a, int alen )
*/

/*!
  \overload void QPainter::drawChord( const QRect &r, int a, int alen )
*/

/*!
  \overload void QPainter::drawPixmap( const QPoint &p, const QPixmap &pm, const QRect &sr )
*/

/*!
  \overload void QPainter::drawPixmap( const QPoint &p, const QPixmap &pm )

  This version of the call draws the entire pixmap.
*/

void QPainter::drawPixmap( const QPoint &p, const QPixmap &pm )
{
    drawPixmap( p.x(), p.y(), pm, 0, 0, pm.width(), pm.height() );
}


/*!
  Draws at (\a x, \a y) the \a sw by \a sh area of pixels
  from (\a sx, \a sy) in \a image.

  This function simply converts \a image to a QPixmap and draws it.

  \sa drawPixmap() QPixmap::convertFromImage()
*/
void QPainter::drawImage( int x, int y, const QImage & image,
			    int sx, int sy, int sw, int sh )
{
    if ( !isActive() || image.isNull() )
	return;

    // right/bottom
    if ( sw < 0 )
	sw = image.width()  - sx;
    if ( sh < 0 )
	sh = image.height() - sy;

    // Sanity-check clipping
    if ( sx < 0 ) {
	x -= sx;
	sw += sx;
	sx = 0;
    }
    if ( sw + sx > image.width() )
	sw = image.width() - sx;
    if ( sy < 0 ) {
	y -= sy;
	sh += sy;
	sy = 0;
    }
    if ( sh + sy > image.height() )
	sh = image.height() - sy;

    if ( sw <= 0 || sh <= 0 )
	return;


    if ( testf(ExtDev) ) {
	QPDevCmdParam param[2];
	QPoint p(x,y);
	param[0].point = &p;
	param[1].image = &image;
#if defined(_WS_WIN_)
	if ( !pdev->cmd(PDC_DRAWIMAGE,this,param) || !hdc )
#else
	if ( !pdev->cmd(PDC_DRAWIMAGE,this,param) || !hd )
#endif
	    return;
    }

    QImage subimage;
    if ( image.rect().intersect(QRect(sx,sy,sw,sh)) == image.rect() )
	subimage = image;
    else
	subimage = image.copy(sx,sy,sw,wh);

    QPixmap pm;
    pm.convertFromImage( subimage );
    drawPixmap( x, y, pm );
}

/*!
  \overload void QPainter::drawImage( const QPoint &, const QImage &, const QRect &sr )
*/

/*!
  \overload void QPainter::drawImage( const QPoint &, const QImage & )
*/
void QPainter::drawImage( const QPoint & p, const QImage & i )
{
    drawImage(p, i, i.rect());
}


void bitBlt( QPaintDevice *dst, int dx, int dy,
	     const QImage *src, int sx, int sy, int sw, int sh,
	     int conversion_flags )
{
    QPixmap tmp;
    if ( sx == 0 && sy == 0
	&& (sw<0 || sw==src->width()) && (sh<0 || sh==src->height()) )
    {
	tmp.convertFromImage( *src, conversion_flags );
    } else {
	tmp.convertFromImage( src->copy( sx, sy, sw, sh, conversion_flags),
			      conversion_flags );
    }
    bitBlt( dst, dx, dy, &tmp );
}


/*!
  \overload void QPainter::drawTiledPixmap( const QRect &r, const QPixmap &pm, const QPoint &sp )
*/

/*!
  \overload void QPainter::drawTiledPixmap( const QRect &r, const QPixmap &pm )
*/

/*!
  \overload void QPainter::fillRect( const QRect &r, const QBrush &brush )
*/

/*!
  \fn void QPainter::eraseRect( int x, int y, int w, int h )
  Erases the area inside \e (x,y,w,h).
  Equivalent to <code>fillRect( x, y, w, h, backgroundColor() )</code>.
*/

/*!
  \overload void QPainter::eraseRect( const QRect &r )
*/

/*!
  \overload void QPainter::drawText( const QPoint &p, const char *s, int len )
*/

/*!
  \overload void QPainter::drawText( const QRect &r, int tf, const char *str, int len, QRect *br, char **i )
*/

/*!
  \overload QRect QPainter::boundingRect( const QRect &r, int tf,const char *str, int len, char **i )
*/


static inline void fix_neg_rect( int *x, int *y, int *w, int *h )
{
    if ( *w < 0 ) {
	*w = -*w;
	*x -= *w - 1;
    }
    if ( *h < 0 ) {
	*h = -*h;
	*y -= *h - 1;
    }
}
void QPainter::fix_neg_rect( int *x, int *y, int *w, int *h )
{
    ::fix_neg_rect(x,y,w,h);
}

//
// The drawText function takes two special parameters; 'internal' and 'brect'.
//
// The 'internal' parameter contains a pointer to an array of encoded
// information that keeps internal geometry data.
// If the drawText function is called repeatedly to display the same text,
// it makes sense to calculate text width and linebreaks the first time,
// and use these parameters later to print the text because we save a lot of
// CPU time.
// The 'internal' parameter will not be used if it is a null pointer.
// The 'internal' parameter will be generated if it is not null, but points
// to a null pointer, i.e. internal != 0 && *internal == 0.
// The 'internal' parameter will be used if it contains a non-null pointer.
//
// If the 'brect parameter is a non-null pointer, then the bounding rectangle
// of the text will be returned in 'brect'.
//

/*!
  Draws at most \e len characters from \e str in the rectangle \e (x,y,w,h).

  Note that the meaning of \a y is not the same for the two drawText()
  varieties.

  This function draws formatted text.  The \e tf text formatting is
  the bitwise OR of the following flags:  <ul>
  <li> \c AlignLeft aligns to the left border.
  <li> \c AlignRight aligns to the right border.
  <li> \c AlignHCenter aligns horizontally centered.
  <li> \c AlignTop aligns to the top border.
  <li> \c AlignBottom aligns to the bottom border.
  <li> \c AlignVCenter aligns vertically centered
  <li> \c AlignCenter (= \c AlignHCenter | AlignVCenter)
  <li> \c SingleLine ignores newline characters in the text.
  <li> \c DontClip never clips the text to the rectangle.
  <li> \c ExpandTabs expands tabulators.
  <li> \c ShowPrefix displays "&x" as "x" underlined.
  <li> \c WordBreak breaks the text to fit the rectangle.
  <li> \c GrayText grays out the text.
  </ul>

  Horizontal alignment defaults to AlignLeft and vertical alignment
  defaults to AlignTop.

  If several of the horizontal or several of the vertical alignment flags
  are set, the resulting alignment is undefined.

  If ExpandTabs is set and no \link setTabStops() tab stops \endlink or
  \link setTabArray() tab array \endlink have been set tabs will expand to
  the closest reasonable tab stop based on the current font. For \link
  QFont::setFixedPitch() fixed pitch\endlink (fixed width) fonts you are
  guaranteed that each tab stop will be at a multiple of eight of the
  width of the characters in the font.

  \a brect (if non-null) is set to the actual bounding rectangle of
  the output.  \a internal is, yes, internal.

  These flags are defined in qwindowdefs.h.

  \sa boundingRect()
*/

void QPainter::drawText( int x, int y, int w, int h, int tf,
			 const char *str, int len, QRect *brect,
			 char **internal )
{
    if ( !isActive() )
	return;
    if ( len < 0 )
	len = strlen( str );
    if ( len == 0 )				// empty string
	return;

    if ( testf(DirtyFont|ExtDev) ) {
	if ( testf(DirtyFont) )
	    updateFont();
	if ( testf(ExtDev) && (tf & DontPrint) == 0 ) {
	    QPDevCmdParam param[3];
	    QRect r( x, y, w, h );
	    QString newstr = str;
	    newstr.truncate( len );
	    param[0].rect = &r;
	    param[1].ival = tf;
	    param[2].str = newstr.data();
	    if ( pdev->devType() != PDT_PRINTER ) {
#if defined(_WS_WIN_)
		if ( !pdev->cmd(PDC_DRAWTEXTFRMT,this,param) || !hdc )
#else
		if ( !pdev->cmd(PDC_DRAWTEXTFRMT,this,param) || !hd )
#endif
		    return;			// QPrinter wants PDC_DRAWTEXT
	    }
	}
    }

    QFontMetrics fm = fontMetrics();		// get font metrics

    // Optimize for the trivial case where we're printing a single line
    // of short text with no shortcut underscores (&) or tabbing
    if ( internal == 0 && len <= 80 && (tf & (WordBreak|GrayText)) == 0 ) {
	bool newline  = FALSE;
	bool tabbing  = FALSE;
	bool shortcut = FALSE;
	const char *p = str;
	const char *end = &str[len];
	while ( p < end ) {
	    switch ( *p++ ) {
		case '\n':
		    newline = TRUE;
		    break;
		case '\t':
		    tabbing = TRUE;
		    break;
		case '&':
		    shortcut = TRUE;
		    break;
	    }
	}
	if ( ((tf & SingleLine) != 0 || !newline) &&
	     ((tf & ExpandTabs) == 0 || !tabbing) &&
	     ((tf & ShowPrefix) == 0 || !shortcut) ) {
	    // OK, trivial case goes here
	    int tw = fm.width(str,len);
	    int fa = fm.ascent();
	    int fh = fm.height();
	    int xp, yp;
	    tw -= fm.minLeftBearing() + fm.minRightBearing();
	    if ( (tf & AlignVCenter) == AlignVCenter )
		yp = h/2 - fh/2;
	    else if ( (tf & AlignBottom) == AlignBottom)
		yp = h - fh;
	    else
		yp = 0;
	    if ( (tf & AlignRight) == AlignRight ) {
		xp = w - tw;
	    } else if ( (tf & AlignHCenter) == AlignHCenter ) {
		xp = w/2 - tw/2;
	    } else {
		xp = 0;
	    }
	    QRect br( x+xp, y+yp, tw, fh );
	    if ( brect )
		*brect = br;
	    if ( (tf & DontPrint) != 0 )
		return;
	    QRegion save_rgn = crgn;		// save the current region
	    bool    clip_on  = testf(ClipOn);

	    if ( br.x() >= x && br.y() >= y && br.width() < w &&
		 br.height() < h )
		tf |= DontClip;			// no need to clip

	    if ( (tf & DontClip) == 0 ) {	// clip text
		QRegion new_rgn;
		QRect r( x, y, w, h );
		if ( txop == TxRotShear ) {	// world xform active
		    QPointArray a( r );		// complex region
		    a = xForm( a );
		    new_rgn = QRegion( a );
		} else {
		    r = xForm( r );
		    new_rgn = QRegion( r );
		}
		if ( clip_on )			// combine with existing region
		    new_rgn = new_rgn.intersect( crgn );
		setClipRegion( new_rgn );
	    }

	    drawText( x+xp, y+yp+fa, str, len );

	    if ( (tf & DontClip) == 0 ) {	// restore clipping
		if ( clip_on ) {		// set original region
		    setClipRegion( save_rgn );
		} else {			// clipping was off
		    crgn = save_rgn;
		    setClipping( FALSE );
		}
	    }

	    return;
	}
    }

    qt_format_text(fm, x, y, w, h, tf, str, len, brect,
		   tabstops, tabarray, tabarraylen, internal, this);
}


void qt_format_text( const QFontMetrics& fm, int x, int y, int w, int h,
		     int tf, const char *str, int len, QRect *brect,
		     int tabstops, int* tabarray, int tabarraylen,
		     char **internal, QPainter* painter )
{
    if ( w <= 0 || h <= 0 )
	fix_neg_rect( &x, &y, &w, &h );

    struct text_info {				// internal text info
	char  tag[4];				// contains "qptr"
	int   w;				// width
	int   h;				// height
	int   tf;				// flags (alignment etc.)
	int   len;				// text length
	int   maxwidth;				// max text width
	int   nlines;				// number of lines
	int   codelen;				// length of encoding
    };

    ushort codearray[200];
    int	   codelen    = 200;
    bool   code_alloc = FALSE;
    ushort *codes     = codearray;
    ushort cc;					// character code
    bool   decode     = internal && *internal;	// decode from internal data
    bool   encode     = internal && !*internal; // build internal data

    if ( len > 150 && !decode ) {		// need to alloc code array
	codelen = len + len/2;
	codes	= (ushort *)malloc( codelen*sizeof(ushort) );
	code_alloc = TRUE;
    }

    const int BEGLINE  = 0x8000;		// encoding 0x8zzz, zzz=width
    const int TABSTOP  = 0x4000;		// encoding 0x4zzz, zzz=tab pos
    const int PREFIX   = 0x2000;		// encoding 0x20zz, zz=char
    const int WIDTHBITS= 0x1fff;		// bits for width encoding
    const int MAXWIDTH = 0x1fff;		// max width value

    char *p = (char *)str;
    int nlines;					// number of lines
    int index;					// index for codes
    int begline;				// index at beginning of line
    int breakindex;				// index where to break
    int breakwidth;				// width of text at breakindex
    int maxwidth;				// maximum width of a line
    int bcwidth;				// width of break char
    int tabindex;				// tab array index
    int cw;					// character width
    int k;					// index for p
    int tw;					// text width
    short charwidth[255];			// character widths
    memset( charwidth, -1, 255*sizeof(short) );

#undef	UCHAR
#define UCHAR(x)  (uchar)(x)
#define CWIDTH(x) (charwidth[UCHAR(x)]>=0 ? charwidth[UCHAR(x)] : (charwidth[UCHAR(x)]=fm.width(x)))

    bool wordbreak  = (tf & WordBreak)	== WordBreak;
    bool expandtabs = (tf & ExpandTabs) == ExpandTabs;
    bool singleline = (tf & SingleLine) == SingleLine;
    bool showprefix = (tf & ShowPrefix) == ShowPrefix;

    int	 spacewidth = CWIDTH( (int)' ' );	// width of space char

    nlines = 0;
    index  = 1;					// first index contains BEGLINE
    begline = breakindex = breakwidth = maxwidth = bcwidth = tabindex = 0;
    k = tw = 0;

    if ( decode )				// skip encoding
	k = len;

    int localTabStops = 0;	       		// tab stops
    if ( tabstops )
	localTabStops = tabstops;
    else
	localTabStops = fm.width('x')*8;       	// default to 8 times x

    while ( k < len ) {				// convert string to codes

	if ( UCHAR(*p) > 32 ) {			// printable character
	    if ( *p == '&' && showprefix ) {
		cc = '&';			// assume ampersand
		if ( k < len-1 ) {
		    k++;
		    p++;
		    if ( *p != '&' && UCHAR(*p) > 32 )
			cc = PREFIX | UCHAR(*p);// use prefix char
		}
	    } else {
		cc = UCHAR(*p);
	    }
	    cw = CWIDTH( cc & 0xff );

	} else {				// not printable (except ' ')

	    if ( *p == 32 ) {			// the space character
		cc = ' ';
		cw = spacewidth;
	    } else if ( *p == '\n' ) {		// newline
		if ( singleline ) {
		    cc = ' ';			// convert newline to space
		    cw = spacewidth;
		} else {
		    cc = BEGLINE;
		    cw = 0;
		}
	    } else if ( *p == '\t' ) {		// TAB character
		if ( expandtabs ) {
		    cw = 0;
		    if ( tabarray ) {		// use tab array
			while ( tabindex < tabarraylen ) {
			    if ( tabarray[tabindex] > tw ) {
				cw = tabarray[tabindex] - tw;
				tabindex++;
				break;
			    }
			    tabindex++;
			}
		    }
		    if ( cw == 0 )		// use fixed tab stops
			cw = localTabStops - tw%localTabStops;
		    cc = TABSTOP | QMIN(tw+cw,MAXWIDTH);
		} else {			// convert TAB to space
		    cc = ' ';
		    cw = spacewidth;
		}
	    } else {				// ignore character
		k++;
		p++;
		continue;
	    }

	    if ( wordbreak ) {			// possible break position
		breakindex = index;
		breakwidth = tw;
		bcwidth = cw;
	    }
	}

	if ( wordbreak && breakindex > 0 && tw+cw > w ) {
	    if ( index == breakindex ) {	// break at current index
		cc = BEGLINE;
		cw = 0;
	    } else {				// break at breakindex
		codes[begline] = BEGLINE | QMIN(breakwidth,MAXWIDTH);
		maxwidth = QMAX(maxwidth,breakwidth);
		begline = breakindex;
		nlines++;
		tw -= breakwidth + bcwidth;
		breakindex = tabindex = 0;
	    }
	}

	tw += cw;				// increment text width

	if ( cc == BEGLINE ) {
	    codes[begline] = BEGLINE | QMIN(tw,MAXWIDTH);
	    maxwidth = QMAX(maxwidth,tw);
	    begline = index;
	    nlines++;
	    tw = 0;
	    breakindex = tabindex = 0;
	}
	codes[index++] = cc;
	if ( index >= codelen - 1 ) {		// grow code array
	    codelen *= 2;
	    if ( code_alloc ) {
		codes = (ushort *)realloc( codes, sizeof(ushort)*codelen );
	    } else {
		codes = (ushort *)malloc( sizeof(ushort)*codelen );
		code_alloc = TRUE;
	    }
	}
	k++;
	p++;
    }

    if ( decode ) {				// decode from internal data
	char	  *data = *internal;
	text_info *ti	= (text_info*)data;
	if ( strncmp(ti->tag,"qptr",4)!=0 || ti->w != w || ti->h != h ||
	     ti->tf != tf || ti->len != len ) {
#if defined(CHECK_STATE)
	    warning( "QPainter::drawText: Internal text info is invalid" );
#endif
	    return;
	}
	maxwidth = ti->maxwidth;		// get internal values
	nlines	 = ti->nlines;
	codelen	 = ti->codelen;
	codes	 = (ushort *)(data + sizeof(text_info));
    } else {
	codes[begline] = BEGLINE | QMIN(tw,MAXWIDTH);
	maxwidth = QMAX(maxwidth,tw);
	nlines++;
	codes[index++] = 0;
	codelen = index;
    }

    if ( encode ) {				// build internal data
	char	  *data = new char[sizeof(text_info)+codelen*sizeof(ushort)];
	text_info *ti	= (text_info*)data;
	strncpy( ti->tag, "qptr", 4 );		// set tag
	ti->w	     = w;			// save parameters
	ti->h	     = h;
	ti->tf	     = tf;
	ti->len	     = len;
	ti->maxwidth = maxwidth;
	ti->nlines   = nlines;
	ti->codelen  = codelen;
	memcpy( data+sizeof(text_info), codes, codelen*sizeof(ushort) );
	*internal = data;
    }

    int	    fascent  = fm.ascent();		// get font measurements
    int	    fheight  = fm.height();
    int	    xp, yp;
    int	    xc;					// character xp
    char    p_array[200];
    bool    p_alloc;

    if ( (tf & AlignVCenter) == AlignVCenter )	// vertically centered text
	yp = h/2 - nlines*fheight/2;
    else if ( (tf & AlignBottom) == AlignBottom)// bottom aligned
	yp = h - nlines*fheight;
    else					// top aligned
	yp = 0;
    maxwidth -= fm.minLeftBearing()+fm.minRightBearing();
    if ( (tf & AlignRight) == AlignRight ) {
	xp = w - maxwidth;			// right aligned
    } else if ( (tf & AlignHCenter) == AlignHCenter ) {
	xp = w/2 - maxwidth/2;			// centered text
    } else {
	xp = 0;				// left aligned
    }

#if defined(CHECK_RANGE)
    int hAlignFlags = 0;
    if ( (tf & AlignRight) == AlignRight )
	hAlignFlags++;
    if ( (tf & AlignHCenter) == AlignHCenter )
	hAlignFlags++;
    if ( (tf & AlignLeft ) == AlignLeft )
	hAlignFlags++;

    if ( hAlignFlags > 1 )
	warning("QPainter::drawText: More than one of AlignRight, AlignLeft\n"
		"                    and AlignHCenter set in the tf parameter."
		);

    int vAlignFlags = 0;
    if ( (tf & AlignTop) == AlignTop )
	vAlignFlags++;
    if ( (tf & AlignVCenter) == AlignVCenter )
	vAlignFlags++;
    if ( (tf & AlignBottom ) == AlignBottom )
	vAlignFlags++;

    if ( hAlignFlags > 1 )
	warning("QPainter::drawText: More than one of AlignTop, AlignBottom\n"
		"                    and AlignVCenter set in the tf parameter."
		);
#endif // CHECK_RANGE

    QRect br( x+xp, y+yp, maxwidth, nlines*fheight );
    if ( brect )				// set bounding rect
	*brect = br;

    if ( !painter || (tf & DontPrint) != 0 ) {	// can't/don't print any text
	if ( code_alloc )
	    free( codes );
	return;
    }

    // From here, we have a painter.

    QRegion save_rgn = painter->crgn;		// save the current region
    bool    clip_on  = painter->testf(QPainter::ClipOn);

    if ( len > 200 ) {
	p = new char[len];			// buffer for printable string
	CHECK_PTR( p );
	p_alloc = TRUE;
    } else {
	p = p_array;
	p_alloc = FALSE;
    }

    if ( br.x() >= x && br.y() >= y && br.width() < w && br.height() < h )
	tf |= DontClip;				// no need to clip

    if ( (tf & DontClip) == 0 ) {		// clip text
	QRegion new_rgn;
	QRect r( x, y, w, h );
	if ( painter->txop == TxRotShear ) {		// world xform active
	    QPointArray a( r );			// complex region
	    a = painter->xForm( a );
	    new_rgn = QRegion( a );
	} else {
	    r = painter->xForm( r );
	    new_rgn = QRegion( r );
	}
	if ( clip_on )				// combine with existing region
	    new_rgn = new_rgn.intersect( painter->crgn );
	painter->setClipRegion( new_rgn );
    }

    QBitmap  *mask;
    QPainter *pp;
    QPixmap *pm;

    if ( (tf & GrayText) == GrayText ) {	// prepare to draw gray text
	// #### NOTE: will not work with too-big-to-fit unclipped text.
	mask = new QBitmap( w, fheight );
	CHECK_PTR( mask );
	pp = new QPainter( mask );
	pp->setBrush( Dense4Pattern );
	pp->setBackgroundMode( TransparentMode );
	pp->setPen( color1 );
	CHECK_PTR( pp );
	pm = new QPixmap( w, fheight );
	CHECK_PTR( pm );
    } else {
	mask = 0;
	pp = 0;
	pm = 0;
    }

    yp += fascent;

    register ushort *cp = codes;

    while ( *cp ) {				// finally, draw the text

	tw = *cp++ & WIDTHBITS;			// text width

	if ( tw == 0 ) {			// ignore empty line
	    while ( *cp && (*cp & BEGLINE) != BEGLINE )
		cp++;
	    yp += fheight;
	    continue;
	}

	if ( (tf & AlignRight) == AlignRight ) {
	    xc = w - tw + fm.minRightBearing();
	} else if ( (tf & AlignHCenter) == AlignHCenter ) {
	    xc = w/2 - (tw-fm.minLeftBearing()-fm.minRightBearing())/2
		     - fm.minLeftBearing();
	} else {
	    xc = -fm.minLeftBearing();
	}

	if ( pp )				// erase pixmap if gray text
	    pp->fillRect( 0, 0, w, fheight, color0 );

	int bxc = xc;				// base x position (chars)
	while ( TRUE ) {
	    k = 0;
	    while ( *cp && (*cp & (BEGLINE|TABSTOP)) == 0 ) {
		if ( (*cp & PREFIX) == PREFIX ) {
		    int xcpos = fm.width( p, k );
		    if ( pp )			// gray text
			pp->fillRect( xc+xcpos, fascent+fm.underlinePos(),
				      CWIDTH( *cp&0xff ), fm.lineWidth(),
				      color1 );
		    else
			painter->fillRect( x+xc+xcpos, y+yp+fm.underlinePos(),
				  CWIDTH( *cp&0xff ), fm.lineWidth(),
				  painter->cpen.color() );
		}
		p[k++] = (char)*cp++;
	    }
	    if ( pp )				// gray text
		pp->drawText( xc, fascent, p, k );
	    else
		painter->drawText( x+xc, y+yp, p, k );	// draw the text
	    if ( (*cp & TABSTOP) == TABSTOP ) {
		int w = (*cp++ & WIDTHBITS);
		xc = bxc + w;
	    } else {				// *cp == 0 || *cp == BEGLINE
		break;
	    }
	}
	if ( pp ) {				// gray text
	    pp->setPen(color0);
	    pp->drawRect( mask->rect() );
	    pp->setPen(color1);
	    pm->fill( painter->cpen.color() );
	    pp->end();
	    pm->setMask( *mask );
	    painter->drawPixmap( x, y+yp-fascent, *pm );
	    pp->begin( mask );
	}

	yp += fheight;
    }

    if ( pp ) {					// gray text
	pp->end();
	delete pp;
	delete pm;
    }

    if ( (tf & DontClip) == 0 ) {		// restore clipping
	if ( clip_on ) {			// set original region
	    painter->setClipRegion( save_rgn );
	} else {				// clipping was off
	    painter->crgn = save_rgn;
	    painter->setClipping( FALSE );
	}
    }

    if ( p_alloc )
	delete [] p;
    if ( code_alloc )
	free( codes );
}


/*!

  Returns the bounding rectangle of the aligned text that would be
  printed with the corresponding drawText() function (the first \e len
  characters from \e str).  The drawing, and hence the bounding
  rectangle, is constrained to the rectangle \e (x,y,w,h).

  The \e tf text formatting is the bitwise OR of the following flags:
  <ul>
  <li> \c AlignLeft aligns to the left border.
  <li> \c AlignRight aligns to the right border.
  <li> \c AlignHCenter aligns horizontally centered.
  <li> \c AlignTop aligns to the top border.
  <li> \c AlignBottom aligns to the bottom border.
  <li> \c AlignVCenter aligns vertically centered
  <li> \c AlignCenter (= \c AlignHCenter | \c AlignVCenter)
  <li> \c SingleLine ignores newline characters in the text.
  <li> \c ExpandTabs expands tabulators.
  <li> \c ShowPrefix displays "&x" as "x" underlined.
  <li> \c WordBreak breaks the text to fit the rectangle.
  </ul>

  These flags are defined in qwindowdefs.h.

  \sa drawText(), fontMetrics()
*/

QRect QPainter::boundingRect( int x, int y, int w, int h, int tf,
			      const char *str, int len, char **internal )
{
    QRect brect;
    if ( str && *str )
	drawText( x, y, w, h, tf | DontPrint, str, len, &brect, internal );
    else
	brect.setRect( x,y, 0,0 );
    return brect;
}

/*****************************************************************************
  QPen member functions
 *****************************************************************************/

/*!
  \class QPen qpen.h
  \brief The QPen class defines how a QPainter should draw lines and outlines
  of shapes.
  \ingroup drawing
  \ingroup shared

  A pen has a style, a width and a color.

  The pen style defines the line type. The default pen style is \c SolidPen.
  Setting the style to \c NoPen tells the painter to not draw lines or
  outlines.

  The pen width defines the line width. The default line width is 0,
  which draws a 1-pixel line very fast, but with lower presicion than
  with a line width of 1. Setting the line width to 1 or more draws
  lines that are precise, but drawing is slower.

  The pen color defines the color of lines and text. The default line
  color is black.  The QColor documentation lists predefined colors.

  Use the QBrush class for specifying fill styles.

  Example:
  \code
    QPainter painter;
    QPen     pen( red, 2 );		// red solid line, 2 pixel width
    painter.begin( &anyPaintDevice );	// paint something
    painter.setPen( pen );		// set the red, fat pen
    painter.drawRect( 40,30, 200,100 ); // draw rectangle
    painter.setPen( blue );		// set blue pen, 0 pixel width
    painter.drawLine( 40,30, 240,130 ); // draw diagonal in rectangle
    painter.end();			// painting done
  \endcode

  See the setStyle() function for a complete list of pen styles.

  \sa QPainter, QPainter::setPen()
*/


/*!
  \internal
  Initializes the pen.
*/

void QPen::init( const QColor &color, uint width, PenStyle style )
{
    data = new QPenData;
    CHECK_PTR( data );
    data->style = style;
    data->width = width;
    data->color = color;
}

/*!
  Constructs a default black solid line pen with 0 width.
*/

QPen::QPen()
{
    init( black, 0, SolidLine );		// default pen
}

/*!
  Constructs a	pen black with 0 width and a specified style.
  \sa setStyle()
*/

QPen::QPen( PenStyle style )
{
    init( black, 0, style );
}

/*!
  Constructs a pen with a specified color, width and style.
  \sa setWidth(), setStyle(), setColor()
*/

QPen::QPen( const QColor &color, uint width, PenStyle style )
{
    init( color, width, style );
}

/*!
  Constructs a pen which is a copy of \e p.
*/

QPen::QPen( const QPen &p )
{
    data = p.data;
    data->ref();
}

/*!
  Destroys the pen.
*/

QPen::~QPen()
{
    if ( data->deref() )
	delete data;
}


/*!
  Detaches from shared pen data to makes sure that this pen is the only
  one referring the data.

  If multiple pens share common data, this pen dereferences the data
  and gets a copy of the data. Nothing is done if there is just a
  single reference.
*/

void QPen::detach()
{
    if ( data->count != 1 )
	*this = copy();
}


/*!
  Assigns \e c to this pen and returns a reference to this pen.
*/

QPen &QPen::operator=( const QPen &p )
{
    p.data->ref();				// beware of p = p
    if ( data->deref() )
	delete data;
    data = p.data;
    return *this;
}


/*!
  Returns a
  \link shclass.html deep copy\endlink of the pen.
*/

QPen QPen::copy() const
{
    QPen p( data->color, data->width, data->style );
    return p;
}


/*!
  \fn PenStyle QPen::style() const
  Returns the pen style.
  \sa setStyle()
*/

/*!
  Sets the pen style to \e s.

  The pen styles are:
  <ul>
  <li> \c NoPen  no outline is drawn.
  <li> \c SolidLine  solid line (default).
  <li> \c DashLine  - - - (dashes) line.
  <li> \c DotLine  * * * (dots) line.
  <li> \c DashDotLine  - * - * line.
  <li> \c DashDotDotLine  - ** - ** line.
  </ul>

  \sa style()
*/

void QPen::setStyle( PenStyle s )
{
    if ( data->style == s )
	return;
    detach();
    data->style = s;
}


/*!
  \fn uint QPen::width() const
  Returns the pen width.
  \sa setWidth()
*/

/*!
  Sets the pen width to \e w.
  \sa width()
*/

void QPen::setWidth( uint w )
{
    if ( data->width == w )
	return;
    detach();
    data->width = w;
}


/*!
  \fn const QColor &QPen::color() const
  Returns the pen color.
  \sa setColor()
*/

/*!
  Sets the pen color to \e c.
  \sa color()
*/

void QPen::setColor( const QColor &c )
{
    detach();
    data->color = c;
}


/*!
  \fn bool QPen::operator!=( const QPen &p ) const
  Returns TRUE if the pen is different from \e p, or FALSE if the pens are
  equal.

  Two pens are different if they have different styles, widths or colors.

  \sa operator==()
*/

/*!
  Returns TRUE if the pen is equal to \e p, or FALSE if the pens are
  different.

  Two pens are equal if they have equal styles, widths and colors.

  \sa operator!=()
*/

bool QPen::operator==( const QPen &p ) const
{
    return (p.data == data) || (p.data->style == data->style &&
	    p.data->width == data->width && p.data->color == data->color);
}


/*****************************************************************************
  QPen stream functions
 *****************************************************************************/

/*!
  \relates QPen
  Writes a pen to the stream and returns a reference to the stream.

  The serialization format is:
  <ol>
  <li> The pen style (UINT8)
  <li> The pen width (UINT8)
  <li> The pen color (QColor)
  </ol>
*/

QDataStream &operator<<( QDataStream &s, const QPen &p )
{
    return s << (UINT8)p.style() << (UINT8)p.width() << p.color();
}

/*!
  \relates QPen
  Reads a pen from the stream and returns a reference to the stream.
*/

QDataStream &operator>>( QDataStream &s, QPen &p )
{
    UINT8 style, width;
    QColor color;
    s >> style;
    s >> width;
    s >> color;
    p = QPen( color, (uint)width, (PenStyle)style );
    return s;
}


/*****************************************************************************
  QBrush member functions
 *****************************************************************************/

/*!
  \class QBrush qbrush.h

  \brief The QBrush class defines the fill pattern of shapes drawn by a QPainter.

  \ingroup drawing
  \ingroup shared

  A brush has a style and a color.  One of the brush styles is a custom
  pattern, which is defined by a QPixmap.

  The brush style defines the fill pattern. The default brush style is \c
  NoBrush (depends on how you construct a brush).  This style tells the
  painter to not fill shapes. The standard style for filling is called \c
  SolidPattern.

  The brush color defines the color of the fill pattern.
  The QColor documentation lists the predefined colors.

  Use the QPen class for specifying line/outline styles.

  Example:
  \code
    QPainter painter;
    QBrush   brush( yellow );		// yellow solid pattern
    painter.begin( &anyPaintDevice );	// paint something
    painter.setBrush( brush );		// set the yellow brush
    painter.setPen( NoPen );		// do not draw outline
    painter.drawRect( 40,30, 200,100 ); // draw filled rectangle
    painter.setBrush( NoBrush );	// do not fill
    painter.setPen( black );		// set black pen, 0 pixel width
    painter.drawRect( 10,10, 30,20 );	// draw rectangle outline
    painter.end();			// painting done
  \endcode

  See the setStyle() function for a complete list of brush styles.

  \sa QPainter, QPainter::setBrush(), QPainter::setBrushOrigin()
*/


/*!
  \internal
  Initializes the brush.
*/

void QBrush::init( const QColor &color, BrushStyle style )
{
    data = new QBrushData;
    CHECK_PTR( data );
    data->style	 = style;
    data->color	 = color;
    data->pixmap = 0;
}

/*!
  Constructs a default black brush with the style \c NoBrush (will not fill
  shapes).
*/

QBrush::QBrush()
{
    init( black, NoBrush );
}

/*!
  Constructs a black brush with the specified style.
  \sa setStyle()
*/

QBrush::QBrush( BrushStyle style )
{
    init( black, style );
}

/*!
  Constructs a brush with a specified color and style.
  \sa setColor(), setStyle()
*/

QBrush::QBrush( const QColor &color, BrushStyle style )
{
    init( color, style );
}

/*!
  Constructs a brush with a specified color and a custom pattern.

  The color will only have an effect for monochrome pixmaps, i.e.
  QPixmap::depth() == 1.

  \sa setColor(), setPixmap()
*/

QBrush::QBrush( const QColor &color, const QPixmap &pixmap )
{
    init( color, CustomPattern );
    data->pixmap = new QPixmap( pixmap );
}

/*!
  Constructs a brush which is a
  \link shclass.html shallow copy\endlink of \e b.
*/

QBrush::QBrush( const QBrush &b )
{
    data = b.data;
    data->ref();
}

/*!
  Destroys the brush.
*/

QBrush::~QBrush()
{
    if ( data->deref() ) {
	delete data->pixmap;
	delete data;
    }
}


/*!
  Detaches from shared brush data to makes sure that this brush is the only
  one referring the data.

  If multiple brushes share common data, this pen dereferences the data
  and gets a copy of the data. Nothing is done if there is just a single
  reference.
*/

void QBrush::detach()
{
    if ( data->count != 1 )
	*this = copy();
}


/*!
  Assigns \e b to this brush and returns a reference to this brush.
*/

QBrush &QBrush::operator=( const QBrush &b )
{
    b.data->ref();				// beware of b = b
    if ( data->deref() ) {
	delete data->pixmap;
	delete data;
    }
    data = b.data;
    return *this;
}


/*!
  Returns a
  \link shclass.html deep copy\endlink of the brush.
*/

QBrush QBrush::copy() const
{
    if ( data->style == CustomPattern ) {     // brush has pixmap
	QBrush b( data->color, *data->pixmap );
	return b;
    } else {				      // brush has std pattern
	QBrush b( data->color, data->style );
	return b;
    }
}


/*!
  \fn BrushStyle QBrush::style() const
  Returns the brush style.
  \sa setStyle()
*/

/*!
  Sets the brush style to \e s.

  The brush styles are:
  <ul>
  <li> \c NoBrush  will not fill shapes (default).
  <li> \c SolidPattern  solid (100%) fill pattern.
  <li> \c Dense1Pattern  94% fill pattern.
  <li> \c Dense2Pattern  88% fill pattern.
  <li> \c Dense3Pattern  63% fill pattern.
  <li> \c Dense4Pattern  50% fill pattern.
  <li> \c Dense5Pattern  37% fill pattern.
  <li> \c Dense6Pattern  12% fill pattern.
  <li> \c Dense7Pattern  6% fill pattern.
  <li> \c HorPattern  horizontal lines pattern.
  <li> \c VerPattern  vertical lines pattern.
  <li> \c CrossPattern  crossing lines pattern.
  <li> \c BDiagPattern  diagonal lines (directed / ) pattern.
  <li> \c FDiagPattern  diagonal lines (directed \ ) pattern.
  <li> \c DiagCrossPattern  diagonal crossing lines pattern.
  <li> \c CustomPattern  set when a pixmap pattern is being used.
  </ul>

  \sa style()
*/

void QBrush::setStyle( BrushStyle s )		// set brush style
{
    if ( data->style == s )
	return;
#if defined(CHECK_RANGE)
    if ( s == CustomPattern )
	warning( "QBrush::setStyle: CustomPattern is for internal use" );
#endif
    detach();
    data->style = s;
}


/*!
  \fn const QColor &QBrush::color() const
  Returns the brush color.
  \sa setColor()
*/

/*!
  Sets the brush color to \e c.
  \sa color(), setStyle()
*/

void QBrush::setColor( const QColor &c )
{
    detach();
    data->color = c;
}


/*!
  \fn QPixmap *QBrush::pixmap() const
  Returns a pointer to the custom brush pattern.

  A null pointer is returned if no custom brush pattern has been set.

  \sa setPixmap()
*/

/*!
  Sets the brush pixmap.  The style is set to \c CustomPattern.

  The curren brush color will only have an effect for monochrome pixmaps,
  i.e.	QPixmap::depth() == 1.

  \sa pixmap(), color()
*/

void QBrush::setPixmap( const QPixmap &pixmap )
{
    detach();
    data->style = CustomPattern;
    if ( data->pixmap )
	delete data->pixmap;
    data->pixmap = new QPixmap( pixmap );
}


/*!
  \fn bool QBrush::operator!=( const QBrush &b ) const
  Returns TRUE if the brush is different from \e b, or FALSE if the brushes are
  equal.

  Two brushes are different if they have different styles, colors or pixmaps.

  \sa operator==()
*/

/*!
  Returns TRUE if the brush is equal to \e b, or FALSE if the brushes are
  different.

  Two brushes are equal if they have equal styles, colors and pixmaps.

  \sa operator!=()
*/

bool QBrush::operator==( const QBrush &b ) const
{
    return (b.data == data) || (b.data->style == data->style &&
	    b.data->color  == data->color &&
	    b.data->pixmap == data->pixmap);
}


/*****************************************************************************
  QBrush stream functions
 *****************************************************************************/

/*!
  \relates QBrush
  Writes a brush to the stream and returns a reference to the stream.

  The serialization format is:
  <ol>
  <li> The brush style (UINT8)
  <li> The brush color (QColor)
  <li> If style == \c CustomPattern: the brush pixmap (QPixmap)
  </ol>
*/

QDataStream &operator<<( QDataStream &s, const QBrush &b )
{
    s << (UINT8)b.style() << b.color();
    if ( b.style() == CustomPattern )
	s << *b.pixmap();
    return s;
}

/*!
  \relates QBrush
  Reads a brush from the stream and returns a reference to the stream.
*/

QDataStream &operator>>( QDataStream &s, QBrush &b )
{
    UINT8 style;
    QColor color;
    s >> style;
    s >> color;
    if ( style == CustomPattern ) {
	QPixmap pm;
	s >> pm;
	b = QBrush( color, pm );
    }
    else
	b = QBrush( color, (BrushStyle)style );
    return s;
}
