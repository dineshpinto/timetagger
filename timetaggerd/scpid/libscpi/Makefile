#include ../../config.mak

CFLAGS+=-O2 -g -Wall -fPIC

ifeq ($(PLATFORM),"linux")
CFLAGS+=-DLINUX
else
ifeq ($(PLATFORM),"mingw")
CFLAGS+=-DWIN32
endif
endif

SOURCES=debug.o error.o fifo.o ieee488.o minimal.o parser.o units.o utils.o


all: libscpi.a

clean: 
	rm -f libscpi.a *.o

libscpi.a: $(SOURCES)
	ar r $@ $?
	
%.o: %.c
	gcc -c $(CFLAGS)  $<
 


