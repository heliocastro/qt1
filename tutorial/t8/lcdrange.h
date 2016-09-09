/****************************************************************
**
** Definition of LCDRange class, Qt tutorial 8
**
****************************************************************/

#ifndef LCDRANGE_H
#define LCDRANGE_H

#include <qwidget.h>

class QScrollBar;
class QLCDNumber;


class LCDRange : public QWidget
{
    Q_OBJECT
public:
    LCDRange( QWidget *parent=0, const char *name=0 );

    int value() const;
public slots:
    void setValue( int );
    void setRange( int minVal, int maxVal );
signals:
    void valueChanged( int );
protected:
    void resizeEvent( QResizeEvent * );
private:
    QScrollBar  *sBar;
    QLCDNumber  *lcd;
};

#endif // LCDRANGE_H
