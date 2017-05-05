#include <iostream>
#include "json.h"

#if 0
void test1(std::ostream &out) {

	out << JSON(
				"key1" << 1
			<< 	"key2" << "test"
	);
	out << std::endl;

}


void meth23(ObjectWriter::KeyWriter &json) {
	json << "result" << JSON( "A" << 23 << "B" << JSON( "a" << 1 << "b" << 5) );
}

void meth22(ObjectWriter::KeyWriter &json) {
	json << "result" << JSON( "A" << 23 << "B" << "test" );
}

void meth21(ObjectWriter::KeyWriter &json) {
	json << "result" << 23;
}

void test2(std::ostream &out) {
	ObjectWriter::KeyWriter json = (out << ObjectWriter() << "jsonrpc" << "2.0");
	json << "status" << 1;
	meth21(json);
	meth22(json);
	meth23(json);
	json << ObjectWriter::endObject();
	out << std::endl;
}


void meth31(ObjectWriter::KeyWriter &json) {
	json << "result" << JSARRAY( 1 << 2 << 3 << 4 );
}

void meth32(ObjectWriter::KeyWriter &json) {
	// this aint work yet
	//json << "result" << JSARRAY( JSON_ITEM("a" << 2) );
}

void meth33(ObjectWriter::KeyWriter &json) {
	std::vector<int> vec;
	vec.push_back(1);
	vec.push_back(2);
	vec.push_back(3);
	json << "result" << vec;
}

void meth34(ObjectWriter::KeyWriter &json) {
	std::vector<double> vec;
	vec.push_back(1);
	vec.push_back(2);
	vec.push_back(3);
	json << "result" << vec;
}

void test3(std::ostream &out) {
	ObjectWriter::KeyWriter json = (out << ObjectWriter() << "jsonrpc" << "2.0");
	meth31(json);
	meth32(json);
	meth33(json);
	meth34(json);
	json << ObjectWriter::endObject();
	out << std::endl;

}

int main() {

	test1(std::cout);
	test2(std::cout);
	test3(std::cout);

}
#endif
