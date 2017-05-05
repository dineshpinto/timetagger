/*
 * TimetaggerFPGA.cpp
 *
 *  Created on: 31.05.2013
 *      Author: mag
 */

#include "TimetaggerFPGA.h"
#include "Logger.h"
#include "FileSearcher.h"

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <iostream>

#ifndef _MSC_VER
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#endif

#ifdef WIN32
#pragma warning( disable : 4996)

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif



static const std::string bitfilename = "TimeTaggerController.bit";		// location of bitfile for fpga

//const int DAC_channel_map [8] = {3,2,1,0,4,5,6,7}; // AD5318
const int DAC_channel_map [8] = {7,5,3,1,6,4,2,0}; // DAC8568


static void error(const char *fmt, ...) {
	va_list va;
	va_start(va, fmt);
	Logger::vlog(Logger::BACKEND, Logger::LOG_ERROR, fmt, va);
}

static void fatal(const char *fmt, ...) {
	va_list va;
	va_start(va, fmt);
	Logger::vlog(Logger::BACKEND, Logger::LOG_FATAL, fmt, va);
	exit(255);
}

TimetaggerFPGA::TimetaggerFPGA(std::string _serial, int _blocksize)
: serial(_serial), blocksize(_blocksize)  {
	suppress_warnings=false;
	is_configured=false;

	//configure();
}

TimetaggerFPGA::~TimetaggerFPGA() {
}

static std::string get_bitfile() {
	return search_file(bitfilename, "firmware");
}


int TimetaggerFPGA::configure() {
	bool ok;

	this->setBlocksize(blocksize);

	// configure fpga
	// open USB connection
	if (!(ok = this->open(serial.c_str())) && !suppress_warnings) {
		error("Opening of USB connection failed");
	}

	// prepare data to be uploaded to PLL
#ifdef WIN32
	// on windows, use opalkelly pll class
	okCPLL22150 *pll = new okCPLL22150();
	pll->SetVCOParameters(250, 36); // set VCO to 300MHz
	pll->SetDiv1(pll->DivSrc_VCO, 4); // set custom clock 1 to VCO/4 = 83 MHz
	pll->SetOutputSource(0,pll->ClkSrc_Div1ByN);
	pll->SetOutputEnable(0, true); // enable the clocks
	pll->SetOutputSource(2,pll->ClkSrc_Div1By2); // set PLL clk2 to Div1By2 = 200 MHz --> "clk"
	pll->SetOutputEnable(2, true); // enable the clocks
#if 0
	// dev code, to extract binary configuration
	unsigned char pllconfig[20];
	for(int x=0; x<20; ++x)pllconfig[x]=0;
	pll->GetProgrammingInfo(pllconfig);
	for(int x=0; x<20; ++x)
		fprintf(stderr, "\\x%02x ", pllconfig[x]);
	delete pll;
#else
	unsigned char pllconfig[20];
	pll->GetProgrammingInfo(pllconfig);
#endif
#else
	// on linux, use prebuild config
	const char *pllconfig="\x05\x04\x20\x95\xc4\x79\x22\x21\x00\x3f\x04\x00";
#endif

	// upload PLL configuration
	if (ok && !(ok = this->setPLLConfig((unsigned char *) pllconfig))) {
		error("Uploading PLL configuration failed");
	}

	std::string bitfile=get_bitfile();
	if (bitfile=="") {
		fatal("firmware not found.");
	}
	//upload design to FPGA
	if (ok && !(ok = this->uploadBitfile(bitfile))) {
		error("Uploading FPGA configuration failed");
	}

	// set programmable empty threshold of fifo
	if(ok && !(ok = this->setWireIn(ADDR_EMPTY_THRESHOLD, blocksize/2))) {
		error("Uploading empty threshold failed");
	}

	if(ok) {
		this->UpdateWireIns();
		this->setTimeout(1000); // set timeout to 1s
	}

    // set filter
    if(ok && !(ok = this->setWireIn(ADDR_LASER_FILTER, 0))) {
            error("unsetting reset failed");
    }
    if(ok) {
           this->UpdateWireIns();
    }

    // initialize DAC
    if(ok) {
        ok = sendDacCommand(0,1<<3,0,0,1); // activate internal reference voltage
    }
    for(int i=0; i<8; i++) {
        if(ok) {
            ok = sendDacCommand(0, 3, DAC_channel_map[i], int(0.5/2.5*32768), 0); // write and update trigger levels
        }
    }

	is_configured = ok;
	suppress_warnings = !is_configured;

	return is_configured;
}

bool TimetaggerFPGA::configured() {
	return is_configured;
}

int TimetaggerFPGA::getModel() {
//TODO
	return 0;
}


bool TimetaggerFPGA::sendDacCommand(int prefix, int control, int address, int data, int feature) { // see DAC8568 data sheet
	bool ok=true;
	if( ok && !(ok = this->setWireIn(ADDR_DAC_DIN_LOW, data<<4 | feature))) { // low 16 bit word
			error("setting DAC value failed");
	}
	if( ok && !(ok = this->setWireIn(ADDR_DAC_DIN_HIGH, prefix<<12 | control<<8 | address<<4 | data>>12))) { // high 16 bit word
			error("setting DAC value failed");
	}
	if(ok) {
			this->UpdateWireIns();
	}
	if( ok && !(ok = this->ActivateTrigger(ADDR_DAC_WR_EN,0))) {
			error("triggering DAC failed");
	}
	return ok;
}

void TimetaggerFPGA::setChannels(int channels) {
	if(!this->setWireIn(ADDR_CHANNEL_SELECTION, channels))
		is_configured = 0;
	else
		this->UpdateWireIns();
}

//--------------------
//added 04.09.2015, n.abt --> for bitstream and jump address for AWG
//--------------------

void TimetaggerFPGA::setAWGDataOne(int data_one) {
	if(!this->setWireIn(ADDR_AWG_DATA_LOW, data_one))
		is_configured = 0;
	else
		this->UpdateWireIns();
}

void TimetaggerFPGA::setAWGDataTwo(int data_two) {
	if(!this->setWireIn(ADDR_AWG_DATA_HIGH, data_two))
		is_configured = 0;
	else
		this->UpdateWireIns();
}

//--------------------

void TimetaggerFPGA::setFilter(bool state) {
	if( !this->setWireIn(ADDR_LASER_FILTER, state) ) {
		error("setting filter failed");
		is_configured = 0;
	} else {
		this->UpdateWireIns();
	}
}

void TimetaggerFPGA::setTriggerLevel(int channel, double voltage) {
	if (is_configured) {
		is_configured = sendDacCommand(0, 3, DAC_channel_map[channel], int(voltage/2.5*32768.0), 0);
	}
}

int TimetaggerFPGA::read(void *buffer, int buffersize) {
	if (is_configured) {
		int len = this->ReadFromPipeOut(ADDR_PIPE, buffersize, (unsigned char *)buffer);
		if (len<0) {
			error("PipeBackend: Pipe transfer failed, len=%d",len);
			is_configured = 0;
		}
		return len;
	} else {
		return -1;
	}
}
