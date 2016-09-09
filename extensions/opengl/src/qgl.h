/****************************************************************************
** $Id: qgl.h,v 1.9.2.6 1999/01/28 12:26:14 aavit Exp $
**
** Definition of OpenGL classes for Qt
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

#ifndef QGL_H
#define QGL_H


#define QGL_VERSION	400
#define QGL_VERSION_STR	"4.0b"

const char *qGLVersion();


#ifndef QT_H
#include <qwidget.h>
#endif // QT_H

#if !(defined(Q_WGL) || defined(Q_GLX))
#if defined(_OS_WIN32_)
#define Q_WGL
#else
#define Q_GLX
#endif
#endif
#if defined(Q_WGL)
#include <windows.h>
#endif
#include <GL/gl.h>
#include <GL/glu.h>


class QPixmap;

// Namespace class:
class QGL
{
public:
    enum FormatOption {
	DoubleBuffer		= 0x0001,
	DepthBuffer		= 0x0002,
	Rgba			= 0x0004,
	AlphaChannel		= 0x0008,
	AccumBuffer		= 0x0010,
	StencilBuffer		= 0x0020,
	StereoBuffers		= 0x0040,
	DirectRendering		= 0x0080,
	SingleBuffer		= DoubleBuffer	<< 16,
	NoDepthBuffer		= DepthBuffer	<< 16,
	ColorIndex		= Rgba		<< 16,
	NoAlphaChannel		= AlphaChannel	<< 16,
	NoAccumBuffer		= AccumBuffer	<< 16,
	NoStencilBuffer		= StencilBuffer	<< 16,
	NoStereoBuffers		= StereoBuffers	<< 16,
	IndirectRendering	= DirectRendering << 16
    };
};



class QGLFormat : public QGL
{
public:
    QGLFormat();
    QGLFormat( int options );

    bool    		doubleBuffer() const;
    void    		setDoubleBuffer( bool enable );
    bool    		depth() const;
    void    		setDepth( bool enable );
    bool    		rgba() const;
    void    		setRgba( bool enable );
    bool    		alpha() const;
    void    		setAlpha( bool enable );
    bool    		accum() const;
    void    		setAccum( bool enable );
    bool    		stencil() const;
    void    		setStencil( bool enable );
    bool    		stereo() const;
    void    		setStereo( bool enable );
    bool    		directRendering() const;
    void    		setDirectRendering( bool enable );

    void		setOption( FormatOption opt );
    bool		testOption( FormatOption opt ) const;
    
    static QGLFormat	defaultFormat();
    static void		setDefaultFormat( const QGLFormat& f );

    static bool		hasOpenGL();

    friend bool		operator==( const QGLFormat&, const QGLFormat& );
    friend bool		operator!=( const QGLFormat&, const QGLFormat& );
    
private:
    uint opts;
};



class QGLContext : public QGL
{
public:
    QGLContext( const QGLFormat& format, QPaintDevice* device );
    virtual ~QGLContext();

    virtual bool	create( const QGLContext* shareContext = 0 );
    bool		isValid() const;
    bool		isSharing() const;
    virtual void	reset();

    QGLFormat		format() const;
    virtual void	setFormat( const QGLFormat& format );

    virtual void	makeCurrent();
    virtual void	swapBuffers();

    QPaintDevice*	device() const;

protected:
    virtual bool	chooseContext( const QGLContext* shareContext = 0 );
    virtual void	doneCurrent();

#if defined(Q_WGL)
    virtual int		choosePixelFormat( void* pfd, HANDLE pdc );
#elif defined(Q_GLX)
    virtual void*	tryVisual( const QGLFormat& f );
    virtual void*	chooseVisual();
#endif

    bool		deviceIsPixmap() const;
    bool		windowCreated() const;
    void		setWindowCreated( bool on );
    bool		initialized() const;
    void		setInitialized( bool on );

protected:
#if defined(Q_WGL)
    HANDLE		rc;
    HANDLE		dc;
    HANDLE		win;
#elif defined(Q_GLX)
    void*		vi;
    void*		cx;
    Q_UINT32		gpm;
#endif

    QGLFormat		glFormat;
    
private:
    bool		valid;
    bool		sharing;
    bool		initDone;
    bool		crWin;
    QPaintDevice*	paintDevice;

private:
    friend class QGLWidget;
    
private:	// Disabled copy constructor and operator=
    QGLContext() {}
    QGLContext( const QGLContext& ) {}
    QGLContext&		operator=( const QGLContext& ) { return *this; }
};


class QGLWidget : public QWidget, public QGL
{
    Q_OBJECT
public:
    QGLWidget( QWidget* parent=0, const char* name=0,
	       const QGLWidget* shareWidget = 0, WFlags f=0 );
    QGLWidget( const QGLFormat& format, QWidget* parent=0, const char* name=0,
	       const QGLWidget* shareWidget = 0, WFlags f=0 );
   ~QGLWidget();

    bool		isValid() const;
    bool		isSharing() const;
    virtual void	makeCurrent();

    bool		doubleBuffer() const;
    virtual void	swapBuffers();

    QGLFormat		format() const;
    virtual void	setFormat( const QGLFormat& format );

    const QGLContext*	context() const;
    virtual void	setContext( QGLContext* context,
				    const QGLContext* shareContext = 0,
				    bool deleteOldContext = TRUE );

    virtual QPixmap	renderPixmap( int w = 0, int h = 0,
				      bool useContext = FALSE );

public slots:
    virtual void	updateGL();

protected:
    virtual void	initializeGL();
    virtual void	paintGL();
    virtual void	resizeGL( int w, int h );

    void		setAutoBufferSwap( bool on );
    bool		autoBufferSwap() const;

    void		paintEvent( QPaintEvent* );
    void		resizeEvent( QResizeEvent* );

    virtual void	glInit();
    virtual void	glDraw();
    
private:
    QGLContext*		glcx;
    bool		autoSwap;

private:	// Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QGLWidget( const QGLWidget& );
    QGLWidget&		operator=( const QGLWidget& );
#endif
};


//
// QGLFormat inline functions
//

inline bool QGLFormat::doubleBuffer() const
{
    return testOption( DoubleBuffer );
}

inline bool QGLFormat::depth() const
{
    return testOption( DepthBuffer );
}

inline bool QGLFormat::rgba() const
{
    return testOption( Rgba );
}

inline bool QGLFormat::alpha() const
{
    return testOption( AlphaChannel );
}

inline bool QGLFormat::accum() const
{
    return testOption( AccumBuffer );
}

inline bool QGLFormat::stencil() const
{
    return testOption( StencilBuffer );
}

inline bool QGLFormat::stereo() const
{
    return testOption( StereoBuffers );
}

inline bool QGLFormat::directRendering() const
{
    return testOption( DirectRendering );
}

//
// QGLContext inline functions
//

inline bool QGLContext::isValid() const
{
    return valid;
}
inline bool QGLContext::isSharing() const
{
    return sharing;
}
inline QGLFormat QGLContext::format() const
{
    return glFormat;
}

inline QPaintDevice* QGLContext::device() const
{
    return paintDevice;
}

inline bool QGLContext::deviceIsPixmap() const
{
    return paintDevice->devType() == PDT_PIXMAP;
}


inline bool QGLContext::windowCreated() const
{
    return crWin;
}


inline void QGLContext::setWindowCreated( bool on )
{
    crWin = on;
}

inline bool QGLContext::initialized() const
{
    return initDone;
}

inline void QGLContext::setInitialized( bool on )
{
    initDone = on;
}

//
// QGLWidget inline functions
//

inline QGLFormat QGLWidget::format() const
{
    return glcx->format();
}

inline const QGLContext *QGLWidget::context() const
{
    return glcx;
}

inline bool QGLWidget::doubleBuffer() const
{
    return glcx->format().doubleBuffer();
}

inline void QGLWidget::setAutoBufferSwap( bool on )
{
    autoSwap = on;
}

inline bool QGLWidget::autoBufferSwap() const
{
    return autoSwap;
}


#endif // QGL_H
