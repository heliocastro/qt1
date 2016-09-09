/****************************************************************************
** $Id: qtetrix.h,v 2.4 1998/06/16 11:39:35 warwick Exp $
**
** Copyright (C) 1992-1998 Troll Tech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#ifndef QTETRIX_H
#define QTETRIX_H

#include "qtetrixb.h"
#include <qframe.h>
#include <qlcdnumber.h>
#include <qpushbutton.h>
#include <qpainter.h>


class ShowNextPiece : public QFrame
{
    Q_OBJECT
    friend class QTetrix;
public:
    ShowNextPiece( QWidget *parent=0, const char *name=0  );
public slots:
    void drawNextSquare( int x, int y,QColor *color );
signals:
    void update();
private:
    void paintEvent( QPaintEvent * );
    void resizeEvent( QResizeEvent * );
    
    int      blockWidth,blockHeight;
    int      xOffset,yOffset;
};


class QTetrix : public QWidget
{
    Q_OBJECT
public:
    QTetrix( QWidget *parent=0, const char *name=0 );
    void startGame() { board->startGame(); }

public slots:
    void gameOver();
    void quit();
private:
    void keyPressEvent( QKeyEvent *e ) { board->keyPressEvent(e); }

    QTetrixBoard  *board;
    ShowNextPiece *showNext;
    QLCDNumber    *showScore;
    QLCDNumber    *showLevel;
    QLCDNumber    *showLines;
    QPushButton   *quitButton;
    QPushButton   *startButton;
    QPushButton   *pauseButton;
};


void drawTetrixButton( QPainter *, int x, int y, int w, int h,
		       const QColor *color );


#endif
