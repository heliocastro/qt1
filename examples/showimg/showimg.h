/****************************************************************************
** $Id: showimg.h,v 2.10 1998/05/21 19:24:57 agulbra Exp $
**
** Copyright (C) 1992-1998 Troll Tech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#ifndef SHOWIMG_H
#define SHOWIMG_H

#include <qwidget.h>
#include <qimage.h>


class QLabel;
class QMenuBar;
class QPopupMenu;

class ImageViewer : public QWidget
{
    Q_OBJECT
public:
    ImageViewer( QWidget *parent=0, const char *name=0, int wFlags=0 );
    ~ImageViewer();
    bool	loadImage( const char *fileName );
protected:
    void	paintEvent( QPaintEvent * );
    void	resizeEvent( QResizeEvent * );
    void	mousePressEvent( QMouseEvent * );
    void	mouseReleaseEvent( QMouseEvent * );
    void	mouseMoveEvent( QMouseEvent * );

private:
    void	scale();
    int		conversion_flags;
    bool	smooth() const;
    bool	useColorContext() const;
    int		alloc_context;
    bool	convertEvent( QMouseEvent* e, int& x, int& y );
    const char* filename;
    QImage	image;			// the loaded image
    QPixmap	pm;			// the converted pixmap
    QPixmap	pmScaled;		// the scaled pixmap
    QMenuBar   *menubar;
    QPopupMenu *file;
    QPopupMenu *options;
    QPopupMenu *saveimage;
    QPopupMenu *savepixmap;
    QWidget    *helpmsg;
    QLabel     *status;
    int         si, sp, ac, co, mo, fd, bd, // Menu item ids
		td, ta, ba, fa, au, ad, dd,
		ss, cc;
    void	updateStatus();
    void	setMenuItemFlags();
    bool 	reconvertImage();
    int		pickx, picky;
    int		clickx, clicky;
    bool	may_be_other;
    static ImageViewer* other;

private slots:
    void	newWindow();
    void	openFile();
    void	saveImage(int);
    void	savePixmap(int);
    void	giveHelp();
    void	doOption(int);
    void	copyFrom(ImageViewer*);
};


#endif // SHOWIMG_H
