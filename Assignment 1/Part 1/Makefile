#! /usr/bin/make

INCLUDEDIR=/usr/include/
LIBDIR=/usr/lib

SOURCE= FruitTetris.cpp include/InitShader.cpp

# The compiler 
CC= g++

CFLAGS= -O3 -g -Wall -pedantic -DGL_GLEXT_PROTOTYPES

# The name of the final executable 
EXECUTABLE= FruitTetris

# Linux (default)
LDFLAGS = -lGL -lglut -lGLEW -lXext -lX11 -lm

INCLUDEFLAG= -I. -I$(INCLUDEDIR) -Iinclude/
LIBFLAG= -L$(LIBDIR)

OBJECT= $(SOURCE:.cpp=.o)

all: $(OBJECT) depend
	$(CC) $(CFLAGS) $(INCLUDEFLAG) $(LIBFLAG) $(OBJECT) -o $(EXECUTABLE) $(LDFLAGS)

depend:
	$(CC) -M $(SOURCE) > depend

$(OBJECT):
	$(CC) $(CFLAGS) $(INCLUDEFLAG) -c -o $@ $(@:.o=.cpp)


include depend
