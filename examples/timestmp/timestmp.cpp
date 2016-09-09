/****************************************************************************
** $Id: timestmp.cpp,v 2.4.2.1 1998/09/01 09:51:02 hanord Exp $
**
** Copyright (C) 1992-1998 Troll Tech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#include <qdir.h>
#include <qdatastream.h>
#include <qdatetime.h>
#include <qstring.h>
#include <qapplication.h>

const char *dName = "time.qt";             // dir to make under home dir
const char *fName = "stamp.qt";            // time stamp file name

bool createTimeStamp();

int main( int, char ** ) 
{
    warning( "---------------------------- Qt timestamp example "
             "-----------------------------\n" );
    if ( createTimeStamp() ) {
        warning( "    Successfully created a new timestamp." );
	warning( "\n------------------------------------------------"
		 "-------------------------------" );
	return 0;
    } else {
        warning( "    Failed to create a new timestamp." );
	warning( "\n------------------------------------------------"
		 "-------------------------------" );
	return 1; // any non-zero value
    }
}


bool createTimeStamp()
{
    QDir d = QDir::home();     // now points to home directory
    QString absName = d.absFilePath( dName );     // absolute path
    if ( !d.cd( dName ) ) {   // now points to "time.qt" under home dir if OK
        QFileInfo fi( d, dName );                 // find out why cd failed
        if ( fi.exists() ) {                      // does it exist?
            if ( fi.isDir() )
                warning( "Cannot cd into \"%s\".", absName.data());
            else
                warning( "Cannot make directory \"%s\".\n"
                         "A file named \"%s\" already exists in \"%s\".", 
                         absName.data(), dName, (char*) d.path() );
            return FALSE;
        } else {
            warning("Making directory \"%s\".", absName.data() );
            if ( !d.mkdir( dName ) ) {
                warning("Could not make directory \"%s\".",absName.data() );
                return FALSE;
            }
            if ( !d.cd( dName ) ) {
                warning( "Cannot cd into \"%s\".", (char*) absName.data() );
                return FALSE;
	    }
        }
        
    }
    QString fp = d.filePath( fName );
    QFile file( fp );
    bool noFile = FALSE;                   // set to TRUE if file doesn't exist
    QDataStream s( &file );                // create a datastream on the file
    if ( file.exists() ) {
        if ( file.open( IO_ReadOnly ) ) {
            QDateTime last;
            s >> last;                 // read time stamp
            if ( last.isValid() ) {    // valid date and time?
                warning("This program was last run on %s.", 
                        (const char*)last.toString() );
            } else {                   // error in file format:
                warning("Error reading %s.", file.name() );
	    }
	} else {
            warning("Could not open the file \"%s\" for reading.",file.name());
	}
        file.close();                      // close file;
    } else {                               // file did not exist:
        noFile = TRUE;
    }
    if ( file.open( IO_WriteOnly | IO_Truncate ) ) {
        s << QDateTime::currentDateTime(); // write time stamp at beginning
        file.close();                      // flush and close file
        if ( noFile )                      // first time we write file?
            warning("This is probably the first time you're running"
                    " this program, try again!" );
        return TRUE;        
    } else {
        warning("Could not open the file \"%s\" for writing.", file.name());
        file.close();                      // close file
        return FALSE;
    }
}
