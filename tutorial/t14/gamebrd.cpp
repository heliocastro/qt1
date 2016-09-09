/****************************************************************
**
** Implementation of GameBoard class, Qt tutorial 14
**
****************************************************************/

#include "gamebrd.h"

#include <qfont.h>
#include <qapplication.h>
#include <qlabel.h>
#include <qaccel.h>
#include <qpushbutton.h>
#include <qlcdnumber.h>

#include "lcdrange.h"
#include "cannon.h"

GameBoard::GameBoard( QWidget *parent, const char *name )
        : QWidget( parent, name )
{
    setMinimumSize( 500, 355 );

    quit = new QPushButton( "Quit", this, "quit" );
    quit->setFont( QFont( "Times", 18, QFont::Bold ) );

    connect( quit, SIGNAL(clicked()), qApp, SLOT(quit()) );

    angle  = new LCDRange( "ANGLE", this, "angle" );
    angle->setRange( 5, 70 );

    force  = new LCDRange( "FORCE", this, "force" );
    force->setRange( 10, 50 );

    frame = new QFrame( this, "cannonFrame" );
    frame->setFrameStyle( QFrame::WinPanel | QFrame::Sunken );

    cannonField = new CannonField( this, "cannonField" );
    cannonField->setBackgroundColor( QColor( 250, 250, 200) );

    connect( angle,SIGNAL(valueChanged(int)), cannonField,SLOT(setAngle(int)));
    connect( cannonField,SIGNAL(angleChanged(int)), angle,SLOT(setValue(int)));

    connect( force,SIGNAL(valueChanged(int)), cannonField,SLOT(setForce(int)));
    connect( cannonField,SIGNAL(forceChanged(int)), force,SLOT(setValue(int)));

    connect( cannonField, SIGNAL(hit()),SLOT(hit()) );
    connect( cannonField, SIGNAL(missed()),SLOT(missed()) );

    angle->setValue( 60 );
    force->setValue( 25 );

    shoot = new QPushButton( "Shoot", this, "shoot" );
    shoot->setFont( QFont( "Times", 18, QFont::Bold ) );

    connect( shoot, SIGNAL(clicked()), SLOT(fire()) );

    restart = new QPushButton( "New Game", this, "newgame" );
    restart->setFont( QFont( "Times", 18, QFont::Bold ) );

    connect( restart, SIGNAL(clicked()), SLOT(newGame()) );

    hits  	       = new QLCDNumber( 2, this, "hits" );
    shotsLeft 	       = new QLCDNumber( 2, this, "shotsleft" );
    QLabel *hitsL      = new QLabel( "HITS", this, "hitsLabel" );
    QLabel *shotsLeftL = new QLabel( "SHOTS LEFT", this, "shotsleftLabel" );

    QAccel *accel = new QAccel( this );
    accel->connectItem( accel->insertItem( Key_Space), this, SLOT(fire()) );
    accel->connectItem( accel->insertItem( Key_Q), qApp, SLOT(quit()) );

    quit->setGeometry( 10, 10, 75, 30 );
    angle->setGeometry( 10, quit->y() + quit->height() + 10, 75, 130 );
    force->setGeometry( 10, angle->y() + angle->height() + 10, 75, 130 );
    frame->move( angle->x() + angle->width() + 10, angle->y() );
    cannonField->move( frame->x() + 2, frame->y() + 2 );
    shoot->setGeometry( 10, 315, 75, 30 );
    restart->setGeometry( 380, 10, 110, 30 );
    hits->setGeometry( 130, 10, 40, 30 );
    hitsL->setGeometry( hits->x() + hits->width() + 5, 10, 60, 30 );
    shotsLeft->setGeometry( 240, 10, 40, 30 );
    shotsLeftL->setGeometry( shotsLeft->x()+shotsLeft->width()+5, 10, 70, 30 );

    newGame();
}

void GameBoard::resizeEvent( QResizeEvent * )
{
    frame->resize( width()  - frame->x() - 10,
		   height() - frame->y() - 10 );
    cannonField->resize( frame->width() - 4, frame->height() - 4 );
}

void GameBoard::fire()
{
    if ( cannonField->gameOver() || cannonField->isShooting() )
	return;
    shotsLeft->display( shotsLeft->intValue() - 1 );
    cannonField->shoot();
}

void GameBoard::hit()
{
    hits->display( hits->intValue() + 1 );
    if ( shotsLeft->intValue() == 0 )
	cannonField->setGameOver();
    else
	cannonField->newTarget();
}

void GameBoard::missed()
{
    if ( shotsLeft->intValue() == 0 )
	cannonField->setGameOver();
}

void GameBoard::newGame()
{
    shotsLeft->display( 15 );
    hits->display( 0 );
    cannonField->restartGame();
    cannonField->newTarget();
}
