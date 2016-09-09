/****************************************************************************
** $Id: qscrollview.h,v 2.22.2.3 1998/10/01 17:04:00 warwick Exp $
**
** Definition of QScrollView class
**
** Created : 970523
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
#ifndef QSCROLLVIEW_H
#define QSCROLLVIEW_H

#ifndef QT_H
#include "qframe.h"
#include "qscrollbar.h"
#endif // QT_H

struct QScrollViewData;

class Q_EXPORT QScrollView : public QFrame
{
    Q_OBJECT
public:
    QScrollView(QWidget *parent=0, const char *name=0, WFlags f=0);
    ~QScrollView();

    enum ResizePolicy { Default, Manual, AutoOne };
    virtual void setResizePolicy( ResizePolicy );
    ResizePolicy resizePolicy() const;

    void addChild(QWidget* child);
    void removeChild(QWidget* child);
    virtual void addChild(QWidget* child, int x, int y);
    virtual void moveChild(QWidget* child, int x, int y);
    int childX(QWidget* child);
    int childY(QWidget* child);
    bool childIsVisible(QWidget* child);
    void showChild(QWidget* child, bool yes=TRUE);

    enum ScrollBarMode { Auto, AlwaysOff, AlwaysOn };

    ScrollBarMode vScrollBarMode() const;
    virtual void  setVScrollBarMode( ScrollBarMode );

    ScrollBarMode hScrollBarMode() const;
    virtual void  setHScrollBarMode( ScrollBarMode );

    QWidget*     cornerWidget() const;
    virtual void setCornerWidget(QWidget*);

    QScrollBar*  horizontalScrollBar();
    QScrollBar*  verticalScrollBar();
    QWidget*	 viewport();

    int		contentsWidth() const;
    int		contentsHeight() const;
    int		contentsX() const;
    int		contentsY() const;

    void	resize( int w, int h );
    void	resize( const QSize& );
    void	show();

signals:
    void	contentsMoving(int x, int y);

public slots:
    virtual void resizeContents( int w, int h );
    void	scrollBy( int dx, int dy );
    void        setContentsPos( int x, int y );
    void	ensureVisible(int x, int y);
    void	ensureVisible(int x, int y, int xmargin, int ymargin);
    void	center(int x, int y);
    void	center(int x, int y, float xmargin, float ymargin);

    void	updateScrollBars();

protected:
    void	resizeEvent(QResizeEvent*);
    bool	eventFilter( QObject *, QEvent *e );

    virtual void viewportPaintEvent( QPaintEvent* );
    virtual void viewportMousePressEvent( QMouseEvent* );
    virtual void viewportMouseReleaseEvent( QMouseEvent* );
    virtual void viewportMouseDoubleClickEvent( QMouseEvent* );
    virtual void viewportMouseMoveEvent( QMouseEvent* );

    virtual void drawContentsOffset(QPainter*, int ox, int oy,
		    int cx, int cy, int cw, int ch);
    void	frameChanged();

    void setMargins(int left, int top, int right, int bottom);
    int leftMargin() const;
    int topMargin() const;
    int rightMargin() const;
    int bottomMargin() const;

    bool focusNextPrevChild( bool next );

private:
    void moveContents(int x, int y);

    QScrollViewData* d;

private slots:
    void hslide(int);
    void vslide(int);

private:	// Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QScrollView( const QScrollView & );
    QScrollView &operator=( const QScrollView & );
#endif
    void changeFrameRect(const QRect&);
};

inline void QScrollView::addChild(QWidget* child)
{
    addChild(child,0,0);
}

#endif
