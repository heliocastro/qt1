/****************************************************************************
** $Id: qimageio.cpp,v 1.3 1998/07/03 00:09:26 hanord Exp $
**
** Implementation of QImage IO Library API
**
** Created : 970521
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

#include "qjpegio.h"
#include "qpngio.h"

void qInitImageIO()
{
    qInitJpegIO();
    qInitPngIO();
}
