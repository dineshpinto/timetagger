/*
 * json_reader.h
 *
 *  Created on: 24.04.2013
 *      Author: mag
 */

#ifndef JSON_READER_H_
#define JSON_READER_H_

#ifndef JSON_H_
#error please, dont include this file directly, use #include "json.h" instead
#endif

class JsonReader {

public:
	typedef enum {
		JSON=0,
		ECMASCRIPT=1,
		TENGEN=2
	} MODE;

	JsonReader(std::istream &, MODE m=JSON);
	jsVar *parse();

protected:

	jsVar *parse_var();
	std::string *parse_identifier();
	std::string *parse_string(char);
	double parse_number();
	unsigned parse_hex();

	jsVar *syntax_error();

	void append_utf8(std::string &result, unsigned uc);

	char nch();
	char nch_skip_blanks();
	bool eof();
	void unget();

private:
	std::istream &in;
	int lino;
	int pos;
	MODE mode;
};

#endif /* JSON_READER_H_ */
