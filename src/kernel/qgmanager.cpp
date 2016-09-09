/****************************************************************************
** $Id: qgmanager.cpp,v 2.37.2.1 1998/10/29 00:06:11 ettrich Exp $
**
** Implementation of QGGeometry class
**
** Created : 960406
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

#include "qgmanager.h"
#include "qlist.h"
#include "qmenubar.h"
#include "qapplication.h"

/*!
  \class QGManager qgmanager.h
  \brief The QGManager class provides one-dimensional geometry management.

  \ingroup geomanagement

  This class is intended for those who write geometry managers and
  graphical designers. <strong>It is not for the faint of
  heart. </strong> The QHBoxLayout, QVBoxLayout and QGridLayout classes are
  available for normal application programming.

  Each dimension (horizontal and vertical) is handled independently. Widgets
  are organized in chains, which can be parallel or serial.

  In a serial chain, elements are added one after another. Available
  space is divided among the elements according to stretch and
  max-/minsize.

  In parallel chains, elements are added on top of each other. All
  elements are given the full length of the chain, and are placed at
  the same position.


  \sa QHBoxLayout QVBoxLayout QGridLayout

*/


static inline bool horz( QGManager::Direction dir )
{
    return dir == QGManager::RightToLeft || dir == QGManager::LeftToRight;
}
struct WidgetInfo {
    QRect geom;
    QWidget *widget;
};

typedef QIntDict<WidgetInfo> wDict;


WidgetInfo *lookup( QWidget * w, wDict & table,
		    bool create = FALSE )
{
    WidgetInfo *wi = table[ (long) w ];
    if ( !wi && create ) {
	wi = new WidgetInfo;
	wi->widget = w;
	table.insert( (long) w, wi );
    }
    return wi;
}




static void setWinfo( QWidget * w, wDict &dict, QGManager::Direction d, int p, int s )
{
    WidgetInfo *wi = lookup( w, dict, TRUE );
    if ( horz( d ) ) {
	wi->geom.setX( p );
	wi->geom.setWidth( s );
    } else {
	wi->geom.setY( p );
	wi->geom.setHeight( s );
    }
}

/*
  \class QChain qgmanager.cpp

  \brief internal class for the QGManager.

  \internal

  Everything is put into chains. Use QGManager::newParChain()
  or QGManager::newSerChain() to make chains.

  \sa QGManager.
*/

class QChain
{
public:

    QChain( QGManager::Direction d ) { dir = d; }
    virtual ~QChain() {}

    bool add( QChain *s, int stretch )
    {
	if ( addC(s) ) {
	    s->sstretch = stretch;
	    return TRUE;
	} else
	    return FALSE;
    }

    virtual bool addBranch( QChain*, int, int )
    {
	return FALSE;
    }

    virtual int maxSize() = 0;
    virtual int minSize() = 0;
    int stretch() { return sstretch; }
    void setStretch( int s ) { sstretch = s; }
    virtual void recalc() {}

    virtual void distribute( wDict&, int pos, int space) = 0;

    QGManager::Direction direction() { return dir; }

    virtual bool removeWidget( QWidget * ) { return FALSE; }
    virtual void annihilate() {}

    virtual void setName( const char * ) {}
    virtual const char *name() { return 0; }

protected:
    virtual bool addC( QChain *s ) = 0;

private:
    QGManager::Direction dir;

    int sstretch;
};


class QSpaceChain : public QChain
{
public:
    QSpaceChain( QGManager::Direction d, int min, int max )
	: QChain( d ), minsize( min ), maxsize( max ) {}
    // needs direction for consistency check.....
    bool addC( QChain * ) { return FALSE; }


    void distribute( wDict&, int, int ) {}

    int maxSize() { return maxsize; }
    int minSize() { return minsize; }

private:
    int minsize;
    int maxsize;
};

class QWidChain : public QChain
{
public:
    QWidChain( QGManager::Direction d,  QWidget * w )
	: QChain( d ), widget ( w ) {}
    bool addC( QChain * ) { return FALSE; }

    int minSize();
    int maxSize();

    bool removeWidget( QWidget *w ) {
	if ( w == widget ) {
	    widget = 0;
	    return TRUE;
	} else {
	    return FALSE;
	}
    }

    void distribute( wDict & wd, int pos, int space ) {
	if ( widget ) setWinfo( widget, wd, direction(),  pos, space );
    }

private:
    QWidget * widget;
};

int QWidChain::minSize()
{
    if ( !widget )
	return 0;
    QSize s = widget->minimumSize();
    if ( horz( direction() ) )
	return s.width();
    else
	return s.height();
}
int QWidChain::maxSize()
{
    if ( !widget )
	return QGManager::unlimited;
    QSize s = widget->maximumSize();
    if ( horz( direction() ) )
	return s.width();
    else
	return s.height();
}


class QParChain : public QChain
{
public:

    QParChain( QGManager::Direction d )
	: QChain( d )
    {
    }

    ~QParChain();
    bool addC( QChain *s );

    void recalc();

    void distribute( wDict &, int, int );
    bool removeWidget( QWidget *w );

    int maxSize() { return maxsize; }
    int minSize() { return minsize; }

    void setName( const char *s ) { nam = s; }
    const char *name() { return nam; }

private:
    int maxsize;
    int minsize;
    int sstretch;

    QList<QChain> chain;

    int minMax();
    int maxMin();
    QString nam;
};


struct QBranchData {
    int from;
    int to;
    QChain *chain;
};

/*
  If all members have zero stretch, the space is divided equally;
  this is perhaps not what you want. Use setMaximumSize to be sure your
  widget is not stretched.

 */
class QSerChain : public QChain
{
public:

    QSerChain( QGManager::Direction d ) : QChain( d ) {}
    ~QSerChain();

    bool addC( QChain *s );
    bool addBranch( QChain*, int, int );

    void recalc();
    void distribute( wDict &, int, int);
    bool removeWidget( QWidget *w );
    int maxSize() { return  maxsize; }
    int minSize() { return minsize; }

    void setName( const char *s ) { nam = s; }
    const char *name() { return nam; }

private:
    int maxsize;
    int minsize;

    QList<QChain> chain;
    QList<QBranchData> branches;
    int sumMax();
    int sumMin();
    int sumStretch();
    QString nam;
};



QParChain::~QParChain()
{
    int i;
    for ( i = 0; i < (int)chain.count(); i++ ) {
	delete chain.at(i);
    }
}

void QParChain::distribute( wDict & wd, int pos, int space )
{
    int i;
    for ( i = 0; i < (int)chain.count(); i++ ) {
	chain.at(i)->distribute(  wd, pos, space );
    }
}

bool QParChain::removeWidget( QWidget *w )
{
    QChain *c = chain.first();
    while ( c ) {
	if ( c->removeWidget( w ) ) {
	    return TRUE; //only one in a chain
	}
	c = chain.next();
    }
    return FALSE;
}


QSerChain::~QSerChain()
{
    int i;
    for ( i = 0; i < (int)chain.count(); i++ ) {
	delete chain.at(i);
    }
    for ( i = 0; i < (int)branches.count(); i++ ) {
	QBranchData *bd = branches.at(i);
	delete bd->chain;
	delete bd;
    }
}

//### possible bug if RightToLeft or Up
bool QSerChain::addBranch( QChain *b, int from, int to )
{
    if ( from < 0 || to < from || from >= (int)chain.count() ) {
	warning( "QGManager: Invalid anchor for branch" );
	return FALSE;
    }
    if ( horz( direction() ) != horz( b->direction() ) ) {
	warning( "QGManager: branch 90 degrees off" );
	return FALSE;
    }
    QBranchData *d = new QBranchData;
    d->chain = b;
    d->from = from;
    d->to = to;
    branches.append( d );
    return TRUE;
}




bool QSerChain::removeWidget( QWidget *w )
{
    int i;
    for ( i = 0; i < (int)branches.count(); i++ ) {
	if ( branches.at(i)->chain->removeWidget( w ) )
	    return TRUE;
    }
    for ( i = 0; i < (int)chain.count(); i++ ) {
	if ( chain.at(i)->removeWidget( w ) )
	    return TRUE;
    }
    return FALSE;

    //######## Memory leak and extra space problem

}



static inline int toFixed( int i ) { return i * 256; }
static inline int fRound( int i ) {
    return  i % 256 < 128 ? i / 256 : 1 + i / 256;
}
/*
  \internal
  This is the main workhorse of the geometry manager. It portions out
  available space to the chain's children.

  The calculation is done in fixed point: "fixed" variables are scaled
  by a factor of 256.

  If the chain runs "backwards" (i.e. RightToLeft or Up) the layout
  is computed mirror-reversed, and then turned the right way at the end.

*/

void QSerChain::distribute( wDict & wd, int pos, int space )
{
    typedef int fixed;

    fixed available = toFixed( space - minSize() );
    if ( available < 0 ) {
#ifdef QT_LAYOUT_WARNINGS	
	QString msg;
	if ( !name() )
	    msg.sprintf( "QGManager: not enough space for"
			 " unnamed %d-item %s chain",
			 chain.count(),
			 horz( direction() ) ? "horizontal" : "vertical",
			 branches.count()
			 );
	else
	    msg.sprintf( "QGManager: not enough space for %s chain %s",
			 horz( direction() ) ? "horizontal" : "vertical",
			 name() );
	warning( msg );
#endif	
	available = 0;
    }
    int sf = sumStretch();

#define DISTRIBUTE_AVAILABLE_AFTER_MINIMUM // Qt 1.x
#ifdef DISTRIBUTE_AVAILABLE_AFTER_MINIMUM
    QArray<fixed> sizes( chain.count() );
    int i;
    for ( i = 0; i < (int)chain.count(); i++ )
	sizes[i] = 0;
    bool doAgain = TRUE;
    int numChains = chain.count();
    while ( doAgain && numChains ) {
	doAgain = FALSE;
	for ( i = 0; i < (int)chain.count(); i++ ) {
	    fixed maxS = toFixed( chain.at(i)->maxSize() );
	    if ( sizes[i] == maxS )
		continue;
	    fixed minS = toFixed( chain.at(i)->minSize() );
	    fixed siz = minS;
	    if ( sf )
		siz += available * chain.at(i)->stretch() / sf;
	    else
		siz += available  / numChains;
	    if ( siz >=  maxS ) {
		sizes[i] = maxS;
		available -= maxS - minS;
		sf -= chain.at(i)->stretch();
		numChains--;
		doAgain = TRUE;
		break;
	    }
	    sizes[i] = siz;
	}
    }
#else
    QArray<fixed> sizes( chain.count() );
    QArray<bool> forced( chain.count() );
    int i;
    for ( i = 0; i < (int)chain.count(); i++ )
	forced[i] = FALSE;
    bool doAgain = TRUE;
    bool done_pass = FALSE;
    int numChains = chain.count();
    fixed sp = toFixed( space );
    while ( doAgain && numChains ) {
	doAgain = FALSE;
	for ( i = 0; i < (int)chain.count(); i++ ) {
	    if ( forced[i] )
		continue;
	    fixed maxS = toFixed( chain.at(i)->maxSize() );
	    fixed minS = toFixed( chain.at(i)->minSize() );
	    fixed siz;
	    if ( sf )
		siz = sp * chain.at(i)->stretch() / sf;
	    else
		siz = sp / numChains;
	    if ( siz <= minS ) {
		sp -= minS;
		sizes[i] = minS;
		forced[i] = TRUE;
		sf -= chain.at(i)->stretch();
		numChains--;
		doAgain = TRUE;
		break;
	    } else if ( siz >= maxS ) {
		if ( done_pass ) {
		    sp -= maxS;
		    sizes[i] = maxS;
		    forced[i] = TRUE;
		    sf -= chain.at(i)->stretch();
		    numChains--;
		    doAgain = TRUE;
		    break;
		} else {
		    doAgain = TRUE;
		}
	    }
	    sizes[i] = siz;
	}
	if ( i == (int)chain.count() ) done_pass = TRUE;
    }
    fixed tsize = 0;
    int nsf0 = 0;
    for ( i = 0; i < (int)chain.count(); i++ ) {
	tsize += sizes[i];
	if ( chain.at(i)->stretch() == 0
	     && sizes[i] < toFixed( chain.at(i)->maxSize() ) ) nsf0++;
    }
    while ( tsize < toFixed(space) && nsf0 ) {
	fixed tsp = (toFixed(space) - tsize);
	fixed portion = tsp / nsf0;
	int n0 = nsf0;
	int n = nsf0;
	for ( i = 0; i < (int)chain.count(); i++ ) {
	    if ( chain.at(i)->stretch() == 0
		 && sizes[i] < toFixed( chain.at(i)->maxSize() ) )
		{
		    fixed extra = toFixed( chain.at(i)->maxSize() ) - sizes[i];
		    if ( --n == 0 ) {
			portion = tsp - (n0-1) * portion;
		    }
		    if ( extra <= portion ) {
			nsf0--;
		    } else {
			extra = portion;
		    }
		    sizes[i] += portion;
		    tsize += portion;
		}
	}
    }
#endif

    int n = chain.count();
    QArray<int> places( n + 1 );
    places[n] = pos + space;
    fixed fpos = toFixed( pos );
    for ( i = 0; i < (int)chain.count(); i++ ) {
	places[i] = QMAX( fRound( fpos ), pos );  // only give what we've got
	fpos += sizes[i];
    }

    bool backwards = ( direction() == QGManager::RightToLeft ||
		       direction() == QGManager::Up );

    for ( i = 0; i < (int)chain.count(); i++ ) {
	int p = places[i];
	int s = places[i+1] - places[i];
	if ( backwards )
	    p = 2 * pos + space - p - s;
	chain.at(i)->distribute( wd, p, s );
    }

    for ( i = 0; i < (int)branches.count(); i++ ) {
	QBranchData *b = branches.at( i );
	int from = places[ b->from ];
	int to = places[ b->to + 1 ];
	int s = to - from;
	if ( backwards )
	    from = 2 * pos + space - from - s;
	branches.at(i)->chain->distribute( wd, from, s  );
    }
}

void QParChain::recalc()
{
    for ( int i = 0; i < (int)chain.count(); i ++ )
	chain.at(i)->recalc();
    maxsize = minMax();
    minsize = maxMin();
}


int QParChain::maxMin()
{
    int max = 0;
    for ( int i = 0; i < (int)chain.count(); i ++ ) {
	int m = chain.at(i)->minSize();
	if ( m	> max )
	    max = m;
    }
    return max;
}

int QParChain::minMax()
{
    int min = QGManager::unlimited;
    for ( int i = 0; i < (int)chain.count(); i ++ ) {
	int m = chain.at(i)->maxSize();
	if ( m < min )
	    min = m;
    }
    return min;
}

void QSerChain::recalc()
{
    int i;
    for ( i = 0; i < (int)chain.count(); i ++ )
	chain.at(i)->recalc();
    for ( i = 0; i < (int)branches.count(); i ++ )
	branches.at(i)->chain->recalc();
    minsize = sumMin();
    maxsize = sumMax();
}


int QSerChain::sumStretch()
{
    int s = 0;
    for ( int i = 0; i < (int)chain.count(); i ++ )
	 s += chain.at(i)->stretch();
    return s;
}

int QSerChain::sumMin()
{
    int s = 0;
    for ( int i = 0; i < (int)chain.count(); i ++ )
	s += chain.at(i)->minSize();
    return s;
}

int QSerChain::sumMax()
{
    int s = 0;
    for ( int i = 0; i < (int)chain.count(); i ++ )
	s += chain.at(i)->maxSize();
    if ( s > QGManager::unlimited )
	s = QGManager::unlimited;
    return s;
}



bool QSerChain::addC( QChain *s )
{
    if ( horz( s->direction() ) != horz( direction() ) ) {
	if ( horz( direction() ) )
	    warning("QGManager:Cannot add vertical chain to horizontal serial chain");
	else
	    warning("QGManager:Cannot add horizontal chain to vertical serial chain");
	return FALSE;
    }
    chain.append( s );
    return TRUE;
}

bool QParChain::addC( QChain *s )
{
    if ( horz( s->direction() ) != horz( direction() ) ) {
	if ( horz( direction() ) )
	    warning("QGManager:Cannot add vertical chain to horizontal parallel chain");
	else
	    warning("QGManager:Cannot add horizontal chain to vertical parallel chain");
	return FALSE;
    }
    chain.append( s );
    return TRUE;
}

/*!
  Creates a new QGManager which manages \e parent's children.
*/
QGManager::QGManager( QWidget *parent, const char *name )
    : QObject( parent, name )
{
    main = parent;
    border = 0;
    frozen = FALSE;
    menuBar = 0;
    menuBarHeight = 0;

    xC = new QParChain( LeftToRight );
    yC = new QParChain( Down );

    if ( parent ) {
	parent->installEventFilter( this );
    }
}

/*!
  Destroys the QGManager, deleting all add()ed chains.
*/
QGManager::~QGManager()
{
    delete xC;
    delete yC;
}


/*!
  \fn QWidget *QGManager::mainWidget()

  Returns the main widget of the manager.
  */


/*!
  \fn QChain *QGManager::xChain()

  Returns the main horizontal chain of the manager. All horizontal chains
  should be inserted into this chain or one of its descendants, otherwise
  they will be ignored.
*/


/*!
  \fn QChain *QGManager::yChain()

  Returns the main vertical chain of the manager. All vertical chains
  should be inserted into this chain or one of its descendants, otherwise
  they will be ignored.
*/


/*!
  \fn void QGManager::setBorder( int b )

  Sets the border around the edge of the widget. \e b is the number of
  pixels between the edge of the widget and the area controlled by the
  manager.
*/

/*!
  \fn void  QGManager::setMenuBar( QWidget *w )

  Makes the geometry manager take account of the menu bar \a w. All
  child widgets are placed below the bottom edge of the menu bar.

*/



/*!
  Creates a new QChain which is \e parallel.
*/

QChain * QGManager::newParChain( Direction d )
{
    QChain * c = new QParChain( d );
    CHECK_PTR(c);
    return c;
}


/*!
  Creates a new QChain which is \e serial.
*/

QChain * QGManager::newSerChain( Direction d )
{
    QChain * c = new QSerChain( d );
    CHECK_PTR(c);
    return c;
}

/*!
  Adds the chain \e source to the chain \e destination.
*/

bool QGManager::add( QChain *destination, QChain *source, int stretch )
{
    return destination->add(source, stretch);
}


/*!
  Adds the widget  \e w to the chain \e d.
*/

bool QGManager::addWidget( QChain *d, QWidget *w, int stretch )
{
    //if ( w->parent() != main ) {
    //	  warning("QGManager::addWidget - widget is not child.");
    //	  return FALSE;
    //}
    return d->add( new QWidChain( d->direction(), w) , stretch );
}


#if 0
/*!
  Adds the widget  \a w to the chain \a d with \a space pixels of padding
  before it. If \a autoCollapse is TRUE, the space will be reclaimed when
  the widget is hidden.
*/

bool QGManager::addWidget( QChain *d, QWidget *w, int stretch,
			   int space, bool autoCollapse )
{
    QWidChain *c = new QWidChain( d->direction(), w);
    c->setAutoCollapse( autoCollapse );
    c->setExtraSpace( space );
    bool ok = d->add( c, stretch );
    //###??? if (!ok) delete c;
	
    return ok;
}
#endif

/*!
  Adds the spacing  \a w to the chain \a d. If \a d is a serial chain, this
  means screen space between widgets. If \a d is parallel, this influences
  the maximum and minimum size.
*/

bool QGManager::addSpacing( QChain *d, int minSize, int stretch, int maxSize )
{
    return d->add( new QSpaceChain( d->direction(), minSize, maxSize), stretch	);
}

/*!
  Grabs all resize events for my parent, and does child widget resizing.
*/

bool QGManager::eventFilter( QObject *o, QEvent *e )
{
    if ( !o->isWidgetType() )
	return FALSE;

    QWidget *w = (QWidget*)o;
    switch ( e->type() ) {
    case Event_Resize: {
	QResizeEvent *r = (QResizeEvent*)e;
	resizeHandle( w, r->size() );
	break;
    }
    case Event_ChildRemoved: {
	QChildEvent *c = (QChildEvent*)e;
	remove( c->child() );
	QEvent *lh = new QEvent( Event_LayoutHint );
	QApplication::postEvent( o, lh );
	break;
    }
    case Event_LayoutHint:
	activate(); //######## ######@#!#@!$ should be optimized somehow...
	break;
    }
    return FALSE;			    // standard event processing
}

void QGManager::resizeHandle( QWidget *, const QSize & )
{
    resizeAll();
}

/*!
  Starts geometry management.
*/

bool QGManager::activate()
{
    if ( frozen )
	return FALSE;

    yC->recalc();
    xC->recalc();

    menuBarHeight = menuBar ? menuBar->height() : 0;

    int ys = yC->minSize() + 2*border + menuBarHeight;
    int xs = xC->minSize() + 2*border;

    main->setMinimumSize( xs, ys );

    ys = yC->maxSize() + 2*border + menuBarHeight;
    if ( ys > QGManager::unlimited )
	ys = QGManager::unlimited;
    if ( ys < 1 )
	ys = 1;
    xs = xC->maxSize() + 2*border;
    if ( xs > QGManager::unlimited )
	xs = QGManager::unlimited;
    if ( xs < 1 )
	xs = 1;

    main->setMaximumSize( xs, ys );

    resizeAll(); //### double recalc...
    return TRUE;
}

/*!
  Fixes the size of the main widget and distributes the available
  space to the child widgets. The size is adjusted to a valid
  value. Thus freeze(0,0) (the default) will fix the widget to its
  minimum size.
*/
void QGManager::freeze( int w, int h )
{
    frozen = FALSE; // so activate can do it.
    activate();

    QSize max = main->maximumSize();
    QSize min = main->minimumSize();

    w = QMAX( min.width(), QMIN( w, max.width() ) );
    h = QMAX( min.height(), QMIN( h, max.height() ) );
    main->setMaximumSize( w, h );
    main->setMinimumSize( w, h );
    main->resize( w, h );
    resizeAll(); // deferred resize!
    frozen = TRUE;
}


/*!
  Undoes the effect of a previous freeze(). The main widget will
  now again be resizable.
*/

void QGManager::unFreeze()
{
    if ( !frozen )
	return;
    frozen = FALSE;
    activate();
}


void QGManager::resizeAll()
{

    QIntDict<WidgetInfo> lookupTable;

    xC->recalc();
    yC->recalc();

    QSize max = main->maximumSize();
    QSize min = main->minimumSize();


    // Resize menubar first to get valid size for rest of widgets to
    // arrange themselves around.

    // size may not be set yet
    int ww = QMAX( min.width(), QMIN( main->width(), max.width() ) );
    int hh;

    int mbh = menuBar ? ((QMenuBar *)menuBar)->heightForWidth( ww ) : 0;

    if ( menuBar && mbh != menuBarHeight ) {
	int ombh = menuBarHeight;
	menuBarHeight = mbh;
	main->setMinimumHeight(  min.height() + mbh - ombh );
	if ( max.height() < unlimited )
	    main->setMaximumHeight( max.height() + mbh - ombh );
    }
    ww = QMAX( min.width(), QMIN( main->width(), max.width() ) );
    hh = QMAX( min.height(), QMIN( main->height(), max.height() ) );

    xC->distribute( lookupTable, border, ww - 2*border );
    yC->distribute( lookupTable, mbh + border, hh - 2*border - mbh );

    QIntDictIterator<WidgetInfo> it( lookupTable );

    WidgetInfo *w;
    while (( w = it.current() )) {
	++it;
	if ( w->widget )
	    w->widget->setGeometry( w->geom );
	delete w;
    }
}

/*!
  Adds \a branch to \a destination as a branch going from \a fromIndex
  to \a toIndex. A branch is a chain that is anchored at two locations
  in a serial chain. The branch does not influence the main chain;
  if the branch's minimum size is greater than the minimum distance
  between the anchors, things will look ugly.

  The branch goes from the beginning of the item at \a fromIndex to the
  end of the item at \a toIndex. Note: remember to count spacing when
  calculating indices.
*/

bool QGManager::addBranch( QChain *destination, QChain *branch,
			       int fromIndex, int toIndex )
{
    bool success = destination->addBranch( branch, fromIndex, toIndex );
    if ( ! success )
	warning( "QGManager: Couldn't add branch" );
    return success;
}


/*!
  Sets the stretch factor on \a c to \a s. This stretch factor is
  overridden by add(), so there's no point in calling this function
  before you add() the chain.

  \sa add()
*/

void QGManager::setStretch( QChain *c, int s )
{
    c->setStretch( s );
}


/*!
  Removes the widget \a w from geometry management. Will not remove spacing
  around the widget. This function is called automatically if the
  widget is deleted.
*/

void QGManager::remove( QWidget *w )
{
    xC->removeWidget( w );
    yC->removeWidget( w );
}

#if 0
/*!
  Removes the chain \a c from geometry management. Will not remove spacing
  around the chain.
*/

void QGManager::remove( QChain *c )
{
    c->annihilate();
}

#endif


/*!
  Sets the name of \a chain to \a name. The name is used for debugging
  purposes.
*/

void QGManager::setName( QChain *chain, const char *name )
{
    chain->setName( name );
}
