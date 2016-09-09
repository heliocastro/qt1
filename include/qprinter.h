/**********************************************************************
** $Id: qprinter.h,v 2.8.2.4 1998/10/29 16:15:19 warwick Exp $
**
** Definition of QPrinter class
**
** Created : 940927
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

#ifndef QPRINTER_H
#define QPRINTER_H

#ifndef QT_H
#include "qpaintdevice.h"
#include "qstring.h"
#endif // QT_H

#ifdef B0
#undef B0 // Terminal hang-up.
#endif

class Q_EXPORT QPrinter : public QPaintDevice
{
public:
    QPrinter();
   ~QPrinter();

    enum Orientation { Portrait, Landscape };
    enum PageSize    { A4, B5, Letter, Legal, Executive,
		       A0, A1, A2, A3, A5, A6, A7, A8, A9, B0, B1,
		       B10, B2, B3, B4, B6, B7, B8, B9, C5E, Comm10E,
		       DLE, Folio, Ledger, Tabloid };
    enum PageOrder   { FirstPageFirst, LastPageFirst };
    enum ColorMode   { GrayScale, Color };

    const char *printerName()	const;
    void	setPrinterName( const char * );
    bool	outputToFile()	const;
    void	setOutputToFile( bool );
    const char *outputFileName()const;
    void	setOutputFileName( const char * );
    const char *printProgram()	const;
    void	setPrintProgram( const char * );

    const char *docName()	const;
    void	setDocName( const char * );
    const char *creator()	const;
    void	setCreator( const char * );

    Orientation orientation()	const;
    void	setOrientation( Orientation );
    PageSize	pageSize()	const;
    void	setPageSize( PageSize );

    void	setPageOrder( PageOrder );
    PageOrder	pageOrder() const;

    void	setColorMode( ColorMode );
    ColorMode	colorMode() const;

    int		fromPage()	const;
    int		toPage()	const;
    void	setFromTo( int fromPage, int toPage );
    int		minPage()	const;
    int		maxPage()	const;
    void	setMinMax( int minPage, int maxPage );
    int		numCopies()	const;
    void	setNumCopies( int );

    bool	newPage();
    bool	abort();
    bool	aborted()	const;

    bool	setup( QWidget *parent = 0 );

protected:
    bool	cmd( int, QPainter *, QPDevCmdParam * );
    int		metric( int ) const;

#if defined(_WS_WIN_)
    void	setActive();
    void	setIdle();
#endif

private:
#if defined(_WS_X11_)
    QPaintDevice *pdrv;
#endif
    int		state;
    QString	printer_name;
    QString	output_filename;
    bool	output_file;
    QString	print_prog;
    QString	doc_name;
    QString	creator_name;
    Orientation orient;
    PageSize	page_size;
    short	from_pg, to_pg;
    short	min_pg,	 max_pg;
    short	ncopies;

private:	// Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QPrinter( const QPrinter & );
    QPrinter &operator=( const QPrinter & );
#endif
};


inline const char *QPrinter::printerName() const
{ return printer_name; }

inline bool QPrinter::outputToFile() const
{ return output_file; }

inline const char *QPrinter::outputFileName() const
{ return output_filename; }

inline const char *QPrinter::printProgram() const
{ return print_prog; }

inline const char *QPrinter::docName() const
{ return doc_name; }

inline const char *QPrinter::creator() const
{ return creator_name; }

inline QPrinter::PageSize QPrinter::pageSize() const
{ return (PageSize) ( ((int)page_size) & 255 ); }

inline QPrinter::Orientation QPrinter::orientation() const
{ return orient; }

inline int QPrinter::fromPage() const
{ return from_pg; }

inline int QPrinter::toPage() const
{ return to_pg; }

inline int QPrinter::minPage() const
{ return min_pg; }

inline int QPrinter::maxPage() const
{ return max_pg; }

inline int QPrinter::numCopies() const
{ return ncopies; }


#endif // QPRINTER_H
