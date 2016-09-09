/****************************************************************************
** $Id: menu.cpp,v 2.15 1998/06/16 11:39:33 warwick Exp $
**
** Copyright (C) 1992-1998 Troll Tech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#include "menu.h"
#include <qpopupmenu.h>
#include <qkeycode.h>
#include <qapplication.h>
#include <qmessagebox.h>
#include <qpixmap.h>

/* XPM */
static const char * p1_xpm[] = {
"16 16 3 1",
" 	c None",
".	c #000000000000",
"X	c #FFFFFFFF0000",
"                ",
"                ",
"         ....   ",
"        .XXXX.  ",
" .............. ",
" .XXXXXXXXXXXX. ",
" .XXXXXXXXXXXX. ",
" .XXXXXXXXXXXX. ",
" .XXXXXXXXXXXX. ",
" .XXXXXXXXXXXX. ",
" .XXXXXXXXXXXX. ",
" .XXXXXXXXXXXX. ",
" .XXXXXXXXXXXX. ",
" .XXXXXXXXXXXX. ",
" .............. ",
"                "};

/* XPM */
static const char * p2_xpm[] = {
"16 16 3 1",
" 	c None",
".	c #000000000000",
"X	c #FFFFFFFFFFFF",
"                ",
"   ......       ",
"   .XXX.X.      ",
"   .XXX.XX.     ",
"   .XXX.XXX.    ",
"   .XXX.....    ",
"   .XXXXXXX.    ",
"   .XXXXXXX.    ",
"   .XXXXXXX.    ",
"   .XXXXXXX.    ",
"   .XXXXXXX.    ",
"   .XXXXXXX.    ",
"   .XXXXXXX.    ",
"   .........    ",
"                ",
"                "};

/* XPM */
static const char * p3_xpm[] = {
"16 16 3 1",
" 	c None",
".	c #000000000000",
"X	c #FFFFFFFFFFFF",
"                ",
"                ",
"   .........    ",
"  ...........   ",
"  ........ ..   ",
"  ...........   ",
"  ...........   ",
"  ...........   ",
"  ...........   ",
"  ...XXXXX...   ",
"  ...XXXXX...   ",
"  ...XXXXX...   ",
"  ...XXXXX...   ",
"   .........    ",
"                ",
"                "};



MenuExample::MenuExample( QWidget *parent, const char *name )
    : QWidget( parent, name )
{
    QPixmap p1( p1_xpm );
    QPixmap p2( p2_xpm );
    QPixmap p3( p3_xpm );

    QPopupMenu *print = new QPopupMenu;
    CHECK_PTR( print );
    print->insertItem( "&Print to printer", this, SLOT(printer()) );
    print->insertItem( "Print to &file", this, SLOT(file()) );
    print->insertItem( "Print to fa&x", this, SLOT(fax()) );
    print->insertSeparator();
    print->insertItem( "Printer &Setup", this, SLOT(printerSetup()) );

    QPopupMenu *file = new QPopupMenu();
    CHECK_PTR( file );
    file->insertItem( p1, "&Open",  this, SLOT(open()), CTRL+Key_O );
    file->insertItem( p2, "&New", this, SLOT(news()), CTRL+Key_N );
    file->insertItem( p3, "&Save", this, SLOT(save()), CTRL+Key_S );
    file->insertItem( "&Close", this, SLOT(closeDoc()), CTRL+Key_W );
    file->insertSeparator();
    file->insertItem( "&Print", print, CTRL+Key_P );
    file->insertSeparator();
    file->insertItem( "E&xit",  qApp, SLOT(quit()), CTRL+Key_Q );

    QPopupMenu *edit = new QPopupMenu();
    CHECK_PTR( edit );
    int undoID = edit->insertItem( "&Undo", this, SLOT(undo()) );
    int redoID = edit->insertItem( "&Redo", this, SLOT(redo()) );
    edit->setItemEnabled( undoID, FALSE );
    edit->setItemEnabled( redoID, FALSE );

    options = new QPopupMenu();
    CHECK_PTR( options );
    options->insertItem( "&Normal Font", this, SLOT(normal()) );
    options->insertSeparator();
    boldID = options->insertItem( "&Bold", this, SLOT(bold()) );
    underlineID = options->insertItem( "&Underline", this, SLOT(underline()) );
    isBold = FALSE;
    isUnderline = FALSE;
    options->setCheckable( TRUE );
    

    QPopupMenu *help = new QPopupMenu;
    CHECK_PTR( help );
    help->insertItem( "&About", this, SLOT(about()), CTRL+Key_H );
    help->insertItem( "About &Qt", this, SLOT(aboutQt()) );

    menu = new QMenuBar( this );
    CHECK_PTR( menu );
    menu->insertItem( "&File", file );
    menu->insertItem( "&Edit", edit );
    menu->insertItem( "&Options", options );
    menu->insertSeparator();
    menu->insertItem( "&Help", help );
    menu->setSeparator( QMenuBar::InWindowsStyle );

    label = new QLabel( this );
    CHECK_PTR( label );
    label->setGeometry( 20, rect().center().y()-20, width()-40, 40 );
    label->setFrameStyle( QFrame::Box | QFrame::Raised );
    label->setLineWidth( 1 );
    label->setAlignment( AlignCenter );

    connect( this,  SIGNAL(explain(const char *)),
	     label, SLOT(setText(const char *)) );

    setMinimumSize( 100, 80 );
}


void MenuExample::open()
{
    emit explain( "File/Open selected" );
}


void MenuExample::news()
{
    emit explain( "File/New selected" );
}

void MenuExample::save()
{
    emit explain( "File/Save selected" );
}


void MenuExample::closeDoc()
{
    emit explain( "File/Close selected" );
}


void MenuExample::undo()
{
    emit explain( "Edit/Undo selected" );
}


void MenuExample::redo()
{
    emit explain( "Edit/Redo selected" );
}


void MenuExample::normal()
{
    isBold = FALSE;
    isUnderline = FALSE;
    options->setItemChecked( boldID, isBold );
    options->setItemChecked( underlineID, isUnderline );
    emit explain( "Options/Normal selected" );
}


void MenuExample::bold()
{
    isBold = !isBold;
    options->setItemChecked( boldID, isBold );
    emit explain( "Options/Bold selected" );
}


void MenuExample::underline()
{
    isUnderline = !isUnderline;
    options->setItemChecked( underlineID, isUnderline );
    emit explain( "Options/Underline selected" );
}


void MenuExample::about()
{
    QMessageBox::about( this, "Qt Menu Example",
			"This example demonstrates simple use of Qt menus.\n"
			"You can cut and paste lines from it to your own\n"
			"programs." );
}


void MenuExample::aboutQt()
{
    QMessageBox::aboutQt( this, "Qt Menu Example" );
}


void MenuExample::printer()
{
    emit explain( "File/Printer/Print selected" );
}

void MenuExample::file()
{
    emit explain( "File/Printer/Print To File selected" );
}

void MenuExample::fax()
{
    emit explain( "File/Printer/Print To Fax selected" );
}

void MenuExample::printerSetup()
{
    emit explain( "File/Printer/Printer Setup selected" );
}


void MenuExample::resizeEvent( QResizeEvent * )
{
    label->setGeometry( 20, rect().center().y()-20, width()-40, 40 );
}



int main( int argc, char ** argv )
{
    QApplication a( argc, argv );
    MenuExample m;

    a.setMainWidget( &m );
    m.show();
    return a.exec();
}
