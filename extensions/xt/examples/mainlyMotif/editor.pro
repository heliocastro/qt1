TEMPLATE    	=	app
CONFIG      	=	qt debug
DEFINES		=	HAVE_MOTIF
HEADERS	    	=   
SOURCES     	=	editor.cpp
TMAKE_LIBS	=	-L$(QTDIR)/lib -lqxt -lXaw -lXm -lXpm -lXt -lm
TARGET      	=	editor
