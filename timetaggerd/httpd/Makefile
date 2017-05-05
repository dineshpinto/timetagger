include ../../config.mak

SOURCE=$(SOURCE_ROOT)/../../timetaggerd/httpd

include $(SOURCE_ROOT)/../../rules.mak

%.o: $(SOURCE)/%.cpp
	@echo compiling $<
	@$(CXX) -c $(CFLAGS) $<

%.o: $(SOURCE)/%.c
	@echo compiling $<
	@$(CC) -c $(CFLAGS) $<

.PHONY: all clean all-msg

HTTPD = Httpd.o json.o mongoose.o HttpStream.o


all: all-msg libhttpd.a collect-files
all-msg:
	@echo building httpd

libhttpd.a: $(HTTPD)
	@echo creating static lib
	@$(AR) r $@ $?
	@$(RANLIB) $@ 

collect-files:
	@g++ -o collect-files $(SOURCE)/collect-files.cpp
	
clean: 
	@echo cleaning httpd: 
	rm -f *.o *~ *pyc gmon.out libhttpd.a core.* collect-files

dist-clean: clean
