# android crossbuild

SOURCE_ROOT=..

# target platform
LINUX=1
#WIN32=1
ANDROID=1

ARM_LINUX=1
#I386_LINUX=1

NDKHOME=/home/mag/android/ndk
TOOLCHAIN_HOME=$(NDKHOME)/toolchains/arm-linux-androideabi-4.7/prebuilt/linux-x86_64
TOOLCHAIN=$(TOOLCHAIN_HOME)/bin/arm-linux-androideabi

# cross builds
#WITH_WIN32=1
#WITH_ARM=1
#WITH_ANDROID=1

# whether to build python bindings
WITH_PYTHON=1

# whether to build documentation
#WITH_DOXYGEN=1

# threading library
THREAD_PTHREADS = 1
#THREAD_BOOST=1

# whether to include htdocs into binary
WITH_EMBEDDED_HTDOCS=1

# select hardware frontend
FPGA_DIRECT_IO=1
#FPGA_FRONTPANEL=1
