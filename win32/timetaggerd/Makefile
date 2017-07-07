include ../config.mak

SOURCE=$(SOURCE_ROOT)/../timetaggerd

include $(SOURCE_ROOT)/../rules.mak

%.o: $(SOURCE)/%.cpp
	@echo compiling $<
	@$(CXX) -c $(CFLAGS) $<

SUBDIRS = httpd modules

.PHONY:	all clean

TIMETAGGERD = BackendController.o ServerController.o BackendHttpd.o Datastore.o DiagramData.o memwatch.o main.o
LIBS = ../backend/libtimetagger.a httpd/libhttpd.a modules/libmodules.a

ifdef WITH_EMBEDDED_HTDOCS
TIMETAGGERD += internal_http_files.o
CFLAGS += -DEMBEDD_HTDOCS
endif

ifdef FPGA_DIRECT_IO
CFLAGS += -DFPGA_DIRECT_IO
else
CFLAGS += -DFPGA_FRONTPANEL
endif

all: all-subdirs all-me all-done 
all-done:
	@echo finished building timetagger app 
	 
all-subdirs: 
	@for i in $(SUBDIRS); do make -C $$i all ; done 

all-me: me-msg timetaggerd
me-msg:
	@echo building timetaggerd

.PHONY: internal_http_files.cpp
internal_http_files.cpp:
	@httpd/collect-files www >internal_http_files.cpp

timetaggerd: $(TIMETAGGERD) $(LIBS)
	@echo linking timetaggerd
	@$(CXX) -o timetaggerd $(TIMETAGGERD) $(LIBS) $(LFLAGS)

clean:  clean-subdirs clean-me
clean-subdirs:
	@for i in $(SUBDIRS); do make -C $$i clean; done	
clean-me:
	@echo cleaning timetaggerd
	rm -f *.o *~ *pyc gmon.out core.* timetaggerd internal_http_files.cpp

dist-clean: clean
