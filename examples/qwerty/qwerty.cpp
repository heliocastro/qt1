/****************************************************************************
** $Id: qwerty.cpp,v 1.11.2.1 1998/10/27 17:19:03 hanord Exp $
**
** Copyright (C) 1992-1998 Troll Tech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#include "qwerty.h"
#include <qapplication.h>
#include <qfile.h>
#include <qfiledialog.h>
#include <qkeycode.h>
#include <qpopupmenu.h>
#include <qtextstream.h>
#include <qpainter.h>
#include <qmessagebox.h>
#include <qpaintdevicemetrics.h>
#include <qlist.h>

typedef QList<Editor> EditorList;

static EditorList *spawnedEditors = 0;		// list of  editors spawned by
						// Editor::newDoc()

Editor::Editor( QWidget * parent , const char * name )
    : QWidget( parent, name )
{
    m = new QMenuBar( this, "menu" );
    QPopupMenu * file = new QPopupMenu();
    CHECK_PTR( file );
    m->insertItem( "&File", file );

    file->insertItem( "New",   this, SLOT(newDoc()),   ALT+Key_N );
    file->insertItem( "Open",  this, SLOT(load()),     ALT+Key_O );
    file->insertItem( "Save",  this, SLOT(save()),     ALT+Key_S );
    file->insertSeparator();
    file->insertItem( "Print", this, SLOT(print()),    ALT+Key_P );
    file->insertSeparator();
    file->insertItem( "Close", this, SLOT(closeDoc()),ALT+Key_W );
    file->insertItem( "Quit",  qApp, SLOT(quit()),     ALT+Key_Q );

    e = new QMultiLineEdit( this, "editor" );
    e->setFocus();
}

Editor::~Editor()
{
    if ( spawnedEditors ) {
	spawnedEditors->removeRef( this );	 // does nothing if not in list
	if ( spawnedEditors->count() == 0 ) {
	    delete spawnedEditors;
	    spawnedEditors = 0;
	}
    }
}

void Editor::newDoc()
{
    if ( !spawnedEditors )
	spawnedEditors = new EditorList;
    Editor *ed = new Editor;
    spawnedEditors->append( ed );	       	// add to list of spawned eds
    ed->resize( 400, 400 );
    ed->show();
}

void Editor::load()
{
    QString fn = QFileDialog::getOpenFileName( 0, 0, this );
    if ( !fn.isEmpty() ) 
	load( fn );
}


void Editor::load( const char *fileName )
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
}

void Editor::save()
{
    QMessageBox::message( "Note", "Left as an exercise for the user." );
}

void Editor::print()
{
    const int MARGIN = 10;

    if ( printer.setup(this) ) {		// opens printer dialog
	QPainter p;
	p.begin( &printer );			// paint on printer
	p.setFont( e->font() );
	int yPos        = 0;			// y position for each line
	QFontMetrics fm = p.fontMetrics();
	QPaintDeviceMetrics metrics( &printer ); // need width/height
	                                         // of printer surface
	for( int i = 0 ; i < e->numLines() ; i++ ) {
	    if ( MARGIN + yPos > metrics.height() - MARGIN ) {
		printer.newPage();		// no more room on this page
		yPos = 0;			// back to top of page
	    }
	    p.drawText( MARGIN, MARGIN + yPos, 
			metrics.width(), fm.lineSpacing(),
			ExpandTabs | DontClip,
			e->textLine( i ) );
	    yPos = yPos + fm.lineSpacing();
	}
	p.end();				// send job to printer
    }
}

void Editor::closeDoc()
{
    close();					// will call closeEvent()
}

void Editor::resizeEvent( QResizeEvent * )
{
    if ( e && m )
	e->setGeometry( 0, m->height(), width(), height() - m->height() );
}

void Editor::closeEvent( QCloseEvent * )
{
    if ( spawnedEditors && 
	 spawnedEditors->findRef(this) != -1 ){	// Was it created by newDoc()?
	delete this;				// Goodbye cruel world!
    } else {
	hide();					// Original editor, just hide
    }
}
