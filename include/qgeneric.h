/****************************************************************************
** $Id: qgeneric.h,v 2.12 1998/07/03 00:09:45 hanord Exp $
**
** Macros for pasting tokens; utilized by our generic classes
**
** Created : 920529
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

#ifndef QGENERIC_H
#define QGENERIC_H

#ifndef QT_H
#include "qglobal.h"
#endif // QT_H


// Try to include the system defines where it is sure to exist
#if defined(_CC_SUN_) || (defined(_CC_EDG_) && defined(_OS_IRIX_))
#include <generic.h>
#endif


// Define Qt generic macros
// At some time in the future, these will be the only #defines left here

#if defined(QT_ADD_GENERIC_MACROS)

#define Q_NAME2(a,b)		Q_NAME2_AUX(a,b)
#define Q_NAME2_AUX(a,b)	a##b
#define Q_DECLARE(a,t)		Q_NAME2(a,declare)(t)

#endif // QT_ADD_GENERIC_MACROS


// Standard token-pasting macros for ANSI C preprocessors
// We will remove these from Qt in version 2.0 or 3.0

#if !defined(declare)

#define name2(a,b)		_name2_aux(a,b)
#define _name2_aux(a,b)		a##b
#define name3(a,b,c)		_name3_aux(a,b,c)
#define _name3_aux(a,b,c)	a##b##c
#define name4(a,b,c,d)		_name4_aux(a,b,c,d)
#define _name4_aux(a,b,c,d)	a##b##c##d

#define declare(a,t)		name2(a,declare)(t)
#define implement(a,t)		name2(a,implement)(t)
#define declare2(a,t1,t2)	name2(a,declare2)(t1,t2)
#define implement2(a,t1,t2)	name2(a,implement2)(t1,t2)

#endif // !declare


#endif // QGENERIC_H
