/****************************************************************************
** $Id: qsplitter.h,v 1.11.2.1 1998/08/19 16:02:43 agulbra Exp $
**
** Defintion of  QSplitter class
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
#ifndef QSPLITTER_H
#define QSPLITTER_H

#ifndef QT_H
#include "qframe.h"
#endif // QT_H

class QInternalSplitter;

class Q_EXPORT QSplitter : public QFrame
{
	Q_OBJECT
public:
    enum Orientation { Horizontal, Vertical };
    enum ResizeMode { Stretch, KeepSize };
    
    QSplitter( QWidget *parent=0, const char *name=0 );
    QSplitter( Orientation, QWidget *parent=0, const char *name=0 );

    void setOrientation( Orientation );
    Orientation orientation() const { return orient; }

    void setResizeMode( QWidget *w, ResizeMode );

    bool event( QEvent * );

    void setOpaqueResize( bool = TRUE );
    bool opaqueResize() const { return opaque; }

    void moveToFirst( QWidget * );
    void moveToLast( QWidget * );
    
    //void setHidden( QWidget *, bool );
    //bool isHidden( QWidget *) const;

    void refresh() { recalc( TRUE ); }
    
protected:
    void childInsertEvent( QChildEvent * );
    void childRemoveEvent( QChildEvent * );
    void layoutHintEvent( QEvent * );
    void resizeEvent( QResizeEvent * );

    void moveSplitter( QCOORD pos );
    virtual void drawSplitter( QPainter*, QCOORD x, QCOORD y,
			       QCOORD w, QCOORD h );

    int adjustPos( int );
    void setRubberband( int );
    // virtual int border2()

private:
    void init();
    void recalc( bool update = FALSE );
    int hit( QPoint p );
    void doResize();
    QWidget *splitterWidget();

    void startMoving();
    void moveTo( QPoint );
    void stopMoving( );



    QCOORD pick( const QPoint &p ) const
    { return orient == Horizontal ? p.x() : p.y(); }
    QCOORD pick( const QSize &s ) const
    { return orient == Horizontal ? s.width() : s.height(); }

    QCOORD trans( const QPoint &p ) const
    { return orient == Vertical ? p.x() : p.y(); }
    QCOORD trans( const QSize &s ) const
    { return orient == Vertical ? s.width() : s.height(); }

    QCOORD newpos() const;

    QWidget *w1;
    QWidget *w2;
    int moving;
    //    bool w1show;
    //    bool w2show;
    QWidget *fixedWidget;
    QInternalSplitter *d;
    bool opaque;

    Orientation orient;
    QCOORD bord; //half border
    friend class QInternalSplitter;
};


#endif //QSPLITTER_H
