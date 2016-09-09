/****************************************************************************
** $Id: application.cpp,v 1.16.2.1 1999/02/01 19:58:21 hanord Exp $
**
** Copyright (C) 1992-1998 Troll Tech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#include "application.h"

#include <qpixmap.h>
#include <qtoolbar.h>
#include <qtoolbutton.h>
#include <qpopupmenu.h>
#include <qmenubar.h>
#include <qkeycode.h>
#include <qmultilinedit.h>
#include <qfile.h>
#include <qfiledialog.h>
#include <qstatusbar.h>
#include <qmessagebox.h>
#include <qprinter.h>
#include <qapplication.h>
#include <qaccel.h>
#include <qtextstream.h>
#include <qpainter.h>
#include <qpaintdevicemetrics.h>
#include <qwhatsthis.h>

#include "filesave.xpm"
#include "fileopen.xpm"
#include "fileprint.xpm"

const char * fileOpenText = "Click this button to open a new file.\n\n"
"You can also select the Open command from the File menu.";
const char * fileSaveText = "Click this button to save the file you are "
"editing.  You will be prompted for a file name.\n\n"
"You can also select the Save command from the File menu.\n\n"
"Note that implementing this function is left as an exercise for the reader.";
const char * filePrintText = "Click this button to print the file you "
"are editing.\n\n"
"You can also select the Print command from the File menu.";

ApplicationWindow::ApplicationWindow()
    : QMainWindow( 0, "example application main window" )
{
    printer = new QPrinter;
    printer->setMinMax( 1, 10 );
    QPixmap openIcon, saveIcon, printIcon;

    fileTools = new QToolBar( this, "file operations" );

    openIcon = QPixmap( fileopen );
    QToolButton * fileOpen = new QToolButton( openIcon, "Open File", 0,
					      this, SLOT(load()),
					      fileTools, "open file" );

    saveIcon = QPixmap( filesave );
    QToolButton * fileSave = new QToolButton( saveIcon, "Save File", 0,
					      this, SLOT(save()),
					      fileTools, "save file" );

    printIcon = QPixmap( fileprint );
    QToolButton * filePrint = new QToolButton( printIcon, "Print File", 0,
					       this, SLOT(print()),
					       fileTools, "print file" );

    (void)QWhatsThis::whatsThisButton( fileTools );
    QWhatsThis::add( fileOpen, fileOpenText, FALSE );
    QWhatsThis::add( fileSave, fileSaveText, FALSE );
    QWhatsThis::add( filePrint, filePrintText, FALSE );
    
    QPopupMenu * file = new QPopupMenu();
    menuBar()->insertItem( "&File", file );

    file->insertItem( "New", this, SLOT(newDoc()), CTRL+Key_N );
    file->insertItem( openIcon, "Open", this, SLOT(load()), CTRL+Key_O );
    file->insertItem( saveIcon, "Save", this, SLOT(save()), CTRL+Key_S );
    file->insertSeparator();
    file->insertItem( printIcon, "Print", this, SLOT(print()), CTRL+Key_P );
    file->insertSeparator();
    file->insertItem( "Close", this, SLOT(closeDoc()), CTRL+Key_W );
    file->insertItem( "Quit", qApp, SLOT(quit()), CTRL+Key_Q );

    controls = new QPopupMenu();
    menuBar()->insertItem( "&Controls", controls );

    mb = controls->insertItem( "Menu bar", this, SLOT(toggleMenuBar()), CTRL+Key_M);
    // Now an accelerator for when the menubar is invisible!
    QAccel* a = new QAccel(this);
    a->connectItem( a->insertItem( CTRL+Key_M ), this, SLOT(toggleMenuBar()) );

    tb = controls->insertItem( "Tool bar", this, SLOT(toggleToolBar()), CTRL+Key_T);
    sb = controls->insertItem( "Status bar", this, SLOT(toggleStatusBar()), CTRL+Key_B);
    controls->setCheckable( TRUE );
    controls->setItemChecked( mb, TRUE );
    controls->setItemChecked( tb, TRUE );
    controls->setItemChecked( sb, TRUE );

    e = new QMultiLineEdit( this, "editor" );
    e->setFocus();
    setCentralWidget( e );
    statusBar()->message( "Ready", 2000 );
}


/*! Destroys the object and frees any allocated resources.

*/

ApplicationWindow::~ApplicationWindow()
{
    delete printer;
}



void ApplicationWindow::newDoc()
{
    ApplicationWindow *ed = new ApplicationWindow;
    ed->resize( 400, 400 );
    ed->show();
}

void ApplicationWindow::load()
{
    QString fn = QFileDialog::getOpenFileName(0,0,this);
    if ( !fn.isEmpty() )
	load( fn );
    else
	statusBar()->message( "Loading aborted", 2000 );
}


void ApplicationWindow::load( const char *fileName )
{
    QFile f( fileName );
    if ( !f.open( IO_ReadOnly ) )
	return;

    e->setAutoUpdate( FALSE );
    e->clear();

    QTextStream t(&f);
    while ( !t.eof() ) {
	QString s = t.readLine();
	e->append( s );
    }
    f.close();

    e->setAutoUpdate( TRUE );
    e->repaint();
    setCaption( fileName );
    QString s;
    s.sprintf( "Loaded document %s", fileName );
    statusBar()->message( s, 2000 );
}


void ApplicationWindow::save()
{
    statusBar()->message( "File->Save is not implemented" );
    QMessageBox::message( "Note", "Left as an exercise for the user." );
}


void ApplicationWindow::print()
{
    const int MARGIN = 10;
    int pageNo = 1;

    if ( printer->setup(this) ) {		// printer dialog
	statusBar()->message( "Printing..." );
	QPainter p;
	p.begin( printer );			// paint on printer
	p.setFont( e->font() );
	int yPos        = 0;			// y position for each line
	QFontMetrics fm = p.fontMetrics();
	QPaintDeviceMetrics metrics( printer ); // need width/height
	                                         // of printer surface
	for( int i = 0 ; i < e->numLines() ; i++ ) {
	    if ( MARGIN + yPos > metrics.height() - MARGIN ) {
		QString msg;
		msg.sprintf( "Printing (page %d)...", ++pageNo );
		statusBar()->message( msg );
		printer->newPage();		// no more room on this page
		yPos = 0;			// back to top of page
	    }
	    p.drawText( MARGIN, MARGIN + yPos,
			metrics.width(), fm.lineSpacing(),
			ExpandTabs | DontClip,
			e->textLine( i ) );
	    yPos = yPos + fm.lineSpacing();
	}
	p.end();				// send job to printer
	statusBar()->message( "Printing completed", 2000 );
    } else {
	statusBar()->message( "Printing aborted", 2000 );
    }

}

void ApplicationWindow::closeDoc()
{
    close( TRUE ); // close AND DELETE!
}

void ApplicationWindow::toggleMenuBar()
{
    if ( menuBar()->isVisible() ) {
	menuBar()->hide();
	controls->setItemChecked( mb, FALSE );
    } else {
	menuBar()->show();
	controls->setItemChecked( mb, TRUE );
    }
}

void ApplicationWindow::toggleToolBar()
{
    if ( fileTools->isVisible() ) {
	fileTools->hide();
	controls->setItemChecked( tb, FALSE );
    } else {
	fileTools->show();
	controls->setItemChecked( tb, TRUE );
    }
}

void ApplicationWindow::toggleStatusBar()
{
    if ( statusBar()->isVisible() ) {
	statusBar()->hide();
	controls->setItemChecked( sb, FALSE );
    } else {
	statusBar()->show();
	controls->setItemChecked( sb, TRUE );
    }
}
