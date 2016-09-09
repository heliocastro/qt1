/****************************************************************************
** $Id: qsocketnotifier.cpp,v 2.8 1998/07/03 00:09:41 hanord Exp $
**
** Implementation of QSocketNotifier class
**
** Created : 951114
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

#include "qsocketnotifier.h"
#include "qevent.h"


extern bool qt_set_socket_handler( int, int, QObject *, bool );


/*!
  \class QSocketNotifier qsocketnotifier.h
  \brief The QSocketNotifer class provides support for socket callbacks.

  \ingroup kernel
  \ingroup io

  This class makes it possible to write e.g. asynchronous TCP/IP
  socket-based code in Qt.  Using synchronous socket operations blocks
  the program, which is clearly not acceptable for an event-based GUI
  program.

  Once you have opened a non-blocking socket (either for TCP, UDP, a
  unix-domain socket, or any other protocol family your operating
  system supports), you can create a socket notifier to monitor the
  socket.  Then connect the activated() signal to the slot you want to
  be called when a socket event occurs.

  There are three types of socket notifiers (read, write and exception)
  and you must specify one of these in the constructor.

  The type specifies when the activated() signal is to be emitted:
  <ol>
  <li> \c QSocketNotifier::Read: There is data to be read (socket read event).
  <li> \c QSocketNotifier::Write: Data can be written (socket write event).
  <li> \c QSocketNofifier::Exception: An exception has ocurred (socket
  exception event).  We recommend against using this.
  </ol>

  For example, if you need to monitor both reads and writes for the same
  socket, you must create two socket notifiers.

  Example:
  \code
    int sockfd;					// socket identifier
    struct sockaddr_in sa;			// should contain host address
    sockfd = socket( AF_INET, SOCK_STREAM, 0 ); // create TCP socket
    // make the socket non-blocking here, usually using fcntl( O_NONBLOCK )
    ::connect( sockfd, (struct sockaddr*)&sa,	// connect to host
	       sizeof(sa) );			//   NOT QObject::connect()!
    QSocketNotifier *sn;
    sn = new QSocketNotifier( sockfd, QSocketNotifier::Read, parent );
    QObject::connect( sn, SIGNAL(activated(int)),
		      myObject, SLOT(dataReceived()) );
  \endcode

  The optional \a parent argument can be set to make the socket notifier a
  child of some widget and therefore be automatically destroyed when the
  widget is destroyed.

  For read notifiers, it makes little sense to connect the activated()
  signal to more than one slot, because the data can be read from the
  socket only once.

  Make sure to disable the socket notifier for write operations when
  there is nothing to be written, otherwise the notifier fires on
  every pass of the main event loop.  The socket notifier is enabled
  when it is created.

  Also observe that if you do not read all the available data when the
  read notifier fires, it fires again and again.

  If you disable the read notifier, your program may deadlock.	Avoid
  it if you do not know what you are doing.  (The same applies to
  exception notifiers if you have to use that, for instance if you \e
  have to use TCP urgent data.)

  If you need a time-out for your sockets, you can use either
  \link QObject::startTimer() timer events\endlink or the QTimer class.

  Socket action is detected in the \link QApplication::exec() main event
  loop\endlink of Qt.  The X11 version of Qt has has a single UNIX
  select() call which incorporates all socket notifiers and the X socket.

  Note that on XFree86 for OS/2, select() only works in the thread in
  which main() is running, therefore you should use that thread for GUI
  operations.
*/


/*!
  Constructs a socket notifier with a \e parent and a \e name.

  \arg \e socket is the socket to be monitored.
  \arg \e type specifies the socket operation to be detected;
    \c QSocketNotifier::Read, \c QSocketNotifier::Write or
    \c QSocketNotifier::Exception.

  The \e parent and \e name arguments are sent to the QObject constructor.

  The socket notifier is initially enabled.  It is generally advisable to
  explicitly enable or disable it, especially for write notifiers.

  \sa setEnabled(), isEnabled()
*/

QSocketNotifier::QSocketNotifier( int socket, Type type, QObject *parent,
				  const char *name )
    : QObject( parent, name )
{
#if defined(CHECK_RANGE)
    if ( socket < 0 )
	warning( "QSocketNotifier: Invalid socket specified" );
#endif
    sockfd = socket;
    sntype = type;
    snenabled = TRUE;
    qt_set_socket_handler( sockfd, sntype, this, TRUE );
}

/*!
  Destroys the socket notifier.
*/

QSocketNotifier::~QSocketNotifier()
{
    setEnabled( FALSE );
}


/*!
  \fn void QSocketNotifier::activated( int socket )

  This signal is emitted under certain conditions, specified by the
  notifier \link type() type\endlink:
  <ol>
  <li> \c QSocketNotifier::Read: There is data to be read (socket read event).
  <li> \c QSocketNotifier::Write: Data can be written (socket write event).
  <li> \c QSocketNofifier::Exception: An exception has ocurred (socket
  exception event).
  </ol>

  The \e socket argument is the \link socket() socket\endlink identifier.

  \sa type(), socket()
*/


/*!
  \fn int QSocketNotifier::socket() const
  Returns the socket identifier specified to the constructor.
  \sa type()
*/

/*!
  \fn Type QSocketNotifier::type() const
  Returns the socket event type specified to the constructor;
  \c QSocketNotifier::Read, \c QSocketNotifier::Write or
  \c QSocketNotifier::Exception.
  \sa socket()
*/


/*!
  \fn bool QSocketNotifier::isEnabled() const
  Returns TRUE if the notifier is enabled, or FALSE if it is disabled.
  \sa setEnabled()
*/

/*!
  Enables the notifier if \e enable is TRUE, or disables it if \e enable is
  FALSE.

  The notifier is by default enabled.

  If the notifier is enabled, it emits the activated() signal whenever a
  socket event corresponding to its \link type() type\endlink occurs.  If
  it is disabled, it ignores socket events (the same effect as not creating
  the socket notifier).

  Disable the socket notifier for writes if there is nothing to be
  written, otherwise your program hogs the CPU.

  \sa isEnabled(), activated()
*/

void QSocketNotifier::setEnabled( bool enable )
{
    if ( sockfd < 0 )
	return;
    if ( snenabled == enable )			// no change
	return;
    snenabled = enable;
    qt_set_socket_handler( sockfd, sntype, this, snenabled );
}


/*!
  Handles events for the socket notifier object.

  Emits the activated() signal when a \c Event_SockAct is received.
*/

bool QSocketNotifier::event( QEvent *e )
{
    QObject::event( e );			// will activate filters
    if ( e->type() == Event_SockAct ) {
	emit activated( sockfd );
	return TRUE;
    }
    return FALSE;
}
