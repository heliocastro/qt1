/****************************************************************************
** $Id: qspinbox.h,v 2.22.2.2 1998/09/28 13:18:12 aavit Exp $
**
** Definition of QSpinBox widget class
**
** Created : 1997
**
** Copyright (C) 1992-1999 Troll Tech AS.  All rights reserved.
**
** This file is part of Qt Free Edition, version 1.45.
**
** See the file LICENSE included in the distribution for the usage
** and distribution terms, or http://www.troll.no/free-license.html.
**
** IMPORTANT NOTE: You may NOT copy this file or any part of it into
** your own programs or libraries.
**
** Please see http://www.troll.no/pricing.html for information about 
** Qt Professional Edition, which is this same library but with a
** license which allows creation of commercial/proprietary software.
**
*****************************************************************************/

#ifndef QSPINBOX_H
#define QSPINBOX_H

#ifndef QT_H
#include "qframe.h"
#include "qrangecontrol.h"
#endif // QT_H

class QPushButton;
class QLineEdit;
class QValidator;
struct QSpinBoxData;


class Q_EXPORT QSpinBox: public QFrame, public QRangeControl
{
    Q_OBJECT
public:
    QSpinBox( QWidget* parent = 0, const char* name = 0 );
    QSpinBox( int minValue, int maxValue, int step = 1,
	      QWidget* parent = 0, const char* name = 0 );
    ~QSpinBox();

    const char* 	text() const;
    virtual const char*	prefix() const;
    virtual const char*	suffix() const;
    virtual QString 	cleanText() const;

    void		setSpecialValueText( const char* text );
    const char* 	specialValueText() const;

    void 		setWrapping( bool on );
    bool 		wrapping() const;

    void		setValidator( QValidator* v );

    QSize 		sizeHint() const;

public slots:
    virtual void	setValue( int value );
    virtual void	setPrefix( const char* text );
    virtual void	setSuffix( const char* text );
    virtual void	stepUp();
    virtual void	stepDown();

signals:
    void		valueChanged( int value );
    void		valueChanged( const char* valueText );

protected:
    virtual QString	mapValueToText( int value );
    virtual int		mapTextToValue( bool* ok );
    QString		currentValueText();

    virtual void	updateDisplay();
    virtual void	interpretText();

    QPushButton*	upButton() const;
    QPushButton*	downButton() const;
    QLineEdit*		editor() const;

    virtual void	valueChange();
    virtual void	rangeChange();

    bool		eventFilter( QObject* obj, QEvent* ev );
    void		resizeEvent( QResizeEvent* ev );

    void		paletteChange( const QPalette& );
    void		enabledChange( bool );
    void		fontChange( const QFont& );
    void		styleChange( GUIStyle );

protected slots:
    void		textChanged();

private:
    void initSpinBox();
    struct QSpinBoxData* extra;
    QPushButton* up;
    QPushButton* down;
    QLineEdit* vi;
    QValidator* validator;
    QString pfix;
    QString sfix;
    QString specText;
    bool wrap;
    bool edited;

private:	// Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QSpinBox( const QSpinBox& );
    QSpinBox& operator=( const QSpinBox& );
#endif

};


#endif
