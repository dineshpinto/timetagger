/*
 * FPGA_frontpanel.cpp
 *
 *  Created on: 01.06.2013
 *      Author: mag
 */

#include "FPGA.h"

#ifdef FPGA_FRONTPANEL

#include "Logger.h"
#include "FileSearcher.h"
#include <stdlib.h>

#ifdef WIN32
#include <windows.h>
#endif

static void error(const char *fmt, ...) {
	va_list va;
	va_start(va, fmt);
	Logger::vlog(Logger::BACKEND, Logger::LOG_ERROR, fmt, va);
}

static void debug(const char *fmt, ...) {
	va_list va;
	va_start(va, fmt);
	Logger::vlog(Logger::BACKEND, Logger::LOG_DEBUG, fmt, va);
}

static void fatal(const char *fmt, ...) {
	va_list va;
	va_start(va, fmt);
	Logger::vlog(Logger::BACKEND, Logger::LOG_FATAL, fmt, va);
	exit(255);
}




static void init_frontpanel() {
#ifdef __x86_64__
	std::string dir = "win64\\driver";
#else
	std::string dir = "win32\\driver";
#endif

#ifdef WIN32
	std::string file = "okFrontPanel.dll";
#else
	std::string file = "okFrontPanel.so";
#endif

	// load FrontPanel library
	std::string dllfile = search_file(file, dir);
	if ( FALSE == okFrontPanelDLL_LoadLib(dllfile == "" ? NULL : dllfile.c_str()) ) {
		fatal("Could not load FrontPanel DLL.");
	}
}

FPGA::FPGA() {
	xem=0;
	blocksize=1024;
	init_frontpanel();
}

FPGA::~FPGA() {
	close();
}

bool FPGA::open(std::string serial) {
	close();
	xem=new okCFrontPanel();
	return 0 == xem->OpenBySerial(serial);
}

void FPGA::close() {
	if (xem) delete xem;
	xem=0;
}

std::string FPGA::getSerial() {
	return xem->GetSerialNumber();
}

void FPGA::setTimeout(unsigned timeout) {
	xem->SetTimeout(timeout);
}

void FPGA::setBlocksize(unsigned blocksize) {
	this->blocksize=blocksize;
}

bool FPGA::setPLLConfig(unsigned char *config) {
	okCPLL22150 pll;
	pll.InitFromProgrammingInfo(config);
	return 0==xem->SetPLL22150Configuration(pll);
}

bool FPGA::uploadBitfile(std::string bitfile) {
	return 0==xem->ConfigureFPGA(bitfile);
}

bool FPGA::setWireIn(unsigned addr, short value) {
	return 0==xem->SetWireInValue(addr, value);
}

bool FPGA::UpdateWireIns() {
	xem->UpdateWireIns();
	return true;
}

bool FPGA::ActivateTrigger(unsigned addr, unsigned bit) {
	return 0==xem->ActivateTriggerIn(addr, bit);
}

int FPGA::ReadFromPipeOut(unsigned endpoint, unsigned size, unsigned char *buffer) {
	return xem->ReadFromBlockPipeOut(endpoint, blocksize, size, buffer);
}

std::vector<FPGADevice> FPGA::getDeviceList() {
	std::vector<FPGADevice> list;

	init_frontpanel();

	okCFrontPanel *fp=new okCFrontPanel();
	int count=fp->GetDeviceCount();
debug("%d devices", count);
	for (int n=0; n<count; ++n) {
		std::string serial=fp->GetDeviceListSerial(n);
debug("found %s", serial.c_str());
		okCFrontPanel *xem=new okCFrontPanel();
		if (xem->OpenBySerial(serial) == 0) {
			FPGADevice device;
			device.serial=serial;
			switch (xem->GetBoardModel()) {
			case okCFrontPanel::brdXEM3005:
				device.model=0x151f0023; break;
			default:
				device.model=0x0; break;
			}

			device.state=FPGADevice::IDLE;
			device.id=xem->GetDeviceID();
			delete xem;
			list.push_back(device);
		}
	}
	delete fp;
	return list;
}

#endif
