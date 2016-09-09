/****************************************************************************
** $Id: qfiledialog.h,v 2.20.2.3 1998/08/21 19:13:22 hanord Exp $
**
** Definition of QFileDialog class
**
** Created : 950428
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

#ifndef QFILEDIALOG_H
#define QFILEDIALOG_H

struct QFileDialogPrivate;
class QLineEdit;
class QPushButton;
class QListView;
class QListViewItem;
class QLabel;
class QWidget;

#ifndef QT_H
#include "qdir.h"
#include "qdialog.h"
#endif // QT_H


class Q_EXPORT QFileIconProvider: public QObject
{
    Q_OBJECT
public:
    QFileIconProvider( QObject * parent = 0, const char * name = 0 );

    virtual const QPixmap * pixmap( const QFileInfo & );
};


class Q_EXPORT QFileDialog : public QDialog
{
    Q_OBJECT
public:
    QFileDialog( const char *dirName, const char *filter = 0,
		 QWidget *parent=0, const char *name=0, bool modal=FALSE );
    QFileDialog( QWidget *parent=0, const char *name=0, bool modal=FALSE );
   ~QFileDialog();

    // recommended static functions

    static QString getOpenFileName( const char *initially = 0,
				    const char *filter= 0,
				    QWidget *parent = 0, const char *name = 0);
    static QString getSaveFileName( const char *initially = 0,
				    const char *filter= 0,
				    QWidget *parent = 0, const char *name = 0);
    static QString getExistingDirectory( const char *dir = 0,
					 QWidget *parent = 0,
					 const char *name = 0 );
    static QStrList getOpenFileNames( const char *filter= 0,
				      const char * dir = 0,
				      QWidget *parent = 0,
				      const char *name = 0);

    // other static functions

    static void setIconProvider( QFileIconProvider * );
    static QFileIconProvider * iconProvider();

    // non-static function for special needs

    QString selectedFile() const;
    void setSelection( const char* );

    const char *dirPath() const;

    void setDir( const QDir & );
    const QDir *dir() const;

    void rereadDir();

    enum Mode { AnyFile, ExistingFile, Directory, ExistingFiles };
    void setMode( Mode );
    Mode mode() const;

    bool eventFilter( QObject *, QEvent * );

public slots:
    void setDir( const char * );
    void setFilter( const char * );
    void setFilters( const char ** );
    void setFilters( const QStrList & );

signals:
    void fileHighlighted( const char * );
    void fileSelected( const char * );
    void dirEntered( const char * );

private slots:
    void fileSelected( int );
    void fileHighlighted( int );
    void dirSelected( int );
    void pathSelected( int );

    void updateFileNameEdit( QListViewItem *);
    void selectDirectoryOrFile( QListViewItem * );
    void popupContextMenu( QListViewItem *, const QPoint &, int );
    void fileNameEditDone();

    void okClicked();
    void filterClicked(); // not used
    void cancelClicked();

    void cdUpClicked();

    void fixupNameEdit();

protected:
    void resizeEvent( QResizeEvent * );
    void keyPressEvent( QKeyEvent * );

    void addWidgets( QLabel *, QWidget *, QPushButton * );

private slots:
    void updateGeometry();
    void modeButtonsDestroyed();

private:
    void init();
    void updatePathBox( const char * );
    bool trySetSelection( const QFileInfo&, bool );

    QDir cwd;
    QString fileName;

    QFileDialogPrivate *d;
    QListView  *files;

    QLineEdit  *nameEdit; // also filter
    void *unused1;
    void *unused2;
    void *unused3;
    void *unused4;
    void *unused5;
    void *unused6;
    void *unused7;
    QPushButton *okB;
    QPushButton *cancelB;

private:	// Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QFileDialog( const QFileDialog & );
    QFileDialog &operator=( const QFileDialog & );
#endif
};


#endif // QFILEDIALOG_H
