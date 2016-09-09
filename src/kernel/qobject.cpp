 /****************************************************************************
** $Id: qobject.cpp,v 2.52.2.5 1998/11/20 12:28:28 warwick Exp $
**
** Implementation of QObject class
**
** Created : 930418
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

#include "qobject.h"
#include "qobjectlist.h"
#include "qobjectdict.h"
#include "qsignalslotimp.h"
#include "qregexp.h"
#include <ctype.h>
#if defined(DEBUG)
#include "qapplication.h"
#endif

/*!
  \class QObject qobject.h
  \brief The QObject class is the base class of all Qt objects that can
  deal with signals, slots and events.

  \ingroup kernel

  Qt provides a very powerful mechanism for seamless object
  communication; \link metaobjects.html signal/slot
  connections\endlink. The signal/slot mechanism is an advanced way
  of making traditional callback routines.

  Example:
  \code
    //
    // The Mandelbrot class uses a QTimer to calculate the mandelbrot
    // set one scanline at a time without blocking the CPU.
    // It inherits QObject to use signals and slots.
    // Calling start() starts the calculation. The done() signal is
    // emitted when it has finished.
    // Note that this example is not complete. Feel free to complete it.
    //

    class Mandelbrot : public QObject
    {
	Q_OBJECT				// required for signals/slots
    public:
	Mandelbrot( QObject *parent=0, const char *name );
	...
    public slots:
	void	start();
    signals:
	void	done();
    private slots:
	void	calculate();
    private:
	QTimer	timer;
	...
    };

    //
    // Constructs and initializes a Mandelbrot object.
    //

    Mandelbrot::Mandelbrot( QObject *parent=0, const char *name )
	: QObject( parent, name )
    {
	connect( &timer, SIGNAL(timeout()), SLOT(calculate()) );
	...
    }

    //
    // Starts the calculation task. The internal calculate() slot
    // will be activated every 10 milliseconds.
    //

    void Mandelbrot::start()
    {
	if ( !timer.isActive() )		// not already running
	    timer.start( 10 );			// timeout every 10 ms
    }

    //
    // Calculates one scanline at a time.
    // Emits the done() signal when finished.
    //

    void Mandelbrot::calculate()
    {
	...			// perform the calculation for a scanline
	if ( finished ) {	// no more scanlines
	   timer.stop();
	   emit done();
	}
    }
  \endcode

  When an object has changed in some way that might be interesting for
  the outside world, it emits a signal to tell whoever is listening.
  All slots that are connected to this signal will be activated
  (called).  It is even possible to connect a signal directly to
  another signal.  (This will emit the second signal immediately
  whenever the first is emitted.)

  There is no limitation on how many slots that can be connected to
  a signal.  The slots will be activated in the order they were connected
  to the signal.

  Notice that the \c Q_OBJECT macro is mandatory for any object that
  implement signals or slots.  You also need to run the \link
  metaobjects.html moc program (Meta Object Compiler) \endlink on the
  source file.

  The signal/slot mechanism allows objects to easily reused, because
  the object that emits a signal does not need to know what the
  signals are connected to.

  All Qt widgets inherit QObject and use signals and slots.  A
  QScrollBar, for example, emits \link QScrollBar::valueChanged()
  valueChanged()\endlink whenever the scroll bar value changes.

  Meta objects are useful for doing more than connecting signals to slots.
  They also allow the programmer to obtain information about the class to
  which an object is instantiated from (see isA() and inherits()) or to
  produce a list of child objects that inherit a particular class
  (see queryList()).
*/


/*!
  \class QSenderObject qobject.h
  \brief Internal object used for sending signals.

  \internal

  It is generally a very bad idea to use this class directly in
  application programs.

  In particular, you cannot not use it to send signals from classes
  that do not inherit QObject. If you wish to do that, make an
  internal class that inherits QObject and has the necessary signals
  and slots.  Alternatively, you can use the QSignal class.
*/

/*!
  \fn void QSenderObject::setSender (QObject* s)

  Internal function, used in signal-slot connections.
*/


//
// Remove white space from SIGNAL and SLOT names.
// Internal for QObject::connect() and QObject::disconnect()
//

static inline bool isIdentChar( char x )
{						// Avoid bug in isalnum
    return x == '_' || (x >= '0' && x <= '9') ||
	 (x >= 'a' && x <= 'z') || (x >= 'A' && x <= 'Z');
}

static inline bool isSpace( char x )
{
#if defined(_CC_BOR_)
  /*
    Borland C++ 4.5 has a weird isspace() bug.
    isspace() usually works, but not here.
    This implementation is sufficient for our internal use: rmWS()
  */
    return (uchar)x <= 32;
#else
    return isspace( x );
#endif
}

static QString rmWS( const char *src )
{
    QString result( strlen(src)+1 );
    char *d = result.data();
    char *s = (char *)src;
    char last = 0;
    while( *s && isSpace(*s) )			// skip leading space
	s++;
    while ( *s ) {
	while ( *s && !isSpace(*s) )
	    last = *d++ = *s++;
	while ( *s && isSpace(*s) )
	    s++;
	if ( *s && isIdentChar(*s) && isIdentChar(last) )
	    last = *d++ = ' ';
    }
    result.truncate( (int)(d - result.data()) );
    int void_pos = result.find("(void)");
    if ( void_pos >= 0 )
	result.remove( void_pos+1, strlen("void") );
    return result;
}


// Event functions, implemented in qapp_xxx.cpp

int   qStartTimer( int interval, QObject *obj );
bool  qKillTimer( int id );
bool  qKillTimer( QObject *obj );

void  qRemovePostedEvents( QObject * );


#if !defined(QT_MAKEDLL)
Q_DECLARE(QDictM,QListM_QConnection);		// dict for multiple signals
Q_DECLARE(QDictIteratorM,QListM_QConnection);	// dict iterator
#endif

QMetaObject *QObject::metaObj = 0;


static void removeObjFromList( QObjectList *objList, const QObject *obj,
			       bool single=FALSE )
{
    if ( !objList )
	return;
    int index = objList->findRef( obj );
    while ( index >= 0 ) {
	objList->remove();
	if ( single )
	    return;
	index = objList->findNextRef( obj );
    }
}


/*!
  \relates QObject

  Returns a pointer to the child named \a name of QObject \a parent
  which inherits type \a type.

  Returns 0 if there is no such child.

  The CHILD macro (defined in qwindowdefs.h) does all this, and also
  casts the returt type to \a type *.

  Examples:
  \code
    QListBox * c = CHILD(myWidget,QListBox,"listboxname");
    if ( c )
        c->insertItem( "another string" );
  \endcode

  \code
    QListBox * c = (QListBox *)::qt_find_obj_child(myWidget,QListBox,
                                                   "listboxname");
    if ( c )
        c->insertItem( "another string" );
  \endcode
*/

void *qt_find_obj_child( QObject *parent, const char *type, const char *name )
{
    const QObjectList *list = parent->children();
    if ( list ) {
	QObjectListIt it( *list );
	QObject *obj;
	while ( (obj=it.current()) ) {
	    ++it;
	    if ( qstrcmp(name,obj->name()) == 0 &&
		obj->inherits(type) )
		return obj;
	}
    }
    return 0;
}


/*****************************************************************************
  QObject member functions
 *****************************************************************************/

/*!
  Constructs an object with the parent object \e parent and a \e name.

  The parent of an object may be viewed as the object's owner. For
  instance, a \link QDialog dialog box\endlink is the parent of the
  "ok" and "cancel" buttons inside it.

  The destructor of a parent object destroys all child objects.

  Setting \e parent to 0 constructs an object with no parent.
  If the object is a widget, it will become a top-level window.

  The object name is a text that can be used to identify this QObject.
  It is not very useful in the current version of Qt, but it will become
  increasingly important in the future.

  The queryList() function searches the object tree for objects that
  matches a particular object name.

  \sa parent(), name(), queryList()
*/

QObject::QObject( QObject *parent, const char *name )
{
    if ( !objectDict )				// will create object dict
	initMetaObject();
    objname = name ? qstrdup(name) : 0;		// set object name
    parentObj = parent;				// set parent
    childObjects = 0;				// no children yet
    connections = 0;				// no connections yet
    senderObjects = 0;				// no signals connected yet
    eventFilters = 0;				// no filters installed
    sigSender = 0;				// no sender yet
    isSignal   = FALSE;				// assume not a signal object
    isWidget   = FALSE;				// assume not a widget object
    hiPriority = FALSE;				// normal priority
    pendTimer  = FALSE;				// no timers yet
    pendEvent  = FALSE;				// no events yet
    blockSig   = FALSE;				// not blocking signals
    if ( parentObj )				// add object to parent
	parentObj->insertChild( this );
}

/*!
  Destroys the object and all its children objects.

  All signals to and from the object are automatically disconnected.

  \warning \e All child objects are deleted.  If any of these objects are
  on the stack or global, your program will sooner or later crash.  We do
  not recommend holding pointers to child objects from outside the parent.
  If you still do, the QObject::destroyed() signal gives you an
  opportunity to detect when an object is destroyed.
*/

QObject::~QObject()
{
    emit destroyed();
    if ( objname )
	delete [] objname;
    if ( pendTimer )				// might be pending timers
	qKillTimer( this );
    if ( pendEvent )				// pending posted events
	qRemovePostedEvents( this );
    if ( parentObj )				// remove it from parent object
	parentObj->removeChild( this );
    register QObject *obj;
    if ( senderObjects ) {			// disconnect from senders
	QObjectList *tmp = senderObjects;
	senderObjects = 0;
	obj = tmp->first();
	while ( obj ) {				// for all senders...
	    obj->disconnect( this );
	    obj = tmp->next();
	}
	delete tmp;
    }
    if ( connections ) {			// disconnect receivers
	QSignalDictIt it(*connections);
	QConnectionList *clist;
	while ( (clist=it.current()) ) {	// for each signal...
	    ++it;
	    register QConnection *c;
	    QConnectionListIt cit(*clist);
	    while( (c=cit.current()) ) {	// for each connected slot...
		++cit;
		if ( (obj=c->object()) )
		    removeObjFromList( obj->senderObjects, this );
	    }
	}
	delete connections;
	connections = 0;
    }
    if ( eventFilters ) {
	delete eventFilters;
	eventFilters = 0;
    }
    if ( childObjects ) {			// delete children objects
	QObjectListIt it(*childObjects);
	while ( (obj=it.current()) ) {
	    ++it;
	    obj->parentObj = 0;
	    delete obj;
	    if ( !childObjects )		// removeChild resets it
		break;
	}
	delete childObjects;
    }
}


/*!
  \fn QMetaObject *QObject::metaObject() const
  Returns a pointer to the meta object of this object.

  A meta object contains information about a class that inherits QObject:
  class name, super class name, signals and slots. Every class that contains
  the \c Q_OBJECT macro will also have a meta object.

  The meta object information is required by the signal/slot connection
  mechanism.  The functions isA() and inherits() also make use of the
  meta object.

  The meta object is created by the initMetaObject() function, which
  is generated by the meta object compiler and called from the QObject
  constructor.
*/

/*!
  Returns the class name of this object.

  This function is generated by the \link metaobjects.html Meta Object
  Compiler. \endlink

  \warning This function will return an invalid name if the class
  definition lacks the \c Q_OBJECT macro.

  \sa name(), inherits(), isA(), isWidgetType()
*/

const char *QObject::className() const
{
    return "QObject";
}


/*!
  Returns TRUE if this object is an instance of a specified class,
  otherwise FALSE.

  Example:
  \code
    QTimer *t = new QTimer;		// QTimer inherits QObject
    t->isA("QTimer");			// returns TRUE
    t->isA("QObject");			// returns FALSE
  \endcode

  \sa inherits(), metaObject()
*/

bool QObject::isA( const char *clname ) const
{
    QMetaObject *meta = queryMetaObject();
    return meta ? strcmp(clname,meta->className()) == 0 : FALSE;
}

/*!
  Returns TRUE if this object is an instance of a class that inherits
  \e clname, and \a clname inherits QObject.

  (A class is considered to inherit itself.)

  Example:
  \code
    QTimer *t = new QTimer;		// QTimer inherits QObject
    t->inherits("QTimer");		// returns TRUE
    t->inherits("QObject");		// returns TRUE
    t->inherits("QButton");		// returns FALSE

    QScrollBar * s = new QScrollBar;	// inherits QWidget and QRangeControl
    s->inherits( "QWidget" );		// returns TRUE
    s->inherits( "QRangeControl" ); 	// returns FALSE
  \endcode

  \sa isA(), metaObject()
*/

bool QObject::inherits( const char *clname ) const
{
    QMetaObject *meta = queryMetaObject();
    while ( meta ) {
	if ( strcmp(clname,meta->className()) == 0 )
	    return TRUE;
	meta = meta->superClass();
    }
    return FALSE;
}


/*!
  \fn const char *QObject::name() const

  Returns the name of this object, or 0 if the object does not have a
  name.

  The object name is set by the constructor or by the setName() function.
  The object name is not very useful in the current version of Qt, but
  will become increasingly important in the future.

  The queryList() function searches the object tree for objects that
  matches a particular object name.

  \sa setName(), className(), queryList()
*/

/*!
  \overload const char *QObject::name( const char * defaultName ) const

  Returns the name of this object, or \a defaultName if the object
  does not have a name.

  This version of name() is particularly useful for debug() calls such as:

  \code
    debug( "MyClass::setPrecision(): (%s) unable to set precision to %f",
	    name( "unnamed" ), newPrecision );
  \endcode

  name( "unnamed" ) is used here so that printf() (used in debug())
  will not be asked to output a null pointer.
*/

/*!
  Sets the name of this object to \e name.  The default name is the
  one assigned by the constructor.

  The object name is not very useful in the current version of Qt, but
  it will become increasingly important in the future.

  The queryList() function searches the object tree for objects that
  matches a particular object name.

  \sa name(), className(), queryList()
*/

void QObject::setName( const char *name )
{
    if ( objname )
	delete objname;
    objname = name ? qstrdup(name) : 0;
}

/*!
  \fn bool QObject::isWidgetType() const
  Returns TRUE if the object is a widget, or FALSE if not.

  Calling this function is equivalent to calling inherits("QWidget"),
  except that it is much faster.
*/

/*!
  \fn bool QObject::highPriority() const
  Returns TRUE if the object is a high priority object, or FALSE if it is a
  standard priority object.

  High priority objects are placed first in list of children,
  on the assumption that they will be referenced very often.
*/


/*!
  This virtual function receives events to an object and should return
  TRUE if the event was recognized and processed.

  The event() function can be reimplemented to customize the behavior of
  an object.

  \sa installEventFilter(), timerEvent(), QApplication::sendEvent(),
  QApplication::postEvent(), QWidget::event()
*/

bool QObject::event( QEvent *e )
{
#if defined(CHECK_NULL)
    if ( e == 0 )
	warning( "QObject::event: Null events are not permitted" );
#endif
    if ( eventFilters ) {			// try filters
	if ( activate_filters(e) )		// stopped by a filter
	    return TRUE;
    }
    if ( e->type() == Event_Timer ) {		// timer event
	timerEvent( (QTimerEvent*)e );
	return TRUE;
    }
    return FALSE;
}

/*!
  This event handler can be reimplemented in a subclass to receive
  timer events for the object.

  The default implementation does nothing.

  QTimer provides a higher-level interface to the timer functionality,
  and also more general information about timers.

  \sa startTimer(), killTimer(), killTimers(), event()
*/

void QObject::timerEvent( QTimerEvent * )
{
}


/*!
  Filters events if this object has been installed as an event filter for
  another object.

  The reimplementation of this virtual function must return TRUE if the
  event should be stopped, or FALSE if the event should be dispatched normally.

  \sa installEventFilter()
*/

bool QObject::eventFilter( QObject *, QEvent * )
{
    return FALSE;				// don't do anything with it
}


/*!
  \internal
  Activates all event filters for this object.
  This function is normally called from QObject::event() or QWidget::event().
*/

bool QObject::activate_filters( QEvent *e )
{
    if ( !eventFilters )			// no event filter
	return FALSE;
    QObjectListIt it( *eventFilters );
    register QObject *obj = it.current();
    while ( obj ) {				// send to all filters
	++it;					//   until one returns TRUE
	if ( obj->eventFilter(this,e) ) {
	    return TRUE;
	}
	obj = it.current();
    }
    return FALSE;				// don't do anything with it
}

/*!
  \fn bool QObject::signalsBlocked() const
  Returns TRUE if signals are blocked, or FALSE if signals are not blocked.

  Signals are not blocked by default.
  \sa blockSignals()
*/

/*!
  Blocks signals if \e block is TRUE, or unblocks signals if \e block is FALSE.

  Emitted signals disappear into hyperspace if signals are blocked.
*/

void QObject::blockSignals( bool block )
{
    blockSig = block;
}


//
// The timer flag hasTimer is set when startTimer is called.
// It is not reset when killing the timer because more than
// one timer might be active.
//

/*!
  Starts a timer and returns a timer identifier, or returns zero if
  it could not start a timer.

  A timer event will occur every \e interval milliseconds until
  killTimer() or killTimers() is called.  If \e interval is 0, then
  the timer event occurs once every time there are no more window system
  events to process.

  The virtual timerEvent() function is called with the QTimerEvent event
  parameter class when a timer event occurs.  Reimplement this function to
  get timer events.

  If multiple timers are running, the QTimerEvent::timerId() can be
  used to find out which timer was activated.

  Example:
  \code
    class MyObject : public QObject
    {
    public:
	MyObject( QObject *parent=0, const char *name=0 );
    protected:
	void  timerEvent( QTimerEvent * );
    };

    MyObject::MyObject( QObject *parent, const char *name )
	: QObject( parent, name )
    {
	startTimer( 50 );			// 50 millisec timer
	startTimer( 1000 );			// 1 second timer
	startTimer( 60000 );			// 1 minute timer
    }

    void MyObject::timerEvent( QTimerEvent *e )
    {
	debug( "timer event, id=%d", e->timerId() );
    }
  \endcode

  There is practically no upper limit for the interval value (more than
  one year).  The accuracy depends on the underlying operating system.
  Windows 95 has 55 millisecond (18.2 times per second) accuracy; other
  systems that we have tested (UNIX X11, Windows NT and OS/2) can
  handle 1 millisecond intervals.

  The QTimer class provides a high-level programming interface with
  one-shot timers and timer signals instead of events.

  \sa timerEvent(), killTimer(), killTimers()
*/

int QObject::startTimer( int interval )
{
    pendTimer = TRUE;				// set timer flag
    return qStartTimer( interval, (QObject *)this );
}

/*!
  Kills the timer with the identifier \e id.

  The timer identifier is returned by startTimer() when a timer event is
  started.

  \sa timerEvent(), startTimer(), killTimers()
*/

void QObject::killTimer( int id )
{
    qKillTimer( id );
}

/*!
  Kills all timers that this object has started.

  Using this function makes it harder to subclass your class (it kills
  timers started by subclasses as well as those started by you), so it
  is generally better to use killTimer().

  \sa timerEvent(), startTimer(), killTimer()
*/

void QObject::killTimers()
{
    qKillTimer( this );
}


static void objSearch( QObjectList *result,
		       QObjectList *list,
		       const char  *inheritsClass,
		       const char  *objName,
		       QRegExp	   *rx,
		       bool	    recurse )
{
    if ( !list || list->isEmpty() )		// nothing to search
	return;
    QObject *obj = list->first();
    while ( obj ) {
	bool ok = TRUE;
	if ( inheritsClass && !obj->inherits(inheritsClass) )
	    ok = FALSE;
	if ( ok ) {
	    if ( objName )
		ok = qstrcmp(objName,obj->name()) == 0;
	    else if ( rx )
		ok = rx->match(obj->name()) >= 0;
	}
	if ( ok )				// match!
	    result->append( obj );
	if ( recurse && obj->children() )
	    objSearch( result, (QObjectList *)obj->children(), inheritsClass,
		       objName, rx, recurse );
	obj = list->next();
    }
}


/*!
  \fn QObject *QObject::parent() const
  Returns a pointer to the parent object.
  \sa children()
*/

/*!
  \fn const QObjectList *QObject::children() const
  Returns a list of child objects, or 0 if this object has no children.

  The QObjectList class is defined in the qobjcoll.h header file.

  The latest child added is the \link QList::first() first\endlink object
  in the list and the first child is added is the \link QList::last()
  last\endlink object in the list.

  Note that the list order might change when \link QWidget widget\endlink
  children are \link QWidget::raise() raised\endlink or \link
  QWidget::lower() lowered\endlink. A widget that is raised becomes the
  last object in the list.  A widget that is lowered becomes the first
  object in the list.

  \sa queryList(), parent(), insertChild(), removeChild()
*/


/*!
  Returns a list of child objects found by a query.

  The query is specified by:
  \arg \e inheritsClass is the name of the base class that an object should
  inherit. Any class will be matched if \e inheritsClass is 0.
  \arg \e objName is the object name to search for. Any object name will be
  matched if \e objName is 0.
  \arg \e regexpMatch specifies whether \e objName is a regular expression
  (default) or not.
  \arg \e recursiveSearch must be \c TRUE (default) if you want to search
  the entire object tree, or \c FALSE if you want the search to traverse
  just the 1st level child objects of this object.

  Example:
  \code
    //
    // Sets a Courier 24 point fonts for all children in myWidget that
    // inherit QButton (i.e. QPushButton, QCheckBox, QRadioButton).
    //
    QObjectList	*list = myWidget->queryList( "QButton" );
    QObjectListIt it( *list );		// iterate over the buttons
    QFont newFont( "Courier", 24 );
    QObject * obj;
    while ( (obj=it.current()) != 0 ) {	// for each found object...
	++it;
	((QButton*)obj)->setFont( newFont );
    }
    delete list;			// delete the list, not the objects
  \endcode

  The QObjectList class is defined in the qobjcoll.h header file.

  \warning
  Delete the list away as soon you have finished using it.
  You can get in serious trouble if you for instance try to access
  an object that has been deleted.

  \sa children(), parent(), inherits(), name(), QRegExp
*/

QObjectList *QObject::queryList( const char *inheritsClass,
				 const char *objName,
				 bool regexpMatch,
				 bool recursiveSearch )
{
    QObjectList *list = new QObjectList;
    CHECK_PTR( list );
    if ( regexpMatch && objName ) {		// regexp matching
	QRegExp rx = objName;
	objSearch( list, (QObjectList *)children(), inheritsClass,
		   0, &rx, recursiveSearch );
    } else {
	objSearch( list, (QObjectList *)children(), inheritsClass,
		   objName, 0, recursiveSearch );
    }
    return list;
}


/*!
  Returns a list of objects/slot pairs that are connected to the
  signal, or 0 if nothing is connected to it.

  This function is for internal use.
*/

QConnectionList *QObject::receivers( const char *signal ) const
{
    if ( connections && signal ) {
	if ( *signal == '2' ) {			// tag == 2, i.e. signal
	    QString s = rmWS( signal+1 );
	    return connections->find( s );
	} else {
	    return connections->find( signal );
	}
    }
    return 0;
}


#if QT_VERSION == 200
#error "insertChild and removeChild should be virtual."
#endif

/*!
  Inserts an object \e obj into the list of child objects.

  \warning This function cannot be used to make a widget a child
  widget of another.  Child widgets can be created only by setting the
  parent widget in the constructor or by calling QWidget::recreate().

  \sa removeChild(), QWidget::recreate()
*/

void QObject::insertChild( QObject *obj )
{
    if ( obj->parentObj ) {
#if defined(CHECK_STATE)
	if ( obj->parentObj != this && obj->isWidgetType() )
	    warning( "QObject::insertChild: Cannot reparent a widget, "
		     "use QWidget::recreate() instead" );
#endif
	obj->parentObj->removeChild( obj );
    }

    if ( !childObjects ) {
	childObjects = new QObjectList;
	CHECK_PTR( childObjects );
    }
#if defined(CHECK_STATE)
    else if ( childObjects->findRef(obj) >= 0 ) {
	warning( "QObject::insertChild: Object %s::%s already in list",
		 obj->className(), obj->name( "unnamed" ) );
	return;
    }
#endif
    obj->parentObj = this;
    if ( obj->hiPriority )
	childObjects->insert( 0, obj );		// high priority inserts
    else
	childObjects->append( obj );		// normal priority appends
}

/*!
  Removes the child object \e obj from the list of children.

  \warning
  This function will not remove a child widget from the screen.
  It will only remove it from the parent widget's list of children.

  \sa insertChild(), QWidget::recreate()
*/

void QObject::removeChild( QObject *obj )
{
    if ( childObjects && childObjects->removeRef(obj) ) {
	obj->parentObj = 0;
	if ( childObjects->isEmpty() ) {
	    delete childObjects;		// last child removed
	    childObjects = 0;			// reset children list
	}
    }
}


/*!
  Installs an event filter object for this object.

  An event filter is an object that receives all events that are sent to
  this object.	The filter can either stop the event or forward it to this
  object.  The event filter object receives events via the eventFilter()
  function.  The eventFilter() function must return TRUE if the event
  should be stopped, or FALSE if the event should be dispatched normally.

  If multiple event filters are installed for a single object, the
  filter that was installed last will be activated first.

  Example:
  \code
    #include <qwidget.h>

    class MyWidget : public QWidget
    {
    public:
	MyWidget::MyWidget( QWidget *parent=0, const char *name=0 );
    protected:
	bool  eventFilter( QObject *, QEvent * );
    };

    MyWidget::MyWidget( QWidget *parent, const char *name )
	: QWidget( parent, name )
    {
	if ( parent )				// has a child widget
	    parent->installEventFilter( this ); // then install filter
    }

    bool MyWidget::eventFilter( QObject *, QEvent *e )
    {
	if ( e->type() == Event_KeyPress ) {	// key press
	    QKeyEvent *k = (QKeyEvent*)e;
	    debug( "Ate key press %d", k->key() );
	    return TRUE;			// eat event
	}
	return FALSE;				// standard event processing
    }
  \endcode

  The QAccel class was implemented using this technique.

  \warning
  If you delete the receiver object in your eventFilter() function, be
  sure to return TRUE. If you return FALSE, Qt sends the event to the
  deleted object and the program will crash.

  \sa removeEventFilter(), eventFilter(), event()
*/

void QObject::installEventFilter( const QObject *obj )
{
    if ( !eventFilters ) {
	eventFilters = new QObjectList;
	CHECK_PTR( eventFilters );
    }
    eventFilters->insert( 0, obj );
    connect( obj, SIGNAL(destroyed()), this, SLOT(cleanupEventFilter()) );
}

/*!
  Removes an event filter object \e obj from this object.
  The request is ignored if such an event filter has not been installed.

  All event filters for this object are automatically removed when this
  object is destroyed.

  It is always safe to remove an event filter, even during event filter
  activation (i.e. from the eventFilter() function).

  \sa installEventFilter(), eventFilter(), event()
*/

void QObject::removeEventFilter( const QObject *obj )
{
    if ( eventFilters && eventFilters->removeRef(obj) ) {
	if ( eventFilters->isEmpty() ) {	// last event filter removed
	    delete eventFilters;
	    eventFilters = 0;			// reset event filter list
	}
	disconnect( obj,  SIGNAL(destroyed()),
		    this, SLOT(cleanupEventFilter()) );
    }
}


/*****************************************************************************
  Signal connection management
 *****************************************************************************/

#if defined(CHECK_RANGE)

static bool check_signal_macro( const QObject *sender, const char *signal,
				const char *func, const char *op )
{
    int sigcode = (int)(*signal) - '0';
    if ( sigcode != SIGNAL_CODE ) {
	if ( sigcode == SLOT_CODE )
	    warning( "QObject::%s: Attempt to %s non-signal %s::%s",
		     func, op, sender->className(), signal+1 );
	else
	    warning( "QObject::%s: Use the SIGNAL macro to %s %s::%s",
		     func, op, sender->className(), signal );
	return FALSE;
    }
    return TRUE;
}

static bool check_member_code( int code, const QObject *object,
			       const char *member, const char *func )
{
    if ( code != SLOT_CODE && code != SIGNAL_CODE ) {
	warning( "QObject::%s: Use the SLOT or SIGNAL macro to "
		 "%s %s::%s", func, func, object->className(), member );
	return FALSE;
    }
    return TRUE;
}

static void err_member_notfound( int code, const QObject *object,
				 const char *member, const char *func )
{
    const char *type = 0;
    switch ( code ) {
	case SLOT_CODE:	  type = "slot";   break;
	case SIGNAL_CODE: type = "signal"; break;
    }
    if ( strchr(member,')') == 0 )		// common typing mistake
	warning( "QObject::%s: Parentheses expected, %s %s::%s",
		 func, type, object->className(), member );
    else
	warning( "QObject::%s: No such %s %s::%s",
		 func, type, object->className(), member );
}


static void err_info_about_objects( const char * func,
				    const QObject * sender,
				    const QObject * receiver )
{
    const char * a = sender->name(), * b = receiver->name();
    if ( a )
	warning( "QObject::%s:  (sender name:   '%s')", func, a );
    if ( b )
	warning( "QObject::%s:  (receiver name: '%s')", func, b );
}


#endif // CHECK_RANGE


/*!
  \fn const QObject *QObject::sender()
  Returns a pointer to the object that sent the last signal received by
  this object.

  \warning
  This function violates the object-oriented principle of modularity,
  However, getting access to the sender might be practical when many
  signals are connected to a single slot. The sender is undefined if
  the slot is called as a normal C++ function.
*/

/*!
  \fn void QObject::connectNotify( const char *signal )

  This virtual function is called when something has been connected to
  \e signal in this object.

  \warning
  This function violates the object-oriented principle of modularity.
  However, it might be useful when you need to perform expensive
  initialization only if something is connected to a signal.

  \sa connect(), disconnectNotify()
*/

void QObject::connectNotify( const char * )
{
}

/*!
  \fn void QObject::disconnectNotify( const char *signal )

  This virtual function is called when something has been disconnected from
  \e signal in this object.

  \warning
  This function violates the object-oriented principle of modularity.
  However, it might be useful for optimizing access to expensive resources.

  \sa disconnect(), connectNotify()
*/

void QObject::disconnectNotify( const char * )
{
}


/*!
  \fn bool QObject::checkConnectArgs( const char *signal, const QObject *receiver, const char *member )

  Returns TRUE if the \e signal and the \e member arguments are compatible,
  otherwise FALSE.

  \warning
  We recommend that you do not reimplement this function but use the default
  implementation.

  \internal
  TRUE:	 "signal(<anything>)",	"member()"
  TRUE:	 "signal(a,b,c)",	"member(a,b,c)"
  TRUE:	 "signal(a,b,c)",	"member(a,b)", "member(a)" etc.
  FALSE: "signal(const a)",	"member(a)"
  FALSE: "signal(a)",		"member(const a)"
  FALSE: "signal(a)",		"member(b)"
  FALSE: "signal(a)",		"member(a,b)"
*/

bool QObject::checkConnectArgs( const char    *signal,
				const QObject *,
				const char    *member )
{
    const char *s1 = signal;
    const char *s2 = member;
    while ( *s1++ != '(' ) ;			// scan to first '('
    while ( *s2++ != '(' ) ;
    if ( *s2 == ')' || strcmp(s1,s2) == 0 )	// member has no args or
	return TRUE;				//   exact match
    int s1len = strlen(s1);
    int s2len = strlen(s2);
    if ( s2len < s1len && strncmp(s1,s2,s2len-1)==0 && s1[s2len-1]==',' )
	return TRUE;				// member has less args
    return FALSE;
}


/* tmake ignore Q_OBJECT */
/* tmake ignore Q_OBJECT */

/*!
  Internal function, called from initMetaObject(). Used to emit a warning
  when a class containing the macro Q_OBJECT inherits from a class that
  does not contain it.
*/

void QObject::badSuperclassWarning( const char *className,
				    const char *superclassName )
{
#if defined(CHECK_NULL)
    warning(
    "%s::initMetaObject(): Warning:\n"
    "    The class \"%s\" contains the Q_OBJECT macro, but inherits from the\n"
    "    \"%s\" class, which does not contain the Q_OBJECT macro.\n"
    "    Signal/slot behavior is undefined.\n",
    (const char*)className, (const char*)className,
    (const char*)superclassName );
#endif
}

/*!
  \overload bool QObject::connect( const QObject *sender, const char *signal, const char *member ) const

  Connects \e signal from the \e sender object to \e member in this object.

  Equivalent to: <code>QObject::connect(sender, signal, this, member)</code>.

  \sa disconnect()
*/

/*!
  Connects \e signal from the \e sender object to \e member in object \e
  receiver.

  You must use the SIGNAL() and SLOT() macros when specifying the \e signal
  and the \e member.

  Example:
  \code
    QLabel     *label  = new QLabel;
    QScrollBar *scroll = new QScrollBar;
    QObject::connect( scroll, SIGNAL(valueChanged(int)),
		      label,  SLOT(setNum(int)) );
  \endcode

  This example connects the scroll bar's \link QScrollBar::valueChanged()
  valueChanged()\endlink signal to the label's \link QLabel::setNum()
  setNum()\endlink slot. It makes the label always display the current
  scroll bar value.

  A signal can even be connected to another signal, i.e. \e member is
  a SIGNAL().

  \code
    class MyWidget : public QWidget
    {
    public:
	MyWidget();
    ...
    signals:
	void aSignal(int);
    ...
    private:
    ...
	QPushButton *aButton;
    };

    MyWidget::MyWidget()
    {
	aButton = new QPushButton( this );
	connect( aButton, SIGNAL(clicked()), SIGNAL(aSignal()) );
    }
  \endcode

  In its constructor, MyWidget creates a private button and connects the
  \link QButton::clicked() clicked()\endlink signal to relay clicked() to
  the outside world. You can achieve the same effect by connecting the
  clicked() signal to a private slot and emitting aSignal() in this slot,
  but that takes a few lines of extra code and is not quite as clear, of
  course.

  A signal can be connected to many slots/signals. Many signals can be
  connected to one slot.

  If a signal is connected to several slots, the slots are activated
  in arbitrary order when the signal is emitted.

  \sa disconnect()
*/

bool QObject::connect( const QObject *sender,	const char *signal,
		       const QObject *receiver, const char *member )
{
#if defined(CHECK_NULL)
    if ( sender == 0 || receiver == 0 || signal == 0 || member == 0 ) {
	warning( "QObject::connect: Cannot connect %s::%s to %s::%s",
		 sender ? sender->className() : "(null)",
		 signal ? signal+1 : "(null)",
		 receiver ? receiver->className() : "(null)",
		 member ? member+1 : "(null)" );
	return FALSE;
    }
#endif
    QString signal_name = rmWS( signal );	// white space stripped
    QString member_name = rmWS( member );
    signal = signal_name;
    member = member_name;

    QMetaObject *smeta = sender->queryMetaObject();
    if ( !smeta )				// no meta object
	return FALSE;

#if defined(CHECK_RANGE)
    if ( !check_signal_macro( sender, signal, "connect", "bind" ) )
	return FALSE;
#endif
    signal++;					// skip member type code
    QMetaData *sm;
    if ( !(sm=smeta->signal(signal,TRUE)) ) {	// no such signal
#if defined(CHECK_RANGE)
	err_member_notfound( SIGNAL_CODE, sender, signal, "connect" );
	err_info_about_objects( "connect", sender, receiver );
#endif
	return FALSE;
    }
    signal = sm->name;				// use name from meta object

    int membcode = member[0] - '0';		// get member code

    QObject *s = (QObject *)sender;		// we need to change them
    QObject *r = (QObject *)receiver;		//   internally

#if defined(CHECK_RANGE)
    if ( !check_member_code( membcode, r, member, "connect" ) )
	return FALSE;
#endif
    member++;					// skip code
    QMetaData   *rm = 0;
    QMetaObject *rmeta = r->queryMetaObject();
    if ( !rmeta )				// no meta object
	return FALSE;
    switch ( membcode ) {			// get receiver member
	case SLOT_CODE:	  rm = rmeta->slot( member, TRUE );   break;
	case SIGNAL_CODE: rm = rmeta->signal( member, TRUE ); break;
    }
    if ( !rm ) {				// no such member
#if defined(CHECK_RANGE)
	err_member_notfound( membcode, r, member, "connect" );
	err_info_about_objects( "connect", sender, receiver );
#endif
	return FALSE;
    }
#if defined(CHECK_RANGE)
    if ( !s->checkConnectArgs(signal,receiver,member) )
	warning( "QObject::connect: Incompatible sender/receiver arguments"
		 "\n\t%s::%s --> %s::%s",
		 s->className(), signal,
		 r->className(), member );
#endif
    if ( !s->connections ) {		// create connections dict
	s->connections = new QSignalDict( 7, TRUE, FALSE );
	CHECK_PTR( s->connections );
	s->connections->setAutoDelete( TRUE );
    }
    QConnectionList *clist = s->connections->find( signal );
    if ( !clist ) {				// create receiver list
	clist = new QConnectionList;
	CHECK_PTR( clist );
	clist->setAutoDelete( TRUE );
	s->connections->insert( signal, clist );
    }
    QConnection *c = new QConnection(r, rm->ptr, rm->name);
    CHECK_PTR( c );
    clist->append( c );
    if ( !r->senderObjects ) {			// create list of senders
	r->senderObjects = new QObjectList;
	CHECK_PTR( r->senderObjects );
    }
    r->senderObjects->append( s );		// add sender to list
    s->connectNotify( signal_name );
    return TRUE;
}


/*!
  \overload bool QObject::disconnect( const char *signal, const QObject *receiver, const char *member )

  Disconnects \e signal from \e member of \e receiver.

  A signal-slot connection is removed when either of the objects
  involved are destroyed.
*/

/*!
  \overload bool QObject::disconnect( const QObject *receiver, const char *member )

  Disconnects all signals in this object from \e member of \e receiver.

  A signal-slot connection is removed when either of the objects
  involved are destroyed.
*/

#if (QT_VERSION >= 200)
#error "disconnect const foo"
#endif

/*!
  Disconnects \e signal in object \e sender from \e member in object \e
  receiver.

  A signal-slot connection is removed when either of the objects
  involved are destroyed.

  disconnect() is typically used in three ways, as the following examples
  show.

  <ol>
  <li> Disconnect everything connected to an object's signals:
  \code
    disconnect( myObject );
  \endcode
  <li> Disconnect everything connected to a signal:
  \code
    disconnect( myObject, SIGNAL(mySignal()) );
  \endcode
  <li> Disconnect a specific receiver.
  \code
    disconnect( myObject, 0, myReceiver, 0 );
  \endcode
  </ol>

  0 may be used as a wildcard in three of the four arguments, meaning
  "any signal", "any receiving object" or "any slot in the receiving
  object" respectively.

  The \e sender has no default and may never be 0.  (You cannot
  disconnect signals from more than one object.)

  If \e signal is 0, it disconnects \e receiver and \e member from any
  signal.  If not, only the specified signal is disconnected.

  If \e receiver is 0, it disconnects anything connected to \e signal.
  If not, slots in objects other than \e receiver are not disconnected.

  If \e member is 0, it disconnects anything that is connected to \e
  receiver.  If not, only slots named \e member will be disconnected,
  and all other slots are left alone.  The \e member must be 0 if \e
  receiver is left out, so you cannot disconnect a specifically-named
  slot on all objects.

  \sa connect()
*/

bool QObject::disconnect( const QObject *sender,   const char *signal,
			  const QObject *receiver, const char *member )
{
#if defined(CHECK_NULL)
    if ( sender == 0 || (receiver == 0 && member != 0) ) {
	warning( "QObject::disconnect: Unexpected null parameter" );
	return FALSE;
    }
#endif
    if ( !sender->connections )			// no connected signals
	return FALSE;
    QString signal_name;
    QString member_name;
    QMetaData *rm = 0;
    QObject *s = (QObject *)sender;
    QObject *r = (QObject *)receiver;
    if ( member ) {
	member_name = rmWS( member );
	member = member_name.data();
	int membcode = member[0] - '0';
#if defined(CHECK_RANGE)
	if ( !check_member_code( membcode, r, member, "disconnect" ) )
	    return FALSE;
#endif
	member++;
	QMetaObject *rmeta = r->queryMetaObject();
	if ( !rmeta )				// no meta object
	    return FALSE;
	switch ( membcode ) {			// get receiver member
	    case SLOT_CODE:   rm = rmeta->slot( member, TRUE );	  break;
	    case SIGNAL_CODE: rm = rmeta->signal( member, TRUE ); break;
	}
	if ( !rm ) {				// no such member
#if defined(CHECK_RANGE)
	    err_member_notfound( membcode, r, member, "disconnect" );
	    err_info_about_objects( "disconnect", sender, receiver );
#endif
	    return FALSE;
	}
    }

    QConnectionList *clist;
    register QConnection *c;
    if ( signal == 0 ) {			// any/all signals
	QSignalDictIt it(*(s->connections));
	while ( (clist=it.current()) ) {	// for all signals...
	    const char *curkey = it.currentKey();
	    ++it;
	    c = clist->first();
	    while ( c ) {			// for all receivers...
		if ( r == 0 ) {			// remove all receivers
		    removeObjFromList( c->object()->senderObjects, s );
		    c = clist->next();
		} else if ( r == c->object() &&
			    (member == 0 ||
			     strcmp(member,c->memberName()) == 0) ) {
		    removeObjFromList( c->object()->senderObjects, s );
		    clist->remove();
		    c = clist->current();
		} else {
		    c = clist->next();
		}
	    }
	    if ( r == 0 )			// disconnect all receivers
		s->connections->remove( curkey );
	}
	s->disconnectNotify( 0 );
    }

    else {					// specific signal
	signal_name = rmWS( signal );
	signal = signal_name.data();
#if defined(CHECK_RANGE)
	if ( !check_signal_macro( s, signal, "disconnect", "unbind" ) )
	    return FALSE;
#endif
	signal++;
	clist = s->connections->find( signal );
	if ( !clist ) {
#if defined(CHECK_RANGE)
	    QMetaObject *smeta = s->queryMetaObject();
	    if ( !smeta )			// no meta object
		return FALSE;
	    if ( !smeta->signal(signal,TRUE) )
		warning( "QObject::disconnect: No such signal %s::%s",
			 s->className(), signal );
#endif
	    return FALSE;
	}
	c = clist->first();
	while ( c ) {				// for all receivers...
	    if ( r == 0 ) {			// remove all receivers
		removeObjFromList( c->object()->senderObjects, s, TRUE );
		c = clist->next();
	    } else if ( r == c->object() && (member == 0 ||
				      strcmp(member,c->memberName()) == 0) ) {
		removeObjFromList( c->object()->senderObjects, s, TRUE );
		clist->remove();
		c = clist->current();
	    } else {
		c = clist->next();
	    }
	}
	if ( r == 0 )				// disconnect all receivers
	    s->connections->remove( signal );
	s->disconnectNotify( signal_name );
    }
    return TRUE;
}


/*!
  This signal is emitted immediately before the object is destroyed.

  All the objects's children are destroyed immediately after this signal
  is emitted.
*/

void QObject::destroyed()
{
    activate_signal( "destroyed()" );
}


/*!
  This slot is connected to the destroyed() signal of other objects
  that have installed event filters on this object. When the other
  object is destroyed, we want to remove its event filter.
*/

void QObject::cleanupEventFilter()
{
    removeEventFilter( sender() );
}


/*!
  \internal
  Returns the meta object for this object. If necessary, calls
  initMetaObject().
  \sa metaObject()
*/

QMetaObject *QObject::queryMetaObject() const
{
    register QObject *x = (QObject *)this;	// fake const
    QMetaObject *m = x->metaObject();
    if ( !m ) {					// not meta object
	x->initMetaObject();			//   then try to create it
	m = x->metaObject();
    }
#if defined(CHECK_NULL)
    if ( !m )					// still no meta object: error
	warning( "QObject: Object %s::%s has no meta object",
		 x->className(), x->name( "unnamed" ) );
#endif
    return m;
}

/*!
  Initializes the \link metaObject() meta object\endlink of this
  object. This method is automatically executed on demand from the
  QObject constructor.
  \sa metaObject()
*/
void QObject::initMetaObject()
{
    if ( metaObj )
	return;
    typedef void(QObject::*m1_t0)();
    m1_t0 v1_0 = &QObject::cleanupEventFilter;
    QMetaData *slot_tbl = new QMetaData[1];
    slot_tbl[0].name = "cleanupEventFilter()";
    slot_tbl[0].ptr = *((QMember*)&v1_0);
    typedef void(QObject::*m2_t0)();
    m2_t0 v2_0 = &QObject::destroyed;
    QMetaData *signal_tbl = new QMetaData[1];
    signal_tbl[0].name = "destroyed()";
    signal_tbl[0].ptr = *((QMember*)&v2_0);
    metaObj = new QMetaObject( "QObject", "",
	slot_tbl, 1,
	signal_tbl, 1 );
}


/*!
  \internal Signal activation with the most frequently used
  parameter/argument types.  All other combinations are generated by the
  meta object compiler.
*/
void QObject::activate_signal( const char *signal )
{
    if ( !connections )
	return;
    QConnectionList *clist = connections->find( signal );
    if ( !clist || signalsBlocked() )
	return;
    typedef void (QObject::*RT)();
    typedef RT *PRT;
    RT r;
    QConnectionListIt it(*clist);
    register QConnection *c;
    register QObject *object;
    while ( (c=it.current()) ) {
	++it;
	object = c->object();
	object->sigSender = this;
	r = *((PRT)(c->member()));
	(object->*r)();
    }
}

/*!
  \overload void QObject::activate_signal( const char *signal, short )
*/

/*!
  \overload void QObject::activate_signal( const char *signal, int )
*/

/*!
  \overload void QObject::activate_signal( const char *signal, long )
*/

/*!
  \overload void QObject::activate_signal( const char *signal, const char * )
*/


#define ACTIVATE_SIGNAL_WITH_PARAM(TYPE)				      \
void QObject::activate_signal( const char *signal, TYPE param )		      \
{									      \
    if ( !connections )							      \
	return;								      \
    QConnectionList *clist = connections->find( signal );		      \
    if ( !clist || signalsBlocked() )					      \
	return;								      \
    typedef void (QObject::*RT0)();					      \
    typedef RT0 *PRT0;							      \
    typedef void (QObject::*RT1)( TYPE );				      \
    typedef RT1 *PRT1;							      \
    RT0 r0;								      \
    RT1 r1;								      \
    QConnectionListIt it(*clist);					      \
    register QConnection *c;						      \
    register QObject *object;						      \
    while ( (c=it.current()) ) {					      \
	++it;								      \
	object = c->object();						      \
	object->sigSender = this;					      \
	if ( c->numArgs() ) {						      \
	    r1 = *((PRT1)(c->member()));				      \
	    (object->*r1)( param );					      \
	} else {							      \
	    r0 = *((PRT0)(c->member()));				      \
	    (object->*r0)();						      \
	}								      \
    }									      \
}

// We don't want to duplicate too much text so...

ACTIVATE_SIGNAL_WITH_PARAM( short )
ACTIVATE_SIGNAL_WITH_PARAM( int )
ACTIVATE_SIGNAL_WITH_PARAM( long )
ACTIVATE_SIGNAL_WITH_PARAM( const char * )


/*****************************************************************************
  QObject debugging output routines.
 *****************************************************************************/

static void dumpRecursive( int level, QObject *object )
{
#if defined(DEBUG)
    if ( object ) {
	QString buf;
	buf.fill( '\t', level );
	const char *name = object->name( "unnamed" );
	QString flags="";
	if ( qApp->focusWidget() == object )
	    flags += 'F';
	if ( object->isWidgetType() ) {
	    QWidget * w = (QWidget *)object;
	    if ( w->isVisible() ) {
		QString t;
		t.sprintf( "<%d,%d,%d,%d>",
			   w->x(), w->y(), w->width(), w->height() );
		flags += t;
	    } else {
		flags += 'I';
	    }
	}	
	debug( "%s%s::%s %s", (const char*)buf, object->className(), name,
	    (const char*)flags );
	if ( object->children() ) {
	    QObjectListIt it(*object->children());
	    while ( it ) {
		dumpRecursive( level+1, it );
		++it;
	    }
	}
    }
#endif
}

/*!
  Dumps a tree of children to the debug output.

  This function is useful for debugging.
*/

void QObject::dumpObjectTree()
{
    dumpRecursive( 0, this );
}

/*!
  Dumps information about signal connections etc. for this object to the
  debug output.

  This function is useful for debugging.
*/

void QObject::dumpObjectInfo()
{
#if defined(DEBUG)
    debug( "OBJECT %s::%s", className(), name( "unnamed" ) );
    debug( "  SIGNALS OUT" );
    int n = 0;
    if ( connections ) {
	QSignalDictIt it(*connections);
	QConnectionList *clist;
	while ( (clist=it.current()) ) {
	    debug( "\t%s", it.currentKey() );
	    n++;
	    ++it;
	    register QConnection *c;
	    QConnectionListIt cit(*clist);
	    while ( (c=cit.current()) ) {
		++cit;
		debug( "\t  --> %s::%s %s", c->object()->className(),
		       c->object()->name( "unnamed" ), c->memberName() );
	    }
	}
    }
    if ( n == 0 )
	debug( "\t<None>" );
    debug( "  SIGNALS IN" );
    n = 0;
    if ( senderObjects ) {
	QObject *sender = senderObjects->first();
	while ( sender ) {
	    debug( "\t%s::%s",
		   sender->className(), sender->name( "unnamed" ) );
	    n++;
	    sender = senderObjects->next();
	}
    }
    if ( n == 0 )
	debug( "\t<None>" );
#endif
}



/*!  Returns a translated version of \a text, or \a text if there is
  no appropriate translated version.
*/

const char * QObject::tr( const char * text ) const
{
    return text;
}
