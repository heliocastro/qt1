/****************************************************************************
** $Id: qmessagebox.h,v 2.27.2.2 1998/08/21 19:13:22 hanord Exp $
**
** Definition of QMessageBox class
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

#ifndef QMESSAGEBOX_H
#define QMESSAGEBOX_H

#ifndef QT_H
#include "qdialog.h"
#endif // QT_H

class  QLabel;
class  QPushButton;
struct QMBData;


class Q_EXPORT QMessageBox : public QDialog
{
    Q_OBJECT
public:
    enum Icon { NoIcon = 0, Information = 1, Warning = 2, Critical = 3 };

    QMessageBox( QWidget *parent=0, const char *name=0 );
    QMessageBox( const char *caption, const char *text, Icon icon,
		 int button0, int button1, int button2,
		 QWidget *parent=0, const char *name=0, bool modal=TRUE,
		 WFlags f=0 );
   ~QMessageBox();

    enum { Ok = 1, Cancel = 2, Yes = 3, No = 4, Abort = 5, Retry = 6,
	   Ignore = 7, ButtonMask = 0x07,
	   Default = 0x100, Escape = 0x200, FlagMask = 0x300 };

    static int information( QWidget *parent, const char *caption,
			    const char *text,
			    int button0, int button1=0, int button2=0 );
    static int information( QWidget *parent, const char *caption,
			    const char *text,
			    const char *button0Text = "OK",
			    const char *button1Text = 0, 
			    const char *button2Text = 0,
			    int defaultButtonNumber = 0,
			    int escapeButtonNumber = -1 );

    static int warning( QWidget *parent, const char *caption,
			const char *text,
			int button0, int button1, int button2=0 );
    static int warning( QWidget *parent, const char *caption,
			const char *text,
			const char *button0Text = "OK",
			const char *button1Text = 0, 
			const char *button2Text = 0,
			int defaultButtonNumber = 0,
			int escapeButtonNumber = -1 );

    static int critical( QWidget *parent, const char *caption,
			 const char *text,
			 int button0, int button1, int button2=0 );
    static int critical( QWidget *parent, const char *caption,
			 const char *text,
			 const char *button0Text = "OK",
			 const char *button1Text = 0, 
			 const char *button2Text = 0,
			 int defaultButtonNumber = 0,
			 int escapeButtonNumber = -1 );
    
    static void about( QWidget *parent, const char *caption,
		       const char *text );

    static void aboutQt( QWidget *parent, const char *caption=0 );

#if 1 /* OBSOLETE */
    static int message( const char *caption,
			const char *text,  const char *buttonText=0,
			QWidget *parent=0, const char *name=0 );

    static bool query( const char *caption,
		       const char *text,  const char *yesButtonText=0,
		       const char *noButtonText=0,
		       QWidget *parent=0, const char *name=0 );
#endif

    const char *text() const;
    void	setText( const char * );

    Icon	icon() const;
    void	setIcon( Icon );

    const QPixmap *iconPixmap() const;
    void	setIconPixmap( const QPixmap & );

#if 1 /* OBSOLETE */
    const char *buttonText() const;
    void	setButtonText( const char * );
#endif

    const char *buttonText( int button ) const;
    void	setButtonText( int button, const char * );

    void	adjustSize();

    void	setStyle( GUIStyle );

    static QPixmap standardIcon( Icon icon, GUIStyle style );

protected:
    void	resizeEvent( QResizeEvent * );
    void	keyPressEvent( QKeyEvent * );

private slots:
    void	buttonClicked();

private:
    void	init( int, int, int );
    int		indexOf( int ) const;
    void	resizeButtons();
    QLabel     *label;
    QMBData    *mbd;
    void       *reserved1;
    void       *reserved2;

private:	// Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QMessageBox( const QMessageBox & );
    QMessageBox &operator=( const QMessageBox & );
#endif
};


#endif // QMESSAGEBOX_H
