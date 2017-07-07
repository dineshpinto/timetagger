/*
 * HttpStream.cpp
 *
 *  Created on: 24.04.2013
 *      Author: mag
 */

#include "HttpStream.h"

#include <algorithm>
#include <cstring>

using std::size_t;

MongooseInbuffer::MongooseInbuffer(MGCON *c, size_t bufsz, size_t pb)
: connection(c),
  putback( std::max(pb, size_t(1)) ), 				// min 1 byte putback size
  buffersize( std::max(bufsz, putback) + putback),	// buffersize must be bigger than 2*put_back
  buffer(buffersize)   {
    char *end = &buffer.front() + buffer.size();
    setg(end, end, end);
}


int MongooseInbuffer::underflow() {

    if (gptr() < egptr()) // buffer not exhausted
        return traits_type::to_int_type(*gptr());

    char *base = &buffer.front();
    char *start = base;

    if (eback() == base) { // true when this isn't the first fill
        // Make arrangements for putback characters
        std::memmove(base, egptr() - putback, putback);
        start += putback;
    }

    // start is now the start of the buffer, proper.
    // Read from fptr_ in to the provided buffer
    size_t n = mg_read(connection, start, buffer.size() - (start - base));
    if (n == 0)
        return traits_type::eof();

    // Set buffer pointers
    setg(base, start, start + n);

    return traits_type::to_int_type(*gptr());
}


MongooseOutbuffer::MongooseOutbuffer(MGCON *c, std::size_t buff_sz)
: connection(c),
  buffer(buff_sz + 1) {
    char *base = &buffer.front();
    setp(base, base + buffer.size() - 1); // -1 to make overflow() easier
}

int MongooseOutbuffer::overflow(int ch) {
    if (ch != traits_type::eof()) {
        //assert(std::less_equal<char *>()(pptr(), epptr()));
        *pptr() = ch;
        pbump(1);
        if (flush()<0)
        	return -1;
        else
        	return ch;
    }
    return -1;
}

int MongooseOutbuffer::flush() {
	 int cnt = pptr()-pbase();
	 char *base = &buffer.front();
	 if (mg_write(connection, base, cnt) != cnt) {
		 return -1;
	 }
	 pbump(-cnt); // reset put pointer accordingly
	 return cnt;
}

int MongooseOutbuffer::sync() {
	if (flush()<0)
		return -1;
	else
		return 0;
}


