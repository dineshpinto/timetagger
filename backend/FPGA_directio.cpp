/*
 * FPGA_directio.cpp
 *
 *  Created on: 01.06.2013
 *      Author: mag
 */

#include "FPGA.h"

#ifdef FPGA_DIRECT_IO

#include <stdlib.h>
#include <iostream>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#include <linux/usbdevice_fs.h>
#include <linux/usb/ch9.h>

#include "Logger.h"

static void error(const char *fmt, ...) {
	va_list va;
	va_start(va, fmt);
	Logger::vlog(Logger::BACKEND, Logger::LOG_ERROR, fmt, va);
}

static void debug(const char *fmt, ...) {
	va_list va;
	va_start(va, fmt);
	Logger::vlog(Logger::BACKEND, Logger::LOG_ERROR, fmt, va);
}


static const int VENDOR_OPALKELLY=0x151f;
static const int PRODUCT_XEM3005=0x23;
static const char *DEV_BUS_PATH="/dev/bus/usb";

static const int SERIAL_LENGTH=10;



static int io_get_language(int fd, int timeout);
static std::string io_get_serial(int fd, int language, int timeout);
static std::string io_get_id(int fd, int timeout);
static int io_claim_interface(int fd, int timeout);
static int io_set_configuration(int fd, int interface, int timeout);
static int io_release_interface(int fd, int timeout);

static int io_set_pll_config(int fd, unsigned char *config, int timeout);

static int io_activate_trigger(int fd, int addr, int bit, int timeout);
static int io_update_wire_in(int fd, short *wire_ins, int timeout);
static int io_update_wire_out(int fd, short *wire_outs, int timeout);
static int io_update_trigger_out(int fd, short *trigger_outs, int timeout);

static int io_upload_bitfile(int fd, unsigned size, unsigned char *bits, int timeout);
static int io_read_from_pipe_block(int fd, int endpoint, unsigned size, unsigned char *buffer, int timeout);


typedef bool (*TRAVERSE_FUNC)(int fd, const std::string &path, int vendor, int product, const std::string &serial, const std::string &id, void *data);
static bool traverse_usb_dir(std::string path, TRAVERSE_FUNC func, void *data);


bool FPGA::list_func(int fd, const std::string &path, int vid, int pid, const std::string &serial, const std::string &id, void *data) {
	std::vector<FPGADevice> *list=(std::vector<FPGADevice> *) data;


debug("traverse device %04x:%04x: %s", vid,pid, serial.c_str());
	if (io_claim_interface(fd, 1000)>=0) {
debug("got it");

		FPGADevice device;
		device.serial=serial;
		device.vendor=vid;
		device.product=pid;
		device.id = io_get_id(fd,1000);
		list->push_back(device);
		io_release_interface(fd, 1000);
		::close(fd);
	}

	return false;
}

std::vector<FPGADevice> FPGA::getDeviceList() {
	std::vector<FPGADevice> list;
	traverse_usb_dir(std::string(DEV_BUS_PATH), list_func, &list);
	return list;
}

FPGA::FPGA() {
	fd=-1;
	blocksize=1024;
	timeout=1000;
}

FPGA::~FPGA() {
	close();
}


bool FPGA::open_func(int fd, const std::string &path, int vid, int pid, const std::string &serial, const std::string &id, void *data) {
	FPGA *THIS=(FPGA *)data;
	THIS->fd=fd;
	return true;
}

bool FPGA::open(std::string serial) {
	close();
	if (traverse_usb_dir(std::string(DEV_BUS_PATH), open_func, this)) {
		int ret=io_set_configuration(fd, 1, timeout);
		if (ret>=0) {
			ret=io_claim_interface(fd, timeout);
			if (ret>=0) {
				return true;
			} else {
				std::cerr << "claim interface failed" << std::endl;
			}
		} else {
			std::cerr << "set configuration failed" << std::endl;
		}
	}
	return false;
}

void FPGA::close() {
	if (fd>=0)
		::close(fd);
	fd=-1;
}

std::string FPGA::getSerial() {
	return "";
}

void FPGA::setTimeout(unsigned timeout) {
	this->timeout=timeout;
}

void FPGA::setBlocksize(unsigned blocksize) {
	this->blocksize=blocksize;
}

bool FPGA::setPLLConfig(unsigned char *config) {
	if (fd>=0) {
		int ret=io_set_pll_config(fd, config, timeout);
		return ret>=0;
 	} else
 		return false;
}

bool FPGA::setWireIn(unsigned addr, short value) {
	if (fd>=0) {
		wire_ins[addr]=value;
		return true;
	}
	return false;
}

bool FPGA::UpdateWireIns() {
	return (fd>=0) && (io_update_wire_in(fd, wire_ins, timeout)>=0);
}

bool FPGA::UpdateWireOuts() {
	return (fd>=0) && (io_update_wire_out(fd, wire_outs, timeout)>=0);
}

bool FPGA::UpdateTriggerOuts() {
	return (fd>=0) && (io_update_trigger_out(fd, trigger_outs, timeout)>=0);
}

bool FPGA::isTriggered(unsigned addr, unsigned bit) {
	return (fd>=0) && trigger_outs[addr] & (1<<bit);
}

bool FPGA::ActivateTrigger(unsigned addr, unsigned bit) {
	return (fd>=0) && (io_activate_trigger(fd, addr, bit, timeout)>=0);
}

bool FPGA::uploadBitfile(std::string bitfile) {
	if (fd>=0) {
		FILE *fbits=fopen(bitfile.c_str(), "r");
		if (fbits) {
			fseek(fbits, 0, SEEK_END);
			unsigned size=ftell(fbits);
			fseek(fbits, 0, SEEK_SET);
			unsigned char *bits=new unsigned char[size];
			unsigned off;
			for (off=0; off<size; ) {
				unsigned bytesread=fread(bits+off, 1, size, fbits);
				if (bytesread) {
					off+=bytesread;
				}
			}
			fclose(fbits);
			off=0;
			for (off=0; off<size-8; ++off) {
				if (strncmp((const char*)bits+off, "\xFF\xFF\xFF\xFF\xAA\x99\x55\x66",8)==0)
					break;
			}
			if (off<size) {
				unsigned char *fpga_bits=new unsigned char[2*(size - off)];
				unsigned char *p=fpga_bits;
				for (unsigned n=off; n<size; ++n) {
					*p++=bits[n]; *p++=0;
				}
				delete [] bits;
				int ret=io_upload_bitfile(fd, 2*(size - off), fpga_bits, timeout);
				delete [] fpga_bits;
				if (ret>=0)
					return true;
				else {
					error("uploading bitfile failed");
					return false;
				}

			} else {
				error("bad bitfile!");
			}
		} else {
			error("bitfile not found!");
		}
	}
	return false;
}

int FPGA::ReadFromPipeOut(unsigned endpoint, unsigned size, unsigned char *buffer) {
	if (fd>=0) {
		return io_read_from_pipe_block(fd, endpoint, size, buffer, timeout);
	} else
		return -1;
}


static bool traverse_usb_dir(std::string path, TRAVERSE_FUNC func, void *data) {
	DIR *dir=opendir(path.c_str());
	if (dir) {
		for (;;) {
			struct dirent *de=readdir(dir);
			struct stat st;
			if (!de)
				break;
			if (de->d_name[0]=='.') continue;
			std::string file=path+"/"+de->d_name;
			if (stat(file.c_str(), &st)==0) {
				if (st.st_mode & S_IFDIR) {
					if (traverse_usb_dir(file, func, data)) {
						closedir(dir);
						return true;
					}
				} else {
					int fd=::open(file.c_str(), O_RDWR);
					if (fd>=0) {
						unsigned short words[12];
						if (read(fd, words, 24)==24) {
							unsigned short vid=words[4];
							unsigned short pid=words[5];
							if (vid==VENDOR_OPALKELLY && pid==PRODUCT_XEM3005) {
								int lang=io_get_language(fd, 1000);
								std::string serial=io_get_serial(fd, lang, 1000);
								//std::string id=io_get_id(fd, 1000);
								std::string id="";
								if (func(fd, path, vid, pid, serial, id, data) == true) {
									closedir(dir);
									return true;
								}
							}
						}
						::close(fd);
					}
				}
			}
		}
		closedir(dir);
	}
	return false;
}


// control messages
static const int GET_DESCRIPTOR = 0x06;

static const int FPGA_ID 		= 0xb0;
static const int FPGA_PLL 		= 0xb1;
static const int FPGA_BITFILE 	= 0xb2;
static const int FPGA_WIRES 	= 0xb5;
static const int FPGA_PIPES 	= 0xb7;
static const int FPGA_STATUS 	= 0xb9;


typedef unsigned char BYTE;
typedef unsigned short WORD;

/*
 * USB Device Descriptor delivered by the XEM3005:
 * 12 01
 * 0200
 * 00 00 ff   	no device class, no protocol
 * 40  			max packetsize: 64
 * 151f			vendor
 * 0023			product
 * 0301 		release
 * 01 02 03
 * 01			obne configuration
 *
 */
struct device_descriptor {
	BYTE bLength;				// Size of the Descriptor in Bytes (18 bytes)
	BYTE bDescriptorType;		// Device Descriptor (0x01)
	WORD bcdUSB	; 				// USB Specification Number which device complies too.
	BYTE bDeviceClass;			// Class Code (Assigned by USB Org)
								// If equal to Zero, each interface specifies it’s own class code
								// If equal to 0xFF, the class code is vendor specified.
								// Otherwise field is valid Class Code.
	BYTE bDeviceSubClass;		// Subclass Code (Assigned by USB Org)
	BYTE bDeviceProtocol;		// Protocol Code (Assigned by USB Org)
	BYTE bMaxPacketSize;		// Maximum Packet Size for Zero Endpoint. Valid Sizes are 8, 16, 32, 64
	WORD idVendor;				// Vendor ID (Assigned by USB Org)
	WORD idProduct;				// Product ID (Assigned by Manufacturer)
	WORD bcdDevice;				// Device Release Number
	BYTE iManufacturer;			// Index of Manufacturer String Descriptor
	BYTE iProduct;				// Index of Product String Descriptor
	BYTE iSerialNumber;			// Index of Serial Number String Descriptor
	BYTE bNumConfigurations;	// Number of Possible Configurations
};

/*
 * USB Configuration Descriptor delivered by the XEM3005:
 * 09 02
 * 0020
 * 01 			one interface
 * 01 00
 * c0 			self powered
 * fa 			500mA
 */
struct configuration_descriptor {
	BYTE bLength;				// Size of Descriptor in Bytes
	BYTE bDescriptorType;		// Configuration Descriptor (0x02)
	WORD wTotalLength;			// Total length in bytes of data returned
	BYTE bNumInterfaces;		// Number of Interfaces
	BYTE bConfigurationValue;	// Value to use as an argument to select this configuration
	BYTE iConfiguration;		// Index of String Descriptor describing this configuration
	BYTE bmAttributes;			// D7 Reserved, set to 1. (USB 1.0 Bus Powered)
								// D6 Self Powered
								// D5 Remote Wakeup
								// D4..0 Reserved, set to 0.
	BYTE bMaxPower;				// Maximum Power Consumption in 2mA units
};


/*
 * USB Interface Descriptor delivered by the XEM3005:
 *
 * 09 04
 * 00 00 		interface#0, no alternate setting
 * 02 			two endpoints
 * ff 00 00 	no class, no protocol
 * 00
 */
struct interface_descriptor {
	BYTE bLength;				// Size of Descriptor in Bytes
	BYTE bDescriptorType;		// Interface Descriptor (0x04)
	BYTE bInterfaceNumber;		// Number of Interface
	BYTE bAlternateSetting;		// Value used to select alternative setting
	BYTE bNumEndpoints;			// Number of Endpoints used for this interface
	BYTE bInterfaceClass;		// Class Code (Assigned by USB Org)
	BYTE bInterfaceSubClass;	// Subclass Code (Assigned by USB Org)
	BYTE bInterfaceProtocol;	// Protocol Code (Assigned by USB Org)
	BYTE iInterface;			// Index of String Descriptor Describing this interface
};

/*
 * USB Endpoint Descriptors delivered by the XEM3005:
 *
 * 07 05
 * 02		endpoint #2
 * 02 		BULK
 * 0200 	512bytes packet size
 * 00
 *
 * 07 05
 * 86		endpoint #6, INPUT
 * 02 		BULK
 * 0200
 * 00
 */
struct endpoint_descriptor {
	BYTE bLength;				// Size of Descriptor in Bytes
	BYTE bDescriptorType;		// Endpoint Descriptor (0x05)
	BYTE bEndpointAddress;		// Endpoint Address
								// Bits 0..3b Endpoint Number.
								// Bits 4..6b Reserved. Set to Zero
								// Bits 7 Direction 0 = Out, 1 = In (Ignored for Control Endpoints)
	BYTE bmAttributes;			// Bits 0..1 Transfer Type
								//	00 = Control
								//	01 = Isochronous
								//	10 = Bulk
								//	11 = Interrupt
								// Bits 2..7 are reserved. If Isochronous endpoint,
								// Bits 3..2 = Synchronisation Type (Iso Mode)
								//	00 = No Synchonisation
								//	01 = Asynchronous
								//	10 = Adaptive
								//	11 = Synchronous
								// Bits 5..4 = Usage Type (Iso Mode)
								//	00 = Data Endpoint
								//	01 = Feedback Endpoint
								//	10 = Explicit Feedback Data Endpoint
								//	11 = Reserved
	BYTE wMaxPacketSize;		// Maximum Packet Size this endpoint is capable of sending or receiving
	BYTE bInterval;				// Interval for polling endpoint data transfers. Value in frame counts.
								// Ignored for Bulk & Control Endpoints. Isochronous must equal 1 and field may range
								// from 1 to 255 for interrupt endpoints.
};


static int io_get_language(int fd, int timeout) {
	struct usbdevfs_ctrltransfer ctl;
	char buffer[32];

	ctl.bRequestType = 128;  // dir=USB_DIR_IN | type=USB_TYPE_STANDARD | recip=USB_RECIP_DEVICE
	ctl.bRequest = GET_DESCRIPTOR;
	ctl.wValue = 768; // 0x0300 => string descriptor #0
	ctl.wIndex = 0;
	ctl.wLength = 32;
	ctl.timeout = timeout;
	ctl.data = buffer;

	int ret=ioctl(fd, USBDEVFS_CONTROL, &ctl);
	if (ret>=0) {
		return buffer[2] | buffer[3]<<8;
	}
	return ret;
}

static std::string io_get_serial(int fd, int language, int timeout) {
	struct usbdevfs_ctrltransfer ctl;
	char buffer[32];

	ctl.bRequestType = 128;  // dir=USB_DIR_IN | type=USB_TYPE_STANDARD | recip=USB_RECIP_DEVICE
	ctl.bRequest = GET_DESCRIPTOR;
	ctl.wValue = 771;	// 0x0303 => string descriptor #3
	ctl.wIndex = language;
	ctl.wLength = 32;
	ctl.timeout = timeout;
	ctl.data = buffer;

	int ret=ioctl(fd, USBDEVFS_CONTROL, &ctl);
	if (ret>=0) {
		char serial[SERIAL_LENGTH+1];
		char *p=serial;
		for (int n=2; n<buffer[0]; n+=2) {
			*p++=buffer[n];
			p[1]=0;
		}
		return std::string(serial);
	} else {
		return "<unknown>";
	}
}

static std::string io_get_id(int fd, int timeout) {
	struct usbdevfs_ctrltransfer ctl;
	char buffer[64];

	ctl.bRequestType = 192;  // dir=USB_DIR_IN | type=USB_TYPE_VENDOR | recip=USB_RECIP_DEVICE
	ctl.bRequest = 0xb0;
	ctl.wValue = 8144;
	ctl.wIndex = 0;
	ctl.wLength = 32;
	ctl.timeout = timeout;
	ctl.data = buffer;

	int ret=ioctl(fd, USBDEVFS_CONTROL, &ctl);
	if (ret>=0)
		return std::string(buffer);
	else
		return "<unknown>";
}

static int io_get_status(int fd, int timeout) {
	struct usbdevfs_ctrltransfer ctl;
	unsigned char data[1];
	ctl.bRequestType = 192;  // dir=USB_DIR_IN | type=USB_TYPE_VENDOR | recip=USB_RECIP_DEVICE
	ctl.bRequest = 0xb9;
	ctl.wValue = 0;
	ctl.wIndex = 0;
	ctl.wLength = 1;
	ctl.timeout = timeout;
	ctl.data = data;

	int ret=ioctl(fd, USBDEVFS_CONTROL, &ctl);
	return ret;
}

static int io_claim_interface(int fd, int timeout) {
	struct usbdevfs_connectinfo connect;
	connect.devnum=0;
	connect.slow=1;
	int ret=ioctl(fd, USBDEVFS_CLAIMINTERFACE, &connect);
	io_get_status(fd, 1000);
	return ret;
}

static int io_release_interface(int fd, int timeout) {
	int ret=ioctl(fd, USBDEVFS_RELEASEINTERFACE, 0);
	return ret;
}

static int io_set_configuration(int fd, int configuration, int timeout) {
	int ret=ioctl(fd, USBDEVFS_SETCONFIGURATION, &configuration);
	//int ret=ioctl(fd, USBDEVFS_SETCONFIGURATION, 1);
	return ret;
}

static int io_set_pll_config(int fd, unsigned char *config, int timeout) {
	struct usbdevfs_ctrltransfer ctl;
	ctl.bRequestType = 64;  // dir=USB_DIR_OUT | type=USB_TYPE_VENDOR | recip=USB_RECIP_DEVICE
	ctl.bRequest = 0xb1;
	ctl.wValue = 0;
	ctl.wIndex = 0;
	ctl.wLength = 11;
	ctl.timeout = timeout;
	ctl.data = config;

	int ret=ioctl(fd, USBDEVFS_CONTROL, &ctl);
	return ret;
}

static int io_update_wire_in(int fd, short *wire_ins, int timeout) {
	struct usbdevfs_ctrltransfer ctl;
	ctl.bRequestType = 64;  // dir=USB_DIR_OUT | type=USB_TYPE_VENDOR | recip=USB_RECIP_DEVICE
	ctl.bRequest = 0xb5;
	ctl.wValue = 0;
	ctl.wIndex = 0;
	ctl.wLength = 64;
	ctl.timeout = timeout;
	ctl.data = wire_ins;

	int ret=ioctl(fd, USBDEVFS_CONTROL, &ctl);
	return ret;
}

static int io_update_wire_out(int fd, short *wire_outs, int timeout) {
	struct usbdevfs_ctrltransfer ctl;
	ctl.bRequestType = 192;  // dir=USB_DIR_IN | type=USB_TYPE_VENDOR | recip=USB_RECIP_DEVICE
	ctl.bRequest = 0xb5;
	ctl.wValue = 32;
	ctl.wIndex = 0;
	ctl.wLength = 64;
	ctl.timeout = timeout;
	ctl.data = wire_outs;

	int ret=ioctl(fd, USBDEVFS_CONTROL, &ctl);
	return ret;
}

static int io_update_trigger_out(int fd, short *trigger_outs, int timeout) {
	struct usbdevfs_ctrltransfer ctl;
	ctl.bRequestType = 192;  // dir=USB_DIR_IN | type=USB_TYPE_VENDOR | recip=USB_RECIP_DEVICE
	ctl.bRequest = 0xb5;
	ctl.wValue = 96;
	ctl.wIndex = 1;
	ctl.wLength = 64;
	ctl.timeout = timeout;
	ctl.data = trigger_outs;

	int ret=ioctl(fd, USBDEVFS_CONTROL, &ctl);
	return ret;
}

static int io_activate_trigger(int fd, int addr, int bit, int timeout) {
	struct usbdevfs_ctrltransfer ctl;
	unsigned char data[2];

	ctl.bRequestType = 64;  // dir=USB_DIR_OUT | type=USB_TYPE_VENDOR | recip=USB_RECIP_DEVICE
	ctl.bRequest = 0xb5;
	ctl.wValue = addr;
	ctl.wIndex = 1;
	ctl.wLength = 2;
	ctl.timeout = timeout;
	ctl.data = data;
	unsigned mask=(1<<bit);
	data[0]=mask&0xff;
	data[1]=(mask>>8) & 0xff;

	int ret=ioctl(fd, USBDEVFS_CONTROL, &ctl);
	return ret;
}

static int io_upload_bitfile(int fd, unsigned size, unsigned char *bits, int timeout) {
	// send setup message
	struct usbdevfs_ctrltransfer ctl;
	char buffer[1];

	// setup bitfile transfer
	ctl.bRequestType = 64;  // dir=USB_DIR_OUT | type=USB_TYPE_VENDOR | recip=USB_RECIP_DEVICE
	ctl.bRequest = 0xb2;
	ctl.wValue = 0;
	ctl.wIndex = 0;
	ctl.wLength = 0;
	ctl.timeout = timeout;
	ctl.data = buffer;

	int ret=ioctl(fd, USBDEVFS_CONTROL, &ctl);

	if (ret>=0) {
		struct usbdevfs_urb urb;
		urb.type=3;
		urb.endpoint=2;
		urb.status=0;
		urb.flags=0;
		urb.start_frame=0;
		urb.actual_length=0;
		urb.number_of_packets=0;
		urb.error_count=0;
		urb.signr=0;

		unsigned off=0;
		while (size>0) {
			unsigned len=16384;
			if (len>size) len=size;
			urb.buffer=bits+off;
			urb.buffer_length=len;

			ret=ioctl(fd, USBDEVFS_SUBMITURB, &urb);

			while(ioctl(fd, USBDEVFS_REAPURBNDELAY, &(urb.iso_frame_desc))<0) {
				fd_set empty_fd, write_fd;
				FD_ZERO(&empty_fd);
				FD_ZERO(&write_fd);
				FD_SET(fd,&write_fd);
				ret=select(fd+1, &write_fd, &write_fd, &write_fd, 0);
				if (ret<0) return ret;
			}
			off+=len;
			size -=len;
		}

		// check status?
		ctl.bRequestType = 192;  // dir=USB_DIR_IN | type=USB_TYPE_VENDOR | recip=USB_RECIP_DEVICE
		ctl.bRequest = 0xb2;
		ctl.wValue = 0;
		ctl.wIndex = 0;
		ctl.wLength = 1;
		ctl.timeout = timeout;
		ctl.data = buffer;
		int ret=ioctl(fd, USBDEVFS_CONTROL, &ctl);

		return ret;
	}
	return ret;
}

static int io_read_from_pipe_block(int fd, int endpoint, unsigned size, unsigned char *buffer, int timeout) {
	struct usbdevfs_ctrltransfer ctl;
	char _ctrl[6];

	// send setup message
	ctl.bRequestType = 64;  // dir=USB_DIR_OUT | type=USB_TYPE_VENDOR | recip=USB_RECIP_DEVICE
	ctl.bRequest = 0xb7;
	ctl.wValue = 512;
	ctl.wIndex = 0;
	ctl.wLength = 6;
	ctl.timeout = timeout;
	ctl.data = _ctrl;
	_ctrl[0]=endpoint;
	_ctrl[1]=6;
	_ctrl[2]=(unsigned char)(size & 0xff);
	_ctrl[3]=(unsigned char)((size>>8) & 0xff);
	_ctrl[4]=(unsigned char)((size>>16) & 0xff);
	_ctrl[5]=(unsigned char)((size>>24) & 0xff);

	int ret=ioctl(fd, USBDEVFS_CONTROL, &ctl);

	if (ret>=0) {
		struct usbdevfs_urb urb;
		urb.type=3;
		urb.endpoint=6 | USB_DIR_IN;
		urb.status=0;
		urb.flags=0;
		urb.start_frame=0;
		urb.number_of_packets=0;
		urb.error_count=0;
		urb.signr=0;

		unsigned off=0;
		while (size>0) {
			unsigned len=16384;
			if (len>size) len=size;
			urb.actual_length=0;
			urb.buffer=(char *)buffer+off;
			urb.buffer_length=len;

			ioctl(fd, USBDEVFS_SUBMITURB, &urb);
			int retry=0;
			while(ioctl(fd, USBDEVFS_REAPURBNDELAY, &(urb.iso_frame_desc))<0) {
				fd_set empty_fd, write_fd;

				if (++retry>100)	// HACK: when stalled, assume failure
					return -1;

				FD_ZERO(&empty_fd);
				FD_ZERO(&write_fd);
				FD_SET(fd,&write_fd);
				struct timeval tv = { 0,100 };
				ret=select(fd+1, &write_fd, &write_fd, &write_fd, &tv);
				//if (ret<0) return ret;
			}
			off+=len;
			size -=len;
		}
		return off;
	}
	return ret;
}

#endif
