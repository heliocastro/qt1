/****************************************************************************
** $Id: qevent.cpp,v 2.18.2.5 1999/01/13 18:33:39 ettrich Exp $
**
** Implementation of event classes
**
** Created : 931029
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

#include "qevent.h"

void qRemovePostedEvent( QEvent * );		// defined in qapplication_xxx.cpp

void QEvent::peErrMsg()				// posted event error message
{
#if defined(CHECK_STATE)
    const char *n = 0;
    switch ( t ) {				// convert type to msg string
	case Event_Timer:
	    n = "Timer";
	    break;
	case Event_MouseButtonPress:
	    n = "MouseButtonPress";
	    break;
	case Event_MouseButtonRelease:
	    n = "MouseButtonRelease";
	    break;
	case Event_MouseButtonDblClick:
	    n = "MouseButtonDblClick";
	    break;
	case Event_MouseMove:
	    n = "MouseMove";
	    break;
	case Event_KeyPress:
	    n = "KeyPress";
	    break;
	case Event_KeyRelease:
	    n = "KeyRelease";
	    break;
	case Event_FocusIn:
	    n = "FocusIn";
	    break;
	case Event_FocusOut:
	    n = "FocusOut";
	    break;
	case Event_Enter:
	    n = "Enter";
	    break;
	case Event_Leave:
	    n = "Leave";
	    break;
	case Event_Paint:
	    n = "Paint";
	    break;
	case Event_Move:
	    n = "Move";
	    break;
	case Event_Resize:
	    n = "Resize";
	    break;
	case Event_Create:
	    n = "Create";
	    break;
	case Event_Destroy:
	    n = "Destroy";
	    break;
	case Event_Close:
	    n = "Close";
	    break;
	case Event_Quit:
	    n = "Quit";
	    break;
    }
    if ( n )
	warning( "QEvent: Posted event %s cannot be stack variable, ignored",
		 n );
    else
	warning( "QEvent: Posted event %d cannot be stack variable, ignored",
		 t );
#endif
    qRemovePostedEvent( this );
}


/*!
  \class QEvent qevent.h
  \brief The QEvent class is base class of all
  event classes. Event objects contain event parameters.

  \ingroup event
  \ingroup kernel

  The \link QApplication::exec() main event loop\endlink of Qt fetches
  native window system events from the event queue, translates them
  into QEvent and sends the translated events to QObjects.

  Generally, events come from the underlying window system, but it is
  also possible to manually send events through the QApplication class
  using QApplication::sendEvent() and QApplication::postEvent().

  QObject received events by having its QObject::event() function
  called.  The default implementation simply calls
  QObject::timerEvent() for timer events and ignores all other events.
  QWidget reimplements \link QWidget::event() event() \endlink and
  dispatches incoming events to various event handles such as
  QWidget::keyPressEvent() and QWidget::mouseMoveEvent().

  The basic QEvent only contains an event type parameter.  Subclasses
  of QEvent contain additional parameters that descripe the particular
  event.

  \sa QObject::event() QObject::installEventFilter() QWidget::event()
  QApplication::sendEvent() QApplcation::postEvent()
  QApplication::processEvents()
*/

/*!
  \fn QEvent::QEvent( int type )
  Contructs an event object with a \a type. The file qevent.h lists
  all event types.
*/

/*!
  \fn QEvent::~QEvent()
  Destroys the event.  Reports an error if the event has been
  \link QApplication::postEvent() posted. \endlink
*/

/*!
  \fn int QEvent::type() const
  Returns the event type.
*/


/*!
  \class QTimerEvent qevent.h
  \brief The QTimerEvent class contains parameters that describe a
  timer event.

  \ingroup event


  Timer events are sent at regular intervals to objects that have
  started one or more timers.  Each timer has a unique identifier.

  If interval is 0, then the timer event occurs
  once every time there are no more window system events to process.

  The QTimer class provides a high-level programming interface with one-shot
  timers and timer signals instead of events.

  The event handler QObject::timerEvent() receives timer events.

  \sa QTimer, QObject::timerEvent(), QObject::startTimer(),
  QObject::killTimer(), QObject::killTimers()
*/

/*!
  \fn QTimerEvent::QTimerEvent( int timerId )
  Constructs a timer event object with the timer identifier set to \a timerId.
*/

/*!
  \fn int QTimerEvent::timerId() const
  Returns the unique timer identifier, which is the same identifier as
  returned from QObject::startTimer().
*/


/*!
  \class QMouseEvent qevent.h

  \brief The QMouseEvent class contains parameters that describe a mouse event.

  \ingroup event


  Mouse events occur when a mouse button is pressed or released inside a
  widget, or when the mouse cursor is moved.

  Mouse move events will only occur when some mouse button is pressed down,
  unless \link QWidget::setMouseTracking() mouse tracking\endlink has been
  enabled.

  Qt makes an automatic mouse grab when a mouse button is pressed inside a
  widget, and the widget will continue to receive mouse events until the
  last mouse button is released.

  The QWidget::setEnable() function can be used to enable or disable mouse
  and keyboard events for a widget.

  The QCursor widget has static functions for reading and setting the
  position of the mouse cursor.

  The event handlers QWidget::mousePressEvent(), QWidget::mouseReleaseEvent(),
  QWidget::mouseDoubleClickEvent() and QWidget::mouseMoveEvent() receive
  mouse events.

  \sa QWidget::setMouseTracking(), QWidget::grabMouse()
*/

/*!
  \fn QMouseEvent::QMouseEvent( int type, const QPoint &pos, int button, int state )

  Constructs a mouse event object.

  The type parameter must be \c Event_MouseButtonPress,
  \c Event_MouseButtonRelease,
  \c Event_MouseButtonDblClick or \c Event_MouseMove.
*/

/*!
  \fn const QPoint &QMouseEvent::pos() const
  Returns the position of the mouse pointer, relative to the widget that
  received the event.
  \sa x(), y()
*/

/*!
  \fn const QPoint &QMouseEvent::globalPos() const
  Returns the global position of the mouse pointer at the time of the event
  \sa globalX(), globalY()
*/

/*!
  \fn int QMouseEvent::x() const
  Returns the X position of the mouse pointer, relative to the widget that
  received the event.
  \sa y(), pos()
*/

/*!
  \fn int QMouseEvent::y() const
  Returns the Y position of the mouse pointer, relative to the widget that
  received the event.
  \sa x(), pos()
*/

/*!
  \fn int QMouseEvent::globalX() const
  Returns the global X position of the mouse pointer at the time of the event
  \sa globalY(), globalPos()
*/

/*!
  \fn int QMouseEvent::globalY() const
  Returns the global Y position of the mouse pointer at the time of the event
  \sa globalX(), globalPos()
*/

#if (QT_VERSION > 199)
#error "non-static in 2.0"
#endif
QPoint QMouseEvent::g;

/*!
  \fn int QMouseEvent::button() const
  Returns the button that caused the event.

  Possible return values are \c LeftButton, \c RightButton, \c MidButton and
  \c NoButton.

  Note that the returned value is always \c NoButton (0) for mouse move
  events.

  \sa state()
*/


/*!
  \fn int QMouseEvent::state() const

  Returns the button state (a combination of mouse buttons and keyboard
  modifiers), i.e. what buttons and keys were being held depressed
  immediately before the event was generated.

  Note that this means that for \c Event_MouseButtonPress and \c
  Event_MouseButtonDblClick, the flag for the button() itself will not be
  set in the state; while for \c Event_MouseButtonRelease, it will.

  The returned value is \c LeftButton, \c RightButton, \c MidButton,
  \c ShiftButton, \c ControlButton and \c AltButton OR'ed together.

  \sa button() */


/*!
  \class QKeyEvent qevent.h
  \brief The QKeyEvent class contains parameters that describe a key event.

  \ingroup event


  Key events occur when a key is pressed or released when a widget has
  keyboard input focus.

  A key event contains a special accept flag which tells whether the
  receiver wants the key.  You should call QKeyEvent::ignore() if the
  key press or release event is not handled by your widget.

  The QWidget::setEnable() function can be used to enable or disable mouse
  and keyboard events for a widget.

  The event handlers QWidget::keyPressEvent() and QWidget::keyReleaseEvent()
  receive key events.

  \sa QFocusEvent, QWidget::grabKeyboard()
*/

/*!
  \fn QKeyEvent::QKeyEvent( int type, int key, int ascii, int state )
  Constructs a key event object.

  The \a type parameter must be \c Event_KeyPress or \c Event_KeyRelease.

  If \a key is 0, the event is not a result of a known key (e.g. it
  may be the result of a compose sequence or keyboard macro).

  The accept flag is set to TRUE.
*/

/*!
  \fn int QKeyEvent::key() const
  Returns the code of the key that was pressed or released.

  The header file qkeycode.h lists the possible keyboard codes.	 These codes
  are independent of the underlying window system.

  Key code 0 means that the event is not a result of a known key (e.g. it
  may be the result of a compose sequence or keyboard macro).
*/

/*!
  \fn int QKeyEvent::ascii() const
  Returns the ASCII code of the key that was pressed or released.
*/

/*!
  \fn int QKeyEvent::state() const
  Returns the keyboard modifier flags.

  The returned value is \c ShiftButton, \c ControlButton and \c AltButton
  OR'ed together.
*/

/*!
  \fn bool QKeyEvent::isAccepted() const
  Returns TRUE if the receiver of the event wants to keep the key.
*/

/*!
  \fn void QKeyEvent::accept()
  Sets the accept flag of the key event object.

  Setting the accept parameter indicates that the receiver of the event wants
  the key event. Unwanted key events are sent to the parent widget.

  The accept flag is set by default.

  \sa ignore()
*/

/*!
  \fn void QKeyEvent::ignore()
  Clears the accept flag parameter of the key event object.

  Clearing the accept parameter indicates that the event receiver does
  not want the key event. Unwanted key events are sent to the parent widget.

  The accept flag is set by default.

  \sa accept()
*/


/*!
  \class QFocusEvent qevent.h
  \brief The QFocusEvent class contains event parameters for widget focus
  events.

  \ingroup event


  Focus events are sent to widgets when the keyboard input focus changes.

  The event handlers QWidget::focusInEvent() and QWidget::focusOutEvent()
  receive focus events.

  \sa QWidget::setFocus(), QWidget::setFocusEnabled()
*/

/*!
  \fn QFocusEvent::QFocusEvent( int type )
  Constructs a focus event object.

  The \a type parameter must be either \a Event_FocusIn or \a Event_FocusOut.
*/

/*!
  \fn bool QFocusEvent::gotFocus() const
  Returns TRUE if the widget received the text input focus.
*/

/*!
  \fn bool QFocusEvent::lostFocus() const
  Returns TRUE if the widget lost the text input focus.
*/


/*!
  \class QPaintEvent qevent.h
  \brief The QPaintEvent class contains event parameters for paint events.

  \ingroup event


  Paint events are sent to widgets that need to update themselves, for instance
  when a part of a widget is exposed because an overlying widget is moved away.

  The event handler QWidget::paintEvent() receives paint events.

  \sa QPainter, QWidget::update(), QWidget::repaint()
*/

/*!
  \fn QPaintEvent::QPaintEvent( const QRect &paintRect )
  Constructs a paint event object with the rectangle that should be updated.
*/

/*!
  \fn const QRect &QPaintEvent::rect() const
  Returns the rectangle that should be updated.
*/


/*!
  \class QMoveEvent qevent.h
  \brief The QMoveEvent class contains event parameters for move events.

  \ingroup event


  Move events are sent to widgets that have been moved to a new position
  relative to their parent.

  The event handler QWidget::moveEvent() receives move events.

  \sa QWidget::move(), QWidget::setGeometry()
*/

/*!
  \fn QMoveEvent::QMoveEvent( const QPoint &pos, const QPoint &oldPos )
  Constructs a move event with the new and old widget positions.
*/

/*!
  \fn const QPoint &QMoveEvent::pos() const
  Returns the new position of the widget, which is the same as
  QWidget::pos().
*/

/*!
  \fn const QPoint &QMoveEvent::oldPos() const
  Returns the old position of the widget.
*/


/*!
  \class QResizeEvent qevent.h
  \brief The QResizeEvent class contains event parameters for resize events.

  \ingroup event


  Resize events are sent to widgets that have been resized.

  The event handler QWidget::resizeEvent() receives resize events.

  \sa QWidget::resize(), QWidget::setGeometry()
*/

/*!
  \fn QResizeEvent::QResizeEvent( const QSize &size, const QSize &oldSize )
  Constructs a resize event with the new and old widget sizes.
*/

/*!
  \fn const QSize &QResizeEvent::size() const
  Returns the new size of the widget, which is the same as
  QWidget::size().
*/

/*!
  \fn const QSize &QResizeEvent::oldSize() const
  Returns the old size of the widget.
*/


/*!
  \class QCloseEvent qevent.h
  \brief The QCloseEvent class contains parameters that describe a close event.

  \ingroup event


  Close events are sent to widgets that the user wants to close, usually
  by choosing "Close" from the window menu. They are also sent when you
  call QWidget::close() to close a widget from inside the program.

  Close events contain a special accept flag which tells whether the
  receiver wants the widget to be closed.  When a widget accepts the close
  event, it is \link QWidget::hide() hidden\endlink. If it refuses to
  accept the close event, nothing happens.

  The \link QApplication::setMainWidget() main widget\endlink of the
  application is a special case. When it accepts the close event, the
  application is immediately \link QApplication::quit()
  terminated\endlink.

  The event handler QWidget::closeEvent() receives close events.

  The default implementation of this event handler accepts the close event.
  This makes Qt \link QWidget::hide() hide\endlink the widget.

  \code
    void QWidget::closeEvent( QCloseEvent *e )
    {
	e->accept();				// hides the widget
    }
  \endcode

  If you do not want your widget to be hidden, you should reimplement the
  event handler.

  \code
    void MyWidget::closeEvent( QCloseEvent *e )
    {
	e->ignore();				// does not hide the widget
    }
  \endcode

  If you want your widget to be deleted when it is closed, simply delete
  it in the close event. In this case, calling QCloseEvent::accept() or
  QCloseEvent::ignore() makes no difference.

  \warning Be careful.  The code below assumes that the widget was created
  on the heap using the \c new operator. Even when the widget has been
  created by new doing this is a tricky opreation. Be sure that you cannot
  have any other pointers to the widget hanging around.

  \code
    void MyWidget::closeEvent( QCloseEvent * )
    {
	delete this;
    }
  \endcode

  QObject emits the \link QObject::destroyed() destroyed()\endlink signal
  when it is deleted.  This is a useful signal if a widget needs to know
  when another widget is deleted.

  \sa QWidget::close(), QWidget::hide(), QObject::destroyed(),
  QApplication::setMainWidget(), QApplication::quit()
*/

/*!
  \fn QCloseEvent::QCloseEvent()
  Constructs a close event object with the accept parameter flag set to FALSE.
*/

/*!
  \fn bool QCloseEvent::isAccepted() const
  Returns TRUE if the receiver of the event has agreed to close the widget.
  \sa accept(), ignore()
*/

/*!
  \fn void QCloseEvent::accept()
  Sets the accep flag of the close event object.

  Setting the accept flag indicates that the receiver of this event agrees
  to close the widget.

  The accept flag is not set by default.

  If you choose to accept in QWidget::closeEvent(), the widget will be
  hidden.

  \sa ignore(), QWidget::hide()
*/

/*!
  \fn void QCloseEvent::ignore()
  Clears the accept flag of the close event object.

  Clearing the accept flag indicates that the receiver of this event does not
  want the widget to be hidden.

  The accept flag is not set by default.

  \sa accept()
*/


/*!
  \class QChildEvent qevent.h
  \brief The QChildEvent class contains event parameters for child widget
  events.

  \ingroup event


  Child events are sent to widgets when children are inserted or removed.

  In this release of Qt, no event handlers are defined to receive
  child events. Reimplement QObject::event() or install an
  QObject::eventFilter() to handle these events.

*/

#if QT_VERSION >= 200
#error "Implement child event handlers"
#endif

/*!
  \fn QChildEvent::QChildEvent( int type, QWidget *child )
  Constructs a child event object.

  The \a type parameter must be either \a Event_ChildInserted
  or \a Event_ChildRemoved.
*/

/*!
  \fn QWidget *QChildEvent::child() const
  Returns the child widget inserted or removed.
*/

/*!
  \fn bool QChildEvent::inserted() const
  Returns TRUE if the widget received a new child.
*/

/*!
  \fn bool QChildEvent::removed() const
  Returns TRUE if the widget lost a child.
*/




/*!
  \class QCustomEvent qevent.h
  \brief The QCustomEvent class provides support for custom events.

  \ingroup event


  QCustomEvent is a user-defined event type which contains a \c void*.

  \warning
  This event class is internally used to implement Qt enhancements.  It is
  not advisable to use QCustomEvent in normal applications, where other
  event types and the signal/slot mechanism can do the job.
*/

/*!
  \fn QCustomEvent::QCustomEvent( int type, void *data )
  Constructs a custom event object with the event type \a type and a
  pointer to \a data.
*/

/*!
  \fn void *QCustomEvent::data() const
  Returns a pointer to the event data (specified in the constructor).

  The event data can be anything and must be casted to something useful
  based on the \link type() event type\endlink. Again, it is not
  recommended to use custom events unless you are implementing Qt kernel
  enhancements.
*/

/*!
  \fn QDragMoveEvent::QDragMoveEvent( const QPoint& pos )

  Creates a QDragMoveEvent for which the mouse is at point \a pos.

  Note that internal state is also involved with QDragMoveEvent,
  so it is not useful to create these yourself.
*/

/*!
  \fn QDragMoveEvent::QDragMoveEvent( const QPoint& pos, int type )

  Creates a QDragMoveEvent for which the mouse is at point \a pos,
  and the given event \a type.

  Note that internal state is also involved with QDragMoveEvent,
  so it is not useful to create these yourself.
*/

/*!
  \fn const QPoint& QDragMoveEvent::pos() const

  Returns the position of the mouse when the event occurred.
*/

/*!
  \fn bool   QDragMoveEvent::isAccepted() const

  Returns TRUE if accept() has been called.
*/

/*!
  \fn void   QDragMoveEvent::accept()

  Call this to indicate that the event provides data which your widget
  can process.  Use provides(), or preferrably, the canDecode() methods
  of existing QDragObject subclasses, such as
  QTextDrag::canDecode(), or your own subclasses.
*/

/*!
  \fn void   QDragMoveEvent::ignore()

  The opposite of accept().
*/

/*!
  \fn void   QDragMoveEvent::accept( const QRect & r )

  The same as accept() above, but also notifies that future moves will
  also be acceptable if they remain withing the rectangle \a r on the
  widget - this can improve performance, but may also be ignored by
  the underlying system.
*/

/*!
  \fn void   QDragMoveEvent::ignore( const QRect & r)

  The opposite of accept(const QRect&).
*/

/*!
  \fn QRect  QDragMoveEvent::answerRect() const { return rect; }

  Returns the rectangle for which the acceptance of
  the move event applies.
*/



/*!
  \fn QDropEvent::QDropEvent( const QPoint& pos )

  Creates a QDropEvent for which the mouse is at point \a pos.

  Note that internal state is also involved with QDropEvent,
  so it is not useful to create these yourself.
*/

/*!
  \fn void   QDropEvent::accept()

  Call this to indicate that the event provided data which your widget
  processed.  Use data(), or preferrably, the decode() methods
  of existing QDragObject subclasses, such as
  QTextDrag::decode(), or your own subclasses.
*/

/*!
  \fn void   QDropEvent::ignore()

  The opposite of accept().
*/
