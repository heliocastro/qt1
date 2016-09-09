/****************************************************************
**
** Definition of GameBoard class, Qt tutorial 14
**
****************************************************************/

#ifndef GAMEBRD_H
#define GAMEBRD_H

#include <qwidget.h>

class QPushButton;
class LCDRange;
class QLCDNumber;
class QFrame;
class CannonField;

#include "lcdrange.h"
#include "cannon.h"


class GameBoard : public QWidget
{
    Q_OBJECT
public:
    GameBoard( QWidget *parent=0, const char *name=0 );
protected:
    void  resizeEvent( QResizeEvent * );
protected slots:
    void  fire();
    void  hit();
    void  missed();
    void  newGame();
private:
    QPushButton *quit;
    QPushButton *shoot;
    QPushButton *restart;
    LCDRange    *angle;
    LCDRange    *force;
    QLCDNumber  *hits;
    QLCDNumber  *shotsLeft;
    QFrame	*frame;
    CannonField *cannonField;
};

#endif // GAMEBRD_H
