include config.mak

SUBDIRS = backend timetaggerd doc

ifdef WITH_WIN32
SUBDIRS += win32 
endif

ifdef WITH_WIN64
SUBDIRS += win64 
endif

ifdef WITH_ARM
SUBDIRS += arm-linux
endif

ifdef WITH_ANDROID
SUBDIRS += android
endif

.PHONY:	all clean

all: 
	@for i in $(SUBDIRS); do make -C $$i all || exit 1; done
	make -C setup all;

clean:
	@for i in $(SUBDIRS); do make -C $$i clean || exit 1; done	

dist-clean:
	@for i in $(SUBDIRS); do make -C $$i dist-clean || exit 1; done	

