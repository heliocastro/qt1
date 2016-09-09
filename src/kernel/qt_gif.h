/****************************************************************************
** $Id: qt_gif.h,v 1.1.2.4 1999/02/22 20:54:45 eiriken Exp $
**
** To enable built-in reading of GIF images in Qt, change the definition
** below to "#define QT_BUILTIN_GIF_READER 1".
**
** To disable built-in reading of GIF images in Qt, change the definition
** below to "#define QT_BUILTIN_GIF_READER 0".
**
** WARNING:
**      A separate license from Unisys may be required to use the gif
**      reader. See http://corp2.unisys.com/LeadStory/lzwfaq.html
**      for information from Unisys
**
*****************************************************************************/

#define QT_BUILTIN_GIF_READER 1

bool qt_builtin_gif_reader();
