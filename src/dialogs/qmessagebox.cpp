/****************************************************************************
** $Id: qmessagebox.cpp,v 2.51.2.2 1999/02/10 12:26:05 warwick Exp $
**
** Implementation of QMessageBox class
**
** Created : 950503
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

#include "qmessagebox.h"
#include "qlabel.h"
#include "qpushbutton.h"
#include "qimage.h"
#include "qkeycode.h"
#include "qapplication.h"

// Message box icons, from page 210 of the Windows style guide.

// Hand-drawn to resemble Microsoft's icons, but in the Mac/Netscape
// palette.  The "question mark" icon, which Microsoft recommends not
// using but a lot of people still use, is left out.

/* XPM */
static const char* information_xpm[]={
"32 32 5 1",
". c None",
"c c #000000",
"# c #999999",
"a c #ffffff",
"b c #0000ff",
"...........########.............",
"........###aaaaaaaa###..........",
"......##aaaaaaaaaaaaaa##........",
".....#aaaaaaaaaaaaaaaaaa#.......",
"....#aaaaaaaabbbbaaaaaaaac......",
"...#aaaaaaaabbbbbbaaaaaaaac.....",
"..#aaaaaaaaabbbbbbaaaaaaaaac....",
".#aaaaaaaaaaabbbbaaaaaaaaaaac...",
".#aaaaaaaaaaaaaaaaaaaaaaaaaac#..",
"#aaaaaaaaaaaaaaaaaaaaaaaaaaaac#.",
"#aaaaaaaaaabbbbbbbaaaaaaaaaaac#.",
"#aaaaaaaaaaaabbbbbaaaaaaaaaaac##",
"#aaaaaaaaaaaabbbbbaaaaaaaaaaac##",
"#aaaaaaaaaaaabbbbbaaaaaaaaaaac##",
"#aaaaaaaaaaaabbbbbaaaaaaaaaaac##",
"#aaaaaaaaaaaabbbbbaaaaaaaaaaac##",
".#aaaaaaaaaaabbbbbaaaaaaaaaac###",
".#aaaaaaaaaaabbbbbaaaaaaaaaac###",
"..#aaaaaaaaaabbbbbaaaaaaaaac###.",
"...caaaaaaabbbbbbbbbaaaaaac####.",
"....caaaaaaaaaaaaaaaaaaaac####..",
".....caaaaaaaaaaaaaaaaaac####...",
"......ccaaaaaaaaaaaaaacc####....",
".......#cccaaaaaaaaccc#####.....",
"........###cccaaaac#######......",
"..........####caaac#####........",
".............#caaac##...........",
"...............caac##...........",
"................cac##...........",
".................cc##...........",
"..................###...........",
"...................##..........."};
/* XPM */
static const char* warning_xpm[]={
"32 32 4 1",
". c None",
"a c #ffff00",
"# c #000000",
"b c #999999",
".............###................",
"............#aaa#...............",
"...........#aaaaa#b.............",
"...........#aaaaa#bb............",
"..........#aaaaaaa#bb...........",
"..........#aaaaaaa#bb...........",
".........#aaaaaaaaa#bb..........",
".........#aaaaaaaaa#bb..........",
"........#aaaaaaaaaaa#bb.........",
"........#aaaa###aaaa#bb.........",
".......#aaaa#####aaaa#bb........",
".......#aaaa#####aaaa#bb........",
"......#aaaaa#####aaaaa#bb.......",
"......#aaaaa#####aaaaa#bb.......",
".....#aaaaaa#####aaaaaa#bb......",
".....#aaaaaa#####aaaaaa#bb......",
"....#aaaaaaaa###aaaaaaaa#bb.....",
"....#aaaaaaaa###aaaaaaaa#bb.....",
"...#aaaaaaaaa###aaaaaaaaa#bb....",
"...#aaaaaaaaaa#aaaaaaaaaa#bb....",
"..#aaaaaaaaaaa#aaaaaaaaaaa#bb...",
"..#aaaaaaaaaaaaaaaaaaaaaaa#bb...",
".#aaaaaaaaaaaa##aaaaaaaaaaa#bb..",
".#aaaaaaaaaaa####aaaaaaaaaa#bb..",
"#aaaaaaaaaaaa####aaaaaaaaaaa#bb.",
"#aaaaaaaaaaaaa##aaaaaaaaaaaa#bb.",
"#aaaaaaaaaaaaaaaaaaaaaaaaaaa#bbb",
"#aaaaaaaaaaaaaaaaaaaaaaaaaaa#bbb",
".#aaaaaaaaaaaaaaaaaaaaaaaaa#bbbb",
"..#########################bbbbb",
"....bbbbbbbbbbbbbbbbbbbbbbbbbbb.",
".....bbbbbbbbbbbbbbbbbbbbbbbbb.."};
/* XPM */
static const char* critical_xpm[]={
"32 32 4 1",
". c None",
"a c #999999",
"# c #ff0000",
"b c #ffffff",
"...........########.............",
".........############...........",
".......################.........",
"......##################........",
".....####################a......",
"....######################a.....",
"...########################a....",
"..#######b##########b#######a...",
"..######bbb########bbb######a...",
".######bbbbb######bbbbb######a..",
".#######bbbbb####bbbbb#######a..",
"#########bbbbb##bbbbb#########a.",
"##########bbbbbbbbbb##########a.",
"###########bbbbbbbb###########aa",
"############bbbbbb############aa",
"############bbbbbb############aa",
"###########bbbbbbbb###########aa",
"##########bbbbbbbbbb##########aa",
"#########bbbbb##bbbbb#########aa",
".#######bbbbb####bbbbb#######aa.",
".######bbbbb######bbbbb######aa.",
"..######bbb########bbb######aaa.",
"..#######b##########b#######aa..",
"...########################aaa..",
"....######################aaa...",
"....a####################aaa....",
".....a##################aaa.....",
"......a################aaa......",
".......aa############aaaa.......",
".........aa########aaaaa........",
"...........aaaaaaaaaaa..........",
".............aaaaaaa............"};


/*!
  \class QMessageBox qmessagebox.h
  \brief Displays a brief message, an icon, and some buttons.
  \ingroup dialogs

  \define QMessageBox::Icon

  A message box is a modal dialog that displays an icon, a text and up to
  three push buttons.  It's used for simple messages and questions.

  QMessageBox provides a range of different messages, arranged roughly
  along two axes: Severity and complexity.

  Severity is
  <ul>
  <li> \c Information - for message boxes that are part of normal operation,
  <li> \c Warning - for message boxes that tell the user about errors
  or ask the user how to fix an error, or
  <li> \c Critical - as Warning, but for critical errors.
  </ul>
  The message box has a different icon for each of the severity levels.

  Complexity is one button (Ok, sometimes Dismiss for Motif
  applications) for a simple messages, or two or even three buttons
  for questions.

  Here are some examples of how to use the static member functions.
  After these examples you will find an overview of the non-static
  member functions.

  If a program is unable to find a supporting file, it may perhaps do:

  \code
    QMessageBox::information( this, "Application name here",
                              "Unable to find the file \"index.html\".\n"
			      "The factory default will be used instead." );
  \endcode

  The Microsoft Windows User Interface Guidelines strongly recommends
  using the application name as window caption.  The message box has
  just one button, OK, and its text tells the user both what happened
  and what the program will do about it.  Since the application is
  able to make do, the message box is just information, not a warning
  or a critical error.

  Exiting a program is part of its normal operation, and if there are
  unsaved data the user probably should be asked what to do, for
  example like this:

  \code
    switch( QMessageBox::information( this, "Application name here",
                                      "The document contains unsaved work\n"
                                      "Do you want to save it before exiting?",
			              "&Save", "&Don't Save", "&Cancel",
                                      0,      // Enter == button 0
				      2 ) ) { // Escape == button 2
    case 0: // Save clicked, Alt-S or Enter pressed.
        // save
	break;
    case 1: // Don't Save clicked or Alt-D pressed
        // don't save but exit
	break;
    case 2: // Cancel clicked, Alt-C or Escape pressed
        // don't exit
	break;
    }
  \endcode

  Again, the application name is used as window caption, as Microsoft
  recommends.  The Escape button cancels the entire Exit operation,
  and Enter/Return saves the document and exits.

  warning() can be used to tell the user about unusual errors, or
  errors which can't be easily fixed:

  \code
    switch( QMessageBox::warning( this, "Application name here",
                                  "Could not connect to the <mumble> server.\n"
				  "This program can't function correctly "
				  "without the server.\n\n",
				  "Try again", "Quit", 0,
				  0, 1 );
    case 0: // Try again or Enter
        // try again
	break;
    case 1: // Quit or Escape
        // exit
	break;
    }
  \endcode

  Disk full errors are unusual (in a perfect world, they are) and they
  certainly can be hard to correct.  This example uses predefined buttons
  instead of hardcoded button texts:

  \code
    switch( QMessageBox::warning( this, "Application name here",
                                  "Could not save the the user preferences,\n"
				  "because the disk is full.  You can delete\n"
				  "some files and press Retry, or you can\n"
				  "abort the Save Preferences operation.",
				  QMessageBox::Retry | QMessageBox::Default,
				  QMessageBox::Abort | QMessageBox::Escape )) {
    case QMessageBox::Retry: // Retry or Enter
        // try again
	break;
    case QMessageBox::Abort: // Abort or Cancel
        // abort
	break;
    }
  \endcode

  The critical() function should be reserved for critical errors.  In
  this example, errorDetails is a QString or const char *, and QString
  is used to concatenate several strings:

  \code
    QMessageBox::critical( 0, "Application name here",
                           QString("An internal error occured. Please call ") +
			   "technical support at 123456789 and report these\n"+
			   "numbers:\n\n" + errorDetails +
			   "\n\n<Application> will now exit." );
  \endcode

  QMessageBox provides a very simple About box, which displays an
  appropriate icon and the string you give it:

  \code
     QMessageBox::about( this, "About <Application>",
                         "<Application> is a <one-paragraph blurb>\n\n"
			 "Copyright 1951-1997 Such-and-such.  "
			 "<License words here.>\n\n"
			 "For technical support, call 123456789 or see\n"
			 "http://www.such-and-such.com/Application/\n" );
  \endcode

  See about() for more information.

  Finally, you can make a QMessageBox from scratch and set custom
  button texts:

  \code
    QMessageBox mb( "Application name here",
		    "Saving the file will overwrite the old file on disk.\n"
		    "Do you really want to save?",
		    QMessageBox::Information,
		    QMessageBox::Yes | QMessageBox::Default,
		    QMessageBox::No,
		    QMessageBox::Cancel | QMessageBox::Escape );
    mb.setButtonText( QMessageBox::Yes, "Save" );
    mb.setButtonText( QMessageBox::No, "Don't Save" );
    switch( mb.exec() ) {
        case QMessageBox::Yes:
	    // save and exit
	    break;
        case QMessageBox::No:
	    // exit without saving
	    break;
	case QMessageBox::Cancel:
	    // don't save and don't exit
	    break;
    }
  \endcode

  QMessageBox defines two enum types, Icon and an unnamed button type.
  Icon defines the \c Information, \c Warning and \c Critical icons for
  each GUI style.  It is used by the constructor, by the static member
  functions information(), warning() and critical(), and there is a
  function called standardIcon() which gives you access to the various
  icons.

  The button types are:
  <ul>
  <li> \c Ok - the default for single-button message boxes
  <li> \c Cancel - note that this is \e not automatically Escape
  <li> \c Yes
  <li> \c No
  <li> \c Abort
  <li> \c Retry
  <li> \c Ignore
  </ul>

  Button types can be combined with two modifiers by using OR:
  <ul>
  <li> \c Default - makes pressing Enter or Return be equivalent with
  clicking this button.  Normally used with Ok, Yes or similar.
  <li> \c Escape - makes pressing Escape be equivalent with this button.
  Normally used with Abort, Cancel or similar.
  </ul>

  The text(), icon() and iconPixmap() functions provide access to the
  current text and pixmap of a message box, and setText(), setIcon()
  and setIconPixmap() lets you change it.  The difference between
  setIcon() and setIconPixmap() is that the former accepts a
  QMessageBox::Icon and can it be used to set standard icons while the
  latter accepts a QPixmap and can be used to set custom icons.

  setButtonText() and buttonText() provide access to the buttons.

  QMessageBox has no signals or slots.

  <img src=qmsgbox-m.gif> <img src=qmsgbox-w.gif>

  \sa QDialog, <a href="http://www.iarchitect.com/errormsg.htm">Isys on
  error messages,</a>
  <a href="guibooks.html#fowler">GUI Design Handbook: Message Box.</a>

*/


struct QMBData {
    QMBData(QMessageBox* parent) :
	iconLabel( parent, "icon" )
    {
    }

    int			numButtons;		// number of buttons
    QMessageBox::Icon	icon;			// message box icon
    QLabel		iconLabel;		// label holding any icon
    int			button[3];		// button types
    int			defButton;		// default button (index)
    int			escButton;		// escape button (index)
    QSize		buttonSize;		// button size
    QPushButton	       *pb[3];			// buttons
};

static const int LastButton = QMessageBox::Ignore;

/*
  NOTE: The table of button texts correspond to the button enum.
*/

static const char *mb_texts[] = {
    0, "OK", "Cancel", "Yes", "No", "Abort", "Retry", "Ignore", 0
};


/*!
  Constructs a message box with no text and a button with the text "OK".

  If \e parent is 0, then the message box becomes an application-global
  modal dialog box.  If \e parent is a widget, the message box becomes
  modal relative to \e parent.

  The \e parent and \e name arguments are passed to the QDialog constructor.
*/

QMessageBox::QMessageBox( QWidget *parent, const char *name )
    : QDialog( parent, name, TRUE )
{
    init( Ok, 0, 0 );
}


/*!
  Constructs a message box with a \a caption, a \a text, an \a icon and up
  to three buttons.

  The \a icon must be one of:
  <ul>
  <li> \c QMessageBox::NoIcon
  <li> \c QMessageBox::Information
  <li> \c QMessageBox::Warning
  <li> \c QMessageBox::Critical
  </ul>

  Each button can have one of the following values:
  <ul>
  <li>\c QMessageBox::Ok
  <li>\c QMessageBox::Cancel
  <li>\c QMessageBox::Yes
  <li>\c QMessageBox::No
  <li>\c QMessageBox::Abort
  <li>\c QMessageBox::Retry
  <li>\c QMessageBox::Ignore
  </ul>

  One of the buttons can be combined with the \c QMessageBox::Default flag
  to make a default button.

  One of the buttons can be combined with the \c QMessageBox::Escape flag
  to make an escape option.  Hitting the Esc key on the keyboard has
  the same effect as clicking this button with the mouse.

  Example:
  \code
    QMessageBox mb( "Hardware failure",
		    "Disk error detected\nDo you want to stop?",
		    QMessageBox::NoIcon,
		    QMessageBox::Yes | QMessageBox::Default,
		    QMessageBox::No | QMessageBox::Escape );
    if ( mb.exec() == QMessageBox::No )
        // try again
  \endcode

  If \a parent is 0, then the message box becomes an application-global
  modal dialog box.  If \a parent is a widget, the message box becomes
  modal relative to \e parent.

  If \a modal is TRUE the message becomes modal, otherwise it becomes
  modeless.

  The \a parent, \a name, \a modal and \a f arguments are passed to the
  QDialog constructor.

  \sa setCaption(), setText(), setIcon()
*/

QMessageBox::QMessageBox( const char *caption, const char *text, Icon icon,
			  int button0, int button1, int button2,
			  QWidget *parent, const char *name,
			  bool modal, WFlags f )
    : QDialog( parent, name, modal, f )
{
    init( button0, button1, button2 );
    setCaption( caption );
    setText( text );
    setIcon( icon );
}


/*!
  Destroys the message box.
*/

QMessageBox::~QMessageBox()
{
    delete mbd;
}


void QMessageBox::init( int button0, int button1, int button2 )
{
    label = new QLabel( this, "text" );
    CHECK_PTR( label );
    label->setAlignment( AlignLeft );

    if ( (button2 && !button1) || (button1 && !button0) ) {
#if defined(CHECK_RANGE)
	::warning( "QMessageBox: Inconsistent button parameters" );
#endif
	button0 = button1 = button2 = 0;
    }
    mbd = new QMBData(this);
    CHECK_PTR( mbd );
    mbd->numButtons = 0;
    mbd->button[0] = button0;
    mbd->button[1] = button1;
    mbd->button[2] = button2;
    mbd->defButton = -1;
    mbd->escButton = -1;
    int i;
    for ( i=0; i<3; i++ ) {
	int b = mbd->button[i];
	if ( (b & Default) ) {
	    if ( mbd->defButton >= 0 ) {
#if defined(CHECK_RANGE)
		::warning( "QMessageBox: There can be at most one "
			   "default button" );
#endif
	    } else {
		mbd->defButton = i;
	    }
	}
	if ( (b & Escape) ) {
	    if ( mbd->escButton >= 0 ) {
#if defined(CHECK_RANGE)
		::warning( "QMessageBox: There can be at most one "
			   "escape button" );
#endif
	    } else {
		mbd->escButton = i;
	    }
	}
	b &= ButtonMask;
	if ( b == 0 ) {
	    if ( i == 0 )			// no buttons, add an Ok button
		b = Ok;
	} else if ( b < 0 || b > LastButton ) {
#if defined(CHECK_RANGE)
	    ::warning( "QMessageBox: Invalid button specifier" );
#endif
	    b = Ok;
	} else {
	    if ( i > 0 && mbd->button[i-1] == 0 ) {
#if defined(CHECK_RANGE)
		::warning( "QMessageBox: Inconsistent button parameters; "
			   "button %d defined but not button %d",
			   i+1, i );
#endif
		b = 0;
	    }
	}
	mbd->button[i] = b;
	if ( b )
	    mbd->numButtons++;
    }
    for ( i=0; i<3; i++ ) {
	if ( i >= mbd->numButtons ) {
	    mbd->pb[i] = 0;
	} else {
	    QString buttonName;
	    buttonName.sprintf( "button%d", i+1 );
	    mbd->pb[i] = new QPushButton( mb_texts[mbd->button[i]],
					  this, buttonName );
	    if ( mbd->defButton == i ) {
		mbd->pb[i]->setDefault( TRUE );
		mbd->pb[i]->setFocus();
	    }
	    mbd->pb[i]->setAutoDefault( TRUE );
	    mbd->pb[i]->setFocusPolicy( QWidget::StrongFocus );
	    connect( mbd->pb[i], SIGNAL(clicked()), SLOT(buttonClicked()) );
	}
    }
    resizeButtons();
    reserved1 = reserved2 = 0;
    setFontPropagation( SameFont );
    setPalettePropagation( SamePalette );
}


int QMessageBox::indexOf( int button ) const
{
    int index = -1;
    for ( int i=0; i<mbd->numButtons; i++ ) {
	if ( mbd->button[i] == button ) {
	    index = i;
	    break;
	}
    }
    return index;
}


void QMessageBox::resizeButtons()
{
    int i;
    QSize maxSize( style() == MotifStyle ? 0 : 75, 0 );
    for ( i=0; i<mbd->numButtons; i++ ) {
	QSize s = mbd->pb[i]->sizeHint();
	maxSize.setWidth(  QMAX(maxSize.width(), s.width()) );
	maxSize.setHeight( QMAX(maxSize.height(),s.height()) );
    }
    mbd->buttonSize = maxSize;
    for ( i=0; i<mbd->numButtons; i++ )
	mbd->pb[i]->resize( maxSize );
}


/*!
  Returns the message box text currently set, or null if no text has been set.
  \sa setText()
*/

const char *QMessageBox::text() const
{
    return label->text();
}

/*!
  Sets the message box text to be displayed.
  \sa text()
*/

void QMessageBox::setText( const char *text )
{
    label->setText( text );
}


/*!
  Returns the icon of the message box.

  The return value is one of the following:
  <ul>
  <li> \c QMessageBox::NoIcon
  <li> \c QMessageBox::Information
  <li> \c QMessageBox::Warning
  <li> \c QMessageBox::Critical
  </ul>

  \sa setIcon(), iconPixmap()
*/

QMessageBox::Icon QMessageBox::icon() const
{
    return mbd->icon;
}


/*!
  Sets the icon of the message box to \a icon, which is a predefined icon:

  <ul>
  <li> \c QMessageBox::NoIcon
  <li> \c QMessageBox::Information
  <li> \c QMessageBox::Warning
  <li> \c QMessageBox::Critical
  </ul>

  The actual pixmap used for displaying the icon depends on the current
  \link style() GUI style\endlink.  You can also set a custom pixmap icon
  using the setIconPixmap() function.

  \sa icon(), setIconPixmap(), iconPixmap()
*/

void QMessageBox::setIcon( Icon icon )
{
    setIconPixmap( standardIcon(icon, style()) );
    mbd->icon = icon;
}

/*!
  Returns the pixmap used for a standard icon.  This
  allows the pixmaps to be used in more complex message boxes.
*/

QPixmap QMessageBox::standardIcon( Icon icon, GUIStyle style )
{
    const char **xpm_data;
    switch ( icon ) {
	case Information:
	    xpm_data = information_xpm;
	    break;
	case Warning:
	    xpm_data = warning_xpm;
	    break;
	case Critical:
	    xpm_data = critical_xpm;
	    break;
	default:
	    xpm_data = 0;	
    }
    QPixmap pm;
    if ( xpm_data ) {
	QImage image(xpm_data);
	if ( style == MotifStyle ) {
	    // All that colour looks ugly in Motif
	    QColorGroup g = QApplication::palette()->normal();
	    switch ( icon ) {
	      case Information:
		image.setColor( 2, 0xff000000 | g.dark().rgb() );
		image.setColor( 3, 0xff000000 | g.base().rgb() );
		image.setColor( 4, 0xff000000 | g.text().rgb() );
		break;
	      case Warning:
		image.setColor( 1, 0xff000000 | g.base().rgb() );
		image.setColor( 2, 0xff000000 | g.text().rgb() );
		image.setColor( 3, 0xff000000 | g.dark().rgb() );
		break;
	      case Critical:
		image.setColor( 1, 0xff000000 | g.dark().rgb() );
		image.setColor( 2, 0xff000000 | g.text().rgb() );
		image.setColor( 3, 0xff000000 | g.base().rgb() );
	        break;
	      default:
		; // Can't happen
	    }
	}
	pm.convertFromImage(image);
    }
    return pm;
}


/*!
  Returns the icon pixmap of the message box.

  Example:
  \code
    QMessageBox mb(...);
    mb.setIcon( QMessageBox::Warning );
    mb.iconPixmap();	// returns the warning icon pixmap
  \endcode

  \sa setIconPixmap(), icon()
*/

const QPixmap *QMessageBox::iconPixmap() const
{
    return mbd->iconLabel.pixmap();
}

/*!
  Sets the icon of the message box to a custom \a pixmap.  Note that
  it's often hard to draw one pixmap which looks appropriate in both
  Motif and Windoes GUI styles.  You may want to draw two.

  \sa iconPixmap(), setIcon()
*/

void QMessageBox::setIconPixmap( const QPixmap &pixmap )
{
    mbd->iconLabel.setPixmap(pixmap);
    mbd->icon = NoIcon;
}


/*!
  This function will be removed in a future version of Qt.
*/

const char *QMessageBox::buttonText() const
{
    return buttonText( Ok );
}

/*!
  This function will be removed in a future version of Qt.
*/

void QMessageBox::setButtonText( const char *text )
{
    setButtonText( Ok, text ? text : "OK" );
}


/*!
  Returns the text of the message box button \a button, or null if the
  message box does not contain the button.

  Example:
  \code
    QMessageBox mb( QMessageBox::Ok, QMessageBox::Cancel, 0 );
    mb.buttonText( QMessageBox::Cancel );  // returns "Cancel"
    mb.buttonText( QMessageBox::Ignore );  // returns 0
  \endcode

  \sa setButtonText()
*/

const char *QMessageBox::buttonText( int button ) const
{
    int index = indexOf(button);
    return index >= 0 && mbd->pb[index] ? mbd->pb[index]->text() : 0;
}


/*!
  Sets the text of the message box button \a button to \a text.
  Setting the text of a button that is not in the message box is quietly
  ignored.

  Example:
  \code
    QMessageBox mb( QMessageBox::Ok, QMessageBox::Cancel, 0 );
    mb.setButtonText( QMessageBox::Ok, "All Right" );
    mb.setButtonText( QMessageBox::Yes, "Yo" );	  // ignored
  \endcode

  \sa buttonText()
*/

void QMessageBox::setButtonText( int button, const char *text )
{
    int index = indexOf(button);
    if ( index >= 0 && mbd->pb[index] ) {
	mbd->pb[index]->setText( text );
	resizeButtons();
    }
}


/*!
  Internal slot to handle button clicks.
*/

void QMessageBox::buttonClicked()
{
    int reply = 0;
    const QObject *s = sender();
    for ( int i=0; i<mbd->numButtons; i++ ) {
	if ( mbd->pb[i] == s )
	    reply = mbd->button[i];
    }
    done( reply );
}


/*!
  Adjusts the size of the message box to fit the contents just before
  QDialog::exec() or QDialog::show() is called.

  This function will not be called if the message box has been explicitly
  resized before showing it.
*/

void QMessageBox::adjustSize()
{
    int i;
    QSize smax = mbd->buttonSize;
    int border = smax.height()/2;
    if ( border == 0 )
	border = 10;
    else if ( style() == MotifStyle )
	border += 6;
    for ( i=0; i<mbd->numButtons; i++ )
	mbd->pb[i]->resize( smax );
    label->adjustSize();
    int bw = mbd->numButtons * smax.width() + (mbd->numButtons-1)*border;
    int w = QMAX( bw, label->width() ) + 2*border;
    int h = smax.height();
    if ( label->height() )
	h += label->height() + 3*border;
    else
	h += 2*border;
    if ( mbd->iconLabel.pixmap() && mbd->iconLabel.pixmap()->width() )  {
	mbd->iconLabel.adjustSize();
	w += mbd->iconLabel.pixmap()->width() + border;
	if ( h < mbd->iconLabel.pixmap()->height() + 3*border + smax.height() )
	    h = mbd->iconLabel.pixmap()->height() + 3*border + smax.height();
    }
    resize( w, h );	
}


/*!
  Handles resize events for the message box.
*/

void QMessageBox::resizeEvent( QResizeEvent * )
{
    int i;
    int n  = mbd->numButtons;
    int bw = mbd->buttonSize.width();
    int bh = mbd->buttonSize.height();
    int border = bh/2;
    if ( border == 0 )
	border = 10;
    else if ( style() == MotifStyle )
	border += 6;
    int lmargin = 0;
    mbd->iconLabel.adjustSize();
    mbd->iconLabel.move( border, border );
    if ( mbd->iconLabel.pixmap() && mbd->iconLabel.pixmap()->width() )
	lmargin += mbd->iconLabel.pixmap()->width() + border;
    label->move( (width() + lmargin)/2 - label->width()/2,
		 (height() - border - bh - label->height()) / 2 );
    int space = (width() - bw*n)/(n+1);
    for ( i=0; i<n; i++ ) {
	mbd->pb[i]->move( space*(i+1)+bw*i,
			  height() - border - bh );
    }
}


/*!
  Handles key press events for the message box.
*/

void QMessageBox::keyPressEvent( QKeyEvent *e )
{
    if ( e->key() == Key_Escape ) {
	if ( mbd->escButton >= 0 ) {
	    QPushButton *pb = mbd->pb[mbd->escButton];
	    pb->animateClick();
	}
	e->accept();
    } else {
	QDialog::keyPressEvent( e );
    }
}


/*****************************************************************************
  Static QMessageBox functions
 *****************************************************************************/

/*!
  Opens a modal message box directly using the specified parameters.

  \warning This function is kept for compatibility with old Qt programs
  and will be removed in a future version of Qt.  Please use
  information(), warning() or critical() instead.
*/

int QMessageBox::message( const char *caption,
			  const char *text,
			  const char *buttonText,
			  QWidget    *parent,
			  const char * )
{
    return QMessageBox::information( parent, caption, text, buttonText?buttonText:"Ok" ) == 0;
}


/*!
  Queries the user using a modal message box with two buttons.
  Note that \a caption is not always shown, it depends on the window manager.

  \warning This function is kept for compatibility with old Qt programs
  and will be removed in a future version of Qt.  Please use
  information(), warning() or critical() instead.
*/

bool QMessageBox::query( const char *caption,
			 const char *text,
			 const char *yesButtonText,
			 const char *noButtonText,
			 QWidget *parent, const char * )
{
    return QMessageBox::information( parent, caption, text,
				     yesButtonText?yesButtonText:"Ok", noButtonText ) == 0;
}


/*!
  Opens an information message box with a caption, a text and up to three
  buttons.  Returns the identifier of the button that was clicked.

  If \e parent is 0, then the message box becomes an application-global
  modal dialog box.  If \e parent is a widget, the message box becomes
  modal relative to \e parent.

  \sa warning(), critical()
*/

int QMessageBox::information( QWidget *parent,
			      const char *caption, const char *text,
			      int button0, int button1, int button2 )
{
    QMessageBox *mb = new QMessageBox( caption, text, Information,
				       button0, button1, button2,
				       parent, "information" );
    CHECK_PTR( mb );
    int reply = mb->exec();
    delete mb;
    return reply;
}


/*!
  Opens a warning message box with a caption, a text and up to three
  buttons.  Returns the identifier of the button that was clicked.

  If \e parent is 0, then the message box becomes an application-global
  modal dialog box.  If \e parent is a widget, the message box becomes
  modal relative to \e parent.

  \sa information(), critical()
*/

int QMessageBox::warning( QWidget *parent,
			  const char *caption, const char *text,
			  int button0, int button1, int button2 )
{
    QMessageBox *mb = new QMessageBox( caption, text, Warning,
				       button0, button1, button2,
				       parent, "warning" );
    CHECK_PTR( mb );
    int reply = mb->exec();
    delete mb;
    return reply;
}


/*!
  Opens a critical message box with a caption, a text and up to three
  buttons.  Returns the identifier of the button that was clicked.

  If \e parent is 0, then the message box becomes an application-global
  modal dialog box.  If \e parent is a widget, the message box becomes
  modal relative to \e parent.

  \sa information(), warning()
*/

int QMessageBox::critical( QWidget *parent,
			   const char *caption, const char *text,
			   int button0, int button1, int button2 )
{
    QMessageBox *mb = new QMessageBox( caption, text, Critical,
				       button0, button1, button2,
				       parent, "critical" );
    CHECK_PTR( mb );
    int reply = mb->exec();
    delete mb;
    return reply;
}


/*!
  Displays a simple about box with window caption \a caption and
  body text \a text.

  about() looks for a suitable icon for the box in four locations:
  <ol> <li>It prefers \link QWidget::icon() parent->icon() \endlink
  if that exists.  <li>If not, it tries the top level widget
  containing \a parent <li>If that too fails, it tries the \link
  QApplication::mainWidget() main widget. \endlink <li>As a last
  resort it uses the Information icon. </ol>

  The about box has a single button labelled OK.

  \sa QWidget::icon() QApplication::mainWidget()
*/

void QMessageBox::about( QWidget *parent, const char *caption,
			 const char *text )
{
    QMessageBox *mb = new QMessageBox( caption, text,
				       Information,
				       Ok + Default, 0, 0,
				       parent, "simple about box" );
    CHECK_PTR( mb );
    QPixmap i;
    if ( parent && parent->icon())
	i = *(parent->icon());
    if ( i.isNull() && parent &&
	 parent->topLevelWidget()->icon() )
	i = *(parent->topLevelWidget()->icon());
    if ( i.isNull() && qApp && qApp->mainWidget() &&
	 qApp->mainWidget()->icon() )
	i = *(qApp->mainWidget()->icon());
    if ( !i.isNull() )
	mb->setIconPixmap( i );
    mb->exec();
    delete mb;
}


/*!
  Reimplemented for implementational reasons.
*/

void QMessageBox::setStyle( GUIStyle s )
{
    QWidget::setStyle( s );
    if ( mbd->icon != NoIcon ) {
	// Reload icon for new style
	setIcon( mbd->icon );
    }
}


static int textBox( QWidget *parent, QMessageBox::Icon severity,
		    const char *caption, const char *text,
		    const char *button0Text,
		    const char *button1Text,
		    const char *button2Text,
		    int defaultButtonNumber,
		    int escapeButtonNumber )
{
    int b[3];
    b[0] = (button0Text && *button0Text) ? 1 : 0;
    b[1] = (button1Text && *button1Text) ? 2 : 0;
    b[2] = (button2Text && *button2Text) ? 3 : 0;

    int i;
    for( i=0; i<3; i++ ) {
	if ( b[i] && defaultButtonNumber == i )
	    b[i] += QMessageBox::Default;
	if ( b[i] && escapeButtonNumber == i )
	    b[i] += QMessageBox::Escape;
    }

    QMessageBox *mb = new QMessageBox( caption, text, severity,
				       b[0], b[1], b[2],
				       parent, "information" );
    CHECK_PTR( mb );
    if ( b[0] )
	mb->setButtonText( 1, button0Text );
    if ( b[1] )
	mb->setButtonText( 2, button1Text );
    if ( b[2] )
	mb->setButtonText( 3, button2Text );
    int reply = mb->exec();
    delete mb;
    return reply-1;
}


/*!
  Displays an information message box with a caption, a text and
  1-3 buttons.  Returns the number of the button that was clicked
  (0, 1 or 2).

  \a button0Text is the text of the first button and must be present,
  \a button1Text is the text of the second button and is optional, and
  \a button2Text is the text of the third button and is optional.  \a
  defaultbuttonNumber (0-2) is the index of the default button;
  pressing Return or Enter is the same as clicking the default button.
  It defaults to 0 (the first button).  \a escapeButtonNumber is the
  index of the Escape button; pressing Escape is the same as clicking
  this button.  It defaults to -1 (pressing Escape does nothing);
  supply 0, 1 or 2 to make pressing Escape be equivalent with clicking
  the relevant button.

  If \e parent is 0, then the message box becomes an application-global
  modal dialog box.  If \e parent is a widget, the message box becomes
  modal relative to \e parent.

  \sa warning(), critical()
*/

int QMessageBox::information( QWidget *parent, const char *caption,
			      const char *text,
			      const char *button0Text,
			      const char *button1Text,
			      const char *button2Text,
			      int defaultButtonNumber,
			      int escapeButtonNumber )
{
    return textBox( parent, Information, caption, text,
		    button0Text, button1Text, button2Text,
		    defaultButtonNumber, escapeButtonNumber );
}


/*!
  Displays a warning message box with a caption, a text and
  1-3 buttons.  Returns the number of the button that was clicked
  (0, 1 or 2).

  \a button0Text is the text of the first button and must be present,
  \a button1Text is the text of the second button and is optional, and
  \a button2Text is the text of the third button and is optional.  \a
  defaultbuttonNumber (0-2) is the index of the default button;
  pressing Return or Enter is the same as clicking the default button.
  It defaults to 0 (the first button).  \a escapeButtonNumber is the
  index of the Escape button; pressing Escape is the same as clicking
  this button.  It defaults to -1 (pressing Escape does nothing);
  supply 0, 1 or 2 to make pressing Escape be equivalent with clicking
  the relevant button.

  If \e parent is 0, then the message box becomes an application-global
  modal dialog box.  If \e parent is a widget, the message box becomes
  modal relative to \e parent.

  \sa information(), critical()
*/

int QMessageBox::warning( QWidget *parent, const char *caption,
				 const char *text,
				 const char *button0Text,
				 const char *button1Text,
				 const char *button2Text,
				 int defaultButtonNumber,
				 int escapeButtonNumber )
{
    return textBox( parent, Warning, caption, text,
		    button0Text, button1Text, button2Text,
		    defaultButtonNumber, escapeButtonNumber );
}


/*!
  Displays a critical error message box with a caption, a text and
  1-3 buttons.  Returns the number of the button that was clicked
  (0, 1 or 2).

  \a button0Text is the text of the first button and must be present,
  \a button1Text is the text of the second button and is optional, and
  \a button2Text is the text of the third button and is optional.  \a
  defaultbuttonNumber (0-2) is the index of the default button;
  pressing Return or Enter is the same as clicking the default button.
  It defaults to 0 (the first button).  \a escapeButtonNumber is the
  index of the Escape button; pressing Escape is the same as clicking
  this button.  It defaults to -1 (pressing Escape does nothing);
  supply 0, 1 or 2 to make pressing Escape be equivalent with clicking
  the relevant button.

  If \e parent is 0, then the message box becomes an application-global
  modal dialog box.  If \e parent is a widget, the message box becomes
  modal relative to \e parent.

  \sa information() warning()
*/

int QMessageBox::critical( QWidget *parent, const char *caption,
				  const char *text,
				  const char *button0Text,
				  const char *button1Text,
				  const char *button2Text,
				  int defaultButtonNumber,
				  int escapeButtonNumber )
{
    return textBox( parent, Critical, caption, text,
		    button0Text, button1Text, button2Text,
		    defaultButtonNumber, escapeButtonNumber );
}


static const char *textAboutQt =
"This program is developed with Qt, the multi-platform C++ GUI toolkit.\n\n"
"Qt is a product of Troll Tech AS (http://www.troll.no).\n"
"Qt is available under two different licenses:\n"
"- The Free Edition, which may be used free of charge to develop\n"
"  Free Software on the X Window System.\n"
"- The Professional Edition, which may be used to develop commercial\n"
"  software on both X and Microsoft Windows.\n\n"
"Please contact sales@troll.no for information and pricing.";


/*!
  Displays a simple message box about Qt, with window caption \a
  caption and optionally centered over \a parent.

  This is neat for inclusion into the Help menu.  See the menu.cpp
  example.
*/

void QMessageBox::aboutQt( QWidget *parent, const char *caption )
{
    if ( !caption )
	caption = "About Qt";
    information( parent, caption, textAboutQt );
}
