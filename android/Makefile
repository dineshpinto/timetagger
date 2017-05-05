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


