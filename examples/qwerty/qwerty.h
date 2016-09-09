/****************************************************************************
** $Id: qwerty.h,v 1.9 1998/06/22 15:22:08 warwick Exp $
**
** Copyright (C) 1992-1998 Troll Tech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#ifndef QWERTY_H
#define QWERTY_H

#include <qwidget.h>
#include <qmenubar.h>
#include <qmultilinedit.h>
#include <qprinter.h>

class Editor : public QWidget
{
    Q_OBJECT
public:
    Editor( QWidget *parent=0, const char *name=0 );
   ~Editor();

public slots:
    void newDoc();
    void load();
    void load( const char *fileName );
    void save();
    void print();
    void closeDoc();

protected:
    void resizeEvent( QResizeEvent * );
    void closeEvent( QCloseEvent * );

private:
    QMenuBar 	   *m;
    QMultiLineEdit *e;
    QPrinter        printer;
};

#endif // QWERTY_H
