@echo off

set CFLAGS=-c -O3 -std=c++11 -Wall -D_hypot=hypot -I%VIRTUAL_ENV%\include -I%VIRTUAL_ENV%\Lib\site-packages\numpy\core\include -DFPGA_FRONTPANEL=1 -DWIN32
set LFLAGS=-lpython27 -D_hypot=hypot -L%VIRTUAL_ENV%\libs -static-libstdc++ -static-libgcc

echo Loesche bereits kompilierte Dateien.
del *.o TimeTagger _TimeTagger.so *~ *pyc TimeTagger_wrap.cxx gmon.out TimeTagger.py

echo Erstelle Python Interface
swig -Wall -c++ -python TimeTagger.i
g++ %CFLAGS% TimeTagger_wrap.cxx -DNPY_NO_DEPRECATED_API=NPY_1_7_API_VERSION

echo Kompiliere TimeTagger
g++ %CFLAGS% okFrontPanelDLL.cpp
g++ %CFLAGS% TimeTagger.cpp
g++ %CFLAGS% backend.cpp
g++ %CFLAGS% FPGA_frontpanel.cpp
g++ %CFLAGS% TimetaggerFPGA.cpp
g++ %CFLAGS% Logger.cpp
g++ %CFLAGS% HWClock.cpp
g++ %CFLAGS% SSRTimeTrace.cpp
g++ %CFLAGS% Thread.cpp -DTHREAD_USE_WINDOWS

echo Linke Python-Modul
g++ -shared TimeTagger_wrap.o okFrontPanelDLL.o TimeTagger.o Thread.o backend.o FPGA_frontpanel.o TimetaggerFPGA.o Logger.o HWClock.o SSRTimeTrace.o -o _TimeTagger.pyd %LFLAGS%
