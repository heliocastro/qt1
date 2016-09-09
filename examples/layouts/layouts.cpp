/****************************************************************************
** $Id: layouts.cpp,v 1.6 1998/06/29 21:53:43 aavit Exp $
**
** Copyright (C) 1992-1998 Troll Tech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#include <qapplication.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qcolor.h>
#include <qgroupbox.h> 
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qlayout.h>

#include "qtbuttonrow.h"
#include "qtlabelled.h"
#include "qtgrid.h"
#include "qthbox.h"
#include "qtvbox.h"

class StrongHeading : public QtHBox {
public:
    StrongHeading(const char* label, const char* desc, QWidget* parent=0, const char* name=0) :
	QtHBox(parent, name)
    {
	QLabel* l = new QLabel(label, this);
	QLabel* d = new QLabel(desc, this);
	QPalette p = palette();
	QColorGroup n = palette().normal();
	QColorGroup g(n.background(), n.foreground(), n.light(), n.dark(),
		      n.mid(), n.background(), n.base());
	p.setNormal( g );
	setPalette(p);
	l->setPalette(p);
	d->setPalette(p);
	l->setMargin(3);
	d->setMargin(2);

	QFont bold = *QApplication::font();
	bold.setBold(TRUE);
	bold.setPointSize(bold.pointSize()+2);
	l->setFont( bold );

	l->setFixedSize(l->sizeHint());
    }
};




static QWidget *navigatorDialog()
{
    QtVBox *page = new QtVBox;
    (void) new StrongHeading( "Navigator",   "Specify the home page location",
			    page );
    QtLabelled *frame = new QtLabelled( "Browser starts with", page );
    QtVBox *box = new QtVBox( frame );
    (void) new QRadioButton( "Blank page", box );
    (void) new QRadioButton( "Home page", box );
    (void) new QRadioButton( "Last page visited", box );

    frame = new QtLabelled( "Home Page", page );
    box = new QtVBox( frame );
    (void) new QLabel( "Clicking the Home button will take you to this page",
		box );

    QtHBox *hbox = new QtHBox( box );
    (void) new QLabel( "Location:", hbox );
    (void) new QLineEdit( hbox );
    hbox = new QtHBox( box );
    (void) new QPushButton( "Use Current Page" );
    (void) new QPushButton ( "Choose" );
    frame = new QtLabelled( "History", page );
    hbox = new QtHBox( frame );
    (void) new QLabel( "History expires after", hbox );
    (void) new QLineEdit( hbox );
    (void) new QLabel( "days", hbox );
    (void) new QPushButton( "Clear History", hbox );
    page->addStretch();

    QtButtonRow *br = new QtButtonRow( page );
    
    (void) new QPushButton( "OK", br );
    (void) new QPushButton( "Apply", br );
    (void) new QPushButton( "Cancel", br );

    return page;
}



int main( int argc, char **argv )
{
    QApplication a( argc, argv );

    QWidget *g = navigatorDialog();

    g->show();
    a.setMainWidget(g);
    return a.exec();
}
