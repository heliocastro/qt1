/****************************************************************************
** $Id: qfiledialog.cpp,v 2.83.2.15 1999/02/10 12:26:04 warwick Exp $
**
** Implementation of QFileDialog class
**
** Created : 950429
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

#include "qfiledialog.h"
#include "qlineedit.h"
#include "qcombobox.h"
#include "qlabel.h"
#include "qpushbutton.h"
#include "qmessagebox.h"
#include "qlistview.h"
#include "qapplication.h"
#include "qlayout.h"
#include "qlistview.h"
#include "qpixmap.h"
#include "qbitmap.h"
#include "qpopupmenu.h"
#include "qwidgetstack.h"
#include "qbuttongroup.h"
#include "qvector.h"
#include "qkeycode.h"
#include "qregexp.h"
#include "qstrlist.h"
#include "qtimer.h"

#include <time.h>
#include <ctype.h>

#if defined(_WS_WIN_)
#if defined(_CC_BOOL_DEF_)
#undef	bool
#include <windows.h>
#define bool int
#else
#include <windows.h>
#endif
#endif

static QFileIconProvider * fileIconProvider = 0;


/* XPM */
static const char* open_xpm[]={
"16 13 6 1",
". c None",
"b c #ffff00",
"d c #000000",
"# c #999999",
"c c #cccccc",
"a c #ffffff",
"...#####........",
"..#aaaaa#.......",
".#abcbcba######.",
".#acbcbcaaaaaa#d",
".#abcbcbcbcbcb#d",
"#############b#d",
"#aaaaaaaaaa##c#d",
"#abcbcbcbcbbd##d",
".#abcbcbcbcbcd#d",
".#acbcbcbcbcbd#d",
"..#acbcbcbcbb#dd",
"..#############d",
"...ddddddddddddd"};
/* XPM */
static const char* closed_xpm[]={
"15 13 6 1",
". c None",
"b c #ffff00",
"d c #000000",
"# c #999999",
"a c #cccccc",
"c c #ffffff",
"..#####........",
".#ababa#.......",
"#abababa######.",
"#cccccccccccc#d",
"#cbababababab#d",
"#cabababababa#d",
"#cbababababab#d",
"#cabababababa#d",
"#cbababababab#d",
"#cabababababa#d",
"#cbababababab#d",
"##############d",
".dddddddddddddd"};
/* XPM */
static const char* cdtoparent_xpm[]={
"15 13 3 1",
". c None",
"# c #000000",
"a c #ffff99",
"..#####........",
".#aaaaa#.......",
"###############",
"#aaaaaaaaaaaaa#",
"#aaaa#aaaaaaaa#",
"#aaa###aaaaaaa#",
"#aa#####aaaaaa#",
"#aaaa#aaaaaaaa#",
"#aaaa#aaaaaaaa#",
"#aaaa######aaa#",
"#aaaaaaaaaaaaa#",
"#aaaaaaaaaaaaa#",
"###############"};
/* XPM */
static const char* detailedview_xpm[]={
"14 11 3 1",
". c None",
"# c #000000",
"a c #000099",
".####.###.###.",
"..............",
"aaaaaaaaaaaaaa",
"..............",
".####.###.###.",
"..............",
".####.###.###.",
"..............",
".####.###.###.",
"..............",
".####.###.###."};
/* XPM */
static const char* mclistview_xpm[]={
"15 11 4 1",
"# c None",
"b c #000000",
". c #000099",
"a c #ffffff",
"...#####...####",
".a.#bbb#.a.#bbb",
"...#####...####",
"###############",
"...#####...####",
".a.#bbb#.a.#bbb",
"...#####...####",
"###############",
"...#####...####",
".a.#bbb#.a.#bbb",
"...#####...####"};
static QPixmap * openFolderIcon = 0;
static QPixmap * closedFolderIcon = 0;
static QPixmap * detailViewIcon = 0;
static QPixmap * multiColumnListViewIcon = 0;
static QPixmap * cdToParentIcon = 0;
static QPixmap * fifteenTransparentPixels = 0;

static QString * tmpString = 0;
static QString * workingDirectory = 0;

static void cleanup() {
    delete openFolderIcon;
    openFolderIcon = 0;
    delete closedFolderIcon;
    closedFolderIcon = 0;
    delete detailViewIcon;
    detailViewIcon = 0;
    delete multiColumnListViewIcon;
    multiColumnListViewIcon = 0;
    delete cdToParentIcon;
    cdToParentIcon = 0;
    delete tmpString;
    tmpString = 0;
    delete workingDirectory;
    workingDirectory = 0;
}


static void makeVariables() {
    if ( !tmpString ) {
	qAddPostRoutine( cleanup );
	tmpString = new QString;
	workingDirectory = new QString;
	openFolderIcon = new QPixmap(open_xpm);
	closedFolderIcon = new QPixmap(closed_xpm);
	detailViewIcon = new QPixmap(detailedview_xpm);
	multiColumnListViewIcon = new QPixmap(mclistview_xpm);
	cdToParentIcon = new QPixmap(cdtoparent_xpm);
	fifteenTransparentPixels = new QPixmap( closedFolderIcon->width(), 1 );
	QBitmap m( fifteenTransparentPixels->width(), 1 );
	m.fill( color0 );
	fifteenTransparentPixels->setMask( m );
    }
}


struct QFileDialogPrivate {
    bool geometryDirty;
    QComboBox * paths;
    QComboBox * types;
    QLabel * pathL;
    QLabel * fileL;
    QLabel * typeL;

    QVBoxLayout * topLevelLayout;
    QHBoxLayout * extraWidgetsLayout;
    QLabel * extraLabel;
    QWidget * extraWidget;
    QButton * extraButton;

    QWidgetStack * stack;

    QPushButton * cdToParent, * detailView, * mcView;
    QButtonGroup * modeButtons;

    QString currentFileName;

    struct File: public QListViewItem {
	File( const QFileInfo * fi, QListViewItem * parent, int h )
	    : QListViewItem( parent ), info( *fi ) { setHeight( h ); }
	File( const QFileInfo * fi, QListView * parent, int h  )
	    : QListViewItem( parent ), info( *fi ) { setHeight( h ); }

	const char * text( int column ) const;
	const char * key( int column, bool ) const;
	const QPixmap * pixmap( int ) const;

	QFileInfo info;
    };

    class MCList: public QTableView {
    public:
	MCList( QListView *, QWidget * );
	~MCList();
	void paintCell( QPainter *, int row, int col );

	void clear();
    protected:
	void mousePressEvent( QMouseEvent * );
	void mouseMoveEvent( QMouseEvent * );
	void mouseReleaseEvent( QMouseEvent * );
	void mouseDoubleClickEvent( QMouseEvent * );
	void focusInEvent( QFocusEvent * );
	void focusOutEvent( QFocusEvent * );
	void paintEvent( QPaintEvent * );
	void resizeEvent( QResizeEvent * );
	void keyPressEvent( QKeyEvent * );
	int cellWidth ( int );
    private:
	void setFocusToPoint( const QPoint & );
	void setUpContents();
	void updateItem( QListViewItem * );
	void ensureVisible( int col );
	QListView * lv;
	QVector<QListViewItem> * items;
	QArray<int> * widths;
    };

    MCList * moreFiles;

    QFileDialog::Mode mode;
};


const char * QFileDialogPrivate::File::text( int column ) const
{
    makeVariables();

    switch( column ) {
    case 0:
	*tmpString = info.fileName();
	break;
    case 1:
	if ( info.isFile() )
	    tmpString->sprintf( "%d", info.size() );
	else
	    *tmpString = "";
	break;
    case 2:
	if ( info.isFile() )
	    *tmpString = "File";
	else if ( info.isDir() )
	    *tmpString = "Directory";
	else
	    *tmpString = "Special File";
	if ( info.isSymLink() )
	    tmpString->prepend( "Link to " );
	break;
    case 3:
	{
	    QDateTime epoch;
	    epoch.setTime_t( 0 );
	    char a[256];
	    time_t t1 = epoch.secsTo( info.lastModified() );
	    struct tm * t2 = ::localtime( &t1 );
	    if ( strftime( a, 255, "%x  %X", t2 ) > 0 )
		*tmpString = a;
	    else
		*tmpString = "????";
	}
	break;
    case 4:
	if ( info.isReadable() )
	    *tmpString = info.isWritable() ? "Read-write" : "Read-only";
	else
	    *tmpString = info.isWritable() ? "Write-only" : "Inaccessible";
	break;
    default:
	*tmpString = "<--->";
    }

    tmpString->detach();
    return *tmpString;
}


const QPixmap * QFileDialogPrivate::File::pixmap( int column ) const
{
    if ( column )
	return 0;
    else if ( fileIconProvider )
	return fileIconProvider->pixmap( info );
    else if ( info.isDir() )
	return closedFolderIcon;
    else
	return fifteenTransparentPixels;
}


const char * QFileDialogPrivate::File::key( int column, bool ascending ) const
{
    makeVariables();
    QDateTime epoch( QDate( 1968, 6, 19 ) );

    char majorkey = ascending == info.isDir() ? '0' : '1';

    if ( info.fileName() == ".." ) {
        *tmpString = ascending ? "0" : "a"; // a > 9
    } else if ( column == 1 ) {
	tmpString->sprintf( "%c%08d", majorkey, info.size() );
    } else if ( column == 3 ) {
	tmpString->sprintf( "%c%08d",
			    majorkey, epoch.secsTo( info.lastModified() ) );
    } else {
	*tmpString = text( column );
	tmpString->insert( 0, majorkey );
    }

    return *tmpString;
}



QFileDialogPrivate::MCList::MCList( QListView * files, QWidget * parent )
    : QTableView( parent, "multi-column list box" )
{
    lv = files;
    items = 0;
    widths = 0;
    setCellHeight( fontMetrics().height() + 2*files->itemMargin() );
    setCellWidth( 0 );
    setBackgroundMode( PaletteBase ); // NoBackground for really cool bugs
}



QFileDialogPrivate::MCList::~MCList()
{
    delete items;
    delete widths;
}


int QFileDialogPrivate::MCList::cellWidth ( int c )
{
    if ( !widths )
	setUpContents();
    if ( !widths || c >= (int)(widths->size()) )
	return 25;
    return (*widths)[c];
}


void QFileDialogPrivate::MCList::paintCell( QPainter *p, int row, int col )
{
    if ( (uint)(col * numRows() + row) >= items->count() )
	return;

    QListViewItem * file = (*items)[col*numRows() + row];
    if ( file ) {
	file->paintCell( p, colorGroup(), 0, cellWidth(col), AlignLeft );
	if ( lv->currentItem() == file )
	    file->paintFocus( p, colorGroup(),
			      QRect( 0, 0,
				     cellWidth( col ), cellHeight( row ) ) );
    }
}


void QFileDialogPrivate::MCList::clear()
{
    delete items;
    items = 0;
}

void QFileDialogPrivate::MCList::setFocusToPoint( const QPoint & p )
{
    if ( !items )
	setUpContents();

    int row = findRow( p.y() ), col = findCol( p.x() );
    if ( row < 0 || col < 0 )
	return;

    int i = col * numRows() + row;
    if ( i >= (int)items->count() )
	return;

    const QListViewItem * file = (*items)[col*numRows() + row];
    if ( !file )
	return;

    QListViewItem * prev = lv->currentItem();
    if ( prev != file ) {
	bool s = TRUE;
	if ( lv->isMultiSelection() &&
	     ( prev ? !prev->isSelected() : file->isSelected() ) )
	    s = FALSE;
	lv->setSelected( (QListViewItem *)file, s );
	lv->setCurrentItem( (QListViewItem *)file );
	ensureVisible( col );
	updateCell( row, col );
    }
    updateItem( prev );
}


void QFileDialogPrivate::MCList::ensureVisible( int col )
{
    if ( col < leftCell() ) {
	setLeftCell( col );
    } else if ( col >= lastColVisible() ) {
	int w = viewWidth();
	int i = col;
	while( i >= 0 && w > 0 ) {
	    w -= cellWidth( i );
	    if ( w >= 0 )
		i--;
	}
	if ( w < 0 && i < col )
	    i++;
	if ( i < 0 )
	    i = 0;
	setLeftCell( i );
    }
}


void QFileDialogPrivate::MCList::setUpContents()
{
    bool oldAutoUpdate = autoUpdate();
    setAutoUpdate( FALSE );
    const QListViewItem * file;
    if ( !items ) {
	setNumRows( QMAX( 1, (height()+2) / cellHeight() ) );

	file = lv->firstChild();
	int i = 0;
	int w, maxw, totalw;
	maxw = 40;
	totalw = 0;
	
	int count = 0;
	while( file ) {
	    file = file->nextSibling();
	    count++;
	}
	
	file = lv->firstChild();

	delete widths;
	widths = new QArray<int>( (count+numRows()-1)/numRows() );

	while( file ) {
	    if ( file->height() ) {
		w = file->width( fontMetrics(), lv, 0 );
		if ( w > maxw )
		    maxw = w;
		file = file->nextSibling();
		if ( (i+1) % numRows() == 0 || !file ) {
		    (*widths)[ i / numRows() ] = maxw;
		    totalw += maxw;
		    maxw = 40;
		}
		i++;
	    } else {
		file = file->nextSibling();
	    }
	}

	if ( totalw > width() ) {
	    i = numRows();
	    setNumRows( (height() + 2 - horizontalScrollBar()->height())
			/ cellHeight() );
	    if ( i != numRows() ) {
		// ### not optimal; repeated code.
		file = lv->firstChild();
		i = 0;
		maxw = 40;
		/* totalw = 0; */

		delete widths;
		widths = new QArray<int>( (count+numRows()-1)/numRows() );

		while( file ) {
		    if ( file->height() ) {
			w = file->width( fontMetrics(), lv, 0 );
			if ( w > maxw )
			    maxw = w;
			file = file->nextSibling();
			if ( (i+1) % numRows() == 0 || !file ) {
			    (*widths)[ i / numRows() ] = maxw;
			    /* totalw += maxw; */
			    maxw = 40;
			}
			i++;
		    } else {
			file = file->nextSibling();
		    }
		}
	    }
	    setTableFlags( Tbl_hScrollBar
			   + Tbl_snapToHGrid + Tbl_clipCellPainting
			   + Tbl_cutCellsV + Tbl_smoothHScrolling );
	} else {
	    setTableFlags( Tbl_snapToHGrid + Tbl_clipCellPainting
			   + Tbl_cutCellsV );
	}
	setNumCols( widths->size() );

	items = new QVector<QListViewItem>( i );
	i = 0;
	file = lv->firstChild();
	// may have the wrong order.  fix that later.
	while( file ) {
	    if ( file->height() )
		items->insert( i++, file );
	    file = file->nextSibling();
	}
    }
    setAutoUpdate( oldAutoUpdate );
}


void QFileDialogPrivate::MCList::updateItem( QListViewItem * file )
{
    setUpContents();
    int i = items->count() -1 ;
    while( i >= 0 ) {
	if ( (*items)[i] == file ) {
	    // and all this because it's too much work to
	    // add a proper QListBox::setMultiColumn()? hm...

	    int col, row;
	    col = i / numRows();
	    row = i - ( col * numRows() );
	    updateCell( row, col );
	    return;
	}
	i--;
    }
}


void QFileDialogPrivate::MCList::mousePressEvent( QMouseEvent * e )
{
    if ( e ) {
	if ( lv->isMultiSelection() && lv->currentItem() ) {
	    QListViewItem * i = lv->currentItem();
	    lv->setCurrentItem( 0 );
	    updateItem( i );
	}
	setFocusToPoint( e->pos() );
    }
}


void QFileDialogPrivate::MCList::mouseMoveEvent( QMouseEvent * e )
{
    if ( e )
	setFocusToPoint( e->pos() );
}


void QFileDialogPrivate::MCList::mouseReleaseEvent( QMouseEvent * e )
{
    if ( e )
	setFocusToPoint( e->pos() );
}


void QFileDialogPrivate::MCList::mouseDoubleClickEvent( QMouseEvent * e )
{
    if ( e ) {
	setFocusToPoint( e->pos() );
	// ### uglehack alert ###
	QKeyEvent ke( Event_KeyPress, Key_Enter, 0, 0 );
	QApplication::sendEvent( lv, &ke );
    }
}


void QFileDialogPrivate::MCList::paintEvent( QPaintEvent * e )
{
    if ( !items )
	setUpContents();
    QTableView::paintEvent( e );
}


void QFileDialogPrivate::MCList::resizeEvent( QResizeEvent * e )
{
    clear();
    setNumRows( (height()+2) / cellHeight() );
    QTableView::resizeEvent( e );
}


void QFileDialogPrivate::MCList::keyPressEvent( QKeyEvent * e )
{
    if ( !e )
	return;

    int col=0, row=0;
    setUpContents();
    int i = items->count() -1 ;
    while( i >= 0 ) {
	if ( (*items)[i] == lv->currentItem() ) {
	    col = i / numRows();
	    row = i - ( col * numRows() );
	    break;
	}
	i--;
    }

    switch( e->key() ) {
    case Key_Down:
	e->accept();
	if ( row < numRows() - 1 )
	    row++;
	break;
    case Key_Up:
	e->accept();
	if ( row )
	    row--;
	break;
    case Key_Left:
	e->accept();
	if ( col )
	    col--;
	break;
    case Key_Right:
	e->accept();
	if ( col < numCols() - 1 )
	    col++;
	break;
    case Key_Space:
	if ( lv->isMultiSelection() && lv->currentItem() ) {
	    QListViewItem * i = lv->currentItem();
	    lv->setSelected( i, !i->isSelected() );
	    updateItem( i );
	    return;
	}
    case Key_Enter:
	QApplication::sendEvent( lv, e );
	clear();
	update();
	return;
    default:
	if ( e->ascii() > 32 && e->ascii() < 127 ) {
	    QListViewItem * oldFile = lv->currentItem();
	    QApplication::sendEvent( lv, e );
	    QListViewItem * newFile = lv->currentItem();
	    if ( oldFile != newFile ) {
		updateItem( oldFile );
		updateItem( newFile );
	    }
	}
	return;
    }

    if ( col * numRows() + row >= (int)(items->count()) )
	return;

    QListViewItem * prev = lv->currentItem();
    QListViewItem * file = (*items)[col * numRows() + row];
    if ( prev != file ) {
	if ( lv->isMultiSelection() && e->state() && prev )
	    lv->setSelected( file, prev->isSelected() );
	else if ( !lv->isMultiSelection() )
	lv->setSelected( file, TRUE );
	lv->setCurrentItem( file );
	ensureVisible( col );
	updateCell( row, col );
	updateItem( prev );
    }
}


void QFileDialogPrivate::MCList::focusInEvent( QFocusEvent * )
{
    updateItem( lv->currentItem() );
}


void QFileDialogPrivate::MCList::focusOutEvent( QFocusEvent * )
{
    updateItem( lv->currentItem() );
}

/*!
  \class QFileDialog qfiledialog.h
  \brief The QFileDialog provides a dialog widget for inputting file names.
  \ingroup dialogs

  Example:
  \code
    QString fileName = QFileDialog::getOpenFileName();
    if ( !fileName.isNull() ) {			// got a file name
	...
    }
  \endcode

  There are two ready-made convenience functions, getOpenFileName()
  and getSaveFileName(), which may be used like this:

  \code
    QString s( QFileDialog::getOpenFileName() );
    if ( s.isNull() )
	return;

    open( s ); // open() being your function to read the file
  \endcode

  <img src=qfiledlg-m.gif> <img src=qfiledlg-w.gif>

  \sa QPrintDialog
*/


/*!
  Constructs a file dialog with a \e parent, \e name and \e modal flag.

  The dialog becomes modal if \e modal is TRUE, otherwise modeless.
*/

QFileDialog::QFileDialog( QWidget *parent, const char *name, bool modal )
    : QDialog( parent, name, modal )
{
    init();
    cwd.convertToAbs();
    rereadDir();
}


/*!
  Constructs a file dialog with a \e parent, \e name and \e modal flag.

  The dialog becomes modal if \e modal is TRUE, otherwise modeless.
*/

QFileDialog::QFileDialog( const char *dirName, const char *filter,
			  QWidget *parent, const char *name, bool modal )
    : QDialog( parent, name, modal )
{
    init();
    if ( filter ) {
	cwd.setNameFilter( filter );
	d->types->insertItem( filter );
    } else {
	d->types->insertItem( tr( "All files (*)" ) );
    }
    if ( dirName )
	cwd.setPath( dirName );

    cwd.convertToAbs();
    rereadDir();
}


/*!
  \internal
  Initializes the file dialog.
*/

void QFileDialog::init()
{
    d = new QFileDialogPrivate();
    d->mode = AnyFile;

    nameEdit = new QLineEdit( this, "name/filter editor" );
    connect( nameEdit, SIGNAL(textChanged(const char*)),
	     this,  SLOT(fileNameEditDone()) );
    nameEdit->installEventFilter( this );

    d->stack = new QWidgetStack( this, "files and more files" );
    d->stack->setFrameStyle( QFrame::WinPanel + QFrame::Sunken );
			
    files = new QListView( d->stack, "current directory listing" );
    QFontMetrics fm( fontMetrics() );
    files->addColumn( tr("Name"), 150 );
    files->setColumnWidthMode( 0, QListView::Manual );
    files->addColumn( tr("Size"), 30 + fm.width( tr("Size") ) );
    files->setColumnWidthMode( 1, QListView::Maximum );
    files->setColumnAlignment( 1, AlignRight );
    files->addColumn( tr("Type"), 10 + fm.width( tr("Directory") ) );
    files->setColumnWidthMode( 2, QListView::Maximum );
    files->addColumn( tr("Date"), 50 );
    files->setColumnWidthMode( 3, QListView::Maximum );
    files->addColumn( tr("Attributes"), 10 + fm.width( tr("Attributes") ) );
    files->setColumnWidthMode( 0, QListView::Maximum );

    files->setMinimumSize( 50, 25 + 2*fontMetrics().lineSpacing() );

    connect( files, SIGNAL(selectionChanged(QListViewItem *)),
	     this, SLOT(updateFileNameEdit(QListViewItem *)) );
    connect( files, SIGNAL(doubleClicked(QListViewItem *)),
	     this, SLOT(selectDirectoryOrFile(QListViewItem *)) );
    connect( files, SIGNAL(returnPressed(QListViewItem *)),
	     this, SLOT(selectDirectoryOrFile(QListViewItem *)) );
    connect( files, SIGNAL(rightButtonClicked(QListViewItem *,
					      const QPoint &, int)),
	     this, SLOT(popupContextMenu(QListViewItem *,
					 const QPoint &, int)) );
    files->setFocusPolicy( StrongFocus );

    files->installEventFilter( this );
    files->viewport()->installEventFilter( this );

    d->moreFiles = new QFileDialogPrivate::MCList( files, d->stack );
    d->moreFiles->setFrameStyle( QFrame::NoFrame );
    d->moreFiles->setFocusPolicy( StrongFocus );

    okB = new QPushButton( tr("OK"), this, "OK" ); //### Or "Save (see other "OK")
    okB->setAutoDefault( TRUE );
    okB->setDefault( TRUE );
    okB->setEnabled( FALSE );
    connect( okB, SIGNAL(clicked()), this, SLOT(okClicked()) );
    cancelB = new QPushButton( tr("Cancel") , this, "Cancel" );
    cancelB->setAutoDefault( TRUE );
    connect( cancelB, SIGNAL(clicked()), this, SLOT(cancelClicked()) );

    d->paths = new QComboBox( TRUE, this, "directory history/editor" );
    const QFileInfoList * rootDrives = QDir::drives();
    QFileInfoListIterator it( *rootDrives );
    QFileInfo *fi;
    while ( (fi = it.current()) != 0 ) {
	++it;
	d->paths->insertItem( fi->absFilePath() );
    }
    connect( d->paths, SIGNAL(activated(const char *)),
	     this, SLOT(setDir(const char *)) );

    d->geometryDirty = TRUE;
    d->types = new QComboBox( TRUE, this, "file types" );
    connect( d->types, SIGNAL(activated(const char *)),
	     this, SLOT(setFilter(const char *)) );

    d->pathL = new QLabel( d->paths, tr("Look &in"), this );
    d->fileL = new QLabel( nameEdit, tr("File &name"), this );
    d->typeL = new QLabel( d->types, tr("File &type"), this );

    makeVariables();

    d->cdToParent = new QPushButton( this, "cd to parent" );
    d->cdToParent->setPixmap( *cdToParentIcon );
    connect( d->cdToParent, SIGNAL(clicked()),
	     this, SLOT(cdUpClicked()) );

    d->modeButtons = new QButtonGroup( 0, "invisible group" );
    connect( d->modeButtons, SIGNAL(destroyed()),
	     this, SLOT(modeButtonsDestroyed()) );
    d->modeButtons->setExclusive( TRUE );
    connect( d->modeButtons, SIGNAL(clicked(int)),
	     d->stack, SLOT(raiseWidget(int)) );

    d->detailView = new QPushButton( this, "list view" );
    d->detailView->setPixmap( *detailViewIcon );
    d->detailView->setToggleButton( TRUE );
    d->stack->addWidget( files, d->modeButtons->insert( d->detailView ) );
    d->mcView = new QPushButton( this, "mclistbox view" );
    d->mcView->setPixmap( *multiColumnListViewIcon );
    d->mcView->setToggleButton( TRUE );
    d->stack->addWidget( d->moreFiles, d->modeButtons->insert( d->mcView ) );

#if QT_ENABLE_INCOMPLETE_FEATURES
    d->stack->raiseWidget( d->moreFiles );
    d->mcView->setOn( TRUE );
#else
    d->stack->raiseWidget( files );
    d->detailView->setOn( TRUE );
    d->mcView->setEnabled( FALSE );
#endif

    d->topLevelLayout = new QVBoxLayout( this, 5 );
    d->extraWidgetsLayout = 0;
    d->extraLabel = 0;
    d->extraWidget = 0;
    d->extraButton = 0;

    QHBoxLayout * h;

    h = new QHBoxLayout( 0 );
    d->topLevelLayout->addLayout( h );
    h->addWidget( d->pathL );
    h->addSpacing( 8 );
    h->addWidget( d->paths );
    h->addSpacing( 8 );
    h->addWidget( d->cdToParent );
    h->addSpacing( 8 );
    h->addWidget( d->detailView );
    h->addWidget( d->mcView );
    h->addSpacing( 16 );

    d->topLevelLayout->addWidget( d->stack, 3 );

    h = new QHBoxLayout();
    d->topLevelLayout->addLayout( h );
    h->addWidget( d->fileL );
    h->addWidget( nameEdit );
    h->addSpacing( 15 );
    h->addWidget( okB );

    h = new QHBoxLayout();
    d->topLevelLayout->addLayout( h );
    h->addWidget( d->typeL );
    h->addWidget( d->types );
    h->addSpacing( 15 );
    h->addWidget( cancelB );

    cwd.setMatchAllDirs( TRUE );
    cwd.setSorting( cwd.sorting() );

    updateGeometry();

    d->cdToParent->setFocusPolicy( NoFocus );
    d->detailView->setFocusPolicy( NoFocus );
    d->mcView->setFocusPolicy( NoFocus );

    setTabOrder( d->paths, d->moreFiles );
    setTabOrder( d->moreFiles, files );
    setTabOrder( files, nameEdit );
    setTabOrder( nameEdit, d->types );
    setTabOrder( d->types, okB );
    setTabOrder( okB, cancelB );

    nameEdit->setFocus();

    setFontPropagation( SameFont );
    setPalettePropagation( SamePalette );

    if ( QApplication::desktop()->width() < 1024 ||
	 QApplication::desktop()->height() < 768 ) {
	resize( 420, 236 );
    } else {
	QSize s( files->sizeHint() );
	s = QSize( s.width() + 50, s.height() + 80 );

	if ( s.width() * 3 > QApplication::desktop()->width() * 2 )
	    s.setWidth( QApplication::desktop()->width() * 2 / 3 );

	if ( s.height() * 3 > QApplication::desktop()->height() * 2 )
	    s.setHeight( QApplication::desktop()->height() * 2 / 3 );
	else if ( s.height() * 3 < QApplication::desktop()->height() )
	    s.setHeight( QApplication::desktop()->height() / 3 );

	resize( s );
    }
}

/*!
  Destroys the file dialog.
*/

QFileDialog::~QFileDialog()
{
    delete d->modeButtons;
    delete d;
}


/*!
  Returns the selected file name.

  If a file name was selected, the returned string contains the
  absolute path name.  The returned string is a null string if no file
  name was selected.

  \sa QString::isNull()
*/

QString QFileDialog::selectedFile() const
{
    return d->currentFileName;
}

/*!
  Sets the default selection to \a filename.  If \a filename is
  absolute, setDir() is also called.

  \internal
  Only for external use.  Not useful inside QFileDialog.
*/
void QFileDialog::setSelection( const char* filename )
{
    QFileInfo info(filename);
    if ( info.isDir() ) {
	setDir( filename );
	nameEdit->setText( "" );
    } else {
	setDir( info.dir() );
	nameEdit->setText( info.fileName() );
    }
    trySetSelection( info, FALSE );
}

/*!
  Returns the active directory path string in the file dialog.
  \sa dir(), setDir()
*/

const char *QFileDialog::dirPath() const
{
    return cwd.path();
}


/*!  Sets the filter spec in use to \a newFilter.

  If \a newFilter matches the regular expression
  <tt>([a-zA-Z0-9\.\*\?]*)$</tt> (ie. it ends with a normal wildcard
  expression enclosed in parentheses), only the parenthesized is used.
  This means that these two calls are equivalent:

  \code
     fd->setFilter( "All perl files (*.pl)" );
     fd->setFilter( "*.pl" )
  \endcode
*/

void QFileDialog::setFilter( const char * newFilter )
{
    if ( !newFilter )
	return;
    QString f = newFilter;
    QRegExp r( "([a-zA-Z0-9\\.\\*\\?]*)$" );
    int len;
    int index = r.match( f, 0, &len );
    if ( index >= 0 )
	f = f.mid( index+1, len-2 );
    cwd.setNameFilter( f );
    rereadDir();
}


/*!
  Sets a directory path string for the file dialog.
  \sa dir()
*/

void QFileDialog::setDir( const char *pathstr )
{
    QDir tmp( pathstr );
    setDir( tmp );
}

/*!
  Returns the active directory in the file dialog.
  \sa setDir()
*/

const QDir *QFileDialog::dir() const
{
    return &cwd;
}

/*!
  Sets a directory path for the file dialog.
  \sa dir()
*/

void QFileDialog::setDir( const QDir &dir )
{
    if ( !dir.exists() ||
	 dir.absPath() == cwd.absPath() )
	return;
    QString nf( cwd.nameFilter() );
    cwd = dir;
    cwd.setNameFilter( nf );
    cwd.convertToAbs();
    cwd.setMatchAllDirs( TRUE );
    cwd.setSorting( cwd.sorting() );
    QFileInfo i( cwd, nameEdit->text() );
    trySetSelection( i, FALSE );
    rereadDir();
}


/*!
  Re-reads the active directory in the file dialog.

  It is seldom necessary to call this function.	 It is provided in
  case the directory contents change and you want to refresh the
  directory list box.
*/

void QFileDialog::rereadDir()
{
    if ( d ) {
	QString cp( cwd.canonicalPath() );
	int i = d->paths->count()-1;
	while( i >= 0 && strcmp( d->paths->text( i ), cp ) >= 0 )
	    i--;
	if ( i < d->paths->count() )
	    i++;
	if ( i == d->paths->count() || strcmp( d->paths->text( i ), cp ) )
	    d->paths->insertItem( cwd.canonicalPath(), i );
	d->paths->setCurrentItem( i );
    }

    d->cdToParent->setEnabled( !cwd.isRoot() );

    const QFileInfoList *filist = 0;

    int itemHeight = fontMetrics().height() + 6;

    while ( !filist ) {
	filist = cwd.entryInfoList();
	if ( !filist &&
	     QMessageBox::warning( this, tr("Open File"),
				   QString( tr("Unable to read directory\n") )
				   + cwd.absPath() + "\n\n" +
				   tr("Please make sure that the directory\n"
				      "is readable.\n"),
				   tr("Use Parent Directory"),
				   tr("Use Old Contents"), 0 ) ) {
	    return;
	}
	if ( !filist ) {
	    QString tmp( cwd.absPath() );
	
	    // change to parent, reread
	    // ...

	    // but for now
	    return;
	}
    }

    files->clear();

    QFileInfoListIterator it( *filist );
    QFileInfo *fi;
    while ( (fi = it.current()) != 0 ) {
	++it;
	if ( fi->fileName() != "." &&
	     ( !cwd.isRoot() || fi->fileName() != ".." ) ) {
	    QListViewItem * i
		= new QFileDialogPrivate::File( fi, files, itemHeight );
	    if ( mode() == ExistingFiles && fi->isDir() )
		i->setSelectable( FALSE );
	}
    }
    d->moreFiles->clear();
    d->moreFiles->repaint();
}


/*!
  \fn void QFileDialog::fileHighlighted( const char * )

  This signal is emitted when the user highlights a file.
*/

/*!
  \fn void QFileDialog::fileSelected( const char * )

  This signal is emitted when the user selects a file.
*/

/*!
  \fn void QFileDialog::dirEntered( const char * )

  This signal is emitted when the user has selected a new directory.
*/

// Defined in qapplication.cpp:
void qt_enter_modal( QWidget* );
void qt_leave_modal( QWidget* );

/*!
  Opens a modal file dialog and returns the name of the file to be
  opened.

  If \a startWith is the name of a directory, the dialog starts off in
  that directory.  If \a startWith is the name of an existing file,
  the dialogs starts in that directory, and with \a startWith
  selected.

  Only files matching \a filter are selectable.  If \a filter is 0,
  all files are selectable.

  If \a widget and/or \a name is provided, the dialog will be centered
  over \a widget and \link QObject::name() named \endlink \a name.

  getOpenFileName() returns a \link QString::isNull() null string
  \endlink if the user cancelled the dialog.

  This static function is less capable than the full QFileDialog object,
  but is convenient and easy to use.

  Example:
  \code
    // start at the current working directory and with *.cpp as filter
    QString f = QFileDialog::getOpenFileName( 0, "*.cpp", this );
    if ( !f.isEmpty() ) {
        // the user selected a valid existing file
    } else {
        // the user cancelled the dialog
    }
  \endcode

  getSaveFileName() is another convenience function, equal to this one
  except that it allows the user to specify the name of a nonexistent file
  name.

  \sa getSaveFileName()
*/

QString QFileDialog::getOpenFileName( const char *startWith,
				      const char *filter,
				      QWidget *parent, const char *name )
{
    makeVariables();
    QString initialSelection;
    if ( startWith && *startWith ) {
	QFileInfo fi( startWith );
	if ( fi.exists() && fi.isDir() ) {
	    *workingDirectory = startWith;
	} else if ( fi.exists() && fi.isFile() ) {
	    *workingDirectory = fi.dirPath( TRUE );
	    initialSelection = fi.absFilePath();
	}
    }

    if ( workingDirectory->isNull() )
	*workingDirectory = QDir::currentDirPath();

#if defined(_WS_WIN_)

    *workingDirectory = QDir::convertSeparators( *workingDirectory );

    const int maxstrlen = 256;
    char *file = new char[maxstrlen];
    file[0] = '\0';
    if ( !initialSelection.isEmpty() )
	qstrncpy( file, QDir::convertSeparators(initialSelection), 255 );

    const char all_filter[] = "All Files\0*.*\0\0";
    const int all_len = sizeof(all_filter); // 15
    char* win_filter;
    int total_len = 0;
    if (filter) {
	int fl = strlen(filter)+1; // Include nul
	win_filter = new char[2*fl+all_len];
	for (int i=0; i<2; i++) {
	    memcpy(win_filter+total_len, filter, fl);
	    total_len += fl;
	}
    } else {
	win_filter = new char[all_len];
    }
    memcpy(win_filter+total_len, all_filter, all_len);

    if ( parent )
	parent = parent->topLevelWidget();
    else
	parent = qApp->mainWidget();

    OPENFILENAME ofn;
    memset( &ofn, 0, sizeof(OPENFILENAME) );
    ofn.lStructSize	= sizeof(OPENFILENAME);
    ofn.hwndOwner	= parent ? parent->winId() : 0;
    ofn.lpstrFilter	= win_filter;
    ofn.lpstrFile	= file;
    ofn.nMaxFile	= maxstrlen;
    ofn.lpstrInitialDir = *workingDirectory;
    ofn.lpstrTitle	= "Open";
    ofn.Flags		= (OFN_CREATEPROMPT|OFN_NOCHANGEDIR|OFN_HIDEREADONLY);

    QString result;

    if ( GetOpenFileName(&ofn) ) {
	result = file;
	*workingDirectory = QFileInfo(file).dirPath();
    }

    delete [] win_filter;
    delete [] file;
    return result;

#else

    QFileDialog *dlg = new QFileDialog( *workingDirectory, filter,
					parent, name, TRUE );
    CHECK_PTR( dlg );
    dlg->setCaption( "Open" );
    if ( !initialSelection.isEmpty() )
	dlg->setSelection( initialSelection );
    dlg->setMode( QFileDialog::ExistingFile );
    QString result;
    if ( dlg->exec() == QDialog::Accepted ) {
	result = dlg->selectedFile();
	*workingDirectory = dlg->dirPath();
    }
    delete dlg;
    return result;

#endif
}

/*!
  Opens a modal file dialog and returns the name of the file to be
  saved.

  If \a startWith is the name of a directory, the dialog starts off in
  that directory.  If \a startWith is the name of an existing file,
  the dialogs starts in that directory, and with \a startWith
  selected.

  Only files matching \a filter are selectable.  If \a filter is 0,
  all files are selectable.

  If \a widget and/or \a name is provided, the dialog will be centered
  over \a widget and \link QObject::name() named \endlink \a name.

  Returns a \link QString::isNull() null string\endlink if the user
  cancelled the dialog.

  This static function is less capable than the full QFileDialog object,
  but is convenient and easy to use.

  Example:
  \code
    // start at the current working directory and with *.cpp as filter
    QString f = QFileDialog::getSaveFileName( 0, "*.cpp", this );
    if ( !f.isEmpty() ) {
        // the user gave a file name
    } else {
        // the user cancelled the dialog
    }
  \endcode

  getOpenFileName() is another convenience function, equal to this one
  except that it does not allow the user to specify the name of a
  nonexistent file name.

  \sa getOpenFileName()
*/

QString QFileDialog::getSaveFileName( const char *startWith,
				      const char *filter,
				      QWidget *parent, const char *name )
{
    makeVariables();
    QString initialSelection;
    if ( startWith && *startWith ) {
	QFileInfo fi( startWith );
	if ( fi.exists() && fi.isDir() ) {
	    *workingDirectory = startWith;
	} else if ( !fi.exists() || fi.isFile() ) {
	    *workingDirectory = fi.dirPath( TRUE );
	    initialSelection = fi.absFilePath();
	}
    }

    if ( workingDirectory->isNull() )
	*workingDirectory = QDir::currentDirPath();

#if defined(_WS_WIN_)

    *workingDirectory = QDir::convertSeparators( *workingDirectory );

    const int maxstrlen = 256;
    char *file = new char[maxstrlen];
    file[0] = '\0';
    if ( !initialSelection.isEmpty() )
	qstrncpy( file, QDir::convertSeparators(initialSelection), 255 );

    const char all_filter[] = "All Files\0*.*\0\0";
    const int all_len = sizeof(all_filter); // 15
    char* win_filter;
    int total_len = 0;
    if (filter) {
	int fl = strlen(filter)+1; // Include nul
	win_filter = new char[2*fl+all_len];
	for (int i=0; i<2; i++) {
	    memcpy(win_filter+total_len, filter, fl);
	    total_len += fl;
	}
    } else {
	win_filter = new char[all_len];
    }
    memcpy(win_filter+total_len, all_filter, all_len);

    if ( parent )
	parent = parent->topLevelWidget();
    else
	parent = qApp->mainWidget();

    OPENFILENAME ofn;
    memset( &ofn, 0, sizeof(OPENFILENAME) );
    ofn.lStructSize	= sizeof(OPENFILENAME);
    ofn.hwndOwner	= parent ? parent->winId() : 0;
    ofn.lpstrFilter	= win_filter;
    ofn.lpstrFile	= file;
    ofn.nMaxFile	= maxstrlen;
    ofn.lpstrInitialDir = *workingDirectory;
    ofn.lpstrTitle	= "Save";
    ofn.Flags		= (OFN_CREATEPROMPT|OFN_NOCHANGEDIR|OFN_HIDEREADONLY);

    QString result;

    if ( GetSaveFileName(&ofn) ) {
	result = file;
	*workingDirectory = QFileInfo(file).dirPath();
    }

    delete [] win_filter;
    delete [] file;
    return result;

#else

    QFileDialog *dlg = new QFileDialog( *workingDirectory, filter, parent, name, TRUE );
    CHECK_PTR( dlg );
    dlg->setCaption( "Save As" );
    QString result;
    if ( !initialSelection.isEmpty() )
	dlg->setSelection( initialSelection );
    if ( dlg->exec() == QDialog::Accepted ) {
	result = dlg->selectedFile();
	*workingDirectory = dlg->dirPath();
    }
    delete dlg;
    return result;

#endif
}


/*!
  \internal
  Activated when the "Ok" button is clicked.
*/

void QFileDialog::okClicked()
{
    // if we're in multi-selection mode and something is selected,
    // accept it and be done.
    if ( mode() == ExistingFiles ) {
	QListViewItem * i = files->firstChild();
	while( i && !i->isSelected() )
	    i = i->nextSibling();
	if ( i )
	    accept();
    }

    // If selection is valid, return it, else try
    // using selection as a directory to change to.
    if ( !d->currentFileName.isNull() ) {
	emit fileSelected( d->currentFileName );
	accept();
    } else {
	QFileInfo f;
	QFileDialogPrivate::File * c = (QFileDialogPrivate::File *)files->currentItem();
        if ( c && files->isSelected(c) )
	    f = c->info;
	else
	    f = QFileInfo( cwd, nameEdit->text() );
	if ( f.isDir() ) {
	    setDir( f.absFilePath() );
	    QFileInfo f ( cwd, "." );
	    trySetSelection( f, TRUE );
	}
    }
}

/*!
  \internal
  Activated when the "Filter" button is clicked.
*/

void QFileDialog::filterClicked()
{
    // unused
}

/*!
  \internal
  Activated when the "Cancel" button is clicked.
*/

void QFileDialog::cancelClicked()
{
    reject();
}


/*!
  Handles resize events for the file dialog.
*/

void QFileDialog::resizeEvent( QResizeEvent * )
{
    updateGeometry();
}

/*! \internal

  Obsolete.
*/
void QFileDialog::updatePathBox( const char * )
{
    // unused
}

/*
  \internal
  The only correct way to try to set currentFileName
*/
bool QFileDialog::trySetSelection( const QFileInfo& info, bool updatelined )
{
    QString old = d->currentFileName;

    if ( mode() == Directory ) {
	if ( info.isDir() )
	    d->currentFileName = info.absFilePath();
	else
	    d->currentFileName = 0;
    } else if ( info.isFile() && mode() == ExistingFiles ) {
	d->currentFileName = info.absFilePath();
    } else if ( info.isFile() || (mode() == AnyFile && !info.isDir()) ) {
	d->currentFileName = info.absFilePath();
    } else {
	d->currentFileName = 0;
    }
    if ( updatelined && !d->currentFileName.isNull() ) {
	// If the selection is valid, or if its a directory, allow OK.
	QString tmp;
	if ( !d->currentFileName.isNull() || info.isDir() )
	    tmp = info.fileName();
	nameEdit->setText( tmp );
    }

    if ( !d->currentFileName.isNull() || info.isDir() ) {
	okB->setEnabled( TRUE );
	if ( d->currentFileName.isNull() && info.isDir() )
	    okB->setText(tr("Open"));
	else
           okB->setText( mode() == AnyFile ? tr("Save") : tr("OK"));
    } else {
	okB->setEnabled( FALSE );
    }	

    if ( d->currentFileName.length() && old != d->currentFileName )
	emit fileHighlighted( d->currentFileName );

    return !d->currentFileName.isNull();
}


/*!  Make sure the minimum and maximum sizes of everything are sane.
*/

void QFileDialog::updateGeometry()
{
    if ( !d || !d->geometryDirty )
	return;

    d->geometryDirty = FALSE;

    QSize r, t;

    // we really should have a QSize::unite()
#define RM r.setWidth( QMAX(r.width(),t.width()) ); \
    r.setHeight( QMAX(r.height(),t.height()) )

    // labels first
    r = d->pathL->sizeHint();
    t = d->fileL->sizeHint();
    RM;
    t = d->typeL->sizeHint();
    RM;
    if ( d->extraLabel ) {
	t = d->extraLabel->sizeHint();
	RM;
    }
    d->pathL->setFixedSize( r );
    d->fileL->setFixedSize( r );
    d->typeL->setFixedSize( r );
    if ( d->extraLabel )
	d->extraLabel->setFixedSize( r );

    // single-line input areas
    r = d->paths->sizeHint();
    t = nameEdit->sizeHint();
    RM;
    t = d->types->sizeHint();
    RM;
    r.setWidth( t.width() * 2 / 3 );
    if ( d->extraWidget ) {
	t = d->extraWidget->sizeHint();
	RM;
    }
    t.setWidth( QCOORD_MAX );
    t.setHeight( r.height() );
    d->paths->setMinimumSize( r );
    d->paths->setMaximumSize( t );
    nameEdit->setMinimumSize( r );
    nameEdit->setMaximumSize( t );
    d->types->setMinimumSize( r );
    d->types->setMaximumSize( t );
    if ( d->extraWidget ) {
	d->extraWidget->setMinimumSize( r );
	d->extraWidget->setMaximumSize( t );
    }

    // buttons on top row
    r = QSize( 0, d->paths->minimumSize().height() );
    t = QSize( 21, 20 );
    RM;
    if ( r.height()+1 > r.width() )
	r.setWidth( r.height()+1 );
    d->cdToParent->setFixedSize( r );
    d->mcView->setFixedSize( r );
    d->detailView->setFixedSize( r );
    // ...

    // open/save, cancel
    r = QSize( 75, 20 );
    t = okB->sizeHint();
    RM;
    t = cancelB->sizeHint();
    RM;
    if ( d->extraButton ) {
	t = d->extraButton->sizeHint();
	RM;
    }
    okB->setFixedSize( r );
    cancelB->setFixedSize( r );
    if ( d->extraButton )
	d->extraButton->setFixedSize( r );

    d->topLevelLayout->activate();

#undef RM
}


/*!  Updates the dialog when the cursor moves in the listview.
*/

void QFileDialog::updateFileNameEdit( QListViewItem * newItem )
{
    if ( !newItem )
	return;

    if ( mode() == ExistingFiles ) {
	bool ok = files->isSelected( newItem );
	QListViewItem * i = files->firstChild();
	while( i && !ok ) {
	    ok = i->isSelected();
	    i = i->nextSibling();
	}
	okB->setText( tr( "OK" ) );
	okB->setEnabled( ok );
    } else if ( files->isSelected( newItem ) ) {
	QFileDialogPrivate::File * i = (QFileDialogPrivate::File *)newItem;
	trySetSelection( i->info, TRUE );
    }
}


/*!  Updates the dialog when the file name edit changes. */

void QFileDialog::fileNameEditDone()
{
    QFileInfo f ( cwd, nameEdit->text() );
    trySetSelection( f, FALSE );
}



/*!  This private slot reacts to double-clicks in the list view. */

void QFileDialog::selectDirectoryOrFile( QListViewItem * newItem )
{
    if ( !newItem )
	return;

    QFileDialogPrivate::File * i = (QFileDialogPrivate::File *)newItem;

    if ( i->info.isDir() ) {
	if ( mode() == ExistingFiles ) {
	    QListViewItem * i = files->firstChild();
	    while( i && !i->isSelected() )
		i = i->nextSibling();
	    if ( i ) {
		accept();
		return;
	    }
	}
	setDir( i->info.absFilePath() );
	if ( mode() == Directory ) {
	    QFileInfo f ( cwd, "." );
	    trySetSelection( f, TRUE );
	}
    } else if ( newItem->isSelectable() && trySetSelection( i->info, TRUE ) ) {
	if ( mode() != Directory ) {
	    emit fileSelected( d->currentFileName );
	    accept();
	}
    }
}


void QFileDialog::popupContextMenu( QListViewItem *, const QPoint & p,
				    int c )
{
    QPopupMenu m( 0, "file dialog context menu" );

    int asc = m.insertItem( tr("&Ascending") );
    int desc = m.insertItem( tr("&Descending") );

    m.move( p );
    int res = m.exec();
    if ( res == asc )
	files->setSorting( c, TRUE );
    else if ( res == desc )
	files->setSorting( c, FALSE );
}


void QFileDialog::fileSelected( int  )
{
    // unused
}

void QFileDialog::fileHighlighted( int )
{
    // unused
}

void QFileDialog::dirSelected( int )
{
    // unused
}

void QFileDialog::pathSelected( int )
{
    // unused
}


void QFileDialog::cdUpClicked()
{
    if ( cwd.cdUp() ) {
	cwd.convertToAbs();
	rereadDir();
    }
}


/*!  Ask the user for the name of an existing directory, starting at
  \a dir.  Returns the name of the directory the user selected.

  If \a dir is null, getExistingDirectory() starts wherever the
  previous file dialog left off.
*/

QString QFileDialog::getExistingDirectory( const char *dir,
					   QWidget *parent,
					   const char *name )
{
    makeVariables();
    QFileDialog *dialog	= new QFileDialog( parent, name, TRUE );
    dialog->setCaption( dialog->tr("Find Directory") );

    dialog->setMode( Directory );

    dialog->d->types->clear();
    dialog->d->types->insertItem( dialog->tr("Directories") );
    dialog->d->types->setEnabled( FALSE );

    if ( !workingDirectory->isEmpty() ) {
	QFileInfo f( *workingDirectory );
	if ( f.isDir() )
	    dialog->setDir( *workingDirectory );
    }	
    if ( dir && *dir ) {
	QFileInfo f( dir );
	if ( f.isDir() ) {
	    *workingDirectory = dir;
	    workingDirectory->detach();
	    dialog->setDir( dir );
	}
    }

    QString result;
    if ( dialog->exec() == QDialog::Accepted ) {
	result = dialog->selectedFile();
	QFileInfo f( result );
	if ( f.isDir() ) {
	    *workingDirectory = result;
	    workingDirectory->detach();
	} else {
	    result = 0;
	}
    }
    delete dialog;
    return result;
}


/*!  Sets this file dialog to \a newMode, which can be one of \c
  Directory (directories are accepted), \c ExistingFile (existing
  files are accepted) or \c AnyFile (any valid file name is accepted).

  \sa mode()
*/

void QFileDialog::setMode( Mode newMode )
{
    if ( d->mode != newMode ) {
	cwd.setFilter( QDir::All );
	d->mode = newMode;
	QString sel = d->currentFileName;
	if ( newMode == Directory ) {
	    files->setMultiSelection( FALSE );
	    if ( sel.isNull() )
		sel = ".";
	} else if ( newMode == ExistingFiles ) {
	    files->setMultiSelection( TRUE );
	} else {
	    files->setMultiSelection( FALSE );
	}
	rereadDir();
	QFileInfo f ( cwd, sel );
	trySetSelection( f, TRUE );
    }
}


/*!  Returns the file mode of this dialog.

  \sa setMode()
*/

QFileDialog::Mode QFileDialog::mode() const
{
    return d->mode;
}


/*!  Adds 1-3 widgets to the bottom of the file dialog.  \a l is the
  (optional) label, which is put beneath the "file name" and "file
  type" labels, \a w is a (optional) widget, which is put beneath the
  file type combo box, and \a b is the (you guessed it - optional)
  button, which is put beneath the cancel button.

  If you don't want to add something in one of the columns, pass 0.

  It is not currently possible to add more than one row.
*/

void QFileDialog::addWidgets( QLabel * l, QWidget * w, QPushButton * b )
{
    if ( !l && !w && !b )
	return;
    if ( d->extraLabel || d->extraWidget || d->extraButton )
	return;

    d->extraWidgetsLayout = new QHBoxLayout();
    d->topLevelLayout->addLayout( d->extraWidgetsLayout );

    d->extraLabel = l;
    if ( l )
	d->extraWidgetsLayout->addWidget( l );
    d->extraWidget = w;
    if ( w )
	d->extraWidgetsLayout->addWidget( w );
    d->extraButton = b;
    if ( b )
	d->extraWidgetsLayout->addWidget( b );

    d->topLevelLayout->activate();
    updateGeometry();
}


/*! \reimp */

void QFileDialog::keyPressEvent( QKeyEvent * ke )
{
    if ( ke && ( ke->key() == Key_Enter ||
		 ke->key() == Key_Return ) ) {
	ke->ignore();
	if ( d->paths->hasFocus() ) {
	    ke->accept();
	    if ( cwd.absPath() == d->paths->currentText() )
		nameEdit->setFocus();
	} else if ( d->types->hasFocus() ) {
	    ke->accept();
	    // ### is there a suitable condition for this?  only valid
	    // wildcards?
	    nameEdit->setFocus();
	} else if ( focusWidget() == nameEdit ) {
	    if ( d->currentFileName.isNull() ) {
		// maybe change directory
		QFileInfo i( cwd, nameEdit->text() );
		if ( i.isDir() ) {
		    nameEdit->setText( "" );
		    setDir( i.filePath() );
		}
		ke->accept();
	    } else if ( mode() == ExistingFiles ) {
		QFileInfo i( cwd, nameEdit->text() );
		if ( i.isFile() ) {
		    QListViewItem * i = files->firstChild();
		    while ( i && qstrcmp( nameEdit->text(), i->text( 0 ) ) )
			i = i->nextSibling();
		    if ( i )
			 files->setSelected( i, TRUE );
		    else
			ke->accept(); // strangely, means to ignore that event
		}
	    }
	} else if ( files->hasFocus() || d->moreFiles->hasFocus() ) {
	    ke->accept();
	}
    } else if ( ke->key() == Key_Escape ) {
	ke->ignore();
    }

    if ( !ke->isAccepted() ) {
	QDialog::keyPressEvent( ke );
    }
}


/*! \class QFileIconProvider qfiledialog.h

  \brief The QFileIconProvider class provides icons for QFileDialog to
  use.

  By default, QFileIconProvider is not used, but any application or
  library can subclass it, reimplement pixmap() to return a suitable
  icon, and make all QFileDialog objects use it by calling the static
  function QFileDialog::setIconProvider().

  It's advisable to make all the icons QFileIconProvider returns be of
  the same size, or at least the same width.  This makes the list view
  look much better.

  \sa QFileDialog
*/


/*!  Constructs an empty file icon provider. */

QFileIconProvider::QFileIconProvider( QObject * parent, const char * name )
    : QObject( parent, name )
{
    // nothing necessary
}


/*!  Returns a pointer to a pixmap suitable for display when the file
  dialog next to the name of \a file.

  If pixmap() returns 0, QFileDialog draws nothing.

  The default implementation returns 0 in Qt 1.40.  In future versions
  of Qt it may be extended.
*/

const QPixmap * QFileIconProvider::pixmap( const QFileInfo & )
{
    return 0;
}


/*!  Sets all file dialogs to use \a provider to select icons to draw
  for each file.  By default there is no icon provider, and
  QFileDialog simply draws a "folder" icon next to each directory and
  nothing next to the files.

  \sa QFileIconProvider iconProvider()
*/

void QFileDialog::setIconProvider( QFileIconProvider * provider )
{
    fileIconProvider = provider;
}


/*!  Returns the icon provider currently in use.  By default there is
  no icon provider and this function returns 0.

  \sa setIconProvider() QFileIconProvider
*/

QFileIconProvider * QFileDialog::iconProvider()
{
    return fileIconProvider;
}


/*! \reimp */

bool QFileDialog::eventFilter( QObject * o, QEvent * e )
{
    if ( !o || !e )
	return TRUE;
    if ( mode() == ExistingFiles &&
	 e->type() == Event_MouseButtonDblClick &&
	 ( o == files || o == d->moreFiles || o == files->viewport() ) ) {
	QListViewItem * i = files->firstChild();
	while( i && !i->isSelected() )
	    i = i->nextSibling();
	if ( i )
	    return TRUE;
    } else if ( e->type() == Event_KeyPress &&
		((QKeyEvent *)e)->key() == Key_Backspace &&
		( o == files ||
		  o == d->moreFiles ||
		  o == files->viewport() ) ) {
	cdUpClicked();
	((QKeyEvent *)e)->accept();
	return TRUE;
    } else if ( o == files && e->type() == Event_FocusOut &&
		files->currentItem() && mode() != ExistingFiles ) {
	files->setSelected( files->currentItem(), FALSE );
    } else if ( o == files && e->type() == Event_KeyPress ) {
	QTimer::singleShot( 0, this, SLOT(fixupNameEdit()) );
    } else if ( o == nameEdit && e->type() == Event_KeyPress ) {
	// ### hack.  after 1.40, we need to merge the completion code
	// ### here, in QListView and QComboBox.
	if ( isprint(((QKeyEvent *)e)->ascii()) ) {
	    QString nt( nameEdit->text() );;
	    nt.detach();
	    nt.truncate( nameEdit->cursorPosition() );
	    nt += (char)(((QKeyEvent *)e)->ascii());
	    QListViewItem * i = files->firstChild();
	    while( i && qstrncmp( i->text( 0 ), nt, nt.length() ) )
		i = i->nextSibling();
	    if ( i ) {
		nt = i->text( 0 );
		int cp = nameEdit->cursorPosition()+1;
		nameEdit->validateAndSet( nt, cp, cp, nt.length() );
		return TRUE;
	    }
	}
    } else if ( o == nameEdit && e->type() == Event_FocusIn ) {
	fileNameEditDone();
    }
    return FALSE;
}


/*!  Sets this file dialog to offer \a types in the File Type combo
  box.  \a types must be a null-terminated list of strings; each
  string must be in the format described in the documentation for
  setFilter().

  \sa setFilter()
*/

void QFileDialog::setFilters( const char ** types )
{
    if ( !types || !*types )
	return;

    d->types->clear();
    while( types && *types ) {
	d->types->insertItem( *types );
	types++;
    }
    d->types->setCurrentItem( 0 );
    setFilter( d->types->text( 0 ) );
}


/*! \overload void QFileDialog::setFilters( const QStrList & )
*/

void QFileDialog::setFilters( const QStrList & types )
{
    if ( types.count() < 1 )
	return;

    d->types->clear();
    QStrListIterator it( types );
    it.toFirst();
    const char * t;
    while( (t=it.current()) != 0 ) {
	++it;
	d->types->insertItem( t );
    }
    d->types->setCurrentItem( 0 );
    setFilter( d->types->text( 0 ) );
}


/*!
  Since modeButtons is a top-level widget, it may be destroyed by the
  kernel at application exit time. Notice if this happens to
  avoid double deletion.
*/

void QFileDialog::modeButtonsDestroyed()
{
    if ( d )
	d->modeButtons = 0;
}


/*!  Lets the user select N files from a single directory, and returns
  a list of the selected files.  The list may be empty, and the file
  names are fully qualified (i.e. "/usr/games/quake" or
  "c:\\quake\\quake").

  \a filter is the default glob pattern (which the user can change).
  The default is all files.  \a dir is the starting directory.  If \a
  dir is not supplied, QFileDialog picks something presumably useful
  (such as the directory where the user selected something last, or
  the current working directory).

  \a parent is a widget over which the dialog should be positioned and
  \a name is the object name of the temporary QFileDialog object.

  Note that the returned list has auto-delete turned off.  It is the
  application developer's responsibility to delete the strings in the
  list, for example using code such as:

  \code
    QStrList s( QFileDialog::getOpenFileNames() );
    // do something with the files in s.
    s.setAutoDelete();
    s.clear(); // or just go out of scope.
  \endcode
*/

QStrList QFileDialog::getOpenFileNames( const char *filter,
					const char * dir,
					QWidget *parent,
					const char *name )
{
    makeVariables();

    if ( workingDirectory->isNull() )
	*workingDirectory = QDir::currentDirPath();

    QFileInfo tmp( QDir::root(), dir );
    if ( !tmp.isDir() ) {
	tmp.setFile( QDir::root(), *workingDirectory );
	while( !tmp.isDir() )
	    tmp.setFile( tmp.dirPath( TRUE ) );
    }

    *workingDirectory = tmp.absFilePath();
    QFileDialog *dlg = new QFileDialog( *workingDirectory, filter,
					parent, name, TRUE );
    CHECK_PTR( dlg );
    dlg->setCaption( dlg->tr("Open") );
    dlg->setMode( QFileDialog::ExistingFiles );
    QString result;
    QStrList s;
    s.setAutoDelete( FALSE );
    if ( dlg->exec() == QDialog::Accepted ) {
	QListViewItem * i = dlg->files->firstChild();
	while( i ) {
	    if ( i->isSelected() )
		s.append( ((QFileDialogPrivate::File*)i)->info.absFilePath() );
	    i = i->nextSibling();
	}
	*workingDirectory = dlg->dirPath();
    }
    delete dlg;
    return s;
}



/*!  Updates the line edit to match the speed-key usage in QListView. */

void QFileDialog::fixupNameEdit()
{
    if ( files->currentItem() )
	nameEdit->setText( files->currentItem()->text( 0 ) );
}
