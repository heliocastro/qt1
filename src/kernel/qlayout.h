/****************************************************************************
** $Id: qlayout.h,v 2.19.2.2 1998/08/21 19:13:23 hanord Exp $
**
** Definition of layout classes
**
** Created : 960416
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

#ifndef QLAYOUT_H
#define QLAYOUT_H

#ifndef QT_H
#include "qgmanager.h"
#include "qlist.h"
#endif // QT_H

class QMenuBar;

struct QLayoutData;


class Q_EXPORT QLayout : public QObject
{
    Q_OBJECT
public:
    virtual ~QLayout();
    int defaultBorder() const { return defBorder; }

    enum { unlimited = QCOORD_MAX };

    virtual bool activate();
    void freeze( int w, int h );
    void freeze() { freeze( 0, 0 ); }

    void  setMenuBar( QMenuBar *w );

    QWidget *mainWidget();

protected:
    QLayout( QWidget *parent,  int border,
	     int autoBorder, const char *name );
    QLayout( int autoBorder = -1, const char *name=0 );

    QGManager *basicManager() { return bm; }
    virtual QChain *mainVerticalChain() = 0;
    virtual QChain *mainHorizontalChain() = 0;

    virtual void initGM() = 0;
    void addChildLayout( QLayout *);

    static QChain *verChain( QLayout *l ) { return l->mainVerticalChain(); }
    static QChain *horChain( QLayout *l ) { return l->mainHorizontalChain(); }

private:
    QGManager * bm;
    int defBorder;
    bool    topLevel;

    QLayoutData *extraData;
private:	// Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QLayout( const QLayout & );
    QLayout &operator=( const QLayout & );
#endif

};


class Q_EXPORT QBoxLayout : public QLayout
{
    Q_OBJECT
public:
    enum Direction { LeftToRight, RightToLeft, TopToBottom, BottomToTop,
		     Down = TopToBottom, Up = BottomToTop };

    QBoxLayout( QWidget *parent, Direction, int border=0,
		int autoBorder = -1, const char *name=0 );

    QBoxLayout(	Direction, int autoBorder = -1,
		const char *name=0 );

    ~QBoxLayout();

    void addSpacing( int size );
    void addStretch( int stretch = 0 );
    void addWidget( QWidget *, int stretch = 0, int alignment = AlignCenter );
    void addLayout( QLayout *layout, int stretch = 0 );
    Direction direction() const { return (Direction)dir; }

    void addStrut( int );
protected:
    QChain *mainVerticalChain();
    QChain *mainHorizontalChain();
    void initGM();

private:
    void addB( QLayout *, int stretch );

    QGManager::Direction dir;
    QChain * parChain;
    QChain * serChain;
    bool    pristine;

private:	// Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QBoxLayout( const QBoxLayout & );
    QBoxLayout &operator=( const QBoxLayout & );
#endif

};


class Q_EXPORT QHBoxLayout : public QBoxLayout
{
    Q_OBJECT
public:
    QHBoxLayout( QWidget *parent, int border=0,
		int autoBorder = -1, const char *name=0 );

    QHBoxLayout( int autoBorder = -1, const char *name=0 );

    ~QHBoxLayout();
};



class Q_EXPORT QVBoxLayout : public QBoxLayout
{
    Q_OBJECT
public:
    QVBoxLayout( QWidget *parent, int border=0,
		int autoBorder = -1, const char *name=0 );

    QVBoxLayout( int autoBorder = -1, const char *name=0 );

    ~QVBoxLayout();
};



class Q_EXPORT QGridLayout : public QLayout
{
    Q_OBJECT
public:
    QGridLayout( QWidget *parent, int nRows, int nCols, int border=0,
		 int autoBorder = -1, const char *name=0 );
    QGridLayout( int nRows, int nCols, int autoBorder = -1,
		 const char *name=0 );
    ~QGridLayout();
    void addWidget( QWidget *, int row, int col, int align = 0 );
    void addMultiCellWidget( QWidget *, int fromRow, int toRow,
			       int fromCol, int toCol, int align = 0 );
    void addLayout( QLayout *layout, int row, int col);

    void setRowStretch( int row, int stretch );
    void setColStretch( int col, int stretch );
    void addRowSpacing( int row, int minsize );
    void addColSpacing( int col, int minsize );

    int numRows() const { return rr; }
    int numCols() const { return cc; }

    void expand( int rows, int cols );

protected:
    QChain *mainVerticalChain() { return verChain; }
    QChain *mainHorizontalChain() { return horChain; }
    void initGM();

private:
    QArray<QChain*> *rows;
    QArray<QChain*> *cols;

    QChain *horChain;
    QChain *verChain;
    void init ( int r, int c );

    int rr;
    int cc;

private:	// Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QGridLayout( const QGridLayout & );
    QGridLayout &operator=( const QGridLayout & );
#endif
};


#endif // QLAYOUT_H
