include ../config.mak

ifdef WITH_PYTHON35
	PYTHON_VERSION=python35
else
	PYTHON_VERSION=python27
endif


all:
	mkdir doc/
	cp ../doc/TimeTaggerManual.pdf doc/

	mkdir examples/
	cp -r ../examples/matlab examples/
	cp -r ../examples/python examples/

	mkdir firmware/
	cp ../backend/TimeTaggerController.bit firmware

	mkdir -p win32/${PYTHON_VERSION}/
	cp ../backend/TimeTagger.py win32/${PYTHON_VERSION}/
	cp ../win32/backend/_TimeTagger.pyd win32/${PYTHON_VERSION}/

	mkdir -p win64/${PYTHON_VERSION}/
	cp ../backend/TimeTagger.py win64/${PYTHON_VERSION}/
	cp ../win64/backend/_TimeTagger.pyd win64/${PYTHON_VERSION}/

	mkdir -p win32/driver/
	cp ../win32/prequesits/Driver/i386/WdfCoInstaller01009.dll win32/driver/
	cp ../win32/prequesits/Driver/i386/WinUSBCoInstaller2.dll win32/driver/
	cp ../win32/prequesits/Driver/i386/okusb.cat win32/driver/
	cp ../win32/prequesits/Driver/i386/okusb.inf win32/driver/
	cp ../win32/prequesits/okFrontPanel.dll win32/driver/

	mkdir -p win64/driver/
	cp ../win64/prequesits/Driver/amd64/WdfCoInstaller01009.dll win64/driver/
	cp ../win64/prequesits/Driver/amd64/WinUSBCoInstaller2.dll win64/driver/
	cp ../win64/prequesits/Driver/amd64/okusb.cat win64/driver/
	cp ../win64/prequesits/Driver/amd64/okusb.inf win64/driver/
	cp ../win64/prequesits/okFrontPanel.dll win64/driver/

	cp ../win32/prequesits/vcredist_x86.exe win32/
	cp ../win64/prequesits/vcredist_x64.exe win64/

	zip -r TimeTagger.zip *
	rm -rf doc examples firmware win32 win64

clean:
	rm -rf doc examples firmware win32 win64 TimeTagger.zip

dist-clean:
	rm -rf doc examples firmware win32 win64 TimeTagger.zip
