TEMPLATE	= app
CONFIG		= qt warn_on release
HEADERS		= motor.h \
		  vw.h
SOURCES		= main.cpp \
		  motor.cpp \
		  vw.cpp
unix:LIBS	= -lm
TARGET		= validator
