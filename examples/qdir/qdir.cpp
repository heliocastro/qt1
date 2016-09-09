/****************************************************************************
** $Id: qdir.cpp,v 1.6.2.1 1998/08/18 21:14:36 hanord Exp $
**
** Copyright (C) 1992-1998 Troll Tech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#include <qapplication.h>
#include "qfiledialog.h"

int main( int argc, char ** argv )
{
    QFileDialog::Mode mode = QFileDialog::ExistingFile;
    QString start;
    QString filter;
    QString caption;
    QApplication a( argc, argv );
    for (int i=1; i<argc; i++) {
	QString arg = argv[i];
	if ( arg == "-any" )
	    mode = QFileDialog::AnyFile;
	else if ( arg == "-dir" )
	    mode = QFileDialog::Directory;
	else if ( arg == "-default" )
	    start = argv[++i];
	else if ( arg == "-filter" )
	    filter = argv[++i];
	else if ( arg[0] == '-' ) {
	    debug("Usage: qdir [-any | -dir] [-default f] {-filter f} [caption ...]\n"
		  "      -any         Get any filename, need not exist.\n"
		  "      -dir         Return a directory rather than a file.\n"
		  "      -default f   Start from directory/file f.\n"
		  "      -filter f    eg. '*.gif' '*.bmp'\n"
		  "      caption ...  Caption for dialog.\n"
	    );
	    return 1;
	} else {
	    if ( !caption.isNull() )
		caption += ' ';
	    caption += arg;
	}
    }

    if ( start.isEmpty() )
	start = QDir::currentDirPath();

    if ( caption.isEmpty() )
	caption = mode == QFileDialog::Directory
		    ? "Choose directory..." : "Choose file...";

    QFileDialog fd( 0, filter, 0, 0, TRUE );
    fd.setMode( mode );
    fd.setCaption( caption );
    fd.setSelection( start );
    if ( fd.exec() == QDialog::Accepted ) {
	QString result = fd.selectedFile();
	printf("%s\n", (const char*)result);
	return 0;
    } else {
	return 1;
    }
}
