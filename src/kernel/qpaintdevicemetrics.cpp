/****************************************************************************
** $Id: qpaintdevicemetrics.cpp,v 2.5 1998/07/03 00:09:36 hanord Exp $
**
** Implementation of QPaintDeviceMetrics class
**
** Created : 941109
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

#include "qpaintdevicemetrics.h"

/*!
  \class QPaintDeviceMetrics qpaintdevicemetrics.h
  \brief The QPaintDeviceMetrics class provides information about a
  paint device.

  \ingroup drawing

  Sometimes it is necessary to obtain information about the
  physical size of a paint device when drawing graphics.

  Example:
  \code
    QPaintDeviceMetrics pdm( myWidget );
    float aspect = (float)pdm.widthMM / (float)pdm.heightMM();
  \endcode
*/

/*!
  Constructs a metric for the paint device \e pd.
*/
QPaintDeviceMetrics::QPaintDeviceMetrics( const QPaintDevice *pd )
{
    pdev = (QPaintDevice *)pd;
}


/*!
  \fn int QPaintDeviceMetrics::width() const

  Returns the width of the paint device, in default coordinate system
  units (e.g. pixels for QPixmap and QWidget).
*/

/*!
  \fn int QPaintDeviceMetrics::height() const

  Returns the height of the paint device, in default coordinate system
  units (e.g. pixels for QPixmap and QWidget).
*/

/*!
  \fn int QPaintDeviceMetrics::widthMM() const
  Returns the width of the paint device, measured in millimeters.
*/

/*!
  \fn int QPaintDeviceMetrics::heightMM() const
  Returns the height of the paint device, measured in millimeters.
*/

/*!
  \fn int QPaintDeviceMetrics::numColors() const
  Returns the number of different colors available for the paint device.
*/

/*!
  \fn int QPaintDeviceMetrics::depth() const
  Returns the bit depth (number of bit planes) of the paint device.
*/
