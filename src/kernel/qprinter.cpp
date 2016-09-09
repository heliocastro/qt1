/**********************************************************************
** $Id: qprinter.cpp,v 2.10.2.3 1998/11/02 12:58:39 paul Exp $
**
** Implementation of QPrinter class
**
** Created : 941003
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

#include "qprinter.h"

/*!
  \class QPrinter qprinter.h
  \brief The QPrinter class is a paint device that prints graphics on a
  printer.

  \ingroup drawing

  All window systems that Qt supports, except X11, have built-in
  printer drivers.  For X11, Qt provides PostScript (tm)
  printing.

  Drawing graphics on a printer is almost identical to drawing graphics
  in a widget or a pixmap.  The only difference is that the programmer
  must think about dividing the document into pages and handling abort
  commands.

  The default coordinate system of a printer is a 72 dpi (dots per
  inch) system with (0,0) at the upper left corner, with increasing
  values to the right and downwards.  This causes printer output to be
  roughly the same size as screen output on most screens.  You can
  easily change the coordinate system using QPainter::setViewport().
  QPainter::setWindow() and/or QPainter::setWorldMatrix().

  The newPage() function should be called to finish the current page and
  start printing a new page.

  If the user decides to abort printing, aborted() will return TRUE.
  The QPrinter class handles abortion automatically, but the programmer
  should from time to time check the aborted() flag and stop painting
  if the print job has been aborted.

  Example (a complete application):

  \code
    #include <qapplication.h>
    #include <qpainter.h>
    #include <qprinter.h>
    #include <qprintdialog.h>

    int main( int argc, char **argv )
    {
	QApplication a( argc, argv );

	QPrinter prt;
	if ( prt.setup() ) {
	    QPainter p;
	    p.begin( &prt );
	    p.rotate( 55 );
	    p.setFont( QFont("times", 144, QFont::Bold) );
	    p.drawText( 80,30, "Hello, world!" );
	    p.end();
	}
	return 0;
    }
  \endcode
*/


/*!
  \fn const char *QPrinter::printerName() const
  Returns the printer name.  This value is initially set to the name of the
  default printer.
  \sa setPrinterName()
*/

/*!
  Sets the printer name.

  The default printer will be used if no printer name is set.

  Under X11, the PRINTER environment variable defines the
  default printer.  Under any other window system, the window
  system defines the default printer.

  \sa printerName()
*/

void QPrinter::setPrinterName( const char *name )
{
    if ( state != 0 ) {
#if defined(CHECK_STATE)
	warning( "QPrinter::setPrinterName: Cannot do this during printing" );
#endif
	return;
    }
    printer_name = name;
}


/*!
  \fn bool QPrinter::outputToFile() const
  Returns TRUE if the output should be written to a file, or FALSE if the
  output should be sent directly to the printer.
  The default setting is FALSE.

  This function is currently only supported under X11.

  \sa setOutputToFile(), setOutputFileName()
*/

/*!
  Specifies whether the output should be written to a file or sent
  directly to the printer.

  Will output to a file if \e enable is TRUE, or will output directly
  to the printer if \e enable is FALSE.

  This function is currently only supported under X11.

  \sa outputToFile(), setOutputFileName()
*/

void QPrinter::setOutputToFile( bool enable )
{
    if ( state != 0 ) {
#if defined(CHECK_STATE)
	warning( "QPrinter::setOutputToFile: Cannot do this during printing" );
#endif
	return;
    }
    output_file = enable;
}


/*!
  \fn const char *QPrinter::outputFileName() const
  Returns the name of the output file.	There is no default file name.
  \sa setOutputFileName(), setOutputToFile()
*/

/*!
  Sets the name of the output file.

  Setting a null name (0 or "") disables output to a file, i.e.
  calls setOutputToFile(FALSE);
  Setting non-null name enables output to a file, i.e. calls
  setOutputToFile(TRUE).

  This function is currently only supported under X11.

  \sa outputFileName(), setOutputToFile()
*/

void QPrinter::setOutputFileName( const char *fileName )
{
    if ( state != 0 ) {
#if defined(CHECK_STATE)
	warning("QPrinter::setOutputFileName: Cannot do this during printing");
#endif
	return;
    }
    output_filename = fileName;
    output_file = !output_filename.isEmpty();
}


/*!
  \fn const char *QPrinter::printProgram() const
  Returns the name of the program that sends the print output to the printer.

  The default print program is "lpr" under X11.	 This function
  returns 0 for all other window systems.

  \sa setPrintProgram()
*/

/*!
  Sets the name of the program that should do the print job.

  If an output file has been defined, the printer driver will print to
  the output file instead of directly to the printer.

  On X11, this function sets the program to call with the PostScript
  output.  On other platforms, it has no effect.

  \sa printProgram()
*/

void QPrinter::setPrintProgram( const char *printProg )
{
    if ( state != 0 ) {
#if defined(CHECK_STATE)
	warning( "QPrinter::setPrintProgram: Cannot do this during printing" );
#endif
	return;
    }
    print_prog = printProg;
}


/*!
  \fn const char *QPrinter::docName() const
  Returns the document name.
  \sa setDocName()
*/

/*!
  Sets the document name.
*/

void QPrinter::setDocName( const char *name )
{
    if ( state != 0 ) {
#if defined(CHECK_STATE)
	warning( "QPrinter::setDocName: Cannot do this during printing" );
#endif
	return;
    }
    doc_name = name;
}


/*!
  \fn const char *QPrinter::creator() const
  Returns the creator name.
  \sa setCreator()
*/

/*!
  Sets the creator name.

  Calling this function only has effect for the X11 version of Qt.
  The creator name is the name of the application that created the document.
  If no creator name is specified, then the creator will be set to "Qt".

  \sa creator()
*/

void QPrinter::setCreator( const char *creator )
{
    creator_name = creator;
}


/*!
  \fn Orientation QPrinter::orientation() const
  Returns the orientation setting. The default value is \c QPrinter::Portrait.
  \sa setOrientation()
*/

/*!
  Sets the print orientation.

  The orientation can be either \c QPrinter::Portrait or
  \c QPrinter::Landscape.

  The printer driver reads this setting and prints using the specified
  orientation.

  \sa orientation()
*/

void QPrinter::setOrientation( Orientation orientation )
{
    orient = orientation;
}


/*!
  \fn PageSize QPrinter::pageSize() const
  Returns the printer page size. The default value is \c QPrinter::A4.
  \sa setPageSize()
*/


static QPrinter::PageSize makepagesize( QPrinter::PageSize ps,
					QPrinter::PageOrder po,
					QPrinter::ColorMode cm )
{
    return (QPrinter::PageSize)( ((int)ps & 255) +
				 ((po == QPrinter::LastPageFirst) ? 256 : 0) +
				 ((cm == QPrinter::GrayScale) ? 512 : 0) );
}



//   <li>\c QPrinter::Executive (184 x 267 mm)

/*!
  Sets the printer page size to \a newPageSize.

  The page size can be one of
  <ul>
  <li>\c QPrinter::A0 (841 x 1189 mm)
  <li>\c QPrinter::A1 (594 x 841 mm)
  <li>\c QPrinter::A2 (420 x 594 mm)
  <li>\c QPrinter::A3 (297 x 420 mm)
  <li>\c QPrinter::A4 (210x297 mm, 8.26x11.7 inches)
  <li>\c QPrinter::A5 (148 x 210 mm)
  <li>\c QPrinter::A6 (105 x 148 mm)
  <li>\c QPrinter::A7 (74 x 105 mm)
  <li>\c QPrinter::A8 (52 x 74 mm)
  <li>\c QPrinter::A9 (37 x 52 mm)
  <li>\c QPrinter::B0 (1030 x 1456 mm)
  <li>\c QPrinter::B1 (728 x 1030 mm)
  <li>\c QPrinter::B10 (32 x 45 mm)
  <li>\c QPrinter::B2 (515 x 728 mm)
  <li>\c QPrinter::B3 (364 x 515 mm)
  <li>\c QPrinter::B4 (257 x 364 mm)
  <li>\c QPrinter::B5 (182x257 mm, 7.17x10.13 inches)
  <li>\c QPrinter::B6 (128 x 182 mm)
  <li>\c QPrinter::B7 (91 x 128 mm)
  <li>\c QPrinter::B8 (64 x 91 mm)
  <li>\c QPrinter::B9 (45 x 64 mm)
  <li>\c QPrinter::C5E (163 x 229 mm)
  <li>\c QPrinter::Comm10E (105 x 241 mm, US Common #10 Envelope)
  <li>\c QPrinter::DLE (110 x 220 mm)
  <li>\c QPrinter::Executive (7.5x10 inches, 191x254 mm)
  <li>\c QPrinter::Folio (210 x 330 mm)
  <li>\c QPrinter::Ledger (432 x 279 mm)
  <li>\c QPrinter::Legal (8.5x14 inches, 216x356 mm)
  <li>\c QPrinter::Letter (8.5x11 inches, 216x279 mm)
  <li>\c QPrinter::Tabloid (279 x 432 mm)

  </ul>

  \sa pageSize()
*/

void QPrinter::setPageSize( PageSize newPageSize )
{
    page_size = makepagesize( newPageSize, pageOrder(), colorMode() );
}


/*!  Sets the page order to \a newPageOrder.

  The page order can be \c QPrinter::FirstPageFirst or \c
  QPrinter::LastPageFirst.  The application programmer is responsible
  for reading the page order and printing accordingly.
*/

void QPrinter::setPageOrder( PageOrder newPageOrder )
{
    page_size = makepagesize( pageSize(), newPageOrder, colorMode() );
}


/*!  Returns the current page order.

  The default page order is \a FirstPageFirst.
*/

QPrinter::PageOrder QPrinter::pageOrder() const
{
    if ( ((int)page_size) & 256 )
	return QPrinter::LastPageFirst;
    else
	return QPrinter::FirstPageFirst;
}


/*!  Sets the printer's color mode to \a newColorMode, which can be
  one of \c Color (the default) and \c GrayScale.

  A future version of Qt will modify its printing accordingly.  At
  present, QPrinter behaves as if \c Color is selected.

  \sa colorMode()
*/

void QPrinter::setColorMode( ColorMode newColorMode )
{
    page_size = makepagesize( pageSize(), pageOrder(), newColorMode );
}


/*!  Returns the current color mode.  The default color more is \c
  Color.

  \sa setColorMode()
*/

QPrinter::ColorMode QPrinter::colorMode() const
{
    if ( ((int)page_size) & 512 )
	return QPrinter::GrayScale;
    else
	return QPrinter::Color;


}


/*!
  \fn int QPrinter::fromPage() const
  Returns the from-page setting. The default value is 0.

  The programmer is responsible for reading this setting and print
  accordingly.

  \sa setFromTo(), toPage()
*/

/*!
  \fn int QPrinter::toPage() const
  Returns the to-page setting. The default value is 0.

  The programmer is responsible for reading this setting and print
  accordingly.

  \sa setFromTo(), fromPage()
*/

/*!
  Sets the from page and to page.

  The from-page and to-page settings specify what pages to print.

  \sa fromPage(), toPage(), setMinMax(), setup()
*/

void QPrinter::setFromTo( int fromPage, int toPage )
{
    if ( state != 0 ) {
#if defined(CHECK_STATE)
	warning( "QPrinter::setFromTo: Cannot do this during printing" );
#endif
	return;
    }
    from_pg = fromPage;
    to_pg = toPage;
}


/*!
  \fn int QPrinter::minPage() const
  Returns the min-page setting.	 The default value is 0.
  \sa maxPage(), setMinMax()
*/

/*!
  \fn int QPrinter::maxPage() const
  Returns the max-page setting.	 The default value is 0.
  \sa minPage(), setMinMax()
*/

/*!
  Sets the min page and max page.

  The min-page and max-page restrict the from-page and to-page settings.
  When the printer setup dialog comes up, the user cannot select
  from and to that are outsize the range specified by min and max pages.

  \sa minPage(), maxPage(), setFromTo(), setup()
*/

void QPrinter::setMinMax( int minPage, int maxPage )
{
    min_pg = minPage;
    max_pg = maxPage;
}


/*!
  \fn int QPrinter::numCopies() const
  Returns the number of copies to be printed.  The default value is 1.
  \sa setNumCopies()
*/

/*!
  Sets the number of pages to be printed.

  The printer driver reads this setting and prints the specified number of
  copies.

  \sa numCopies(), setup()
*/

void QPrinter::setNumCopies( int numCopies )
{
    if ( state != 0 ) {
#if defined(CHECK_STATE)
	warning( "QPrinter::setNumCopies: Cannot do this during printing" );
#endif
	return;
    }
    ncopies = numCopies;
}
