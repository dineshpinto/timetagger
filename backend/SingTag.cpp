/*
	backend for TimeTagger, an OpalKelly based single photon counting library
	Copyright (C) 2011  Markus Wick <wickmarkus@web.de>

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License along
	with this program; if not, write to the Free Software Foundation, Inc=2E,
	51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#ifndef TT_SINGTAG_CPP_IMPL
#define TT_SINGTAG_CPP_IMPL

#include <stdio.h>
#include <stdint.h>
#include <vector>

#include "TimeTagger.h"

class SingTag: public _Iterator {
public:
	SingTag(TimeTagger* t, std::vector<channel_t> _channels, int _outmode = 0) : _Iterator(t), channels(_channels) {
		init(_outmode);
	}

	SingTag(TimeTagger* t) : _Iterator(t) {
		for (int n=0; n<8; ++n) {
			channels.push_back(n);
		}
		init(0);
	}

private:
	void init(int _outmode) {
		outmode = _outmode;
		output = stdout;

		for(size_t c=0; c<channels.size(); c++)
			registerChannel(channels[c]);

		clear();
		start();
	}

public:
	virtual ~SingTag() {
		stop();
	}

	virtual void clear() {
		lock();
		curtag.hex = 0;
		unlock();
	}

protected:
	virtual void next(Tag* list, int count, timestamp_t time) {
		for(int i=0; i<count; i++) {
			uint64_t t = (list[i].time/125) & ((1ULL<<49) - 1);

			if(list[i].overflow) {
				flush();
				curtag.time = t;
				curtag.un = 0x01;
				flush();
			}

			for (size_t c=0; c<channels.size(); c++) {
				if (channels[c] == list[i].chan) {
					if (t != curtag.time) flush();

					curtag.time = t;
					curtag.bits |= 1 << c;
				}
			}
		}
	}

private:
	void flush() {
		if (!curtag.hex) return;

		switch (outmode) {
		case 0:
			fprintf(output, "%08x\n%08x\n", curtag.dv ,curtag.cv);
			break;
		case 1:
			fwrite(&curtag.dv, 4, 1, output);
			fwrite(&curtag.cv, 4, 1, output);
			break;
		case 2:
			fprintf(output, "%08x%08x\n", curtag.dv, curtag.cv);
			break;
		}

		curtag.hex = 0;
	}

	std::vector<channel_t> channels;
	int outmode;
	FILE* output;

	union singtag {
		struct {
			uint64_t bits :  8;
			uint64_t un   :  7;
			uint64_t time : 49;
		};
		struct {
			uint32_t cv;
			uint32_t dv;
		};
		uint64_t hex;
	} curtag;
};

#endif // #ifndef TT_SINGTAG_CPP_IMPL
