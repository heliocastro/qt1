/****************************************************************************
** $Id: qdatetime.cpp,v 2.16.2.2 1999/02/15 10:56:45 hanord Exp $
**
** Implementation of date and time classes
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

#define gettimeofday	__hide_gettimeofday
#include "qdatetime.h"
#include "qdatastream.h"
#include <stdio.h>
#include <time.h>
#if defined(_OS_WIN32_)
#if defined(_CC_BOOL_DEF_)
#undef	bool
#include <windows.h>
#define bool int
#else
#include <windows.h>
#endif
#elif defined(_OS_MSDOS_)
#include <dos.h>
#elif defined(_OS_OS2_)
#include <os2.h>
#elif defined(UNIX)
#include <sys/time.h>
#include <unistd.h>
#undef	gettimeofday
extern "C" int gettimeofday( struct timeval *, struct timezone * );
#endif

static const uint FIRST_DAY	= 2361222;	// Julian day for 17520914
static const int  FIRST_YEAR	= 1752;		// ### wrong for many countries
static const uint SECS_PER_DAY	= 86400;
static const uint MSECS_PER_DAY = 86400000;
static const uint SECS_PER_HOUR = 3600;
static const uint MSECS_PER_HOUR= 3600000;
static const uint SECS_PER_MIN	= 60;
static const uint MSECS_PER_MIN = 60000;

static short monthDays[] ={0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

const char *QDate::monthNames[] = {
    "Jan", "Feb", "Mar", "Apr", "May", "Jun",
    "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };

const char *QDate::weekdayNames[] ={
    "Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun" };


/*****************************************************************************
  QDate member functions
 *****************************************************************************/

/*!
  \class QDate qdatetime.h
  \brief The QDate class provides date functions.

  \ingroup time

  The QDate is based on the Gregorian (modern western) calendar. England
  adopted the Gregorian calendar on September 14th 1752, which is the
  earliest date that is supported by QDate.  Using earlier dates will give
  undefined results. Some countries adopted the Gregorian calendar later
  than England, thus the week day of early dates might be incorrect for
  these countries (but correct for England).  The end of time is reached
  around 8000AD, by which time we expect Qt to be obsolete.

  \sa QTime, QDateTime
*/

/*!
  \fn QDate::QDate()
  Constructs a null date.  Null dates are invalid.
*/

/*!
  Constructs a date with the year \e y, month \e m and day \e d.
*/

QDate::QDate( int y, int m, int d )
{
    setYMD( y, m, d );
}


/*!
  \fn bool QDate::isNull() const
  Returns TRUE if the date is null.  A null date is invalid.
  \sa isValid()
*/

/*!
  Returns TRUE if the date is valid.
  \sa isNull()
*/

bool QDate::isValid() const
{
    return jd >= FIRST_DAY;
}


/*!
  Returns the year (>= 1752) for this date.
  \sa month(), day()
*/

int QDate::year() const
{
    int y, m, d;
    jul2greg( jd, y, m, d );
    return y;
}

/*!
  Returns the month (January=1 .. December=12) for this date.
  \sa year(), day()
*/

int QDate::month() const
{
    int y, m, d;
    jul2greg( jd, y, m, d );
    return m;
}

/*!
  Returns the day of the month (1..31) for this date.
  \sa year(), month(), dayOfWeek()
*/

int QDate::day() const
{
    int y, m, d;
    jul2greg( jd, y, m, d );
    return d;
}

/*!
  Returns the weekday (Monday=1 .. Sunday=7) for this date.
  \sa day()
*/

int QDate::dayOfWeek() const
{
    return (((jd+1) % 7) + 6)%7 + 1;
}

/*!
  Returns the day of the year (1..365) for this date.
*/

int QDate::dayOfYear() const
{
    return jd - greg2jul(year(), 1, 1) + 1;
}

/*!
  Returns the number of days in the month (28..31) for this date.
*/

int QDate::daysInMonth() const
{
    int y, m, d;
    jul2greg( jd, y, m, d );
    if ( m == 2 && leapYear(y) )
	return 29;
    else
	return monthDays[m];
}

/*!
  Returns the number of days in the year (365 or 366) for this date.
*/

int QDate::daysInYear() const
{
    int y, m, d;
    jul2greg( jd, y, m, d );
    return leapYear(y) ? 366 : 365;
}


/*!
  Returns the name of the \e month.

  Month 1 == "Jan", month 2 == "Feb" etc.
*/

const char *QDate::monthName( int month) const
{
#if defined(DEBUG)
    ASSERT( month > 0 && month <= 12 );
#endif
    return monthNames[month-1];
}

/*!
  Returns the name of the \e weekday.

  Weekday 1 == "Mon", day 2 == "Tue" etc.
*/

const char *QDate::dayName( int weekday) const
{
#if defined(DEBUG)
    ASSERT( weekday > 0 && weekday <= 7 );
#endif
    return weekdayNames[weekday-1];
}


/*!
  Returns the date as a string.

  The string format is "Sat May 20 1995".
*/

QString QDate::toString() const
{
    QString buf;
    int y, m, d;
    jul2greg( jd, y, m, d );
    buf.sprintf( "%s %s %d %d", dayName(dayOfWeek()), monthName(m), d, y);
    return buf;
}


/*!
  Sets the year \e y, month \e m and day \e d.
  Returns TRUE if the date is valid, otherwise FALSE.
*/

bool QDate::setYMD( int y, int m, int d )
{
    if ( !isValid(y,m,d) ) {
#if defined(CHECK_RANGE)
	 warning( "QDate::setYMD: Invalid date %04d/%02d/%02d", y, m, d );
#endif
	 return FALSE;
    }
    jd = greg2jul( y, m, d );
#if defined(DEBUG)
    ASSERT( year() == y && month() == m && day() == d );
#endif
    return TRUE;
}

/*!
  Returns this date plus \e ndays days.
*/

QDate QDate::addDays( int ndays ) const
{
    QDate d;
    d.jd = jd + ndays;
    return d;
}

/*!
  Returns the number of days from this date to \a d, which is negative
  if \a d is in the past.

  Example:
  \code
    QDate d1( 1995, 5, 17 );		// May 17th 1995
    QDate d2( 1995, 5, 20 );		// May 20th 1995
    d1.daysTo( d2 );			// returns 3
    d2.daysTo( d1 );			// returns -3
  \endcode
  \sa addDays()
*/

int QDate::daysTo( const QDate &d ) const
{
    return d.jd - jd;
}


/*!
  \fn bool QDate::operator==( const QDate &d ) const
  Returns TRUE if this date is equal to \e d, or FALSE if
  they are different.
*/

/*!
  \fn bool QDate::operator!=( const QDate &d ) const
  Returns TRUE if this date is different from \e d, or FALSE if
  they are equal.
*/

/*!
  \fn bool QDate::operator<( const QDate &d ) const
  Returns TRUE if this date is before \e d, otherwise FALSE.
*/

/*!
  \fn bool QDate::operator<=( const QDate &d ) const
  Returns TRUE if this date is before or equal to \e d, otherwise FALSE.
*/

/*!
  \fn bool QDate::operator>( const QDate &d ) const
  Returns TRUE if this date is after \e d, otherwise FALSE.
*/

/*!
  \fn bool QDate::operator>=( const QDate &d ) const
  Returns TRUE if this date is equal to or after \e d, otherwise FALSE.
*/


/*!
  Returns the current date.
  \sa QTime::currentTime(), QDateTime::currentDateTime()
*/

QDate QDate::currentDate()
{
#if defined(_OS_WIN32_)

    SYSTEMTIME t;
    GetLocalTime( &t );
    QDate d;
    d.jd = greg2jul( t.wYear, t.wMonth, t.wDay );
    return d;

#else

    time_t ltime;
    time( &ltime );
    tm *t = localtime( &ltime );
    QDate d;
    d.jd = greg2jul( t->tm_year + 1900, t->tm_mon + 1, t->tm_mday );
    return d;

#endif
}

/*!
  Returns TRUE if the specified date is valid.

  Note that years 00-99 are treated as 1900-1999.

  \sa isNull()
*/

bool QDate::isValid( int y, int m, int d )
{
    if ( y >= 0 && y <= 99 )
	y += 1900;
    else if ( y < FIRST_YEAR || (y == FIRST_YEAR && (m < 9 ||
						    (m == 9 && d < 14))) )
	return FALSE;
    return (d > 0 && m > 0 && m <= 12) &&
	   (d <= monthDays[m] || (d == 29 && m == 2 && leapYear(y)));
}

/*!
  Returns TRUE if the specified year \e y is a leap year.
*/

bool QDate::leapYear( int y )
{
    return y % 4 == 0 && y % 100 != 0 || y % 400 == 0;
}

/*!
  \internal
  Converts a Gregorian date to a Julian day.
  This algorithm is taken from Communications of the ACM, Vol 6, No 8.
  \sa jul2greg()
*/

uint QDate::greg2jul( int y, int m, int d )
{
    uint c, ya;
    if ( y <= 99 )
	y += 1900;
    if ( m > 2 ) {
	m -= 3;
    } else {
	m += 9;
	y--;
    }
    c = y;					// NOTE: Sym C++ 6.0 bug
    c /= 100;
    ya = y - 100*c;
    return 1721119 + d + (146097*c)/4 + (1461*ya)/4 + (153*m+2)/5;
}

/*!
  \internal
  Converts a Julian day to a Gregorian date.
  This algorithm is taken from Communications of the ACM, Vol 6, No 8.
  \sa greg2jul()
*/

void QDate::jul2greg( uint jd, int &y, int &m, int &d )
{
    uint x;
    uint j = jd - 1721119;
    y = (j*4 - 1)/146097;
    j = j*4 - 146097*y - 1;
    x = j/4;
    j = (x*4 + 3) / 1461;
    y = 100*y + j;
    x = (x*4) + 3 - 1461*j;
    x = (x + 4)/4;
    m = (5*x - 3)/153;
    x = 5*x - 3 - 153*m;
    d = (x + 5)/5;
    if ( m < 10 ) {
	m += 3;
    } else {
	m -= 9;
	y++;
    }
}


/*****************************************************************************
  QTime member functions
 *****************************************************************************/

/*!
  \class QTime qdatetime.h

  \brief The QTime class provides time functions 24 hours a day.

  \capt Time functions

  \ingroup time

  The time resolution of QTime is a millisecond, although the accuracy
  depends on the underlying operating system.  Some operating systems
  (e.g. Linux and Window NT) support a one-millisecond resolution, while
  others (MS-DOS and Windows 3.1) support only a 55 millisecond resolution.

  \sa QDate, QDateTime
*/

/*!
  \fn QTime::QTime()
  Constructs a time 00:00:00.000, which is valid.
*/

/*!
  Constructs a time with hour \e h, minute \e m, seconds \e s and milliseconds
  \e ms.
*/

QTime::QTime( int h, int m, int s, int ms )
{
    setHMS( h, m, s, ms );
}


/*!
  \fn bool  QTime::isNull() const
  Returns TRUE if the time is equal to 00:00:00.000. A null time is valid.

  \sa isValid()
*/

/*!
  Returns TRUE if the time is valid, or FALSE if the time is invalid.
  The time 23:30:55.746 is valid, while 24:12:30 is invalid.

  \sa isNull()
*/

bool QTime::isValid() const
{
    return ds < MSECS_PER_DAY;
}


/*!
  Returns the hour part (0..23) of the time.
*/

int QTime::hour() const
{
    return ds / MSECS_PER_HOUR;
}

/*!
  Returns the minute part (0..59) of the time.
*/

int QTime::minute() const
{
    return (ds % MSECS_PER_HOUR)/MSECS_PER_MIN;
}

/*!
  Returns the second part (0..59) of the time.
*/

int QTime::second() const
{
    return (ds / 1000)%SECS_PER_MIN;
}

/*!
  Returns the millisecond part (0..999) of the time.
*/

int QTime::msec() const
{
    return ds % 1000;
}


/*!
  Converts the date to a string, which is returned.  Milliseconds are
  not included. The string format is "03:40:13".
*/

QString QTime::toString() const
{
    QString buf;
    buf.sprintf( "%.2d:%.2d:%.2d", hour(), minute(), second() );
    return buf;
}


/*!
  Sets the hour \e h, minute \e m, seconds \e s and milliseconds
  \e ms.
  Returns TRUE if the time is valid, otherwise FALSE.
*/

bool QTime::setHMS( int h, int m, int s, int ms )
{
    if ( !isValid(h,m,s,ms) ) {
#if defined(CHECK_RANGE)
	warning( "QTime::setHMS Invalid time %02d:%02d:%02d.%03d", h, m, s,
		 ms );
#endif
	return FALSE;
    }
    ds = (h*SECS_PER_HOUR + m*SECS_PER_MIN + s)*1000 + ms;
    return TRUE;
}

/*!
  Returns the time plus \e nsecs seconds.
  \sa secsTo()
*/

QTime QTime::addSecs( int nsecs ) const
{
    QTime t;
    t.ds = ((int)ds + nsecs*1000) % MSECS_PER_DAY;
    return t;
}

/*!
  Returns the number of seconds from this time to \a t (which is
  negative if \a t is in the past).

  Since QTime measures time within a day and there are 86400 seconds
  in a day, the result is between -86400 and 86400.

  \sa addSecs() QDateTime::secsTo()
*/

int QTime::secsTo( const QTime &t ) const
{
    return ((int)t.ds - (int)ds)/1000;
}

/*!
  Returns the time plus \e ms milliseconds.
*/

QTime QTime::addMSecs( int ms ) const
{
    QTime t;
    t.ds = (ds + ms) % MSECS_PER_DAY;
    return t;
}

/*!
  Returns the number of milliseconds between this time and \e t.
*/

int QTime::msecsTo( const QTime &t ) const
{
    return (int)t.ds - (int)ds;
}


/*!
  \fn bool QTime::operator==( const QTime &t ) const
  Returns TRUE if this time is equal to \e t, or FALSE if
  they are different.
*/

/*!
  \fn bool QTime::operator!=( const QTime &t ) const
  Returns TRUE if this time is different from \e t, or FALSE if
  they are equal.
*/

/*!
  \fn bool QTime::operator<( const QTime &t ) const
  Returns TRUE if this time is before \e t, otherwise FALSE.
*/

/*!
  \fn bool QTime::operator<=( const QTime &t ) const
  Returns TRUE if this time is before or equal to \e t, otherwise FALSE.
*/

/*!
  \fn bool QTime::operator>( const QTime &t ) const
  Returns TRUE if this time is after \e t, otherwise FALSE.
*/

/*!
  \fn bool QTime::operator>=( const QTime &t ) const
  Returns TRUE if this time is equal to or after \e t, otherwise FALSE.
*/



/*!
  Returns the current time.
*/

QTime QTime::currentTime()
{
    QTime ct;
    currentTime( &ct );
    return ct;
}

/*!
  Fetches the current time and returns TRUE if the time is within one
  minute after midnight, otherwise FALSE. The return value is used by
  QDateTime::currentDateTime() to ensure that the date there is correct.
*/

bool QTime::currentTime( QTime *ct )
{
    if ( !ct ) {
#if defined(CHECK_NULL)
	warning( "QTime::currentTime(QTime *): Null pointer not allowed" );
#endif
	return FALSE;
    }

#if defined(_OS_WIN32_)

    SYSTEMTIME t;
    GetLocalTime( &t );
    ct->ds = MSECS_PER_HOUR*t.wHour + MSECS_PER_MIN*t.wMinute +
	     1000*t.wSecond + t.wMilliseconds;
    return (t.wHour == 0 && t.wMinute == 0);

#elif defined(_OS_OS2_)

    DATETIME t;
    DosGetDateTime( &t );
    ct->ds = MSECS_PER_HOUR*t.hours + MSECS_PER_MIN*t.minutes +
	     1000*t.seconds + 10*t.hundredths;
    return (t.hours == 0 && t.minutes == 0);

#elif defined(_OS_MSDOS_)

    _dostime_t t;
    _dos_gettime( &t );
    ct->ds = MSECS_PER_HOUR*t.hour + MSECS_PER_MIN*t.minute +
	     t.second*1000 + t.hsecond*10;
    return (t.hour== 0 && t.minute == 0);

#elif defined(UNIX)

    struct timeval tv;
    gettimeofday( &tv, 0 );
    time_t ltime = tv.tv_sec;
    tm *t = localtime( &ltime );
    ct->ds = (uint)( MSECS_PER_HOUR*t->tm_hour + MSECS_PER_MIN*t->tm_min +
		     1000*t->tm_sec + tv.tv_usec/1000 );
    return (t->tm_hour== 0 && t->tm_min == 0);

#else

    time_t ltime;			// no millisecond resolution!!
    ::time( &ltime );
    tm *t = localtime( &ltime );
    ct->ds = MSECS_PER_HOUR*t->tm_hour + MSECS_PER_MIN*t->tm_min +
	     1000*t->tm_sec;
    return (t->tm_hour== 0 && t->tm_min == 0);
#endif
}

/*!
  Returns TRUE if the specified time is valid, otherwise FALSE.

  Example:
  \code
    QTime::isValid(21, 10, 30);		// returns TRUE
    QTime::isValid(22, 5,  62);		// returns FALSE
  \endcode
*/

bool QTime::isValid( int h, int m, int s, int ms )
{
    return (uint)h < 24 && (uint)m < 60 && (uint)s < 60 && (uint)ms < 1000;
}


/*!
  Sets the time to the current time, e.g. for timing:
  \code
    QTime t;
    t.start();				// start clock
    ... // some lengthy task
    debug( "%d\n", t.elapsed() );	// prints # msecs elapsed
  \endcode

  \sa restart(), elapsed()
*/

void QTime::start()
{
    *this = currentTime();
}

/*!
  Sets *this to the current time, and returns the number of
  milliseconds that have elapsed since the last start() or restart().

  restart is guaranteed to be atomic, and so is very handy for
  repeated measurements; call start() to start the first measurement,
  then restart() for each later measurement.

  Note that the counter wraps to zero 24 hours after the last call to
  start() or restart().

  \warning If the system's local time changes, the result is undefined.
  This can happen e.g. when daylight saving is turned on or off.

  \sa start(), elapsed()
*/

int QTime::restart()
{
    QTime t = currentTime();
    int n = msecsTo( t );
    if ( n < 0 )				// passed midnight
	n += 86400*1000;
    *this = t;
    return n;
}

/*!
  Returns the number of milliseconds that have elapsed since start() or
  restart() were called.

  Note that the counter wraps to zero 24 hours after the last call to
  start() or restart.

  \warning If the system's local time changes, the result is undefined.
  This can happen e.g. when daylight saving is turned on or off.

  \sa start(), restart()
*/

int QTime::elapsed()
{
    int n = msecsTo( currentTime() );
    if ( n < 0 )				// passed midnight
	n += 86400*1000;
    return n;
}


/*****************************************************************************
  QDateTime member functions
 *****************************************************************************/

/*!
  \class QDateTime qdatetime.h
  \brief The QDateTime class combines QDate and QTime into a single class.

  \ingroup time

  QDateTime provides high precision date and time functions since it can work
  with Gregorian dates up to about year 8000.

  Most countries that use the Gregorian calendar switched to it between 1550
  and 1920.

  \sa QDate, QTime
*/

/*!
  \fn QDateTime::QDateTime()
  Constructs a null datetime (i.e. null date and null time).  A null
  datetime is invalid, since the date is invalid.
*/

/*!
  Constructs a datetime with date \e date and null time (00:00:00.000).
*/

QDateTime::QDateTime( const QDate &date )
    : d(date)
{
}

/*!
  Constructs a datetime with date \e date and time \e time.
*/

QDateTime::QDateTime( const QDate &date, const QTime &time )
    : d(date), t(time)
{
}


/*!
  \fn bool QDateTime::isNull() const
  Returns TRUE if both the date and the time are null.	A null date is invalid.
  \sa QDate::isNull(), QTime::isNull()
*/

/*!
  \fn bool QDateTime::isValid() const
  Returns TRUE if both the date and the time are value.
  \sa QDate::isValid(), QTime::isValid()
*/

/*!
  \fn QDate QDateTime::date() const
  Returns the date part of this datetime.
  \sa time()
*/

/*!
  \fn QTime QDateTime::time() const
  Returns the time part of this datetime.
  \sa date()
*/

/*!
  \fn void QDateTime::setDate( const QDate &date )
  Sets the date part of this datetime.
  \sa setTime()
*/

/*!
  \fn void QDateTime::setTime( const QTime &time )
  Sets the time part of this datetime.
  \sa setDate()
*/


/*!
  Sets the local date and time given the number of seconds that have passed
  since 00:00:00 on January 1, 1970, Coordinated Universal Time (UTC).
  On systems that do not support timezones this function will behave as if
  local time were UTC.
*/

void QDateTime::setTime_t( uint secsSince1Jan1970UTC )
{
    time_t tmp = (time_t) secsSince1Jan1970UTC;
    tm *tM = localtime( &tmp );
    if ( !tM ) {
	tM = localtime( 0 );
	if ( !tM ) {
#if defined(CHECK_NULL)
	    warning( "QDateTime::setTime_t: Cannot get localtime" );
#endif
	    return;
	}
    }
    d.jd = QDate::greg2jul( tM->tm_year + 1900, tM->tm_mon + 1, tM->tm_mday );
    t.ds = MSECS_PER_HOUR*tM->tm_hour + MSECS_PER_MIN*tM->tm_min +
	    1000*tM->tm_sec;
}


/*!
  Returns the datetime as a string.

  The string format is "Sat May 20 1995 03:40:13".
*/

QString QDateTime::toString() const
{
    QString buf;
    QString time = t.toString();
    buf.sprintf( "%s %s %d %s %d", d.dayName(d.dayOfWeek()),
		 d.monthName(d.month()), d.day(), (const char*)time, d.year());
    return buf;
}

/*!
  Returns the datetime plus \e ndays days.
  \sa daysTo()
*/

QDateTime QDateTime::addDays( int ndays ) const
{
    return QDateTime( d.addDays(ndays), t );
}

/*!
  Returns the datetime plus \e nsecs seconds.
  \sa secsTo()
*/

QDateTime QDateTime::addSecs( int nsecs ) const
{
    uint dd = d.jd;
    int  tt = t.ds;
    int  sign = 1;
    if ( nsecs < 0 ) {
	nsecs = -nsecs;
	sign = -1;
    }
    if ( nsecs >= (int)SECS_PER_DAY ) {
	dd += sign*(nsecs/SECS_PER_DAY);
	nsecs %= SECS_PER_DAY;
    }
    tt += sign*nsecs*1000;
    if ( tt < 0 ) {
	tt = MSECS_PER_DAY - tt - 1;
	dd -= tt / MSECS_PER_DAY;
	tt = tt % MSECS_PER_DAY;
	tt = MSECS_PER_DAY - tt - 1;
    } else if ( tt >= (int)MSECS_PER_DAY ) {
	dd += ( tt / MSECS_PER_DAY );
	tt = tt % MSECS_PER_DAY;
    }
    QDateTime ret;
    ret.t.ds = tt;
    ret.d.jd = dd;
    return ret;
}

/*!
  Returns the number of days from this datetime to \a dt, which is
  negative if \a dt is in the past.

  \sa addDays() secsTo()
*/

int QDateTime::daysTo( const QDateTime &dt ) const
{
    return d.daysTo( dt.d );
}

/*!
  Returns the number of seconds from this datetime to \a dt, which is
  negative if \a t is in the past.

  Example:
  \code
    QDateTime dt = QDateTime::currentDateTime();
    QDateTime x( QDate(dt.year(),12,24), QTime(17,00) );
    debug( "There are %d seconds to Christmas", dt.secsTo(x) );
  \endcode

  \sa addSecs() daysTo() QTime::secsTo()
*/

int QDateTime::secsTo( const QDateTime &dt ) const
{
    return t.secsTo(dt.t) + d.daysTo(dt.d)*SECS_PER_DAY;
}


/*!
  Returns TRUE if this datetime is equal to \e dt, or FALSE if
  they are different.
  \sa operator!=()
*/

bool QDateTime::operator==( const QDateTime &dt ) const
{
    return  t == dt.t && d == dt.d;
}

/*!
  Returns TRUE if this datetime is different from \e dt, or FALSE if
  they are equal.
  \sa operator==()
*/

bool QDateTime::operator!=( const QDateTime &dt ) const
{
    return  t != dt.t || d != dt.d;
}

/*!
  Returns TRUE if this datetime is before \e dt, otherwise FALSE.
*/

bool QDateTime::operator<( const QDateTime &dt ) const
{
    if ( d < dt.d )
	return TRUE;
    return d == dt.d ? t < dt.t : FALSE;
}

/*!
  Returns TRUE if this datetime is before or equal to \e dt, otherwise
  FALSE.
*/

bool QDateTime::operator<=( const QDateTime &dt ) const
{
    if ( d < dt.d )
	return TRUE;
    return d == dt.d ? t <= dt.t : FALSE;
}

/*!
  Returns TRUE if this datetime is after \e dt, otherwise FALSE.
*/

bool QDateTime::operator>( const QDateTime &dt ) const
{
    if ( d > dt.d )
	return TRUE;
    return d == dt.d ? t > dt.t : FALSE;
}

/*!
  Returns TRUE if this datetime is equal to or after \e dt, otherwise
  FALSE.
*/

bool QDateTime::operator>=( const QDateTime &dt ) const
{
    if ( d > dt.d )
	return TRUE;
    return d == dt.d ? t >= dt.t : FALSE;
}

/*!
  Returns the current datetime.
  \sa QDate::currentDate(), QTime::currentTime()
*/

QDateTime QDateTime::currentDateTime()
{
    QDate cd = QDate::currentDate();
    QTime ct;
    if ( QTime::currentTime(&ct) )		// too close to midnight?
	cd = QDate::currentDate();		// YES! time for some midnight
						// voodoo, fetch date again
    return QDateTime( cd, ct );
}


/*****************************************************************************
  Date/time stream functions
 *****************************************************************************/

/*!
  \relates QDate
  Writes the date to the stream.

  Serialization format: [Q_UINT32], Julian day.
*/

QDataStream &operator<<( QDataStream &s, const QDate &d )
{
    return s << (Q_UINT32)(d.jd);
}

/*!
  \relates QDate
  Reads a date from the stream.
*/

QDataStream &operator>>( QDataStream &s, QDate &d )
{
    Q_UINT32 jd;
    s >> jd;
    d.jd = jd;
    return s;
}

/*!
  \relates QTime
  Writes a time to the stream.

  Serialization format: [Q_UINT32], milliseconds since midnight.
*/

QDataStream &operator<<( QDataStream &s, const QTime &t )
{
    return s << (Q_UINT32)(t.ds);
}

/*!
  \relates QTime
  Reads a time from the stream.
*/

QDataStream &operator>>( QDataStream &s, QTime &t )
{
    Q_UINT32 ds;
    s >> ds;
    t.ds = ds;
    return s;
}

/*!
  \relates QDateTime
  Writes a datetime to the stream.

  Serialization format: [QDate QTime].
*/

QDataStream &operator<<( QDataStream &s, const QDateTime &dt )
{
    return s << dt.d << dt.t;
}

/*!
  \relates QDateTime
  Reads a datetime from the stream.
*/

QDataStream &operator>>( QDataStream &s, QDateTime &dt )
{
    s >> dt.d >> dt.t;
    return s;
}
