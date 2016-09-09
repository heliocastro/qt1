/****************************************************************************
** $Id: widgets.cpp,v 2.46 1998/07/08 13:22:45 aavit Exp $
**
** Copyright (C) 1992-1998 Troll Tech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#include <qdialog.h>
#include <qmessagebox.h>
#include <qpixmap.h>
#include <qmovie.h>
#include <qlayout.h>
#include <qapplication.h>
#include <qkeycode.h>

// Standard Qt widgets

#include <qmenubar.h>
#include <qpopupmenu.h>
#include <qbuttongroup.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qframe.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qlcdnumber.h>
#include <qmultilinedit.h>
#include <qlineedit.h>
#include <qlistbox.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qslider.h>
#include <qtooltip.h>
#include <qspinbox.h>

// Some sample widgets

#include "../aclock/aclock.h"
#include "../dclock/dclock.h"


#define MOVIEFILENAME "trolltech.gif"

//
// WidgetView contains lots of Qt widgets.
//

class WidgetView : public QWidget
{
    Q_OBJECT
public:
    WidgetView( QWidget *parent=0, const char *name=0 );

public slots:
    void	setStatus(const char*);

private slots:
    void	button1Clicked();
    void	checkBoxClicked( int );
    void	radioButtonClicked( int );
    void	sliderValueChanged( int );
    void	listBoxItemSelected( int );
    void	comboBoxItemActivated( int );
    void	edComboBoxItemActivated( const char * );
    void	lineEditTextChanged( const char * );
    void	movieStatus( int );
    void	movieUpdate( const QRect& );
    void	spinBoxValueChanged( const char * );

private:
    bool	eventFilter( QObject *, QEvent * );
    QLabel     *msg;
    QCheckBox  *cb[3];
    QLabel     *movielabel;
    QMovie      movie;
};


//
// Construct the WidgetView with buttons
//

WidgetView::WidgetView( QWidget *parent, const char *name )
    : QWidget( parent, name )
{
    QColor col;

  // Set the window caption/title

    setCaption( "Qt Widgets Demo Application" );

  // Install an application-global event filter

    qApp->installEventFilter( this );

  // Create a layout to position the widgets

    QVBoxLayout *topLayout = new QVBoxLayout( this, 10 );

  // Create a grid layout to hold most of the widgets

    QGridLayout *grid = new QGridLayout( 6, 3 );
 // This layout will get all of the stretch
    topLayout->addLayout( grid, 10 );

  // Create a menubar
    QMenuBar *menubar = new QMenuBar( this );
    menubar->setSeparator( QMenuBar::InWindowsStyle );
  // Create an easter egg
    QToolTip::add( menubar, QRect( 0, 0, 2, 2 ), "easter egg" );

    QPopupMenu* popup;
    popup = new QPopupMenu;
    int id;
    id = popup->insertItem( "&New" );
    popup->setItemEnabled( id, FALSE );
    id = popup->insertItem( "&Open" );
    popup->setItemEnabled( id, FALSE );
    popup->insertSeparator();
    popup->insertItem( "&Quit", qApp, SLOT(quit()), CTRL+Key_Q );

    menubar->insertItem( "&File", popup );

    // Must tell the layout about a menubar in a widget
    topLayout->setMenuBar( menubar );

  // Create an analog and a digital clock

    AnalogClock  *aclock = new AnalogClock( this );
    DigitalClock *dclock = new DigitalClock( this );
    dclock->setMaximumWidth(200);
    grid->addWidget( aclock, 0, 2 );
    grid->addWidget( dclock, 1, 2 );

  // Give the dclock widget a blue palette

    col.setRgb( 0xaa, 0xbe, 0xff );
    dclock->setPalette( QPalette( col ) );

  // make tool tips for both of them

    QToolTip::add( aclock, "custom widget: analog clock" );
    QToolTip::add( dclock, "custom widget: digital clock" );

  // Create a push button.

    QPushButton *pb;
    pb = new QPushButton( this, "button1" );	// create button 1
    pb->setText( "Push button 1" );
    pb->setFixedHeight( pb->sizeHint().height() );
    grid->addWidget( pb, 0, 0, AlignVCenter );
    connect( pb, SIGNAL(clicked()), SLOT(button1Clicked()) );
    QToolTip::add( pb, "push button 1" );
    QPixmap pm;
    bool pix = pm.load("qt.bmp");		// load pixmap for button 2
    if ( !pix ) {
	QMessageBox::information( 0, "Qt Widgets Example",
				  "Could not load the file \"qt.bmp\", which\n"
				  "contains an icon used...\n\n"
				  "The text \"line 42\" will be substituted.",
				  QMessageBox::Ok + QMessageBox::Default );
    }

    // Create a label containing a QMovie

    movielabel = new QLabel( this, "label0" );
    movie = QMovie( MOVIEFILENAME );
    movie.connectStatus(this, SLOT(movieStatus(int)));
    movie.connectUpdate(this, SLOT(movieUpdate(const QRect&)));
    movielabel->setFrameStyle( QFrame::Box | QFrame::Plain );
    movielabel->setMovie( movie );
    movielabel->setMargin( 0 );
    movielabel->setFixedSize( 128+movielabel->frameWidth()*2,
			      64+movielabel->frameWidth()*2 );
    grid->addWidget( movielabel, 0, 1, AlignCenter );
    QToolTip::add( movielabel, "movie" );

  // Create a group of check boxes

    QButtonGroup *bg = new QButtonGroup( this, "checkGroup" );
    bg->setTitle( "Check Boxes" );
    grid->addWidget( bg, 1, 0 );

  // Create a layout for the check boxes
    QVBoxLayout *vbox = new QVBoxLayout(bg, 10);

    vbox->addSpacing( bg->fontMetrics().height() );

    cb[0] = new QCheckBox( bg );
    cb[0]->setText( "Read" );
    vbox->addWidget( cb[0] );
    cb[0]->setMinimumSize( cb[0]->sizeHint() );
    cb[1] = new QCheckBox( bg );
    cb[1]->setText( "Write" );
    vbox->addWidget( cb[1] );
    cb[1]->setMinimumSize( cb[1]->sizeHint() );
    cb[2] = new QCheckBox( bg );
    cb[2]->setText( "Execute" );
    cb[2]->setMinimumSize( cb[2]->sizeHint() );
    vbox->addWidget( cb[2] );
    bg->setMinimumSize( bg->childrenRect().size() );
    vbox->activate();

    connect( bg, SIGNAL(clicked(int)), SLOT(checkBoxClicked(int)) );

    QToolTip::add( cb[0], "check box 1" );
    QToolTip::add( cb[1], "check box 2" );
    QToolTip::add( cb[2], "check box 3" );

  // Create a group of radio buttons

    QRadioButton *rb;
    bg = new QButtonGroup( this, "radioGroup" );
    bg->setTitle( "Radio buttons" );

    grid->addWidget( bg, 1, 1 );

  // Create a layout for the radio buttons
    vbox = new QVBoxLayout(bg, 10);

    vbox->addSpacing( bg->fontMetrics().height() );
    rb = new QRadioButton( bg );
    rb->setText( "&AM" );
    rb->setChecked( TRUE );
    vbox->addWidget(rb);
    rb->setMinimumSize( rb->sizeHint() );
    QToolTip::add( rb, "radio button 1" );
    rb = new QRadioButton( bg );
    rb->setText( "&FM" );
    vbox->addWidget(rb);
    rb->setMinimumSize( rb->sizeHint() );
    QToolTip::add( rb, "radio button 2" );
    rb = new QRadioButton( bg );
    rb->setText( "&Short Wave" );
    vbox->addWidget(rb);
    rb->setMinimumSize( rb->sizeHint() );
    vbox->activate();

    connect( bg, SIGNAL(clicked(int)), SLOT(radioButtonClicked(int)) );
    QToolTip::add( rb, "radio button 3" );

  // Create a list box

    QListBox *lb = new QListBox( this, "listBox" );
    for ( int i=0; i<100; i++ ) {		// fill list box
	QString str;
	str.sprintf( "line %d", i );
	if ( i == 42 && pix )
	    lb->insertItem( pm );
	else
	    lb->insertItem( str );
    }
    grid->addMultiCellWidget( lb, 2, 4, 0, 0 );
    connect( lb, SIGNAL(selected(int)), SLOT(listBoxItemSelected(int)) );
    QToolTip::add( lb, "list box" );

    vbox = new QVBoxLayout(8);
    grid->addLayout( vbox, 2, 1 );

  // Create a slider

    QSlider *sb = new QSlider( 0, 300, 1, 100, QSlider::Horizontal,
			       this, "Slider" );
    sb->setTickmarks( QSlider::Below );
    sb->setTickInterval( 10 );
    sb->setFocusPolicy( QWidget::TabFocus );
    sb->setFixedHeight(sb->sizeHint().height());
    vbox->addWidget( sb );

    connect( sb, SIGNAL(valueChanged(int)), SLOT(sliderValueChanged(int)) );
    QToolTip::add( sb, "slider" );

  // Create a combo box

    QComboBox *combo = new QComboBox( FALSE, this, "comboBox" );
    combo->insertItem( "darkBlue" );
    combo->insertItem( "darkRed" );
    combo->insertItem( "darkGreen" );
    combo->insertItem( "blue" );
    combo->insertItem( "red" );
    combo->setFixedHeight(combo->sizeHint().height());
    vbox->addWidget( combo );
    connect( combo, SIGNAL(activated(int)), SLOT(comboBoxItemActivated(int)) );
    QToolTip::add( combo, "read-only combo box" );

  // Create an editable combo box

    QComboBox *edCombo = new QComboBox( TRUE, this, "edComboBox" );
    edCombo->insertItem( "Permutable" );
    edCombo->insertItem( "Malleable" );
    edCombo->insertItem( "Adaptable" );
    edCombo->insertItem( "Alterable" );
    edCombo->insertItem( "Inconstant" );
    edCombo->setFixedHeight(edCombo->sizeHint().height());
    vbox->addWidget( edCombo );
    connect( edCombo, SIGNAL(activated(const char *)),
	     SLOT(edComboBoxItemActivated(const char *)) );
    QToolTip::add( edCombo, "editable combo box" );

    edCombo->setAutoCompletion( TRUE );

    vbox->addStretch( 1 );

    vbox = new QVBoxLayout(8);
    grid->addLayout( vbox, 2, 2 );

  // Create a spin box

    QSpinBox *spin = new QSpinBox( 0, 10, 1, this, "spin" );
    spin->setSuffix(" mm");
    spin->setSpecialValueText( "Auto" );
    spin->setMinimumSize( spin->sizeHint() );
    connect( spin, SIGNAL( valueChanged(const char*) ), 
	     SLOT( spinBoxValueChanged(const char*) ) );
    QToolTip::add( spin, "spin box" );
    vbox->addWidget( spin );
    
    vbox->addStretch( 1 );

  // Create a multi line edit

    QMultiLineEdit *mle = new QMultiLineEdit( this, "multiLineEdit" );

    grid->addMultiCellWidget( mle, 3, 3, 1, 2 );
    mle->setMinimumHeight(mle->fontMetrics().height()*3);
    mle->setText("This is a QMultiLineEdit widget,\n"
	         "useful for small multi-line\n"
		 "input fields.");
    QToolTip::add( mle, "multi line editor" );

  // Create a single line edit

    QLineEdit *le = new QLineEdit( this, "lineEdit" );
    grid->addMultiCellWidget( le, 4, 4, 1, 2 );
    le->setFixedHeight(le->sizeHint().height());
    connect( le, SIGNAL(textChanged(const char *)),
	         SLOT(lineEditTextChanged(const char *)) );
    QToolTip::add( le, "single line editor" );

  // Create a horizontal line (sort of QFrame) above the message line

    QFrame *separator = new QFrame( this, "separatorLine" );
    separator->setFrameStyle( QFrame::HLine | QFrame::Sunken );
    separator->setFixedHeight( separator->sizeHint().height() );
    grid->addMultiCellWidget( separator, 5, 5, 0, 2 );
    QToolTip::add( separator, "tool tips on a separator! wow!" );

    grid->setRowStretch(0,0);
    grid->setRowStretch(1,0);
    grid->setRowStretch(2,0);
    grid->setRowStretch(3,1);
    grid->setRowStretch(4,1);
    grid->setRowStretch(5,0);

    grid->setColStretch(0,1);
    grid->setColStretch(1,1);
    grid->setColStretch(2,1);


  // Create an label and a message in a plain widget
  // The message is updated when buttons are clicked etc.

    QHBoxLayout *hbox = new QHBoxLayout();
    topLayout->addLayout( hbox );
    QLabel *msgLabel = new QLabel( this, "msgLabel" );
    msgLabel->setText( "Message:" );
    msgLabel->setAlignment( AlignHCenter|AlignVCenter );
    msgLabel->setFixedSize( msgLabel->sizeHint() );
    hbox->addWidget( msgLabel );
    QToolTip::add( msgLabel, "label 1" );

    msg = new QLabel( this, "message" );
    msg->setFrameStyle( QFrame::Panel | QFrame::Sunken );
    msg->setAlignment( AlignCenter );
    msg->setFont( QFont("times",12,QFont::Bold) );
    msg->setText( "Message" );
    msg->setFixedHeight( msg->sizeHint().height() );
    msg->setText( "" );
    hbox->addWidget( msg, 5 );
    QToolTip::add( msg, "label 2" );


    topLayout->activate();
}

void WidgetView::setStatus(const char* text)
{
    msg->setText(text);
}

void WidgetView::button1Clicked()
{
    msg->setText( "The first push button was clicked" );
}


void WidgetView::movieUpdate( const QRect& )
{
    // Uncomment this to test animated icons on your window manager
    //setIcon( movie.framePixmap() );
}

void WidgetView::movieStatus( int s )
{
    switch ( s ) {
      case QMovie::SourceEmpty:
	movielabel->setText("Could not load\n" MOVIEFILENAME );
	movielabel->setAlignment( AlignCenter );
	movielabel->setBackgroundColor( backgroundColor() );
      break;
      default:
	if ( movielabel->movie() )	 	// for flicker-free animation:
	    movielabel->setBackgroundMode( NoBackground );
    }
}


void WidgetView::checkBoxClicked( int id )
{
    QString str;
    str.sprintf( "Check box %d clicked : ", id );
    QString chk = "---";
    if ( cb[0]->isChecked() )
	chk[0] = 'r';
    if ( cb[1]->isChecked() )
	chk[1] = 'w';
    if ( cb[2]->isChecked() )
	chk[2] = 'x';
    str += chk;
    msg->setText( str );
}


void WidgetView::edComboBoxItemActivated( const char * text)
{
    QString str;
    str.sprintf( "Editable Combo Box set to %s", text );
    msg->setText( str );
}


void WidgetView::radioButtonClicked( int id )
{
    QString str;
    str.sprintf( "Radio button #%d clicked", id );
    msg->setText( str );
}


void WidgetView::listBoxItemSelected( int index )
{
    QString str;
    str.sprintf( "List box item %d selected", index );
    msg->setText( str );
}


void WidgetView::sliderValueChanged( int value )
{
    QString str;
    str.sprintf( "Movie set to %d%% of normal speed", value );
    msg->setText( str );
    movie.setSpeed( value );
}


void WidgetView::comboBoxItemActivated( int index )
{
    QString str;
    str.sprintf( "Combo box item %d activated", index );
    msg->setText( str );
    switch ( index ) {
    default:
    case 0:
	QApplication::setWinStyleHighlightColor( darkBlue );
	break;
    case 1:
	QApplication::setWinStyleHighlightColor( darkRed );
	break;
    case 2:
	QApplication::setWinStyleHighlightColor( darkGreen );
	break;
    case 3:
	QApplication::setWinStyleHighlightColor( blue );
	break;
    case 4:
	QApplication::setWinStyleHighlightColor( red );
	break;
    }
}



void WidgetView::lineEditTextChanged( const char *newText )
{
    QString str( "Line edit text: ");
    str += newText;
    msg->setText( str );
}


void WidgetView::spinBoxValueChanged( const char * valueText )
{
    QString str( "Spin box value: " );
    str += valueText;
    msg->setText( str );
}    

//
// All application events are passed throught this event filter.
// We're using it to display some information about a clicked
// widget (right mouse button + CTRL).
//

bool WidgetView::eventFilter( QObject *obj, QEvent *event )
{
    static bool identify_now = TRUE;
    if ( event->type() == Event_MouseButtonPress && identify_now ) {
	QMouseEvent *e = (QMouseEvent*)event;
	if ( e->button() == RightButton && (e->state() & ControlButton) != 0 ){
	    QString str = "The clicked widget is a\n";
	    str += obj->className();
	    str += "\nThe widget's name is\n";
	    if ( obj->name() )
		str += obj->name();
	    else
		str += "<no name>";
	    identify_now = FALSE;		// don't do it in message box
	    QMessageBox::message( "Identify Widget", str, 0, (QWidget*)obj );
	    identify_now = TRUE;		// allow it again
	}
    }
    return FALSE;				// don't eat event
}

//
// Include the meta-object code for classes in this file
//

#include "widgets.moc"


//
// Create and display our WidgetView.
//

int main( int argc, char **argv )
{
    QApplication::setColorSpec( QApplication::CustomColor );
    QApplication a( argc, argv );
    WidgetView w;
    a.setMainWidget( &w );
    w.show();
    return a.exec();
}
