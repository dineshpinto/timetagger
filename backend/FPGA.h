
#include <string>
#include <vector>

#ifdef _MSC_VER
#if _MSC_VER>1500
#include <stdint.h>
#else
// stupid vc80 doesnt know about <stdint.h>
typedef __int8              int8_t;
typedef __int16             int16_t;
typedef __int32             int32_t;
typedef __int64             int64_t;
typedef unsigned __int8     uint8_t;
typedef unsigned __int16    uint16_t;
typedef unsigned __int32    uint32_t;
typedef unsigned __int64    uint64_t;

#define INT8_MIN            _I8_MIN
#define INT8_MAX            _I8_MAX
#define INT16_MIN           _I16_MIN
#define INT16_MAX           _I16_MAX
#define INT32_MIN           _I32_MIN
#define INT32_MAX           _I32_MAX
#define INT64_MIN           _I64_MIN
#define INT64_MAX           _I64_MAX
#define UINT8_MAX           _UI8_MAX
#define UINT16_MAX          _UI16_MAX
#define UINT32_MAX          _UI32_MAX
#define UINT64_MAX          _UI64_MAX
#endif
#else

#include <stdint.h>
#endif


//#define FPGA_FRONTPANEL
//#define FPGA_LIBUSB
//#define FPGA_DIRECT_IO


#if defined(FPGA_FRONTPANEL)
#include "okFrontPanelDLL.h"

#elif defined(FPGA_DIRECT_IO)

#else

#error NO FPGA INTERFACE DEFINED

#endif

enum OK_WIRE_ADDR {
    ADDR_EMPTY_THRESHOLD = 0x01,
    ADDR_CHANNEL_SELECTION = 0x02,
    ADDR_DAC_DIN_LOW = 0x03,
    ADDR_DAC_DIN_HIGH = 0x04,
    ADDR_LASER_FILTER = 0x05,
    ADDR_CALIBRATION = 0x06,
    ADDR_DEADTIME = 0x10,
    ADDR_DEADTIME_LAST = 0x1f,
    ADDR_DAC_WR_EN = 0x40,
    ADDR_PIPE = 0xa0,
    //added, 04.09.2015, n.abt --> endpoint addresses for the 2x16 bit for AWG
    ADDR_AWG_DATA_LOW = 0x07,
    ADDR_AWG_DATA_HIGH = 0x08,
};

struct FPGADevice {
	enum {
		USED=0, IDLE=1
	} state;
	std::string serial;
	std::string id;
	union {
		struct {
			uint16_t product;
			uint16_t vendor;
		};
		uint32_t model;
	};
};


class FPGA {
public:
	FPGA();
	~FPGA();

	bool open(std::string serial);
	void close();
	std::string getSerial();

	void setTimeout(unsigned);
	void setBlocksize(unsigned);

	bool setPLLConfig(unsigned char []);
	bool uploadBitfile(std::string );

	bool setWireIn(unsigned addr, short value);
	bool getWireOut(unsigned addr, short &value);

	bool ActivateTrigger(unsigned addr, unsigned bit);
	bool isTriggered(unsigned addr, unsigned bit);

	bool UpdateWireIns();
	bool UpdateWireOuts();
	bool UpdateTriggerOuts();

	int ReadFromPipeOut(unsigned backend, unsigned size, unsigned char *buffer);

	static std::vector<FPGADevice> getDeviceList();

#if defined(FPGA_DIRECT_IO)
protected:
	static bool list_func(int fd, const std::string &path,
			              int vid, int pid,
			              const std::string &serial, const std::string &id, void *data);
	static bool open_func(int fd, const std::string &path,
			              int vid, int pid,
			              const std::string &serial, const std::string &id, void *data);
private:
	int fd;
	int timeout;
	int blocksize;
	short wire_ins[32];
	short wire_outs[32];
	short trigger_outs[32];

#endif

#if defined(FPGA_FRONTPANEL)

private:
	okCFrontPanel *xem;
	unsigned blocksize;

#endif
};

