/****************************************************************************
** $Id: qxt.h,v 1.3.2.1 1998/08/14 09:13:21 warwick Exp $
**
** Definition of Qt extension classes for Xt/Motif support.
**
** Created : 980107
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

#ifndef QXT_H
#define QXT_H

#include <qapplication.h>
#include <qwidget.h>
#include <X11/Intrinsic.h>

class QXtApplication : public QApplication {
    Q_OBJECT
    void init();
public:
    QXtApplication(int& argc, char** argv,
	const char* appclass=0,
	XrmOptionDescRec *options=0, int num_options=0,
	const char** resources=0);
    QXtApplication(Display*);
    ~QXtApplication();

    bool x11EventFilter(XEvent*);
};

class QXtWidget : public QWidget {
    Q_OBJECT
    Widget xtw;
    void init(const char* name, WidgetClass widget_class,
		    Widget parent, ArgList args, Cardinal num_args,
		    bool managed);
    friend void qwidget_realize(
	Widget                widget,
	XtValueMask*          mask,
	XSetWindowAttributes* attributes
    );
public:
    QXtWidget(const char* name, Widget parent, bool managed=FALSE);
    QXtWidget(const char* name, WidgetClass widget_class,
	      QXtWidget *parent, ArgList args=0, Cardinal num_args=0,
	      bool managed=FALSE);
    ~QXtWidget();

    Widget xtWidget() const { return xtw; }

    void setGeometry( int x, int y, int w, int h );
    void setGeometry( const QRect & );

protected:
    bool x11Event( XEvent* );
    void leaveEvent(QEvent*);
};

#endif
