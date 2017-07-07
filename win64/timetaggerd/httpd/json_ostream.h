/*
 * json_ostream.hpp
 *
 *  Created on: 24.04.2013
 *      Author: mag
 */

#ifndef JSON_OSTREAM_HPP_
#define JSON_OSTREAM_HPP_

#ifndef JSON_H_
#error please, dont include this file directly, use #include "json.h" instead
#endif

/*
 */
class JsonWriter {
public:
	static void write_string(std::ostream &o, const std::string &);
	static void write(std::ostream &o, jsVar *);
};


inline std::ostream &operator <<(std::ostream &out, jsVar *var) {
	JsonWriter::write(out, var);
	return out;
}

/* json support for regular ostreams using operator magic.
 * focus is on performance and conformance, using templates would have made things
 * a lot easier thow...
 *
 * sample:
 * 		std::cout << JSON( "key" << 1 << "key2" << JSON( "key3 << "test) )
 * 		std::cout << JSARRAY( 1 << 2 << 3 << 4 )
 * 		std::cout << JSARRAY( 1 << JSNULL() << 3 )
 * 		std::cout << JSARRAY( 1 << JSOBJECT() << 3 )
 *
 * also:
 * 		JSON( "key" << JSNULL << "key2" << JSEMPTY )
 * 			=>	{ "key": null, "key2": {} }
 *
 * note: std::cout << JSON() does *not* work...
 *
 * additionally, the following will *not* work:
 * 		std::cout << JSON( "key" << 1 ) << <some other stuff to emit> << std::endl
 * 		std::cout << JSON( "key" << 1 ) << std::endl
 * use instead:
 * 		std::cout << JSON( "key" << 1 ) << JSEND << <some other stuff to emit> << endl
 * 		std::cout << JSON( "key" << 1 ) << JSENDL
 *
 *
 * 	JSONs cannot be used in JSARRAYs, JSARRAYs cannot be nested.
 *  use:
 * 		std::cout << JSARRAY( 1 << JSON_ITEM( "key" << "value) << 2 )
 * 		std::cout << JSARRAY( 1 << ARRAY_ITEM( 2 << 3 ) << 2 )
 *
 * */

std::istream &operator >>(std::istream &in, jsVar *&);

#define JSON(args) ObjectWriter() << args << ObjectWriter::endObject()
#define JSARRAY(args) ObjectWriter::ArrayWriter() << args << ObjectWriter::endObject()

#define JSON_ITEM(args) ObjectWriter() << args << ObjectWriter::nextItem()()
#define ARRAY_ITEM(args) ObjectWriter::ArrayWriter() << args << ObjectWriter::nextItem()
#define JSNULL
#define JSEMPTY	ObjectWriter() << ObjectWriter::endObject()
#define JSEMPTY_ARRAY ObjectWriter::ArrayWriter()  << ObjectWriter::endObject()

#define JSKEY(str) "\"" str "\":"

#define JSON_FRAGMENT(args)  ObjectWriter() << args
#define JS_ENDOBJECT     ObjectWriter::endObject()

class ObjectWriter {
public:
	class FirstKeyWriter {
	public:
		FirstKeyWriter(std::ostream &o) : os(o) { }
		std::ostream &os;
	};
	class KeyWriter {
	public:
		KeyWriter(std::ostream &o) : os(o) { }
		std::ostream &os;
	};
	class ValueWriter {
	public:
		ValueWriter(std::ostream &o) : os(o) { }
		std::ostream &os;
	};

	class ArrayWriter {
	};

	class FirstItemWriter {
	public:
		FirstItemWriter(std::ostream &o) : os(o) { }
		std::ostream &os;
	};
	class ItemWriter {
	public:
		ItemWriter(std::ostream &o) : os(o) { }
		std::ostream &os;
	};

	class endObject {
	};
	class nextItem {
	};
	class null {
	};
	class empty {
	};
};


typedef std::ostream JSON_STREAM;
typedef ObjectWriter::KeyWriter JSON_FRAGMENT_STREAM;


// ostream << START MARK -> FKW
inline ObjectWriter::FirstKeyWriter operator <<(std::ostream &os, ObjectWriter ow) {
	os << '{';
	return ObjectWriter::FirstKeyWriter(os);
}
// FKW << END MARK -> KW
inline ObjectWriter::KeyWriter operator <<(ObjectWriter::FirstKeyWriter ow, ObjectWriter::endObject eo) {
	ow.os << '}';
	return ObjectWriter::KeyWriter(ow.os);
}
// KW << END MARK -> KW
inline ObjectWriter::KeyWriter operator <<(ObjectWriter::KeyWriter ow, ObjectWriter::endObject eo) {
	ow.os << '}';
	return ObjectWriter::KeyWriter(ow.os);
}
// KW << str -> VW
inline ObjectWriter::ValueWriter operator <<(ObjectWriter::KeyWriter ow, const std::string &key) {
	ow.os << ",\"" << key << "\":";
	return ObjectWriter::ValueWriter(ow.os);
}
// FKW << str -> VW
inline ObjectWriter::ValueWriter operator <<(ObjectWriter::FirstKeyWriter ow, const std::string &key) {
	ow.os << '"' << key << "\":";
	return ObjectWriter::ValueWriter(ow.os);
}

// allow premature end of JSON macro
inline ObjectWriter::ItemWriter operator <<(ObjectWriter::ValueWriter ow, ObjectWriter::endObject) {
	return ObjectWriter::ItemWriter(ow.os);
}

// json << ARRAY MARK -> FIW
inline ObjectWriter::FirstItemWriter operator <<(ObjectWriter::ValueWriter json, ObjectWriter::ArrayWriter ow) {
	json.os << '[';
	return ObjectWriter::FirstItemWriter(json.os);
}

// ostream << ARRAY MARK -> FIW
inline ObjectWriter::FirstItemWriter operator <<(std::ostream &os, ObjectWriter::ArrayWriter ow) {
	os << '[';
	return ObjectWriter::FirstItemWriter(os);
}

// FIW << END MARK -> KW
inline ObjectWriter::KeyWriter operator <<(ObjectWriter::FirstItemWriter ow, ObjectWriter::endObject eo) {
	ow.os << ']';
	return ObjectWriter::KeyWriter(ow.os);
}
// IW << END MARK -> KW
inline ObjectWriter::KeyWriter operator <<(ObjectWriter::ItemWriter ow, ObjectWriter::endObject eo) {
	ow.os << ']';
	return ObjectWriter::KeyWriter(ow.os);
}

// VW << START MARK -> FKW
inline ObjectWriter::FirstKeyWriter operator <<(ObjectWriter::ValueWriter ow, ObjectWriter o) {
	ow.os << "{";
	return ObjectWriter::FirstKeyWriter(ow.os);
}


/* writing object values */

// VW << str -> KW
inline ObjectWriter::KeyWriter operator <<(ObjectWriter::ValueWriter ow, const std::string &val) {
	ow.os << '"' << val << "\"";
	return ObjectWriter::KeyWriter(ow.os);
}

// VW << str -> KW
inline ObjectWriter::KeyWriter operator <<(ObjectWriter::ValueWriter ow, const char *val) {
	ow.os << '"' << val << "\"";
	return ObjectWriter::KeyWriter(ow.os);
}

// VW << bool -> KW
inline ObjectWriter::KeyWriter operator <<(ObjectWriter::ValueWriter ow, bool val) {
	ow.os << val;
	return ObjectWriter::KeyWriter(ow.os);
}
// VW << int -> KW
inline ObjectWriter::KeyWriter operator <<(ObjectWriter::ValueWriter ow, int val) {
	ow.os << val;
	return ObjectWriter::KeyWriter(ow.os);
}
// VW << uint -> KW
inline ObjectWriter::KeyWriter operator <<(ObjectWriter::ValueWriter ow, unsigned int val) {
	ow.os << val;
	return ObjectWriter::KeyWriter(ow.os);
}
// VW << long -> KW
inline ObjectWriter::KeyWriter operator <<(ObjectWriter::ValueWriter ow, long val) {
	ow.os << val;
	return ObjectWriter::KeyWriter(ow.os);
}
// VW << ulong -> KW
inline ObjectWriter::KeyWriter operator <<(ObjectWriter::ValueWriter ow, unsigned long val) {
	ow.os << val;
	return ObjectWriter::KeyWriter(ow.os);
}
// VW << long long -> KW
inline ObjectWriter::KeyWriter operator <<(ObjectWriter::ValueWriter ow, long long val) {
	ow.os << val;
	return ObjectWriter::KeyWriter(ow.os);
}
// VW << unsigned long long -> KW
inline ObjectWriter::KeyWriter operator <<(ObjectWriter::ValueWriter ow, unsigned long long val) {
	ow.os << val;
	return ObjectWriter::KeyWriter(ow.os);
}
// VW << double -> KW
inline ObjectWriter::KeyWriter operator <<(ObjectWriter::ValueWriter ow, double val) {
	ow.os << val;
	return ObjectWriter::KeyWriter(ow.os);
}

template<typename T>
inline ObjectWriter::KeyWriter operator <<(ObjectWriter::ValueWriter ow, std::vector<T> vec) {
	std::ostream &out=ow.os;
	out << '[' ;
	for (size_t n=0; n<vec.size(); ++n) {
		if (n>0) out << ',';
		out << vec[n];
	}
	out << ']' ;
	return ObjectWriter::KeyWriter(out);
}



/* writing array values, first element */

// VW << str -> KW
inline ObjectWriter::ItemWriter operator <<(ObjectWriter::FirstItemWriter ow, const std::string &val) {
	ow.os << '"' << val << "\"";
	return ObjectWriter::ItemWriter(ow.os);
}
// VW << str -> KW
inline ObjectWriter::ItemWriter operator <<(ObjectWriter::FirstItemWriter ow, const char *val) {
	ow.os << '"' << val << "\"";
	return ObjectWriter::ItemWriter(ow.os);
}
// VW << bool -> KW
inline ObjectWriter::ItemWriter operator <<(ObjectWriter::FirstItemWriter ow, bool val) {
	ow.os << val;
	return ObjectWriter::ItemWriter(ow.os);
}
// VW << int -> KW
inline ObjectWriter::ItemWriter operator <<(ObjectWriter::FirstItemWriter ow, int val) {
	ow.os << val;
	return ObjectWriter::ItemWriter(ow.os);
}
// VW << long -> KW
inline ObjectWriter::ItemWriter operator <<(ObjectWriter::FirstItemWriter ow, long val) {
	ow.os << val;
	return ObjectWriter::ItemWriter(ow.os);
}
// VW << long long -> KW
inline ObjectWriter::ItemWriter operator <<(ObjectWriter::FirstItemWriter ow, long long val) {
	ow.os << val;
	return ObjectWriter::ItemWriter(ow.os);
}
// VW << double -> KW
inline ObjectWriter::ItemWriter operator <<(ObjectWriter::FirstItemWriter ow, double val) {
	ow.os << val;
	return ObjectWriter::ItemWriter(ow.os);
}

/* writing array values */

// VW << str -> KW
inline ObjectWriter::ItemWriter operator <<(ObjectWriter::ItemWriter ow, const std::string &val) {
	ow.os << ",\"" << val << "\"";
	return ObjectWriter::ItemWriter(ow.os);
}
inline ObjectWriter::ItemWriter operator <<(ObjectWriter::ItemWriter ow, const char *val) {
	ow.os << ",\"" << val << "\"";
	return ObjectWriter::ItemWriter(ow.os);
}
// VW << bool -> KW
inline ObjectWriter::ItemWriter operator <<(ObjectWriter::ItemWriter ow, bool val) {
	ow.os << ',' << val;
	return ObjectWriter::ItemWriter(ow.os);
}
// VW << int -> KW
inline ObjectWriter::ItemWriter operator <<(ObjectWriter::ItemWriter ow, int val) {
	ow.os << ',' << val;
	return ObjectWriter::ItemWriter(ow.os);
}
// VW << long -> KW
inline ObjectWriter::ItemWriter operator <<(ObjectWriter::ItemWriter ow, long val) {
	ow.os << ',' << val;
	return ObjectWriter::ItemWriter(ow.os);
}
// VW << long long -> KW
inline ObjectWriter::ItemWriter operator <<(ObjectWriter::ItemWriter ow, long long val) {
	ow.os << ',' << val;
	return ObjectWriter::ItemWriter(ow.os);
}
// VW << double -> KW
inline ObjectWriter::ItemWriter operator <<(ObjectWriter::ItemWriter ow, double val) {
	ow.os << ',' << val;
	return ObjectWriter::ItemWriter(ow.os);
}



#endif /* JSON_OSTREAM_HPP_ */
