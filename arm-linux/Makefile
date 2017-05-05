SUBDIRS = backend timetaggerd

.PHONY:	all clean

all: copy-makefiles 
	@for i in $(SUBDIRS); do make --no-print-directory -C $$i all || exit 1; done 

	
copy-makefiles: backend/Makefile timetaggerd/Makefile timetaggerd/httpd/Makefile
  
  
backend/Makefile:
	mkdir -p backend
	cp ../backend/Makefile backend
	 
timetaggerd/Makefile:
	mkdir -p timetaggerd
	cp ../timetaggerd/Makefile timetaggerd

timetaggerd/httpd/Makefile:
	mkdir -p timetaggerd/httpd
	cp ../timetaggerd/httpd/Makefile timetaggerd/httpd
	
  
clean: copy-makefiles
	@for i in $(SUBDIRS); do make --no-print-directory -C $$i clean; done
	
dist-clean:
	rm -Rf backend timetaggerd dist



install:
	@echo collecting win32 files
	
	mkdir -p dist/system
	cp ../backend/TimeTaggerController.bit dist/system
	### ==> cp lib/okFrontPanel.dll dist/system
	
	cp timetaggerd/timetaggerd dist/timetaggerd.exe
	
	mkdir -p dist/c++
	cp ../backend/doc/index.html dist/c++
	cp ../backend/doc/*.pdf dist/c++
	cp ../backend/doc/*.chm dist/c++
	cp -R ../backend/doc/html dist/c++
	
	mkdir -p dist/c++/include
	cp ../backend/TimeTagger.h dist/c++/include
	cp ../backend/HWClock.h dist/c++/include
	cp ../backend/Thread.h dist/c++/include
	cp ../backend/Logger.h dist/c++/include
	cp ../backend/Iterator.cpp dist/c++/include
	cp ../backend/Counter.cpp dist/c++/include
	cp ../backend/Histogram.cpp dist/c++/include
	cp ../backend/Correlation.cpp dist/c++/include
	cp ../backend/CountBetweenMarkers.cpp dist/c++/include
	
	mkdir -p dist/c++/lib
	
	
	mkdir -p dist/python
	cp backend/_TimeTagger.pyd dist/python
	cp ../backend/TimeTagger.py dist/python
	cp ../demo/*.py dist/python
	cp ../demo/doc/*.chm dist/python
	cp ../demo/doc/*.pdf dist/python
	cp ../demo/doc/index.html dist/python
	cp -R ../demo/doc/html dist/python
