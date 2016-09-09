/****************************************************************************
** $Id: qprintdialog.h,v 2.10.2.2 1998/08/21 19:13:22 hanord Exp $
**
** Definition of print dialog.
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

#ifndef QPRINTDIALOG_H
#define QPRINTDIALOG_H

#ifndef QT_H
#include "qdialog.h"
#endif // QT_H

class QGroupBox;


struct QPrintDialogPrivate;


class Q_EXPORT QPrintDialog : public QDialog
{
    Q_OBJECT
public:
    QPrintDialog( QPrinter *, QWidget *parent=0, const char *name=0 );
    ~QPrintDialog();

    static bool getPrinterSetup( QPrinter * );

    void setPrinter( QPrinter *, bool = FALSE );
    QPrinter * printer() const;

private slots:
    void browseClicked();
    void okClicked();

    void printerOrFileSelected( int );
    void landscapeSelected( int );
    void paperSizeSelected( int );
    void orientSelected( int );
    void pageOrderSelected( int );
    void colorModeSelected( int );
    void setNumCopies( int );
    void printRangeSelected( int );
    void setFirstPage( int );
    void setLastPage( int );

private:
    QPrintDialogPrivate *d;

    QGroupBox * setupDestination();
    QGroupBox * setupOptions();
    QGroupBox * setupPaper();

private:	// Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QPrintDialog( const QPrintDialog & );
    QPrintDialog &operator=( const QPrintDialog & );
#endif
};


#endif // QPRINTDIALOG_H
