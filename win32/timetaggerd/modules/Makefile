include ../../config.mak

SOURCE=$(SOURCE_ROOT)/../../timetaggerd/modules

include $(SOURCE_ROOT)/../../rules.mak

%.o: $(SOURCE)/%.cpp
	@echo compiling $<
	@$(CXX) -c $(CFLAGS) $<

%.o: $(SOURCE)/%.c
	@echo compiling $<
	@$(CC) -c $(CFLAGS) $<

.PHONY: all clean all-msg

MODULES = CountbetweenmarkersModule.o CounterModule.o CountrateModule.o

all: all-msg libmodules.a
all-msg:
	@echo building modules

libmodules.a: $(MODULES)
	@echo creating static lib
	@$(AR) r $@ $?
	@$(RANLIB) $@ 

clean: 
	@echo cleaning modules: 
	rm -f *.o *~ gmon.out libmodules.a

dist-clean: clean
