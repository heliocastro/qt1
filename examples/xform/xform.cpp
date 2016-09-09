/****************************************************************************
** $Id: xform.cpp,v 2.9.2.1 1998/10/05 13:28:02 warwick Exp $
**
** Copyright (C) 1992-1998 Troll Tech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#include <qapplication.h>

#include <qdialog.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qcheckbox.h>
#include <qradiobutton.h>
#include <qbuttongroup.h>
#include <qlcdnumber.h>
#include <qslider.h>
#include <qmenubar.h>

#include <qpainter.h>
#include <qpixmap.h>

#include <stdlib.h>


class FontSelect;


class XFormControl : public QFrame
{
    Q_OBJECT
public:
    XFormControl( QWidget *parent=0, const char *name=0 );
   ~XFormControl() {}
signals:
    void newMatrix( QWMatrix );
    void newText( const char * );
    void newFont( const QFont & );
    void newMode( bool image );
private slots:
    void newMtx();
    void selectFont();
    void selectFontDestroyed();
    void fontSelected( const QFont & );
    void toggleMode();
private:
    QSlider	*rotS;		       // Rotation angle scroll bar
    QSlider	*shearS;	       // Shear value scroll bar
    QSlider	*magS;		       // Magnification value scroll bar
    QLCDNumber	*rotLCD;	       // Rotation angle LCD display
    QLCDNumber	*shearLCD;	       // Shear value LCD display
    QLCDNumber	*magLCD;	       // Magnification value LCD display
    QCheckBox	*mirror;	       // Checkbox for mirror image on/of
    QLineEdit	*textEd;	       // Inp[ut field for xForm text
    QPushButton *f;		       // Select font push button
    QCheckBox	*imgCheckBox;	       // Check box for image on/off
    FontSelect	*fs;		       // font select dialog box
    bool	 fsUp;		       // TRUE if font select is visible
};


/*
  ShowXForm displays a text or a pixmap (QPixmap) using a coordinate
  transformation matrix (QWMatrix)
*/

class ShowXForm : public QWidget
{
    Q_OBJECT
public:
    ShowXForm( QWidget *parent=0, const char *name=0 );
   ~ShowXForm() {}
    void showIt();			// (Re)displays text or pixmap
    bool pixmapMode() const { return usingPixmap; }
public slots:
    void setText( const char* );
    void setMatrix( QWMatrix );
    void setFont( const QFont &f );
    void setPixmap( QPixmap );
    void setPixmapMode( bool b );
private:
    void paintEvent( QPaintEvent * );
    void resizeEvent( QResizeEvent * );
    QString   text;			// text to be displayed
    QWMatrix  mtx;			// coordinate transform matrix
    QPixmap   pix;			// pixmap to be displayed
    bool      usingPixmap;		// TRUE if pix is being displayed
    QRect     eraseRect;		// covers last displayed text/pixmap
};

class FontSelect : public QDialog
{
    Q_OBJECT
public:
    FontSelect( QWidget *parent=0, const char *name=0 );
private slots:
    void	 newStrikeOut();
    void	 doFont();
    void	 newUnderline();
    void	 newItalic();
    void	 newWeight( int id );
    void	 newFamily();
    void	 newSize( int );
    void	 slidingSize( int );
    void	 doApply();
    void	 familyActivated( int );
    void	 pSizeActivated( int );
signals:
    void	 newFont( const QFont & );
protected:
    void	 updateMetrics();
private:
    QLabel	 *familyL;
    QLabel	 *sizeL;
    QLineEdit	 *family;
    QLineEdit	 *sizeE;
    QCheckBox	 *italic;
    QCheckBox	 *underline;
    QCheckBox	 *strikeOut;
    QPushButton	 *apply;
    QPushButton	 *stat;
    QSlider	 *sizeS;
    QRadioButton *rb[5];
    QButtonGroup *weight;
    QGroupBox	 *mGroup;
    QFont	  f;
    QMenuBar	 *menu;
    QLabel	 *metrics[4][2];
    QWidget	 *fontInternal;
};


XFormControl::XFormControl( QWidget *parent, const char *name )
	: QFrame( parent, name )
{
    rotLCD	= new QLCDNumber( 4, this, "rotateLCD" );
    rotS	= new QSlider( QSlider::Horizontal, this,
				  "rotateSlider" );
    shearLCD	= new QLCDNumber( 5,this, "shearLCD" );
    shearS	= new QSlider( QSlider::Horizontal, this,
				  "shearSlider" );
    mirror	= new QCheckBox( this, "mirrorCheckBox" );
    textEd	= new QLineEdit( this, "text" );
    f		= new QPushButton( this, "text" );
    imgCheckBox = new QCheckBox( this, "fontOrImage" );

    rotLCD->setGeometry( 10, 10, 100, 60 );
    rotLCD->display( "  0'" );

    rotS->setGeometry( 10, 80, 100, 15 );
    rotS->setRange( -180, 180 );
    rotS->setValue( 0 );
    connect( rotS, SIGNAL(valueChanged(int)), SLOT(newMtx()) );

    shearLCD->setGeometry( 10, 105, 100, 60 );
    shearLCD->display( "0.00" );

    shearS->setGeometry( 10, 175, 100, 15 );
    shearS->setRange( -25, 25 );
    shearS->setValue( 0 );
    connect( shearS, SIGNAL(valueChanged(int)), SLOT(newMtx()) );

    mirror->setGeometry( 10, 200, 100, 15 );
    mirror->setText( "Mirror" );
    connect( mirror, SIGNAL(clicked()), SLOT(newMtx()) );

    imgCheckBox->setGeometry( 10, 220, 100, 15 );
    imgCheckBox->setText( "Image" );
    connect( imgCheckBox, SIGNAL(clicked()), SLOT(toggleMode()) );

    textEd->setGeometry( 10, 245, 100, 20 );
    textEd->setText( "Troll" );
    connect( textEd, SIGNAL(textChanged(const char*)),
		     SIGNAL(newText(const char*)) );

    f->setGeometry( 10, 275, 100, 20 );
    f->setText( "Select font..." );
    connect( f, SIGNAL(clicked()), SLOT(selectFont()) );

    magS    = 0;
    fs	    = 0;
}

/*
    Called whenever the user has changed one of the matrix parameters
    (i.e. rotate, shear or magnification)
*/
void XFormControl::newMtx()
{
    QWMatrix m;
    if ( imgCheckBox->isChecked() ) {
	double magVal = 1.0*magS->value()/100;
	m.scale( magVal, magVal );
    }
    double shearVal = 1.0*shearS->value()/25;
    m.shear( shearVal, shearVal );
    m.rotate( rotS->value() );
    if ( mirror->isChecked() ) {
	m.scale( 1, -1 );
	m.rotate( 180 );
    }

    QString tmp;
    tmp.sprintf( "%1.2f", shearVal  );
    if ( shearVal >= 0 )
	tmp.insert( 0, " " );
    shearLCD->display( tmp );

    int rot = rotS->value();
    if ( rot < 0 )
	rot = rot + 360;
    tmp.sprintf( "%3i'", rot );
    rotLCD->display( tmp );
    emit newMatrix( m );
}


void XFormControl::selectFont()
{
    if (!fs) {
	fs   = new FontSelect;
	connect( fs, SIGNAL(destroyed()),    SLOT(selectFontDestroyed()) );
	connect( fs, SIGNAL(newFont(const QFont&)),
		     SLOT(fontSelected(const QFont&)) );
	fs->setGeometry( QRect( 100, 200, 380, 260 ) );
	fsUp = FALSE;
    }
    fsUp = !fsUp;
    if ( fsUp )
	fs->show();
    else
	fs->hide();
}

/*
    Called when the user has closed the SelectFont dialog via the
    window manager.
*/
void XFormControl::selectFontDestroyed()
{
    fs = 0;
}

void XFormControl::fontSelected( const QFont &font )
{
    imgCheckBox->setChecked( FALSE );
    emit newFont( font );
    toggleMode();
}

/*
    Toggles between image and text mode.
*/

void XFormControl::toggleMode()
{
    if ( magS == 0 ) {
	magS = new QSlider( QSlider::Horizontal, this,
			       "magnifySlider" );
	magS->setGeometry( 10, 375, 100, 15 );
	magS->setRange( 0, 400 );
	magS->setValue( 100 );
	connect( magS, SIGNAL(valueChanged(int)), SLOT(newMtx()) );
	magLCD = new QLCDNumber( 4,this, "shearLCD" );
	magLCD->setGeometry( 10, 305, 100, 60 );
	magLCD->display( "100" );
	connect( magS, SIGNAL(valueChanged(int)), magLCD, SLOT(display(int)));
    }
    emit newMode( imgCheckBox->isChecked() );
    newMtx();
    if ( imgCheckBox->isChecked() ) {
	magS->show();
	magLCD->show();
    } else {
	magS->hide();
	magLCD->hide();
    }
    qApp->flushX();
}

const int yOff = 35;

ShowXForm::ShowXForm( QWidget *parent, const char *name )
	: QWidget( parent, name )
{
    setFont( QFont( "Charter", 48, QFont::Bold ) );
    setBackgroundColor( white );
    usingPixmap = FALSE;
    eraseRect = QRect( 0, 0, 0, 0 );
}

void ShowXForm::paintEvent( QPaintEvent * )
{
    showIt();
}

void ShowXForm::resizeEvent( QResizeEvent * )
{
    eraseRect = QRect( width()/2, height()/2, 0, 0 );
}

void ShowXForm::setText( const char *s )
{
    text = s;
    showIt();
}

void ShowXForm::setMatrix( QWMatrix w )
{
    mtx = w;
    showIt();
}

void ShowXForm::setFont( const QFont &f )
{
    usingPixmap = FALSE;
    QWidget::setFont( f );
}

void ShowXForm::setPixmap( QPixmap pm )
{
    pix		= pm;
    usingPixmap = TRUE;
    showIt();
}

void ShowXForm::setPixmapMode( bool enable )
{
    usingPixmap = enable;
}

void ShowXForm::showIt()
{
    QPainter p;
    QRect r;	  // rectangle covering new text/pixmap in virtual coordinates
    QWMatrix m;	  // copy user specified transform
    int textYPos = 0; // distance from boundingRect y pos to baseline
    int textXPos = 0; // distance from boundingRect x pos to text start
    QRect br;
    QFontMetrics fm( fontMetrics() );	// get widget font metrics
    if ( pixmapMode() ) {
	r = pix.rect();
    } else {
	br = fm.boundingRect( text );	// rectangle covering text
	r  = br;
	textYPos = -r.y();
	textXPos = -r.x();
	br.moveTopLeft( QPoint( -br.width()/2, -br.height()/2 ) );
    }
    r.moveTopLeft( QPoint(-r.width()/2, -r.height()/2) );
	  // compute union of new and old rect
	  // the resulting rectangle will cover what is already displayed
	  // and have room for the new text/pixmap
    eraseRect = eraseRect.unite( mtx.map(r) );
    eraseRect.moveBy( -1, -1 ); // add border for matrix round off
    eraseRect.setSize( QSize( eraseRect.width() + 2,eraseRect.height() + 2 ) );
    int pw = QMIN(eraseRect.width(),width());
    int ph = QMIN(eraseRect.height(),height());
    QPixmap pm( pw, ph );		// off-screen drawing pixmap
    pm.fill( backgroundColor() );

    p.begin( &pm );
    m.translate( pw/2, ph/2 );	// 0,0 is center
    m = mtx * m;
    p.setWorldMatrix( m );
    if ( pixmapMode() ) {
	p.drawPixmap( -pix.width()/2, -pix.height()/2, pix );
    } else {
	p.setFont( font() );		// use widget font
	p.drawText( r.left() + textXPos, r.top() + textYPos, text );
    }
#if 0
    p.setPen( red );
    p.drawRect( br );
#endif
    p.end();

    int xpos = width()/2  - pw/2;
    int ypos = height()/2 - ph/2;
    bitBlt( this, xpos, ypos,			// copy pixmap to widget
	    &pm, 0, 0, -1, -1 );
    eraseRect =	 mtx.map( r );
}


FontSelect::FontSelect( QWidget *parent, const char *name)
    : QDialog( parent, name, 0 ), f( "Charter", 48, QFont::Bold )
{
    static const char *radios[] = {
	"Light (25)", "Normal (50)", "DemiBold (63)",
	"Bold (75)", "Black (87)"
    };
    int i;

    fontInternal = new QWidget( this );
    fontInternal->setFont( f );
    fontInternal->hide();

    familyL    = new QLabel(	 this, "familyLabel" );
    sizeL      = new QLabel(	 this, "sizeLabel" );
    family     = new QLineEdit(	 this, "family" );
    sizeE      = new QLineEdit(	 this, "pointSize" );
    italic     = new QCheckBox(	 this, "italic" );
    underline  = new QCheckBox(	 this, "underline" );
    strikeOut  = new QCheckBox(	 this, "strikeOut" );
    apply      = new QPushButton( this, "apply" );
    sizeS      = new QSlider( QSlider::Horizontal, this,
			      "pointSizeSlider" );

    familyL->setGeometry( 10, yOff + 10, 100,20 );
    familyL->setText( "Family :" );

    sizeL->setGeometry( 10, yOff + 40, 100, 20 );
    sizeL->setText( "Point size :" );

    family->setGeometry( 110, yOff + 10, 100, 20 );
    family->setText( "Charter" );

    sizeE->setGeometry( 110, yOff + 40, 100, 20 );
    sizeE->setText( "48" );

    sizeS->setGeometry( 220, yOff + 40, 100, 20 );
    sizeS->setRange( 1, 100 );
    sizeS->setValue( 48 );
    sizeS->setTracking( FALSE );
    connect( sizeS, SIGNAL(valueChanged(int)), SLOT(newSize(int)) );
    connect( sizeS, SIGNAL(sliderMoved(int)),  SLOT(slidingSize(int)) );

    italic->setGeometry( 10, yOff + 70, 80, 20 );
    italic->setText( "Italic" );
    connect( italic, SIGNAL(clicked()), SLOT(newItalic()) );

    underline->setGeometry( 110, yOff + 70, 80, 20 );
    underline->setText( "Underline" );
    connect( underline, SIGNAL(clicked()), SLOT(newUnderline()) );

    strikeOut->setGeometry( 210, yOff + 70, 80, 20 );
    strikeOut->setText( "StrikeOut" );
    connect( strikeOut, SIGNAL(clicked()), SLOT(newStrikeOut()) );

    apply->setGeometry( 235, yOff + 10, 70, 20);
    apply->setText( "APPLY" );
    apply->setDefault( TRUE );
    connect( apply, SIGNAL(clicked()), SLOT(doApply()) );

    weight = new QButtonGroup( "Weight", this, "weightGroupBox" );
    weight->setGeometry( 10, yOff + 100, 120, 120 );
    connect( weight, SIGNAL(clicked(int)), SLOT(newWeight(int)) );
    QString wname;
    for( i = 0 ; i < 5 ; i++ ) {
	wname.sprintf("radioButton %i",i);
	rb[i] = new QRadioButton( weight, wname );
	rb[i]->setGeometry( 10, 15+i*20 , 95, 20 );
	rb[i]->setText( radios[i] );
    }
    rb[3]->setChecked( TRUE );

#ifdef _OS_WIN32_
    static const char *families[] = {
	"Arial", "Book Antiqua", "Bookman", "Century Schoolbook",
	"Comic Sans MS", "Courier New", "Garamond",
	"Haettenschweiler", "Impact", "Symbol", "Times New Roman",
	"Verdana", "WingDings", 0
    };
#else
    static const char *families[] = {
	"Charter", "Clean", "Courier", "Fixed", "Gothic", "Helvetica",
	"Lucida", "Lucidabright", "Lucidatypewriter", "Mincho", 
	"New century schoolbook", "Symbol", "Terminal", "Times", "Utopia",
	0
    };
#endif // _OS_WIN32_

    static const char *pSizes[]	 = {
	"8", "10", "12", "14", "18", "24", "36", "48", "72", "96", 0
    };

    QPopupMenu *familyPopup = new QPopupMenu;
    const char **tmp;
    tmp = families;
    while( *tmp )
	familyPopup->insertItem( *tmp++ );

    QPopupMenu *pSize = new QPopupMenu;
    tmp = pSizes;
    while( *tmp )
	pSize->insertItem( *tmp++ );

    menu = new QMenuBar( this );
    menu->move( 0, 0 );
    menu->resize( 350, 30 );
    menu->insertItem( "Family", familyPopup );
    menu->insertItem( "Point size", pSize );

    connect( familyPopup, SIGNAL(activated(int)), SLOT(familyActivated(int)) );
    connect( pSize, SIGNAL(activated(int)), SLOT(pSizeActivated(int)) );

    static const char *mLabelStr[] = {
	"Family:", "Point size:", "Weight:", "Italic:"
    };

    mGroup = new QButtonGroup( this, "metricsGroupBox" );
    mGroup->setTitle( "Actual font" );
    mGroup->setGeometry(140, yOff + 100, 230, 100);
    for( i = 0 ; i < 4 ; i++ ) {
	wname.sprintf("MetricsLabel[%i][%i]",i,0);
	metrics[i][0] = new QLabel( mGroup, wname);
	metrics[i][0]->setGeometry(10, 15 + 20*i, 70, 20);
	metrics[i][0]->setText( mLabelStr[i] );

	wname.sprintf("MetricsLabel[%i][%i]",i,1);
	metrics[i][1] = new QLabel( mGroup, wname);
	metrics[i][1]->setGeometry(90, 15 + 20*i, 135, 20);
    }
    updateMetrics();
}

void FontSelect::newStrikeOut()
{
    f.setStrikeOut( strikeOut->isChecked() );
    doFont();
}

void FontSelect::doFont()
{
    QFont xyz = f;
    xyz.setPointSize( f.pointSize()+1 );
    xyz.setPointSize( f.pointSize() );
    fontInternal->setFont( xyz );
    updateMetrics();
}

void FontSelect::newUnderline()
{
    f.setUnderline( underline->isChecked() );
    doFont();
}

void FontSelect::newItalic()
{
    f.setItalic( italic->isChecked() );
    doFont();
}

void FontSelect::newFamily()
{
    f.setFamily( family->text() );
    doFont();
}

void FontSelect::newWeight( int id )
{
    switch( id ) {
	case 0 :
	    f.setWeight( QFont::Light );
	    break;
	case 1 :
	    f.setWeight( QFont::Normal );
	    break;
	case 2 :
	    f.setWeight( QFont::DemiBold );
	    break;
	case 3 :
	    f.setWeight( QFont::Bold );
	    break;
	case 4 :
	    f.setWeight( QFont::Black );
	    break;
	default:
	    return;
    }
    doFont();
}

void FontSelect::newSize( int value )
{
    QString tmp;
    tmp.sprintf("%i", value);
    sizeE->setText( tmp );
    f.setPointSize( value );
    doFont();
}

void FontSelect::slidingSize( int value )
{
    QString tmp;

    tmp.sprintf("%i", value);
    sizeE->setText( tmp );
}

void FontSelect::doApply()
{
    int sz = atoi( sizeE->text() );
    if ( sz > 100) {
	sizeS->blockSignals( TRUE );
	sizeS->setValue( 100 );
	sizeS->blockSignals( FALSE );
	f.setPointSize( sz );
    } else {
	sizeS->setValue( atoi( sizeE->text() ) );
    }
    f.setFamily( family->text() );
    doFont();

    emit newFont( fontInternal->font() );
}

void FontSelect::familyActivated( int id )
{
    family->setText( ((QPopupMenu*)sender())->text(id) );
    newFamily();
}

void FontSelect::pSizeActivated( int id )
{
    int value = atoi( ( (QPopupMenu*)sender())->text( id ) );
    sizeS->blockSignals( TRUE );
    sizeS->setValue( value );
    sizeS->blockSignals( FALSE );
    newSize( value );
}

void FontSelect::updateMetrics()
{
    QFontInfo fi = fontInternal->fontInfo();
    metrics[0][1]->setText( fi.family() );
    metrics[1][1]->setNum( fi.pointSize() );
    metrics[2][1]->setNum( fi.weight() );
    metrics[3][1]->setNum( (int)fi.italic() );
}

/*
    Grand unifying widget, putting ShowXForm and XFormControl
    together.
*/

class XFormCenter : public QWidget
{
    Q_OBJECT
public:
    XFormCenter( QWidget *parent=0, const char *name=0 );
public slots:
    void setFont( const QFont &f ) { sx->setFont( f ); }
    void newMode( bool );
private:
    void resizeEvent( QResizeEvent* );
    ShowXForm	*sx;
    XFormControl *xc;
};

void XFormCenter::resizeEvent( QResizeEvent* )
{
    sx->resize( width() - 120, height() );
    xc->resize( 120, height() );
}

void XFormCenter::newMode( bool showPix )
{
    static bool first = TRUE;

    if ( sx->pixmapMode() == showPix )
	return;
    if ( showPix && first ) {
	first = FALSE;
	QPixmap pm;
	pm.load( "image.any" );
	sx->setPixmap( pm );
	return;
    }
    sx->setPixmapMode( showPix );
}

XFormCenter::XFormCenter( QWidget *parent, const char *name )
    : QWidget( parent, name )
{
    sx = new ShowXForm(this);
    sx->move( 120, 0 );		    // the size is set by resizeEvent
    xc = new XFormControl(this);
    xc->move( 0, 0 );		    // the size is set by resizeEvent
    xc->setFrameStyle( QFrame::Box | QFrame::Sunken );
    xc->setLineWidth( 2 );
    connect( xc, SIGNAL(newText(const char*)), sx,
		 SLOT(setText(const char*)) );
    connect( xc, SIGNAL(newMatrix(QWMatrix)),
	     sx, SLOT(setMatrix(QWMatrix)) );
    connect( xc, SIGNAL(newFont(const QFont&)), sx,
		 SLOT(setFont(const QFont&)) );
    connect( xc, SIGNAL(newMode(bool)), SLOT(newMode(bool)) );
    sx->setText( "Troll" );
}


int main( int argc, char **argv )
{
    QApplication a( argc, argv );

#if 0
    QColor x( 0xaa, 0xbe, 0xff );	// kind of blue
    QColorGroup g( black, x, x.light(), x.dark(), x.dark(120),	black, white );
    QPalette p( g, g, g );
    a.setPalette( p );
#endif

    XFormCenter *xfc = new XFormCenter;
    xfc->setGeometry( 0, 0, 500, 400 );

    a.setMainWidget( xfc );
    xfc->show();
    return a.exec();
}

#include "xform.moc"		      // include metadata generated by the moc
