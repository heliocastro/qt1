/****************************************************************************
** $Id: scrollview.cpp,v 1.18.2.2 1998/10/01 20:44:34 warwick Exp $
**
** Copyright (C) 1992-1998 Troll Tech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#include <qscrollview.h>
#include <qapplication.h>
#include <qmenubar.h>
#include <qpopupmenu.h>
#include <qpushbutton.h>
#include <qpainter.h>
#include <qpixmap.h>
#include <qmessagebox.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qmultilinedit.h>
#include <stdlib.h>


static const int style_id	= 0x1000;
static const int lw_id		= 0x2000;
static const int mlw_id	    	= 0x4000;
static const int mw_id		= 0x8000;
static const int max_lw		= 16;
static const int max_mlw	= 5;
static const int max_mw		= 10;


class BigShrinker : public QFrame {
    Q_OBJECT
public:
    BigShrinker()
    {
	setFrameStyle(QFrame::Box|QFrame::Sunken);
	int h=35;
	int b=0;
	for (int y=0; y<2000-h; y+=h+10) {
	    if (y == 0) {
		QButton* q=new QPushButton("Quit", this);
		connect(q, SIGNAL(clicked()), qApp, SLOT(quit()));
	    } else {
		QString str;
		if ( b > 0 ) {
		    str.sprintf("Button %d", b++);
		} else {
		    str = "I'm shrinking!";
		    ++b;
		}
		(new QPushButton(str, this))->move(y/2,y);
	    }
	}
	resize(1000,2000);

	startTimer(250);
    }

    void timerEvent(QTimerEvent*)
    {
	int w=width();
	int h=height();
	if ( w > 50 ) w -= 1;
	if ( h > 50 ) h -= 2;
	resize(w,h);
    }

    void mouseReleaseEvent(QMouseEvent* e)
    {
	emit clicked(e->x(), e->y());
    }

signals:
    void clicked(int,int);
};

class BigMatrix : public QScrollView {
    QMultiLineEdit *dragging;
public:
    BigMatrix(QWidget* parent) :
	QScrollView(parent,"matrix"),
	bg("bg.ppm")
    {
	bg.load("bg.ppm");
	viewport()->setBackgroundMode(NoBackground);
	resizeContents(400000,300000);

	QMultiLineEdit *l = new QMultiLineEdit(viewport(),"First");
	l->setText("Drag out more of me!");
	l->resize(100,100);
	addChild(l, 100,100);

	dragging = 0;
    }

    void viewportMousePressEvent(QMouseEvent* e)
    {
	int x = e->x() + contentsX();
	int y = e->y() + contentsY();
	dragging = new QMultiLineEdit(viewport(),"Another");
	dragging->setText("Thanks!");
	dragging->resize(100,100);
	addChild(dragging, x, y);
	showChild(dragging);
    }

    void viewportMouseReleaseEvent(QMouseEvent*)
    {
	dragging = 0;
    }

    void viewportMouseMoveEvent(QMouseEvent* e)
    {
	if ( dragging ) {
	    int mx = e->x() + contentsX();
	    int my = e->y() + contentsY();
	    int cx = childX(dragging);
	    int cy = childY(dragging);
	    int w = mx - cx + 1;
	    int h = my - cy + 1;
	    QString msg;
	    msg.sprintf("at (%d,%d) %d by %d",cx,cy,w,h);
	    dragging->setText(msg);
	    dragging->resize(w,h);
	}
    }

//#define DBLBUFF
protected:
    void drawContentsOffset(QPainter* paint, int ox, int oy, int cx, int cy, int cw, int ch)
    {
// Define DBLBUFF for even smooth repaints.
#ifdef DBLBUFF
	dblbuff.resize(viewport()->width(), viewport()->height());
	QPainter p2;
	QPainter* p = &p2;
	p->begin(&dblbuff);
#else
	QPainter* p = paint;
#endif
	p->setClipRect(cx-ox, cy-oy, cw, ch);

	// The Background
	{
	    int rowheight=bg.height();
	    int toprow=cy/rowheight;
	    int bottomrow=(cy+ch+rowheight-1)/rowheight;
	    int colwidth=bg.width();
	    int leftcol=cx/colwidth;
	    int rightcol=(cx+cw+colwidth-1)/colwidth;
	    for (int r=toprow; r<=bottomrow; r++) {
		int py=r*rowheight;
		for (int c=leftcol; c<=rightcol; c++) {
		    int px=c*colwidth;
		    p->drawPixmap(px-ox, py-oy, bg);
		}
	    }
	}

	// The Numbers
	{
	    QFontMetrics fm=p->fontMetrics();
	    int rowheight=fm.lineSpacing();
	    int toprow=cy/rowheight;
	    int bottomrow=(cy+ch+rowheight-1)/rowheight;
	    int colwidth=fm.width("00000,000000 ")+3;
	    int leftcol=cx/colwidth;
	    int rightcol=(cx+cw+colwidth-1)/colwidth;
	    QString str;
	    for (int r=toprow; r<=bottomrow; r++) {
		int py=r*rowheight;
		for (int c=leftcol; c<=rightcol; c++) {
		    int px=c*colwidth;
		    str.sprintf("%d,%d",c,r);
		    p->drawText(px-ox+3, py-oy+fm.ascent(), str);
		}
	    }

	    // The Big Hint
	    if (leftcol<10 && toprow<5) {
		p->setFont(QFont("Charter",30));
		p->setPen(red);
		QString text;
		text.sprintf("HINT:  Look at %d,%d",415000/colwidth,115000/rowheight);
		p->drawText(100-ox,50-oy,text);
	    }
	}

	// The Big X
	{
	    if (cx+cw>400000 && cy+ch>100000 && cx<430000 && cy<130000) {
		p->drawLine(400000-ox,100000-oy,429999-ox,129999-oy);
		p->drawLine(429999-ox,100000-oy,400000-ox,129999-oy);

		// X marks the spot!
		p->setFont(QFont("Charter",100));
		p->setPen(blue);
		p->drawText(415000-ox-500,115000-oy-100,1000,200,AlignCenter,"YOU WIN!!!!!");
	    }
	}

#ifdef DBLBUFF
	p->end();
	paint->drawPixmap(0,0,dblbuff);
#endif
    }

private:
    QPixmap bg;
#ifdef DBLBUFF
    QPixmap dblbuff;
#endif
};

// Windows corner thingy.  Of course, it doesn't *function*.
//
class WinCorner : public QWidget {
public:
    WinCorner(QWidget* parent=0, const char* name=0, WFlags f=0) :
	QWidget(parent, name, f)
    {
    }

protected:
    void paintEvent(QPaintEvent*)
    {
	QPainter p(this);

	int ox = width()-12;
	int oy = height()-12;

	QColorGroup g = colorGroup();
	for ( int i=0; i<11; i++ ) {
	    int n = i & 3;
	    if ( n == 0 )
		p.setPen( g.light() );
	    else if ( n < 3 )
		p.setPen( QColor(90,90,90) );
	    if ( n != 3 )
		p.drawLine( ox+11, oy+i, ox+i, oy+11 );
	}
    }

    void mouseReleaseEvent(QMouseEvent*)
    {
	QMessageBox::message("Notice", "The corner widget\ndoes not work yet.");
    }
};

class ScrollViewExample : public QWidget {
    Q_OBJECT

public:
    ScrollViewExample(int technique)
    {
	QMenuBar* menubar = new QMenuBar(this);
	CHECK_PTR( menubar );

	QPopupMenu* file = new QPopupMenu();
	CHECK_PTR( file );
	menubar->insertItem( "&File", file );
	file->insertItem( "Quit", qApp,  SLOT(quit()) );

	vp_options = new QPopupMenu();
	CHECK_PTR( vp_options );
	vp_options->setCheckable( TRUE );
	menubar->insertItem( "&ScrollView", vp_options );
	connect( vp_options, SIGNAL(activated(int)),
	    this, SLOT(doVPMenuItem(int)) );

	vauto_id = vp_options->insertItem( "Vertical Auto" );
	vaoff_id = vp_options->insertItem( "Vertical AlwaysOff" );
	vaon_id = vp_options->insertItem( "Vertical AlwaysOn" );
	vp_options->insertSeparator();
	hauto_id = vp_options->insertItem( "Horizontal Auto" );
	haoff_id = vp_options->insertItem( "Horizontal AlwaysOff" );
	haon_id = vp_options->insertItem( "Horizontal AlwaysOn" );
	vp_options->insertSeparator();
	corn_id = vp_options->insertItem( "cornerWidget" );

	if (technique == 1) {
	    vp = new QScrollView(this);
	    BigShrinker *bs = new BigShrinker;
	    vp->addChild(bs);
	    QObject::connect(bs, SIGNAL(clicked(int,int)),
			    vp, SLOT(center(int,int)));
	} else {
	    vp = new BigMatrix(this);
	}

	f_options = new QPopupMenu();
	CHECK_PTR( f_options );
	f_options->setCheckable( TRUE );
	menubar->insertItem( "&Frame", f_options );
	connect( f_options, SIGNAL(activated(int)),
	    this, SLOT(doFMenuItem(int)) );

	f_options->insertItem( "No frame", style_id );
	f_options->insertItem( "Box", style_id|QFrame::Box );
	f_options->insertItem( "Panel", style_id|QFrame::Panel );
	f_options->insertItem( "WinPanel", style_id|QFrame::WinPanel );
	f_options->insertSeparator();
	f_options->insertItem( "Plain", style_id|QFrame::Plain );
	f_options->insertItem( "Raised", style_id|QFrame::Raised );
	f_laststyle = f_options->indexOf(
	    f_options->insertItem( "Sunken", style_id|QFrame::Sunken ));
	f_options->insertSeparator();
	lw_options = new QPopupMenu;
	CHECK_PTR( lw_options );
	lw_options->setCheckable( TRUE );
	for (int lw = 1; lw <= max_lw; lw++) {
	    QString str;
	    str.sprintf("%d pixels", lw);
	    lw_options->insertItem( str, lw_id | lw );
	}
	f_options->insertItem( "Line width", lw_options );
	connect( lw_options, SIGNAL(activated(int)),
	    this, SLOT(doFMenuItem(int)) );
	mlw_options = new QPopupMenu;
	CHECK_PTR( mlw_options );
	mlw_options->setCheckable( TRUE );
	for (int mlw = 0; mlw <= max_mlw; mlw++) {
	    QString str;
	    str.sprintf("%d pixels", mlw);
	    mlw_options->insertItem( str, mlw_id | mlw );
	}
	f_options->insertItem( "Midline width", mlw_options );
	connect( mlw_options, SIGNAL(activated(int)),
	    this, SLOT(doFMenuItem(int)) );
	mw_options = new QPopupMenu;
	CHECK_PTR( mw_options );
	mw_options->setCheckable( TRUE );
	for (int mw = 0; mw <= max_mw; mw++) {
	    QString str;
	    str.sprintf("%d pixels", mw);
	    mw_options->insertItem( str, mw_id | mw );
	}
	f_options->insertItem( "Margin width", mw_options );
	connect( mw_options, SIGNAL(activated(int)),
	    this, SLOT(doFMenuItem(int)) );

	setFMenuItems();

	QVBoxLayout* vbox = new QVBoxLayout(this);
	vbox->setMenuBar(menubar);
	menubar->setSeparator(QMenuBar::InWindowsStyle);
	vbox->addWidget(vp);
	vbox->activate();

	corner = new WinCorner(this);
	corner->hide();
    }

private slots:
    void doVPMenuItem(int id)
    {
	if (id == vauto_id ) {
	    vp->setVScrollBarMode(QScrollView::Auto);
	} else if (id == vaoff_id) {
	    vp->setVScrollBarMode(QScrollView::AlwaysOff);
	} else if (id == vaon_id) {
	    vp->setVScrollBarMode(QScrollView::AlwaysOn);
	} else if (id == hauto_id) {
	    vp->setHScrollBarMode(QScrollView::Auto);
	} else if (id == haoff_id) {
	    vp->setHScrollBarMode(QScrollView::AlwaysOff);
	} else if (id == haon_id) {
	    vp->setHScrollBarMode(QScrollView::AlwaysOn);
	} else if (id == corn_id) {
	    bool corn = !vp->cornerWidget();
	    vp->setCornerWidget(corn ? corner : 0);
	} else {
	    return; // Not for us to process.
	}
	setVPMenuItems();
    }

    void setVPMenuItems()
    {
	QScrollView::ScrollBarMode vm = vp->vScrollBarMode();
	vp_options->setItemChecked( vauto_id, vm == QScrollView::Auto );
	vp_options->setItemChecked( vaoff_id, vm == QScrollView::AlwaysOff );
	vp_options->setItemChecked( vaon_id, vm == QScrollView::AlwaysOn );

	QScrollView::ScrollBarMode hm = vp->hScrollBarMode();
	vp_options->setItemChecked( hauto_id, hm == QScrollView::Auto );
	vp_options->setItemChecked( haoff_id, hm == QScrollView::AlwaysOff );
	vp_options->setItemChecked( haon_id, hm == QScrollView::AlwaysOn );

	vp_options->setItemChecked( corn_id, !!vp->cornerWidget() );
    }

    void doFMenuItem(int id)
    {
	if (id & style_id) {
	    int sty;

	    if (id == style_id) {
		sty = 0;
	    } else if (id & QFrame::MShape) {
		sty = vp->frameStyle()&QFrame::MShadow;
		sty = (sty ? sty : QFrame::Plain) | id&QFrame::MShape;
	    } else {
		sty = vp->frameStyle()&QFrame::MShape;
		sty = (sty ? sty : QFrame::Box) | id&QFrame::MShadow;
	    }
	    vp->setFrameStyle(sty);
	} else if (id & lw_id) {
	    vp->setLineWidth(id&~lw_id);
	} else if (id & mlw_id) {
	    vp->setMidLineWidth(id&~mlw_id);
	} else {
	    vp->setMargin(id&~mw_id);
	}

	vp->update();
	setFMenuItems();
    }

    void setFMenuItems()
    {
	int sty = vp->frameStyle();

	f_options->setItemChecked( style_id, !sty );

	for (int i=1; i <= f_laststyle; i++) {
	    int id = f_options->idAt(i);
	    if (id & QFrame::MShape)
		f_options->setItemChecked( id,
		    ((id&QFrame::MShape) == (sty&QFrame::MShape)) );
	    else
		f_options->setItemChecked( id,
		    ((id&QFrame::MShadow) == (sty&QFrame::MShadow)) );
	}

	for (int lw=1; lw<=max_lw; lw++)
	    lw_options->setItemChecked( lw_id|lw, vp->lineWidth() == lw );

	for (int mlw=0; mlw<=max_mlw; mlw++)
	    mlw_options->setItemChecked( mlw_id|mlw, vp->midLineWidth() == mlw );

	for (int mw=0; mw<=max_mw; mw++)
	    mw_options->setItemChecked( mw_id|mw, vp->margin() == mw );
    }

private:
    QScrollView* vp;
    QPopupMenu* vp_options;
    QPopupMenu* f_options;
    QPopupMenu* lw_options;
    QPopupMenu* mlw_options;
    QPopupMenu* mw_options;
    WinCorner* corner;

    int vauto_id, vaoff_id, vaon_id,
	hauto_id, haoff_id, haon_id,
	corn_id;

    int f_laststyle;
};

int main( int argc, char **argv )
{
    QApplication a( argc, argv );

    ScrollViewExample ve1(1);
    ScrollViewExample ve2(2);

    ve1.show();
    ve2.show();
    ve2.resize(400,300+(new QMenuBar())->height());

    QObject::connect(qApp, SIGNAL(lastWindowClosed()), qApp, SLOT(quit()));

    return a.exec();
}

#include "scrollview.moc"
