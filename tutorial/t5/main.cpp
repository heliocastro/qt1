/****************************************************************
**
** Qt tutorial 5
**
****************************************************************/

#include <qapplication.h>
#include <qpushbutton.h>
#include <qscrollbar.h>
#include <qlcdnumber.h>
#include <qfont.h>

class MyWidget : public QWidget
{
public:
    MyWidget( QWidget *parent=0, const char *name=0 );
protected:
    void resizeEvent( QResizeEvent * );
private:
    QPushButton *quit;
    QScrollBar  *sBar;
    QLCDNumber  *lcd;
};


MyWidget::MyWidget( QWidget *parent, const char *name )
        : QWidget( parent, name )
{
    setMinimumSize( 200, 200 );

    quit = new QPushButton( "Quit", this, "quit" );
    quit->setGeometry( 10, 10, 75, 30 );
    quit->setFont( QFont( "Times", 18, QFont::Bold ) );

    connect( quit, SIGNAL(clicked()), qApp, SLOT(quit()) );

    lcd  = new QLCDNumber( 2, this, "lcd" );
    lcd->move( 10, quit->y() + quit->height() + 10 );

    sBar = new QScrollBar( 0, 99,		       	// range
			   1, 10, 			// line/page steps
			   0, 				// inital value
			   QScrollBar::Horizontal, 	// orientation
                           this, "scrollbar" );

    connect( sBar, SIGNAL(valueChanged(int)), lcd, SLOT(display(int)) );
}

void MyWidget::resizeEvent( QResizeEvent * )
{
    sBar->setGeometry( 10, height() - 10 - 16, width() - 20, 16 );
    lcd->resize( sBar->width(), sBar->y() - lcd->y() - 5 );
}

int main( int argc, char **argv )
{
    QApplication a( argc, argv );

    MyWidget w;
    w.setGeometry( 100, 100, 200, 200 );
    a.setMainWidget( &w );
    w.show();
    return a.exec();
}
