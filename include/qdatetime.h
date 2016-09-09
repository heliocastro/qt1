/****************************************************************************
** $Id: qdatetime.h,v 2.5.2.2 1998/08/25 09:20:54 hanord Exp $
**
** Definition of date and time classes
**
** Created : 940124
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

#ifndef QDATETIME_H
#define QDATETIME_H

#ifndef QT_H
#include "qstring.h"
#endif // QT_H


/*****************************************************************************
  QDate class
 *****************************************************************************/

class Q_EXPORT QDate
{
public:
    QDate()  { jd=0; }				// set null date
    QDate( int y, int m, int d );		// set date

    bool   isNull()	 const { return jd == 0; }
    bool   isValid()	 const;			// valid date

    int	   year()	 const;			// 1752..
    int	   month()	 const;			// 1..12
    int	   day()	 const;			// 1..31
    int	   dayOfWeek()	 const;			// 1..7 (monday==1)
    int	   dayOfYear()	 const;			// 1..365
    int	   daysInMonth() const;			// 28..31
    int	   daysInYear()	 const;			// 365 or 366

    virtual const char *monthName( int month ) const;
    virtual const char *dayName( int weekday ) const;

    QString toString()	 const;

    bool   setYMD( int y, int m, int d );

    QDate  addDays( int days )		const;
    int	   daysTo( const QDate & )	const;

    bool   operator==( const QDate &d ) const { return jd == d.jd; }
    bool   operator!=( const QDate &d ) const { return jd != d.jd; }
    bool   operator<( const QDate &d )	const { return jd < d.jd; }
    bool   operator<=( const QDate &d ) const { return jd <= d.jd; }
    bool   operator>( const QDate &d )	const { return jd > d.jd; }
    bool   operator>=( const QDate &d ) const { return jd >= d.jd; }

    static QDate currentDate();
    static bool	 isValid( int y, int m, int d );
    static bool	 leapYear( int year );

protected:
    static uint	 greg2jul( int y, int m, int d );
    static void	 jul2greg( uint jd, int &y, int &m, int &d );
private:
    static const char *monthNames[];
    static const char *weekdayNames[];
    uint	 jd;
    friend class QDateTime;
    friend Q_EXPORT QDataStream &operator<<( QDataStream &, const QDate & );
    friend Q_EXPORT QDataStream &operator>>( QDataStream &, QDate & );
};


/*****************************************************************************
  QTime class
 *****************************************************************************/

class Q_EXPORT QTime
{
public:
    QTime() { ds=0; }				// set null time
    QTime( int h, int m, int s=0, int ms=0 );	// set time

    bool   isNull()	 const { return ds == 0; }
    bool   isValid()	 const;			// valid time

    int	   hour()	 const;			// 0..23
    int	   minute()	 const;			// 0..59
    int	   second()	 const;			// 0..59
    int	   msec()	 const;			// 0..999

    QString toString()	 const;

    bool   setHMS( int h, int m, int s, int ms=0 );

    QTime  addSecs( int secs )		const;
    int	   secsTo( const QTime & )	const;
    QTime  addMSecs( int ms )		const;
    int	   msecsTo( const QTime & )	const;

    bool   operator==( const QTime &d ) const { return ds == d.ds; }
    bool   operator!=( const QTime &d ) const { return ds != d.ds; }
    bool   operator<( const QTime &d )	const { return ds < d.ds; }
    bool   operator<=( const QTime &d ) const { return ds <= d.ds; }
    bool   operator>( const QTime &d )	const { return ds > d.ds; }
    bool   operator>=( const QTime &d ) const { return ds >= d.ds; }

    static QTime currentTime();
    static bool	 isValid( int h, int m, int s, int ms=0 );

    void   start();
    int	   restart();
    int	   elapsed();

private:
    static bool currentTime( QTime * );

    uint   ds;
    friend class QDateTime;
    friend Q_EXPORT QDataStream &operator<<( QDataStream &, const QTime & );
    friend Q_EXPORT QDataStream &operator>>( QDataStream &, QTime & );
};


/*****************************************************************************
  QDateTime class
 *****************************************************************************/

class Q_EXPORT QDateTime
{
public:
    QDateTime() {}				// set null date and null time
    QDateTime( const QDate & );
    QDateTime( const QDate &, const QTime & );

    bool   isNull()	const		{ return d.isNull() && t.isNull(); }
    bool   isValid()	const		{ return d.isValid() && t.isValid(); }

    QDate  date()	const		{ return d; }
    QTime  time()	const		{ return t; }
    void   setDate( const QDate &date ) { d=date; }
    void   setTime( const QTime &time ) { t=time; }
    void   setTime_t( uint secsSince1Jan1970UTC );

    QString toString()	const;

    QDateTime addDays( int days )	const;
    QDateTime addSecs( int secs )	const;
    int	   daysTo( const QDateTime & )	const;
    int	   secsTo( const QDateTime & )	const;

    bool   operator==( const QDateTime &dt ) const;
    bool   operator!=( const QDateTime &dt ) const;
    bool   operator<( const QDateTime &dt )  const;
    bool   operator<=( const QDateTime &dt ) const;
    bool   operator>( const QDateTime &dt )  const;
    bool   operator>=( const QDateTime &dt ) const;

    static QDateTime currentDateTime();

private:
    QDate  d;
    QTime  t;
    friend Q_EXPORT QDataStream &operator<<( QDataStream &, const QDateTime &);
    friend Q_EXPORT QDataStream &operator>>( QDataStream &, QDateTime & );
};


/*****************************************************************************
  Date and time stream functions
 *****************************************************************************/

Q_EXPORT QDataStream &operator<<( QDataStream &, const QDate & );
Q_EXPORT QDataStream &operator>>( QDataStream &, QDate & );
Q_EXPORT QDataStream &operator<<( QDataStream &, const QTime & );
Q_EXPORT QDataStream &operator>>( QDataStream &, QTime & );
Q_EXPORT QDataStream &operator<<( QDataStream &, const QDateTime & );
Q_EXPORT QDataStream &operator>>( QDataStream &, QDateTime & );


#endif // QDATETIME_H
