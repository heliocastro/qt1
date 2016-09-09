/****************************************************************
**
** Qt tutorial 7
**
****************************************************************/

#include <qapplication.h>
#include <qpushbutton.h>
#include <qscrollbar.h>
#include <qlcdnumber.h>
#include <qfont.h>

#include "lcdrange.h"


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

    for( int i = 0 ; i < 16 ; i++ ) {
	value[i] = new LCDRange( this );
	if ( i > 0 )
	    connect( value[i], SIGNAL(valueChanged(int)), 
		     value[i - 1], SLOT(setValue(int)) );
    }
}

void MyWidget::resizeEvent( QResizeEvent * )
{
    int valueWidth = (width() - 20)/4;
    int valueHeight = (height() - 65)/4;
    for( int i = 0 ; i < 16 ; i++ )
	value[i]->setGeometry( 10 + (i%4)*valueWidth,  55 + (i/4)*valueHeight,
                               valueWidth - 5, valueHeight - 5 );
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
