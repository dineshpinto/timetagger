/*
 * HttpStream.hpp
 *
 *  Created on: 24.04.2013
 *      Author: mag
 */

#ifndef HTTPSTREAM_HPP_
#define HTTPSTREAM_HPP_

#include <streambuf>
#include <iosfwd>
#include <vector>

#include "mongoose.h"

typedef struct mg_connection MGCON;

class MongooseInbuffer : public std::streambuf {
public:
	MongooseInbuffer(MGCON *con, size_t buff_sz = 1024, size_t put_back = 8);

private:
    // overrides base class underflow()
    int underflow();

    // copying not allowed
    MongooseInbuffer(const MongooseInbuffer &);
    MongooseInbuffer &operator= (const MongooseInbuffer &);

private:
    MGCON *connection;
    const size_t putback, buffersize;
    std::vector<char> buffer;
};


class MongooseOutbuffer : public std::streambuf {
public:
	explicit MongooseOutbuffer(MGCON *con, std::size_t buff_sz = 9000);	// being optimistic, ~9k is GbitEth MTU

private:
	int overflow(int ch);
	int flush();
	int sync();

	// copying not allowed
	MongooseOutbuffer(const MongooseOutbuffer &);
	MongooseOutbuffer &operator= (const MongooseOutbuffer &);

private:
	MGCON *connection;
	std::vector<char> buffer;
};


#endif /* HTTPSTREAM_HPP_ */
