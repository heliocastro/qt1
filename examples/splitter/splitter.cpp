#include <qapplication.h>
#include <qlabel.h>
#include <qsplitter.h>
#include <qmultilinedit.h>

#include <qpainter.h>


class Test : public QWidget {
public:
    Test(QWidget* parent=0, const char* name=0, int f=0);
    void paintEvent(QPaintEvent* e);
private:
};



Test::Test(QWidget* parent, const char* name, int f) :
    QWidget(parent, name, f)
{
}

void Test::paintEvent(QPaintEvent* e)
{
    QPainter p(this);
    p.setClipRect(e->rect());
    const int d = 1000; //large number
    int x1 = 0;
    int x2 = width()-1;
    int y1 = 0;
    int y2 = height()-1;

    int x = (x1+x2)/2;
    p.drawLine( x, y1, x+d, y1+d   );
    p.drawLine( x, y1, x-d, y1+d   );
    p.drawLine( x, y2, x+d, y2-d   );
    p.drawLine( x, y2, x-d, y2-d   );

    int y = (y1+y2)/2;
    p.drawLine( x1, y, x1+d, y+d   );
    p.drawLine( x1, y, x1+d, y-d   );
    p.drawLine( x2, y, x2-d, y+d   );
    p.drawLine( x2, y, x2-d, y-d   );
}


int main( int argc, char ** argv )
{
    QApplication a( argc, argv );

    QSplitter *s1 = new QSplitter( QSplitter::Vertical, 0 , "main" );

    QSplitter *s2 = new QSplitter( QSplitter::Horizontal, s1, "top" );

    Test *t1 = new Test( s2 );
    t1->setBackgroundColor( blue.light( 180 ) );
    t1->setMinimumSize( 50, 0 );

    Test *t2 = new Test( s2 );
    t2->setBackgroundColor( green.light( 180 ) );
    s2->setResizeMode( t2, QSplitter::KeepSize );
    s2->moveToFirst( t2 );

    QSplitter *s3 = new QSplitter( QSplitter::Horizontal,  s1, "bottom" );

    // s4 is nested inside s3 - allowing 3 widgets to be split
    QSplitter *s4 = new QSplitter( QSplitter::Horizontal,  s3, "bottom" );

    Test *t3 = new Test( s4 );
    t3->setBackgroundColor( red );

    Test *t4 = new Test( s4 );
    t4->setBackgroundColor( white );

    Test *t5 = new Test( s3 );
    t5->setMaximumHeight( 250 );
    t5->setMinimumSize( 80, 50 );
    t5->setBackgroundColor( yellow );

    // Test widgets draw fast...
    //s1->setOpaqueResize( TRUE );
    s2->setOpaqueResize( TRUE );
    s3->setOpaqueResize( TRUE );
    s4->setOpaqueResize( TRUE );

    a.setMainWidget( s1 );
    s1->show();
    return a.exec();
}
