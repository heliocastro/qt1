/****************************************************************************
** $Id: globjwin.h,v 1.1.2.1 1998/11/30 10:47:00 aavit Exp $
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
class QLabel;
class QPopupMenu;

class GLObjectWindow : public QWidget
{
    Q_OBJECT
public:
    GLObjectWindow( QWidget* parent = 0, const char* name = 0 );

protected slots:

    void		makePixmap();
    void		makePixmapManually();
    void		makePixmapHidden();
    void		makePixmapHiddenManually();
    void		useFixedPixmapSize();

private:
    void		drawOnPixmap( QPixmap* pm );
    GLBox* c1;
    QLabel* lb;
    int fixMenuItemId;
    QSize pmSz;
    QPopupMenu* file;
};


#endif // GLOBJWIN_H
