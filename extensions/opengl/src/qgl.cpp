/****************************************************************************
** $Id: qgl.cpp,v 1.24.2.9 1999/01/28 12:26:12 aavit Exp $
**
** Implementation of OpenGL classes for Qt
**
** Created : 970112
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

#include "qgl.h"
#include <qpixmap.h>
#include <qpaintdevicemetrics.h>

#if defined(Q_GLX)
#include <qintdict.h>
#define INT8  dummy_INT8
#define INT32 dummy_INT32
#include <GL/glx.h>
#undef  INT8
#undef  INT32
#define	 GC GC_QQQ
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include <X11/Xatom.h>
#include <X11/Xmu/StdCmap.h>
#endif

#if defined(_CC_MSVC_)
#pragma warning(disable:4355) // 'this' : used in base member initializer list
#endif


/*!
  \relates QGLFormat
  Returns the version number string for the Qt OpenGL extension,
  e.g. "1.0".
*/

const char *qGLVersion()
{
    return QGL_VERSION_STR;
}


/*****************************************************************************
  QGLFormat implementation
 *****************************************************************************/


/*!
  \class QGLFormat qgl.h
  \brief The QGLFormat class specifies the display format of an OpenGL
  rendering context.

  \extension OpenGL

  A display format has several characteristics:
  <ul>
  <li> \link setDoubleBuffer() Double or single buffering.\endlink
  <li> \link setDepth() Depth buffer.\endlink
  <li> \link setRgba() RGBA or color index mode.\endlink
  <li> \link setAlpha() Alpha channel.\endlink
  <li> \link setAccum() Accumulation buffer.\endlink
  <li> \link setStencil() Stencil buffer.\endlink
  <li> \link setStereo() Stereo buffers.\endlink
  <li> \link setDirectRendering() Direct rendering..\endlink
  </ul>

  You create and tell a QGLFormat object what rendering options
  you want from an OpenGL rendering context.

  OpenGL drivers or accelerated hardware may or may not support
  advanced features like alpha channel or stereographic viewing. If
  you request some features the driver/hardware does not provide when
  you create a QGLWidget, you will get the a rendering context with
  the nearest subset of features.

  There are different ways of defining the display characteristics
  of a rendering context. One is to create a QGLFormat and make
  it default for the entire application:
  \code
    QGLFormat f;
    f.setAlpha( TRUE );
    f.setStereo( TRUE );
    QGLFormat::setDefaultFormat( f );
  \endcode

  Or you can specify the desired format when creating an object of
  your QGLWidget subclass:
  \code
    QGLFormat f;
    f.setDoubleBuffer( FALSE );
    f.setDirectRendering( FALSE );
    MyGLWidget* myWidget = new MyGLWidget( f, ... );
  \endcode

  You can even set the format for an already existing QGLWidget:
  \code
    MyGLWidget *w;
      ...
    QGLFormat f;
    f.setAlpha( TRUE );
    f.setStereo( TRUE );
    w->setFormat( f );
    if ( !w->format().stereo() ) {
        // ok, goggles off
        if ( !w->format().alpha() ) {
            fatal( "Cool hardware wanted" );
        }
    }
  \endcode

  \sa QGLContext, QGLWidget
*/


/*!
  Constructs a QGLFormat object with the factory default settings:
  <ul>
  <li> \link setDoubleBuffer() Double buffer:\endlink Enabled.
  <li> \link setDepth() Depth buffer:\endlink Enabled.
  <li> \link setRgba() RGBA:\endlink Enabled (i.e. color index disabled).
  <li> \link setAlpha() Alpha channel:\endlink Disabled.
  <li> \link setAccum() Accumulator buffer:\endlink Disabled.
  <li> \link setStencil() Stencil buffer:\endlink Disabled.
  <li> \link setStereo() Stereo:\endlink Disabled.
  <li> \link setDirectRendering() Direct rendering:\endlink Enabled.
  </ul>
*/

QGLFormat::QGLFormat()
{
    opts = DoubleBuffer | DepthBuffer | Rgba | DirectRendering;
}


/*!
  Creates a QGLFormat object that is a copy of the current \link
  defaultFormat() application default format\endlink.

  If \a options is not 0, this copy will be modified by these format options.
  The \a options parameter must be FormatOption values OR'ed together.
  
  This constructor makes it easy to specify a certain desired format
  in classes derived from QGLWidget, for example:
  \code
    // The rendering in MyGLWidget depends on using 
    // stencil buffer and alpha channel
    MyGLWidget::MyGLWidget( QWidget* parent, const char* name )
        : QGLWidget( QGLFormat( StencilBuffer | AlphaChannel ), parent, name )
    {
      if ( !format().stencil() )
        warning( "Could not get stencil buffer; results will be suboptimal" );
      if ( !format().alphaChannel() )
        warning( "Could not get alpha channel; results will be suboptimal" );
      ...
   }
  \endcode

  Note that there exists FormatOption values for both turning on and
  off all format settings, e.g. DepthBuffer and NoDepthBuffer,
  DirectRendering and IndirectRendering, etc.

  \sa defaultFormat(), setOption()
*/

QGLFormat::QGLFormat( int options )
{
    uint newOpts = options;
    opts = defaultFormat().opts;
    opts |= ( newOpts & 0xffff );
    opts &= ~( newOpts >> 16 );
}


/*!
  \fn bool QGLFormat::doubleBuffer() const
  Returns TRUE if double buffering is enabled, otherwise FALSE.
  Double buffering is enabled by default.
  \sa setDoubleBuffer()
*/

/*!
  Sets double buffering if \a enable is TRUE og single buffering if
  \a enable is FALSE.

  Double buffering is enabled by default.

  Double buffering is a technique where graphics is rendered to an off-screen
  buffer and not directly to the screen. When the drawing has been
  completed, the program calls a swapBuffers function to exchange the screen
  contents with the buffer. The result is flicker-free drawing and often
  better performance.

  \sa doubleBuffer(), QGLContext::swapBuffers(), QGLWidget::swapBuffers()
*/

void QGLFormat::setDoubleBuffer( bool enable )
{
    setOption( enable ? DoubleBuffer : SingleBuffer );
}


/*!
  \fn bool QGLFormat::depth() const
  Returns TRUE if the depth buffer is enabled, otherwise FALSE.
  The depth buffer is enabled by default.
  \sa setDepth()
*/

/*!
  Enables the depth buffer if \a enable is TRUE, or disables
  it if \a enable is FALSE.

  The depth buffer is enabled by default.

  The purpose of a depth buffer (or z-buffering) is to remove hidden
  surfaces. Pixels are assigned z values based on the distance to the
  viewer. A pixel with a high z value is closer to the viewer than a
  pixel with a low z value. This information is used to decide whether
  to draw a pixel or not.

  \sa depth()
*/

void QGLFormat::setDepth( bool enable )
{
    setOption( enable ? DepthBuffer : NoDepthBuffer );
}


/*!
  \fn bool QGLFormat::rgba() const
  Returns TRUE if RGBA color mode is set, or FALSE if color index
  mode is set. The default color mode is RGBA.
  \sa setRgba()
*/

/*!
  Sets RGBA mode if \a enable is TRUE, or color index mode if \a enable
  is FALSE.

  The default color mode is RGBA.

  RGBA is the preferred mode for most OpenGL applications.
  In RGBA color mode you specify colors as a red + green + blue + alpha
  quadruplet.

  In color index mode you specify an index into a color lookup table.

  \sa rgba()
*/

void QGLFormat::setRgba( bool enable )
{
    setOption( enable ? Rgba : ColorIndex );
}


/*!
  \fn bool QGLFormat::alpha() const

  Returns TRUE if the alpha channel of the framebuffer is enabled,
  otherwise FALSE.  The alpha channel is disabled by default.

  \sa setAlpha()
*/

/*!

  Enables the alpha channel of the framebuffer if \a enable is TRUE,
  or disables it if \a enable is FALSE.

  The alpha buffer is disabled by default.

  The alpha channel is typically used for implementing transparency or
  translucency.  The A in RGBA specifies the transparency of a pixel.

  \sa alpha()
*/

void QGLFormat::setAlpha( bool enable )
{
    setOption( enable ? AlphaChannel : NoAlphaChannel );
}


/*!
  \fn bool QGLFormat::accum() const
  Returns TRUE if the accumulation buffer is enabled, otherwise FALSE.
  The accumulation buffer is disabled by default.
  \sa setAccum()
*/

/*!
  Enables the accumulation buffer if \a enable is TRUE, or disables
  it if \a enable is FALSE.

  The accumulation buffer is disabled by default.

  The accumulation buffer is used for create blur effects and
  multiple exposures.

  \sa accum()
*/

void QGLFormat::setAccum( bool enable )
{
    setOption( enable ? AccumBuffer : NoAccumBuffer );
}


/*!
  \fn bool QGLFormat::stencil() const
  Returns TRUE if the stencil buffer is enabled, otherwise FALSE.
  The stencil buffer is disabled by default.
  \sa setStencil()
*/

/*!
  Enables the stencil buffer if \a enable is TRUE, or disables
  it if \a enable is FALSE.

  The stencil buffer is disabled by default.

  The stencil buffer masks away drawing from certain parts of
  the screen.

  \sa stencil()
*/

void QGLFormat::setStencil( bool enable )
{
    setOption( enable ? StencilBuffer: NoStencilBuffer );
}


/*!
  \fn bool QGLFormat::stereo() const
  Returns TRUE if stereo buffering is enabled, otherwise FALSE.
  Stereo buffering is disabled by default.
  \sa setStereo()
*/

/*!
  Enables stereo buffering if \a enable is TRUE, or disables
  it if \a enable is FALSE.

  Stereo buffering is disabled by default.

  Stereo buffering provides extra color buffers to generate left-eye
  and right-eye images.

  \sa stereo()
*/

void QGLFormat::setStereo( bool enable )
{
    setOption( enable ? StereoBuffers : NoStereoBuffers );
}


/*!
  \fn bool QGLFormat::directRendering() const
  Returns TRUE if direct rendering is enabled, otherwise FALSE.

  Direct rendering is enabled by default.

  \sa setDirectRendering()
*/

/*!
  Enables direct rendering if \a enable is TRUE, or disables
  it if \a enable is FALSE.

  Direct rendering is enabled by default.

  Enabling this option will make OpenGL bypass the underlying window
  system and render directly from hardware to the screen, if this is
  supported by the system.

  \sa directRendering()
*/

void QGLFormat::setDirectRendering( bool enable )
{
    setOption( enable ? DirectRendering : IndirectRendering );
}


/*!
  Sets the option \a opt.

  \sa testOption()
*/

void QGLFormat::setOption( FormatOption opt )
{
    if ( opt & 0xffff )
	opts |= opt;
    else
	opts &= ~( opt >> 16 );
}



/*!
  Returns TRUE if format option \a opt is set, otherwise FALSE.

  \sa setOption()
*/

bool QGLFormat::testOption( FormatOption opt ) const
{
    if ( opt & 0xffff )
	return ( opts & opt ) != 0;
    else
	return ( opts & ( opt >> 16 ) ) == 0;
}    



/*!
  Returns TRUE if the window system has any OpenGL support,
  otherwise FALSE.
*/

bool QGLFormat::hasOpenGL()
{
#if defined(Q_WGL)
    return TRUE;
#else
    return glXQueryExtension(qt_xdisplay(),0,0) != 0;
#endif
}


static QGLFormat *default_format = 0;

static void cleanupGLFormat()
{
    delete default_format;
    default_format = 0;
}


/*!
  Returns the default QGLFormat for the application.
  All QGLWidgets that are created use this format unless
  anything else is specified.

  If no special default format has been set using setDefaultFormat(),
  the default format is the same as that created with QGLFormat().

  \sa setDefaultFormat()
*/

QGLFormat QGLFormat::defaultFormat()
{
    if ( !default_format ) {
	default_format = new QGLFormat;
	qAddPostRoutine( cleanupGLFormat );
    }
    return *default_format;
}

/*!
  Sets a new default QGLFormat for the application.
  For example, to set single buffering as default instead
  of double buffering, your main() can contain:
  \code
    QApplication a(argc, argv);
    QGLFormat f;
    f.setDoubleBuffer( FALSE );
    QGLFormat::setDefaultFormat( f );
  \endcode

  \sa defaultFormat()
*/

void QGLFormat::setDefaultFormat( const QGLFormat &f )
{
    if ( !default_format ) {
	default_format = new QGLFormat;
	qAddPostRoutine( cleanupGLFormat );
    }
    *default_format = f;
}


/*!
  Returns TRUE if all options of the two QGLFormats are equal.
*/

bool operator==( const QGLFormat& a, const QGLFormat& b )
{
    return a.opts == b.opts;
}


/*!
  Returns FALSE if all options of the two QGLFormats are equal.
*/

bool operator!=( const QGLFormat& a, const QGLFormat& b )
{
    return !( a == b );
}


#if defined(Q_GLX)

/*
  The create_cmap function is internal and used by QGLWidget::setContext()
  and GLX (not Windows).  If the application can't find any sharable
  colormaps, it must at least create as few colormaps as possible.  The
  dictionary solution below ensures only one colormap is created per visual.
  Colormaps are also deleted when the application terminates.
*/

struct CMapEntry {
    CMapEntry( Colormap m, bool a ) : cmap(m), alloc(a) {}
   ~CMapEntry();
    Colormap cmap;
    bool     alloc;
};

CMapEntry::~CMapEntry()
{
    if ( alloc )
	XFreeColormap( QPaintDevice::x__Display(), cmap );
}

static bool		    cmap_init = FALSE;
static QIntDict<CMapEntry> *cmap_dict = 0;
static bool		    mesa_gl   = FALSE;

static void cleanup_cmaps()
{
    if ( !cmap_dict )
	return;
    cmap_dict->setAutoDelete( TRUE );
    delete cmap_dict;
    cmap_dict = 0;
}

static Colormap choose_cmap( Display *dpy, XVisualInfo *vi )
{
    if ( !cmap_init ) {
	cmap_init = TRUE;
	cmap_dict = new QIntDict<CMapEntry>;
	const char *v = glXQueryServerString( dpy, vi->screen, GLX_VERSION );
	mesa_gl = strstr(v,"Mesa") != 0;
	qAddPostRoutine( cleanup_cmaps );
    }

    CMapEntry *x = cmap_dict->find( (long)vi->visualid );
    if ( x )					// found colormap for visual
	return x->cmap;

    Colormap cmap = 0;
    bool alloc = FALSE;
    XStandardColormap *c;
    int n, i;

    if ( vi->visual==DefaultVisual(dpy,vi->screen) )
	return DefaultColormap( dpy, vi->screen );

    if ( mesa_gl ) {				// we're using MesaGL
	Atom hp_cmaps = XInternAtom( dpy, "_HP_RGB_SMOOTH_MAP_LIST", TRUE );
	if ( hp_cmaps && vi->visual->c_class == TrueColor && vi->depth == 8 ) {
	    if ( XGetRGBColormaps(dpy,RootWindow(dpy,vi->screen),&c,&n,
				  hp_cmaps) ) {
		i = 0;
		while ( i < n && cmap == 0 ) {
		    if ( c[i].visualid == vi->visual->visualid )
			cmap = c[i].colormap;
		    i++;
		}
		XFree( (char *)c );
	    }
	}
    }
#if !defined(_OS_SOLARIS_)
    if ( !cmap ) {
	if ( XmuLookupStandardColormap(dpy,vi->screen,vi->visualid,vi->depth,
				       XA_RGB_DEFAULT_MAP,FALSE,TRUE) ) {
	    if ( XGetRGBColormaps(dpy,RootWindow(dpy,vi->screen),&c,&n,
				  XA_RGB_DEFAULT_MAP) ) {
		i = 0;
		while ( i < n && cmap == 0 ) {
		    if ( c[i].visualid == vi->visualid )
			cmap = c[i].colormap;
		    i++;
		}
		XFree( (char *)c );
	    }
	}
    }
#endif
    if ( !cmap ) {				// no shared cmap found
	cmap = XCreateColormap( dpy, RootWindow(dpy,vi->screen), vi->visual,
				AllocNone );
	alloc = TRUE;
    }
    x = new CMapEntry( cmap, alloc );		// associate cmap with visualid
    cmap_dict->insert( (long)vi->visualid, x );
    return cmap;
}

#endif // Q_GLX


/*****************************************************************************
  QGLContext implementation
 *****************************************************************************/


/*!
  \class QGLContext qgl.h
  \brief The QGLContext class encapsulates an OpenGL rendering context.

  \extension OpenGL

  An OpenGL rendering context is a complete set of OpenGL state
  variables.

*/


/*!
  Constructs an OpenGL context for the paint device \a device, which
  can be a widget or a pixmap. The \a format specifies several display
  options for this context.

  If the underlying OpenGL/Window system cannot satisfy all the
  features requested in \a format, the nearest subset of features will
  be used. After creation, the format() method will return the actual
  format obtained.

  The context will be \link isValid() invalid\endlink if it was not
  possible to obtain a GL context at all.

  \sa format(), isValid()
*/

QGLContext::QGLContext( const QGLFormat &format, QPaintDevice *device )
    : glFormat(format), paintDevice(device)
{
    valid = FALSE;
#if defined(Q_GLX)
    gpm = 0;
#endif
    crWin = FALSE;
    initDone = FALSE;
    sharing = FALSE;
    if ( paintDevice == 0 ) {
#if defined(CHECK_NULL)
	warning( "QGLContext: Paint device cannot be null" );
	return;
#endif
    }
    if ( paintDevice->devType() != PDT_WIDGET &&
	 paintDevice->devType() != PDT_PIXMAP ) {
#if defined(CHECK_RANGE)
	warning( "QGLContext: Unsupported paint device type" );
#endif
    }
}

/*!
  Destroys the OpenGL context.
*/

QGLContext::~QGLContext()
{
    reset();
}


/*!
  \fn QGLFormat QGLContext::format() const
  Returns the format.
  \sa setFormat()
*/

/*!
  Sets a \a format for this context. The context is \link reset()
  reset\endlink.

  Call create() to create a new GL context that tries to match the new
  format.

  \code
    QGLContext *cx;
      ...
    QGLFormat f;
    f.setStereo( TRUE );
    cx->setFormat( f );
    if ( !cx->create() )
        exit(); // no OpenGL support, or cannot render on specified paintdevice
    if ( !cx->format().stereo() )
	exit(); // could not create stereo context
  \endcode

  \sa format(), reset(), create()
*/

void QGLContext::setFormat( const QGLFormat &format )
{
    reset();
    glFormat = format;
}


/*!
  \fn bool QGLContext::isValid() const
  Returns TRUE if a GL rendering context has been successfully created.
*/

/*!
  \fn bool QGLContext::isSharing() const 

  Returns TRUE if display list sharing with another context was
  requested in the create() call, and the GL system was able to
  fulfill this request. Note that display list sharing may possibly
  not be supported between contexts with different formats.
*/

/*!
  \fn bool QGLContext::deviceIsPixmap() const 

  Returns TRUE if the paint device of this context is a pixmap,
  otherwise FALSE.
*/

/*!
  \fn bool QGLContext::windowCreated() const 

  Returns TRUE if a window has been created for this context,
  otherwise FALSE.

  \sa setWindowCreated()
*/

/*!
  \fn void QGLContext::setWindowCreated( bool on )

  Tells the context whether a window has already been created for it.

  \sa windowCreated()
*/

/*!
  \fn bool QGLContext::initialized() const

  Returns TRUE if this context has been initialized, i.e. if
  QGLWidget::initializeGL() has been performed on it.

  \sa setInitialized()
*/

/*!
  \fn void QGLContext::setInitialized( bool on )

  Tells the context whether it has been initialized, i.e. whether
  QGLWidget::initializeGL() has been performed on it.

  \sa initialized()
*/

/*!
  Creates the GL context. Returns TRUE if it was successful in
  creating a GL rendering context on the paint device specified in the
  constructor, otherwise FALSE is returned (the context is invalid).

  After successful creation, format() returns the set of features of
  the created GL rendering context.

  If \a shareContext points to a valid QGLContext, this method will
  try to establish OpenGL display list sharing between this context
  and \a shareContext. Note that this may fail if the two contexts
  have different formats. Use isSharing() to test.

  <strong>Implementation note:</strong> Initialization of C++ class members
  usually takes place in the class constructor. QGLContext is an exception
  because it must be simple to customize. The virtual functions
  chooseContext() (and chooseVisual() for X11) can be reimplemented in a
  subclass to select a particular context. The trouble is that virtual
  functions are not properly called during construction (which is indeed
  correct C++), hence we need a create() function.

  \sa chooseContext(), format(), isValid()
*/

bool QGLContext::create( const QGLContext* shareContext )
{
    reset();
    valid = chooseContext( shareContext );
    return valid;
}



/*!
  \fn bool QGLContext::chooseContext( const QGLContext* shareContext = 0 )

  This semi-internal function is called by create(). It creates a
  system-dependent OpenGL handle that matches the specified \link
  setFormat() format\endlink as closely as possible.

  <strong>Windows</strong>: Calls choosePixelFormat() which finds a
  matching pixel format identifier.

  <strong>X11</strong>: Calls chooseVisual() which finds an appropriate
  X visual.

  choosePixelFormat() and chooseVisual() can be reimplemented in a
  subclass if you need to choose a very custom context.
*/


/*!
  \fn void QGLContext::reset()

  Resets the context and makes it invalid.
  \sa create(), isValid()
*/


/*!
  \fn void QGLContext::makeCurrent()

  Makes this context the current OpenGL rendering context.  All GL
  functions you call operate on this context until another context is
  made current.
*/


/*!
  \fn void QGLContext::swapBuffers()

  Swaps the screen contents with an off-screen buffer. Works only if
  the context is in double buffer mode.
  \sa QGLFormat::setDoubleBuffer()
*/


/*!
  \fn void QGLContext::doneCurrent()

  Makes no GL context the current context. Normally, you do not need
  to call this function, QGLContext calls it as necessary.
*/


/*!
  \fn QPaintDevice* QGLContext::device() const

  Returns the paint device set for this context.

  \sa QGLContext::QGLContext()
*/


#if defined(Q_WGL)

void qwglError( const char* method, const char* func )
{
#if defined(CHECK_NULL)
    LPVOID lpMsgBuf;
    FormatMessage(
		  FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		  0, GetLastError(),
		  MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		  (LPTSTR) &lpMsgBuf, 0, 0 );
    warning( "%s : %s failed: %s", method, func, (const char *)lpMsgBuf );
    LocalFree( lpMsgBuf );
#endif
    const char* dummy = method; // Avoid compiler warning
    dummy = func;
}


/*****************************************************************************
  QGLContext Win32/WGL-specific code
 *****************************************************************************/


bool QGLContext::chooseContext( const QGLContext* shareContext )
{
    bool success = TRUE;
    if ( deviceIsPixmap() ) {
	win = 0;
	dc  = paintDevice->handle();
    }
    else {
	win = ((QWidget*)paintDevice)->winId();
	dc  = GetDC( win );
    }

    if ( !dc ) {
#if defined(CHECK_NULL)
	warning( "QGLContext::chooseContext(): Paint device cannot be null" );
#endif
	return FALSE;
    }

    PIXELFORMATDESCRIPTOR pfd;
    PIXELFORMATDESCRIPTOR realPfd;
    int pixelFormatId = choosePixelFormat( &pfd, dc );
    if ( pixelFormatId == 0 ) {
	qwglError( "QGLContext::chooseContext()", "ChoosePixelFormat" );
	success = FALSE;
    }
    else {
	DescribePixelFormat( dc, pixelFormatId, sizeof(PIXELFORMATDESCRIPTOR),
			     &realPfd );
	glFormat.setDoubleBuffer( realPfd.dwFlags & PFD_DOUBLEBUFFER );
	glFormat.setDepth( realPfd.cDepthBits );
	glFormat.setRgba( realPfd.iPixelType == PFD_TYPE_RGBA );
	glFormat.setAlpha( realPfd.cAlphaBits );
	glFormat.setAccum( realPfd.cAccumBits );
	glFormat.setStencil( realPfd.cStencilBits );
	glFormat.setStereo( realPfd.dwFlags & PFD_STEREO );
	glFormat.setDirectRendering( FALSE );

	if ( deviceIsPixmap() && !(realPfd.dwFlags & PFD_DRAW_TO_BITMAP) ) {
#if defined(CHECK_NULL)
	    warning( "QGLContext::chooseContext(): Failed to get pixmap rendering context." );
#endif
	    return FALSE;
	}
	if ( deviceIsPixmap() && 
	     (((QPixmap*)paintDevice)->depth() != realPfd.cColorBits ) ) {
#if defined(CHECK_NULL)
	    warning( "QGLContext::chooseContext(): Failed to get pixmap rendering context of suitable depth." );
#endif
	    return FALSE;
	}

    }

    if ( success && !SetPixelFormat(dc, pixelFormatId, &realPfd) ) {
	qwglError( "QGLContext::chooseContext()", "SetPixelFormat" );
	success = FALSE;
    }

    if ( success && !(rc = wglCreateContext( dc ) ) ) {
	qwglError( "QGLContext::chooseContext()", "wglCreateContext" );
	success = FALSE;
    }

    if ( success && shareContext && shareContext->isValid() )
	sharing = ( wglShareLists( shareContext->rc, rc ) != 0 );
    
    if ( !success ) {
	rc = 0;
	dc = 0;
    }

    return success;
}


/* 

<strong>Win32 only</strong>: This virtual function chooses a pixel format
that matches the OpenGL \link setFormat() format\endlink \a
fmt. Reimplement this function in a subclass if you need a custom context.

  \warning The \a pfd pointer is really a \c PIXELFORMATDESCRIPTOR*.
  We use \c void to avoid using Windows-specific types in our header files.

  \sa chooseContext() */

int QGLContext::choosePixelFormat( void *pfd, HANDLE pdc )
{
    PIXELFORMATDESCRIPTOR *p = (PIXELFORMATDESCRIPTOR *)pfd;
    memset( p, 0, sizeof(PIXELFORMATDESCRIPTOR) );
    p->nSize = sizeof(PIXELFORMATDESCRIPTOR);
    p->nVersion = 1;
    p->dwFlags  = PFD_SUPPORT_OPENGL;
    if ( deviceIsPixmap() )
	p->dwFlags |= PFD_DRAW_TO_BITMAP;
    else
	p->dwFlags |= PFD_DRAW_TO_WINDOW;
    if ( glFormat.doubleBuffer() )
	p->dwFlags |= PFD_DOUBLEBUFFER;
    if ( glFormat.stereo() )
	p->dwFlags |= PFD_STEREO;
    if ( glFormat.depth() )
	p->cDepthBits = 32;
    if ( glFormat.rgba() ) {
	p->iPixelType = PFD_TYPE_RGBA;
	if ( deviceIsPixmap() )
	    p->cColorBits = ((QPixmap*)paintDevice)->depth();
	else
	    p->cColorBits = 24;
    } else {
	p->iPixelType = PFD_TYPE_COLORINDEX;
	p->cColorBits = 8;
    }
    if ( glFormat.alpha() )
	p->cAlphaBits = 8;
    if ( glFormat.accum() )
	p->cAccumBits = p->cColorBits + p->cAlphaBits;
    if ( glFormat.stencil() )
	p->cStencilBits = 4;
    p->iLayerType = PFD_MAIN_PLANE;
    return ChoosePixelFormat( pdc, p );
}



void QGLContext::reset()
{
    if ( !valid )
	return;
    doneCurrent();
    if ( win )
	ReleaseDC( win, dc );
    wglDeleteContext( rc );
    rc  = 0;
    dc  = 0;
    win = 0;
    sharing = FALSE;
    valid = FALSE;
    initDone = FALSE;
}


static QGLContext *currentContext = 0;

//
// NOTE: In a multi-threaded environment, each thread has a current
// context. If we want to make this code thread-safe, we probably
// have to use TLS (thread local storage) for keeping current contexts.
//

void QGLContext::makeCurrent()
{
    if ( currentContext ) {
	if ( currentContext == this )		// already current
	    return;
	currentContext->doneCurrent();
    }
    if ( !valid || !dc )
	return;
    if ( QColor::hPal() ) {
	SelectPalette( dc, QColor::hPal(), FALSE );
	RealizePalette( dc );
    }
    if ( !wglMakeCurrent( dc, rc ) )
	qwglError( "QGLContext::makeCurrent()", "wglMakeCurrent" );
    currentContext = this;
}


void QGLContext::doneCurrent()
{
    if ( currentContext != this )
	return;
    currentContext = 0;
    wglMakeCurrent( dc, 0 );
}


void QGLContext::swapBuffers()
{
    if ( dc && glFormat.doubleBuffer() )
	SwapBuffers( dc );
}

#endif // Q_WGL



#if defined(Q_GLX)

/*****************************************************************************
  QGLContext UNIX/GLX-specific code
 *****************************************************************************/

bool QGLContext::chooseContext( const QGLContext* shareContext )
{
    bool ok = TRUE;
    vi = chooseVisual();
    if ( !vi )
	return FALSE;

    Bool direct = format().directRendering() ? True : False;
    

    if ( shareContext && 
	 ( !shareContext->isValid() || !shareContext->cx ) ) {
#if defined(CHECK_NULL)
	    warning("QGLContext::chooseContext(): Cannot share with invalid context");
#endif
	    shareContext = 0;
    }

    cx = 0;
    if ( shareContext ) {
	cx = glXCreateContext( paintDevice->x11Display(), (XVisualInfo *)vi,
			       (GLXContext)shareContext->cx, direct );
	if ( cx )
	    sharing = TRUE;
    }
    if ( !cx )
	cx = glXCreateContext( paintDevice->x11Display(), (XVisualInfo *)vi,
			       None, direct );
    if ( !cx )
	return FALSE;
    glFormat.setDirectRendering( glXIsDirect( paintDevice->x11Display(), 
					      (GLXContext)cx ) );
    if ( deviceIsPixmap() ) {
#ifdef GLX_MESA_pixmap_colormap
	gpm = glXCreateGLXPixmapMESA( paintDevice->x11Display(),
				      (XVisualInfo *)vi,
				      paintDevice->handle(),
				      choose_cmap( paintDevice->x11Display(),
						   (XVisualInfo *)vi ) );
#else
	gpm = (Q_UINT32)glXCreateGLXPixmap( paintDevice->x11Display(),
					    (XVisualInfo *)vi,
					    paintDevice->handle() );
#endif
	if ( !gpm )
	    return FALSE;
    }
    return TRUE;
}


/*
  <strong>X11 only</strong>: This virtual function tries to find a
  visual that matches the format, reducing the demands if the original
  request cannot be met.

  The algorithm for reducing the demands of the format is quite
  simple-minded, so override this method in your subclass if your
  application has spcific requirements on visual selection.

  \sa chooseContext()
*/

void *QGLContext::chooseVisual()
{
    //todo: if pixmap, also make sure that vi->depth == pixmap->depth
    void* vis = 0;
    bool fail = FALSE;
    QGLFormat fmt = format();
    bool tryDouble = !fmt.doubleBuffer();  // Some GL impl's only have double
    bool triedDouble = FALSE;
    while( !fail && !( vis = tryVisual( fmt ) ) ) {
	if ( tryDouble ) {
	    fmt.setDoubleBuffer( TRUE );
	    tryDouble = FALSE;
	    triedDouble = TRUE;
	    continue;
	}
	else if ( triedDouble ) {
	    fmt.setDoubleBuffer( FALSE );
	    triedDouble = FALSE;
	}
	if ( fmt.stereo() ) {
	    fmt.setStereo( FALSE );
	    continue;
	}
	if ( fmt.accum() ) {
	    fmt.setAccum( FALSE );
	    continue;
	}
	if ( fmt.stencil() ) {
	    fmt.setStencil( FALSE );
	    continue;
	}
	if ( fmt.alpha() ) {
	    fmt.setAlpha( FALSE );
	    continue;
	}
	if ( fmt.depth() ) {
	    fmt.setDepth( FALSE );
	    continue;
	}
	if ( fmt.doubleBuffer() ) {
	    fmt.setDoubleBuffer( FALSE );
	    continue;
	}
	fail = TRUE;
    }
    glFormat = fmt;
    return vis;
}

	  
/*
  <strong>X11 only</strong>: This virtual function chooses a visual
  that matches the OpenGL \link setFormat() format\endlink. Reimplement this
  function in a subclass if you need a custom visual.

  \sa chooseContext()
*/

void *QGLContext::tryVisual( const QGLFormat& f )
{
    int spec[40];
    int i = 0;
    spec[i++] = GLX_LEVEL;
    spec[i++] = 0;
    if ( f.doubleBuffer() )
	spec[i++] = GLX_DOUBLEBUFFER;
    if ( f.depth() ) {
	spec[i++] = GLX_DEPTH_SIZE;
	spec[i++] = 1;
    }
    if ( f.rgba() ) {
	spec[i++] = GLX_RGBA;
	spec[i++] = GLX_RED_SIZE;
	spec[i++] = 1;
	spec[i++] = GLX_GREEN_SIZE;
	spec[i++] = 1;
	spec[i++] = GLX_BLUE_SIZE;
	spec[i++] = 1;
 	if ( f.alpha() ) {
	    spec[i++] = GLX_ALPHA_SIZE;
	    spec[i++] = 1;
	}
	if ( f.accum() ) {
	    spec[i++] = GLX_ACCUM_RED_SIZE;
	    spec[i++] = 1;
	    spec[i++] = GLX_ACCUM_GREEN_SIZE;
	    spec[i++] = 1;
	    spec[i++] = GLX_ACCUM_BLUE_SIZE;
	    spec[i++] = 1;
	    if ( f.alpha() ) {
		spec[i++] = GLX_ACCUM_ALPHA_SIZE;
		spec[i++] = 1;
	    }
        }
	if ( f.stereo() ) {
	    spec[i++] = GLX_STEREO;
	}
    } else {
	spec[i++] = GLX_BUFFER_SIZE;
	spec[i++] = 24;
    }
    if ( f.stencil() ) {
	spec[i++] = GLX_STENCIL_SIZE;
	spec[i++] = 1;
    }
    spec[i] = None;
    return glXChooseVisual( paintDevice->x11Display(),
			    paintDevice->x11Screen(), spec );
}


void QGLContext::reset()
{
    if ( !valid )
	return;
    doneCurrent();
    if ( gpm )
	glXDestroyGLXPixmap( paintDevice->x11Display(), (GLXPixmap)gpm );
    gpm = 0;
    glXDestroyContext( paintDevice->x11Display(), (GLXContext)cx );
    if ( vi )
	XFree( vi );
    vi = 0;
    cx = 0;
    crWin = FALSE;
    sharing = FALSE;
    valid = FALSE;
    initDone = FALSE;
}


void QGLContext::makeCurrent()
{
    if ( !valid ) {
#if defined(CHECK_NULL)
	warning("QGLContext::makeCurrent(): Cannot make invalid context current.");
#endif
	return;
    }
    bool ok = TRUE;
    if ( deviceIsPixmap() )
	ok = glXMakeCurrent( paintDevice->x11Display(),
			     (GLXPixmap)gpm, 
			     (GLXContext)cx );
	     
    else
	ok = glXMakeCurrent( paintDevice->x11Display(),
			     ((QWidget *)paintDevice)->winId(),
			     (GLXContext)cx );
#if defined(CHECK_NULL)
    //    debug("makeCurrent: %i, vi=%i, vi->vi=%i, vi->id=%i", (int)this, (int)vi, (int)((XVisualInfo*)vi)->visual, (int)((XVisualInfo*)vi)->visualid );
    if ( !ok )
	warning("QGLContext::makeCurrent(): Failed.");
#endif
}

void QGLContext::doneCurrent()
{
    glXMakeCurrent( paintDevice->x11Display(), 0, 0 );
}


void QGLContext::swapBuffers()
{
    if ( !valid )
	return;
    if ( !deviceIsPixmap() )
	glXSwapBuffers( paintDevice->x11Display(),
			((QWidget *)paintDevice)->winId() );
}

#endif // Q_GLX



/*****************************************************************************
  QGLWidget implementation
 *****************************************************************************/


/*!
  \class QGLWidget qgl.h
  \brief The QGLWidget class is a widget for rendering OpenGL graphics.

  \extension OpenGL

  QGLWidget provides functionality for displaying OpenGL graphics
  integrated in a Qt application. It is very simple to use: you
  inherit from it and use the subclass like any other QWidget, only
  that instead of drawing the widget's contents using QPainter & al.,
  you use the standard OpenGL rendering commands.

  QGLWidget provides three convenient virtual functions that you can
  reimplement in your subclass to perform the typical OpenGL tasks:

  <ul>
  <li> paintGL() - Render the OpenGL scene. Gets called whenever the widget
  needs to be updated.
  <li> resizeGL() - Set up OpenGL viewport, projection etc. Gets called
  whenever the the widget has been resized (and also when it shown
  for the first time, since all newly created widgets get a resize
  event automatically).
  <li> initializeGL() - Set up the OpenGL rendering context, define display
  lists etc. Gets called once before the first time resizeGL() or
  paintGL() is called.
  </ul>

  Here is a rough outline of how your QGLWidget subclass may look:

  \code
    class MyGLDrawer : public QGLWidget
    {
        Q_OBJECT	// must include this if you use Qt signals/slots

    public:
        MyGLDrawer( QWidget *parent, const char *name )
	    : QGLWidget(parent,name) {}

    protected:

        void initializeGL()
	{
	  // Set up the rendering context, define display lists etc.:
	  ...
	  glClearColor( 0.0, 0.0, 0.0, 0.0 );
	  glEnable(GL_DEPTH_TEST);
	  ...
	}

	void resizeGL( int w, int h )
	{
	  // setup viewport, projection etc.:
	  glViewport( 0, 0, (GLint)w, (GLint)h );
	  ...
	  glFrustum( ... );
	  ...
	}

        void paintGL()
	{
	  // draw the scene:
	  ...
	  glRotatef( ... );
	  glMaterialfv( ... );
	  glBegin( GL_QUADS );
	  glVertex3f( ... );
	  glVertex3f( ... );
	  ...
	  glEnd();
	  ...
	}

    };
  \endcode

  If you need to trigger a repaint from other places than paintGL() (a
  typical example is when using \link QTimer timers\endlink to animate
  scenes), you should call the widget's updateGL() function.

  When paintGL(), resizeGL() or initializeGL() is called, your
  widget's OpenGL rendering context has been made current.  If you
  need to call the standard OpenGL API functions from other places
  (e.g. in your widget's constructor), you must call makeCurrent()
  first.

  QGLWidget provides advanced functions for requesting a new display
  \link QGLFormat format\endlink, and you can even set a new rendering
  \link QGLContext context\endlink.

  You can achieve sharing of OpenGL display lists between QGLWidgets,
  see the documentation of the QGLWidget constructors for details.
*/


/*!
  Constructs an OpenGL widget with a \a parent widget and a \a name.

  The \link QGLFormat::defaultFormat() default format\endlink is
  used. The widget will be \link isValid() invalid\endlink if the
  system has no \link QGLFormat::hasOpenGL() OpenGL support\endlink.

  The \e parent, \e name and \e f arguments are passed to the QWidget
  constructor.

  If the \a shareWidget parameter points to a valid QGLWidget, this
  widget will share OpenGL display lists with \a shareWidget. Note: If
  this widget and \a shareWidget has different \link format()
  formats\endlink, display list sharing may fail. You can check
  whether display list sharing succeeded by using the isSharing()
  method.

  Note: Initialization of OpenGL rendering state etc. should be done
  by overriding the initializeGL() function, not in the constructor of
  your QGLWidget subclass.

  \sa QGLFormat::defaultFormat()
*/

QGLWidget::QGLWidget( QWidget *parent, const char *name,
		      const QGLWidget* shareWidget, WFlags f )
    : QWidget(parent, name, f)
{
    glcx = 0;
    autoSwap = TRUE;
    if ( shareWidget )
	setContext( new QGLContext( QGLFormat::defaultFormat(), this ),
		    shareWidget->context() );
    else
	setContext( new QGLContext( QGLFormat::defaultFormat(), this ) );
    setBackgroundMode( NoBackground );
}


/*!
  Constructs an OpenGL widget with a \a parent widget and a \a name.

  The \a format argument specifies the desired \link QGLFormat
  rendering options \endlink. If the underlying OpenGL/Window system
  cannot satisfy all the features requested in \a format, the nearest
  subset of features will be used. After creation, the format() method
  will return the actual format obtained.

  The widget will be \link isValid() invalid\endlink if the
  system has no \link QGLFormat::hasOpenGL() OpenGL support\endlink.

  The \e parent, \e name and \e f arguments are passed to the QWidget
  constructor.

  If the \a shareWidget parameter points to a valid QGLWidget, this
  widget will share OpenGL display lists with \a shareWidget. Note: If
  this widget and \a shareWidget has different \link format()
  formats\endlink, display list sharing may fail. You can check
  whether display list sharing succeeded by using the isSharing()
  method.

  Note: Initialization of OpenGL rendering state etc. should be done
  by overriding the initializeGL() function, not in the constructor of
  your QGLWidget subclass.

  \sa QGLFormat::defaultFormat(), isValid()
*/

QGLWidget::QGLWidget( const QGLFormat &format, QWidget *parent,
		      const char *name, const QGLWidget* shareWidget,
		      WFlags f )
    : QWidget(parent, name, f)
{
    glcx = 0;
    autoSwap = TRUE;
    if ( shareWidget )
	setContext( new QGLContext( format, this ), shareWidget->context() );
    else
	setContext( new QGLContext( format, this ) );
    setBackgroundMode( NoBackground );
}


/*!
  Destroys the widget.
*/

QGLWidget::~QGLWidget()
{
#if defined(GLX_MESA_release_buffers)
    bool doRelease = ( glcx && glcx->windowCreated() );
#endif
    delete glcx;
#if defined(GLX_MESA_release_buffers)
    if ( doRelease )
	glXReleaseBuffersMESA( dpy, winId() );
#endif
}





/*!
  \fn QGLFormat QGLWidget::format() const
  Returns the format of the contained GL rendering context.
  \sa setFormat()
*/

/*!
  \fn bool QGLWidget::doubleBuffer() const
  Returns TRUE if the contained GL rendering context has double buffering.
  \sa QGLFormat::doubleBuffer()
*/

/*!
  \fn void QGLWidget::setAutoBufferSwap( bool on ) 

  Turns on or off the automatic GL buffer swapping. If on, and the
  widget is using a double-buffered format, the background and
  foreground GL buffers will automatically be swapped after each time
  the paintGL() function has been called.
  
  The buffer auto-swapping is on by default.

  \sa autoBufferSwap(), doubleBuffer(), swapBuffers()
*/

/*!
  \fn bool QGLWidget::autoBufferSwap() const 

  Returns TRUE if the widget is doing automatic GL buffer swapping.

  \sa setAutoBufferSwap()
*/

/*!
  \fn bool QGLWidget::isValid() const 

  Returns TRUE if the widget has a valid GL rendering context. A
  widget will be invalid if the system has no \link
  QGLFormat::hasOpenGL() OpenGL support\endlink.

*/

bool QGLWidget::isValid() const
{
    return glcx->isValid();
}

/*!
  \fn bool QGLWidget::isSharing() const 

  Returns TRUE if display list sharing with another QGLWidget was
  requested in the constructor, and the GL system was able to provide
  it. The GL system may fail to provide display list sharing if the
  two QGLWidgets use different formats.

  \sa format()
*/

bool QGLWidget::isSharing() const
{
    return glcx->isSharing();
}

/*!
  \fn void QGLWidget::makeCurrent()

  Makes this widget the current widget for OpenGL
  operations. I.e. makes this widget's rendering context the current
  OpenGL rendering context.
*/

void QGLWidget::makeCurrent()
{
    glcx->makeCurrent();
}

/*!
  \fn void QGLWidget::swapBuffers()
  Swaps the screen contents with an off-screen buffer. Works only if
  the widget's format specifies double buffer mode.

  Normally, there is no need to explicitly call this function, because
  it is done automatically after each widget repaint, i.e. after each
  time paintGL() has been executed.

  \sa doubleBuffer(), setAutoBufferSwap(), QGLFormat::setDoubleBuffer()
*/

void QGLWidget::swapBuffers()
{
    glcx->swapBuffers();
}


/*!
  Sets a new format for this widget.

  If the underlying OpenGL/Window system cannot satisfy all the
  features requested in \a format, the nearest subset of features will
  be used. After creation, the format() method will return the actual
  rendering context format obtained.

  The widget will be assigned a new QGLContext, and the initializeGL()
  function will be executed for this new context before the first
  resizeGL() or paintGL().

  This method will try to keep any existing display list sharing with
  other QGLWidgets, but it may fail. Use isSharing() to test.

  \sa format(), setContext(), isSharing(), isValid()
*/

void QGLWidget::setFormat( const QGLFormat &format )
{
    setContext( new QGLContext(format,this) );
}




/*!
  \fn const QGLContext *QGLWidget::context() const
  Returns the current context.
  \sa setContext()
*/

/*!
  Sets a new context for this widget. The QGLContext \a context must
  be created using \e new. QGLWidget will delete \a context when
  another context is set or when the widget is destroyed.

  If \a context is invalid, QGLContext::create() is performed on
  it. The initializeGL() function will then be executed for the new
  context before the first resizeGL() or paintGL().

  If \a context is invalid, this method will try to keep any existing
  display list sharing with other QGLWidgets this widget currently
  has, or (if \a shareContext points to a valid context) start display
  list sharing with that context, but it may fail. Use isSharing() to
  test.

  If \a deleteOldContext is TRUE (the default), the existing context
  will be deleted. You may use FALSE here if you have kept a pointer
  to the old context (as returned by context()), and want to restore
  that context later.

  \sa context(), setFormat(), isSharing()
*/

void QGLWidget::setContext( QGLContext *context,
			    const QGLContext* shareContext,
			    bool deleteOldContext )
{
    if ( context == 0 ) {
#if defined(CHECK_NULL)
	warning( "QGLWidget::setContext: Cannot set null context" );
#endif
	return;
    }
    if ( !context->deviceIsPixmap() && context->device() != this ) {
#if defined(CHECK_STATE)
	warning( "QGLWidget::setContext: Context must refer to this widget" );
#endif
	return;
    }

    if ( glcx )
	glcx->doneCurrent();

    QGLContext* oldcx = glcx;
    glcx = context;

#if defined(Q_WGL)
    if ( oldcx && oldcx->windowCreated() && !glcx->deviceIsPixmap() && 
	 !glcx->windowCreated() ) {
	// We already have a context and must therefore create a new
	// window since Windows does not permit setting a new OpenGL
	// context for a window that already has one set.
	destroy( TRUE, TRUE );
	create( 0, TRUE, TRUE );
    }
#endif

    bool createFailed = FALSE;
    if ( !glcx->isValid() ) {
	if ( !glcx->create( shareContext ? shareContext : oldcx ) )
	    createFailed = TRUE;
    }
    if ( deleteOldContext )
	delete oldcx;
    if ( createFailed )
	return;

#if defined(Q_GLX)
    if ( glcx->windowCreated() || glcx->deviceIsPixmap() )
	return;

    bool visible = isVisible();
    if ( visible )
	hide();

    XVisualInfo *vi = (XVisualInfo*)glcx->vi;
    XSetWindowAttributes a;

    a.colormap = choose_cmap( dpy, vi );	// find best colormap
    a.background_pixel = backgroundColor().pixel();
    a.border_pixel = black.pixel();
    Window p = RootWindow( dpy, DefaultScreen(dpy) );
    if ( parentWidget() )
	p = parentWidget()->winId();
    Window w = XCreateWindow( dpy, p,  x(), y(), width(), height(),
			      0, vi->depth, InputOutput,  vi->visual,
			      CWBackPixel|CWBorderPixel|CWColormap, &a );

    Window *cmw;
    Window *cmwret;
    int count;
    if ( XGetWMColormapWindows(dpy,topLevelWidget()->winId(),&cmwret,&count) ){
	cmw = new Window[count+1];
        memcpy( (char *)cmw, (char *)cmwret, sizeof(Window)*count );
	XFree( (char *)cmwret );
	int i;
	for ( i=0; i<count; i++ ) {
	    if ( cmw[i] == winId() ) {		// replace old window
		cmw[i] = w;
		break;
	    }
	}
	if ( i >= count )			// append new window
	    cmw[count++] = w;
    } else {
	count = 1;
	cmw = new Window[count];
	cmw[0] = w;
    }

#if defined(GLX_MESA_release_buffers)
    if ( oldcx && oldcx->windowCreated() )
	glXReleaseBuffersMESA( dpy, winId() );
#endif
    create( w );

    XSetWMColormapWindows( dpy, topLevelWidget()->winId(), cmw, count );
    delete [] cmw;

    if ( visible )
	show();
    XFlush( dpy );
    glcx->setWindowCreated( TRUE );
#endif // Q_GLX
}


/*!
  \fn void QGLWidget::updateGL()
  Updates the widget by calling glDraw().
*/

void QGLWidget::updateGL()
{
    glDraw();
}

/*!
  This virtual function is called one time before the first call to
  paintGL() or resizeGL(), and then one time whenever the widget has
  been assigned a new QGLContext.  Reimplement it in a subclass.

  This function should take care of setting any required OpenGL
  context rendering flags, defining display lists, etc.

  There is no need to call makeCurrent() because this has already been
  done when this function is called.
*/

void QGLWidget::initializeGL()
{
}


/*!
  This virtual function is called whenever the widget needs to be painted.
  Reimplement it in a subclass.

  There is no need to call makeCurrent() because this has already been
  done when this function is called.
*/

void QGLWidget::paintGL()
{
}


/*!
  \fn void QGLWidget::resizeGL( int width , int height )
  This virtual function is called whenever the widget has been resized.
  Reimplement it in a subclass.

  There is no need to call makeCurrent() because this has already been
  done when this function is called.
*/

void QGLWidget::resizeGL( int, int )
{
}




/*!
  Handles paint events. Will cause the virtual paintGL() fucntion to
  be called, initializing first as necessary.
*/

void QGLWidget::paintEvent( QPaintEvent * )
{
    glDraw();
}


/*!
  Handles resize events. Calls the virtual function resizeGL().
*/

void QGLWidget::resizeEvent( QResizeEvent * )
{
    makeCurrent();
    if ( !glcx->initialized() )
	glInit();
#if defined(Q_GLX)
    glXWaitX();
#endif
    resizeGL( width(), height() );
}



/*!
  Renders the current scene on a pixmap and returns it.

  You may use this method on both visible and invisible QGLWidgets.

  This method will create a pixmap and a temporary QGLContext to
  render on it. Then, initializeGL(), resizeGL(), and paintGL() are
  called on this context. Finally, the widget's original GL context is
  restored.

  The size of the pixmap will be width \a w and height \a h. If any of
  those are 0 (the default), the pixmap will have the same size as the
  widget.

  If \a useContext is TRUE, this method will try to be more efficient
  by using the existing GL context to render the pixmap. The default
  is FALSE. Use only if you know what you are doing.

  \bug May give unexpected results if the depth of the GL rendering
  context is different from the depth of the desktop.
*/

QPixmap QGLWidget::renderPixmap( int w, int h, bool useContext )
{
    QPixmap nullPm;
    QSize sz = size();
    if ( (w > 0) && (h > 0) )
	sz = QSize( w, h );
    QPixmap pm( sz );
    glcx->doneCurrent();
    bool success = TRUE;

#if defined(Q_GLX)
    if ( useContext && isValid() && 
	 ( ((XVisualInfo*)glcx->vi)->depth == pm.depth() ) ) {
	GLXPixmap glPm;
#ifdef GLX_MESA_pixmap_colormap
	glPm = glXCreateGLXPixmapMESA( x11Display(),
				       (XVisualInfo*)glcx->vi,
				       (Pixmap)pm.handle(),
				       choose_cmap( pm.x11Display(),
						    (XVisualInfo*)glcx->vi ) );
#else
	glPm = (Q_UINT32)glXCreateGLXPixmap( x11Display(),
					    (XVisualInfo*)glcx->vi,
					    (Pixmap)pm.handle() );
#endif
	if ( !glXMakeCurrent( x11Display(), glPm, (GLXContext)glcx->cx ) ) {
	    glXDestroyGLXPixmap( x11Display(), glPm );
	    if ( !format().directRendering() )	// may be problem; try without
		return nullPm;			// Something else is wrong
	}
	else {
	    glDrawBuffer( GL_FRONT_LEFT );
	    if ( !glcx->initialized() )
		glInit();
	    resizeGL( pm.width(), pm.height() );
	    paintGL();
	    glFlush();
	    makeCurrent();
	    glXDestroyGLXPixmap( x11Display(), glPm );
	    resizeGL( width(), height() );
	    return pm;
	}
    }
#endif
    QGLFormat fmt = format();
    //    fmt.setDirectRendering( FALSE );		// No direct rendering
    QGLContext* pcx = new QGLContext( fmt, &pm );
    QGLContext* ocx = (QGLContext*)context();
    setContext( pcx, 0, FALSE );
    if ( pcx->isValid() )
	updateGL();
    else
	success = FALSE;
    setContext( ocx );				// Will delete pcx
    
    if ( success )
	return pm;
    else
	return nullPm;
}

/*!
  Initializes OpenGL for this widget's context. Calls the virtual
  function initializeGL().
*/

void QGLWidget::glInit()
{
    initializeGL();
    glcx->setInitialized( TRUE );
}


/*!
  Executes the virtual function paintGL(), initializing first as necessary.
*/

void QGLWidget::glDraw()
{
    makeCurrent();
    if ( glcx->deviceIsPixmap() )
	glDrawBuffer( GL_FRONT_LEFT );
    if ( !glcx->initialized() ) {
	glInit();
	QPaintDeviceMetrics dm( glcx->device() );
	resizeGL( dm.width(), dm.height() ); // New context needs this "resize"
    }
    paintGL();
    if ( doubleBuffer() ) {
	if ( autoSwap )
	    swapBuffers();
    }
    else {
	glFlush();
    }
}

/*****************************************************************************
  QGL classes overview documentation.
 *****************************************************************************/

/*! \page qgl.html

<title>Qt OpenGL Classes</title>
</head><body bgcolor="#ffffff">

<h1 align=center>Qt OpenGL Classes</h1>
<hr>


<h2>Introduction</h2>

OpenGL is a standard API for rendering 3D graphics.

OpenGL only deals with 3D rendering and provides little or no support
for GUI programming issues. The user interface for an OpenGL
application must be created with another toolkit, such as Motif on the
X platform, Microsoft Foundation Classes (MFC) under Windows -- or Qt
on both platforms.

The Qt OpenGL Extension provides integration of OpenGL with Qt, making
it very easy to use OpenGL rendering in a Qt application.


<h2>The QGL Classes</h2>

The OpenGL support classes in Qt are:
<ul>
<li> <strong>\link QGLWidget QGLWidget\endlink:</strong> An easy-to-use Qt
  widget for rendering OpenGL scenes.
<li> <strong>\link QGLContext QGLContext\endlink:</strong> Encapsulates an OpenGL rendering context.
<li> <strong>\link QGLFormat QGLFormat\endlink:</strong> Specifies the
display format of a rendering context.
</ul>

Many applications need only the high-level QGLWidget class. The other QGL
classes provide advanced features.
*/
