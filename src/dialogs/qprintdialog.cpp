/****************************************************************************
** $Id: qprintdialog.cpp,v 2.32.2.3 1998/11/02 12:58:38 paul Exp $
**
** Implementation of internal print dialog (X11) used by QPrinter::select().
**
** Created : 950829
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

#include "qprintdialog.h"

#include "qfiledialog.h"

#include "qfile.h"

#include "qcombobox.h"
#include "qframe.h"
#include "qlabel.h"
#include "qlineedit.h"
#include "qpushbutton.h"
#include "qprinter.h"
#include "qlistview.h"
#include "qlayout.h"
#include "qbuttongroup.h"
#include "qradiobutton.h"
#include "qspinbox.h"
#include "qapplication.h"

#include "qstring.h"
#include "qregexp.h"

#include <ctype.h>
#include <stdlib.h>

struct QPrintDialogPrivate
{
    QPrinter * printer;

    QButtonGroup * printerOrFile;

    bool outputToFile;
    QListView * printers;
    QLineEdit * fileName;
    QPushButton * browse;

    QButtonGroup * printRange;
    QLabel * firstPageLabel;
    QSpinBox * firstPage;
    QLabel * lastPageLabel;
    QSpinBox * lastPage;
    QRadioButton * printAllButton;
    QRadioButton * printRangeButton;

    QButtonGroup * paperSize;
    QPrinter::PageSize pageSize;

    QButtonGroup * orient;
    QPrinter::Orientation orientation;

    QButtonGroup * pageOrder;
    QPrinter::PageOrder pageOrder2;

    QButtonGroup * colorMode;
    QPrinter::ColorMode colorMode2;

    QSpinBox * copies;
    int numCopies;
};


static void perhapsAddPrinter( QListView * printers, const char * name,
			       const char * host, const char * comment )
{
    const QListViewItem * i = printers->firstChild();
    while( i && qstrcmp( i->text( 0 ), name ) )
	i = i->nextSibling();
    if ( !i )
	(void)new QListViewItem( printers, name,
				 host ? host : "locally connected",
				 comment ? comment : "" );
}


static void parsePrintcap( QListView * printers )
{
    QFile printcap( "/etc/printcap" );
    if ( !printcap.open( IO_ReadOnly ) )
	return;

    char * line = new char[1025];
    line[1024] = '\0';

    QString printerDesc;
    int lineLength = 0;

    while( !printcap.atEnd() &&
	   (lineLength=printcap.readLine( line, 1024 )) > 0 ) {
	if ( *line == '#' ) {
	    *line = '\0';
	    lineLength = 0;
	}
	if ( lineLength >= 2 && line[lineLength-2] == '\\' ) {
	    line[lineLength-2] = '\0';
	    printerDesc += line;
	} else {
	    printerDesc += line;
	    printerDesc = printerDesc.simplifyWhiteSpace();
	    int i = printerDesc.find( ':' );
	    QString printerName, printerComment, printerHost;
	    if ( i >= 0 ) {
		// have : want |
		int j = printerDesc.findRev( '|', i-1 );
		printerName = printerDesc.mid( j+1, i-j-1 );
		if ( j > 0 ) {
		    // try extracting a comment from the aliases...
		    printerComment = "Aliases: ";
		    printerComment += printerDesc.mid( 0, j );
		    for( j=printerComment.length(); j>-1; j-- )
			if ( printerComment[j] == '|' )
			    printerComment[j] = ',';
		}
		// then look for a real comment
		j = i+1;
		while( printerDesc[j] && isspace(printerDesc[j]) )
		    j++;
		if ( printerDesc[j] != ':' ) {
		    printerComment = printerDesc.mid( i, j-i );
		    printerComment.simplifyWhiteSpace();
		}
		// look for signs of this being a remote printer
		i = printerDesc.find( QRegExp( ": *rm *=" ) );
		if ( i >= 0 ) {
		    // point k at the end of remote host name
		    while( printerDesc[i] != '=' )
			i++;
		    while( printerDesc[i] == '=' || isspace( printerDesc[i] ) )
			i++;
		    j = i;
		    while( printerDesc[j] != ':' && printerDesc[j] )
			j++;

		    // and stuff that into the string
		    printerHost = printerDesc.mid( i, j-i );
		}
	    }
	    if ( printerName.length() )
		perhapsAddPrinter( printers, printerName, printerHost,
				   printerComment );
	    // chop away the line, for processing the next one
	    printerDesc = 0;
	}
    }
    delete[] line;
}

// solaris, not 2.6
static void parseEtcLpPrinters( QListView * printers )
{
    QDir lp( "/etc/lp/printers" );
    const QFileInfoList * dirs = lp.entryInfoList();
    if ( !dirs )
	return;

    QFileInfoListIterator it( *dirs );
    QFileInfo *printer;
    QString tmp;
    while ( (printer = it.current()) != 0 ) {
	++it;
	if ( printer->isDir() ) {
	    tmp.sprintf( "/etc/lp/printers/%s/configuration",
			 printer->fileName().data() );
	    QFile configuration( tmp );
	    char * line = new char[1025];
	    QRegExp remote( "^Remote:" );
	    QRegExp contentType( "^Content types:" );
	    QString printerHost;
	    bool canPrintPostscript = FALSE;
	    if ( configuration.open( IO_ReadOnly ) ) {
		while( !configuration.atEnd() &&
		       configuration.readLine( line, 1024 ) > 0 ) {
		    if ( remote.match( line ) == 0 ) {
			const char * p = line;
			while( *p != ':' )
			    p++;
			p++;
			while( isspace(*p) )
			    p++;
			printerHost = p;
			printerHost.simplifyWhiteSpace();
		    } else if ( contentType.match( line ) == 0 ) {
			char * p = line;
			while( *p != ':' )
			    p++;
			p++;
			char * e;
			while( *p ) {
			    while( isspace(*p) )
				p++;
			    if ( *p ) {
				char s;
				e = p;
				while( isalnum(*e) )
				    e++;
				s = *e;
				*e = '\0';
				if ( !qstrcmp( p, "postscript" ) ||
				     !qstrcmp( p, "any" ) )
				    canPrintPostscript = TRUE;
				*e = s;
				if ( s == ',' )
				    e++;
				p = e;
			    }
			}
		    }
		}
		if ( canPrintPostscript )
		    perhapsAddPrinter( printers, printer->fileName().data(),
				       printerHost, 0 );
	    }
	    delete[] line;
	}
    }
}


// solaris 2.6
static char * parsePrintersConf( QListView * printers )
{
    QFile pc( "/etc/printers.conf" );
    if ( !pc.open( IO_ReadOnly ) )
	return 0;

    char * line = new char[1025];
    line[1024] = '\0';

    QString printerDesc;
    int lineLength = 0;

    char * defaultPrinter = 0;

    while( !pc.atEnd() &&
	   (lineLength=pc.readLine( line, 1024 )) > 0 ) {
	if ( *line == '#' ) {
	    *line = '\0';
	    lineLength = 0;
	}
	if ( lineLength >= 2 && line[lineLength-2] == '\\' ) {
	    line[lineLength-2] = '\0';
	    printerDesc += line;
	} else {
	    printerDesc += line;
	    printerDesc = printerDesc.simplifyWhiteSpace();
	    int i = printerDesc.find( ':' );
	    QString printerName, printerHost, printerComment;
	    if ( i >= 0 ) {
		// have : want |
		int j = printerDesc.find( '|', 0 );
		if ( j >= i )
		    j = -1;
		printerName = printerDesc.mid( 0, j < 0 ? i : j );
		if ( printerName == "_default" ) {
		    i = printerDesc.find( QRegExp( ": *use *=" ) );
		    while( printerDesc[i] != '=' )
			i++;
		    while( printerDesc[i] == '=' || isspace( printerDesc[i] ) )
			i++;
		    j = i;
		    while( printerDesc[j] != ':' &&
			   printerDesc[j] != ',' &&
			   printerDesc[j] )
			j++;
		    // that's our default printer
		    defaultPrinter = qstrdup( printerDesc.mid( i, j-i ) );
		    printerName = 0;
		    printerDesc = 0;
		} else if ( printerName == "_all" ) {
		    // skip it.. any other cases we want to skip?
		    printerName = 0;
		    printerDesc = 0;
		}

		if ( j > 0 ) {
		    // try extracting a comment from the aliases...
		    printerComment = "Aliases: ";
		    printerComment += printerDesc.mid( j+1, i-j-1 );
		    for( j=printerComment.length(); j>-1; j-- )
			if ( printerComment[j] == '|' )
			    printerComment[j] = ',';
		}
		// look for signs of this being a remote printer
		i = printerDesc.find( QRegExp( ": *bsdaddr *=" ) );
		if ( i >= 0 ) {
		    // point k at the end of remote host name
		    while( printerDesc[i] != '=' )
			i++;
		    while( printerDesc[i] == '=' || isspace( printerDesc[i] ) )
			i++;
		    j = i;
		    while( printerDesc[j] != ':' &&
			   printerDesc[j] != ',' &&
			   printerDesc[j] )
			j++;
		    // and stuff that into the string
		    printerHost = printerDesc.mid( i, j-i );
		    // maybe stick the remote printer name into the comment
		    if ( printerDesc[j] == ',' ) {
			i = ++j;
			while( isspace( printerDesc[i] ) )
			    i++;
			j = i;
			while( printerDesc[j] != ':' &&
			       printerDesc[j] != ',' &&
			       printerDesc[j] )
			    j++;
			if ( printerName != printerDesc.mid( i, j-i ) ) {
			    printerComment = "Remote name: ";
			    printerComment += printerDesc.mid( i, j-i );
			}
		    }
		}
	    }
	    if ( printerName.length() )
		perhapsAddPrinter( printers, printerName, printerHost,
				   printerComment );
	    // chop away the line, for processing the next one
	    printerDesc = 0;
	}
    }
    delete[] line;
    return defaultPrinter;
}



// HP-UX
static void parseEtcLpMember( QListView * printers )
{
    QDir lp( "/etc/lp/member" );
    const QFileInfoList * dirs = lp.entryInfoList();
    if ( !dirs )
	return;

    QFileInfoListIterator it( *dirs );
    QFileInfo *printer;
    QString tmp;
    while ( (printer = it.current()) != 0 ) {
	++it;
	// uglehack.
	// I haven't found any real documentation, so I'm guessing that
	// since lpstat uses /etc/lp/member rather than one of the
	// other directories, it's the one to use.  I did not find a
	// decent way to locate aliases and remote printers.
	if ( printer->isFile() )
	    perhapsAddPrinter( printers, printer->fileName().data(),
			       "unknown", 0 );
    }
}

// IRIX 6.x
static void parseSpoolInterface( QListView * printers )
{
    QDir lp( "/usr/spool/lp/interface" );
    const QFileInfoList * files = lp.entryInfoList();
    if( !files )
	return;

    QFileInfoListIterator it( *files );
    QFileInfo *printer;
    while ( (printer = it.current()) != 0) {
	++it;

	if ( !printer->isFile() )
	    continue;

	// parse out some information
	QFile configFile( printer->filePath() );
	if ( !configFile.open( IO_ReadOnly ) )
	    continue;

	QString line(1025);
	QString hostName;
	QString hostPrinter;
	QString printerType;

	QRegExp typeKey("^TYPE=");
	QRegExp hostKey("^HOSTNAME=");
	QRegExp hostPrinterKey("^HOSTPRINTER=");
	int length;

	while( !configFile.atEnd() &&
	    (configFile.readLine( line.data(), 1024 )) > 0 ) {

	    if(typeKey.match(line, 0, &length) == 0) {
		printerType = line.mid(length, line.length()-length);
		printerType = printerType.simplifyWhiteSpace();
	    }
	    if(hostKey.match(line, 0, &length) == 0) {
		hostName = line.mid(length, line.length()-length);
		hostName = hostName.simplifyWhiteSpace();
	    }
	    if(hostPrinterKey.match(line, 0, &length) == 0) {
		hostPrinter = line.mid(length, line.length()-length);
		hostPrinter = hostPrinter.simplifyWhiteSpace();
	    }
	}
	configFile.close();

	printerType = printerType.stripWhiteSpace();
	if ( !printerType.isEmpty() && qstricmp( printerType, "postscript" ))
	    continue;

	if(hostName.isEmpty() || hostPrinter.isEmpty())
	{
	    perhapsAddPrinter( printers, printer->fileName().data(),
		0, 0);
	} else
	{
	    QString comment("Remote name: ");
	    comment += hostPrinter;
	    perhapsAddPrinter( printers, printer->fileName().data(),
		hostName, comment);
	}
    }
}

static QPrintDialog * globalPrintDialog = 0;


/*! \class QPrintDialog qprintdialog.h

  \brief The QPrintDialog class provides an X-only dialog for specifying
  print-out details.

  \ingroup dialogs

  It encompasses both the sort of details needed for doing a simple
  print-out and some print configuration setup.

  \warning For this release, the printer dialog is only available
  under X11. Use QPrinter::setup() for portability.
  
  At present, the only easy way to use the class is through the static
  function getPrinterSetup().  You can however also call the global
  QPrintDialot::getPrinterConfigure(), or subclass in order to extend
  one of the group boxes.

  Note that in 1.40 the printer dialog is a little too high for
  comfortable use on a small-screen machine.  This will be improved on
  in 2.0.

  <img src="printerdialog.gif" width="479" height="540"><br clear=all>
  The printer dialog, on a large screen, in Motif style.
*/


/*! Creates a new modal printer dialog that configures \a prn and is a
  child of \a parent named \a name.
*/

QPrintDialog::QPrintDialog( QPrinter *prn, QWidget *parent, const char *name )
    : QDialog( parent, name, TRUE )
{
    d = new QPrintDialogPrivate;
    d->numCopies = 1;

    QBoxLayout * tll = new QBoxLayout( this, QBoxLayout::Down, 12, 0 );

    QGroupBox * g;
    g = setupDestination();
    tll->addWidget( g, 1 );
    tll->addSpacing( 12 );

    QBoxLayout * horiz = new QBoxLayout( QBoxLayout::LeftToRight );
    tll->addLayout( horiz, 0 );

    g = setupOptions();
    horiz->addWidget( g );
    horiz->addSpacing( 12 );

    g = setupPaper();
    horiz->addWidget( g );

    tll->addSpacing( 12 );

    horiz = new QBoxLayout( QBoxLayout::LeftToRight );
    tll->addLayout( horiz );

    if ( style() != MotifStyle )
	horiz->addStretch( 1 );

    QPushButton * ok = new QPushButton( this, "ok" );
    ok->setText( "Ok" );
    ok->setAutoDefault( TRUE );
    ok->setDefault( TRUE );
    horiz->addWidget( ok );
    if ( style() == MotifStyle )
	horiz->addStretch( 1 );
    horiz->addSpacing( 6 );

    QPushButton * cancel = new QPushButton( this, "cancel" );
    cancel->setText( "Cancel" );
    cancel->setAutoDefault( TRUE );
    horiz->addWidget( cancel );

    QSize s1 = ok->sizeHint();
    QSize s2 = cancel->sizeHint();
    s1 = QSize( QMAX(s1.width(), s2.width()),
		QMAX(s1.height(), s2.height()) );

    ok->setFixedSize( s1 );
    cancel->setFixedSize( s1 );

    tll->activate();

    connect( ok, SIGNAL(clicked()), SLOT(okClicked()) );
    connect( cancel, SIGNAL(clicked()), SLOT(reject()) );

    QSize ms( minimumSize() );
    QSize ss( QApplication::desktop()->size() );
    if ( ms.height() < 512 && ss.height() >= 600 )
	ms.setHeight( 512 );
    else if ( ms.height() < 460 && ss.height() >= 480 )
	ms.setHeight( 460 );
    resize( ms );

    setPrinter( prn, TRUE );
    d->printers->setFocus();

    setFontPropagation( SameFont );
    setPalettePropagation( SamePalette );
}


/*! Destroys the object and frees any allocated resources.  Does not
  delete the associated QPrinter object.
*/

QPrintDialog::~QPrintDialog()
{
    if ( this == globalPrintDialog )
	globalPrintDialog = 0;
    delete d;
}


QGroupBox * QPrintDialog::setupDestination()
{
    QGroupBox * g = new QGroupBox( tr( "Print Destination"),
				   this, "destination group box" );

    QBoxLayout * tll = new QBoxLayout( g, QBoxLayout::Down, 12, 0 );
    tll->addSpacing( 8 );

    d->printerOrFile = new QButtonGroup( this );
    d->printerOrFile->hide();
    connect( d->printerOrFile, SIGNAL(clicked(int)),
	     this, SLOT(printerOrFileSelected(int)) );

    // printer radio button, list
    QRadioButton * rb = new QRadioButton( tr( "Print to Printer:" ), g,
					  "printer" );
    rb->setMinimumSize( rb->sizeHint() );
    tll->addWidget( rb );
    d->printerOrFile->insert( rb, 0 );
    rb->setChecked( TRUE );
    d->outputToFile = FALSE;

    QBoxLayout * horiz = new QBoxLayout( QBoxLayout::LeftToRight );
    tll->addLayout( horiz, 3 );
    horiz->addSpacing( 19 );

    d->printers = new QListView( g, "list of printers" );
    d->printers->setAllColumnsShowFocus( TRUE );
    d->printers->addColumn( "Printer", 125 );
    d->printers->addColumn( "Host", 125 );
    d->printers->addColumn( "Comment", 150 );
    d->printers->setFrameStyle( QFrame::WinPanel + QFrame::Sunken );

#if defined(UNIX)
    char * etcLpDefault = 0;

    QFileInfo f;

    // now do the tiresome unix printer lookup
    f.setFile( "/etc/printcap" );
    if ( f.isFile() && f.isReadable() )
	parsePrintcap( d->printers );

    f.setFile( "/etc/lp/printers" );
    if ( f.isDir() ) {
	parseEtcLpPrinters( d->printers );
	QFile def( "/etc/lp/default" );
	if ( def.open( IO_ReadOnly ) ) {
	    etcLpDefault = new char[1025];
	    def.readLine( etcLpDefault, 1024 );
	    char * p = etcLpDefault;
	    while( p && *p ) {
		if ( !isprint(*p) || isspace(*p) )
		    *p = 0;
		else
		    p++;
	    }
	}
    }

    f.setFile( "/etc/printers.conf" );
    if ( f.isFile() ) {
	char * def = parsePrintersConf( d->printers );
	if ( def ) {
	    if ( etcLpDefault )
		delete[] etcLpDefault;
	    etcLpDefault = def;
	}
    }

    f.setFile( "/etc/lp/member" );
    if ( f.isDir() )
	parseEtcLpMember( d->printers );

    f.setFile( "/usr/spool/lp/interface" );
    if ( f.isDir() )
	parseSpoolInterface( d->printers );

    // all printers hopefully known.  try to find a good default
    char * dollarPrinter;
    dollarPrinter = getenv( "PRINTER" );
    if ( !dollarPrinter || !*dollarPrinter )
	dollarPrinter = getenv( "LPDEST" );
    int quality = 0;

    // bang the best default into the listview
    const QListViewItem * lvi = d->printers->firstChild();
    d->printers->setCurrentItem( (QListViewItem *)lvi );
    while( lvi ) {
	QRegExp ps1( "[^a-z]ps[^a-z]" );
	QRegExp ps2( "[^a-z]ps$" );
	QRegExp lp1( "[^a-z]lp[^a-z]" );
	QRegExp lp2( "[^a-z]lp$" );
	if ( quality < 4 &&
	     !qstrcmp( lvi->text( 0 ), dollarPrinter ) ) {
	    d->printers->setCurrentItem( (QListViewItem *)lvi );
	    quality = 4;
	} else if ( quality < 3 && etcLpDefault &&
		    !qstrcmp( lvi->text( 0 ), etcLpDefault ) ) {
	    d->printers->setCurrentItem( (QListViewItem *)lvi );
	    quality = 3;
	} else if ( quality < 2 &&
		    ( !qstrcmp( lvi->text( 0 ), "ps" ) ||
		      ps1.match( lvi->text( 2 ) ) > -1 ||
		      ps2.match( lvi->text( 2 ) ) > -1 ) ) {
	    d->printers->setCurrentItem( (QListViewItem *)lvi );
	    quality = 2;
	} else if ( quality < 1 &&
		    ( !qstrcmp( lvi->text( 0 ), "lp" ) ||
		      lp1.match( lvi->text( 2 ) ) > -1 ||
		      lp2.match( lvi->text( 2 ) ) > -1 ) ) {
	    d->printers->setCurrentItem( (QListViewItem *)lvi );
	    quality = 1;
	}
	lvi = lvi->nextSibling();
    }
    if ( d->printers->currentItem() )
	d->printers->setSelected( d->printers->currentItem(), TRUE );

    if ( etcLpDefault )			// Avoid purify complaint
	delete[] etcLpDefault;
#endif

    d->printers->setMinimumSize( 404, fontMetrics().height() * 5 );
    horiz->addWidget( d->printers, 3 );

    tll->addSpacing( 6 );

    // file radio button, edit/browse
    rb = new QRadioButton( tr( "Print to File:" ), g, "file" );
    rb->setMinimumSize( rb->sizeHint() );
    tll->addWidget( rb );
    d->printerOrFile->insert( rb, 1 );

    horiz = new QBoxLayout( QBoxLayout::LeftToRight );
    tll->addLayout( horiz );
    horiz->addSpacing( 19 );

    d->fileName = new QLineEdit( g, "file name" );
    d->fileName->setMinimumSize( d->fileName->sizeHint() );
    horiz->addWidget( d->fileName, 1 );
    horiz->addSpacing( 6 );
    d->browse = new QPushButton( tr("Browse"), g, "browse files" );
    d->browse->setMinimumSize( d->browse->sizeHint() );
    connect( d->browse, SIGNAL(clicked()),
	     this, SLOT(browseClicked()) );
    horiz->addWidget( d->browse );

    d->fileName->setEnabled( FALSE );
    d->browse->setEnabled( FALSE );

    tll->activate();

    return g;
}


QGroupBox * QPrintDialog::setupOptions()
{
    QGroupBox * g = new QGroupBox( tr( "Options"),
				   this, "options group box" );

    QBoxLayout * tll = new QBoxLayout( g, QBoxLayout::Down, 12, 2 );
    tll->addSpacing( 8 );

    d->printRange = new QButtonGroup( this );
    d->printRange->hide();
    connect( d->printRange, SIGNAL(clicked(int)),
	     this, SLOT(printRangeSelected(int)) );

    d->pageOrder = new QButtonGroup( this );
    d->pageOrder->hide();
    connect( d->pageOrder, SIGNAL(clicked(int)),
	     this, SLOT(pageOrderSelected(int)) );

    d->colorMode = new QButtonGroup( this );
    d->colorMode->hide();
    connect( d->colorMode, SIGNAL(clicked(int)),
	     this, SLOT(colorModeSelected(int)) );

    d->printAllButton = new QRadioButton( tr("Print all"), g, "print all" );
    d->printAllButton->setMinimumSize( d->printAllButton->sizeHint() );
    d->printRange->insert( d->printAllButton, 0 );
    tll->addWidget( d->printAllButton );

    d->printRangeButton = new QRadioButton( tr("Print Range:"),
					    g, "print range" );
    d->printRangeButton->setMinimumSize( d->printRangeButton->sizeHint() );
    d->printRange->insert( d->printRangeButton, 1 );
    tll->addWidget( d->printRangeButton );

    QBoxLayout * horiz = new QBoxLayout( QBoxLayout::LeftToRight );
    tll->addLayout( horiz );

    d->firstPageLabel = new QLabel( tr("From page"), g, "first page" );
    horiz->addSpacing( 19 );
    horiz->addWidget( d->firstPageLabel );

    d->firstPage = new QSpinBox( 1, 9999, 1, g, "first page" );
    d->firstPage->setValue( 1 );
    d->firstPage->setMinimumSize( d->firstPage->sizeHint() );
    horiz->addWidget( d->firstPage, 1 );
    connect( d->firstPage, SIGNAL(valueChanged(int)),
	     this, SLOT(setFirstPage(int)) );

    horiz = new QBoxLayout( QBoxLayout::LeftToRight );
    tll->addLayout( horiz );

    d->lastPageLabel = new QLabel( tr("To page"), g, "last page" );
    horiz->addSpacing( 19 );
    horiz->addWidget( d->lastPageLabel );

    d->lastPage = new QSpinBox( 1, 9999, 1, g, "last page" );
    d->lastPage->setValue( 9999 );
    d->lastPage->setMinimumSize( d->lastPage->sizeHint() );
    horiz->addWidget( d->lastPage, 1 );
    connect( d->lastPage, SIGNAL(valueChanged(int)),
	     this, SLOT(setLastPage(int)) );

    QFrame * divider = new QFrame( g, "divider", 0, TRUE );
    divider->setFrameStyle( QFrame::HLine + QFrame::Sunken );
    divider->setMinimumHeight( 6 );
    tll->addWidget( divider, 1 );

    // print order
    QRadioButton * rb = new QRadioButton( tr("Print first page first"),
					  g, "first page first" );
    rb->setMinimumSize( rb->sizeHint() );
    tll->addWidget( rb );
    d->pageOrder->insert( rb, QPrinter::FirstPageFirst );
    rb->setChecked( TRUE );

    rb = new QRadioButton( tr("Print last page first"),
			   g, "last page first" );
    rb->setMinimumSize( rb->sizeHint() );
    tll->addWidget( rb );
    d->pageOrder->insert( rb, QPrinter::LastPageFirst );

    divider = new QFrame( g, "divider", 0, TRUE );
    divider->setFrameStyle( QFrame::HLine + QFrame::Sunken );
    divider->setMinimumHeight( 6 );
    tll->addWidget( divider, 1 );

    // color mode
    rb = new QRadioButton( tr("Print in color if available"),
			   g, "color" );
    rb->setMinimumSize( rb->sizeHint() );
    tll->addWidget( rb );
    d->colorMode->insert( rb, QPrinter::Color );
    rb->setChecked( TRUE );

    rb = new QRadioButton( tr("Print in grayscale"),
			   g, "graysacle" );
    rb->setMinimumSize( rb->sizeHint() );
    tll->addWidget( rb );
    d->colorMode->insert( rb, QPrinter::GrayScale );

    divider = new QFrame( g, "divider", 0, TRUE );
    divider->setFrameStyle( QFrame::HLine + QFrame::Sunken );
    divider->setMinimumHeight( 6 );
    tll->addWidget( divider, 1 );

    // copies

    horiz = new QBoxLayout( QBoxLayout::LeftToRight );
    tll->addLayout( horiz );

    QLabel * l = new QLabel( tr("Number of copies"), g, "Number of copies" );
    horiz->addWidget( l );

    d->copies = new QSpinBox( 1, 99, 1, g, "copies" );
    d->copies->setValue( 1 );
    d->copies->setMinimumSize( d->copies->sizeHint() );
    horiz->addWidget( d->copies, 1 );
    connect( d->copies, SIGNAL(valueChanged(int)),
	     this, SLOT(setNumCopies(int)) );

    QSize s = d->firstPageLabel->sizeHint()
	      .expandedTo( d->lastPageLabel->sizeHint() )
	      .expandedTo( l->sizeHint() );
    d->firstPageLabel->setMinimumSize( s );
    d->lastPageLabel->setMinimumSize( s );
    l->setMinimumSize( s.width() + 19, s.height() );

    tll->activate();

    return g;
}


QGroupBox * QPrintDialog::setupPaper()
{
    QGroupBox * g = new QGroupBox( tr( "Paper format"),
				   this, "Paper format" );

    QBoxLayout * tll = new QBoxLayout( g, QBoxLayout::Down, 12, 0 );
    tll->addSpacing( 8 );

    d->orient = new QButtonGroup( this );
    d->orient->hide();
    connect( d->orient, SIGNAL(clicked(int)),
	     this, SLOT(orientSelected(int)) );

    d->paperSize = new QButtonGroup( this );
    d->paperSize->hide();
    connect( d->paperSize, SIGNAL(clicked(int)),
	     this, SLOT(paperSizeSelected(int)) );

    // page orientation
    QRadioButton * rb = new QRadioButton( "Portrait", g, "portrait format" );
    rb->setMinimumSize( rb->sizeHint() );
    d->orient->insert( rb, (int)QPrinter::Portrait );
    tll->addWidget( rb );

    rb->setChecked( TRUE );
    d->orientation = QPrinter::Portrait;

    rb = new QRadioButton( "Landscape", g, "landscape format" );
    rb->setMinimumSize( rb->sizeHint() );
    d->orient->insert( rb, (int)QPrinter::Landscape );
    tll->addWidget( rb );

    QFrame * divider = new QFrame( g, "divider", 0, TRUE );
    divider->setFrameStyle( QFrame::HLine + QFrame::Sunken );
    divider->setMinimumHeight( 6 );
    tll->addWidget( divider, 1 );

    // paper size
    rb = new QRadioButton( "A4 (210 x 297 mm)", g, "A4" );
    rb->setMinimumSize( rb->sizeHint() );
    d->paperSize->insert( rb, 0 );
    rb->setChecked( TRUE );
    d->pageSize = QPrinter::A4;
    tll->addWidget( rb );

    rb = new QRadioButton( "B5", g, "B5" );
    rb->setMinimumSize( rb->sizeHint() );
    d->paperSize->insert( rb, 1 );
    tll->addWidget( rb );

    rb = new QRadioButton( "Letter (8½ x 11in)", g, "Letter" );
    rb->setMinimumSize( rb->sizeHint() );
    d->paperSize->insert( rb, 2 );
    tll->addWidget( rb );

    rb = new QRadioButton( "Legal", g, "Letter" );
    rb->setMinimumSize( rb->sizeHint() );
    d->paperSize->insert( rb, 3 );
    tll->addWidget( rb );

    rb = new QRadioButton( "Executive", g, "Letter" );
    rb->setMinimumSize( rb->sizeHint() );
    d->paperSize->insert( rb, 4 );
    tll->addWidget( rb );

    tll->activate();

    return g;
}


/*!
  Display a dialog and allow the user to configure the QPrinter \a
  p.  Returns TRUE if the user clicks OK or presses Enter, FALSE if
  the user clicks Cancel or presses Escape.

  getPrinterSetup() remembers the settings and provides the same
  settings the next time the dialog is shown.

  \warning For this release, the printer dialog is only available
  under X11. Use QPrinter::setup() for portability.

*/

bool QPrintDialog::getPrinterSetup( QPrinter * p )
{
    if ( !globalPrintDialog )
	globalPrintDialog = new QPrintDialog( 0, 0, "global print dialog" );

    globalPrintDialog->setPrinter( p );
    bool r = globalPrintDialog->exec() == QDialog::Accepted;
    globalPrintDialog->setPrinter( 0 );
    return r;
}


void QPrintDialog::printerOrFileSelected( int id )
{
    d->outputToFile = id ? TRUE : FALSE;
    if ( d->outputToFile ) {
	d->browse->setEnabled( TRUE );
	d->fileName->setEnabled( TRUE );
	d->fileName->setFocus();
	d->printers->setEnabled( FALSE );
    } else {
	d->printers->setEnabled( TRUE );
	if ( d->fileName->hasFocus() || d->browse->hasFocus() )
	    d->printers->setFocus();
	d->browse->setEnabled( FALSE );
	d->fileName->setEnabled( FALSE );
    }
}


void QPrintDialog::landscapeSelected( int id )
{
    d->orientation = (QPrinter::Orientation)id;
}


void QPrintDialog::paperSizeSelected( int id )
{
    d->pageSize = QPrinter::PageSize(id);
}


void QPrintDialog::orientSelected( int id )
{
    d->orientation = (QPrinter::Orientation)id;
}


void QPrintDialog::pageOrderSelected( int id )
{
    d->pageOrder2 = (QPrinter::PageOrder)id;
}


void QPrintDialog::setNumCopies( int copies )
{
    d->numCopies = copies;
}


void QPrintDialog::browseClicked()
{
    QString fn = QFileDialog::getSaveFileName();
    if ( !fn.isNull() )
	d->fileName->setText( fn );
}


void QPrintDialog::okClicked()
{
    if ( d->outputToFile ) {
	d->printer->setOutputToFile( TRUE );
	d->printer->setOutputFileName( d->fileName->text() );
    } else {
	d->printer->setOutputToFile( FALSE );
	QListViewItem * l = d->printers->currentItem();
	if ( l )
	    d->printer->setPrinterName( l->text( 0 ) );
    }

    d->printer->setOrientation( d->orientation );
    d->printer->setPageSize( d->pageSize );
    d->printer->setPageOrder( d->pageOrder2 );
    d->printer->setColorMode( d->colorMode2 );
    d->printer->setNumCopies( d->numCopies );
    if ( d->printAllButton->isChecked() )
	d->printer->setFromTo( d->printer->minPage(), d->printer->maxPage() );
    else
	d->printer->setFromTo( d->firstPage->value(), d->lastPage->value() );

    accept();
}


void QPrintDialog::printRangeSelected( int id )
{
    bool enable = id ? TRUE : FALSE;
    d->firstPage->setEnabled( enable );
    d->lastPage->setEnabled( enable );
}


void QPrintDialog::setFirstPage( int fp )
{
    if ( d->printer )
	d->lastPage->setRange( fp, QMAX(fp,d->printer->maxPage()) );
}


void QPrintDialog::setLastPage( int lp )
{
    if ( d->printer )
	d->firstPage->setRange( QMIN(lp,d->printer->minPage()), lp );
}


/*!  Sets this dialog to configure \a p, or no printer if \a p is
  FALSE.  If \a pickUpSettings is TRUE, the dialog reads most of its
  settings from \a printer.  If \a pickUpSettings is FALSE (the
  default) the dialog keeps its old settings. */

void QPrintDialog::setPrinter( QPrinter * p, bool pickUpSettings )
{
    d->printer = p;

    if ( p && pickUpSettings ) {
	// top to botton in the old dialog.
	// printer or file
	d->printerOrFile->setButton( p->outputToFile() );
	printerOrFileSelected( p->outputToFile() );

	// printer name
	if ( p->printerName() ) {
	    QListViewItem * i = d->printers->firstChild();
	    while( i && qstrcmp( i->text( 0 ), p->printerName() ) )
		i = i->nextSibling();
	    if ( i )
		d->printers->setSelected( i, TRUE );
	}

	// print command does not exist any more

	// file name
	d->fileName->setText( p->outputFileName() );

	// orientation
	d->orient->setButton( (int)p->orientation() );
	orientSelected( p->orientation() );

	// page size
	d->paperSize->setButton( p->pageSize() );
	paperSizeSelected( p->pageSize() );

	// New stuff (Options)

	// page order
	d->pageOrder->setButton( (int)p->pageOrder() );
	pageOrderSelected( p->pageOrder() );

	// color mode
	d->colorMode->setButton( (int)p->colorMode() );
	colorModeSelected( p->colorMode() );

	// number of copies
	d->copies->setValue( p->numCopies() );
	setNumCopies( p->numCopies() );
    }

    if ( p && p->maxPage() ) {
	d->printRangeButton->setEnabled( TRUE );
	d->firstPage->setRange( p->minPage(), p->maxPage() );
	d->lastPage->setRange( p->minPage(), p->maxPage() );
	// page range
	int some = p->maxPage()
		&& p->fromPage() && p->toPage()
		&& (p->fromPage() != p->minPage()
		    || p->toPage() != p->maxPage());
	if ( p->fromPage() ) {
	    setFirstPage( p->fromPage() );
	    setLastPage( p->toPage() );
	    d->firstPage->setValue(p->fromPage());
	    d->lastPage->setValue(p->toPage());
	}
	d->printRange->setButton( some );
	printRangeSelected( some );
    } else {
	d->printRange->setButton( 0 );	
	d->printRangeButton->setEnabled( FALSE );
	d->firstPage->setEnabled( FALSE );
	d->lastPage->setEnabled( FALSE );
	d->firstPageLabel->setEnabled( FALSE );
	d->lastPageLabel->setEnabled( FALSE );
	d->firstPage->setValue( 1 );
	d->lastPage->setValue( 1 );
    }
}


/*!  Returns a pointer to the printer this dialog configures, or 0 if
  this dialog does not operate on any printer. */

QPrinter * QPrintDialog::printer() const
{
    return d->printer;
}


void QPrintDialog::colorModeSelected( int id )
{
    d->colorMode2 = (QPrinter::ColorMode)id;
}


/*! \base64 printerdialog.gif

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
NAQnuGUd2IGhZoP+B4M96IM6eINVFoP+J4NEUIMjyIMAaIIO6IBKKIRaloIOBnxu93JFuIMu
SINMyIM5CIVX1oVN6INh6IJXaINm6IVTJoZj2IRrqINgmIEFGIRoWGVSKFIVp3hWSIZ6KIdl
JYZMOIdN9oZGKIN62IaECIhMpoZ8KId+6GRuiIhWVocftoKrB25leIltWARXyIZPCImOuIdL
eIRLWIaaaIie/piIpbiGO6iErJiKndiHg3iKUiaJMHWHntd/hSiIM7iJhyiLBAiKbIiBhYiJ
fwiJHAiMwaiKwqhkj+iLUEaLNmWL/JeHYaiLW6iLzsiMwNiIpmiKxYiIx9iN2AiGv4iFyZiN
SgaNZEaJp3dkl8iKwyiMT/iK6EiMRjiPigiPyoiO4eiGZ2iOoYiEvEiG9FiPYMZp3qds4Cd3
JWiQDvmQXfaNDqmOt2ZkDQmRGJmRaaiR6YiQ7fd97wd9XyaRHFmSDkmS9UiR0miBF0dnLvmS
MBmTMjmTaqaS7Bh7ELaKNLmTPNmTPvmTMWmTVIiHogeURnmUSJmUSjkFQul+VViU/lroZj+4
lEdohlHpBDpJlVq5lb7UlCD5lLh3BFkpTKs4lkqZlVgIBWbJlWzZlljglQoZksxWlURYlWm5
BNXIhU+wlkh5l3cJg6JIl4AZg1bploZ5mEUAly0nlwxJl6PIhQeIl35ZmHjZlpN5lZf5l+Zo
gIjZmYapmM83l1ZZlqNJmWLpmGrplplZmH9Zmq7Zmp4Zm0sJmocmmpj5mp0omGW5m1PJl2eJ
mxyolzQokK4pm8aZlLQZd29Fmi8YlbBpBMwpmEjgm7/pnKapicWJms15nNz5k8m5kMvpmKu5
nWI5nklAnUk5nsxpnubZne65k9/JmOFZmgGZnacZinGo/gR9yJXsiZ/12YrW+Z4COpPxCZa2
yQXoOaAKuqDvWaBEGZZekKAMOqEUipgOeosv9wUSWqEc2qGz6ZH6N5QYCpUeWqImaqIXOo0k
eqIs2qIKmqIsuXguOqM02p0wyoItWaM6uqNueaOVKKM8GqRCqpQ+2o4bOqRImqRrVqQ4aWJH
qqRQGqW8xKSsJ6VWeqXCRKU5iqVc2qVdoKVA6qViOqZVAKYZSqZomqZNYKYrqqZu6qZsCqFv
OqdqGqcHSqd46qV22ph52qdduqfz6aeCKqWAimtPOqiIiqIgSoEiqqJymqiQyqOFapGRWqk7
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
9QON9VWv9fPM9U8P8W7ruWA/6V5PzmVP2GdPzWlvXGtvORHdt1Mf9yal1ksfwqWOOXdv628/
4QCWy5msy1580WEP7B8e9Ft+9LHW97vbXDCv0Uc3zBjtWi2vWzCf+D7P+Lp0zR4P+fqcz4hl
wJ6f0ces0fvc6XWf96+7+R5mVvC2d7Xk5OL93aWm+U5/+JK+zdyszvbszQrN+6Hv+zSl+jtc
WwC/dOUe3y6OYE4XQCa8vXJM+Muczwc91qW80Ouc/SR169Bfype144T0/QsvQC8s+nx8/cJf
/b0//fyt/v2/7+kO9e3mj8uur+FOp8EDj2ADXni2T/qy/FQgoIijQo1meYooyabvy6LySto3
nus73/s/MCgsYXqUIo+iPC1zgNtTFI0qpiOqNGujWrXYITgsHh97iYQBcMCcjcjkUum6zWt2
Gkylx/NXM/1YoOAgIdmbTplRXJMT1JVWlVdXV2RlJcDkY+EmJ1DizlnaWlvS4Y4cY04dzN/d
nItMHCsTK+NqJ26urqopziecHA/m8BVmJFexpXExFvHTc/Py7nTnb06oGluCm2cqdeHtt/g4
p7VvL6I3OdDXunug+Q326HZpUPi7EWA+f79PvA2Aqvz9aEfwIC8zaLKROoUO/uGgYBAnEhR4
4iFFIQYzIrQoYp42bhxHkizpUKTJlCoT8gDZEBHGlTJnjvNIhCZOkzZd1juZMyWqoEKHEi1q
9CjSohiWMm3qdCk+mzZ/Ul23cyE9lFU5Ju3q9SvYoE/HNlXXIuYKtFvXUrsqKqQ9thnNbm1K
h05UtVPl8t3klmFPmH0n0q1qt4W3wjfjDm6s629Wxo77Kf552I9EPz43T+5MCDJczp7dVc55
eSgtwZJHswYD+iXL1lbxWWaKWahm1aJl89Z6DWto3eCC9kYccK3doyXy+i7unI5ans0F0ZZt
q3TKy6eYr37uPa3Ct7DPVfMVw1YM86kyL69Vvdwr/uRQve6G/v3+xfCAp8Pz1ep8e3U08V8r
A+7TVnx1YQBWfQGphd9orwUW23CZ4ZEehi0cmCGB5CQmHzh6PQhhZxLyR4Z5dmD4HyCxGOgK
ewgepyBZZHHXIImtmdgddSnOsuJdd+wx5Dsf0lijU4pJNWKOje2IoxirXFigiql1COCP4hhp
mG2KQAlek849KdxnPi7XRx41rCdLgEy851eCXC51JlHtkUlemMWNSWFJb244W2OHFZUbn3jm
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

*/
