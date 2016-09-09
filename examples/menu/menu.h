/****************************************************************************
** $Id: menu.h,v 2.7 1998/05/21 19:24:54 agulbra Exp $
**
** Copyright (C) 1992-1998 Troll Tech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#ifndef MENU_H
#define MENU_H

#include <qwidget.h>
#include <qmenubar.h>
#include <qlabel.h>


class MenuExample : public QWidget
{
    Q_OBJECT
public:
    MenuExample( QWidget *parent=0, const char *name=0 );

public slots:
    void open();
    void news();
    void save();
    void closeDoc();
    void undo();
    void redo();
    void normal();
    void bold();
    void underline();
    void about();
    void aboutQt();
    void printer();
    void file();
    void fax();
    void printerSetup();

protected:
    void    resizeEvent( QResizeEvent * );

signals:
    void    explain( const char * );

private:
    QMenuBar *menu;
    QLabel   *label;
    bool isBold;
    bool isUnderline;
    int boldID, underlineID;
    QPopupMenu *options;
};


#endif // MENU_H
