include ../config.mak

SOURCE=$(SOURCE_ROOT)/../backend

include $(SOURCE_ROOT)/../rules.mak


%.o: $(SOURCE)/%.cpp
	@echo compiling $<
	$(CXX) -c $(CFLAGS) -o $@ $<

%.o: $(SOURCE)/%.cxx
	@echo compiling $<
	@$(CXX) -c $(CFLAGS) -Wno-uninitialized $<

.PHONY: all clean all-msg doc

BACKEND=TimeTagger.o FileSearcher.o Logger.o HWClock.o backend.o TimetaggerFPGA.o

ifdef FPGA_DIRECT_IO
CFLAGS += -DFPGA_DIRECT_IO
BACKEND += FPGA_directio.o
else
CFLAGS += -DFPGA_FRONTPANEL
BACKEND += FPGA_frontpanel.o okFrontPanelDLL.o
endif

TOOLS=StartStop.o Iterator.o CountBetweenMarkers.o Counter.o TimeDifferences.o SingTag.o SSRTimeTrace.o


all: all-msg libtimetagger.a  $(PYTHON_LIB) TimeTaggerTest
all-msg:
	@echo building backend

libtimetagger.a: $(BACKEND) $(TOOLS)
	@echo creating static lib
	@$(AR) r $@ $?
	@$(RANLIB) $@

TimeTaggerTest: libtimetagger.a TimeTaggerTest.o
	@$(CXX) TimeTaggerTest.o -ltimetagger $(LFLAGS) -L. -o TimeTaggerTest

$(SOURCE)/TimeTagger_wrap.cxx: $(SOURCE)/TimeTagger.i $(SOURCE)/TimeTagger.h $(BACKEND) $(TOOLS)
	@echo creating python binding
	@swig -Wall -classic -c++ -python $(SOURCE)/TimeTagger.i

$(PYTHON_LIB): TimeTagger_wrap.o $(BACKEND) $(TOOLS)
	@echo linking python binding
	@$(CXX) -shared $^ -o $(PYTHON_LIB) $(LFLAGS)
	@$(STRIP) $(PYTHON_LIB)

ifdef WITH_DOXYGEN
doc:
	doxygen doc/Doxyfile
	make -C doc
else
doc:

endif

clean:
	@echo cleaning backend:
	rm -f *.o TimeTagger _TimeTagger.so *~ *pyc TimeTagger_wrap.cxx gmon.out TimeTagger.py libtimetagger.a
	make -C doc clean

dist-clean: clean
