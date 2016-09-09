/****************************************************************************
** $Id: qcursor.h,v 2.4.2.2 1998/08/25 09:20:52 hanord Exp $
**
** Definition of QCursor class
**
** Created : 940219
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

#ifndef QCURSOR_H
#define QCURSOR_H

#ifndef QT_H
#include "qpoint.h"
#include "qshared.h"
#endif // QT_H


struct QCursorData;				// internal cursor data


class Q_EXPORT QCursor					// cursor class
{
public:
    QCursor();					// create default arrow cursor
    QCursor( int shape );
    QCursor( const QBitmap &bitmap, const QBitmap &mask,
	     int hotX=-1, int hotY=-1 );
    QCursor( const QCursor & );
   ~QCursor();
    QCursor &operator=( const QCursor & );

    int		  shape()   const;
    void	  setShape( int );

    const QBitmap *bitmap() const;
    const QBitmap *mask()   const;
    QPoint	  hotSpot() const;

    HANDLE	  handle()  const;

    static QPoint pos();
    static void	  setPos( int x, int y );
    static void	  setPos( const QPoint & );

    static void	  initialize();
    static void	  cleanup();

private:
    void	  update() const;
    QCursorData	 *data;
};


inline void QCursor::setPos( const QPoint &p )
{
    setPos( p.x(), p.y() );
}


/*****************************************************************************
  Cursor shape identifiers (correspond to global cursor objects)
 *****************************************************************************/

enum QCursorShape {
    ArrowCursor, UpArrowCursor, CrossCursor, WaitCursor, IbeamCursor,
    SizeVerCursor, SizeHorCursor, SizeBDiagCursor, SizeFDiagCursor,
    SizeAllCursor, BlankCursor, LastCursor=BlankCursor, BitmapCursor=24 };


/*****************************************************************************
  Global cursors
 *****************************************************************************/

extern Q_EXPORT const QCursor arrowCursor;	// standard arrow cursor
extern Q_EXPORT const QCursor upArrowCursor;	// upwards arrow
extern Q_EXPORT const QCursor crossCursor;	// crosshair
extern Q_EXPORT const QCursor waitCursor;	// hourglass/watch
extern Q_EXPORT const QCursor ibeamCursor;	// ibeam/text entry
extern Q_EXPORT const QCursor sizeVerCursor;	// vertical resize
extern Q_EXPORT const QCursor sizeHorCursor;	// horizontal resize
extern Q_EXPORT const QCursor sizeBDiagCursor;	// diagonal resize (/)
extern Q_EXPORT const QCursor sizeFDiagCursor;	// diagonal resize (\)
extern Q_EXPORT const QCursor sizeAllCursor;	// all directions resize
extern Q_EXPORT const QCursor blankCursor;	// blank/invisible cursor


/*****************************************************************************
  QCursor stream functions
 *****************************************************************************/

Q_EXPORT QDataStream &operator<<( QDataStream &, const QCursor & );
Q_EXPORT QDataStream &operator>>( QDataStream &, QCursor & );


#endif // QCURSOR_H
