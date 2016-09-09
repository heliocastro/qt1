#include "qxt.h"
#include <qmultilinedit.h>
#include <qpainter.h>
#include <qmessagebox.h>
#include <X11/Shell.h>
#include <X11/StringDefs.h>
#include <X11/Xaw/AsciiText.h>

static const char* QTEDMSG =
    "This is a Qt widget.\nIt is a QMultiLineEdit.";

static const char* XTEDMSG =
    "This is an Xt widget.\nIt is an asciiTextWidgetClass.";


class EncapsulatedXtWidget : public QXtWidget {
public:
    EncapsulatedXtWidget(QXtWidget* parent) :
	QXtWidget("editor", asciiTextWidgetClass, parent)
    {
	Arg args[20];
	Cardinal nargs=0;
	XtSetArg(args[nargs], XtNeditType, XawtextEdit); nargs++;
	XtSetArg(args[nargs], XtNstring, XTEDMSG);       nargs++;
	XtSetValues(xtWidget(), args, nargs);
	XtMapWidget(xtWidget());
    }
};

class TwoEditors : public QXtWidget {
    QMultiLineEdit* qtchild;
    EncapsulatedXtWidget* xtchild;

public:
    TwoEditors() :
	QXtWidget("editors", topLevelShellWidgetClass, 0, 0, 0, FALSE)
    {
	qtchild = new QMultiLineEdit(this);
	qtchild->setText(QTEDMSG);
	xtchild = new EncapsulatedXtWidget(this);
    }

    void resizeEvent(QResizeEvent*)
    {
	int marg = 10;
	int w = (width()-marg*3)/2;
	int h = height()-marg*2;

	qtchild->setGeometry(marg,marg,w,h);
	xtchild->setGeometry(marg+w+marg,marg,w,h);
    }
};

main(int argc, char** argv)
{
    QXtApplication app(argc, argv, "TwoEditors");
    TwoEditors m;
    app.setMainWidget(&m);
    m.show();
    return app.exec();
}
