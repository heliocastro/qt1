/****************************************************************
**
** Qt tutorial 6
**
****************************************************************/

#include <qapplication.h>
#include <qpushbutton.h>
#include <qscrollbar.h>
#include <qlcdnumber.h>
#include <qfont.h>


class LCDRange : public QWidget
{
public:
    LCDRange( QWidget *parent=0, const char *name=0 );
protected:
    void resizeEvent( QResizeEvent * );
private:
    QScrollBar  *sBar;
    QLCDNumber  *lcd;
};

LCDRange::LCDRange( QWidget *parent, const char *name )
        : QWidget( parent, name )
{
    lcd  = new QLCDNumber( 2, this, "lcd"  );
    lcd->move( 0, 0 );
    sBar = new QScrollBar( 0, 99,		       	// range
			   1, 10, 			// line/page steps
			   0, 				// inital value
			   QScrollBar::Horizontal, 	// orientation
                           this, "scrollbar" );
    connect( sBar, SIGNAL(valueChanged(int)), lcd, SLOT(display(int)) );
}

void LCDRange::resizeEvent( QResizeEvent * )
{
    sBar->setGeometry( 0, height() - 16, width(), 16 );
    lcd->resize( width(), sBar->y() - 5 );
}


class MyWidget : public QWidget
{
public:
    MyWidget( QWidget *parent=0, const char *name=0 );
protected:
    void resizeEvent( QResizeEvent * );
private:
    QPushButton *quit;
    LCDRange *value[16];
};


MyWidget::MyWidget( QWidget *parent, const char *name )
        : QWidget( parent, name )
{
    setMinimumSize( 200, 300 );

    quit = new QPushButton( "Quit", this, "quit" );
    quit->setGeometry( 10, 10, 75, 30 );
    quit->setFont( QFont( "Times", 18, QFont::Bold ) );

    connect( quit, SIGNAL(clicked()), qApp, SLOT(quit()) );

    for( int i = 0 ; i < 16 ; i++ )
	value[i] = new LCDRange( this );
}

void MyWidget::resizeEvent( QResizeEvent * )
{
    int startx      = 10;
    int starty      = quit->y() + quit->height() + 10;
    int valueWidth  = (width() - startx - 10 - 3*5)/4;
    int valueHeight = (height() - starty - 10 - 3*5)/4;
    for( int i = 0 ; i < 16 ; i++ )
	value[i]->setGeometry( startx + (i%4)*(5+valueWidth),
			       starty + (i/4)*(5+valueHeight),
                               valueWidth, valueHeight );
}

int main( int argc, char **argv )
{
    QApplication a( argc, argv );

    MyWidget w;
    w.setGeometry( 100, 100, 400, 400 );
    a.setMainWidget( &w );
    w.show();
    return a.exec();
}
