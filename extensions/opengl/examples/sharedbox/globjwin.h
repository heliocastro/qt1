/****************************************************************************
** $Id: globjwin.h,v 1.1 1998/03/23 11:44:44 aavit Exp $
**
** Definition of GLObjectWindow widget class
** The GLObjectWindow contains a GLBox and three sliders connected to
** the GLBox's rotation slots.
**
****************************************************************************/

#ifndef GLOBJWIN_H
#define GLOBJWIN_H

#include <qwidget.h>
class GLBox;

class GLObjectWindow : public QWidget
{
    Q_OBJECT
public:
    GLObjectWindow( QWidget* parent = 0, const char* name = 0 );

protected slots:

    void		deleteFirstWidget();

private:
    GLBox* c1;
    GLBox* c2;
};


#endif // GLOBJWIN_H
