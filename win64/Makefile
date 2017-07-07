SUBDIRS = backend timetaggerd

.PHONY:	all clean dist

all: copy-makefiles 
	@for i in $(SUBDIRS); do make --no-print-directory -C $$i all || exit 1; done 
	@make dist


copy-makefiles: backend/Makefile timetaggerd/Makefile timetaggerd/httpd/ timetaggerd/modules/ timetaggerd/www/

backend/Makefile:
	@mkdir -p backend
	@cd backend; cp  ../../backend/Makefile .
	 
timetaggerd/Makefile:
	@mkdir -p timetaggerd
	@cd timetaggerd; cp ../../timetaggerd/Makefile .
	@mkdir -p timetaggerd/doc

timetaggerd/httpd/:
	@mkdir -p timetaggerd/httpd
	@cd timetaggerd/httpd; cp -r ../../../timetaggerd/httpd/* .
	
timetaggerd/modules/:
	@mkdir -p timetaggerd/modules
	@cd timetaggerd/modules; cp ../../../timetaggerd/modules/* .

timetaggerd/www/:
	@mkdir -p timetaggerd/www
	@cd timetaggerd/www; cp -r ../../../timetaggerd/www/* .
	
  
clean: copy-makefiles
	@for i in $(SUBDIRS); do make --no-print-directory -C $$i clean; done
	
# Removed timetaggerd from dist-clean
dist-clean:
	rm -Rf backend dist



dist:
	@echo collecting win32 files
	
	@mkdir -p dist/system
	@cp ../backend/TimeTaggerController.bit dist/system
	@cp prequesits/okFrontPanel.dll dist/system
	@cp prequesits/vcredist_x64.exe dist/system
	@cp prequesits/Driver/okusb.inf dist/system
	@mkdir -p dist/system/i386
	@cp ../win32/prequesits/Driver/i386/WdfCoInstaller01009.dll dist/system/i386
	@cp ../win32/prequesits/Driver/i386/WinUSBCoInstaller2.dll dist/system/i386
	@mkdir -p dist/system/amd64

	@cp prequesits/Driver/amd64/WdfCoInstaller01009.dll dist/system/amd64
	@cp prequesits/Driver/amd64/WinUSBCoInstaller2.dll dist/system/amd64
	
	@cp timetaggerd/timetaggerd dist/timetaggerd.exe
	
	#~ @mkdir -p dist/c++
	#~ @cp ../backend/doc/index.html dist/c++
	#~ @cp ../backend/doc/*.pdf dist/c++
	#~ @cp ../backend/doc/*.chm dist/c++
	#~ @cp -R ../backend/doc/html dist/c++
	
	@mkdir -p dist/c++/include
	@cp ../backend/TimeTagger.h dist/c++/include
	@cp ../backend/HWClock.h dist/c++/include
	@cp ../backend/Thread.h dist/c++/include
	@cp ../backend/Logger.h dist/c++/include
	@cp ../backend/Iterator.cpp dist/c++/include
	@cp ../backend/Counter.cpp dist/c++/include
	#~ @cp ../backend/Histogram.cpp dist/c++/include
	#~ @cp ../backend/Correlation.cpp dist/c++/include
	@cp ../backend/CountBetweenMarkers.cpp dist/c++/include
	#~ @cp ../backend/Flim.cpp dist/c++/include
	@cp ../backend/TimeDifferences.cpp dist/c++/include
	#~ @cp ../backend/DistributionTicks.cpp dist/c++/include
	@cp ../backend/SSRTimeTrace.cpp dist/c++/include
	
	@mkdir -p dist/c++/lib
	
	@cp -R vstudio/* dist/c++ 
	
	@cp ../backend/backend.cpp dist/c++/TimeTagger
	@cp ../backend/backend.h dist/c++/TimeTagger
	@cp ../backend/HWClock.cpp dist/c++/TimeTagger
	@cp ../backend/HWClock.h dist/c++/TimeTagger
	@cp ../backend/Logger.cpp dist/c++/TimeTagger
	@cp ../backend/Logger.h dist/c++/TimeTagger
	@cp ../backend/okFrontPanelDLL.cpp dist/c++/TimeTagger
	@cp ../backend/okFrontPanelDLL.h dist/c++/TimeTagger
	@cp ../backend/Thread.cpp dist/c++/TimeTagger
	@cp ../backend/Thread.h dist/c++/TimeTagger
	@cp ../backend/TimeTagger.cpp dist/c++/TimeTagger
	@cp ../backend/TimeTagger.h dist/c++/TimeTagger
	@cp ../backend/TimetaggerFPGA.h dist/c++/TimeTagger
	@cp ../backend/TimetaggerFPGA.cpp dist/c++/TimeTagger
	@cp ../backend/FPGA_frontpanel.cpp dist/c++/TimeTagger
	@cp ../backend/FPGA.h dist/c++/TimeTagger	
	
	@mkdir -p dist/python
	@cp backend/_TimeTagger.pyd dist/python
	@cp ../backend/TimeTagger.py dist/python
	#@cp ../demo/python/*.py dist/python
	#@cp ../demo/python/doc/*.chm dist/python
	#@cp ../demo/python/doc/*.pdf dist/python
	#@cp ../demo/python/doc/index.html dist/python
	#@cp -R ../demo/python/doc/html dist/python
	
	#@cp ../doc/TimeTaggerManual.pdf dist
	@mkdir -p dist/setup
	@cp -R setup/* dist/setup
	
