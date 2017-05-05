/*
 * TimetaggerFPGA.h
 *
 *  Created on: 31.05.2013
 *      Author: mag
 */

#ifndef TIMETAGGERFPGA_H_
#define TIMETAGGERFPGA_H_

#include "FPGA.h"

class TimetaggerFPGA : public FPGA {
public:
	TimetaggerFPGA(std::string serial="", int blocksize=1024);
	~TimetaggerFPGA();

	int getModel();

	int configure();
	bool configured();

	void setChannels(int channels);

	//--------------------
	//added 04.09.2015, n.abt --> for AWG bitstream
	//--------------------
	void setAWGDataOne(int data_one);	
	void setAWGDataTwo(int data_two);
	//--------------------


	void setFilter(bool filter);
	void setTriggerLevel(int channel, double voltage);

	bool sendDacCommand(int prefix, int control, int address, int data, int feature);

	int read(void *buffer, int buffersize);

	static std::vector<std::string> getDeviceList();

protected:
	std::string serial;
	int blocksize;

	bool is_configured;
	bool suppress_warnings;
};

#endif /* TIMETAGGERFPGA_H_ */
