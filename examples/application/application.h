/****************************************************************************
** $Id: application.h,v 1.5 1998/06/23 09:47:41 warwick Exp $
**
** Copyright (C) 1992-1998 Troll Tech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#ifndef APPLICATION_H
#define APPLICATION_H

#include <qmainwindow.h>

class QMultiLineEdit;
class QToolBar;
class QPopupMenu;

class ApplicationWindow: public QMainWindow
{
    Q_OBJECT
public:
    ApplicationWindow();
    ~ApplicationWindow();
    
private slots:
    void newDoc();
    void load();
    void load( const char *fileName );
    void save();
    void print();
    void closeDoc();

    void toggleMenuBar();
    void toggleStatusBar();
    void toggleToolBar();

private:
    QPrinter *printer;
    QMultiLineEdit *e;
    QToolBar *fileTools;
    QPopupMenu *controls;
    int mb, tb, sb;
};


#endif
