TEMPLATE	= app
CONFIG		= qt warn_on release
#  To add the Qt Image IO Extension:
#DEFINES          =   QIMGIO
#unix:TMAKE_LIBS =       -L$(QTDIR)/lib -lqimgio -ljpeg -lpng -lz -lm
HEADERS		= showimg.h
SOURCES		= main.cpp \
		  showimg.cpp
TARGET		= showimg
